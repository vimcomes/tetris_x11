#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <chrono>
#include <ctime>

#include "game.h"
#include "render.h"

int main(){
    Game game;
    game.init();

    Display* dpy = XOpenDisplay(nullptr);
    if(!dpy) return 1;
    int screen = DefaultScreen(dpy);

    int board_w = Game::W*TILE + 2*MARGIN;
    int board_h = Game::H*TILE + 2*MARGIN;
    int width   = board_w + PANEL_W;
    int height  = board_h;

    Window win = XCreateSimpleWindow(
        dpy, RootWindow(dpy,screen),
        200, 100, width, height, 1,
        BlackPixel(dpy,screen),
        WhitePixel(dpy,screen)
    );
    XStoreName(dpy, win, "Tetris (Xlib)");

    long mask = ExposureMask | KeyPressMask | ButtonPressMask | StructureNotifyMask;
    XSelectInput(dpy, win, mask);
    Atom wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(dpy, win, &wm_delete, 1);
    XMapWindow(dpy, win);
    GC gc = XCreateGC(dpy, win, 0, nullptr);

    Rect pause_btn{}, exit_btn{};
    int btn_w = PANEL_W - 2*MARGIN;
    int btn_h = 28;
    int btn_x = board_w + MARGIN;
    int preview_bottom = MARGIN + 18 + TILE*4;
    // ниже текста Score/Level
    int btn_y1 = preview_bottom + 60;
    pause_btn = {btn_x, btn_y1, btn_w, btn_h};
    exit_btn  = {btn_x, btn_y1 + btn_h + 8, btn_w, btn_h};

    using clock = std::chrono::steady_clock;
    auto last_drop = clock::now();

    while(true){
        while(XPending(dpy)){
            XEvent e;
            XNextEvent(dpy, &e);
            if(e.type == Expose){
                render(dpy, win, gc, game, pause_btn, exit_btn);
            } else if(e.type == ClientMessage){
                if((Atom)e.xclient.data.l[0] == wm_delete)
                    goto end;
            } else if(e.type == KeyPress){
                KeySym ks = XLookupKeysym(&e.xkey,0);
                if(ks == XK_Escape) goto end;

                if(game.over && ks == XK_r){
                    game.reset();
                    render(dpy, win, gc, game, pause_btn, exit_btn);
                    continue;
                }
                if(game.over) continue;

                if(ks == XK_p){
                    game.paused = !game.paused;
                    render(dpy, win, gc, game, pause_btn, exit_btn);
                    continue;
                }
                if(game.paused) continue;

                if(ks == XK_Left){
                    game.try_move(-1,0,0);
                } else if(ks == XK_Right){
                    game.try_move(1,0,0);
                } else if(ks == XK_Up){
                    game.try_move(0,0,1);
                } else if(ks == XK_Down){
                    if(!game.collides(game.px,game.py+1,game.pr)){
                        game.py++;
                        last_drop = clock::now();
                    } else {
                        int old_level = game.level;
                        game.lock_piece();
                        if(game.level != old_level)
                            game.update_drop_interval();
                    }
                } else if(ks == XK_space){
                    int old_level = game.level;
                    game.hard_drop();
                    if(game.level != old_level)
                        game.update_drop_interval();
                }
                render(dpy, win, gc, game, pause_btn, exit_btn);
            } else if(e.type == ButtonPress){
                int mx = e.xbutton.x;
                int my = e.xbutton.y;
                auto inside = [](const Rect& r,int x,int y){
                    return x>=r.x && x<=r.x+r.w && y>=r.y && y<=r.y+r.h;
                };
                if(inside(exit_btn, mx,my)) goto end;
                if(inside(pause_btn, mx,my)){
                    game.paused = !game.paused;
                    render(dpy, win, gc, game, pause_btn, exit_btn);
                }
            }
        }

        auto now = clock::now();
        if(!game.over && !game.paused &&
           now - last_drop >= game.drop_ms)
        {
            if(!game.collides(game.px,game.py+1,game.pr)){
                game.py++;
            } else {
                int old_level = game.level;
                game.lock_piece();
                if(game.level != old_level)
                    game.update_drop_interval();
            }
            last_drop = now;
            render(dpy, win, gc, game, pause_btn, exit_btn);
        }

        struct timespec ts{0, 2'000'000};
        nanosleep(&ts, nullptr);
    }

end:
    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}