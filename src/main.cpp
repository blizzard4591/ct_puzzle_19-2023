#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <unordered_set>
#include <queue>

#include "Trie.h"

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

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, std::size_t BIT_COUNT>
class PresentOverlay {
public:
	PresentOverlay(PresentBase<NUM_ROWS, NUM_COLS> const& base) : m_presentBase(base) {
		m_presents.reset();
		for (std::size_t i = 0; i < m_presentBase.getTotalPresentCount(); ++i) {
			m_presents[i] = true;
		}
	}
	PresentOverlay(PresentBase<NUM_ROWS, NUM_COLS> const& base, std::bitset<BIT_COUNT> const& presents) : m_presentBase(base), m_presents(presents) {
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
		if (mappedIndex < BIT_COUNT) {
			m_presents[mappedIndex] = false;
		}
	}

	inline std::size_t getPresentsLeft() const {
		return m_presents.count();
	}

	inline std::size_t getPresentsCollected() const {
		return m_presentBase.getTotalPresentCount() - m_presents.count();
	}

	inline std::bitset<BIT_COUNT> const& getRepresentation() const noexcept(true) {
		return m_presents;
	}

	inline PresentBase<NUM_ROWS, NUM_COLS> const& getBase() const noexcept(true) {
		return m_presentBase;
	}
private:
	PresentBase<NUM_ROWS, NUM_COLS> const m_presentBase;
	std::bitset<BIT_COUNT> m_presents;
};

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS, std::size_t PRESENT_COUNT>
class Board {
public:
	Board(std::array<BoardPiece, NUM_ROWS * NUM_COLS> const& pieces, std::size_t const& penguinStartingPosition, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections)
		: m_pieces(pieces), m_startingPosition(penguinStartingPosition), m_holeConnections(holeConnections) {
		//
	}
	~Board() {
		//
	}

	static std::pair<Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>> fromFieldString(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections) {
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
		} else if (presentPositions.size() > PRESENT_COUNT) {
			std::cerr << "Invalid input, there are too many presents on the board!" << std::endl;
			exit(-1);
		}

		return std::make_pair(Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>(pieces, penguinPosition.value(), holeConnections), PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>(presentPositions));
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

