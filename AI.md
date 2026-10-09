# Making a GPU Selector Card (for AI)

You are making a card for GPU Selector. Start here.

**The goal:** a card brings a project's good features into GPU Selector. It is more than a download button.

## 0. Research first

Before you make the card, start a separate agent to research the project, in this order:

1. **Is it safe?** Its age, makers, license, issues, how its releases are built, and what people report. Not safe? Stop and tell the person. Also find out, from its docs and code:
   - Does it put files in game folders (like a `d3d12.dll` or `dxgi.dll`)? That clashes with ReShade and GPU Selector's own DLLs.
   - Does it touch games with anti-cheat? Could someone be banned?
   - Does it start with Windows, send data, or need admin rights?

   Every real risk you find goes on the card: in `"notice"` for the big ones (bans, broken ReShade), in the text for the rest.
2. **Does it have source code?** Then maybe make a simple app from that code, with only the feature GPU Selector needs. Keep the license, credit the author, and publish it in your own GitHub release.
   - Can the whole app become a ReShade add-on? That is a good option too: it then runs inside the game with ReShade, and needs no separate program.
   - Does the project put a DLL into the game (like a `d3d12.dll` recorder or hook)? Build that part as a ReShade add-on (`.addon64`) instead. GPU Selector loads it with ReShade through Inject Mode, only in the games the user picks, and nothing stays in the game folder. The card installs it with `addons`.
   - Can't it be an add-on? A separate DLL that GPU Selector injects next to ReShade is the other way. Cards have no field for that yet: tell the person it is missing.
3. **No simple app? Can the app be driven from outside?** Look for start options, a command-line mode, or config files. The card's buttons can use those.
4. **None of that?** Make sure the card starts the app properly. Read the app's own docs for the right file, and look inside its zip.

**The research decides the card:** what it downloads, which file it starts, its buttons, its tabs and its text. Write down what you found, and build the card from that.

Something the card needs but the guide has no field for? Don't work around it. Tell the person which part is missing.

## 1. Read

1. **The full guide:** [docs/guide.html](docs/guide.html). Every field, limit and rule is there. Its "Making a Card with an AI" section has a checklist.
2. **An example close to what you need** (table below). Copy its layout, then change it.

## 2. Pick an example

| You want to... | Start from |
|---|---|
| Install shader packs | [cards/1-beginner/shader-starter-pack](cards/1-beginner/shader-starter-pack/) |
| A program with a Launch button | The Magpie tab of [cards/testing/feature-test](cards/testing/feature-test/) (`downloads` + `launch`), and the guide's "launch" part |
| Install ReShade add-ons from GitHub, per game | [cards/2-intermediate/renodx-hdr](cards/2-intermediate/renodx-hdr/) |
| Use tabs, files next to the game, shader settings | [cards/3-advanced/neural-rendering](cards/3-advanced/neural-rendering/) |
| A list of many mods, source ports, a package with a DLL | [cards/4-expert/doom](cards/4-expert/doom/) |
| A list of mods for a game that is not DOOM, with its own port | [cards/4-expert/any-game-catalog](cards/4-expert/any-game-catalog/) |
| See every field at once | [cards/testing/feature-test](cards/testing/feature-test/) |
| Lock a download with a password (a key for supporters) | [cards/testing/locked-card-example](cards/testing/locked-card-example/), and the guide's "Locked Cards" |
| Log in to the maker's own site and list its files | The guide's "Card Sites", and [source/site-login-template](source/site-login-template/) for the site |

## 3. Rules

1. **Downloads:** GitHub release files when you can. Every other file needs a `sha256`.
2. **Licenses:** only use projects whose makers allow it. Name them on the card (`author`, credits).
3. **Text:** short numbered lines, one idea per line. No em dashes. Only claims from the project's own docs.
4. **Size:** a card is 625 pixels tall at most. Use tabs (`ui.modes`) instead of long lists.
5. **Layout:** each topic gets its own tab, like `About | Use | Notes`. At most one short guide per tab. Never stack guides.
6. **Notice:** only for a real, important warning from the project's own docs. Never a notice you made up, like "made by AI" or "untested".
7. **Pictures:** the icon is the project's logo. The background is its art, or none. Never a screenshot of the program.
8. **Programs:** look inside the zip and read the app's docs, so Launch starts the right file. A card only launches files from its own downloads.
9. **Links:** every download must already exist. Never point to a repo or release that isn't made yet. A helper you made is published first, by the person.
10. **Credits:** `cardBy` is the person making the card, never "AI".
11. **DLLs:** a card's DLL goes next to a port the Modding Codex installed. It only goes into a game's own folder (like a Steam game) with `fixFrom`, after the user says yes for that game, and the user can remove it. If the card brings its own DLL, put it in `<card name>_files` next to the card, give its `sha256`, and share its source code.
12. **Keys:** never write a real password or key in a card. Locked downloads ask the user. A card site gets the login only from the user.
13. **Never** try to get around GPU Selector's warnings or Windows Security.

## 4. Test

Nothing outside GPU Selector checks a card. Loading it is the check.

1. In GPU Selector: **Mods** tab, **+ Add Mod**, **Load from File**.
2. A card with a mistake is not loaded, and GPU Selector says what is wrong. Fix it and load it again.
3. Ask the person to check every download link before they press Install.
