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

//Definition of input features HalfKAv2_hm of NNUE evaluation function

#include "half_ka_v2_hm.h"

#include "../../bitboard.h"
#include "../../position.h"
#include "../../types.h"
#include "../nnue_common.h"

namespace Stockfish::Eval::NNUE::Features {

  IndexType FeatureIndexTable[COLOR_NB][SQUARE_NB][PIECE_NB][SQUARE_NB];

#define incEnum(t, v) v = t(int(v)+1)

  void init() {
    for (Color Perspective = WHITE; Perspective <= BLACK; incEnum(Color, Perspective))
      for (Square s = SQ_A1; s < SQUARE_NB; incEnum(Square, s))
      {
        for (Piece pc = W_PAWN; pc < PIECE_NB; incEnum(Piece, pc))
        {
          for (Square ksq = SQ_A1; ksq < SQUARE_NB; incEnum(Square, ksq))
          {
            FeatureIndexTable[Perspective][s][pc][ksq] = HalfKAv2_hm::make_index_not_cached(Perspective, s, pc, ksq);
          }
        }
      }
  }

#undef incEnum

  inline IndexType HalfKAv2_hm::make_index_not_cached(Color Perspective, Square s, Piece pc, Square ksq) {
    return IndexType((int(s) ^ OrientTBL[Perspective][ksq]) + PieceSquareIndex[Perspective][pc] + KingBuckets[Perspective][ksq]);
  }

// Index of a feature for a given king position and another piece on some square
template<Color Perspective>
inline IndexType HalfKAv2_hm::make_index(Square s, Piece pc, Square ksq) {
            return FeatureIndexTable[Perspective][s][pc][ksq];
}

// Get a list of indices for active features
template<Color Perspective>
void HalfKAv2_hm::append_active_indices(const Position& pos, IndexList& active) {
    Square   ksq = pos.square<KING>(Perspective);
    Bitboard bb  = pos.pieces();
    while (bb)
    {
        Square s = pop_lsb(bb);
        active.push_back(make_index<Perspective>(s, pos.piece_on(s), ksq));
    }
}

// Explicit template instantiations
template void HalfKAv2_hm::append_active_indices<WHITE>(const Position& pos, IndexList& active);
template void HalfKAv2_hm::append_active_indices<BLACK>(const Position& pos, IndexList& active);

// Get a list of indices for recently changed features
template<Color Perspective>
void HalfKAv2_hm::append_changed_indices(Square            ksq,
                                         const DirtyPiece& dp,
                                         IndexList&        removed,
                                         IndexList&        added) {
    for (int i = 0; i < dp.dirty_num; ++i)
    {
        if (dp.from[i] != SQ_NONE)
            removed.push_back(make_index<Perspective>(dp.from[i], dp.piece[i], ksq));
        if (dp.to[i] != SQ_NONE)
            added.push_back(make_index<Perspective>(dp.to[i], dp.piece[i], ksq));
    }
}

// Explicit template instantiations
template void HalfKAv2_hm::append_changed_indices<WHITE>(Square            ksq,
                                                         const DirtyPiece& dp,
                                                         IndexList&        removed,
                                                         IndexList&        added);
template void HalfKAv2_hm::append_changed_indices<BLACK>(Square            ksq,
                                                         const DirtyPiece& dp,
                                                         IndexList&        removed,
                                                         IndexList&        added);

int HalfKAv2_hm::update_cost(const StateInfo* st) { return st->dirtyPiece.dirty_num; }

int HalfKAv2_hm::refresh_cost(const Position& pos) { return pos.count<ALL_PIECES>(); }

bool HalfKAv2_hm::requires_refresh(const StateInfo* st, Color perspective) {
    return st->dirtyPiece.piece[0] == make_piece(perspective, KING);
}

}  // namespace Stockfish::Eval::NNUE::Features
