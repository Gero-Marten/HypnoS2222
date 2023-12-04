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

#ifndef TIMEMAN_H_INCLUDED
#define TIMEMAN_H_INCLUDED

#include <cstdint>

#include "misc.h"
#include "search.h"
#include "thread.h"
#include "types.h"

namespace Stockfish {

// The TimeManagement class computes the optimal time to think depending on
// the maximum available time, the game move number, and other parameters.
class TimeManagement {
   public:
    void      init(Search::LimitsType& limits, Color us, int ply);
    TimePoint optimum() const { return optimumTime; }
    TimePoint maximum() const { return maximumTime; }
    TimePoint elapsed() const {
        return Search::Limits.npmsec ? TimePoint(Threads.nodes_searched()) : now() - startTime;
    }

    int64_t availableNodes;  // When in 'nodes as time' mode

   private:
    TimePoint startTime;
    TimePoint optimumTime;
    TimePoint maximumTime;
};

extern TimeManagement Time;

}  // namespace Stockfish

#endif  // #ifndef TIMEMAN_H_INCLUDED
