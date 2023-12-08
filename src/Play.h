#ifndef PLAY_H_
#define PLAY_H_

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>

#include "Trie.h"

#include "lz4_stream.h"

inline bool ends_with(std::string const& value, std::string const& ending) {
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

template<std::size_t PRESENT_COUNT>
void checkForFinalStates(std::vector<Trie<PRESENT_COUNT>> const& knownPositions) {
	if constexpr(PRESENT_COUNT > 0) {
		auto const allZeroBitset = std::bitset<PRESENT_COUNT>();
		std::size_t totalStates = 0;
		std::size_t statesWithAllPresents = 0;
		for (auto itO = knownPositions.cbegin(); itO != knownPositions.cend(); ++itO) {
			Trie<PRESENT_COUNT> const& trie = *itO;
			++totalStates;
			if (trie.hasValueOrSubsetThereof(allZeroBitset)) {
				++statesWithAllPresents;
			}
		}
		std::cout << "We have " << statesWithAllPresents << " of " << totalStates << " states visited with all presents taken." << std::endl;
	}
}

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, std::size_t PRESENT_COUNT>
inline void updateStack(std::vector<Trie<PRESENT_COUNT>>& knownPositions, std::queue<QueueObject<PRESENT_COUNT>>& penguinPositions, QueueObject<PRESENT_COUNT> const& p, PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> const& localOverlay, std::size_t const& newPos, char direction) {
	if (!knownPositions[newPos].hasValueOrSubsetThereof(localOverlay.getRepresentation())) {
		knownPositions[newPos].insertValue(localOverlay.getRepresentation());
		penguinPositions.push(p.moveTo(newPos, localOverlay.getRepresentation(), direction));
	}
}

template<std::size_t NUM_ROWS, std::size_t NUM_COLS, bool IS_TORUS, std::size_t PRESENT_COUNT>
std::size_t play(std::array<std::string, NUM_ROWS> const& fieldString, std::vector<std::pair<std::size_t, std::size_t>> const& holeConnections, bool deleteOldBackups, bool noBackups, std::string const& stateFilename = "") {
	auto const init = Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT>::fromFieldString(fieldString, holeConnections);
	Board<NUM_ROWS, NUM_COLS, IS_TORUS, PRESENT_COUNT> board = init.first;
	PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> presentOverlay = init.second;

	std::queue<QueueObject<PRESENT_COUNT>> penguinPositions;
	std::vector<Trie<PRESENT_COUNT>> knownPositions;

	knownPositions.clear();

	std::size_t newPos;
	std::size_t target;

	// Current Min
	std::size_t currentMinPresentsLeft = std::numeric_limits<std::size_t>::max();
	std::string currentMinPresentsLeftMoves = "";

	// Only print every Nth target, if it is not a record
	std::size_t const everyNthTarget = 250;
	std::size_t const everyNthTargetBackup = 100000;
	std::size_t targetCounter = 1;
	std::string lastBackupFilename = "";
	std::size_t roundCounter = 0;

	if (!stateFilename.empty() && std::filesystem::exists(stateFilename)) {
		auto const beginBackupLoad = std::chrono::steady_clock::now();

		std::ifstream is(stateFilename, std::ios::binary);
		lz4_stream::istream compressedStream(is);
		cereal::BinaryInputArchive archive(compressedStream);
		archive(currentMinPresentsLeft, currentMinPresentsLeftMoves, targetCounter, roundCounter, penguinPositions, knownPositions);

		auto const endBackupLoad = std::chrono::steady_clock::now();
		std::cout << "Loaded state backup at #" << targetCounter << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(endBackupLoad - beginBackupLoad).count() << " ms, stack has " << penguinPositions.size() << " elements." << std::endl;
		lastBackupFilename = stateFilename;
	} else {
		for (std::size_t i = 0; i < NUM_ROWS * NUM_COLS; ++i) {
			knownPositions.push_back(Trie<PRESENT_COUNT>());
		}

		knownPositions[board.getPenguinStartingPosition()].insertValue(presentOverlay.getRepresentation());
		penguinPositions.push(QueueObject<PRESENT_COUNT>(board.getPenguinStartingPosition(), presentOverlay.getRepresentation()));
	}

	auto const beginSearch = std::chrono::steady_clock::now();
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

				std::cout << "Found target #" << targetCounter << " with " << localOverlay.getPresentsLeft() << "/" << localOverlay.getBase().getTotalPresentCount() << " presents left using moves '" << p.getMoves() << "' - current best is " << currentMinPresentsLeft << "/" << localOverlay.getBase().getTotalPresentCount() << " with moves '" << currentMinPresentsLeftMoves << "', stack has " << penguinPositions.size() << " entries. ";
				std::cout << std::setprecision(6) << speedTarget << " us/T, " << std::setprecision(6) << speedRound << " us/R" << std::endl;
			}
			if ((!noBackups) && (isNewRecord || (targetCounter % everyNthTargetBackup == 0))) {
				std::string const backupFilename = "state_" + std::to_string(targetCounter) + "_" + std::to_string(NUM_ROWS) + "_" + std::to_string(NUM_COLS) + "_" + std::to_string(IS_TORUS) + "_" + std::to_string(PRESENT_COUNT) + ".lz4.bin";
				// In case we just restored from this backup
				if (!ends_with(lastBackupFilename, backupFilename)) {
					auto const beginBackup = std::chrono::steady_clock::now();
					std::ofstream os(backupFilename, std::ios::binary);
					lz4_stream::ostream compressedStream(os);
					cereal::BinaryOutputArchive archive(compressedStream); // Create an output archive
					archive(currentMinPresentsLeft, currentMinPresentsLeftMoves, targetCounter, roundCounter, penguinPositions, knownPositions);
					auto const endBackup = std::chrono::steady_clock::now();
					std::cout << "Made a state backup at #" << targetCounter << " in " << std::chrono::duration_cast<std::chrono::milliseconds>(endBackup - beginBackup).count() << " ms." << std::endl;
					if (deleteOldBackups && !lastBackupFilename.empty()) {
						if (std::filesystem::remove(lastBackupFilename)) {
							std::cout << "Deleted last backup '" << lastBackupFilename << "'." << std::endl;
						} else {
							std::cerr << "Failed to delete last backup '" << lastBackupFilename << "'!" << std::endl;
						}
					}
					lastBackupFilename = backupFilename;
				}
			}

			++targetCounter;
			
			if (localOverlay.getPresentsLeft() == 0) {
				std::cout << "Terminating search, found a solution collecting all presents: " << p.getMoves() << std::endl;
				return roundCounter;
			} else {
				penguinPositions.pop();
				continue;
			}
		}

		if (board.canMoveInDir<Direction::UP>(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveInDir<Direction::UP>(p.getPos(), localOverlay);

			updateStack(knownPositions, penguinPositions, p, localOverlay, newPos, 'U');
		}
		if (board.canMoveInDir<Direction::DOWN>(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveInDir<Direction::DOWN>(p.getPos(), localOverlay);
			updateStack(knownPositions, penguinPositions, p, localOverlay, newPos, 'D');
		}
		if (board.canMoveInDir<Direction::LEFT>(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveInDir<Direction::LEFT>(p.getPos(), localOverlay);
			updateStack(knownPositions, penguinPositions, p, localOverlay, newPos, 'L');
		}
		if (board.canMoveInDir<Direction::RIGHT>(p.getPos(), target)) {
			PresentOverlay<NUM_ROWS, NUM_COLS, PRESENT_COUNT> localOverlay(presentOverlay.getBase(), p.getPresentState());
			newPos = board.moveInDir<Direction::RIGHT>(p.getPos(), localOverlay);
			updateStack(knownPositions, penguinPositions, p, localOverlay, newPos, 'R');
		}

		penguinPositions.pop();
	}

	std::cout << "Oh - no more states to explore - maybe there is no solution?" << std::endl;
	return roundCounter;
}


#endif
