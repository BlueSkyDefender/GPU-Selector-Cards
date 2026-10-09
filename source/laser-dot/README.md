# 3D Laser Dot

A small DLL that puts a crosshair dot at the depth of what you aim at, in UZDoom, GZDoom and LZDoom's own 3D (`vr_mode`). Doomsday gets the same crosshair from its own [3D Fix](../doomsday-3d-fix/). Like the laser sight in VR ports. The [DOOM card](../../cards/4-expert/doom/) brings it.

## The problem

1. UZDoom, GZDoom and LZDoom draw the crosshair flat on the screen.
2. In 3D it floats in front of everything, in the wrong place.
3. They also draw sprites (monsters, items) without depth.

## What this DLL does

1. It is a stand-in `dinput8.dll` next to the port's program. Every DirectInput call goes to Windows' own `dinput8.dll`.
2. The port asks Windows for its OpenGL functions by name. The DLL hands it its own versions of a few of them. Nothing in the port is changed.
3. When the port finishes each eye's picture, the DLL reads the depth under the aim point.
4. It draws a small dot there in each eye, at that depth, floating a little in front. Or a circle, a cross, or it moves the game's own crosshair there.
5. In 3D only, it keeps depth on for the port's sprite pass, so the dot lands on monsters and items too.
6. With the dot, circle or cross, it hides the game's own crosshair in 3D by itself, so the port's crosshair settings are never changed.
7. It shows only while you play (the port hides the mouse pointer), not in menus.
8. It is not an `opengl32.dll`, so it works next to mods that bring their own.
9. With the game's own crosshair, it also follows mods that move or grow it (like Precise Crosshair).
10. It is only for the ports the Modding Codex installs. Steam games don't get it: GPU Selector never puts files in their folders.

## Settings (the DOOM card sets these)

| Switch | What it does |
|---|---|
| `-laserdot-circle` | A mini circle instead of the dot |
| `-laserdot-cross` | A small cross instead of the dot |
| `-laserdot-game` | No dot: the game's own crosshair moves to the depth instead |
| `-laserdot-color=RRGGBB` | Its color (red/cyan glasses show it as grey) |
| `-laserdot-off` | The DLL does nothing |
| `-laserdot-hide` | No dot, and the game's own crosshair is hidden in 3D |
| `-laserdot-size=N` | Size in percent, 50 to 200 |
| `-laserdot-hover=N` | How far it floats in front, in percent, 0 to 300 |

It also reads the port's own 3D settings from the command line: `+vr_mode`, `+vr_ipd`, `+vr_screendist`, `+vr_hunits_per_meter`, `+vr_swap_eyes`, `+fov`.

## Files

| File | What it is |
|---|---|
| `laser_dot.cpp` | The DLL's code. Written for GPU Selector. |
| `dinput8.def` | The 6 DirectInput function names it passes on. |
| `build.bat` | Builds the DLL. |

## Build it

You need Visual Studio 2022 with C++ (any edition).

1. Run `build.bat`.
2. It writes `dinput8.dll`.
3. It is built the same way every time (`/Brepro`), so its SHA256 matches the one in the DOOM card:
   `e5988374e1a8f0dcbe5f24a5df3713fe18461ab72b98d41580b53d9820ba4e2e`

## Safety

1. GPU Selector only uses it after you agree, once.
2. It only goes next to the port the Modding Codex installed. If a `dinput8.dll` is already there, it is left alone.
3. None of UZDoom's or GZDoom's code is in it. It uses their public 3D math (`hw_vrmodes.cpp`) as a reference.

## License

[PolyForm Noncommercial 1.0.0](LICENSE.md). In short:

1. The code is open: you can read it, change it and share it.
2. Not for commercial use.
3. Keep the Required Notice when you share it or something made from it:
   `Required Notice: Copyright (C) 2026 Depth3D (BlueSkyDefender) (https://github.com/BlueSkyDefender)`
4. Given "as is", with no warranty. The author is not responsible for any damage it may cause. Use it at your own risk.

The rest of this repository (the cards, the site, the tools) stays BSD Zero Clause.
