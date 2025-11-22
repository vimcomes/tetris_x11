#pragma once

#include "game.h"
#include <X11/Xlib.h>

struct Rect { int x,y,w,h; };

// некоторые константы рендера (не логики)
constexpr int TILE   = 24;
constexpr int MARGIN = 4;
constexpr int PANEL_W = 6*TILE + 2*MARGIN;

constexpr unsigned long BG       = 0x1c1c1c;
constexpr unsigned long GRID     = 0x303030;
constexpr unsigned long PANEL_BG = 0x111111;

unsigned long rgb(unsigned char r,unsigned char g,unsigned char b);

// отрисовка одного кадра
void render(Display* dpy,
            Window win,
            GC gc,
            const Game& game,
            const Rect& pause_btn,
            const Rect& exit_btn);