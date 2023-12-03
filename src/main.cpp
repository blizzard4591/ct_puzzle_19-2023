#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <list>
#include <string>
#include <unordered_set>
#include <queue>

#include <QString>

#include <QGuiApplication>

enum class BoardPiece {
	EMPTY,
	HOLE,
	WALL,
	TREE,
	PRESENT,
	TARGET
};

static const std::array<std::string, 20> fieldStringBasic = {
	"####################", 
	"#   # # #          #", 
	"#      ###  #     O#", 
	"#       #   #  #   #", 
	"##  # #  # #       #", 
	"#       #    #   ###", 
	"#      # #  #      #", 
	"##      #       ## #", 
	"#    ### ### ## #  #", 
	"#    #P #          #", 
	"# O               ##", 
	"#       #          #", 
	"#        # #       #", 
	"#   #   #   #    # #", 
	"#    #   #  #  #   #", 
	"#  #    #      ## ##", 
	"#     #  #    ##   #", 
	"#     # # #        X", 
	"# #  ##    #    #  #", 
	"####################"
};
static const std::vector<std::pair<std::size_t, std::size_t>> holeConnectionsBasic = {
	{58, 202},
	{202, 58}
};

static const std::array<std::string, 40> fieldStringChristmas = {
	"TT T#T   #T#T#T#T #   T #T#  #TT  #T$  #",
	"T     #     $                          T",
	"T       $  T TT  TT##T#TT   T  T  T  T  ",
	"   T     T   T  T         T$            ",
	"#T     T   O   T$TT##TTTT   TT T        ",
	"  $ T                           T   T   ",
	"#                TT#TTTTT$             T",
	"     T  T    TT$                  T$    ",
	" T          T$   T#TT#T#T  T#          #",
	"        T    T T         #      T      O",
	"# T     #    T   T##TTTT# T T      T#T $",
	"    T T              P         # $     #",
	"#          TT    T##T#TTT T   T    T T #",
	"     TTT   T   T       $     #   T   T T",
	"T  #$        T  #T###TT#T T TT          ",
	"   ## T   T     T        T T        T T#",
	"  #T        #  T#T##TT#T#  T T    T     ",
	"T T  T T T  #                # TT T    #",
	"     T   #   T   TT#TTTTT T# T       T  ",
	"T  T       #  #     $           T   TTT ",
	"   T  T      T  #T##TTT## T  T#     T   ",
	"#   T    T    T T        TTT  T TT     T",
	"$T  #T    T    T TT#TT### T  T        T ",
	"# T T#    T #T            TT   T T T    ",
	"#        T  $    T###T#TT            T  ",
	"   T T        T        #  T     T    TT ",
	"T T T  ##   T T  T  T  T$T    T  T T  $ ",
	"   TT  T      T T $# T T   TT        T T",
	"      #TT  T                T    #     T",
	"  T        TTT T T T  T#T           T  T",
	"T  T    T      T             T T      TT",
	"#T   T        T  T TT # TT  TT  #      #",
	"#   T    ##    $   TT    #  #  T# T     ",
	" T     T     T TT TT#T      T   T       ",
	" # T              T T   T  #T T #  TT T#",
	"T  T      T T #    T  T   $            T",
	"T      TT       #        # #  T       TT",
	"# T         T      T          # T  T   #",
	"T$ O           T     T     T    T      X",
	" T #   # T    #T# #  ##   T  T  $ # #TTT"
};
static const std::vector<std::pair<std::size_t, std::size_t>> holeConnectionsChristmas = {
	{399, 1523},
	{1523, 171},
	{171, 399}
};

template<std::size_t NUM_ROWS, std::size_t NUM_COLS>
class PresentBase {
public:
	PresentBase(std::vector<std::size_t> presentCoordinates) {
		std::sort(presentCoordinates.begin(), presentCoordinates.end());

		if (presentCoordinates.size() > 32) {
			std::cerr << "Internal Error: Can not work with > 32 presents." << std::endl;
			exit(-1);
		}
		
		// Mark as "not mapped"
		for (std::size_t i = 0; i < m_mapToBitset.size(); ++i) {
			m_mapToBitset[i] = std::numeric_limits<std::size_t>::max();
		}

		std::size_t indexCounter = 0;
		for (auto it = presentCoordinates.cbegin(); it != presentCoordinates.cend(); ++it) {
			if (*it >= (NUM_ROWS * NUM_COLS)) {
				std::cerr << "Invalid input, present coordinate " << *it << " is out of bounds." << std::endl;
				exit(-1);
			}
			m_mapToBitset[*it] = indexCounter;
			++indexCounter;
		}

		m_totalPresentCount = indexCounter;
	}
	~PresentBase() {
		//
	}

