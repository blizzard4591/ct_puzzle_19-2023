#ifndef PLAYTEST_H_
#define PLAYTEST_H_

#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "Trie.h"

template<std::size_t NUM_COLS>
std::size_t getX(std::size_t const& pos) {
	return pos % NUM_COLS;
}

template<std::size_t NUM_COLS>
std::size_t getY(std::size_t const& pos) {
	return pos / NUM_COLS;
}

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS, std::size_t PRESENT_COUNT>
void playString(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections, std::string const& moves) {
	auto const init = Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>::fromFieldString(fieldString, holeConnections);
	Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT> board = init.first;
	PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> presentOverlay = init.second;

	std::size_t pos = board.getPenguinStartingPosition();

	std::size_t newPos;
	std::size_t target;

	std::cout << "Started in position X = " << getX<NUM_COLS>(pos) << ", Y = " << getY<NUM_COLS>(pos) << "." << std::endl;
	for (std::size_t i = 0; i < moves.size(); ++i) {
		if (board.getPieceAt(pos) == BoardPiece::TARGET) {
			std::cout << "Found target with " << presentOverlay.getPresentsLeft() << " presents left using moves '" << moves << "'." << std::endl;
			return;
		}

		char const move = moves.at(i);
		switch (move) {
			case 'U':
				if (board.canMoveUp(pos, target)) {
					pos = board.moveUp(pos, presentOverlay);
					std::cout << "Moved U to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move U in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			case 'D':
				if (board.canMoveDown(pos, target)) {
					pos = board.moveDown(pos, presentOverlay);
					std::cout << "Moved D to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move D in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			case 'L':
				if (board.canMoveLeft(pos, target)) {
					pos = board.moveLeft(pos, presentOverlay);
					std::cout << "Moved L to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move L in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			case 'R':
				if (board.canMoveRight(pos, target)) {
					pos = board.moveRight(pos, presentOverlay);
					std::cout << "Moved R to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move R in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			default:
				std::cerr << "Invalid move '" << move << "'!" << std::endl;
				exit(-1);
		}
	}
}


#endif
