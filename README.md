# Tetris X11

A small Tetris implementation in C++ using Xlib for rendering.

## Features

- Game logic is separated from rendering:
    - `game.h` / `game.cpp` — board, pieces, rotations, collisions, line clearing, scoring, levels.
    - `render.h` / `render.cpp` — drawing with Xlib (board, pieces, next piece preview, side panel).
    - `main.cpp` — X11 initialization, main game loop, event handling.
- Supports:
    - score;
    - levels (fall speed increases with level);
    - next piece preview;
    - pause (button and `P` key);
    - hard drop with `Space`.

## Controls

Keyboard:
- `Left` / `Right` — move piece horizontally.
- `Up` — rotate piece.
- `Down` — soft drop (faster fall).
- `Space` — hard drop (instant fall).
- `P` — pause / resume.
- `R` — restart after game over.
- `Esc` — quit.

Buttons on the right panel:
- **Pause / Resume** — pause / continue the game.
- **Exit** — quit the game.

## Requirements

- C++17-compatible compiler or newer.
- X11 development headers:
    - On Debian/Ubuntu:
      ```bash
      sudo apt-get install libx11-dev
      ```
- CMake (the project is CMake-based).

## Build and Run
```bash
 git clone [https://github.com/vimcomes/tetris_x11.git](https://github.com/vimcomes/tetris_x11.git) cd tetris_x11 mkdir build cd build cmake .. cmake --build . ./tetris
```

(The executable name may differ, check `add_executable` in `CMakeLists.txt`.)

## Project Structure

- `CMakeLists.txt` — CMake configuration.
- `main.cpp` — entry point, event loop, timing.
- `game.h` / `game.cpp` — game logic:
    - `Game::field` board state,
    - pieces and rotations,
    - line clearing and scoring,
    - level system and fall interval.
- `render.h` / `render.cpp` — rendering:
    - board, grid, active piece,
    - next piece preview,
    - side panel with score, level and buttons.

## Recent Changes

- Rendering: double-buffered Xlib drawing to avoid flicker; palette allocated via `XAllocColor`; ghost piece preview with toggle.
- Gameplay: lock delay when touching the stack (including hard drop), 7-bag randomizer, tunable drop speed per level.
- UX: dashed ghost outline, options section with checkbox, line-clear flash effect, improved multi-line scoring.
- Scoring: bonuses scale with cleared lines (1/2/3/4 → 100/300/700/1200 * level).

## License

Not explicitly specified yet. If you use this code in your own projects, please link back to this repository.
