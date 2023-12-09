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

#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

#include "movepick.h"
#include "position.h"
#include "search.h"
#include "thread_win32_osx.h"
#include "types.h"

namespace Stockfish {

// Thread class keeps together all the thread-related stuff.
class Thread {

    std::mutex              mutex;
    std::condition_variable cv;
    size_t                  idx;
    bool                    exit = false, searching = true;  // Set before starting std::thread
    NativeThread            stdThread;

   public:
    explicit Thread(size_t);
    virtual ~Thread();
    virtual void search();
    void         clear();
    void         idle_loop();
    void         start_searching();
    void         wait_for_search_finished();
    size_t       id() const { return idx; }

    size_t                pvIdx, pvLast;
    std::atomic<uint64_t> nodes, tbHits, bestMoveChanges;
    int                   selDepth, nmpMinPly;
    Value                 bestValue, optimism[COLOR_NB], advantage[COLOR_NB];

    Position              rootPos;
    StateInfo             rootState;
    Search::RootMoves     rootMoves;
    Depth                 rootDepth, completedDepth;
    Value                 rootDelta;
    Value                 rootSimpleEval;
    CounterMoveHistory    counterMoves;
    ButterflyHistory      mainHistory;
    CapturePieceToHistory captureHistory;
    ContinuationHistory   continuationHistory[2][2];
    PawnHistory           pawnHistory;
};


// MainThread is a derived class specific for main thread
struct MainThread: public Thread {

    using Thread::Thread;

    void search() override;
    void check_time();

    double           previousTimeReduction;
    Value            bestPreviousScore;
    Value            bestPreviousAverageScore;
    Value            iterValue[4];
    int              callsCnt;
    bool             stopOnPonderhit;
    std::atomic_bool ponder;
};


// ThreadPool struct handles all the threads-related stuff like init, starting,
// parking and, most importantly, launching a thread. All the access to threads
// is done through this class.
struct ThreadPool {

    void start_thinking(Position&, StateListPtr&, const Search::LimitsType&, bool = false);
    void clear();
    void set(size_t);

    MainThread* main() const { return static_cast<MainThread*>(threads.front()); }
    uint64_t    nodes_searched() const { return accumulate(&Thread::nodes); }
    uint64_t    tb_hits() const { return accumulate(&Thread::tbHits); }
    Thread*     get_best_thread() const;
    void        start_searching();
    void        wait_for_search_finished() const;

    std::atomic_bool stop, increaseDepth;

    auto cbegin() const noexcept { return threads.cbegin(); }
    auto begin() noexcept { return threads.begin(); }
    auto end() noexcept { return threads.end(); }
    auto cend() const noexcept { return threads.cend(); }
    auto size() const noexcept { return threads.size(); }
    auto empty() const noexcept { return threads.empty(); }

   private:
    StateListPtr         setupStates;
    std::vector<Thread*> threads;

    uint64_t accumulate(std::atomic<uint64_t> Thread::*member) const {

        uint64_t sum = 0;
        for (Thread* th : threads)
            sum += (th->*member).load(std::memory_order_relaxed);
        return sum;
    }
};

extern ThreadPool Threads;

}  // namespace Stockfish

#endif  // #ifndef THREAD_H_INCLUDED