	inline std::size_t getBitmapIndex(std::size_t const& pos) const {
		return m_mapToBitset[pos];
	}

	inline std::size_t getTotalPresentCount() const noexcept(true) {
		return m_totalPresentCount;
	}
private:
	std::array<std::size_t, (NUM_ROWS * NUM_COLS)> m_mapToBitset;
	std::size_t m_totalPresentCount;
};

template<std::size_t NUM_ROWS, std::size_t NUM_COLS>
class PresentOverlay {
public:
	PresentOverlay(PresentBase<NUM_ROWS, NUM_COLS> const& base) : m_presentBase(base) {
		m_presents.reset();
		for (std::size_t i = 0; i < m_presentBase.getTotalPresentCount(); ++i) {
			m_presents[i] = true;
		}
	}
	PresentOverlay(PresentBase<NUM_ROWS, NUM_COLS> const& base, std::bitset<32> const& presents) : m_presentBase(base), m_presents(presents) {
		//
	}

	~PresentOverlay() {
		//
	}

	void collectOn(std::size_t const& pos) {
		if (pos >= (NUM_ROWS * NUM_COLS)) {
			std::cerr << "Invalid input, present coordinate " << pos << " is out of bounds." << std::endl;
			exit(-1);
		}
		std::size_t const mappedIndex = m_presentBase.getBitmapIndex(pos);
		if (mappedIndex < 32) {
			m_presents[mappedIndex] = false;
		}
	}

	inline std::size_t getPresentsLeft() const {
		return m_presents.count();
	}

	inline std::size_t getPresentsCollected() const {
		return m_presentBase.getTotalPresentCount() - m_presents.count();
	}

	inline std::bitset<32> const& getRepresentation() const noexcept(true) {
		return m_presents;
	}

	inline PresentBase<NUM_ROWS, NUM_COLS> const& getBase() const noexcept(true) {
		return m_presentBase;
	}
private:
	PresentBase<NUM_ROWS, NUM_COLS> const m_presentBase;
	std::bitset<32> m_presents;
};

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS>
class Board {
public:
	Board(std::array<BoardPiece, NUM_ROWS * NUM_COLS> const& pieces, std::size_t const& penguinStartingPosition, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections)
		: m_pieces(pieces), m_startingPosition(penguinStartingPosition), m_holeConnections(holeConnections) {
		//
	}
	~Board() {
		//
	}

	static std::pair<Board<NUM_ROWS, NUM_COLS, IS_TORUS>, PresentOverlay<NUM_ROWS, NUM_COLS>> fromFieldString(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections) {
		std::array<BoardPiece, NUM_ROWS * NUM_COLS> pieces;
		std::size_t linearCount = 0;
		BoardPiece piece;
		std::optional<std::size_t> penguinPosition;
		std::vector<std::size_t> holePositions;
		bool holeIsValid = false;
		std::vector<std::size_t> presentPositions;

		for (std::size_t row = 0; row < NUM_ROWS; ++row) {
			for (std::size_t col = 0; col < NUM_COLS; ++col) {
				char const c = fieldString.at(row).at(col);
				
				switch (c) {
					case 'T':
						piece = BoardPiece::TREE;
						break;
					case '#':
						piece = BoardPiece::WALL;
						break;
					case ' ':
						piece = BoardPiece::EMPTY;
						break;
					case 'O':
						piece = BoardPiece::HOLE;
						holePositions.push_back(xyToPos(row, col));
						holeIsValid = false;
						for (auto it = holeConnections.cbegin(); it != holeConnections.cend(); ++it) {
							if (it->first == xyToPos(row, col)) {
								holeIsValid = true;
								break;
							}
						}						
						if (!holeIsValid) {
							std::cerr << "Invalid input, found hole that has no connection at row " << row << ", column " << col << "." << std::endl;
							exit(-1);
						}
						break;
					case 'P':
						piece = BoardPiece::EMPTY;
						penguinPosition = xyToPos(row, col);
						break;
					case '$':
						presentPositions.push_back(xyToPos(row, col));
						piece = BoardPiece::EMPTY;
						break;
					case 'X':
						piece = BoardPiece::TARGET;
						break;
					default:
						std::cerr << "Invalid input, could not parse character '" << c << "' at row " << row << ", column " << col << "." << std::endl;
						exit(-1);
				}
				pieces.at(linearCount) = piece;
				++linearCount;
			}
		}
		if (!penguinPosition) {
			std::cerr << "Invalid input, could not find penguin on the board!" << std::endl;
			exit(-1);
		} else if (holePositions.size() != holeConnections.size()) {
			std::cerr << "Invalid input, could not find all hole positions on the board!" << std::endl;
			exit(-1);
		}

		return std::make_pair(Board<NUM_ROWS, NUM_COLS, IS_TORUS>(pieces, penguinPosition.value(), holeConnections), PresentOverlay<NUM_ROWS, NUM_COLS>(presentPositions));
	}

