//
// Created by roman on 2025-11-22.
//
#include "game.h"
#include <algorithm>
#include <random>

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
    int step = 40;    // decrease per level
    int min_ms = 80;  // lower bound
    int ms = base - (level - 1) * step;
    if (ms < min_ms) ms = min_ms;
    return std::chrono::milliseconds(ms);
}

} // namespace

void Game::init(){
    std::random_device rd;
    rng.seed(rd());

    // default piece colors (can be overridden later if needed)
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

    refill_bag();
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

    bag_pos = 0;
    refill_bag();
    cur = next_piece();
    nxt = next_piece();

    update_drop_interval();
}

void Game::update_drop_interval(){
    drop_ms = compute_drop_interval(level);
}

void Game::refill_bag() {
    bag.resize(PIECE_COUNT);
    for(int i=0;i<PIECE_COUNT;++i) bag[i] = i;
    std::shuffle(bag.begin(), bag.end(), rng);
    bag_pos = 0;
}

int Game::next_piece() {
    if(bag.empty() || bag_pos >= bag.size()){
        refill_bag();
    }
    return bag[bag_pos++];
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
    cleared_rows.clear();

    std::vector<int> full_rows;
    for(int r = H-1; r >= 0; --r){
        bool full = true;
        for(int c = 0; c < W; ++c){
            if(!field[r][c]){ full = false; break; }
        }
        if(full){
            full_rows.push_back(r);
            cleared_rows.push_back(ClearedRow{r, field[r]});
        }
    }
    if(full_rows.empty()) return 0;

    Field new_field{};
    int dest = H-1;
    for(int r = H-1; r >= 0; --r){
        if(std::find(full_rows.begin(), full_rows.end(), r) != full_rows.end())
            continue;
        new_field[dest--] = field[r];
    }
    field = new_field;
    return static_cast<int>(full_rows.size());
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
        static const int base_scores[5] = {0,100,300,700,1200}; // tuned rewards per line count
        int capped = std::min(lines, 4);
        score += base_scores[capped] * level;
        total_lines_cleared += lines;
        flashing = true;
        flash_until = std::chrono::steady_clock::now() + std::chrono::milliseconds(240);

        int new_level = 1 + total_lines_cleared / 10;
        if(new_level != level){
            level = new_level;
            update_drop_interval();
        }
    }
    if(lines == 0){
        flashing = false;
        cleared_rows.clear();
    }

    px = 3; py = 0; pr = 0;
    cur = nxt;
    nxt = next_piece();
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
}
