#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <unordered_set>
#include <queue>

#include <cereal/cereal.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>

#include "Board.h"
#include "QueueObject.h"
#include "PlayTest.h"
#include "Play.h"
#include "Trie.h"

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

//#define TEST_TWO_PRESENTS
#ifdef TEST_TWO_PRESENTS
static const std::array<std::string, 40> fieldStringChristmas = {
	"TT T#T   #T#T#T#T #   T #T#  #TT  #T   #",
	"T     #                                T",
	"T          T TT  TT##T#TT   T  T  T  T  ",
	"   T     T   T  T         T             ",
	"#T     T   O   T TT##TTTT   TT T        ",
	"    T                           T   T   ",
	"#                TT#TTTTT              T",
	"     T  T    TT                   T     ",
	" T          T    T#TT#T#T  T#          #",
	"        T    T T         #      T      O",
	"# T     #    T   T##TTTT# T T      T#T  ",
	"    T T                        #       #",
	"#          TT    T##T#TTT T   T    T T #",
	"     TTT   T   T             #   T   T T",
	"T  #         T  #T###TT#T T TT          ",
	"   ## T   T     T        T T        T T#",
	"  #T        #  T#T##TT#T#  T T    T     ",
	"T T  T T T  #                # TT T    #",
	"     T   #   T   TT#TTTTT T# T       T  ",
	"T  T       #  #                 T   TTT ",
	"   T  T      T  #T##TTT## T  T#     T   ",
	"#   T    T    T T        TTT  T TT     T",
	" T  #T    T    T TT#TT### T  T        T ",
	"# T T#    T #T            TT   T T T    ",
	"#        T       T###T#TT            T  ",
	"   T T        T        #  T     T    TT ",
	"T T T  ##   T T  T  T  T T    T  T T    ",
	"   TT  T      T T  # T T   TT        T T",
	"      #TT  T                T    #     T",
	"  T        TTT T T T  T#T           T  T",
	"T  T    T      T             T T      TT",
	"#T   T        T  T TT # TT  TT  #      #",
	"#   T    ##        TT    #  #  T# T     ",
	" T     T     T TT TT#T      T   T       ",
	" # T              T T   T  #T T #  TT T#",
	"T  T      T T #    T  T                T",
	"T      TT       #        # #  T       TT",
	"# T         T      T          # T  T   #",
	"T  O           T     T     T    T      X",
	" T #   # T    #T# #  ##   T  T  P # #TTT"
};
#else
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
#endif
static const std::vector<std::pair<std::size_t, std::size_t>> holeConnectionsChristmas = {
	{399, 1523},
	{1523, 171},
	{171, 399}
};

enum class PlayMode {
	MODE_CLASSIC,
	MODE_CHRISTMAS
};

void printHelp() {
	std::cerr << "Options:" << std::endl;
	std::cerr << "--classic: Play the game as presented in 19/2023." << std::endl;
	std::cerr << "--christmas: Play the game as presented in 28/2023." << std::endl;
	std::cerr << "--play [TURNS STRING]: Play the given turns in the selected game mode." << std::endl;
	std::cerr << "--fromBackup [FILENAME]: Loads the given file as a state backup and resumes operation from there." << std::endl;
	std::cerr << "--noBackups: Disable creation of state backups. Useful for keeping disk usage in check." << std::endl;
	std::cerr << "--deleteOldBackups: Whether to delete the preceeding state backup file when a new one has been written. Useful for keeping disk usage in check." << std::endl;
}

int main(int argc, char* argv[]) {
	std::cout << "c't, Puzzle 19/2023 + 28/2023" << std::endl;
	
	PlayMode playMode = PlayMode::MODE_CLASSIC;
	std::string turnsToPlay;
	std::string backupName;
	bool deleteOldBackups = false;
	bool noBackups = false;

	if (argc > 1) {
		for (std::size_t i = 1; i < argc; ++i) {
			bool const hasOneMore = ((i + 1) < argc);
			std::string const arg = argv[i];
			if (arg.compare("--classic") == 0) {
				playMode = PlayMode::MODE_CLASSIC;
			} else if (arg.compare("--christmas") == 0) {
				playMode = PlayMode::MODE_CHRISTMAS;
			} else if (arg.compare("--play") == 0) {
				if (!hasOneMore) {
					std::cerr << "The option '--play' expects the turns to be given, e.g. '--play ULDRULDR'!" << std::endl;
					return -1;
				}
				++i;
				turnsToPlay = argv[i];
			} else if (arg.compare("--fromBackup") == 0) {
				if (!hasOneMore) {
					std::cerr << "The option '--fromBackup' expects the filename to be given, e.g. '--fromBackup state_012345.lz4.bin'!" << std::endl;
					return -1;
				}
				++i;
				backupName = argv[i];
			} else if (arg.compare("--deleteOldBackups") == 0) {
				deleteOldBackups = true;
			} else if (arg.compare("--noBackups") == 0) {
				noBackups = true;
			} else {
				std::cerr << "Sorry, could not parse option '" << arg << "'!" << std::endl;
				printHelp();
				return -1;
			}
		}
	} else {
		printHelp();
		return 0;
	}

	auto const beginTotal = std::chrono::steady_clock::now();
	std::size_t combinations = 0;
	if (playMode == PlayMode::MODE_CLASSIC) {
		if (turnsToPlay.empty()) {
			combinations = play<20, 20, false, 0>(fieldStringBasic, holeConnectionsBasic, deleteOldBackups, noBackups, backupName);
		} else {
			combinations = playString<20, 20, false, 0>(fieldStringBasic, holeConnectionsBasic, turnsToPlay);
		}
	} else {
		if (turnsToPlay.empty()) {
			combinations = play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, deleteOldBackups, noBackups, backupName);
		} else {
			combinations = playString<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, turnsToPlay);
		}
	}

	auto const endIterate = std::chrono::steady_clock::now();
	std::cout << "Looked at " << combinations << " combinations in " << std::chrono::duration_cast<std::chrono::microseconds>(endIterate - beginTotal).count() << "us." << std::endl;

	
	std::cout << "Done!" << std::endl;
	return 0;
}

#ifdef _MSC_VER
int __stdcall WinMain(struct HINSTANCE__* hInstance, struct HINSTANCE__* hPrevInstance, char* lpszCmdLine, int nCmdShow) {
	return main(__argc, __argv);
}

#endif
