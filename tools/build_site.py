# Builds the site in docs\ from tools\pages\*.html and tools\page_template.html,
# copies the cards (cards\ is the one place to edit them) into docs\cards\ for the site's links,
# makes the DOOM card's .zip package, then exports the card guide from GPU Selector (export_guide.py).
#   python tools\build_site.py
import os
import re
import shutil
import subprocess
import sys
import zipfile
import json

HERE = os.path.dirname(os.path.abspath(__file__))
SITE = os.path.join(HERE, "..", "docs")
PAGES = {"index.html": ("Home", "HOME"), "cards.html": ("Example Cards", "CARDS"), "safety.html": ("Safety", "SAFETY")}

ROOT = os.path.join(HERE, "..")

# 0a. A card's own files (its guide and font, in the <card>_files folder next to it) get their
#     SHA256 filled in, so editing GUIDE.md never hides the Guide button by a stale hash
import glob
import hashlib
for card_path in glob.glob(os.path.join(ROOT, "cards", "*", "*", "*.json")):
    files_folder = os.path.join(os.path.dirname(card_path), os.path.splitext(os.path.basename(card_path))[0] + "_files")
    if not os.path.isdir(files_folder):
        continue
    text = open(card_path, encoding="utf-8", newline="").read()
    ui = json.loads(text).get("ui") or {}
    for key in ("guide", "font"):
        entry = ui.get(key)
        if not isinstance(entry, dict) or not entry.get("file") or not entry.get("sha256"):
            continue
        local = os.path.join(files_folder, entry["file"])
        if os.path.isfile(local):
            new = hashlib.sha256(open(local, "rb").read()).hexdigest()
            if new != entry["sha256"]:
                text = text.replace('"' + entry["sha256"] + '"', '"' + new + '"')
                print("updated ui." + key + " sha256 in " + os.path.basename(card_path))
    open(card_path, "w", encoding="utf-8", newline="").write(text)

# 0. Every card laid out for people to read (spaces only, the content never changes)
subprocess.run([sys.executable, os.path.join(HERE, "format_cards.py")], check=True)

# 1. The cards, for the site's Copy Link and Download buttons
shutil.rmtree(os.path.join(SITE, "cards"), ignore_errors=True)
shutil.copytree(os.path.join(ROOT, "cards"), os.path.join(SITE, "cards"),
                ignore=shutil.ignore_patterns("README.md"))
# Text files with Linux line endings, the same on every PC, so their SHA256 always matches
for folder, _, files in os.walk(os.path.join(SITE, "cards")):
    for name in files:
        if name.endswith((".json", ".txt")):
            path = os.path.join(folder, name)
            data = open(path, "rb").read().replace(b"\r\n", b"\n")
            open(path, "wb").write(data)

# 2. The DOOM card package: the card plus its doom_card_files folder
doom = os.path.join(SITE, "cards", "4-expert", "doom")
with zipfile.ZipFile(os.path.join(doom, "doom_card_package.zip"), "w", zipfile.ZIP_DEFLATED) as package:
    package.write(os.path.join(doom, "doom_card.json"), "doom_card.json")
    for name in sorted(os.listdir(os.path.join(doom, "doom_card_files"))):
        package.write(os.path.join(doom, "doom_card_files", name), "doom_card_files/" + name)
print("docs/cards (with doom_card_package.zip)")

# 3. Cards on the House: the list GPU Selector shows in + Add Mod (made by us, so trusted).
#    Only these cards; the pure examples (Shader Starter Pack, Feature Test) are left out.
#    Add a new card here and it shows up in GPU Selector without an app update.
import hashlib
import json
HOUSE = [
    {"name": "RenoDX HDR", "file": "cards/2-intermediate/renodx-hdr/renodx_hdr.json",
     "short": "HDR for games that never had it. Needs an HDR display.", "note": "Made by us."},
    {"name": "Neural Rendering", "file": "cards/3-advanced/neural-rendering/neural_rendering.json",
     "short": "Neural Rendering (DLSS 5) as ReShade add-ons.", "note": "Made by us."},
    {"name": "DOOM", "file": "cards/4-expert/doom/doom_card_package.zip",
     "short": "DOOM mods, map packs and the original games, ready to play.",
     "note": "Made by us. Brings a DLL we made too: the Doomsday 3D Fix."},
]
for card in HOUSE:
    with open(os.path.join(SITE, card["file"]), "rb") as f:
        card["sha256"] = hashlib.sha256(f.read()).hexdigest()
with open(os.path.join(SITE, "house_cards.json"), "w", encoding="utf-8", newline="\n") as f:
    json.dump({"version": 1, "cards": HOUSE}, f, indent=2)
print("docs/house_cards.json (" + str(len(HOUSE)) + " cards)")

template = open(os.path.join(HERE, "page_template.html"), encoding="utf-8").read()
for name, (title, active) in PAGES.items():
    content = open(os.path.join(HERE, "pages", name), encoding="utf-8").read()
    page = template.replace("{{TITLE}}", title).replace("{{ACTIVE_" + active + "}}", ' class="active"')
    page = re.sub(r"\{\{ACTIVE_[A-Z]+\}\}", "", page).replace("{{CONTENT}}", content)
    open(os.path.join(SITE, name), "w", encoding="utf-8", newline="\n").write(page)
    print("docs/" + name)

subprocess.run([sys.executable, os.path.join(HERE, "export_guide.py")] + sys.argv[1:], check=True)

# No em dashes anywhere on the site
for root, _, files in os.walk(SITE):
    for f in files:
        if f.endswith(".html"):
            text = open(os.path.join(root, f), encoding="utf-8").read()
            if "—" in text or "&mdash;" in text:
                print("WARNING: em dash in", f)
