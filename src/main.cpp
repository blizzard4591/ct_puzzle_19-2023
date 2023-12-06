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

int main(int argc, char* argv[]) {
	std::cout << "c't, Puzzle 19/2023" << std::endl;
	auto const beginTotal = std::chrono::steady_clock::now();

	std::size_t combinations = 0;
	if (argc > 1) {
		if (std::filesystem::exists(argv[1])) {
			std::cout << "Loading backup '" << argv[1] << "'..." << std::endl;
			combinations = play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, argv[1]);
		} else {
			combinations = playString<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, argv[1]);
		}
	} else {
		if (false) {
			combinations = play<20, 20, false, 1>(fieldStringBasic, holeConnectionsBasic);
		} else {
			//combinations = play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, "U:\\state_1000000.lz4.bin");
			//combinations = play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, "U:\\state_1530595.lz4.bin");
			//combinations = play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas, "U:\\state_1500000.lz4.bin");
			combinations = play<40, 40, true, 24>(fieldStringChristmas, holeConnectionsChristmas);
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
