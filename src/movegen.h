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

#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include <algorithm>  // IWYU pragma: keep
#include <cstddef>

#include "types.h"

namespace Stockfish {

class Position;

enum GenType {
    CAPTURES,
    QUIETS,
    QUIET_CHECKS,
    EVASIONS,
    NON_EVASIONS,
    LEGAL
};

struct ExtMove {
    Move move;
    int  value;

    operator Move() const { return move; }
    void operator=(Move m) { move = m; }

    // Inhibit unwanted implicit conversions to Move
    // with an ambiguity that yields to a compile error.
    operator float() const = delete;
};

inline bool operator<(const ExtMove& f, const ExtMove& s) { return f.value < s.value; }

template<GenType>
ExtMove* generate(const Position& pos, ExtMove* moveList);

// The MoveList struct wraps the generate() function and returns a convenient
// list of moves. Using MoveList is sometimes preferable to directly calling
// the lower level generate() function.
template<GenType T>
struct MoveList {

    explicit MoveList(const Position& pos) :
        last(generate<T>(pos, moveList)) {}
    const ExtMove* begin() const { return moveList; }
    const ExtMove* end() const { return last; }
    size_t         size() const { return last - moveList; }
    bool           contains(Move move) const { return std::find(begin(), end(), move) != end(); }

   private:
    ExtMove moveList[MAX_MOVES], *last;
};

}  // namespace Stockfish

#endif  // #ifndef MOVEGEN_H_INCLUDED
