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

#include "tune.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "uci.h"

namespace Stockfish {
enum Value : int;
}

using std::string;

namespace Stockfish {

bool                              Tune::update_on_last;
const UCI::Option*                LastOption = nullptr;
static std::map<std::string, int> TuneResults;

string Tune::next(string& names, bool pop) {

    string name;

    do
    {
        string token = names.substr(0, names.find(','));

        if (pop)
            names.erase(0, token.size() + 1);

        std::stringstream ws(token);
        name += (ws >> token, token);  // Remove trailing whitespace

    } while (std::count(name.begin(), name.end(), '(') - std::count(name.begin(), name.end(), ')'));

    return name;
}

static void on_tune(const UCI::Option& o) {

    if (!Tune::update_on_last || LastOption == &o)
        Tune::read_options();
}

static void make_option(const string& n, int v, const SetRange& r) {

    // Do not generate option when there is nothing to tune (ie. min = max)
    if (r(v).first == r(v).second)
        return;

    if (TuneResults.count(n))
        v = TuneResults[n];

    Options[n] << UCI::Option(v, r(v).first, r(v).second, on_tune);
    LastOption = &Options[n];

    // Print formatted parameters, ready to be copy-pasted in Fishtest
    std::cout << n << "," << v << "," << r(v).first << "," << r(v).second << ","
              << (r(v).second - r(v).first) / 20.0 << ","
              << "0.0020" << std::endl;
}

template<>
void Tune::Entry<int>::init_option() {
    make_option(name, value, range);
}

template<>
void Tune::Entry<int>::read_option() {
    if (Options.count(name))
        value = int(Options[name]);
}

template<>
void Tune::Entry<Value>::init_option() {
    make_option(name, value, range);
}

template<>
void Tune::Entry<Value>::read_option() {
    if (Options.count(name))
        value = Value(int(Options[name]));
}

// Instead of a variable here we have a PostUpdate function: just call it
template<>
void Tune::Entry<Tune::PostUpdate>::init_option() {}
template<>
void Tune::Entry<Tune::PostUpdate>::read_option() {
    value();
}

}  // namespace Stockfish


// Init options with tuning session results instead of default values. Useful to
// get correct bench signature after a tuning session or to test tuned values.
// Just copy fishtest tuning results in a result.txt file and extract the
// values with:
//
// cat results.txt | sed 's/^param: \([^,]*\), best: \([^,]*\).*/  TuneResults["\1"] = int(round(\2));/'
//
// Then paste the output below, as the function body


namespace Stockfish {

void Tune::read_results() { /* ...insert your values here... */
}

}  // namespace Stockfish
