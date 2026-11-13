#include "doctest.h"

#include <fstream>

#include "conch_pty.hpp"

TEST_CASE("Ctrl-C at the prompt doesn't end the shell") {
    Conch c;
    REQUIRE(c.prompt());
    c.send("\x03");
    usleep(100'000);
    c.line("echo here");
    CHECK_MESSAGE(c.expect("here"), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("bg carries a stopped job on in the background") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 0.6");
    usleep(150'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 0.6"), c.output());
    REQUIRE(c.prompt());
    c.line("bg");
    CHECK_MESSAGE(c.expect("[1] sleep 0.6 &"), c.output());
    REQUIRE(c.prompt());
    usleep(800'000);
    c.line("");
    CHECK_MESSAGE(c.expect("[1] Done  sleep 0.6"), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("a whole pipeline stops, and comes back") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5 | sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE_MESSAGE(c.expect("[1] Stopped  sleep 5 | sleep 5"), c.output());
    REQUIRE(c.prompt());
    c.line("fg %1");
    REQUIRE(c.expect("sleep 5 | sleep 5"));
    usleep(200'000);
    c.send("\x03");
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("jobs");
    c.line("echo end");
    REQUIRE(c.expect("end"));
    CHECK(c.output().find("Stopped", c.output().rfind("jobs")) == std::string::npos);
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("a background job that reads the terminal is stopped") {
    {
        std::ofstream("reader.sh") << "read x\necho got $x\n";
    }
    Conch c;
    REQUIRE(c.prompt());
    c.line("sh reader.sh &");
    REQUIRE(c.prompt());
    usleep(300'000);
    c.line("jobs");
    CHECK_MESSAGE(c.expect("[1] Stopped  sh reader.sh"), c.output());
    REQUIRE(c.prompt());
    c.line("fg");
    REQUIRE(c.expect("sh reader.sh"));
    usleep(200'000);
    c.line("typed");
    CHECK_MESSAGE(c.expect("got typed"), c.output());
    REQUIRE_MESSAGE(c.prompt(), c.output());
    c.line("exit");
    CHECK(c.wait() == 0);
}

TEST_CASE("exit warns once about stopped jobs") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("sleep 5");
    usleep(200'000);
    c.send("\x1a");
    REQUIRE(c.expect("Stopped"));
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK_MESSAGE(c.expect("there are stopped jobs"), c.output());
    REQUIRE(c.prompt());
    c.line("exit");
    CHECK(c.wait() == 1);  // $? from the first exit, which refused
}

TEST_CASE("the terminal's settings come back after a job") {
    Conch c;
    REQUIRE(c.prompt());
    c.line("stty -echo");
    REQUIRE(c.prompt());
    c.line("echo visible");
    CHECK_MESSAGE(c.expect("echo visible"), "the job turned echo off, and it stayed off");
    c.line("exit");
    CHECK(c.wait() == 0);
}
