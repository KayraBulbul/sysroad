#pragma once
// Shared by your code and the tests. You won't need to change this file.

#include <sys/types.h>

#include <csignal>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "proc.hpp"

// Collects children the moment they end, from a SIGCHLD handler, so none
// is left a zombie. Only one may exist at a time.
class Reaper {
  public:
    // Installs the SIGCHLD handler, with SA_RESTART.
    Reaper();
    // Puts the old SIGCHLD action back.
    ~Reaper();
    Reaper(const Reaper&) = delete;
    Reaper& operator=(const Reaper&) = delete;

    // Starts argv like start() does, and counts it as running. The child
    // runs with this process's signal mask from before the call.
    pid_t start(const std::vector<std::string>& argv);

    // The children that have ended since the last call, in the order the
    // handler reaped them.
    std::vector<std::pair<pid_t, Status>> finished();

    // How many started children haven't ended yet.
    size_t running();

    // Sleeps until every started child has ended.
    void wait_all();

  private:
    // Moves what the handler collected into done_. Call it only with
    // SIGCHLD blocked.
    void collect();

    std::set<pid_t> running_;
    std::vector<std::pair<pid_t, Status>> done_;
    struct sigaction old_;
};
