#pragma once

#include <array>
#include <vector>
#include <chrono>

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
    int px = 3, py = 0, pr = 0; // позиция и ротация текущей фигуры
    int cur = 0, nxt = 0;       // текущая и следующая фигуры

    bool over = false;
    bool paused = false;

    unsigned long colors[PIECE_COUNT];
    std::array<Piece,PIECE_COUNT> pieces;

    // score / level
    int score = 0;
    int level = 1;
    int total_lines_cleared = 0;

    // скорость падения
    std::chrono::milliseconds drop_ms{500};

    // инициализация фигур и цветов
    void init();

    // сброс игры (для старта или рестарта)
    void reset();

    // пересчитать скорость падения по уровню
    void update_drop_interval();

    // логика движения / коллизий
    bool collides(int nx,int ny,int nr) const;
    void try_move(int dx,int dy,int dr);
    void hard_drop();
    void lock_piece();      // зафиксировать фигуру и посчитать очки
    int  clear_lines();     // возвращает количество очищенных линий

    int rand_piece() const;
};