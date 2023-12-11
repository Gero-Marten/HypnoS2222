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

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <iosfwd>
#include <istream>
#include <map>
#include <ostream>
#include <sstream>
#include <string>

#include "book/book.h"
#include "evaluate.h"
#include "experience.h"
#include "misc.h"
#include "search.h"
#include "syzygy/tbprobe.h"
#include "thread.h"
#include "tt.h"
#include "types.h"
#include "uci.h"

using std::string;

namespace Stockfish {

UCI::OptionsMap Options;  // Global object

namespace UCI {

// 'On change' actions, triggered by an option's value change
static void on_clear_hash(const Option&) { Search::clear(); }
static void on_hash_size(const Option& o) { TT.resize(size_t(o)); }
static void on_logger(const Option& o) { start_logger(o); }
static void on_threads(const Option& o) { Threads.set(size_t(o)); }
static void on_book(const Option& o) { Book::on_book(0, (string)o); }
static void on_tb_path(const Option& o) { Tablebases::init(o); }
static void on_exp_enabled(const Option& /*o*/) { Experience::init(); }
static void on_exp_file(const Option& /*o*/) { Experience::init(); }
static void on_eval_file(const Option&) { Eval::NNUE::init(); }
static void on_materialistic_evaluation_strategy(const Option& o) { Eval::NNUE::MaterialisticEvaluationStrategy = 10 * (int)o; }
static void on_positional_evaluation_strategy(const Option& o) { Eval::NNUE::PositionalEvaluationStrategy = 10 * (int)o; }

// Our case insensitive less() function as required by UCI protocol
bool CaseInsensitiveLess::operator()(const string& s1, const string& s2) const {

    return std::lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end(),
                                        [](char c1, char c2) { return tolower(c1) < tolower(c2); });
}


// Initializes the UCI options to their hard-coded default values
void init(OptionsMap& o) {

    constexpr int MaxHashMB = Is64Bit ? 33554432 : 2048;

    o["Debug Log File"]                      << Option("", on_logger);
    o["Threads"]                             << Option(1, 1, 1024, on_threads);
    o["Hash"]                                << Option(16, 1, MaxHashMB, on_hash_size);
    o["Clear Hash"]                          << Option(on_clear_hash);
    o["Ponder"]                              << Option(false);
    o["MultiPV"]                             << Option(1, 1, 500);
    o["UCI_Chess960"]                        << Option(false);
    o["UCI_LimitStrength"]                   << Option(false);
    o["UCI_Elo"]                             << Option(1320, 1320, 3190);
    o["UCI_ShowWDL"]                         << Option(false);
    o["CTG/BIN Book"]                        << Option("<empty>", on_book);
    o["Repertoire Width"]                    << Option(20, 1, 20);
    o["Depth Moves"]                         << Option(10, 1, 50);
    o["(CTG) Book 1 Only Green"]             << Option(true);
    o["SyzygyPath"]                          << Option("<empty>", on_tb_path);
    o["SyzygyProbeDepth"]                    << Option(1, 1, 100);
    o["Syzygy50MoveRule"]                    << Option(true);
    o["SyzygyProbeLimit"]                    << Option(7, 0, 7);
    o["Experience Enabled"]                  << Option(true, on_exp_enabled);
    o["Experience File"]                     << Option("Hypnos.exp", on_exp_file);
    o["Experience Readonly"]                 << Option(false);
    o["Experience Book"]                     << Option(false);
    o["Experience Book Best Move"]           << Option(true);
    o["Experience Book Eval Importance"]     << Option(5, 0, 10);
    o["Experience Book Min Depth"]           << Option(27, EXP_MIN_DEPTH, 64);
    o["Experience Book Max Moves"]           << Option(100, 1, 100);
    o["EvalFile"]                            << Option(EvalFileDefaultNameSmall, on_eval_file);
    // o["EvalFileSmall"]                    << Option(EvalFileDefaultNameSmall, on_eval_file);
    o["Adjust Rating Advantage"]             << Option(0, -10000, 10000);
    o["Materialistic Evaluation Strategy"]   << Option(0, -12, 12, on_materialistic_evaluation_strategy);
    o["Positional Evaluation Strategy"]      << Option(0, -12, 12, on_positional_evaluation_strategy);
    //Define hidden options
    o["Syzygy50MoveRule"].isHidden = true;
    o["(CTG) Book 1 Only Green"].isHidden = true;
    o["Experience Enabled"].isHidden = true;
    o["Experience Readonly"].isHidden = true;
    o["Experience Book"].isHidden = true;
    o["Experience Book Best Move"].isHidden = true;
}


// Used to print all the options default values in chronological
// insertion order (the idx field) and in the format defined by the UCI protocol.
std::ostream& operator<<(std::ostream& os, const OptionsMap& om) {

    for (size_t idx = 0; idx < om.size(); ++idx)
        for (const auto& it : om)
            if (it.second.idx == idx)
            {
                const Option& o = it.second;
			  if(o.isHidden)
				  break;
			  
                os << "\noption name " << it.first << " type " << o.type;

                if (o.type == "string" || o.type == "check" || o.type == "combo")
                    os << " default " << o.defaultValue;

                if (o.type == "spin")
                    os << " default " << int(stof(o.defaultValue)) << " min " << o.min << " max "
                       << o.max;

                break;
            }

    return os;
}


// Option class constructors and conversion operators

Option::Option(const char* v, OnChange f) :
    type("string"),
    min(0),
    max(0),
    on_change(f) {
    defaultValue = currentValue = v;
}

Option::Option(bool v, OnChange f) :
    type("check"),
    min(0),
    max(0),
    on_change(f) {
    defaultValue = currentValue = (v ? "true" : "false");
}

Option::Option(OnChange f) :
    type("button"),
    min(0),
    max(0),
    on_change(f) {}

Option::Option(double v, int minv, int maxv, OnChange f) :
    type("spin"),
    min(minv),
    max(maxv),
    on_change(f) {
    defaultValue = currentValue = std::to_string(v);
}

Option::Option(const char* v, const char* cur, OnChange f) :
    type("combo"),
    min(0),
    max(0),
    on_change(f) {
    defaultValue = v;
    currentValue = cur;
}

Option::operator int() const {
    assert(type == "check" || type == "spin");
    return (type == "spin" ? std::stoi(currentValue) : currentValue == "true");
}

Option::operator std::string() const {
    assert(type == "string");
    return currentValue;
}

bool Option::operator==(const char* s) const {
    assert(type == "combo");
    return !CaseInsensitiveLess()(currentValue, s) && !CaseInsensitiveLess()(s, currentValue);
}


// Inits options and assigns idx in the correct printing order

void Option::operator<<(const Option& o) {

    static size_t insert_order = 0;

    *this = o;
    idx   = insert_order++;
}


// Updates currentValue and triggers on_change() action. It's up to
// the GUI to check for option's limits, but we could receive the new value
// from the user by console window, so let's check the bounds anyway.
Option& Option::operator=(const string& v) {

    assert(!type.empty());

    if ((type != "button" && type != "string" && v.empty())
        || (type == "check" && v != "true" && v != "false")
        || (type == "spin" && (stof(v) < min || stof(v) > max)))
        return *this;

    if (type == "combo")
    {
        OptionsMap         comboMap;  // To have case insensitive compare
        string             token;
        std::istringstream ss(defaultValue);
        while (ss >> token)
            comboMap[token] << Option();
        if (!comboMap.count(v) || v == "var")
            return *this;
    }

    if (type != "button")
        currentValue = v;

    if (on_change)
        on_change(*this);

    return *this;
}

}  // namespace UCI

}  // namespace Stockfish
