#pragma once
// One item in a shop's stock list.
//
// The fields are right, but they're in the wrong order. Put them in the
// order the compiled code in the listing expects.

struct Item {
    int id;
    short shelf;
    char tag;
    long price;
    int count;
};