	bool canMoveUp(std::size_t const& pos, std::size_t& target, bool& wasTorusMove) const {
		wasTorusMove = false;
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if (pos < NUM_COLS) {
			if (IS_TORUS) {
				wasTorusMove = true;
				target = pos + ((NUM_ROWS - 1) * NUM_COLS);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosAbove(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	bool canMoveDown(std::size_t const& pos, std::size_t& target, bool& wasTorusMove) const {
		wasTorusMove = false;
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if (pos >= ((NUM_ROWS - 1) * NUM_COLS)) {
			if (IS_TORUS) {
				wasTorusMove = true;
				target = pos - ((NUM_ROWS - 1) * NUM_COLS);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosBelow(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	bool canMoveLeft(std::size_t const& pos, std::size_t& target, bool& wasTorusMove) const {
		wasTorusMove = false;
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if ((pos % NUM_COLS) == 0) {
			if (IS_TORUS) {
				wasTorusMove = true;
				target = pos + (NUM_COLS - 1);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosLeft(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	bool canMoveRight(std::size_t const& pos, std::size_t& target, bool& wasTorusMove) const {
		wasTorusMove = false;
		if (pos >= (NUM_ROWS * NUM_COLS)) throw;
		if ((pos % NUM_COLS) == (NUM_COLS - 1)) {
			if (IS_TORUS) {
				wasTorusMove = true;
				target = pos - (NUM_COLS - 1);
				return isPieceNotSolid(getPieceAt(target));
			}
			return false;
		}
		target = getPosRight(pos);
		return isPieceNotSolid(getPieceAt(target));
	}

	std::size_t moveUp(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		static bool wasTorusMove = false;
		while (canMoveUp(pos, result, wasTorusMove)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY || wasTorusMove) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveDown(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		static bool wasTorusMove = false;
		while (canMoveDown(pos, result, wasTorusMove)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY || wasTorusMove) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveLeft(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		static bool wasTorusMove = false;
		while (canMoveLeft(pos, result, wasTorusMove)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY || wasTorusMove) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveRight(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		static bool wasTorusMove = false;
		while (canMoveRight(pos, result, wasTorusMove)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY || wasTorusMove) {
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

template <std::size_t PRESENT_COUNT>
class QueueObject {
public:
	QueueObject(std::size_t const& penguinPosition, std::bitset<PRESENT_COUNT> const& presentState) : m_pos(penguinPosition), m_presentState(presentState), m_moves("") {
		//
	}

	QueueObject(std::size_t const& pos, std::bitset<PRESENT_COUNT> const& presentState, std::string&& moves) : m_pos(pos), m_presentState(presentState), m_moves(moves) {
		//
	}

	inline std::size_t getPos() const noexcept(true) {
		return m_pos;
	}

	inline std::bitset<PRESENT_COUNT> getPresentState() const noexcept(true) {
		return m_presentState;
	}

	inline std::string const& getMoves() const noexcept(true) {
		return m_moves;
	}

	QueueObject moveTo(std::size_t const& newPos, std::bitset<PRESENT_COUNT> const& presentState, char direction) const {
		return QueueObject(newPos, presentState, m_moves + direction);
	}
private:
	std::size_t const m_pos;
	std::bitset<PRESENT_COUNT> const m_presentState;
	std::string const m_moves;
};

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS, std::size_t PRESENT_COUNT>
void play(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections) {
	auto const init = Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>::fromFieldString(fieldString, holeConnections);
	Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT> board = init.first;
	PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> presentOverlay = init.second;

	std::queue<QueueObject<PRESENT_COUNT>> penguinPositions;
	std::vector<Trie<PRESENT_COUNT>> knownPositions;
	
	knownPositions.clear();
	for (std::size_t i = 0; i < NUM_ROWS * NUM_COLS; ++i) {
		knownPositions.push_back(Trie<PRESENT_COUNT>());
	}

	knownPositions[board.getPenguinStartingPosition()].insertValue(presentOverlay.getRepresentation());
	penguinPositions.push(QueueObject<PRESENT_COUNT>(board.getPenguinStartingPosition(), presentOverlay.getRepresentation()));

	std::size_t newPos;
	std::size_t target;
	bool wasTorusMove;

	std::size_t roundCounter = 0;
	while (!penguinPositions.empty()) {
		++roundCounter;
		QueueObject<PRESENT_COUNT> const& p = penguinPositions.front();

		if (board.getPieceAt(p.getPos()) == BoardPiece::TARGET) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			std::cout << "Found target with " << localOverlay.getPresentsLeft() << " presents left using moves '" << p.getMoves() << "'." << std::endl;
			if (localOverlay.getPresentsLeft() == 0) {
				return;
			} else {
				penguinPositions.pop();
				continue;
			}
		}

		if (board.canMoveUp(p.getPos(), target, wasTorusMove)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveUp(p.getPos(), localOverlay);
			
			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'U'));
			}
		}
		if (board.canMoveDown(p.getPos(), target, wasTorusMove)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveDown(p.getPos(), localOverlay);
			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'D'));
			}
		}
		if (board.canMoveLeft(p.getPos(), target, wasTorusMove)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveLeft(p.getPos(), localOverlay);
			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'L'));
			}
		}
		if (board.canMoveRight(p.getPos(), target, wasTorusMove)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveRight(p.getPos(), localOverlay);
			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'R'));
			}
		}

		penguinPositions.pop();
	}
}

int main(int argc, char* argv[]) {
	std::cout << "c't, Puzzle 19/2023" << std::endl;
	auto const beginTotal = std::chrono::steady_clock::now();

	if (false) {
		play<20, 20, false, 1>(fieldStringBasic, holeConnectionsBasic);
	} else {
		play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas);
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
