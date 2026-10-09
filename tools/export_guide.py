# Exports GPU Selector's built-in Custom Mod Card Guide (src\mod_guide_handler.cpp) to docs\guide.html,
# in this site's look. Run it again whenever the guide in the app changes:
#   python tools\export_guide.py [path to mod_guide_handler.cpp]
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SITE = os.path.join(HERE, "..", "docs")
SOURCE = sys.argv[1] if len(sys.argv) > 1 else \
    r"P:\General Projects\Game Selector App\GPUSelectorGUI_VS\src\mod_guide_handler.cpp"

code = open(SOURCE, encoding="utf-8").read()
start = code.index('guideFile << R"(') + len('guideFile << R"(')
end = code.index(')";', start)
html = code[start:end].replace(')" R"(', "")          # the guide is split into several raw strings

body = html[html.index("<body>") + len("<body>"):html.index("</body>")]
body = re.sub(r"<hr[^>]*>\s*<p[^>]*>GPU Selector - Custom Mod Card System[^<]*</p>\s*$", "", body.strip())

page = open(os.path.join(HERE, "page_template.html"), encoding="utf-8").read()
page = page.replace("{{TITLE}}", "Card Guide").replace("{{ACTIVE_GUIDE}}", ' class="active"')
page = re.sub(r"\{\{ACTIVE_[A-Z]+\}\}", "", page)
page = page.replace("{{CONTENT}}", '<div class="guide-content card-guide">\n' + body + "\n</div>")
open(os.path.join(SITE, "guide.html"), "w", encoding="utf-8", newline="\n").write(page)
print("docs/guide.html written,", len(page), "bytes")
