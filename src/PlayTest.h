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
std::size_t playString(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections, std::string const& moves) {
	auto const init = Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>::fromFieldString(fieldString, holeConnections);
	Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT> board = init.first;
	PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> presentOverlay = init.second;

	std::size_t pos = board.getPenguinStartingPosition();

	std::size_t newPos;
	std::size_t target;

	std::size_t roundCounter = 0;
	std::cout << "Started in position X = " << getX<NUM_COLS>(pos) << ", Y = " << getY<NUM_COLS>(pos) << "." << std::endl;
	for (std::size_t i = 0; i < moves.size(); ++i) {
		if (board.getPieceAt(pos) == BoardPiece::TARGET) {
			std::cout << "Found target with " << presentOverlay.getPresentsLeft() << " presents left using moves '" << moves << "'." << std::endl;
			return roundCounter;
		}
		++roundCounter;

		char const move = moves.at(i);
		switch (move) {
			case 'U':
				if (board.canMoveInDir<Direction::UP>(pos, target)) {
					pos = board.moveInDir<Direction::UP>(pos, presentOverlay);
					std::cout << "Moved U to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move U in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			case 'D':
				if (board.canMoveInDir<Direction::DOWN>(pos, target)) {
					pos = board.moveInDir<Direction::DOWN>(pos, presentOverlay);
					std::cout << "Moved D to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move D in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			case 'L':
				if (board.canMoveInDir<Direction::LEFT>(pos, target)) {
					pos = board.moveInDir<Direction::LEFT>(pos, presentOverlay);
					std::cout << "Moved L to position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				} else {
					std::cout << "Failed to move L in position X = " << std::setw(2) << std::setfill(' ') << getX<NUM_COLS>(pos) << ", Y = " << std::setw(2) << std::setfill(' ') << getY<NUM_COLS>(pos) << "." << std::endl;
				}
				break;
			case 'R':
				if (board.canMoveInDir<Direction::RIGHT>(pos, target)) {
					pos = board.moveInDir<Direction::RIGHT>(pos, presentOverlay);
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
	return roundCounter;
}


#endif
