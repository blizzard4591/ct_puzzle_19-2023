#ifndef PLAY_H_
#define PLAY_H_

#include <array>
#include <cstdint>
#include <iomanip>
#include <string>
#include <vector>

#include "Trie.h"

#include <fstream>

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

	// Current Min
	std::size_t currentMinPresentsLeft = std::numeric_limits<std::size_t>::max();
	std::string currentMinPresentsLeftMoves = "";

	// Only print every Nth target, if it is not a record
	std::size_t const everyNthTarget = 250;
	std::size_t const everyNthTargetBackup = 100000;
	std::size_t targetCounter = 1;

	auto const beginSearch = std::chrono::steady_clock::now();

	std::size_t roundCounter = 0;
	while (!penguinPositions.empty()) {
		++roundCounter;
		QueueObject<PRESENT_COUNT> const& p = penguinPositions.front();

		if (board.getPieceAt(p.getPos()) == BoardPiece::TARGET) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			bool isNewRecord = false;
			if (currentMinPresentsLeft > localOverlay.getPresentsLeft()) {
				currentMinPresentsLeft = localOverlay.getPresentsLeft();
				currentMinPresentsLeftMoves = p.getMoves();
				isNewRecord = true;
			}
			
			if (isNewRecord || (targetCounter % everyNthTarget == 0)) {
				auto const currentSearch = std::chrono::steady_clock::now();
				auto const us = std::chrono::duration_cast<std::chrono::microseconds>(currentSearch - beginSearch).count();
				double const speedTarget = static_cast<double>(us) / static_cast<double>(targetCounter);
				double const speedRound = static_cast<double>(us) / static_cast<double>(roundCounter);

				std::cout << "Found target #" << targetCounter << " with " << localOverlay.getPresentsLeft() << " presents left using moves '" << p.getMoves() << "' - current best is " << currentMinPresentsLeft << " with moves '" << currentMinPresentsLeftMoves << "', stack has " << penguinPositions.size() << " entries.";
				std::cout << std::setprecision(2) << speedTarget << " us/T, " << std::setprecision(2) << speedRound << " us/R" << std::endl;
			}
			if (isNewRecord || (targetCounter % everyNthTargetBackup == 0)) {
				auto const beginBackup = std::chrono::steady_clock::now();
				std::ofstream os("state_" + std::to_string(targetCounter) + ".bin", std::ios::binary);
				cereal::BinaryOutputArchive archive(os); // Create an output archive
				archive(currentMinPresentsLeft, currentMinPresentsLeftMoves, targetCounter, roundCounter, penguinPositions, knownPositions);
				auto const endBackup = std::chrono::steady_clock::now();
				std::cout << "Made a state backup at #" << targetCounter << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(endBackup - beginBackup).count() << "." << std::endl;
			}

			++targetCounter;
			
			if (localOverlay.getPresentsLeft() == 0) {
				std::cout << "Terminating search, found a solution collecting all presents: " << p.getMoves() << std::endl;
				return;
			} else {
				penguinPositions.pop();
				continue;
			}
		}

		if (board.canMoveUp(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveUp(p.getPos(), localOverlay);

			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'U'));
			}
		}
		if (board.canMoveDown(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveDown(p.getPos(), localOverlay);
			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'D'));
			}
		}
		if (board.canMoveLeft(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveLeft(p.getPos(), localOverlay);
			if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
				knownPositions[newPos].insertValue(localOverlay.getRepresentation());
				penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), 'L'));
			}
		}
		if (board.canMoveRight(p.getPos(), target)) {
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


#endif
