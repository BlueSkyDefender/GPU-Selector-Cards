# DOOM Card: Field Manual

> **Short version:** find your game files, pick a mod, press its button. Rip and tear.

---

## Your Mission (3 steps)

1. Open **1. Your Games**. GPU Selector looks for DOOM on your PC.
2. Open **3. Mods**. Pick a mod you like.
3. Press its button: **Install & Play**.

> No DOOM on your PC? Press **Get Freedoom (free)** on 1. Your Games. Most mods work with it.

---

## The Pages

### 1. Your Games
- Shows the DOOM game files GPU Selector found.
- **Look Again**: look one more time.
- **Pick a File**: point at your file yourself, like `DOOM2.WAD`.

### 2. Engine Games
- Other games made with DOOM's engine.
- Steam games you own, and free games.

### 3. Mods
- The big list. Use the chips to sort: **Map Packs**, **Total Conversions**, **Mods** and more.
- Type in the search box to find one.
- A green dot means it is installed.

### 4. Options
- **3D Settings**, your own WADs and add-ons, source ports, and the look.

---

## Playing a Mod

1. Find the mod in **3. Mods**.
2. Press **Install & Play**. It downloads, installs and starts.
3. Is the button **Download**? That mod comes from a website.
   - Download the file on the page that opens.
   - Back in GPU Selector, press **Install Download**.
   - Then press **Play**.

> Missing a program to run it? GPU Selector offers **Get It and Play**. One press, then the game starts.

---

## Playing in 3D

1. Press **3D** on a mod. It turns red and blue.
2. Press **Play**.
3. Change how it looks in **4. Options**, **3D Settings**:
   - **3D Mode**: Side by Side, Top and Bottom, and more.
   - **Depth**: how strong the 3D is. Start at 25.
   - **ZPD**: where the screen sits. Keep it low.

> Only ports with **3D** next to their name have their own 3D.
> **Stereo 3D** for other DOOM engines is on the way. It will come in time.

> 3D only works in **OpenGL**.
> 1. With 3D on, GPU Selector starts the game in OpenGL, not Vulkan.
> 2. Vulkan-only effects (like Vulkan ReShade) won't show.

> **Steam games:** the first time you play one in 3D, GPU Selector asks if it may add the **3D Crosshair**.
> Said no and changed your mind? Press **Reset 3D** in 3D Settings to be asked again.

---

## Mixing Mods (the M)

Want a map pack **with** a gameplay mod? Mix them.

1. Press **M** on a map pack. It turns half white.
2. Press **Mix**, right of the mod count.
3. Pick a gameplay mod, like Brutal Doom.
4. Tick any add-ons you want: lights, HUD, music.
5. Press **Play** next to the games at the top.

> With **M** on, that map pack's own **Play** button starts your mix too. Press **M** again to play it as normal.

---

## Start Options

The **Start:** row sits above the search box in **3. Mods**.

1. **Skill**: pick one, or leave **Game's default**.
2. **Map**: type a map, like `MAP01` or `E1M1`. Keep it empty to start as normal.
   - Hexen on Chocolate or Crispy: type the warp number, like `7`.
   - Hexen on UZDoom or GZDoom: the warp number works too, or type the map name, like `MAP13` (the same map: Shadow Wood).
   - Hexen class: add it after the map, like `7, Cleric` or `7, C`.
   - The classes: **Fighter** (F), **Cleric** (C) and **Mage** (M).
   - Just `Cleric` on Chocolate or Crispy starts a new game as the Cleric.
   - Doomsday can't start with a class (a Doomsday bug). Pick it in its New Game menu.
3. **Pistol start**: start each level with only the pistol.
4. **Continue last save**: load your newest save for that mod.

- Every **Play** on the card uses them, mixes too.
- **Continue** is not used for mixes.
- Saves are kept per mod and per port family.
- DSDA-Doom, PrBoom+, Woof!, Nugget and Cherry only continue their own saves.
- The ZDoom ports share saves. An older port may not load a newer port's save, so keep one ZDoom port per mod.
- **Continue** works on UZDoom, GZDoom, LZDoom, ZDoom 2.8.1, Zandronum, GZDoom RT, DSDA-Doom, PrBoom+ RT, Woof!, Nugget, Cherry, Chocolate, Crispy and International Doom (Heretic, Hexen and Strife too).
- **Pistol start** needs GZDoom 4.12 or newer on the GZDoom ports.
- **Continue** keeps the class you saved with.

> A port that can't use an option leaves it out. The status line says so.

---

## Your Own Files

Got WADs or add-ons of your own? Go to **4. Options**, **Your WADs** and **Your Add-ons**.

1. WADs: press **Add Files**, or drop them on **Drop WADs here**.
2. Add-ons: press **Add Files** on the add-ons side, or drop them on **Drop add-ons here**.
3. Have a folder full of them? Press **Add a Folder**. Nothing in it is copied.

---

## Something Went Wrong?

- **"Needs DOOM II" in red:** GPU Selector can't find that game. Press **Look Again**, or pick the file.
- **Download did not install:** press **Install Download** after the file is in your Downloads folder.
- **The game will not start:** try another port in the list next to the mod.
- **3D looks wrong:** press **Reset 3D** in 3D Settings.
- **The game crashes on an AMD card:**
  1. In Discord, turn off **Game Detection**, or quit Discord.
  2. Discord can crash OpenGL games on AMD, even with its overlay off.

---

## Good to Know

- Every download is checked and scanned by Windows Security.
- Mods go in the card's own folder.
- Your game folders only change in two cases:
  1. You say yes to the **3D Crosshair** for a Steam game. Remove it with the trash button on that game's row.
  2. A Steam game starts through its own launcher. With 3D on, GPU Selector adds its 3D lines to that game's settings file and keeps a backup.
- Adult mods stay hidden. You can change that in **Parental Settings**.

> Now go. The demons won't wait.
