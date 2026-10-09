# Lays out every card's JSON so people can read it:
#   1. a blank line between the card's sections (and the catalog's sections),
#   2. a blank line between the entries of a list of objects (mods, ports, downloads, games...),
#   3. short lists of plain values on one line: "ports": ["uzdoom", "gzdoom"].
# The content never changes, only the spaces. build_site.py runs it first.
#   python tools\format_cards.py
import glob
import json
import os

HERE = os.path.dirname(os.path.abspath(__file__))
CARDS = os.path.join(HERE, "..", "cards")
INDENT = "  "
SHORT_LIST = 100    # a list of plain values fits on one line up to this many characters
SPACED = {(), ("catalog",)}    # objects whose sections get a blank line between them


def plain(value):
    return not isinstance(value, (dict, list))


def dump(value, depth, path):
    pad = INDENT * depth
    inner = INDENT * (depth + 1)
    if isinstance(value, dict):
        if not value:
            return "{}"
        text = ""
        last_plain = None
        for key, item in value.items():
            line = inner + json.dumps(key, ensure_ascii=False) + ": " + dump(item, depth + 1, path + (key,))
            if last_plain is not None:
                # In a spaced object, a section (a list or object) gets a blank line around it;
                # plain fields next to each other stay together
                text += ",\n\n" if path in SPACED and not (last_plain and plain(item)) else ",\n"
            text += line
            last_plain = plain(item)
        return "{\n" + text + "\n" + pad + "}"
    if isinstance(value, list):
        if not value:
            return "[]"
        if all(plain(item) for item in value):
            one_line = "[" + ", ".join(json.dumps(item, ensure_ascii=False) for item in value) + "]"
            if len(one_line) <= SHORT_LIST:
                return one_line
        gap = ",\n\n" if all(isinstance(item, dict) for item in value) and len(value) > 1 else ",\n"
        parts = [inner + dump(item, depth + 1, path + ("[]",)) for item in value]
        return "[\n" + gap.join(parts) + "\n" + pad + "]"
    return json.dumps(value, ensure_ascii=False)


def format_card(path):
    text = open(path, encoding="utf-8").read()
    card = json.loads(text)
    pretty = dump(card, 0, ()) + "\n"
    if json.loads(pretty) != card:
        raise SystemExit("format_cards: the layout changed the content of " + path)
    if pretty != text:
        open(path, "w", encoding="utf-8", newline="\n").write(pretty)
        return True
    return False


if __name__ == "__main__":
    changed = [os.path.relpath(p, CARDS) for p in sorted(glob.glob(os.path.join(CARDS, "**", "*.json"), recursive=True))
               if format_card(p)]
    print("cards laid out" + (": " + ", ".join(changed) if changed else " (no changes)"))
