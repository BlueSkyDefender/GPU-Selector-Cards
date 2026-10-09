Files the DOOM card brings
==========================

deng_appfw.dll
  The Doomsday 3D Fix. It turns on convergence in Doomsday 2.3.2 (build 3869),
  so the 3D picture sits behind the screen instead of popping out.
  It also brings the 3D crosshair (laser dot) for Doomsday.

  1. GPU Selector asks before it is used.
  2. It only goes into the DOOM card's own copy of Doomsday.
  3. Doomsday's original file is kept as deng_appfw_doomsday.dll.

  Source code:
  https://github.com/BlueSkyDefender/GPU-Selector-Cards/tree/main/source/doomsday-3d-fix

  SHA256: 708afb5a0d3a80aa2706d20fc8ee462b3b1790f311d1ef9d6a066c3f95ca3122


dinput8.dll
  The 3D Laser Dot. In UZDoom, GZDoom and LZDoom's own 3D, a crosshair dot sits at
  the depth of what you aim at (like a VR laser sight).

  1. GPU Selector asks before it is used.
  2. It is added next to the Modding Codex's UZDoom, GZDoom or LZDoom. Nothing is
     replaced. A dinput8.dll that is already there is left alone.
     Steam games made with GZDoom can get it too, only if you say yes for that
     game. The trash button on the game's row takes it out again.
  3. 3D Settings on the DOOM card: 3D Crosshair (dot, circle, cross, or the
     game's own at depth), Color, Dot Size, Dot Hover.

  Source code:
  https://github.com/BlueSkyDefender/GPU-Selector-Cards/tree/main/source/laser-dot

  SHA256: f0cd8ddfc76378475211eff2716f5e38bf44bcad4fba2abeb9b34bbc029d44db

License: PolyForm Noncommercial 1.0.0 (both DLLs). See LICENSE.md in each
source folder above.
  1. Open code: read it, change it, share it.
  2. Not for commercial use.
  3. Keep the Required Notice: Copyright (C) 2026 Depth3D (BlueSkyDefender)
4. Given "as is", with no warranty. The author is not responsible for any
   damage it may cause. Use it at your own risk.
