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

#ifndef UCI_H_INCLUDED
#define UCI_H_INCLUDED

#include <cstddef>
#include <iosfwd>
#include <map>
#include <string>

#include "types.h"

namespace Stockfish {

class Position;

namespace UCI {

// Normalizes the internal value as reported by evaluate or search
// to the UCI centipawn result used in output. This value is derived from
// the win_rate_model() such that Stockfish outputs an advantage of
// "100 centipawns" for a position if the engine has a 50% probability to win
// from this position in self-play at fishtest LTC time control.
const int NormalizeToPawnValue = 328;

class Option;

// Define a custom comparator, because the UCI options should be case-insensitive
struct CaseInsensitiveLess {
    bool operator()(const std::string&, const std::string&) const;
};

// The options container is defined as a std::map
using OptionsMap = std::map<std::string, Option, CaseInsensitiveLess>;

// The Option class implements each option as specified by the UCI protocol
class Option {

    using OnChange = void (*)(const Option&);

   public:
    Option(OnChange = nullptr);
    Option(bool v, OnChange = nullptr);
    Option(const char* v, OnChange = nullptr);
    Option(double v, int minv, int maxv, OnChange = nullptr);
    Option(const char* v, const char* cur, OnChange = nullptr);

    Option& operator=(const std::string&);
    void    operator<<(const Option&);
    operator int() const;
    operator std::string() const;
    bool operator==(const char*) const;
  
public:
  bool isHidden = false;

   private:
    friend std::ostream& operator<<(std::ostream&, const OptionsMap&);

    std::string defaultValue, currentValue, type;
    int         min, max;
    size_t      idx;
    OnChange    on_change;
};

void        init(OptionsMap&);
void        loop(int argc, char* argv[]);
int         to_cp(Value v);
std::string value(Value v);
std::string square(Square s);
std::string move(Move m, bool chess960);
std::string pv(const Position& pos, Depth depth);
std::string wdl(Value v, int ply);
Move        to_move(const Position& pos, std::string& str);

}  // namespace UCI

extern UCI::OptionsMap Options;
//no uci options, but constants
enum {
    NODES_TIME = 0,
	SLOW_MOVER = 100,
	MOVE_OVERHEAD = 1000};
//end no uci options, but constants
}  // namespace Stockfish

#endif  // #ifndef UCI_H_INCLUDED
