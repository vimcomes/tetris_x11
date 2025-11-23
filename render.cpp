//
// Created by roman on 2025-11-22.
//
#include "render.h"
#include <cstring>
#include <algorithm>
#include <cstdio>
#include <unordered_map>
#include <chrono>

unsigned long rgb(unsigned char r,unsigned char g,unsigned char b){
    return (r<<16)|(g<<8)|b;
}

static unsigned long alloc_color(Display* dpy, unsigned long rgb24){
    static std::unordered_map<unsigned long, unsigned long> cache;
    auto it = cache.find(rgb24);
    if(it != cache.end()) return it->second;

    XColor xc{};
    xc.red   = ((rgb24 >> 16) & 0xff) * 257;
    xc.green = ((rgb24 >> 8)  & 0xff) * 257;
    xc.blue  = ((rgb24)       & 0xff) * 257;
    Colormap cmap = DefaultColormap(dpy, DefaultScreen(dpy));
    if(XAllocColor(dpy, cmap, &xc)){
        cache[rgb24] = xc.pixel;
        return xc.pixel;
    }
    cache[rgb24] = rgb24;
    return rgb24;
}

static void draw_cell(Display* dpy, Drawable drw, GC gc,
                      int x,int y,unsigned long color){
    XSetForeground(dpy, gc, color);
    int pxp = MARGIN + x*TILE;
    int pyp = MARGIN + y*TILE;
    XFillRectangle(dpy, drw, gc, pxp, pyp, TILE-1, TILE-1);
}

static void draw_piece_at(Display* dpy, Drawable drw, GC gc,
                          int ox,int oy,int tile,
                          unsigned long color,
                          const Piece& p,int rot){
    for(auto v: p.rot[rot]){
        XSetForeground(dpy, gc, color);
        XFillRectangle(dpy, drw, gc,
                       ox + v.x*tile,
                       oy + v.y*tile,
                       tile-1, tile-1);
    }
}

static void draw_button(Display* dpy, Drawable drw, GC gc,
                        const Rect& r, const char* label, bool active){
    unsigned long fill = active ? rgb(80,80,120) : rgb(60,60,60);
    XSetForeground(dpy, gc, fill);
    XFillRectangle(dpy, drw, gc, r.x, r.y, r.w, r.h);
    XSetForeground(dpy, gc, rgb(200,200,200));
    XDrawRectangle(dpy, drw, gc, r.x, r.y, r.w, r.h);
    int tx = r.x + 6;
    int ty = r.y + r.h/2 + 4;
    XDrawString(dpy, drw, gc, tx, ty, label, std::strlen(label));
}

static void draw_checkbox(Display* dpy, Drawable drw, GC gc,
                          const Rect& r, const char* label, bool checked,
                          unsigned long text_color){
    unsigned long bg = rgb(50,50,50);
    unsigned long border = rgb(90,90,90);
    unsigned long tick = rgb(200,200,200);
    XSetForeground(dpy, gc, bg);
    XFillRectangle(dpy, drw, gc, r.x, r.y, r.w, r.h);
    XSetForeground(dpy, gc, border);
    XDrawRectangle(dpy, drw, gc, r.x, r.y, r.w, r.h);

    int box = 16;
    int bx = r.x + 6;
    int by = r.y + (r.h - box)/2;
    XDrawRectangle(dpy, drw, gc, bx, by, box, box);
    if(checked){
        // simple X mark
        XSetForeground(dpy, gc, tick);
        XDrawLine(dpy, drw, gc, bx+3, by+3, bx+box-3, by+box-3);
        XDrawLine(dpy, drw, gc, bx+3, by+box-3, bx+box-3, by+3);
    }
    XSetForeground(dpy, gc, text_color);
    int tx = bx + box + 6;
    int ty = r.y + r.h/2 + 4;
    XDrawString(dpy, drw, gc, tx, ty, label, std::strlen(label));
}

