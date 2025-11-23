#pragma once

#include <array>
#include <vector>
#include <chrono>
#include <random>

struct Vec { int x, y; };
using Shape = std::vector<Vec>;

struct Piece {
    std::array<Shape,4> rot;
    unsigned long color{};
};

struct Game {
    static constexpr int W = 12;
    static constexpr int H = 24;
    static constexpr int PIECE_COUNT = 8;

    using Field = std::array<std::array<int,W>,H>;

    Field field{};
    int px = 3, py = 0, pr = 0; // active piece position + rotation
    int cur = 0, nxt = 0;       // current and next piece ids

    bool over = false;
    bool paused = false;
    bool show_ghost = true;

    unsigned long colors[PIECE_COUNT];
    std::array<Piece,PIECE_COUNT> pieces;

    // randomization (7-bag)
    std::mt19937 rng;
    std::vector<int> bag;
    size_t bag_pos = 0;

    struct ClearedRow {
        int row;
        std::array<int,W> data;
    };
    std::vector<ClearedRow> cleared_rows;
    std::chrono::steady_clock::time_point flash_until{};
    bool flashing = false;

    // score / level
    int score = 0;
    int level = 1;
    int total_lines_cleared = 0;

    // fall speed
    std::chrono::milliseconds drop_ms{500};

    // setup pieces and default colors
    void init();

    // reset game state
    void reset();

    // recalc drop speed by level
    void update_drop_interval();

    // movement/collision helpers
    bool collides(int nx,int ny,int nr) const;
    void try_move(int dx,int dy,int dr);
    void hard_drop();
    void lock_piece();      // fix current piece and score
    int  clear_lines();     // returns number of cleared rows
    int  next_piece();
    void refill_bag();
};
