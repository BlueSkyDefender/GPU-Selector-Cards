# 3DConsoleBridge

A tool card for [3DConsoleBridge](https://github.com/NTM-3D/3DConsoleBridge) by NTM-3D: 3D games from PS3, PS4 and Xbox on a 3D display, AR glasses or an SR display.

## The three tabs

1. **3D**: installs the 3DConsoleBridge shader, ShaderGlass (by mausimus) and the EDID tools. Launch ShaderGlass starts it with ReShade.
2. **AR**: downloads and starts 3DConsoleBridge-AR.
3. **SR**: downloads and starts 3DConsoleBridge-SR.

## What to look at

1. `launch` as a list: three programs, one per tab, each with `when`.
2. `"reshade": "proxy"` and `"inApps": true` on a launch.
3. `chips`, with a tip on the PS3 chip.
4. `links`, one with a `popup` (the hardware shopping list).
5. `parts`: the "What You Need" boxes, with the state of a download and a button.
6. A tab's own `notice`, `labels` and `gradient`, and `ui.buttonPosition`, `ui.buttonWide`, `ui.background`.

## Keeping it working

1. The two EDID files come from NTM-3D's GitHub, and the card holds their SHA256. If those files change there, the 3D install stops with a hash error.
2. Then put the new SHA256 of each file into the card (`downloads`, ids `edidpatcher` and `edidbin`), run the site build and push.

This card used to be built into GPU Selector. It is in Cards on the House now, so it can change without an app update.
It needs a newer GPU Selector than 1.3.5, because it uses card features that came after it.
