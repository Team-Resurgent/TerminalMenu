# TerminalMenu

A terminal-style menu application for the original Xbox. It renders a character buffer (40×30 grid) using Direct3D 8 with a bitmap font, supporting ASCII and Unicode box-drawing characters.

## Description

TerminalMenu displays a full-screen “terminal” on the Xbox: a fixed-size character grid (40 columns × 30 rows) drawn with the Cascadia Code font at 8px. The buffer supports writing at any (x, y), line wrapping, and scrolling up. Text is rendered in a single batched draw call for performance. The project includes an **InputManager** for reading controller input (buttons, triggers, D-pad) that can be used for menu navigation or other input handling. It uses the Xbox SDK (xtl, xgraphics), SSFN for font rasterization, and D3D8 for 2D orthographic rendering.

## Screenshot

![TerminalMenu screenshot](Images/Screenshot.png)
