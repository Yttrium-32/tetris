# Tetris


## Introduction

A very simple implementation of Tetris written in C using [SDL](https://www.libsdl.org/),
mostly for learning purposes. Made by following [this](https://youtu.be/kh3rkt6nZ2c) excellent video.


## Compiling

This project depends on SDL for the graphics and bear to generate the
compilation cache.
These packages are available on most package managers.

For example on Arch linux:
```
# pacman -S --needed sdl2 sdl2_ttf bear
```

To compile and run simple run `make` in the project root.


## Controls

<div align="center">

| Keys | Action |
| :--- | :--- |
| <kbd>Space</kbd> | Start Game |
| <kbd>Space</kbd> | Hard Drop |
| <kbd>↓</kbd> | Soft Drop |
| <kbd>←</kbd> | Move Piece Left |
| <kbd>→</kbd> | Move Piece Right |
| <kbd>↑</kbd> | Rotate Piece |
| <kbd>r</kbd> | Restart |
| <kbd>Esc</kbd> | Exit |

</div>

