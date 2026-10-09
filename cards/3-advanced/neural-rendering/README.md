# Neural Rendering

**Level 3: Advanced.** Neural Rendering (DLSS 5) as ReShade add-ons, with a tab for each kind of graphics card.

## Load it

1. Copy the card's link on the [website](https://blueskydefender.github.io/GPU-Selector-Cards/cards.html).
2. In GPU Selector: **Mods** tab, **+ Add Mod**, **Load from URL**, paste the link.
3. Or download `neural_rendering.json` and use **Load from File**.

## What to look at

| Part | Field |
|---|---|
| A tab for NVIDIA, NVIDIA + DLSS, and AMD | `ui.modes` (with `gpu`) |
| Files placed next to the game for one launch | `nextToGame` |
| Files placed next to the add-on | `besideAddon` |
| Shader settings it sets | `preprocessor` |
| Only the effects it needs from a shader pack | `repoShaders` |

## Credits

The projects this card installs belong to their makers and keep their own licenses. The card names them.
