# Pong Game on Arduino LED Matrix
![Pong running on the game system](Images/20231019_002221.jpg)
Recreation of Atari's *home pong* game system which plays a game of pong on a 64x32 LED matrix.  
This repository includes my schematics, PDB designs, and C++ code for the microcontrollers.
The PCB is designed for manufacturing with JLCPCB in two layers, exported files are included.

![Schematic for mainboard](Schematic.pdf)

Due to memory constraints the functions of the system are split between two microcontrollers, one of which runs the game and IO while the other drives the display.
