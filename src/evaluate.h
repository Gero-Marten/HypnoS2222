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

#ifndef EVALUATE_H_INCLUDED
#define EVALUATE_H_INCLUDED

#include <string>

#include "types.h"

namespace Stockfish {

class Position;

namespace Eval {

std::string trace(Position& pos);

Value evaluate(const Position& pos);

extern std::string currentEvalFileName[2];

// The default net name MUST follow the format nn-[SHA256 first 12 digits].nnue
// for the build process (profile-build and fishtest) to work. Do not change the
// name of the macro, as it is used in the Makefile.
#define EvalFileDefaultNameBig   "nn-0000000000a0.nnue"
#define EvalFileDefaultNameSmall "nn-ecb35f70ff2a.nnue"

namespace NNUE {

extern int MaterialisticEvaluationStrategy;
extern int PositionalEvaluationStrategy;
void init();
void verify();

}  // namespace NNUE

}  // namespace Eval

}  // namespace Stockfish

#endif  // #ifndef EVALUATE_H_INCLUDED
