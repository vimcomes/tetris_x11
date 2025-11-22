//
// Created by roman on 2025-11-22.
//
#include "render.h"
#include <cstring>
#include <algorithm>
#include <cstdio>

unsigned long rgb(unsigned char r,unsigned char g,unsigned char b){
    return (r<<16)|(g<<8)|b;
}

static void draw_cell(Display* dpy, Window win, GC gc,
                      int x,int y,unsigned long color){
    XSetForeground(dpy, gc, color);
    int pxp = MARGIN + x*TILE;
    int pyp = MARGIN + y*TILE;
    XFillRectangle(dpy, win, gc, pxp, pyp, TILE-1, TILE-1);
}

static void draw_piece_at(Display* dpy, Window win, GC gc,
                          int ox,int oy,int tile,
                          unsigned long color,
                          const Piece& p,int rot){
    for(auto v: p.rot[rot]){
        XSetForeground(dpy, gc, color);
        XFillRectangle(dpy, win, gc,
                       ox + v.x*tile,
                       oy + v.y*tile,
                       tile-1, tile-1);
    }
}

static void draw_button(Display* dpy, Window win, GC gc,
                        const Rect& r, const char* label, bool active){
    unsigned long fill = active ? rgb(80,80,120) : rgb(60,60,60);
    XSetForeground(dpy, gc, fill);
    XFillRectangle(dpy, win, gc, r.x, r.y, r.w, r.h);
    XSetForeground(dpy, gc, rgb(200,200,200));
    XDrawRectangle(dpy, win, gc, r.x, r.y, r.w, r.h);
    int tx = r.x + 6;
    int ty = r.y + r.h/2 + 4;
    XDrawString(dpy, win, gc, tx, ty, label, std::strlen(label));
}

void render(Display* dpy,
            Window win,
            GC gc,
            const Game& game,
            const Rect& pause_btn,
            const Rect& exit_btn)
{
    const int board_w = Game::W*TILE + 2*MARGIN;
    const int board_h = Game::H*TILE + 2*MARGIN;

    XClearWindow(dpy, win);
    XSetForeground(dpy, gc, BG);
    XFillRectangle(dpy, win, gc, 0, 0, board_w, board_h);

    // grid
    XSetForeground(dpy, gc, GRID);
    for(int r=0;r<=Game::H;++r)
        XDrawLine(dpy, win, gc,
                  MARGIN, MARGIN + r*TILE,
                  MARGIN + Game::W*TILE, MARGIN + r*TILE);
    for(int c=0;c<=Game::W;++c)
        XDrawLine(dpy, win, gc,
                  MARGIN + c*TILE, MARGIN,
                  MARGIN + c*TILE, MARGIN + Game::H*TILE);

    // field
    for(int r=0;r<Game::H;++r)
        for(int c=0;c<Game::W;++c)
            if(game.field[r][c])
                draw_cell(dpy, win, gc, c, r,
                          game.pieces[game.field[r][c]-1].color);

    // active piece
    if(!game.over){
        for(auto v: game.pieces[game.cur].rot[game.pr])
            draw_cell(dpy, win, gc,
                      game.px + v.x,
                      game.py + v.y,
                      game.pieces[game.cur].color);
    }

    // side panel
    int panel_x = board_w;
    XSetForeground(dpy, gc, PANEL_BG);
    XFillRectangle(dpy, win, gc, panel_x, 0, PANEL_W, board_h);

    XSetForeground(dpy, gc, rgb(180,180,180));
    const char* next_label = "Next:";
    XDrawString(dpy, win, gc,
                panel_x + MARGIN, MARGIN + 12,
                next_label, std::strlen(next_label));

    // preview box 4x4
    int box_x = panel_x + MARGIN;
    int box_y = MARGIN + 18;
    int preview_tile = TILE;
    XSetForeground(dpy, gc, GRID);
    XDrawRectangle(dpy, win, gc,
                   box_x-2, box_y-2,
                   preview_tile*4+3, preview_tile*4+3);

    // center preview piece
    int minx=10, maxx=-10, miny=10, maxy=-10;
    for(auto v: game.pieces[game.nxt].rot[0]){
        minx = std::min(minx, v.x);
        maxx = std::max(maxx, v.x);
        miny = std::min(miny, v.y);
        maxy = std::max(maxy, v.y);
    }
    int pw = (maxx-minx+1)*preview_tile;
    int ph = (maxy-miny+1)*preview_tile;
    int ox = box_x + (preview_tile*4 - pw)/2 - minx*preview_tile;
    int oy = box_y + (preview_tile*4 - ph)/2 - miny*preview_tile;
    draw_piece_at(dpy, win, gc, ox, oy, preview_tile,
                  game.pieces[game.nxt].color,
                  game.pieces[game.nxt], 0);

    // score / level text
    char buf[64];
    int text_y = box_y + preview_tile*4 + 24;
    std::snprintf(buf, sizeof(buf), "Score: %d", game.score);
    XDrawString(dpy, win, gc,
                panel_x + MARGIN, text_y,
                buf, std::strlen(buf));
    text_y += 16;
    std::snprintf(buf, sizeof(buf), "Level: %d", game.level);
    XDrawString(dpy, win, gc,
                panel_x + MARGIN, text_y,
                buf, std::strlen(buf));

    // buttons (расположены ниже текста, см. main.cpp)
    draw_button(dpy, win, gc, pause_btn,
                game.paused ? "Resume" : "Pause",
                game.paused);
    draw_button(dpy, win, gc, exit_btn, "Exit", false);

    if(game.over){
        const char* msg = "Stack full! R=restart, Esc=exit";
        XSetForeground(dpy, gc, rgb(255,255,255));
        XDrawString(dpy, win, gc,
                    MARGIN, 20,
                    msg, std::strlen(msg));
    } else if(game.paused){
        const char* msg = "Paused (P or button to resume)";
        XSetForeground(dpy, gc, rgb(220,220,220));
        XDrawString(dpy, win, gc,
                    MARGIN, 20,
                    msg, std::strlen(msg));
    }

    XFlush(dpy);
}