void render(Display* dpy,
            Window win,
            GC gc,
            const Game& game,
            const Rect& pause_btn,
            const Rect& exit_btn,
            const Rect& ghost_btn)
{
    const int board_w = Game::W*TILE + 2*MARGIN;
    const int board_h = Game::H*TILE + 2*MARGIN;
    const int total_w = board_w + PANEL_W;
    int depth = DefaultDepth(dpy, DefaultScreen(dpy));
    Pixmap back = XCreatePixmap(dpy, win, total_w, board_h, depth); // simple double buffer to avoid flicker

    unsigned long col_bg    = alloc_color(dpy, BG);
    unsigned long col_grid  = alloc_color(dpy, GRID);
    unsigned long col_panel = alloc_color(dpy, PANEL_BG);
    unsigned long col_text  = alloc_color(dpy, rgb(180,180,180));
    unsigned long col_btn   = alloc_color(dpy, rgb(200,200,200));

    XSetForeground(dpy, gc, col_bg);
    XFillRectangle(dpy, back, gc, 0, 0, board_w, board_h);

    // grid
    XSetForeground(dpy, gc, col_grid);
    for(int r=0;r<=Game::H;++r)
        XDrawLine(dpy, back, gc,
                  MARGIN, MARGIN + r*TILE,
                  MARGIN + Game::W*TILE, MARGIN + r*TILE);
    for(int c=0;c<=Game::W;++c)
        XDrawLine(dpy, back, gc,
                  MARGIN + c*TILE, MARGIN,
                  MARGIN + c*TILE, MARGIN + Game::H*TILE);

    // field
    for(int r=0;r<Game::H;++r)
        for(int c=0;c<Game::W;++c)
            if(game.field[r][c])
                draw_cell(dpy, back, gc, c, r,
                          alloc_color(dpy, game.pieces[game.field[r][c]-1].color));

    // ghost piece
    if(!game.over && game.show_ghost){
        int gy = game.py;
        while(!game.collides(game.px, gy+1, game.pr))
            ++gy;
        if(gy > game.py){
            unsigned long ghost_px = alloc_color(dpy, rgb(80,80,80));
            char dashes[] = {4,4};
            XSetLineAttributes(dpy, gc, 1, LineOnOffDash, CapButt, JoinMiter);
            XSetDashes(dpy, gc, 0, dashes, 2);
            XSetForeground(dpy, gc, ghost_px);
            for(auto v: game.pieces[game.cur].rot[game.pr]){
                int gx = game.px + v.x;
                int gy_cell = gy + v.y;
                int pxp = MARGIN + gx*TILE;
                int pyp = MARGIN + gy_cell*TILE;
                XDrawRectangle(dpy, back, gc, pxp, pyp, TILE-2, TILE-2);
            }
            // restore solid lines
            XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
        }
    }

    // flash rows if any were just cleared
    auto now = std::chrono::steady_clock::now();
    bool flash_active = game.flashing && now < game.flash_until;
    bool flash_on = flash_active &&
        ((std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() / 80) % 2 == 0);
    if(flash_on){
        char dashes[] = {3,3};
        XSetLineAttributes(dpy, gc, 1, LineOnOffDash, CapButt, JoinMiter);
        XSetDashes(dpy, gc, 0, dashes, 2);
        for(const auto& cr : game.cleared_rows){
            int base_y = MARGIN + cr.row * TILE;
            for(int c=0;c<Game::W;++c){
                if(cr.data[c] == 0) continue;
                unsigned long col = alloc_color(dpy, game.pieces[cr.data[c]-1].color);
                XSetForeground(dpy, gc, col);
                int x = MARGIN + c * TILE;
                XDrawRectangle(dpy, back, gc, x, base_y, TILE-2, TILE-2);
            }
        }
        XSetLineAttributes(dpy, gc, 0, LineSolid, CapButt, JoinMiter);
    }

    // active piece
    if(!game.over){
        for(auto v: game.pieces[game.cur].rot[game.pr])
            draw_cell(dpy, back, gc,
                      game.px + v.x,
                      game.py + v.y,
                      alloc_color(dpy, game.pieces[game.cur].color));
    }

    // side panel
    int panel_x = board_w;
    XSetForeground(dpy, gc, col_panel);
    XFillRectangle(dpy, back, gc, panel_x, 0, PANEL_W, board_h);

    XSetForeground(dpy, gc, col_text);
    const char* next_label = "Next:";
    XDrawString(dpy, back, gc,
                panel_x + MARGIN, MARGIN + 12,
                next_label, std::strlen(next_label));

    // preview box 4x4
    int box_x = panel_x + MARGIN;
    int box_y = MARGIN + 18;
    int preview_tile = TILE;
    XSetForeground(dpy, gc, GRID);
    XDrawRectangle(dpy, back, gc,
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
    draw_piece_at(dpy, back, gc, ox, oy, preview_tile,
                  game.pieces[game.nxt].color,
                  game.pieces[game.nxt], 0);

    // score / level text
    char buf[64];
    int text_y = box_y + preview_tile*4 + 24;
    std::snprintf(buf, sizeof(buf), "Score: %d", game.score);
    XDrawString(dpy, back, gc,
                panel_x + MARGIN, text_y,
                buf, std::strlen(buf));
    text_y += 16;
    std::snprintf(buf, sizeof(buf), "Level: %d", game.level);
    XDrawString(dpy, back, gc,
                panel_x + MARGIN, text_y,
                buf, std::strlen(buf));

// buttons (placed below score block, see main.cpp)
    draw_button(dpy, back, gc, pause_btn,
                game.paused ? "Resume" : "Pause",
                game.paused);
    draw_button(dpy, back, gc, exit_btn, "Exit", false);

    // options separator + ghost toggle under buttons
    int sep_y = ghost_btn.y - 30;
    XSetForeground(dpy, gc, col_grid);
    XDrawLine(dpy, back, gc, panel_x + MARGIN, sep_y, panel_x + PANEL_W - MARGIN, sep_y);
    XSetForeground(dpy, gc, col_text);
    const char* opt = "Options";
    XDrawString(dpy, back, gc, panel_x + MARGIN, sep_y + 16, opt, std::strlen(opt));
    draw_checkbox(dpy, back, gc, ghost_btn, "Ghost", game.show_ghost, col_btn);

    if(game.over){
        const char* msg = "Stack full! R=restart, Esc=exit";
        XSetForeground(dpy, gc, alloc_color(dpy, rgb(255,255,255)));
        XDrawString(dpy, back, gc,
                    MARGIN, 20,
                    msg, std::strlen(msg));
    } else if(game.paused){
        const char* msg = "Paused (P or button to resume)";
        XSetForeground(dpy, gc, alloc_color(dpy, rgb(220,220,220)));
        XDrawString(dpy, back, gc,
                    MARGIN, 20,
                    msg, std::strlen(msg));
    }

    // blit back buffer
    XCopyArea(dpy, back, win, gc, 0, 0, total_w, board_h, 0, 0);
    XFreePixmap(dpy, back);
    XFlush(dpy);
}
