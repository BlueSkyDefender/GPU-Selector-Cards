# Doomsday 3D Fix

A small DLL that makes 3D work properly in the [Doomsday Engine](https://dengine.net/) 2.3.2 (build 3869). The [DOOM card](../../cards/4-expert/doom/) brings it.

## The problem

1. Doomsday has its own 3D modes (side by side, top and bottom, anaglyph).
2. But its convergence code is switched off in its source (`#if 0` in `vrconfig.cpp`).
3. So both eyes look straight ahead, and the whole picture comes out of the screen.

## What this DLL does

1. It takes the place of Doomsday's `deng_appfw.dll`.
2. Doomsday's original file is only renamed to `deng_appfw_doomsday.dll`. It is never changed.
3. Every function (1,537 of them) is passed straight on to the original.
4. Two functions are answered by this DLL: the 3D view for each eye (`VRConfig::projectionMatrix`) and the current eye (`VRConfig::currentEye`, passed on too, it only tells the crosshair when an eye is done).
5. It adds convergence: a point at the screen distance lands in the same place in both eyes. Things farther away go into the screen, closer things come out.
6. **3D crosshair:** a small dot (or circle, or cross) sits on what you aim at, at its depth, like a VR laser sight. It reads the depth under the aim point when an eye is done, and draws the dot at the start of that eye's next view, so closer things still cover it. Use it with Doomsday's own crosshair off (`view-cross-type 0`; the DOOM card does this).
7. **Doomsday's own crosshair at depth:** with `-laserdot-game` and `-laserdot-ddtype=1-5` it draws Doomsday's own crosshair shape instead (the five from Doomsday's `g_game.cpp`), with its size, angle, line width and color (`-laserdot-ddsize`, `-laserdot-ddangle`, `-laserdot-ddwidth`, `-laserdot-ddcolor`). The DOOM card reads these from Doomsday's own `view-cross-*` settings.

## How it is set

1. **Depth:** Doomsday's `rend-vr-ipd` and `rend-vr-player-height`.
2. **Screen distance (ZPD):** `rend-vr-hud-distance`, read in meters. Screen distance in map units = meters x map units per meter.
3. GPU Selector's DOOM card sets these for you from its 3D Settings.
4. **3D crosshair** (command line): `-laserdot-circle`, `-laserdot-cross`, `-laserdot-off`, `-laserdot-color=RRGGBB`, `-laserdot-size=N` (50-200 %), `-laserdot-hover=N` (0-300 %). The DOOM card sets these from 3D Crosshair, Color, Dot Size and Dot Hover.

## Files

| File | What it is |
|---|---|
| `stereo_projection.cpp` | The DLL's code (projection and current eye). Written for GPU Selector. |
| `crosshair.cpp`, `crosshair.h` | The 3D crosshair. Written for GPU Selector. |
| `exports.txt` | The 1,539 function names Doomsday's `deng_appfw.dll` has, with their numbers. |
| `make_forwarders.py` | Makes the list that passes every function but one on to the original. |
| `build.bat` | Builds the DLL. |

## Build it

You need:

1. Visual Studio 2022 with C++ (any edition).
2. Python 3.

Then:

1. Run `build.bat`.
2. The DLL is written to `build\deng_appfw.dll`.
3. It is built the same way every time (`/Brepro`), so its SHA256 matches the one in the DOOM card:
   `708afb5a0d3a80aa2706d20fc8ee462b3b1790f311d1ef9d6a066c3f95ca3122`

## Safety

1. GPU Selector only uses it after you agree, once.
2. It only goes into the DOOM card's own copy of Doomsday, and only over the exact original file (checked by SHA256).
3. It uses only Doomsday's public function names. None of Doomsday's code is in it.

## License

[PolyForm Noncommercial 1.0.0](LICENSE.md). In short:

1. The code is open: you can read it, change it and share it.
2. Not for commercial use.
3. Keep the Required Notice when you share it or something made from it:
   `Required Notice: Copyright (C) 2026 Depth3D (BlueSkyDefender) (https://github.com/BlueSkyDefender)`
4. Given "as is", with no warranty. The author is not responsible for any damage it may cause. Use it at your own risk.

The rest of this repository (the cards, the site, the tools) stays BSD Zero Clause.
