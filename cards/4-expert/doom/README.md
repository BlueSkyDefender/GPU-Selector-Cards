# DOOM

**Level 4: Expert.** DOOM mods, map packs and the original games, ready to play. Also Heretic, Hexen and Strife.

## Load it

1. Download `doom_card_package.zip` from the [website](https://blueskydefender.github.io/GPU-Selector-Cards/cards.html).
2. In GPU Selector: **Mods** tab, **+ Add Mod**, **Load from File**, pick the `.zip`.
3. GPU Selector tells you the card brings two DLLs (the Doomsday 3D Fix and the 3D Crosshair) and asks before they are used.

## What to look at

| Part | Field |
|---|---|
| The list of mods, with categories | `catalog.items`, `catalog.categories` |
| Source ports from the Modding Codex | `catalog.ports` |
| A port the card installs itself (Doomsday) | `"own": true` with `"download"` |
| A DLL the card brings for that port | `"fix"` and the `doom_card_files` folder |
| A DLL added next to a port's program, with its older versions | `"fix"` with `"adds"` and `"oldVersions"` |
| That DLL for a Steam game too (asked once per game) | `"fixFrom"` on the Steam game |
| The 3D Crosshair dot as a crosshair choice | `"crosshairDot"` on the port |
| A game whose 3D is coming later | `"stereoSoon"` |
| Steam games made with DOOM's engine | `catalog.steamGames` |
| Tabs at the top (1. Your Games ... 4. Options) | `ui.modes` |
| A font file in the package, and a GPU Selector theme | `ui.font` (`file` + `sha256`), `ui.appTheme` |
| A guide that opens in the browser | `ui.guide` (`file` + `sha256`) |
| Mixing: one game, one gameplay mod, add-ons | `mixRole`, `addonFor`, `addonSlot`, `mixTc`, `conflictsWith`, `stacks` |
| Ready-made mixes | `catalog.mixes` |
| Adult mods behind Parental Settings | `"adult": true` |
| A free stand-in for the game files (Freedoom) | `catalog.freeData` |
| Searching the idgames archive | (on by default for DOOM cards) |

## Files

| File | What it is |
|---|---|
| `doom_card.json` | The card. |
| `doom_card_files/deng_appfw.dll` | The Doomsday 3D Fix. Its source is in [source/doomsday-3d-fix](../../../source/doomsday-3d-fix/). |
| `doom_card_files/dinput8.dll` | The 3D Crosshair. Its source is in [source/laser-dot](../../../source/laser-dot/). |
| `doom_card_files/README.txt` | What the two DLLs are, and their license. |
| `doom_card_files/FreedoomMenu.woff2` | The card's font. Checked by its SHA256 when the card loads. |
| `doom_card_files/COPYING-Freedoom.txt` | The font's license. |
| `doom_card_files/GUIDE.md` | The card's guide (the **Guide** button). Checked by its SHA256 when the card loads. |

The `.zip` package is made from these files by `tools/build_site.py`.

## Credits

1. The mods, ports and games belong to their makers. The card only links to them.
2. The card's font comes from the [Freedoom](https://freedoom.github.io) project and keeps its own license.
