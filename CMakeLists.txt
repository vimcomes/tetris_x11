cmake_minimum_required(VERSION 3.16)
project(tetris LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(X11 REQUIRED)

add_executable(tetris
        main.cpp
        game.cpp
        render.cpp
        )

if (TARGET X11::X11)
    target_link_libraries(tetris PRIVATE X11::X11)
else()
    target_include_directories(tetris PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(tetris PRIVATE ${X11_LIBRARIES})
endif()