	std::size_t getPenguinStartingPosition() const {
		return m_startingPosition;
	}

	BoardPiece const& getPieceAt(std::size_t const& pos) const {
		return m_pieces.at(pos);
	}

	bool isPieceNotSolid(BoardPiece const& piece) const {
		return piece != BoardPiece::WALL && piece != BoardPiece::TREE;
	}

	bool canMoveUp(std::size_t const& pos, std::size_t& target) const {
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if (pos < NUM_COLS) {
			if (IS_TORUS) {
				target = pos + ((NUM_ROWS - 1) * NUM_COLS);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosAbove(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	bool canMoveDown(std::size_t const& pos, std::size_t& target) const {
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if (pos >= ((NUM_ROWS - 1) * NUM_COLS)) {
			if (IS_TORUS) {
				target = pos - ((NUM_ROWS - 1) * NUM_COLS);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosBelow(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	bool canMoveLeft(std::size_t const& pos, std::size_t& target) const {
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if ((pos % NUM_COLS) == 0) {
			if (IS_TORUS) {
				target = pos + (NUM_COLS - 1);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosLeft(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	bool canMoveRight(std::size_t const& pos, std::size_t& target) const {
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if ((pos % NUM_COLS) == (NUM_COLS - 1)) {
			if (IS_TORUS) {
				target = pos - (NUM_COLS - 1);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosRight(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	std::size_t moveUp(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS>& presents) const {
		std::size_t result = 0;
		while (canMoveUp(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveDown(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS>& presents) const {
		std::size_t result = 0;
		while (canMoveDown(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveLeft(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS>& presents) const {
		std::size_t result = 0;
		while (canMoveLeft(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveRight(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS>& presents) const {
		std::size_t result = 0;
		while (canMoveRight(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}
private:
	std::size_t swapHoleIfOn(std::size_t const& pos) const {
		for (auto it = m_holeConnections.cbegin(); it != m_holeConnections.cend(); ++it) {
			if (pos == it->first) return it->second;
		}
		return pos;
	}

	static inline std::size_t xyToPos(std::size_t const& row, std::size_t const& col) {
		return row * NUM_COLS + col;
	}

	static inline std::size_t getPosAbove(std::size_t const& pos) {
		return pos - NUM_COLS;
	}

	static inline std::size_t getPosBelow(std::size_t const& pos) {
		return pos + NUM_COLS;
	}

	static inline std::size_t getPosLeft(std::size_t const& pos) {
		return pos - 1;
	}

	static inline std::size_t getPosRight(std::size_t const& pos) {
		return pos + 1;
	}

	std::array<BoardPiece, NUM_ROWS * NUM_COLS> const m_pieces;
	std::size_t const m_startingPosition;
	std::vector<std::pair<std::size_t, std::size_t>> const m_holeConnections;
};

class QueueObject {
public:
	QueueObject(std::size_t const& penguinPosition, std::bitset<32> const& presentState) : m_pos(penguinPosition), m_presentState(presentState), m_moves("") {
		//
	}

	QueueObject(std::size_t const& pos, std::bitset<32> const& presentState, std::string&& moves) : m_pos(pos), m_presentState(presentState), m_moves(moves) {
		//
	}

	inline std::size_t getPos() const noexcept(true) {
		return m_pos;
	}

	inline std::bitset<32> getPresentState() const noexcept(true) {
		return m_presentState;
	}

	inline std::string const& getMoves() const noexcept(true) {
		return m_moves;
	}

	QueueObject moveTo(std::size_t const& newPos, std::bitset<32> const& presentState, char direction) const {
		return QueueObject(newPos, presentState, m_moves + direction);
	}
private:
	std::size_t const m_pos;
	std::bitset<32> const m_presentState;
	std::string const m_moves;
};

inline std::size_t makeKey(std::size_t const& pos, unsigned long const& presentState) noexcept(true) {
	return (pos << 32uLL) | (presentState & 0xFFFFFFFFuLL);
}

bool is_b_subset_of_a(unsigned long const& a, unsigned long const& b) noexcept(true) {
	return (b & ~a) == 0ul;
}

std::vector<std::unordered_set<unsigned long>> knownPositions;
bool hasSeenPosition(std::size_t const& pos, std::bitset<32> const& presentState) {
	std::unordered_set<unsigned long> const& set = knownPositions[pos];
	auto const newState = presentState.to_ulong();
	if (set.find(newState) != set.cend()) {
		return true;
	}
	
	for (auto it = set.cbegin(); it != set.cend(); ++it) {
		auto const& knownState = *it;
		if (is_b_subset_of_a(newState, knownState)) {
			return true;
		}
	}
	return false;
}

inline void addPosition(std::size_t const& pos, std::bitset<32> const& presentState) {
	knownPositions[pos].insert(presentState.to_ulong());
}

void cleanupKnownPositions() {
	auto const beginTotal = std::chrono::steady_clock::now();

	std::size_t removalCount = 0;
	std::size_t totalCount = 0;
	for (auto itO = knownPositions.begin(); itO != knownPositions.end(); ++itO) {
		std::unordered_set<unsigned long>& set = *itO;
		totalCount += set.size();
		if (set.size() < 2) {
			continue;
		}

		for (auto it = set.begin(); it != set.end(); ++it) {
			auto const& knownState = *it;
			for (auto it2 = set.begin(); it2 != set.end(); ++it2) {
				auto const& otherState = *it;
				if (knownState == otherState) {
					continue;
				}
				if (is_b_subset_of_a(knownState, otherState)) {
					it = set.erase(it);
					++removalCount;
					break;
				}
			}
		}
	}

	auto const endIterate = std::chrono::steady_clock::now();
	std::cout << "Removed " << removalCount << " entries in cleanup round taking " << std::chrono::duration_cast<std::chrono::microseconds>(endIterate - beginTotal).count() << "us - " << totalCount << " entries (" << (totalCount / knownPositions.size()) << " avg.)." << std::endl;
}

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS>
void play(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections) {
	auto const init = Board<NUM_ROWS, NUM_COLS, IS_TORUS>::fromFieldString(fieldString, holeConnections);
	Board<NUM_ROWS, NUM_COLS, IS_TORUS> board = init.first;
	PresentOverlay<NUM_ROWS, NUM_COLS> presentOverlay = init.second;

	std::queue<QueueObject> penguinPositions;
	
	knownPositions.clear();
	for (std::size_t i = 0; i < NUM_ROWS * NUM_COLS; ++i) {
		knownPositions.push_back(std::unordered_set<unsigned long>());
	}

	addPosition(board.getPenguinStartingPosition(), presentOverlay.getRepresentation());
	penguinPositions.push(QueueObject(board.getPenguinStartingPosition(), presentOverlay.getRepresentation()));

	std::size_t newPos;
	std::size_t target;

	std::size_t roundCounter = 0;
	while (!penguinPositions.empty()) {
		++roundCounter;
		if (roundCounter % 100000 == 0) {
			cleanupKnownPositions();
		}

		QueueObject const& p = penguinPositions.front();

		if (board.getPieceAt(p.getPos()) == BoardPiece::TARGET) {
			PresentOverlay<NUM_ROWS, NUM_COLS> localOverlay(presentOverlay.getBase(), p.getPresentState());
			std::cout << "Found target with " << localOverlay.getPresentsLeft() << " presents left using moves '" << p.getMoves() << "'." << std::endl;
			if (localOverlay.getPresentsLeft() == 0) {
				return;
			} else {
				penguinPositions.pop();
				continue;
			}
		}

		if (board.canMoveUp(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveUp(p.getPos(), localOverlay);
			if (!hasSeenPosition(newPos, localOverlay.getRepresentation())) {
				addPosition(newPos, localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'U'));
			}
		}
		if (board.canMoveDown(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveDown(p.getPos(), localOverlay);
			if (!hasSeenPosition(newPos, localOverlay.getRepresentation())) {
				addPosition(newPos, localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'D'));
			}
		}
		if (board.canMoveLeft(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveLeft(p.getPos(), localOverlay);
			if (!hasSeenPosition(newPos, localOverlay.getRepresentation())) {
				addPosition(newPos, localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'L'));
			}
		}
		if (board.canMoveRight(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveRight(p.getPos(), localOverlay);
			if (!hasSeenPosition(newPos, localOverlay.getRepresentation())) {
				addPosition(newPos, localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'R'));
			}
		}

		penguinPositions.pop();
	}
}

int main(int argc, char* argv[]) {
	QGuiApplication app(argc, argv);

	std::cout << "c't, Puzzle 19/2023" << std::endl;
	auto const beginTotal = std::chrono::steady_clock::now();

	if (false) {
		play<20, 20, false>(fieldStringBasic, holeConnectionsBasic);
	} else {
		play<40, 40, true>(fieldStringChristmas, holeConnectionsChristmas);
	}

	auto const endIterate = std::chrono::steady_clock::now();
	std::cout << "Looked at " << 0 << " combinations in " << std::chrono::duration_cast<std::chrono::microseconds>(endIterate - beginTotal).count() << "us." << std::endl;

	
	std::cout << "Done!" << std::endl;
	return 0;
}

#ifdef _MSC_VER
int __stdcall WinMain(struct HINSTANCE__* hInstance, struct HINSTANCE__* hPrevInstance, char* lpszCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}

#endif
