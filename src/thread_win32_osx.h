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

#ifndef THREAD_WIN32_OSX_H_INCLUDED
#define THREAD_WIN32_OSX_H_INCLUDED

#include <thread>

// On OSX threads other than the main thread are created with a reduced stack
// size of 512KB by default, this is too low for deep searches, which require
// somewhat more than 1MB stack, so adjust it to TH_STACK_SIZE.
// The implementation calls pthread_create() with the stack size parameter
// equal to the Linux 8MB default, on platforms that support it.

#if defined(__APPLE__) || defined(__MINGW32__) || defined(__MINGW64__) || defined(USE_PTHREADS)

    #include <pthread.h>

namespace Stockfish {

static const size_t TH_STACK_SIZE = 8 * 1024 * 1024;

template<class T, class P = std::pair<T*, void (T::*)()>>
void* start_routine(void* ptr) {
    P* p = reinterpret_cast<P*>(ptr);
    (p->first->*(p->second))();  // Call member function pointer
    delete p;
    return nullptr;
}

class NativeThread {

    pthread_t thread;

   public:
    template<class T, class P = std::pair<T*, void (T::*)()>>
    explicit NativeThread(void (T::*fun)(), T* obj) {
        pthread_attr_t attr_storage, *attr = &attr_storage;
        pthread_attr_init(attr);
        pthread_attr_setstacksize(attr, TH_STACK_SIZE);
        pthread_create(&thread, attr, start_routine<T>, new P(obj, fun));
    }
    void join() { pthread_join(thread, nullptr); }
};

}  // namespace Stockfish

#else  // Default case: use STL classes

namespace Stockfish {

using NativeThread = std::thread;

}  // namespace Stockfish

#endif

#endif  // #ifndef THREAD_WIN32_OSX_H_INCLUDED
