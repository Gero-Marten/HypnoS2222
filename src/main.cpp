/*
  Hypnos, a private UCI chess playing engine with derived from Stockfish NNUE.
  with a sophisticated Self-Learning system implemented and control of Evaluation Strategy.
  
  1) Materialistic Evaluation Strategy: Minimum = -12, Maximum = +12, Default = 0.
  Lower values will cause the engine assign less value to material differences between the sides.
  More values will cause the engine to assign more value to the material difference.
  
  2) Positional Evaluation Strategy: Minimum = -12, Maximum = +12, Default = 0.
  Lower values will cause the engine assign less value to positional differences between the sides.
  More values will cause the engine to assign more value to the positional difference.
  
  The NNUE evaluation was first introduced in shogi, and ported to HypnoS afterward.
  It can be evaluated efficiently on CPUs, and exploits the fact that only parts of
  the neural network need to be updated after a typical chess move.
  
  HypnoS allows to use two nets of different sizes. The second net will be smaller/faster
  and used only to lazy evaluate positions w/ high scores.
  Copyright (C) 2004-2024 The Hypnos developers (Marco Zerbinati)
*/

#include <cstddef>
#include <iostream>
#include<stdio.h>

#include "bitboard.h"
#include "evaluate.h"
#include "misc.h"
#include "position.h"
#include "search.h"
#include "thread.h"
#include "tune.h"
#include "types.h"
#include "uci.h"
#include "experience.h"
#include "book/book.h"

using namespace Stockfish;

int main(int argc, char* argv[]) {

{
std::cout << "Licence to: Marco Zerbinati" << std::endl;
}
  Utility::init(argv[0]);
  SysInfo::init();
  show_logo();

  std::cout << engine_info() << std::endl;

  CommandLine::init(argc, argv);

  std::cout
      << "Operating System (OS) : " << SysInfo::os_info() << std::endl
      << "CPU Brand             : " << SysInfo::processor_brand() << std::endl
      << "NUMA Nodes            : " << SysInfo::numa_nodes() << std::endl
      << "Cores                 : " << SysInfo::physical_cores() << std::endl
      << "Threads               : " << SysInfo::logical_cores() << std::endl
      << "Hyper-Threading       : " << SysInfo::is_hyper_threading() << std::endl
      << "L1/L2/L3 cache size   : " << SysInfo::cache_info(0) << "/" << SysInfo::cache_info(1) << "/" << SysInfo::cache_info(2) << std::endl
      << "Memory installed (RAM): " << SysInfo::total_memory() << std::endl << std::endl;

  UCI::init(Options);
  Tune::init();
  Bitboards::init();
  Position::init();
  Experience::init();
  Threads.set(size_t(Options["Threads"]));
  Search::clear(); // After threads are up
  Eval::NNUE::init();
  Book::init();

  UCI::loop(argc, argv);

  Experience::unload();
  Threads.set(0);
  return 0;
}
