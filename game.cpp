//
// Created by roman on 2025-11-22.
//
#include "game.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace {

const std::array<std::array<Shape,4>,Game::PIECE_COUNT> SHAPES = {{
    {{ // I
        {{ {0,1},{1,1},{2,1},{3,1} }},
        {{ {2,0},{2,1},{2,2},{2,3} }},
        {{ {0,2},{1,2},{2,2},{3,2} }},
        {{ {1,0},{1,1},{1,2},{1,3} }}
    }},
    {{ // J
        {{ {0,0},{0,1},{1,1},{2,1} }},
        {{ {1,0},{2,0},{1,1},{1,2} }},
        {{ {0,1},{1,1},{2,1},{2,2} }},
        {{ {1,0},{1,1},{0,2},{1,2} }}
    }},
    {{ // L
        {{ {2,0},{0,1},{1,1},{2,1} }},
        {{ {1,0},{1,1},{1,2},{2,2} }},
        {{ {0,1},{1,1},{2,1},{0,2} }},
        {{ {0,0},{1,0},{1,1},{1,2} }}
    }},
    {{ // O
        {{ {1,0},{2,0},{1,1},{2,1} }},
        {{ {1,0},{2,0},{1,1},{2,1} }},
        {{ {1,0},{2,0},{1,1},{2,1} }},
        {{ {1,0},{2,0},{1,1},{2,1} }}
    }},
    {{ // S
        {{ {1,0},{2,0},{0,1},{1,1} }},
        {{ {1,0},{1,1},{2,1},{2,2} }},
        {{ {1,1},{2,1},{0,2},{1,2} }},
        {{ {0,0},{0,1},{1,1},{1,2} }}
    }},
    {{ // T
        {{ {1,0},{0,1},{1,1},{2,1} }},
        {{ {1,0},{1,1},{2,1},{1,2} }},
        {{ {0,1},{1,1},{2,1},{1,2} }},
        {{ {1,0},{0,1},{1,1},{1,2} }}
    }},
    {{ // Z
        {{ {0,0},{1,0},{1,1},{2,1} }},
        {{ {2,0},{1,1},{2,1},{1,2} }},
        {{ {0,1},{1,1},{1,2},{2,2} }},
        {{ {1,0},{0,1},{1,1},{0,2} }}
    }},
    {{ // Dot
        {{ {0,0} }},
        {{ {0,0} }},
        {{ {0,0} }},
        {{ {0,0} }}
    }}
}};

std::chrono::milliseconds compute_drop_interval(int level) {
    int base = 500;   // ms
    int step = 40;    // уменьшение на уровень
    int min_ms = 80;  // нижний предел
    int ms = base - (level - 1) * step;
    if (ms < min_ms) ms = min_ms;
    return std::chrono::milliseconds(ms);
}

} // namespace

void Game::init(){
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // базовые цвета (по умолчанию, если пользователь не задаст свои)
    colors[0] = 0x00ffff;
    colors[1] = 0x0000ff;
    colors[2] = 0xffa500;
    colors[3] = 0xffff00;
    colors[4] = 0x00ff00;
    colors[5] = 0xa000a0;
    colors[6] = 0xff0000;
    colors[7] = 0xc8c8c8;

    for(size_t i=0;i<pieces.size();++i){
        pieces[i].rot = SHAPES[i];
        pieces[i].color = colors[i];
    }

    reset();
}

void Game::reset(){
    field = {};
    over = false;
    paused = false;
    px = 3; py = 0; pr = 0;

    score = 0;
    level = 1;
    total_lines_cleared = 0;

    cur = rand_piece();
    nxt = rand_piece();

    update_drop_interval();
}

void Game::update_drop_interval(){
    drop_ms = compute_drop_interval(level);
}

int Game::rand_piece() const {
    return std::rand() % PIECE_COUNT;
}

bool Game::collides(int nx,int ny,int nr) const{
    for(auto v: pieces[cur].rot[nr]){
        int gx = nx + v.x;
        int gy = ny + v.y;
        if(gx < 0 || gx >= W || gy < 0 || gy >= H) return true;
        if(field[gy][gx]) return true;
    }
    return false;
}

int Game::clear_lines(){
    int cleared = 0;
    for(int r = H-1; r >= 0; --r){
        bool full = true;
        for(int c = 0; c < W; ++c){
            if(!field[r][c]){ full = false; break; }
        }
        if(full){
            for(int rr = r; rr > 0; --rr) field[rr] = field[rr-1];
            field[0].fill(0);
            ++cleared;
            ++r; // пересканировать ту же строку после сдвига
        }
    }
    return cleared;
}

void Game::lock_piece(){
    for(auto v: pieces[cur].rot[pr]){
        int gx = px + v.x;
        int gy = py + v.y;
        if(gy >= 0 && gy < H && gx >= 0 && gx < W)
            field[gy][gx] = cur + 1;
    }

    int lines = clear_lines();
    if(lines > 0){
        static const int base_scores[5] = {0,100,300,500,800};
        if(lines <= 4){
            score += base_scores[lines] * level;
        } else {
            score += 1200 * level;
        }
        total_lines_cleared += lines;

        int new_level = 1 + total_lines_cleared / 10;
        if(new_level != level){
            level = new_level;
            update_drop_interval();
        }
    }

    px = 3; py = 0; pr = 0;
    cur = nxt;
    nxt = rand_piece();
    if(collides(px,py,pr)){
        over = true;
        paused = false;
    }
}

void Game::try_move(int dx,int dy,int dr){
    int nr = (pr + dr + 4) % 4;
    if(!collides(px+dx, py+dy, nr)){
        px += dx;
        py += dy;
        pr = nr;
    }
}

void Game::hard_drop(){
    while(!collides(px,py+1,pr))
        py++;
    lock_piece();
}