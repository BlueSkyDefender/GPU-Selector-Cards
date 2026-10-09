# Example Game Mods (a catalog for any game)

**Level 4: Expert.** A catalog card for a game that is not DOOM. Everything in it is made up (`example.com`), so copy it and change it for your own game.

## Load it

1. Download `any_game_catalog.json` from the [website](https://blueskydefender.github.io/GPU-Selector-Cards/cards.html).
2. In GPU Selector: **Mods** tab, **+ Add Mod**, **Load from File**, pick the file.
3. It loads and shows its pages. Its downloads and port are made up, so nothing installs.

## What to look at

| Part | Field |
|---|---|
| A catalog for any game (not DOOM) | `"engine": "generic"` |
| The game's own files to find | `gameData[].files`, `steamFolders` |
| Which files count as mods | `fileTypes` |
| A port with its own options | `"family": "custom"` with `args` (`{game}`, `{files}`, `{saves}`) |
| The port's 3D options | `stereoArgs`, `stereoModes`, `stereoOffArgs` |
| The exact text in the port list | `label` |
| Your own 3D starting values | `catalog.stereo` |
| Your own words on the pages | `catalog.text` (here: "Your Mods", "Hidden Gems") |
| Mix renamed to "Loadout" | `catalog.mix.label` |
| Your own add-on groups | `catalog.addonGroups` |
| No idgames search | `"search": "none"` |
| A ready-made mix | `catalog.mixes` |
| Your own colours | `ui.buttonColor`, `ui.hoverBorderColor` |
| A letter on the games it makes (E) | `ui.gameLetter` |
| A notice box in blue (info) | `notice`, `ui.noticeStyle` |
| Your colours for links, the author and step numbers | `ui.colors` |
| "Made by" instead of "Card by", on the left | `ui.labels.cardBy`, `ui.cardByPosition` |
| Your own hover tips | `ui.tooltips` |
| A name for the website link | `ui.website.label` |
| A round icon | `ui.iconShape` |
| Programs first on Options | `catalog.optionsOrder` |
| Your colours for "3D" | `catalog.stereoTitleColors` |
| "1 of 1 ready" instead of "in the Codex" | `catalog.text.inCodex` |

## Make it yours

1. Change the names, links and file names to your game's.
2. Change the port's `args` to the options your game's program really takes.
3. Load it again. GPU Selector says what is wrong if something is.
4. Don't want a field? Leave it out. Your card then looks like GPU Selector's usual card.

Every field is explained in the [guide](https://blueskydefender.github.io/GPU-Selector-Cards/guide.html).
