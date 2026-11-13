# GPU Selector Cards

Example cards for [GPU Selector](https://github.com/BlueSkyDefender/GPUSelector). Use them, or use them to make your own!

**Website:** https://blueskydefender.github.io/GPU-Selector-Cards/

## What is a card?

1. A card is a small file (`.json`) that adds a mod to GPU Selector's **Mods** tab.
2. It can download files, install ReShade shaders and add-ons, and start your games with them.
3. Some cards come as a `.zip` package, with extra files next to the card.

## Load a card

1. In GPU Selector, open the **Mods** tab.
2. Press **+ Add Mod**.
3. Pick **Load from URL** and paste the card's link, or **Load from File** and choose the `.json` or `.zip`.
4. Or pick **Cards on the House**: the cards we made, in categories. Select the ones you want, then press **Load Card**.

## Example cards by level

| Level | Card | What to look at |
|---|---|---|
| 1. Beginner | [Shader Starter Pack](cards/1-beginner/shader-starter-pack/) | Shader packs, games with a ReShade preset |
| 2. Intermediate | [RenoDX HDR](cards/2-intermediate/renodx-hdr/) | GitHub downloads, add-ons per game |
| 3. Advanced | [Neural Rendering](cards/3-advanced/neural-rendering/) | Tabs, files next to the game, shader settings |
| 4. Expert | [DOOM](cards/4-expert/doom/) | A list of mods, source ports, a package with a DLL |
| 4. Expert | [Example Game Mods](cards/4-expert/any-game-catalog/) | A list of mods for any game, with its own port and 3D |
| Testing | [Card Feature Test](cards/testing/feature-test/) | Every feature at once |

## Tool cards

These used to be built into GPU Selector. They are in Cards on the House now, so they change without an app update.

| Card | What to look at |
|---|---|
| [Stereo Loom](cards/tools/stereo-loom/) | A program from GitHub with a Launch button |
| [3DConsoleBridge](cards/tools/3dconsolebridge/) | Three programs on one card, a program started with ReShade, tags, link buttons, What You Need boxes |
| [Windower 4](cards/tools/windower-4/) | Finding a tool that is already installed, and backing up its settings |

## Cards on the House and card lists

1. `docs/house_cards.json` is the list GPU Selector shows. `tools/build_site.ps1` makes it, with each card's SHA256.
2. To add or change a card: edit it in `cards/`, add it to the `house` list in `tools/build_site.ps1` and `tools/build_site.py` (name, file, short text, note, category), run the build, and push everything it changed.
3. A card can be in up to three categories: `"Games, 3D"`.
4. You can host your own list the same way. People load its link with **Load from URL**. The Card Guide has the rules ("Card list rules").

## Make your own

1. Read the [Card Guide](https://blueskydefender.github.io/GPU-Selector-Cards/guide.html). It is the same guide as in GPU Selector.
2. Start from the example closest to what you want.
3. **Using an AI?** Point it at [AI.md](AI.md).

## What's in this repository

```
cards/      The example cards, by level, and the tool cards (cards/tools). Each has a README.
source/     Source code for files a card brings.
  doomsday-3d-fix/   The DLL that fixes Doomsday's 3D (used by the DOOM card)
  laser-dot/         The DLL that puts a 3D crosshair dot at depth in UZDoom, GZDoom and LZDoom (DOOM card)
docs/       The website (GitHub Pages). docs/cards is made by tools/build_site.py.
tools/      Builds the website and copies the Card Guide from GPU Selector.
AI.md       Where an AI should start.
```

## Safety

1. Cards can download programs, like DLL files.
2. GPU Selector checks every file and always asks before a DLL is used.
3. Only load cards you trust. More on the [Safety page](https://blueskydefender.github.io/GPU-Selector-Cards/safety.html).

## No warranty

1. These cards are examples, given "as is", with no warranty.
2. Use them at your own risk. Depth3D is not responsible for any problems a card causes.
3. Cards made by other people are their own, and they are responsible for them.

## License

1. [BSD Zero Clause](LICENSE): copy, change and share the cards freely. You don't need to credit us, and your own cards are yours.
2. The projects the cards download belong to their makers and keep their own licenses. Each card names them.
3. The DLLs in `source/` (the 3D crosshair and the Doomsday 3D Fix) use [PolyForm Noncommercial 1.0.0](source/laser-dot/LICENSE.md): open code, not for commercial use, and keep the Required Notice that names the author. Given "as is": the author is not responsible for any damage they may cause.

## Editing this site

1. Edit the cards in `cards/`, the pages in `tools/pages/`.
2. Run `powershell -ExecutionPolicy Bypass -File tools\build_site.ps1` (no Python needed), or `python tools/build_site.py`.
