#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <csignal>
#include <initializer_list>

// Blocks sigs for as long as it lives, then puts the old mask back.
class BlockSignals {
  public:
    explicit BlockSignals(std::initializer_list<int> sigs);
    ~BlockSignals();
    BlockSignals(const BlockSignals&) = delete;
    BlockSignals& operator=(const BlockSignals&) = delete;

    // The mask from before.
    const sigset_t& previous() const {
        return old_;
    }

  private:
    sigset_t old_;
};

// Whether sig has been sent but is blocked, so not delivered yet.
bool is_pending(int sig);

// Sleeps until one of sigs arrives, and returns it. One that arrived
// earlier, while blocked, counts. Doesn't spin, puts the mask and the
// signals' actions back the way they were, and never loses a wakeup.
int await_signal(std::initializer_list<int> sigs);
