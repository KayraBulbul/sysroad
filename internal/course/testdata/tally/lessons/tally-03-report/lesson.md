---
title: A report
project: tally
contract:
  drops: [stats, Stats]
  functions:
    - name: count_words
      sig: "int(std::string_view)"
    - name: report
      sig: "std::string(std::string_view)"
retires:
  - lesson: tally-02-lines
    tests: [stats_test.cpp]
  - lesson: tally-01-words
    cases: ["a hyphen*"]
    checks: [prints the count]
solution_removes: [src/lines.cpp]
---
Print a report, and let a hyphen split words.
