#ifndef BOARD_H_
#define BOARD_H_

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include "BoardPiece.h"
#include "PresentOverlay.h"

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS, std::size_t PRESENT_COUNT>
class Board {
public:
	Board(std::array<BoardPiece, (NUM_ROWS * NUM_COLS)> const& pieces, std::size_t const& penguinStartingPosition, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections)
		: m_pieces(pieces), m_startingPosition(penguinStartingPosition), m_holeConnections(holeConnections) {
		//
	}
	~Board() {
		//
	}

	static std::pair<Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>> fromFieldString(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections) {
		std::array<BoardPiece, NUM_ROWS* NUM_COLS> pieces;
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

	std::size_t moveUp(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		while (canMoveUp(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveDown(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		while (canMoveDown(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveLeft(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
		while (canMoveLeft(pos, result)) {
			pos = result;
			presents.collectOn(pos);
			if (getPieceAt(pos) != BoardPiece::EMPTY) {
				break;
			}
		}
		return swapHoleIfOn(pos);
	}

	std::size_t moveRight(std::size_t pos, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT>& presents) const {
		static std::size_t result = 0;
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

	std::array<BoardPiece, NUM_ROWS* NUM_COLS> const m_pieces;
	std::size_t const m_startingPosition;
	std::vector<std::pair<std::size_t, std::size_t>> const m_holeConnections;
};

#endif
