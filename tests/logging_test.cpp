#    if WITH_LOG

// clang-format off
#include <catch2/catch_all.hpp>

#include <iostream>
#include <regex>



#include "utilities/strings.h"
#include "logging/logging.h"
// clang-format on

using namespace lython;

#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>

// --

void fail(int signal) {
    kwinfo(outlog(), "handling signal {}", signal);
    raise(signal);
}

std::string read(int fd) {
    Array<String> output;
    String        buffer(4096, ' ');

    while (1) {
        ssize_t count = read(fd, &buffer[0], 4096);

        if (count == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("read");
                break;
            }
        } else if (count == 0) {
            break;
        } else {
            output.push_back(buffer.substr(0, count));
        }
    }
    close(fd);
    wait(0);
    return std::string(join("", output).c_str());
}

template <class F, class... Args>
std::string CHECK_ABORT(F&& f, Args&&... args) {
    int filedes[2];
    if (pipe(filedes) == -1) {
        perror("pipe");
        throw std::runtime_error("Could not open pipe");
    }

    // spawn a new process
    auto child_pid = fork();

    // if the fork succeed
    if (child_pid >= 0) {

        // if we are in the child process
        if (child_pid == 0) {
            // close the file descriptor STDOUT_FILENO if it was previously open, then (re)open it
            // as a copy of filedes[1]
            while ((dup2(filedes[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {}
            close(filedes[1]);

            // Child not does need the output pipe
            close(filedes[0]);

            // call the function that we expect to abort
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);

            // if the function didn't abort, we'll exit cleanly
            std::exit(EXIT_SUCCESS);
        }
    }

    // Parent does not need the input pipe
    close(filedes[1]);
    std::string output = read(filedes[0]);
    close(filedes[0]);

    // determine if the child process aborted
    int exit_status;
    wait(&exit_status);

    // we check the exit status instead of a signal interrupt, because
    // Catch is going to catch the signal and exit with an error
    bool aborted = WEXITSTATUS(exit_status);

    return output;
}

bool check_signal(int signal, std::string const& value) {
    std::regex regex(value);

    auto output = CHECK_ABORT(fail, signal);

    kwinfo(outlog(), "{}", output);

    return std::regex_search(output, regex);
}

TEST_CASE("Cehck Signal Handlers") {
#    if WITH_LOG
    CHECK(check_signal(SIGINT, fmt::format("Received signal {} >>>", SIGINT)));
    CHECK(check_signal(SIGSEGV, fmt::format("Received signal {} >>>", SIGSEGV)));
    CHECK(check_signal(SIGTERM, fmt::format("Received signal {} >>>", SIGTERM)));
#    endif
}

#endif

TEST_CASE("Log API") {
    Logger logger("name");
    auto output = new_output<Stdout>();
    logger.add_output(output);

    for(int j = 0; j < int(LogLevel::All); j++) {
        logger.log(LogLevel(j), LOC, "{} + {}", 2, 3);

        for(int i = 0; i < 10; i++) {
            logger.logtrace<true>(LogLevel(j), LOC, i, "{} + {}", 2, 3);
        }
        for(int i = 0; i < 10; i++) {
            logger.logtrace<false>(LogLevel(j), LOC, 9 - i, "{} + {}", 2, 3);
        }
    }
}

#endif