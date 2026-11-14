#!/usr/bin/env python3
"""The kitchen work looks (docs/ERRANDS.md, "Work sites"): small hand-sized compositions that stand at a
kitchen duty's prompt when the place has no cook piece of its own. Writes them into
Prefabs/ReforgerZ/Work/ with their .meta files; run it again only to change a look (it keeps each
file's GUIDs when the file exists). Units: metres, x right, z forward, yaw in degrees.

    py tools/camp_kit/work_looks.py
"""
import os
import re
import secrets

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
WORK = os.path.join(ROOT, "Prefabs", "ReforgerZ", "Work")

TABLE = "{2DB729170DBCE7BE}Prefabs/Props/Furniture/TableOld_01/TableOld_01.et"
POT = "{F1C5AA461DC6ECCE}Prefabs/Props/Civilian/PotOld_01.et"
BOWL = "{799E5AAAB698E210}Prefabs/Props/Garbage/Generic/LitterInterior_01/Single/LitterInterior_USSR_01_bowl.et"
MUG = "{9F2AA7588FBE77A6}Prefabs/Props/Garbage/Generic/LitterInterior_01/Single/LitterInterior_USSR_01_mug.et"
TIN = "{E9CDDAFD49ACDE2A}Prefabs/Props/Garbage/Generic/LitterInterior_01/Single/LitterInterior_USSR_01_tincan_V1.et"
SACK = "{F4CA533A99B20DAE}Prefabs/Props/Civilian/SackUniversal_01.et"
STOVE = "{1D8BBCA16BD4FAEC}Prefabs/Props/Industrial/StoveIndustrial_01/StoveIndustrial_01.et"

TOP = 0.78        # TableOld_01's top (footprints.json: 0.906 m to its highest point): VERIFY in Workbench
STOVE_TOP = 1.05  # StoveIndustrial_01's hob (footprints.json h 1.05; its origin is at a corner: ox -0.571, oz 0.405)

LOOKS = {
    # Stir the pot: the pot on a canteen stove, a bowl on the ground beside it
    "RZ_Work_Stove": [(STOVE, 0.571, 0, -0.405, 0), (POT, 0, STOVE_TOP, 0, 0), (BOWL, -0.9, 0, 0.2, 0)],
    # Peel and chop: the kitchen table, the pot and bowls on it, a sack of potatoes at its end
    "RZ_Work_KitchenTable": [(TABLE, 0, 0, 0, 0), (POT, 0.45, TOP, 0.05, 0), (BOWL, -0.2, TOP, 0.1, 0),
                             (BOWL, -0.5, TOP, -0.15, 0), (SACK, -1.25, 0, 0.1, 80)],
    # Ladle supper out: the pot in the middle of the table, bowls, a mug and a tin set out
    "RZ_Work_ServeTable": [(TABLE, 0, 0, 0, 0), (POT, 0, TOP, 0, 0), (BOWL, -0.45, TOP, 0.15, 0),
                           (BOWL, -0.6, TOP, -0.1, 0), (BOWL, 0.45, TOP, 0.1, 0), (MUG, 0.65, TOP, -0.15, 0),
                           (TIN, -0.3, TOP, -0.25, 0)],
    # The plainest kitchen there is: the pot on the ground with a bowl and a tin (no table fits)
    "RZ_Work_Pot": [(POT, 0, 0, 0, 0), (BOWL, 0.35, 0, 0.1, 0), (TIN, -0.3, 0, 0.15, 0)],
}

META = """MetaFileClass {{
 Name "{{{guid}}}{path}"
 Configurations {{
  EntityTemplateResourceClass PC {{
  }}
  EntityTemplateResourceClass XBOX_ONE : PC {{
  }}
  EntityTemplateResourceClass XBOX_SERIES : PC {{
  }}
  EntityTemplateResourceClass PS4 : PC {{
  }}
  EntityTemplateResourceClass PS5 : PC {{
  }}
  EntityTemplateResourceClass HEADLESS : PC {{
  }}
 }}
}}"""


def hx():
    return secrets.token_hex(8).upper()


def num(v):
    s = ("%.3f" % v).rstrip("0").rstrip(".")
    return "0" if s in ("-0", "") else s


def write(name, parts):
    rel = "Prefabs/ReforgerZ/Work/%s.et" % name
    path = os.path.join(ROOT, rel)
    s = 'GenericEntity {\n ID "%s"\n components {\n  RplComponent "{%s}" {\n  }\n  Hierarchy "{%s}" {\n  }\n }\n coords 0 0 0\n {\n' % (hx(), hx(), hx())
    for (prefab, x, y, z, yaw) in parts:
        s += '  GenericEntity : "%s" {\n   ID "%s"\n   components {\n    Hierarchy "{%s}" {\n    }\n   }\n   coords %s %s %s\n' % (
            prefab, hx(), hx(), num(x), num(y), num(z))
        if yaw:
            s += "   angles 0 %s 0\n" % num(yaw)
        s += "  }\n"
    s += " }\n}\n"
    with open(path, "w", encoding="utf-8", newline="\n") as f:
        f.write(s)

    meta = path + ".meta"
    guid = None
    if os.path.exists(meta):
        m = re.search(r'Name "\{([0-9A-F]{16})\}', open(meta, encoding="utf-8").read())
        if m:
            guid = m.group(1)
    if not guid:
        guid = hx()
        with open(meta, "w", encoding="utf-8", newline="\n") as f:
            f.write(META.format(guid=guid, path=rel))
    print("{%s}%s" % (guid, rel))


if __name__ == "__main__":
    for n, p in LOOKS.items():
        write(n, p)
