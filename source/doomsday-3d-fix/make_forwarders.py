# Makes forwarders.h: every export of Doomsday's deng_appfw.dll is passed on to the original
# (renamed deng_appfw_doomsday.dll), except the two GPU Selector answers itself: the projection
# (3D convergence) and the current eye (when the 3D crosshair reads the depth).
import os
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
WORK = sys.argv[1] if len(sys.argv) > 1 else HERE      # where the made files go
ORIGINAL = "deng_appfw_doomsday"
OWN = ["?projectionMatrix@VRConfig@de@@QEBA?AV?$Matrix4@M@2@MAEBV?$Vector2@M@2@MM@Z",
       "?currentEye@VRConfig@de@@QEBA?AW4Eye@12@XZ"]

lines = ["// Made by make_forwarders.py: do not edit by hand.", ""]
own_ordinals = {}
for row in open(os.path.join(HERE, "exports.txt"), encoding="ascii"):
    row = row.strip()
    if not row:
        continue
    ordinal, name = row.split(" ", 1)
    if name in OWN:
        own_ordinals[name] = ordinal
        continue
    lines.append('#pragma comment(linker, "/EXPORT:%s=%s.%s,@%s")' % (name, ORIGINAL, name, ordinal))

# Ours keep the original's names and numbers
for name in OWN:
    lines.append('#pragma comment(linker, "/EXPORT:%s,@%s")' % (name, own_ordinals[name]))
open(os.path.join(WORK, "forwarders.h"), "w", encoding="ascii").write("\n".join(lines) + "\n")

# The functions we call in the original (an import library is made from this).
# The original's currentEye is found by name at run time: ours has the same name.
calls = ["?mode@VRConfig@de@@QEBA?AW4StereoMode@12@XZ",
         "?eyeShift@VRConfig@de@@QEBAMXZ",
         "?screenDistance@VRConfig@de@@QEBAMXZ",
         "?mapUnitsPerMeter@VRConfig@de@@QEBAMXZ",
         "?frustumShift@VRConfig@de@@QEBA_NXZ",
         "?viewAspect@VRConfig@de@@QEBAMAEBV?$Vector2@M@2@@Z"]
open(os.path.join(WORK, "original.def"), "w", encoding="ascii").write(
    "LIBRARY " + ORIGINAL + "\nEXPORTS\n" + "".join("    " + c + "\n" for c in calls))
print("forwarders:", len(lines) - 2 - len(OWN), "own ordinals:", own_ordinals)
