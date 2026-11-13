"""The camp-life pieces: one source for their prefabs (build.py) and their drawings (render.py).
Edit a piece here, then run build.py (prefabs) and render.py (the plans page). Units: metres. x right, z forward; +Z is the piece's front
(toward the camp centre for inner pieces, outward for perimeter pieces). Yaw in degrees.

FP: hand-typed prop sizes (w, d, shape, fill-kind): the drawings' shapes and fills, and the
fallback size of a prop footprints.json hasn't measured (each piece's footprint is its props'
measured boxes: footprints.py, docs/SITES.md).
P(name, x, z, yaw, y=0, pitch=0, roll=0) places a prop; S(tag, x, z, anim, look_x, look_z) a seat.
RULES gives every piece its tier, zone and rules, DOCTRINE each kind of site's layers (docs/SITES.md):
the solver lays each site out by them; KITS says only what each kind of site brings.

Every piece stands outdoors (camp life never puts a piece inside a building; only a quartermaster's
counter stands where he does, indoors too when it fits): outdoor props only. No cabinets, shelves,
sinks, towel rails, shower curtains or hospital furniture; tables and chairs only where people
dragged them out to use them (a table, a sofa round a fire).
"""

FP = {
    "Fireplace_01.et": (1.2, 1.2, "c", "fire"), "Campfire_burning.et": (0.5, 0.5, "c", "fire"),
    "WoodLog_01_A.et": (0.3, 2.0, "r", "wood"), "WoodPile_01_A.et": (1.0, 2.6, "r", "wood"), "LogChopping_01.et": (0.5, 0.5, "c", "wood"), "WoodKindling_01.et": (0.6, 0.5, "r", "wood"),
    "WoodPile_02_end_A.et": (1.2, 1.6, "r", "wood"),
    "SackUniversal_01.et": (0.5, 0.8, "r", "cloth"), "SackUniversal_01_Pile.et": (1.3, 1.2, "r", "cloth"),
    "PicnicTable_01.et": (1.8, 1.6, "r", "wood"), "LampKerosene_01_green.et": (0.2, 0.2, "c", "light"), "LanternMilitary_US_01.et": (0.2, 0.2, "c", "light"),
    "CamoNet_Small_US.et": (4.0, 4.0, "r", "net"), "CrateWooden_01_plain.et": (0.9, 0.6, "r", "wood"), "CrateWooden_02_1x1x1.et": (1.0, 1.0, "r", "wood"),
    "BarrelMetal_01_rust.et": (0.6, 0.6, "c", "metal"), "BarrelMetal_01_blue.et": (0.6, 0.6, "c", "metal"), "BarrelMetal_01_grey.et": (0.6, 0.6, "c", "metal"),
    "TentSmall_02_red.et": (2.0, 2.4, "r", "tent"), "TentSmall_02_blue.et": (2.0, 2.4, "r", "tent"), "TentBig_01_orange.et": (3.0, 4.0, "r", "tent"), "TentBig_02_green.et": (3.0, 4.0, "r", "tent"),
    "TentSmallUS_01.et": (2.0, 2.6, "r", "tent"), "TentUS_01_Assembled.et": (5.0, 7.0, "r", "tent"), "TentFIA_01.et": (4.2, 5.0, "r", "tent"),
    "BlanketFolded_01_green.et": (0.4, 0.5, "r", "cloth"), "BlanketBeach_01_brown.et": (0.9, 1.7, "r", "cloth"), "BlanketBeach_01_red.et": (0.9, 1.7, "r", "cloth"),
    "Sandbag_01_long_burlap.et": (2.0, 0.5, "r", "sand"), "Sandbag_01_end_burlap.et": (0.6, 0.5, "r", "sand"), "Sandbag_01_single_burlap.et": (0.6, 0.35, "r", "sand"),
    "ChalkBoard_01.et": (1.2, 0.5, "r", "board"), "Notice_A4_Vertical_V1.et": (0.25, 0.05, "r", "paper"),
    "TirePile_01_small.et": (1.4, 1.4, "c", "rubber"), "Tire_UAZ469.et": (0.8, 0.8, "c", "rubber"),
    "Pallet_01.et": (1.2, 0.8, "r", "wood"), "Pallet_01_Pile.et": (1.2, 0.8, "r", "wood"),
    "TableOld_01.et": (1.2, 0.8, "r", "wood"), "TableOld_01_white.et": (1.2, 0.8, "r", "wood"),
    "ChairOld_01.et": (0.5, 0.5, "r", "wood"), "ChairOld_02.et": (0.5, 0.5, "r", "wood"), "Stool_01.et": (0.35, 0.35, "c", "wood"), "Chair_01_white.et": (0.5, 0.5, "r", "plastic"),
    "DeckChair_01_ochre.et": (0.6, 1.2, "r", "cloth"), "Armchair_01.et": (0.9, 0.9, "r", "cloth"), "Sofa_02_green.et": (2.0, 0.9, "r", "cloth"),
    "BeerBottle_01_empty_brown.et": (0.1, 0.1, "c", "glass"), "BeerBottle_01_group.et": (0.3, 0.3, "c", "glass"),
    "ClotheslineFrame_01.et": (3.2, 0.4, "r", "metal"), "BasketLaundry_01.et": (0.6, 0.4, "r", "cloth"),
    "VegetableBed_01_small.et": (1.2, 3.0, "r", "soil"), "VegetableBed_01_big.et": (2.0, 5.0, "r", "soil"),
    "Bucket_01.et": (0.3, 0.3, "c", "metal"), "Bucket_02.et": (0.3, 0.3, "c", "metal"),
    "TableMilitary_US_01.et": (2.0, 0.8, "r", "metal"),
    "Notebook_01_Stack.et": (0.3, 0.2, "r", "paper"), "FileOffice_01_Stack.et": (0.3, 0.2, "r", "paper"),
    "ToolsSurgical_01_autoclavedrum.et": (0.3, 0.3, "c", "metal"), "Antenna_R161_01.et": (0.5, 0.5, "c", "metal"), "GeneratorPortable_US_01_Off.et": (0.8, 0.5, "r", "metal"),
    "WoodenFenceOld_01_pole.et": (0.15, 0.15, "c", "wood"), "WoodenFenceOld_01_4m.et": (4.0, 0.15, "r", "wood"),
    "SkewerGrill_01.et": (0.6, 0.4, "r", "metal"), "MirrorBathroom_01.et": (0.5, 0.05, "r", "glass"),
    "TinWall_01_3m.et": (3.0, 0.1, "r", "metal"), "TinWall_01_6m_A.et": (6.0, 0.1, "r", "metal"),
    "Junk_01_single_planks.et": (2.0, 0.3, "r", "wood"), "Junk_01_single_mattress.et": (0.9, 1.9, "r", "cloth"), "Junk_01_pile_small.et": (1.5, 1.2, "r", "metal"),
    "Guitar_01.et": (0.4, 1.0, "r", "wood"), "LitterInterior_USSR_01_tincan_V1.et": (0.1, 0.1, "c", "metal"), "LitterInterior_USSR_01_tincan_V2.et": (0.1, 0.1, "c", "metal"),
    "LitterInterior_USSR_01_bowl.et": (0.15, 0.15, "c", "metal"), "LitterInterior_USSR_01_mug.et": (0.1, 0.1, "c", "metal"),
    "Wheelbarrow_01.et": (0.7, 1.4, "r", "metal"), "RadioSmall_01_black.et": (0.3, 0.15, "r", "metal"), "DogHouse_01.et": (1.0, 1.2, "r", "wood"),
    "FlagPole_01_empty.et": (0.2, 0.2, "c", "metal"), "DeerSkull_01_board.et": (0.4, 0.1, "r", "wood"), "MarketStand_01.et": (2.5, 1.5, "r", "wood"),
    "ShowerBeach_01.et": (0.8, 0.8, "r", "metal"), "ForestBench_01.et": (1.8, 0.5, "r", "wood"),
    "TablePhotoFrame_01.et": (0.15, 0.1, "r", "wood"), "Photo_Square_01_V1.et": (0.1, 0.1, "r", "paper"), "FlowerpotFlowers_01_begonia_white.et": (0.4, 0.4, "c", "plant"),
    "CandleHolderDecorative_01.et": (0.12, 0.12, "c", "light"), "Suitcase_01.et": (0.7, 0.25, "r", "cloth"), "Suitcase_04_open.et": (0.7, 0.9, "r", "cloth"),
    "Sandbag_Barricade_USSR_04.et": (3.0, 1.5, "r", "sand"), "BunkBed_01_Mattress.et": (0.9, 1.9, "r", "cloth"),
    # The layers' props (docs/SITES.md, "Building in layers"): wire, fences, scrap, sandbags, lights, signs
    "BarbedTape_01_Long.et": (1.1, 3.45, "r", "wire"), "BarbedTape_01_Short.et": (1.0, 2.0, "r", "wire"), "BarbedTape_01_Stake.et": (0.05, 0.05, "c", "wire"),
    "BarbedTape_KnifeRest.et": (1.4, 3.5, "r", "wire"), "CzechHedgehog_01_rusty.et": (1.7, 1.7, "r", "metal"),
    "NetFence_05_tall_6m_V1.et": (6.3, 0.12, "r", "metal"), "NetFence_05_tall_3m.et": (3.2, 0.12, "r", "metal"), "NetFence_05_tall_pole.et": (0.13, 0.13, "c", "metal"),
    "WoodenFence_01_8m.et": (8.0, 0.17, "r", "wood"), "WoodenFence_01_4m.et": (4.0, 0.13, "r", "wood"),
    "TinWall_01_6m_A_rusty.et": (6.0, 0.4, "r", "metal"), "TinWall_01_3m_rusty.et": (3.1, 0.2, "r", "metal"),
    "Sandbag_01_long_high_burlap.et": (3.4, 1.2, "r", "sand"), "RoadBlock_01.et": (2.2, 0.5, "r", "metal"),
    "GeneratorFloodlight_US_01.et": (3.0, 1.7, "r", "light"), "SignCheckpoint_01_stop.et": (0.7, 0.5, "r", "board"),
    # The reference pieces' props (docs/SITES.md, "From the reference board")
    "Ural4320_wreck_static.et": (2.6, 7.4, "r", "metal"), "UAZ469_wreck_rusty_static.et": (1.8, 4.0, "r", "metal"),
    "S105_wreck_static.et": (1.7, 4.2, "r", "metal"), "S1203_wreck_static.et": (1.9, 4.5, "r", "metal"), "FieldKitchenTrailerUSSR_01.et": (2.0, 3.4, "r", "metal"),
    "Jerrycan_01.et": (0.35, 0.17, "r", "metal"), "GeneratorFloodlight_USSR_01.et": (3.0, 1.7, "r", "light"),
    "LifeGuardTower_01_darkgreen.et": (2.4, 2.4, "r", "wood"), "Loudspeaker.et": (0.4, 0.4, "c", "metal"), "ConeTraffic_01_red.et": (0.4, 0.4, "c", "plastic"),
    "SignIndustrial_USSR_02_warnings_entryforbidden_pole.et": (0.6, 0.1, "r", "board"),
}

def P(name, x, z, yaw=0, y=0, pitch=0, roll=0):
    return ("p", name, x, z, yaw, y, pitch, roll)

def S(tag, x, z, anim, lx, lz):
    return ("s", tag, x, z, anim, lx, lz)

# Each piece: folder, use, role, one line of intent, [parts]
C = {}
# Measured, never typed (docs/SITES.md, "Real footprints"): every prop's box, its base and the top surface at its
# middle are in footprints.json (the dedicated server spawns each prop and reads them). A prop stood on another
# stands on that one's measured top; a board or a pallet stood on its edge is lifted by half its measured side.
import json as _json
import math as _math
import os as _os

_BOX = _json.load(open(_os.path.join(_os.path.dirname(_os.path.abspath(__file__)), "footprints.json"), encoding="utf-8"))["props"]


def TOP(prop):
    """Where something stood on `prop` stands, from the prop's origin: the measured top at its middle (the seat of a
    chair, the top of a table), or the top of its box where the trace fell through (an open barrel)."""
    b = _BOX[prop]
    top = b.get("top", b["h"])
    if top < 0.05:
        return b["h"]
    return top


def _holds(prop, x, z, yaw, px, pz):
    b = _BOX.get(prop)
    if not b:
        return False
    a = _math.radians(yaw)
    cs, sn = _math.cos(a), _math.sin(a)
    cx = x + b["ox"] * cs + b["oz"] * sn
    cz = z - b["ox"] * sn + b["oz"] * cs
    dx, dz = px - cx, pz - cz
    lx = dx * cs - dz * sn
    lz = dx * sn + dz * cs
    return abs(lx) <= b["w"] / 2 + 0.05 and abs(lz) <= b["d"] / 2 + 0.05


def _snap(name, parts):
    """Every prop's height from measurements: on the ground, on the measured top of the prop under it (the one whose
    top is nearest the height typed for it), or stood on its edge. A prop hung on a wall or a post keeps its height."""
    props = sorted([i for i, q in enumerate(parts) if q[0] == "p"], key=lambda i: parts[i][5])
    down = []
    for i in props:
        _, prop, x, z, yaw, y, pitch, roll = parts[i]
        b = _BOX.get(prop)
        if b and (abs(pitch) > 20 or abs(roll) > 20):
            # stood on its edge: its lowest edge on the ground (only where the typed height meant that)
            lift = (b["w"] / 2) * abs(_math.sin(_math.radians(roll)))
            if abs(pitch) > 20:
                lift = (b["d"] / 2) * abs(_math.sin(_math.radians(pitch)))
            if b["h"] < 0.4 and abs(lift - y) <= 0.5:
                y = round(lift, 2)
        elif y >= 0.05:
            best = None
            for prop2, x2, z2, yaw2, y2 in down:
                if _holds(prop2, x2, z2, yaw2, x, z):
                    top = y2 + TOP(prop2)
                    if best is None or abs(top - y) < abs(best - y):
                        best = top
            if best is not None and abs(best - y) <= 0.6:
                y = round(best, 2)
        parts[i] = ("p", prop, x, z, yaw, y, pitch, roll)
        if abs(pitch) <= 20 and abs(roll) <= 20:
            down.append((prop, x, z, yaw, y))
    return parts


# Spacing from measured faces (docs/SITES.md, "Spacing"): a seat stands off a table's edge, a fire's edge or the next
# prop in a row by a stated gap between their measured faces, never by a typed centre.
SEAT_GAP = 0.1    # a seat in use: its front 0.1 m out from the table's edge (knees under the table)
SEAT_LEFT = 0.35  # a seat pushed back by someone who got up
FIRE_GAP = 0.9    # a seat's front 0.9 m from a fire's edge: warm, feet out of the ashes
ROW_TOUCH = 0.0   # sandbags, walls, a bench of crates: face to face


def _rot(yaw, lx, lz):
    a = _math.radians(yaw)
    cs, sn = _math.cos(a), _math.sin(a)
    return lx * cs + lz * sn, -lx * sn + lz * cs


def _yaw_of(dx, dz):
    return _math.degrees(_math.atan2(dx, dz))


def FRONT(prop):
    """A seat's front, measured: the side opposite its back (the face whose edge stands highest, tz0/tz1/tx0/tx1 in
    footprints.json). Returns the front's local direction (x, z), that face's distance from the origin and the box
    centre's offset across it. A seat with no back (a stool, a crate, a stump) faces +z."""
    b = _BOX[prop]
    edges = {k: b.get(k) for k in ("tz0", "tz1", "tx0", "tx1")}
    if b["h"] > 0.7 and any(v is None for v in edges.values()):
        raise SystemExit("%s has a back but its edges aren't measured (FOOTPRINTS=1 tools/runtime_test/run.sh sites)" % prop)
    sides = {"tz0": ((0, 1), b["z1"], b["ox"]), "tz1": ((0, -1), -b["z0"], b["ox"]),
             "tx0": ((1, 0), b["x1"], b["oz"]), "tx1": ((-1, 0), -b["x0"], b["oz"])}
    if b["h"] > 0.7:
        # a back that leans out past the seat leaves nothing under the trace near that face (-100): with the other
        # three level, that open side is the back (ChairOld_01)
        missed = [k for k, v in edges.items() if v < -50]
        rest = [v for k, v in edges.items() if v >= -50]
        if len(missed) == 1 and rest and max(rest) - min(rest) < 0.05:
            return sides[missed[0]]
        high = max(edges, key=lambda k: edges[k])
        rest = sorted(v for k, v in edges.items() if k != high)
        if edges[high] - rest[-1] > 0.2 or edges[high] - rest[0] > 0.2:
            return sides[high]
    return sides["tz0"]


def _face(prop, x, z, yaw, side, along):
    """A point on one face of a placed prop's measured box (side: front +z, back -z, right +x, left -x), `along`
    metres along that face from its middle, and the face's outward direction."""
    b = _BOX[prop]
    n, dist, mid = {"front": ((0, 1), b["z1"], b["ox"]), "back": ((0, -1), -b["z0"], b["ox"]),
                    "right": ((1, 0), b["x1"], b["oz"]), "left": ((-1, 0), -b["x0"], b["oz"])}[side]
    if n[0] == 0:
        lx, lz = mid + along, n[1] * dist
    else:
        lx, lz = n[0] * dist, mid + along
    wx, wz = _rot(yaw, lx, lz)
    nx, nz = _rot(yaw, n[0], n[1])
    return x + wx, z + wz, nx, nz


def _stand_off(seat, fx, fz, nx, nz, gap, turn):
    """A seat whose front faces back along (nx, nz), its front face `gap` from the point (fx, fz)."""
    (sx, sz), fd, across = FRONT(seat)
    yaw = _yaw_of(-nx, -nz) - _yaw_of(sx, sz) + turn
    b = _BOX[seat]
    # the face toward the point (its front, or a side when turned), and the box's centre across it, so the seat
    # sits square on the point
    lx, lz = _rot(-yaw, -nx, -nz)
    if abs(lx) > abs(lz):
        fd = b["x1"] if lx > 0 else -b["x0"]
        cx, cz = 0, b["oz"]
    else:
        fd = b["z1"] if lz > 0 else -b["z0"]
        cx, cz = b["ox"], 0
    ox, oz = _rot(yaw, cx, cz)
    px = fx + nx * (gap + fd) - ox
    pz = fz + nz * (gap + fd) - oz
    return round(px, 3), round(pz, 3), round(((yaw + 180) % 360) - 180, 1)


def SEAT(seat, table, tx, tz, tyaw, side, along=0.0, gap=SEAT_GAP, slot="RZ_Sit", anim="sit", turn=0):
    """A seat pulled up to one side of a table, measured: its front `gap` from the table's edge, square to it,
    `along` metres from the side's middle. With `slot`, the sitting spot on it, looking at the table."""
    fx, fz, nx, nz = _face(table, tx, tz, tyaw, side, along)
    px, pz, yaw = _stand_off(seat, fx, fz, nx, nz, gap, turn)
    parts = [P(seat, px, pz, yaw)]
    if slot:
        tb = _BOX[table]
        cx, cz = _rot(tyaw, tb["ox"], tb["oz"])
        parts.append(S(slot, px, pz, anim, round(tx + cx, 2), round(tz + cz, 2)))
    return parts


def RING(seat, fire, x, z, bearing, gap=FIRE_GAP, slot="RZ_Sit", anim="sit", turn=0):
    """A seat round a fire at `bearing` degrees (0 = +z), its front `gap` from the fire's measured edge."""
    b = _BOX[fire]
    r = max(b["w"], b["d"]) / 2
    nx, nz = _math.sin(_math.radians(bearing)), _math.cos(_math.radians(bearing))
    px, pz, yaw = _stand_off(seat, x + b["ox"] + nx * r, z + b["oz"] + nz * r, nx, nz, gap, turn)
    parts = [P(seat, px, pz, yaw)]
    if slot:
        parts.append(S(slot, px, pz, anim, x, z))
    return parts


def ROW(prop, x, z, yaw, count, gap=ROW_TOUCH, start=0):
    """`count` of one prop end to end along its own x, the first one's origin at (x, z): each one's right face
    `gap` from the next one's left face, measured."""
    b = _BOX[prop]
    step = b["w"] + gap
    parts = []
    for i in range(start, start + count):
        dx, dz = _rot(yaw, step * i, 0)
        parts.append(P(prop, round(x + dx, 3), round(z + dz, 3), yaw))
    return parts


def BESIDE(prop, other, ox_, oz_, oyaw, side, along=0.0, gap=ROW_TOUCH, yaw=None):
    """`prop` set against one face of `other` (placed at ox_, oz_, oyaw), `gap` between their measured faces, turned
    like `other` unless `yaw` is given: a crate against the container, a case against the wall."""
    fx, fz, nx, nz = _face(other, ox_, oz_, oyaw, side, along)
    if yaw is None:
        yaw = oyaw
    b = _BOX[prop]
    # the prop's own face toward `other`, in its local frame
    lx, lz = _rot(-yaw, -nx, -nz)
    if abs(lx) > abs(lz):
        fd = b["x1"] if lx > 0 else -b["x0"]
        cx, cz = 0, b["oz"]
    else:
        fd = b["z1"] if lz > 0 else -b["z0"]
        cx, cz = b["ox"], 0
    ox2, oz2 = _rot(yaw, cx, cz)
    return P(prop, round(fx + nx * (gap + fd) - ox2, 3), round(fz + nz * (gap + fd) - oz2, 3), yaw)


# ------------------------------------------------------------------- Settling: every prop by its measured faces
TOUCH = 0.03      # two faces this close touch
APART = 0.25      # the least gap that reads as meant (a step, a hand's reach)
HOLLOW = ("CamoNet", "Tent", "Antenna", "MarketStand", "ShowerBeach", "GuardTower", "CanvasCover", "LifeGuardTower")
TABLES = ("TableOld_01.et", "TableOld_01_white.et", "TableLong_01.et", "TableMilitary_US_01.et", "PicnicTable_01.et")


def QUAD(prop, x, z, yaw):
    """A placed prop's measured box on the ground, as its four corners."""
    b = _BOX.get(prop)
    if not b:
        return None
    cx, cz = _rot(yaw, b["ox"], b["oz"])
    hw, hd = b["w"] / 2, b["d"] / 2
    return [(x + cx + px, z + cz + pz) for px, pz in (_rot(yaw, lx, lz) for lx, lz in ((-hw, -hd), (hw, -hd), (hw, hd), (-hw, hd)))]


def _closest(qa, qb):
    """The shortest way from box a to box b: (distance, (dx, dz) unit from a to b)."""
    best = None
    for p_, q_ in ((qa, qb), (qb, qa)):
        for pt in p_:
            for i in range(4):
                a, b = q_[i], q_[(i + 1) % 4]
                ax, az = b[0] - a[0], b[1] - a[1]
                t = max(0.0, min(1.0, ((pt[0] - a[0]) * ax + (pt[1] - a[1]) * az) / (ax * ax + az * az or 1)))
                cx, cz = a[0] + t * ax, a[1] + t * az
                d = _math.hypot(pt[0] - cx, pt[1] - cz)
                if best is None or d < best[0]:
                    dx, dz = (cx - pt[0], cz - pt[1]) if p_ is qa else (pt[0] - cx, pt[1] - cz)
                    best = (d, (dx / (d or 1), dz / (d or 1)))
    return best


def _inside(pt, quad):
    sign = 0
    for i in range(4):
        a, b = quad[i], quad[(i + 1) % 4]
        c = (b[0] - a[0]) * (pt[1] - a[1]) - (b[1] - a[1]) * (pt[0] - a[0])
        if c != 0:
            if sign and (c > 0) != (sign > 0):
                return False
            sign = c
    return True


def GAP(qa, qb):
    """The gap between two measured boxes; 0 where they meet or overlap."""
    if any(_inside(p_, qb) for p_ in qa) or any(_inside(p_, qa) for p_ in qb):
        return 0.0
    return _closest(qa, qb)[0]


def _ground(q):
    b = _BOX.get(q[1])
    return (q[0] == "p" and b and q[5] < 0.3 and abs(q[6]) <= 10 and abs(q[7]) <= 10 and b["h"] >= 0.3
            and not any(w in q[1] for w in HOLLOW))


def _seated(a, b, gap):
    """A seat pulled up to a table keeps its stated gap (SEAT_GAP, SEAT_LEFT)."""
    if a[1] in TABLES or b[1] in TABLES:
        return any(abs(gap - g) <= 0.02 for g in (SEAT_GAP, SEAT_LEFT))
    return False


def _settle(parts):
    """No sliver between two props on the ground: a pair whose measured faces are more than TOUCH and less than
    APART apart is closed up to touch (the nearer half of that range) or opened to APART, the smaller one moved
    along the shortest way between them, with whatever stands on it. Props a piece places by SEAT, RING, ROW or
    BESIDE are where those put them already."""
    for _ in range(10):
        moved = False
        ground = [i for i, q in enumerate(parts) if _ground(q)]
        for ii in range(len(ground)):
            for jj in range(ii + 1, len(ground)):
                a, b = parts[ground[ii]], parts[ground[jj]]
                qa, qb = QUAD(a[1], a[2], a[3], a[4]), QUAD(b[1], b[2], b[3], b[4])
                gap = GAP(qa, qb)
                if gap <= TOUCH or gap >= APART or _seated(a, b, gap):
                    continue
                d, (dx, dz) = _closest(qa, qb)
                shift = -gap if gap < APART / 2 else APART + 0.01 - gap
                # move the smaller one (b moves along a->b; a moves the other way)
                small, sign = (ground[jj], 1) if _BOX[b[1]]["w"] * _BOX[b[1]]["d"] <= _BOX[a[1]]["w"] * _BOX[a[1]]["d"] else (ground[ii], -1)
                mx, mz = dx * shift * sign, dz * shift * sign
                if gap < APART / 2:
                    mx, mz = mx - dx * 0.005 * sign, mz - dz * 0.005 * sign   # a hair into contact, not a hair off
                q = parts[small]
                for k, r in enumerate(parts):
                    if r[0] == "p" and (k == small or (r[5] >= 0.05 and _holds(q[1], q[2], q[3], q[4], r[2], r[3]))):
                        parts[k] = ("p", r[1], round(r[2] + mx, 3), round(r[3] + mz, 3), r[4], r[5], r[6], r[7])
                    elif r[0] == "s" and _holds(q[1], q[2], q[3], q[4], r[2], r[3]):
                        parts[k] = ("s", r[1], round(r[2] + mx, 3), round(r[3] + mz, 3), r[4], r[5], r[6])
                moved = True
        if not moved:
            break
    return parts


def _seat_on(parts):
    """A prop stood on another stands over it: at most a quarter of it hangs off (a crate on a crate). One hung on
    a wall or a post (its base under the support's top, or the support a post) isn't asked."""
    ups = [i for i, q in enumerate(parts) if q[0] == "p" and q[5] >= 0.3 and abs(q[6]) <= 20 and abs(q[7]) <= 20 and _BOX.get(q[1])]
    for i in ups:
        _, prop, x, z, yaw, y, pitch, roll = parts[i]
        under = [q for q in parts if q[0] == "p" and q is not parts[i] and q[5] < y and _BOX.get(q[1]) and _holds(q[1], q[2], q[3], q[4], x, z)]
        if not under:
            continue
        g = max(under, key=lambda q: q[5])
        gb = _BOX[g[1]]
        if y < g[5] + TOP(g[1]) - 0.02 or (gb["w"] <= 0.2 and gb["d"] <= 0.2):
            continue
        sx, sz = _rot(g[4], gb["ox"], gb["oz"])
        sx, sz = g[2] + sx, g[3] + sz
        b = _BOX[prop]
        bx, bz = _rot(yaw, b["ox"], b["oz"])
        for step in range(21):
            t = step / 20
            nx, nz = x + (sx - (x + bx)) * t, z + (sz - (z + bz)) * t
            qa = QUAD(prop, nx, nz, yaw)
            if all(_inside(p_, QUAD(g[1], g[2], g[3], g[4])) for p_ in qa) or t == 1:
                parts[i] = ("p", prop, round(nx, 3), round(nz, 3), yaw, y, pitch, roll)
                break
    return parts


def piece(name, folder, use, role, intent, parts):
    parts = _settle(list(parts))
    parts = _snap(name, parts)
    parts = _seat_on(parts)
    C[name] = (folder, use, role, intent, parts)

# ------------------------------------------------------------------- Common
piece("FirePit", "Common", "Sit", "Centre", "A fire ringed by whatever people sat on: two stumps, a crate, a sack, each a measured 0.9 m off the fire's edge. Kindling to one side, a kettle bucket.", [
    P("Fireplace_01.et", 0, 0), *RING("LogChopping_01.et", "Fireplace_01.et", 0, 0, 75), *RING("LogChopping_01.et", "Fireplace_01.et", 0, 0, 245),
    *RING("CrateWooden_01_plain.et", "Fireplace_01.et", 0, 0, 165, turn=90), *RING("SackUniversal_01.et", "Fireplace_01.et", 0, 0, 330, gap=0.7),
    P("WoodKindling_01.et", 2.3, -1.3, 102), P("Bucket_01.et", 1.3, 1.3), P("LitterInterior_USSR_01_mug.et", -1.1, 0.9),
    P("LitterInterior_USSR_01_tincan_V1.et", 1.0, -0.9)])
piece("Table", "Common", "Sit", "Centre", "The meal's table: a picnic table set a little crooked, a lamp, tins and mugs left on it, a crate pulled up across its end as one more seat.", [
    P("PicnicTable_01.et", 0, 0, 7), P("LampKerosene_01_green.et", 0.4, 0.05, 0, 0.78), P("LitterInterior_USSR_01_bowl.et", -0.4, 0.1, 0, 0.78),
    P("LitterInterior_USSR_01_mug.et", 0.1, -0.15, 0, 0.78), BESIDE("CrateWooden_01_plain.et", "PicnicTable_01.et", 0, 0, 7, "right", gap=SEAT_GAP), P("Bucket_02.et", -1.5, -1.6),
    S("RZ_Sit", -0.5, 0.8, "sit", -0.5, 0), S("RZ_Sit", 0.5, 0.85, "sit", 0.5, 0), S("RZ_Sit", -0.4, -0.8, "sit", -0.4, 0), S("RZ_Sit", 0.6, -0.75, "sit", 0.6, 0)])
piece("Tarp", "Common", "Work", "Inner", "A net strung over a work spot: crates stacked unevenly, a pallet as a floor, a stool.", [
    P("CamoNet_Small_US.et", 0, 0, 12), P("Pallet_01.et", -0.5, -0.3, 8), P("CrateWooden_01_plain.et", 0.9, 0.6, 20), P("CrateWooden_01_plain.et", 1.0, 0.5, 35, 0.62),
    P("CrateWooden_02_1x1x1.et", -1.3, 1.1, -10), P("Stool_01.et", 0.1, 0.9), P("Bucket_01.et", -1.4, -1.3),
    S("RZ_Work", -0.6, 0.4, "smoke", 0.9, 0.6), S("RZ_Work", 0.2, -0.9, "loiter", 0.9, 0.6)])
piece("FireBarrel", "Common", "Lean", "Centre", "A burning barrel with a pallet to lean on and a crate for bottles.", [
    P("BarrelMetal_01_rust.et", 0, 0), P("Campfire_burning.et", 0, 0, 0, 0.9), P("Pallet_01.et", 1.3, 0.4, 70, 0.55, 0, 80), P("CrateWooden_01_plain.et", -1.1, -0.6, -25),
    P("BeerBottle_01_empty_brown.et", -1.0, -0.6, 0, 0.62), P("BeerBottle_01_empty_brown.et", 0.6, -0.9),
    S("RZ_Lean", 0.9, 0.3, "lean_l", 0, 0), S("RZ_Lean", -0.8, 0.5, "lean_r", 0, 0)])
piece("Tent_Small_Civilian", "Common", "Sleep", "Inner", "A small tent, its flap side lived in: boots, a bag, a blanket airing on a crate.", [
    P("TentSmall_02_red.et", 0, 0, -6), P("SackUniversal_01.et", 2.0, 0.9, 60), P("BlanketFolded_01_green.et", -2.2, 1.0, 20, 0.83), P("CrateWooden_01_plain.et", -2.2, 1.0, 15),
    P("Bucket_02.et", 1.8, -1.0)])
piece("Tent_Large_Civilian", "Common", "Sleep", "Inner", "A big family tent, a chair at the door with a blanket over it, the sacks and the water bucket by its side.", [
    P("TentBig_01_orange.et", 0, 0, 4), P("ChairOld_02.et", 1.0, 2.7, 200), P("BlanketFolded_01_green.et", 1.0, 2.7, 200, 0.45),
    P("SackUniversal_01_Pile.et", 4.1, -1.2, 30), P("Bucket_01.et", -2.0, -2.6)])
piece("Firewood", "Common", "Chop", "Inner", "A chopping block in a scatter of split logs, two stacks of cut wood behind it, the wheelbarrow.", [
    P("WoodPile_02_end_A.et", -1.6, 0.1, 4), P("WoodPile_02_end_A.et", -1.6, 1.6, -3), P("LogChopping_01.et", 0.7, 0.3), P("WoodKindling_01.et", 1.6, -0.5, 31),
    P("WoodKindling_01.et", 1.5, 0.9, 77), P("WoodKindling_01.et", 0.2, -1.2, 120), P("Wheelbarrow_01.et", 3.0, 2.6, 150),
    S("RZ_Chop", 0.7, 1.1, "loiter", 0, -0.8)])
piece("CookPot", "Common", "Cook", "Inner", "A pot over a fire on a pole tripod, a grill for meat, crates as a counter, bowls drying.", [
    P("Fireplace_01.et", 0, 0), P("WoodenFenceOld_01_pole.et", 0.5, 0, 0, 0, 0, -20), P("WoodenFenceOld_01_pole.et", -0.25, 0.43, 0, 0, 18, 10), P("WoodenFenceOld_01_pole.et", -0.25, -0.43, 0, 0, -18, 10),
    P("Bucket_01.et", 0, 0, 0, 0.5), P("SkewerGrill_01.et", 1.4, 0.6, 28), P("CrateWooden_01_plain.et", -1.6, 0.4, 82), P("CrateWooden_01_plain.et", -1.7, -0.6, 95),
    P("LitterInterior_USSR_01_bowl.et", -1.6, 0.3, 0, 0.62), P("LitterInterior_USSR_01_bowl.et", -1.7, -0.5, 0, 0.62), P("SackUniversal_01.et", 1.1, -1.4, 40),
    S("RZ_Cook", 0.1, 1.3, "loiter", 0, -1.3)])
# The wash stand: the crate's back face against the post, the mirror's back against the post's front face, hung so
# its top ends 5 cm under the post's measured top; a bucket of water a step off the crate's end
_WASH_CRATE = BESIDE("CrateWooden_01_plain.et", "WoodenFenceOld_01_pole.et", 0, 0, 0, "front", yaw=90)
piece("WashStand", "Common", "Wash", "Inner", "A mirror nailed to a post, a basin and a folded towel on a crate pushed against it, a bucket of water; puddled ground.", [
    P("WoodenFenceOld_01_pole.et", 0, 0), _WASH_CRATE,
    P("MirrorBathroom_01.et", 0, round(_BOX["WoodenFenceOld_01_pole.et"]["z1"] - _BOX["MirrorBathroom_01.et"]["z0"], 3), 0,
      round(TOP("WoodenFenceOld_01_pole.et") - _BOX["MirrorBathroom_01.et"]["h"] - 0.05, 2)),
    P("Bucket_02.et", _WASH_CRATE[2] - 0.3, _WASH_CRATE[3], 0, 0.83), P("BlanketFolded_01_green.et", _WASH_CRATE[2] + 0.3, _WASH_CRATE[3], 8, 0.83),
    BESIDE("Bucket_01.et", "CrateWooden_01_plain.et", _WASH_CRATE[2], _WASH_CRATE[3], 90, "back", gap=0.3),
    S("RZ_Wash", _WASH_CRATE[2], _WASH_CRATE[3] + 0.85, "loiter", 0, -1.2)])
# Two tin sheets in an L whose faces meet at the corner (the sheet's origin is at its end, measured): one along +x,
# the other along +z with its outer face flush with the first one's end and its end against the first one's face
_TIN = _BOX["TinWall_01_3m.et"]
piece("Latrine", "Common", "None", "Edge", "Two tin sheets in an L for privacy, the bucket in its corner on a pallet, a sack of lime beside it.", [
    P("TinWall_01_3m.et", 0, 0, 0), P("TinWall_01_3m.et", round(_TIN["z0"], 3), round(_TIN["z1"], 3), -90),
    P("Pallet_01.et", round(0.3 - _BOX["Pallet_01.et"]["x0"], 3), round(_TIN["z1"] + 0.3 - _BOX["Pallet_01.et"]["z0"], 3), 0),
    P("Bucket_01.et", round(0.3 - _BOX["Pallet_01.et"]["x0"], 3), round(_TIN["z1"] + 0.3 - _BOX["Pallet_01.et"]["z0"], 3), 0, 0.14),
    P("SackUniversal_01.et", 1.9, 0.8, 70)])
piece("DryingLine", "Common", "Work", "Edge", "A clothesline frame with its lines strung, a laundry basket and a stool to stand on.", [
    P("ClotheslineFrame_01.et", 0, 0, 4), P("BasketLaundry_01.et", -0.4, 0.8, 18), P("Stool_01.et", 0.7, 0.9),
    S("RZ_Work", 0.2, 1.0, "loiter", 0, -1.0)])

piece("Duckboards", "Common", "None", "Inner", "Duckboards laid end to end: a dry walk from the tents to the fire, the ground under it trodden bare.", [
    P("Duckboard_01.et", 0, 0, 0), P("Duckboard_01.et", 0, round(_BOX["Duckboard_01.et"]["z1"] - _BOX["Duckboard_01.et"]["z0"], 3), 0)])
piece("CrateChairs", "Common", "Sit", "Centre", "Mismatched chairs pulled up to a crate for a table, a lantern on it, two more lanterns on poles.", [
    P("CrateWooden_01_plain.et", 0, 0, 14), *SEAT("ChairOld_01.et", "CrateWooden_01_plain.et", 0, 0, 14, "right"),
    *SEAT("ChairOld_02.et", "CrateWooden_01_plain.et", 0, 0, 14, "front", along=0.1), *SEAT("Stool_01.et", "CrateWooden_01_plain.et", 0, 0, 14, "left", along=-0.2),
    P("LanternMilitary_US_01.et", 0.1, 0, 0, 0.62), P("WoodenFenceOld_01_pole.et", 1.9, 1.6), P("WoodenFenceOld_01_pole.et", -2.1, 1.3),
    P("LanternMilitary_US_01.et", 1.9, 1.6, 0, 1.6), P("LanternMilitary_US_01.et", -2.1, 1.3, 0, 1.6), P("BeerBottle_01_empty_brown.et", -0.1, 0.3, 0, 0.62)])
piece("Clutter", "Common", "None", "Inner", "Life left lying about: sacks, a guitar on one, tins, bottles, a wheelbarrow tipped on a crate.", [
    P("SackUniversal_01.et", 0, 0, 30), P("SackUniversal_01.et", 0.5, 0.4, 70), P("BlanketFolded_01_green.et", 1.1, -0.3, 10), P("Guitar_01.et", 0.3, 0.2, 55, 0.25, 30, 0),
    P("LitterInterior_USSR_01_tincan_V1.et", -0.6, 0.5), P("LitterInterior_USSR_01_tincan_V2.et", -0.75, 0.3), P("BeerBottle_01_empty_brown.et", -0.4, -0.5),
    P("CrateWooden_01_plain.et", 1.9, -1.0, 40), P("Wheelbarrow_01.et", 1.6, -0.2, 120, 0, 0, 25)])
piece("RadioTable", "Common", "Sit", "Centre", "A kitchen table with a radio, a lamp and a chair pulled out; a notebook of frequencies, a crate against its end.", [
    P("TableOld_01.et", 0, 0, -4), P("RadioSmall_01_black.et", 0, -0.1, 180, 0.78), P("LampKerosene_01_green.et", 0.45, 0.1, 0, 0.78), P("Notebook_01_Stack.et", -0.35, 0.05, 20, 0.78),
    *SEAT("ChairOld_01.et", "TableOld_01.et", 0, 0, -4, "front", gap=SEAT_LEFT), BESIDE("CrateWooden_01_plain.et", "TableOld_01.et", 0, 0, -4, "left", gap=0.15)])
# The mattress's box is off its origin (x0 -1.42, x1 0.80): set so its box is centred, a pallet stood on edge
# against each long side, face to face with it, two to a side (their measured 1.23 m)
_MAT = _BOX["Junk_01_single_mattress.et"]
_MAT_X = round(-(_MAT["x0"] + _MAT["x1"]) / 2, 3)
_MAT_Z = round(-(_MAT["z0"] + _MAT["z1"]) / 2, 3)
_PAL = _BOX["Pallet_01.et"]
_PAL_X = round(_MAT["w"] / 2 + _PAL["h"] / 2, 3)
piece("ScrapShelter", "Common", "Sleep", "Inner", "A windbreak of pallets stood on edge either side of a mattress; a blanket on it and a lamp at its head.", [
    P("Junk_01_single_mattress.et", _MAT_X, _MAT_Z, 0), P("BlanketBeach_01_brown.et", 0.1, 0.1, 10, 0.4),
    P("Pallet_01.et", -_PAL_X, -_PAL["d"] / 2, 0, 0.41, 0, 90), P("Pallet_01.et", -_PAL_X, _PAL["d"] / 2, 0, 0.41, 0, 90),
    P("Pallet_01.et", _PAL_X, -_PAL["d"] / 2, 0, 0.41, 0, 90), P("Pallet_01.et", _PAL_X, _PAL["d"] / 2, 0, 0.41, 0, 90),
    P("LampKerosene_01_green.et", 0.5, round(_MAT["d"] / 2 + 0.3 + 0.09, 2))])
piece("Barricade_Fence", "Common", "None", "Approach", "An old fence panel braced with stood-up pallets, a pile of spares and a crate behind it.", [
    P("WoodenFenceOld_01_4m.et", 0, 0, 3), P("Pallet_01.et", 2.3, 0.1, 96, 0.6, 0, 90), P("Pallet_01.et", 2.5, 0.5, 110, 0.6, 0, 90), P("Pallet_01_Pile.et", -2.7, 0.7, 20),
    P("CrateWooden_01_plain.et", -1.0, 1.2, 12)])
piece("Kennel", "Common", "None", "Edge", "A dog house with a bowl and a chewed blanket. No dog.", [
    P("DogHouse_01.et", 0, 0, 12), P("LitterInterior_USSR_01_bowl.et", 0.6, 0.9), P("BlanketFolded_01_green.et", -0.6, 0.8, 40)])

# ------------------------------------------------------------------- Bastion
piece("Tent_Small_Bastion", "Bastion", "Sleep", "Inner", "A two-man tent squared to the line, kit by the flap, a sandbag holding the corner.", [
    P("TentSmallUS_01.et", 0, 0), P("CrateWooden_02_1x1x1.et", 1.8, 0.9, 0), P("Sandbag_01_single_burlap.et", -1.35, -1.3, 90), P("BlanketFolded_01_green.et", 1.8, 0.9, 0, 1.0)])
piece("Tent_Large_Bastion", "Bastion", "Sleep", "Inner", "The barrack tent, a duckboard of pallets at its door, a rack of crates down one side.", [
    P("TentUS_01_Assembled.et", 0, 0), P("Pallet_01.et", 0, 3.9, 0), P("Pallet_01.et", 0, 4.7, 0), P("CrateWooden_02_1x1x1.et", 3.0, 1.5, 0), P("CrateWooden_02_1x1x1.et", 3.0, 0.4, 0),
    P("BarrelMetal_01_grey.et", -3.0, 2.6)])
piece("SentryNest_Bastion", "Bastion", "Watch", "Approach", "A curved sandbag nest, two layers high at the front, a crate seat and an ammo-coloured crate.", [
    P("Sandbag_01_long_burlap.et", 0, 1.3, 0), P("Sandbag_01_long_burlap.et", 0, 1.3, 0, 0.45), P("Sandbag_01_long_burlap.et", -1.4, 0.75, 48), P("Sandbag_01_long_burlap.et", 1.4, 0.75, -48),
    P("Sandbag_01_end_burlap.et", -2.0, -0.2, 80), P("Sandbag_01_end_burlap.et", 2.0, -0.2, -80), P("CrateWooden_01_plain.et", 0.2, -1.3, 85), P("LitterInterior_USSR_01_mug.et", 0.2, -1.3, 0, 0.83),
    S("RZ_Watch", 0, 0.2, "loiter", 0, 10)])
piece("RosterBoard", "Bastion", "Gather", "Centre", "The roll-call board, sheets pinned unevenly, a crate with the clipboard; people stand in a loose arc.", [
    P("ChalkBoard_01.et", 0, 0), P("Notice_A4_Vertical_V1.et", 0.3, -0.04, 180, 1.3), P("Notice_A4_Vertical_V1.et", -0.25, -0.04, 176, 1.25), P("CrateWooden_01_plain.et", 1.2, -0.6, 25),
    P("FileOffice_01_Stack.et", 1.2, -0.6, 10, 0.62),
    S("RZ_Gather", -1.3, -1.7, "loiter", 1.3, 1.7), S("RZ_Gather", -0.4, -2.0, "loiter", 0.4, 2.0), S("RZ_Gather", 0.6, -1.9, "loiter", -0.6, 1.9), S("RZ_Gather", 1.5, -1.5, "loiter", -1.5, 1.5)])
piece("Barricade_Sandbag", "Bastion", "None", "Approach", "A sandbag wall, its ends tapered and turned in a little, a spare bag dropped behind it.", [
    P("Sandbag_01_long_burlap.et", 0, 0, 2), P("Sandbag_01_end_burlap.et", 2.75, 0.2, -12), P("Sandbag_01_end_burlap.et", -2.85, 0.05, 183),
    P("Sandbag_01_single_burlap.et", 0.3, -1.0, 40)])
piece("FlagPole", "Bastion", "None", "Centre", "An empty flag pole in a ring of sandbags. The Zone flies none.", [
    P("FlagPole_01_empty.et", 0, 0), P("Sandbag_01_single_burlap.et", 0.4, 0, 0), P("Sandbag_01_single_burlap.et", -0.38, 0.1, 10), P("Sandbag_01_single_burlap.et", 0.05, 0.4, 92),
    P("Sandbag_01_single_burlap.et", -0.05, -0.4, 85)])
piece("ShavingStand", "Bastion", "Wash", "Inner", "A mirror on a post at head height, a bucket on a stool, a folded towel on an ammo crate; soap tins.", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("MirrorBathroom_01.et", 0, 0.08, 0, 1.5), P("Stool_01.et", 0.1, 0.5), P("Bucket_02.et", 0.1, 0.5, 0, 0.45),
    P("CrateWooden_02_1x1x1.et", 1.0, 0.2, -8), P("BlanketFolded_01_green.et", 1.0, 0.2, -8, 1.0), P("LitterInterior_USSR_01_tincan_V1.et", -0.4, 0.4),
    S("RZ_Wash", 0.05, 1.1, "loiter", 0, -1.1)])
piece("CrateSeats", "Bastion", "Sit", "Centre", "Four crates round a bigger crate as a table, one pushed back, cards and mugs.", [
    P("CrateWooden_02_1x1x1.et", 0, 0, 8), P("CrateWooden_01_plain.et", 0.1, 1.1, 4), P("CrateWooden_01_plain.et", -0.05, -1.15, -6), P("CrateWooden_01_plain.et", 1.2, 0.1, 92),
    P("CrateWooden_01_plain.et", -1.4, -0.2, 70), P("LitterInterior_USSR_01_mug.et", 0.2, 0.2, 0, 1.0), P("LitterInterior_USSR_01_mug.et", -0.25, -0.1, 0, 1.0),
    S("RZ_Sit", 0.1, 1.1, "sit", 0, 0), S("RZ_Sit", -0.05, -1.15, "sit", 0, 0), S("RZ_Sit", 1.2, 0.1, "sit", 0, 0), S("RZ_Sit", -1.4, -0.2, "sit", 0, 0)])
piece("DutyBoard", "Bastion", "Gather", "Centre", "The duty rota on a chalkboard, a crate of paperwork beside it.", [
    P("ChalkBoard_01.et", 0, 0, -5), P("Notice_A4_Vertical_V1.et", -0.3, -0.04, 175, 1.3), P("CrateWooden_01_plain.et", -1.0, -0.4, 60), P("FileOffice_01_Stack.et", -1.0, -0.4, 0, 0.62),
    S("RZ_Gather", -0.5, -1.6, "loiter", 0.5, 1.6), S("RZ_Gather", 0.6, -1.5, "loiter", -0.6, 1.5)])

# ------------------------------------------------------------------- Red Hand
piece("SentryNest_RedHand", "RedHand", "Watch", "Approach", "A lookout behind a tyre pile and a tipped pallet, an armchair someone dragged out to sit in.", [
    P("TirePile_01_small.et", -0.9, 1.9), P("Pallet_01.et", 0.6, 1.3, -8, 0.6, 0, 85), P("Sandbag_01_long_burlap.et", 2.1, 0.9, -60), P("Armchair_01.et", 0, 0, 175),
    P("BeerBottle_01_group.et", -0.8, -0.4),
    S("RZ_Watch", 0, 0.1, "sit", 0, 10)])
piece("CardTable", "RedHand", "Sit", "Centre", "A kitchen table, crates pulled up as benches and a chair round it, bottles and a lamp; a barrel as a bin.", [
    P("TableOld_01.et", 0, 0, 12), BESIDE("CrateWooden_01_plain.et", "TableOld_01.et", 0, 0, 12, "front", gap=SEAT_GAP, yaw=102),
    *SEAT("ChairOld_02.et", "TableOld_01.et", 0, 0, 12, "right"), BESIDE("CrateWooden_01_plain.et", "TableOld_01.et", 0, 0, 12, "left", gap=SEAT_GAP),
    P("BeerBottle_01_group.et", 0.25, 0.1, 0, 0.78), P("LampKerosene_01_green.et", -0.35, -0.1, 0, 0.78), P("KS_RadioStation.et", 0.45, -0.2, 190, 0.9), P("BarrelMetal_01_rust.et", -1.8, 1.5),
    S("RZ_Sit", 0.1, 1.0, "sit", 0, -1.0), S("RZ_Sit", -1.3, -0.3, "sit", 1.1, 0.4)])
piece("TollBarrier", "RedHand", "Lean", "Approach", "Logs dragged across the road on tyres, a chair by a barrel fire for whoever takes the toll.", [
    P("WoodLog_01_A.et", 0, 1.5, 92), P("WoodLog_01_A.et", 0.2, 1.5, 86, 0.35), P("Tire_UAZ469.et", -1.6, 1.5, 0, 0, 90), P("Tire_UAZ469.et", 1.7, 1.4, 0, 0, 90),
    P("BarrelMetal_01_rust.et", 2.9, 0.7), P("Campfire_burning.et", 2.9, 0.7, 0, 0.9), *RING("ChairOld_01.et", "BarrelMetal_01_rust.et", 2.9, 0.7, 215, gap=0.6, slot="RZ_Lean", anim="smoke"),
    S("RZ_Lean", -1.8, 0.5, "lean_l", 0, 3)])
piece("Barricade_Scrap", "RedHand", "None", "Approach", "Tin sheets lashed at angles, a tyre pile behind them and junk at one end, a pallet wedged in.", [
    P("TinWall_01_3m.et", -0.8, 0, 6), P("TinWall_01_3m.et", 1.4, 0.4, -18), P("TirePile_01_small.et", 3.2, -1.2), P("Junk_01_pile_small.et", -3.3, 0.5, 30), P("Pallet_01.et", 0.4, -0.3, 10, 0.6, 0, 80)])
piece("RedHandSheet", "RedHand", "None", "Approach", "A tin sheet stood on two tyre piles, facing the road (the red hand decal still to come).", [
    P("TirePile_01_small.et", -1.0, 0), P("TirePile_01_small.et", 1.1, 0.1), P("TinWall_01_3m.et", 0, 0.05, 2, 0.6)])
piece("TrophyWall", "RedHand", "None", "Approach", "A long tin wall by the way in, hung with skulls on boards that look out at whoever comes up the road; a tyre leaning on it, junk in front of it.", [
    P("TinWall_01_6m_A.et", 0, 0, 4), P("DeerSkull_01_board.et", -1.6, 0.32, 184, 1.6), P("DeerSkull_01_board.et", 0.3, 0.34, 184, 1.8), P("DeerSkull_01_board.et", 1.9, 0.36, 184, 1.5),
    P("Tire_UAZ469.et", -2.6, 0.5, 0, 0.3, 75), P("Junk_01_pile_small.et", 2.6, 2.4, 20)])
piece("Sofa", "RedHand", "Sit", "Centre", "A sofa and an armchair turned to a barrel side table, each an arm's reach off it, bottles everywhere.", [
    P("BarrelMetal_01_rust.et", 0, 0), P("BeerBottle_01_group.et", 0, 0, 0, 0.9), P("LitterInterior_01_vodka_V1.et", 0.15, -0.15, 0, 1.1), *RING("Sofa_02_green.et", "BarrelMetal_01_rust.et", 0, 0, 185, gap=0.35),
    *RING("Armchair_01.et", "BarrelMetal_01_rust.et", 0, 0, 95, gap=0.35),
    P("BeerBottle_01_empty_brown.et", -1.4, -0.5), P("BeerBottle_01_empty_brown.et", 0.6, -0.8), P("Tire_UAZ469.et", -2.0, 0.2, 0, 0, 90)])

# ------------------------------------------------------------------- Haven
piece("WashingLine", "Haven", "Work", "Edge", "Two clothesline frames end to end across the yard, the washing on them, a basket and a bucket.", [
    P("ClotheslineFrame_01.et", -1.7, 0, 2), P("ClotheslineFrame_01.et", 1.7, 0.1, -2), P("BasketLaundry_01.et", -0.7, 0.9, 15), P("Bucket_01.et", 0.5, 1.0),
    S("RZ_Work", -0.2, 1.0, "loiter", 0, -1.0)])
piece("Garden", "Haven", "Work", "Edge", "Two vegetable beds side by side with a step between them, sacks of seed and a bucket at their end, the wheelbarrow beside them.", [
    P("VegetableBed_01_small.et", -1.45, 0, 2), P("VegetableBed_01_small.et", 1.45, 0.1, -2), P("SackUniversal_01.et", 3.4, 1.0, 30), P("SackUniversal_01.et", 3.6, 0.3, 75),
    P("Wheelbarrow_01.et", 0.2, -2.6, 60), P("Bucket_02.et", -0.1, 1.9),
    S("RZ_Work", 0.6, 1.8, "loiter", 0.6, 0)])
piece("WaterBarrel", "Haven", "Work", "Inner", "A rain barrel and a second one beside it, buckets, a ladle mug on the lid.", [
    P("BarrelMetal_01_blue.et", 0, 0), P("BarrelMetal_01_grey.et", 0.7, 0.2), BESIDE("Bucket_01.et", "BarrelMetal_01_blue.et", 0, 0, 0, "left"), BESIDE("Bucket_02.et", "BarrelMetal_01_grey.et", 0.7, 0.2, 0, "front"), P("LitterInterior_USSR_01_mug.et", 0, 0, 0, 0.9),
    S("RZ_Work", 0, 1.0, "loiter", 0, -1.0)])
piece("BearDoor", "Haven", "None", "Approach", "A pallet stood against a post at the way in for a board (the bear decal still to come), a flowerpot.", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("Pallet_01.et", 0, round(_BOX["WoodenFenceOld_01_pole.et"]["z1"] + _BOX["Pallet_01.et"]["h"] / 2, 3), 0, 0.62, 80),
    P("FlowerpotFlowers_01_begonia_white.et", 0.9, 0.6)])
piece("Memorial", "Haven", "Work", "Centre", "A white table with candles, photos and names; flowers in front.", [
    P("TableOld_01_white.et", 0, 0, -3), P("CandleHolderDecorative_01.et", -0.4, 0, 0, 0.78), P("CandleHolderDecorative_01.et", 0, 0.12, 0, 0.78), P("CandleHolderDecorative_01.et", 0.4, -0.05, 0, 0.78),
    P("TablePhotoFrame_01.et", 0.2, -0.2, 180, 0.78), P("Photo_Square_01_V1.et", -0.2, -0.2, 10, 0.79), P("Notice_A4_Vertical_V1.et", 0, 0.25, 0, 0.79),
    P("FlowerpotFlowers_01_begonia_white.et", 0.8, 0.7), P("FlowerpotFlowers_01_begonia_white.et", -0.7, 0.8),
    S("RZ_Work", 0, 1.1, "loiter", 0, -1.1)])
piece("ChildrensCorner", "Haven", "None", "Inner", "A low chalkboard, a blanket on the ground, two stools, chalk tins.", [
    P("ChalkBoard_01.et", 0, 0, 5), P("BlanketBeach_01_red.et", 0.1, 1.3, 70), P("Stool_01.et", -0.9, 1.0), P("Stool_01.et", 0.9, 1.2),
    P("LitterInterior_USSR_01_tincan_V2.et", 0.3, 0.6)])
piece("VegStall", "Haven", "Work", "Edge", "A market stand with crates and sacks, a bucket of water, a chair behind it for whoever minds it.", [
    P("MarketStand_01.et", 0, 0), P("CrateWooden_01_plain.et", -0.6, -0.2, 6), P("CrateWooden_01_plain.et", 0.6, -0.2, -4), P("SackUniversal_01_Pile.et", 1.9, 0.9, 20),
    P("Bucket_01.et", -1.6, 0.6), *SEAT("ChairOld_01.et", "MarketStand_01.et", 0, 0, 0, "back", gap=0.3, slot="RZ_Work", anim="loiter")])

# ------------------------------------------------------------------- Halcyon
piece("Tent_Halcyon", "Halcyon", "Work", "Inner", "The ward tent, its flap to the yard: a field table out front with a sample drum and a lamp, a crate of cases by the door.", [
    P("TentFIA_01.et", 0, 0), P("TableMilitary_US_01.et", 0, 3.4), P("ToolsSurgical_01_autoclavedrum.et", 0.5, 3.4, 0, 0.78),
    P("LampKerosene_01_green.et", -0.6, 3.45, 0, 0.78), P("CrateWooden_02_1x1x1.et", 2.0, 2.9, 6),
    S("RZ_Work", 0, 4.2, "loiter", 0, -1.0)])
piece("FieldTable", "Halcyon", "Sit", "Centre", "A field table with notebooks and a sample drum, two stools pulled up, the cases stacked against its end with a lantern on top.", [
    P("TableMilitary_US_01.et", 0, 0), P("Notebook_01_Stack.et", 0.4, 0, 10, 0.78), P("FileOffice_01_Stack.et", -0.5, 0.1, -5, 0.78), P("ToolsSurgical_01_autoclavedrum.et", 0, -0.1, 0, 0.78),
    *SEAT("Stool_01.et", "TableMilitary_US_01.et", 0, 0, 0, "front", along=0.1), *SEAT("Stool_01.et", "TableMilitary_US_01.et", 0, 0, 0, "back", along=0.2),
    BESIDE("CrateWooden_02_1x1x1.et", "TableMilitary_US_01.et", 0, 0, 0, "right", gap=0.1, yaw=4), P("LanternMilitary_US_01.et", 1.27, 0, 0, 1.0)])
piece("ColdBox", "Halcyon", "Work", "Inner", "The cold store: sample cases stacked on a pallet off the wet ground, a drum on top, a barrel of cooling water beside it.", [
    P("Pallet_01.et", 0, 0, 3), P("CrateWooden_02_1x1x1.et", -0.1, 0, 3, 0.15), P("CrateWooden_01_plain.et", 0.5, 0.05, -8, 1.15), P("ToolsSurgical_01_autoclavedrum.et", -0.2, 0, 0, 1.15),
    P("BarrelMetal_01_blue.et", 1.2, 0.1),
    S("RZ_Work", 0, 1.0, "loiter", 0, -1.0)])
piece("Antenna", "Halcyon", "Work", "Edge", "The antenna on a stand with a radio on a crate and a stool pulled up to it; a cable run to the generator.", [
    P("Antenna_R161_01.et", 0, 0), P("CrateWooden_01_plain.et", 1.0, 0.6, 15), P("RadioSmall_01_black.et", 1.0, 0.6, 195, 0.62),
    *SEAT("Stool_01.et", "CrateWooden_01_plain.et", 1.0, 0.6, 15, "right", slot="RZ_Work", anim="loiter")])
piece("Generator", "Halcyon", "Work", "Edge", "The generator on a pallet behind a row of sandbags, a fuel-coloured barrel, a toolbox crate.", [
    P("Pallet_01.et", 0, 0, 5), P("GeneratorPortable_US_01_Off.et", 0, 0, 5, 0.15), P("BarrelMetal_01_grey.et", -1.1, 0.4), P("CrateWooden_01_plain.et", 1.0, -0.6, 70),
    P("Sandbag_01_long_burlap.et", 0.1, -1.6, 2),
    S("RZ_Work", 0, 1.0, "loiter", 0, -1.0)])
piece("SampleTable", "Halcyon", "Work", "Inner", "The sorting table under a net: sample drums and the log on a field table, the cases under it, worked standing.", [
    P("CamoNet_Small_US.et", 0, 0, 4), P("TableMilitary_US_01.et", 0, -0.3), P("ToolsSurgical_01_autoclavedrum.et", -0.5, -0.3, 0, 0.78), P("ToolsSurgical_01_autoclavedrum.et", -0.2, -0.35, 0, 0.78),
    P("Notebook_01_Stack.et", 0.5, -0.25, 12, 0.78), P("CrateWooden_01_plain.et", 1.3, -0.3, 2), P("LanternMilitary_US_01.et", 1.3, -0.3, 0, 0.83),
    S("RZ_Work", -0.3, 0.6, "loiter", 0, -1.0), S("RZ_Work", 0.6, 0.6, "loiter", 0, -1.0)])
piece("DeconShower", "Halcyon", "Wash", "Inner", "A shower on a pallet floor, a tin sheet stood up on each side for a screen, buckets and a bench for the clothes.", [
    P("Pallet_01.et", 0, 0), P("ShowerBeach_01.et", 0, 0, 0, 0.15), P("TinWall_01_3m.et", 0.75, -0.2, 90), P("TinWall_01_3m.et", -0.75, -0.2, 90),
    P("Bucket_02.et", -1.4, 1.3), P("ForestBench_01.et", 0.2, 2.5, 4),
    S("RZ_Wash", 0, 0, "loiter", 0, 1.0)])
piece("PlagueBoard", "Halcyon", "None", "Approach", "A no-entry sign on its pole at the way in (the beak stencil still to come), a white crate at its foot.", [
    P("SignIndustrial_USSR_02_warnings_entryforbidden_pole.et", 0, 0, 0),
    BESIDE("CrateWooden_01_plain.et", "SignIndustrial_USSR_02_warnings_entryforbidden_pole.et", 0, 0, 0, "back", gap=0.4, yaw=90)])
piece("NotebookBench", "Halcyon", "Sit", "Centre", "A bench with notebooks and files, a lamp and a crate of sample drums.", [
    P("ForestBench_01.et", 0, 0, 3), P("Notebook_01_Stack.et", 0.7, 0.05, 10, 0.45), P("FileOffice_01_Stack.et", -0.7, 0, 0, 0.45), P("LampKerosene_01_green.et", 1.2, 0.3),
    P("CrateWooden_01_plain.et", -1.3, 0.8, 25), P("ToolsSurgical_01_autoclavedrum.et", -1.3, 0.8, 0, 0.62),
    S("RZ_Sit", -0.35, 0.3, "sit", 0, 2.0), S("RZ_Sit", 0.35, 0.3, "sit", 0, 2.0)])

piece("GuardPost_Halcyon", "Halcyon", "Watch", "Approach", "Two stools behind a short sandbag run, a lantern on a crate: the guards who watch the gate and the lights.", [
    P("Sandbag_01_long_burlap.et", 0, 1.1, 0), P("Stool_01.et", -0.5, 0.1), P("Stool_01.et", 0.6, 0.0), P("CrateWooden_01_plain.et", 1.6, 0.3, 80),
    P("LanternMilitary_US_01.et", 1.6, 0.3, 0, 0.62),
    S("RZ_Watch", -0.5, 0.1, "sit", 0, 10), S("RZ_Watch", 0.6, 0.0, "loiter", 0, 10)])
piece("SmokingBench", "Halcyon", "Sit", "Edge", "A bench by the back door for the night shift, a tin of butts and a lantern.", [
    P("ForestBench_01.et", 0, 0, 2), P("LitterInterior_USSR_01_tincan_V1.et", 1.3, 0.4), P("LitterInterior_USSR_01_tincan_V2.et", 1.4, 0.2), P("LanternMilitary_US_01.et", -1.3, 0.3),
    S("RZ_Sit", -0.35, 0.3, "sit", 0, 2.0), S("RZ_Sit", 0.35, 0.3, "smoke", 0, 2.0)])

# ------------------------------------------------------------------- From the reference board (docs/SITES.md)
# Ideas taken from film, TV and game bases, built from vanilla props. The idea each came from is in its line.
piece("Bunker_Bastion", "Bastion", "Watch", "Approach", "The gate's bunker: chest-high sandbags on three sides, another course on the front, a pallet floor, a crate and a lamp inside (the fortified nest at the entrance of an army camp).", [
    P("Sandbag_01_long_high_burlap.et", 0, 1.8, 0), P("Sandbag_01_long_high_burlap.et", -2.1, 0.2, 90), P("Sandbag_01_long_high_burlap.et", 2.1, 0.2, 90),
    P("Sandbag_01_long_burlap.et", 0, 1.8, 0, 1.2), P("Pallet_01.et", 0, -0.6, 6), P("CrateWooden_01_plain.et", 0.8, -0.5, 10), P("LanternMilitary_US_01.et", 0.8, -0.5, 0, 0.62), P("Sandbag_01_end_burlap.et", -1.3, -1.5, 80),
    S("RZ_Watch", -0.2, 0.5, "loiter", 0, 10)])
piece("MotorPool", "Bastion", "Work", "Edge", "The workshop on the forecourt: a dead truck kept for parts, an engine crane at its nose, a jack, fuel drums and jerrycans by the cab, a stack of tyres, a crate with the toolbox on it.", [
    P("Ural4320_wreck_static.et", 0, 0, 6), P("BarrelMetal_01_grey.et", 2.5, 2.2), P("BarrelMetal_01_grey.et", 3.2, 2.0), P("Jerrycan_01.et", 2.6, 1.3, 30), P("Jerrycan_01.et", 3.0, 1.2, -10),
    P("TirePile_01_small.et", -3.4, -2.4), P("CrateWooden_01_plain.et", 2.6, -0.6, 15), P("ToolBox_01_red.et", 2.6, -0.6, 25, 0.84),
    P("EngineCrane_01.et", 0.3, 5.0, 186), P("HydraulicCarJack_01.et", -2.0, 1.6, 20),
    S("RZ_Work", 2.0, 0.4, "loiter", 0, 0.4)])
piece("FieldKitchen", "Bastion", "Cook", "Inner", "The field kitchen on its trailer, still serving: a table for the mess tins, a water drum, kindling (dinner is still served at the blockade).", [
    P("FieldKitchenTrailerUSSR_01.et", 0, 0, 0), P("TableMilitary_US_01.et", 2.7, 0.2, 90), P("LitterInterior_USSR_01_bowl.et", 2.7, 0.4, 0, 0.78), P("LitterInterior_USSR_01_mug.et", 2.6, -0.3, 0, 0.78),
    P("BarrelMetal_01_blue.et", -1.9, 0.9), P("WoodKindling_01.et", -1.8, -1.0, 40),
    S("RZ_Cook", 1.4, 0.2, "loiter", 0, 0.2)])
piece("AidPost", "Bastion", "Work", "Inner", "The aid post: a small tent, a table of instruments beside it, a cot outside under a drip for the one who can't be moved, a screen, a crate of dressings and a bucket, a blanket airing (the busiest tent of a camp short of everything).", [
    P("TentSmallUS_01.et", 0, 0), P("TableMilitary_US_01.et", 2.3, 0.6, 85), P("ToolsSurgical_01_autoclavedrum.et", 2.3, 0.9, 0, 0.78), P("CrateWooden_01_plain.et", -1.6, 1.2, 12), P("Bucket_02.et", -1.5, 0.3),
    P("BlanketFolded_01_green.et", -1.6, 1.2, 12, 0.62), P("Stool_01.et", 1.5, 1.6),
    P("CotMilitary_US_01.et", 2.6, -1.9, 4), P("InfusionStand_01.et", 4.0, -1.3), P("MedicalScreen_01.et", 2.6, -2.9, 2), P("BinMedical_01.et", 3.6, 0.9),
    S("RZ_Work", 1.5, 0.7, "loiter", 2.3, 0.7)])
piece("Floodlight_Bastion", "Bastion", "None", "Edge", "A floodlight mast on its trailer, lit, facing out over the wire (the lawn is the killing ground).", [P("GeneratorFloodlight_USSR_01.et", 0, 0, 180)])
piece("Scrap_RedHand_Wreck", "RedHand", "None", "Approach", "A car wreck dragged into the line, a tyre against its wheel (a wall made of what no longer drives).", [
    P("S105_wreck_static.et", 0, 0, 90), P("Tire_UAZ469.et", 1.2, 0.9, 0, 0, 90)])
piece("WreckGate", "RedHand", "None", "Approach", "The way in, narrowed by a van on its rims: tyres stacked at its nose, an oil drum, a jeep wreck behind it for cover.", [
    P("S1203_wreck_static.et", 0, 0, 78), P("TirePile_01_small.et", 2.9, 0.6), P("BarrelMetal_01_rust.et", -2.8, 0.7), P("UAZ469_wreck_rusty_static.et", 0.6, -3.2, 110)])
piece("FuelDump", "RedHand", "Work", "Edge", "The crew's fuel: drums on a pallet, jerrycans in a row, a tyre to sit on (what the toll is paid in).", [
    P("Pallet_01.et", 0, 0, 5), P("BarrelMetal_01_rust.et", -0.3, 0.1, 0, 0.15), P("BarrelMetal_01_blue.et", 0.35, -0.1, 0, 0.15), P("BarrelMetal_01_rust.et", 1.2, 0.5),
    P("Jerrycan_01.et", -1.1, 0.6, 20), P("Jerrycan_01.et", -1.1, 0.2, 8), P("Jerrycan_01.et", -1.2, -0.2, -15), P("Tire_UAZ469.et", 0.9, -1.1),
    S("RZ_Work", 0.9, -1.1, "smoke", 0, 0)])
piece("Watchtower_Haven", "Haven", "Watch", "Approach", "The lookout: a tower knocked together by the gate, a lantern on its post, a chair at its foot for the one who can't climb (a scaffold tower in a churchyard).", [
    P("LifeGuardTower_01_darkgreen.et", 0, 0, 180), P("ChairOld_01.et", 1.9, 0.4, 180), P("WoodenFenceOld_01_pole.et", -1.7, 0.6), P("LanternMilitary_US_01.et", -1.7, 0.6, 0, 1.6),
    S("RZ_Watch", 1.9, 0.3, "sit", 1.9, 10)])
piece("WorkBoard", "Haven", "Gather", "Approach", "The work board inside the gate: who's on the wall, who's on the wood, who's on the pot; a crate to stand the chalk on (the patrol board by a town's gate).", [
    P("ChalkBoard_01.et", 0, 0, 0), P("Notice_A4_Vertical_V1.et", -0.3, -0.04, 180, 1.3), P("Notice_A4_Vertical_V1.et", 0.25, -0.04, 180, 1.25), P("CrateWooden_01_plain.et", -1.1, -0.3, -15),
    S("RZ_Gather", 0, -1.2, "loiter", 0, 0)])
piece("LanternPost", "Haven", "None", "Inner", "A lantern on a post by the path, lit at dusk (a string of lights across a yard).", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("LanternMilitary_US_01.et", 0, 0, 0, 1.6)])
piece("Gate_Halcyon", "Halcyon", "None", "Approach", "The gate: a road block pulled half across, cones, a no-entry sign on its pole (the boom and the gatehouse of a lab in the woods).", [
    P("RoadBlock_01.et", -1.6, 0, 4), P("ConeTraffic_01_red.et", 0.2, 0.3), P("ConeTraffic_01_red.et", 0.9, 0.5), P("ConeTraffic_01_red.et", 1.6, 0.2),
    P("SignIndustrial_USSR_02_warnings_entryforbidden_pole.et", -3.0, 0.2, 0)])
piece("WarningSigns", "Halcyon", "None", "Approach", "No-entry signs on the way in, well before the building, and a cone (restricted-area signs before you see the lab).", [
    P("SignIndustrial_USSR_02_warnings_entryforbidden_pole.et", -0.8, 0, 6), P("SignIndustrial_USSR_02_warnings_entryforbidden_pole.et", 0.8, 0.1, -8), P("ConeTraffic_01_red.et", 0, 0.5)])
piece("Loudspeaker_Halcyon", "Halcyon", "None", "Edge", "A loudspeaker on a post, wired to the generator (the order that loops once the power is back).", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("Loudspeaker.et", 0, 0, 0, 1.7), P("CrateWooden_01_plain.et", 0.7, 0.2, 25)])
piece("Triage", "Halcyon", "Work", "Approach", "Triage inside the gate: a table where everyone is looked at before they go further, a stool each side of it, a screen to be looked at behind, a drip stand, a crate of forms, a sample drum, a bin.", [
    P("TableMilitary_US_01.et", 0, 0, 0), *SEAT("Stool_01.et", "TableMilitary_US_01.et", 0, 0, 0, "back", along=0.5, slot="RZ_Work"),
    *SEAT("Stool_01.et", "TableMilitary_US_01.et", 0, 0, 0, "front", along=-0.6, slot=""), P("FileOffice_01_Stack.et", 0.5, 0, 0, 0.78),
    P("ToolsSurgical_01_autoclavedrum.et", -0.5, 0.05, 0, 0.78), BESIDE("CrateWooden_02_1x1x1.et", "TableMilitary_US_01.et", 0, 0, 0, "right", gap=0.6, yaw=8),
    P("MedicalScreen_01.et", -1.9, 0.6, 80), P("InfusionStand_01.et", -1.7, -0.6), P("BinMedical_01.et", 1.1, -0.9)])

# ------------------------------------------------------------------- Dense sites (docs/SITES.md, "Dense sites")
# Small set dressing a dense kit repeats many times: stores, fuel, tyres, cover, light, beds.
piece("CrateStack", "Common", "None", "Inner", "Crates stacked against a wall, one on top, a pallet leaning on them.", [
    P("CrateWooden_02_1x1x1.et", 0, 0, 3), P("CrateWooden_02_1x1x1.et", 1.05, 0.05, -4), P("CrateWooden_02_1x1x1.et", 0.5, 0, 6, 1.0), P("CrateWooden_01_plain.et", -1.0, 0.1, 12),
    P("CrateWooden_01_plain.et", -1.0, 0.1, -8, 0.62), P("Pallet_01.et", 1.9, 0.2, 80, 0.55, 0, 78)])
piece("PalletStack", "Common", "None", "Inner", "Pallets piled, a second pile beside them, sacks stacked at the end.", [
    P("Pallet_01_Pile.et", 0, 0, 4), P("Pallet_01_Pile.et", 1.4, 0.1, -6), P("SackUniversal_01_Pile.et", 0.1, 1.3, 20)])
piece("BarrelGroup", "Common", "None", "Inner", "Fuel and water drums stood together, two jerrycans, a bucket.", [
    P("BarrelMetal_01_grey.et", 0, 0), P("BarrelMetal_01_grey.et", 0.82, 0.1), P("BarrelMetal_01_blue.et", 0.4, 0.78), P("BarrelMetal_01_rust.et", -0.82, 0.3),
    P("Jerrycan_01.et", 1.5, 0.55, 25), P("Jerrycan_01.et", 1.55, 0.1, -10), P("Bucket_01.et", -0.3, -0.75)])
piece("TyreStack", "Common", "None", "Inner", "Tyres stacked, one leaning on the stack, one on the ground.", [
    P("TirePile_01_small.et", 0, 0), P("Tire_UAZ469.et", 1.1, 0.3, 0, 0, 90), P("Tire_UAZ469.et", -0.2, 1.1)])
piece("Bedrolls", "Common", "Sleep", "Inner", "Blankets laid out under the sky, packs at their heads, a lantern between them.", [
    P("BlanketBeach_01_brown.et", -1.0, 0, 2), P("BlanketBeach_01_red.et", 1.0, 0.1, -2), P("SackUniversal_01.et", -1.0, 1.25, 30), P("SackUniversal_01.et", 1.0, 1.3, -20),
    P("LanternMilitary_US_01.et", 0, 1.1)])
# Filled sandbags laid end to end in two rows that touch, a second layer over the joints, the spare sacks a step off
_BAG = _BOX["Sandbag_01_single_burlap.et"]
piece("SandbagPile", "Bastion", "None", "Inner", "Filled sandbags waiting to be laid: two rows end to end, a second layer on top, spare sacks beside them.", [
    *ROW("Sandbag_01_single_burlap.et", 0, 0, 0, 3), *ROW("Sandbag_01_single_burlap.et", 0, round(_BAG["z0"] - _BAG["z1"], 3), 0, 3),
    *[P(q[1], round(q[2] + _BAG["w"] / 2, 3), q[3], 0, 0.24) for q in ROW("Sandbag_01_single_burlap.et", 0, round((_BAG["z0"] - _BAG["z1"]) / 2, 3), 0, 2)],
    P("SackUniversal_01.et", round(_BAG["x0"] + 3 * _BAG["w"] + 0.3 - _BOX["SackUniversal_01.et"]["x0"], 3), -0.2, 0)])
piece("SandbagWall", "Bastion", "None", "Inner", "A chest-high sandbag wall two sections long inside the wire: cover between the gate and the door.", [
    P("Sandbag_01_long_high_burlap.et", -1.7, 0, 0), P("Sandbag_01_long_high_burlap.et", 1.7, 0.05, 2)])
piece("MessTable", "Bastion", "Sit", "Centre", "A mess table: a long table with a bench pulled up down each side, tins, mugs, the food carrier and a lamp left on it.", [
    P("TableLong_01.et", 0, 0, 0), *SEAT("BenchWooden_01.et", "TableLong_01.et", 0, 0, 0, "front", along=-0.1), *SEAT("BenchWooden_01.et", "TableLong_01.et", 0, 0, 0, "back", along=0.1),
    P("LitterInterior_USSR_01_bowl.et", -0.6, 0.1, 0, 0.9), P("LitterInterior_USSR_01_mug.et", 0.2, -0.1, 0, 0.9),
    P("LitterInterior_USSR_01_tincan_V1.et", 0.6, 0.15, 0, 0.9), P("LanternMilitary_US_01.et", 0, 0.05, 0, 0.9), P("CarrierFood_01.et", -0.9, -0.15, 0, 0.9)])
piece("CheckpointSign", "Bastion", "None", "Approach", "A stop sign on its stand, a cone either side, a lamp at its foot.", [
    P("SignCheckpoint_01_stop.et", 0, 0, 0), P("ConeTraffic_01_red.et", -0.9, 0.2), P("ConeTraffic_01_red.et", 0.9, 0.3), P("LanternMilitary_US_01.et", 0.4, -0.3)])
piece("JeepWreck", "Bastion", "None", "Edge", "A Humvee that will never run again, kept for its parts: a wheel off beside it, a crate of tools against its side, a jerrycan.", [
    P("M998_wreck_static.et", 0, 0, -8), BESIDE("CrateWooden_01_plain.et", "M998_wreck_static.et", 0, 0, -8, "right", gap=0.3),
    BESIDE("Tire_UAZ469.et", "M998_wreck_static.et", 0, 0, -8, "left", gap=0.4), BESIDE("Jerrycan_01.et", "M998_wreck_static.et", 0, 0, -8, "right", along=-1.5, gap=0.3)])
piece("GeneratorPost", "Bastion", "Work", "Edge", "A generator behind a sandbag end, its fuel beside it, a crate of cable.", [
    P("GeneratorPortable_US_01_Off.et", 0, 0, 0), P("Sandbag_01_end_burlap.et", 0, 1.35, 0), P("Jerrycan_01.et", 0.8, 0.2, 30), P("CrateWooden_01_plain.et", -1.3, -0.3, 10),
    S("RZ_Work", 0, -0.9, "loiter", 0, 0)])
piece("RadioMast", "Bastion", "Work", "Inner", "The radio: a mast, a table with the set and a lamp, a stool pulled up to it.", [
    P("Antenna_US_02.et", 0, 0), P("TableMilitary_US_01.et", 1.6, 0, 90), P("RadioStation_ANPRC77_01.et", 1.6, 0.2, 0, 0.9),
    *SEAT("Stool_01.et", "TableMilitary_US_01.et", 1.6, 0, 90, "front", slot="RZ_Work"), P("LanternMilitary_US_01.et", 1.6, -0.5, 0, 0.78)])

# ------------------------------------------------------------------- From the mods the project loads already (docs/MOD_STACK.md, "The camps' look")
# Pieces no vanilla prop gives: riot fencing (MoreGMProps), security
# mesh (SecurityFences), a guard tower, a container, one forklift, stacked cases and a fuel pump (IH_Better_GM_Props, which
# Timmmmmbs Garbage Composition brings). Tried and left out: IH's diesel generator (the size of a van, bright green) and
# SBBG Wrecks' armoured car and ambulance props (not seen standing where they were put), MoreGMProps' blast walls,
# earth-filled barriers and concrete barriers (the author prefers razor wire to concrete). MoreGMProps' burn pile isn't a
# kit piece: the dead zone holds only what a site's story puts there (docs/SITES.md, "Zones", "The elements").
piece("Watchtower_Bastion", "Bastion", "Watch", "Approach", "A guard tower that sees the gate and the road in; nobody trusts its ladder any more, so the sentry stands at its foot.", [
    P("GuardTower_destructible.et", 0, 0, 0),
    S("RZ_Watch", 2.9, 2.9, "loiter", 2.9, 14)])
piece("SearchTable", "Bastion", "Work", "Approach", "Where packs are opened before anyone goes further: a table, a crate for what is taken off people, a lamp.", [
    P("TableMilitary_US_01.et", 0, 0, 0), P("CrateWooden_01_plain.et", 1.5, 0.1, 85), P("LanternMilitary_US_01.et", -0.4, 0.05, 0, 0.9), P("Suitcase_04_open.et", 0.3, 0, 10, 0.9),
    S("RZ_Work", 0, -0.9, "loiter", 0, 0)])
# One supply yard a site: its back (the container) to the building's wall, the work in front of it
piece("SupplyYard", "Common", "Work", "Inner", "The stores, off the trucks: a container along the wall, pallets set down in a row in front of it, cases stacked, the forklift parked at its end, the storeman's crate to count at.", [
    P("Container_openable.et", 1.29, -2.6, 90), P("Pallet_01_Pile.et", -2.4, 0, 2), P("Pallet_01_Pile.et", -0.8, 0.05, -3), P("Pallet_01_Pile.et", 0.8, 0, 4),
    P("EqBoxStack_destructible.et", 2.6, -0.3, 5), P("EqBoxStack_destructible.et", 2.7, 0.9, -4), P("Forklift_orange.et", 5.3, -1.6, 0),
    P("CrateWooden_01_plain.et", -4.4, 0.4, 80), P("BarrelMetal_01_grey.et", -4.7, -2.0), P("BarrelMetal_01_grey.et", -4.8, -1.2),
    S("RZ_Work", -4.4, 1.3, "loiter", -4.4, 0.4)])
piece("ChopShop", "RedHand", "Work", "Inner", "What the crew does with what it takes: a jeep off Bastion up on a jack, an engine crane over its nose, a welder, a toolbox, the scrap that came off it and its tyres in a pile.", [
    P("M151A2_wreck_static.et", 0, 0, 8), P("EngineCrane_01.et", 0.2, 2.9, 184), P("HydraulicCarJack_01.et", -1.4, 0.9, 30), P("GasWelder_01.et", 1.9, -1.3, 35),
    P("ToolBox_01_red.et", 1.7, 0.5, -15), P("VehicleGarbage_01_pile_small.et", -2.0, -1.7, 20), P("Tire_UAZ469_pile.et", 2.3, 1.9),
    S("RZ_Work", 1.3, 1.2, "loiter", 0, 1.0)])
piece("Scarecrow", "Haven", "None", "Inner", "A scarecrow over the beds: the birds get nothing the village can eat.", [P("Scarecrow_01.et", 0, 0, 0)])
piece("WaitingCots", "Halcyon", "None", "Approach", "Those waiting to be seen: two cots by triage, a drip stand between them, a screen.", [
    P("CotMilitary_US_01.et", 0, 0.7, 0), P("CotMilitary_US_01.et", 0, -0.7, 2), BESIDE("InfusionStand_01.et", "CotMilitary_US_01.et", 0, 0.7, 0, "right", along=-0.7, gap=0.3), P("MedicalScreen_01.et", -1.8, 0, 90)])
piece("LootContainer", "RedHand", "Sit", "Inner", "What the crew took, shut in a container along the wall: a chair in front of it for whoever has the key, a crate, bottles.", [
    P("Container_openable.et", 1.29, -1.4, 90), P("ChairOld_01.et", -1.0, 1.1, 170), P("CrateWooden_01_plain.et", 0.3, 1.0, 80), P("BeerBottle_01_group.et", 0.3, 1.0, 0, 0.83),
    S("RZ_Sit", -1.0, 1.1, "sit", -1.0, 6)])
piece("FuelPoint", "Common", "None", "Inner", "A fuel pump unit on the ground, drums and cans round it.", [
    P("US_fuelpump.et", 0, 0, 0), P("BarrelMetal_01_grey.et", 1.7, 0.2), P("BarrelMetal_01_grey.et", 1.8, -0.6), P("Jerrycan_01.et", -1.4, 0.3, 15), P("Jerrycan_01.et", -1.45, -0.1, -20)])
piece("RiotLine", "Common", "None", "Approach", "Riot fencing in a line: a lane to queue in.", [
    P("RiotFence.et", -2.01, 0, 0), P("RiotFence.et", -0.67, 0, 0), P("RiotFence.et", 0.67, 0, 0), P("RiotFence.et", 2.01, 0, 0)])

# ------------------------------------------------------------------- The layers (docs/SITES.md, "Building in layers")
# The wire: runs of each faction's barrier along the line, longest first (build.py reads each run's
# length from its footprint). Each run lies along its own x, its front (+Z) out of the site.
piece("Wire_Bastion_L", "Bastion", "None", "Approach", "Three coils of concertina on their stakes, end to end.", [
    P("BarbedTape_01_Long.et", -3.4, 0, 90), P("BarbedTape_01_Long.et", 0, 0, 90), P("BarbedTape_01_Long.et", 3.4, 0, 92)])
piece("Wire_Bastion_M", "Bastion", "None", "Approach", "Two coils of concertina.", [
    P("BarbedTape_01_Long.et", -1.7, 0, 90), P("BarbedTape_01_Long.et", 1.7, 0, 88)])
piece("Wire_Bastion_S", "Bastion", "None", "Approach", "A short coil.", [P("BarbedTape_01_Short.et", 0, 0, 90)])
piece("Wire_Bastion_XS", "Bastion", "None", "Approach", "A hedgehog of rusty girders in a gap.", [P("CzechHedgehog_01_rusty.et", 0, 0, 12)])
piece("Sandbags_Bastion_L", "Bastion", "None", "Approach", "A run of sandbags, three long sections chest high.", [
    P("Sandbag_01_long_high_burlap.et", -3.4, 0, 0), P("Sandbag_01_long_high_burlap.et", 0, 0, 2), P("Sandbag_01_long_high_burlap.et", 3.4, 0.05, -1)])
piece("Sandbags_Bastion_M", "Bastion", "None", "Approach", "One long section of sandbags, chest high.", [P("Sandbag_01_long_high_burlap.et", 0, 0, 0)])
piece("Scrap_RedHand_L", "RedHand", "None", "Approach", "Rusted tin sheets stood end to end, one leaning a little.", [
    P("TinWall_01_6m_A_rusty.et", -4.55, 0, 0), P("TinWall_01_3m_rusty.et", 1.5, 0.05, -4)])
piece("Scrap_RedHand_M", "RedHand", "None", "Approach", "A long sheet of rusted tin.", [P("TinWall_01_6m_A_rusty.et", -3.0, 0, 0)])
piece("Scrap_RedHand_S", "RedHand", "None", "Approach", "A road block of steel dragged across a gap.", [P("RoadBlock_01.et", 0, 0, 0)])
piece("Scrap_RedHand_XS", "RedHand", "None", "Approach", "A pile of tyres in a gap.", [P("TirePile_01_small.et", 0, 0, 0)])
piece("Scrap_RedHand_XXS", "RedHand", "None", "Approach", "A pallet stood on its edge in the last gap.", [P("Pallet_01.et", 0, 0, 90, 0.42, 0, 85)])
piece("Fence_Haven_L", "Haven", "None", "Approach", "A long run of plank fence, kept mended.", [P("WoodenFence_01_8m.et", -4.0, 0, 0)])
piece("Fence_Haven_M", "Haven", "None", "Approach", "A section of plank fence.", [P("WoodenFence_01_4m.et", -2.0, 0, 0)])
piece("Fence_Haven_S", "Haven", "None", "Approach", "An old fence panel on a post.", [P("WoodenFenceOld_01_4m.et", -2.0, 0, 0)])
piece("Fence_Haven_XS", "Haven", "None", "Approach", "Crates stacked across a gap, a pallet on top.", [
    P("CrateWooden_02_1x1x1.et", 0, 0, 4), P("Pallet_01.et", 0, 0, 2, 1.0)])
piece("Fence_Halcyon_L", "Halcyon", "None", "Approach", "Tall chain-link, two sections end to end.", [
    P("NetFence_05_tall_6m_V1.et", -4.7, 0, 0), P("NetFence_05_tall_3m.et", 1.6, 0, 0), P("NetFence_05_tall_pole.et", 4.75, 0, 0)])
piece("Fence_Halcyon_M", "Halcyon", "None", "Approach", "A tall chain-link section.", [
    P("NetFence_05_tall_6m_V1.et", -3.1, 0, 0), P("NetFence_05_tall_pole.et", 3.2, 0, 0)])
piece("Fence_Halcyon_S", "Halcyon", "None", "Approach", "A short chain-link section.", [
    P("NetFence_05_tall_3m.et", -1.55, 0, 0), P("NetFence_05_tall_pole.et", 1.6, 0, 0)])
piece("Fence_Halcyon_XS", "Halcyon", "None", "Approach", "A coil of wire in a gap.", [P("BarbedTape_01_Short.et", 0, 0, 90)])
piece("Fence_Halcyon_XXS", "Halcyon", "None", "Approach", "A no-entry sign on its pole in the last gap.", [P("SignIndustrial_USSR_02_warnings_entryforbidden_pole.et", 0, 0, 0)])
# The lab's own line: security mesh four metres high, a razor coil along its top
piece("SecFence_Halcyon_L", "Halcyon", "None", "Approach", "Security mesh four metres high with a razor coil along its top, two sections.", [
    P("SF_SecurityFence_6m.et", -6.0, 0, 0), P("SF_SecurityFence_6m.et", 0, 0, 0)])
piece("SecFence_Halcyon_M", "Halcyon", "None", "Approach", "A section of security mesh, a razor coil along its top.", [P("SF_SecurityFence_6m.et", -3.0, 0, 0)])
piece("SecFence_Halcyon_S", "Halcyon", "None", "Approach", "A short section of security mesh.", [P("SF_SecurityFence_3m.et", -1.5, 0, 0)])
# Road blocks (docs/SITES.md, "One way in"): what a stronghold closes every road with but its one gate. Razor wire, not
# concrete (the author, 2026-11-13).
piece("Block_Bastion_L", "Bastion", "None", "Approach", "The road closed with razor wire: two knife rests end to end across it, a coil of concertina behind them.", [
    P("BarbedTape_KnifeRest.et", -1.8, 0, 90), P("BarbedTape_KnifeRest.et", 1.8, 0.05, 88), P("BarbedTape_01_Long.et", 0, -1.3, 90)])
piece("Block_Bastion_M", "Bastion", "None", "Approach", "A knife rest of razor wire across a lane.", [P("BarbedTape_KnifeRest.et", 0, 0, 90)])
piece("Block_Halcyon_L", "Halcyon", "None", "Approach", "The road closed with the lab's own fence: a section of security mesh, a razor coil along its top.", [P("SF_SecurityFence_6m.et", -3.0, 0, 0)])
piece("Block_Halcyon_M", "Halcyon", "None", "Approach", "A short section of security mesh across a lane.", [P("SF_SecurityFence_3m.et", -1.5, 0, 0)])
piece("Block_RedHand_L", "RedHand", "None", "Approach", "A truck on its rims dragged across the road, tyres piled at its nose.", [
    P("Ural4320_wreck_static.et", -0.9, 0, 90), P("TirePile_01_small.et", 4.3, 0.2, 0)])
piece("Block_RedHand_M", "RedHand", "None", "Approach", "A van turned across the road.", [P("S1203_wreck_static.et", 0, 0, 90)])
piece("Block_RedHand_S", "RedHand", "None", "Approach", "A steel road block, a tyre against it.", [P("RoadBlock_01.et", 0, 0, 0), P("Tire_UAZ469.et", 0.3, 0.7, 0)])
piece("Block_Haven_L", "Haven", "None", "Approach", "A van on its rims across the road, crates stacked to close the rest.", [
    P("UAZ452_wreck_static.et", -1.2, 0, 90), P("CrateWooden_02_1x1x1.et", 1.9, 0, 5), P("CrateWooden_02_1x1x1.et", 3.0, 0.1, -4), P("CrateWooden_02_1x1x1.et", 2.4, 0.05, 8, 1.0)])
piece("Block_Haven_M", "Haven", "None", "Approach", "A fence panel across the lane, braced with crates.", [
    P("WoodenFence_01_4m.et", -2.0, 0, 0), P("CrateWooden_02_1x1x1.et", -0.8, -0.75, 6), P("CrateWooden_02_1x1x1.et", 0.6, -0.75, -5)])
piece("Block_Haven_S", "Haven", "None", "Approach", "Crates stacked in a gap.", [
    P("CrateWooden_02_1x1x1.et", -0.52, 0, 4), P("CrateWooden_02_1x1x1.et", 0.52, 0, -3), P("CrateWooden_02_1x1x1.et", 0, 0, 6, 1.0)])
# The gates: what stands each side of the opening on the road in
piece("Gate_Bastion", "Bastion", "None", "Approach", "The checkpoint: a stop sign on its stand, a knife rest pulled aside along the wire, a sandbag end.", [
    P("SignCheckpoint_01_stop.et", -2.2, 0.1, 0), P("BarbedTape_KnifeRest.et", 0.6, 0, 92), P("Sandbag_01_end_burlap.et", -3.1, -0.1, 90)])
piece("Gate_Haven", "Haven", "None", "Approach", "The gate swung open on its post: a fence panel turned back against a pole, a flowerpot.", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("WoodenFenceOld_01_4m.et", 0, -0.1, 75), P("FlowerpotFlowers_01_begonia_white.et", 0.6, 0.5)])
piece("Lookout_Haven", "Haven", "Watch", "Approach", "A chair on a pallet where someone sits watching, a lantern on a pole, a blanket for the cold.", [
    P("Pallet_01.et", 0, 0, 4), P("ChairOld_01.et", 0, 0, 180, 0.15), P("BlanketFolded_01_green.et", 0, 0, 180, 0.6), P("WoodenFenceOld_01_pole.et", 0.9, 0.4),
    P("LanternMilitary_US_01.et", 0.9, 0.4, 0, 1.6),
    S("RZ_Watch", 0, -0.1, "sit", 0, 10)])
piece("Floodlight", "Halcyon", "None", "Edge", "A floodlight mast on its trailer, lit, facing out over the fence.", [P("GeneratorFloodlight_US_01.et", 0, 0, 180)])
# Harden: boards over windows and blocked doors, against the wall, their front out; a window's at its own height
piece("BoardWindow", "Common", "None", "Approach", "A pallet nailed over a ground-floor window.", [P("Pallet_01.et", 0, 0, 0, -0.6, 90, 0)])
piece("BoardDoor", "Common", "None", "Approach", "A door shut with pallets stood against it, a third nailed across.", [
    P("Pallet_01.et", -0.42, 0.05, 0, 0, 90, 0), P("Pallet_01.et", 0.42, 0.05, 0, 0, 90, 0), P("Pallet_01.et", 0, 0.12, 4, 0.9, 90, 0)])
piece("BoardDoor_Bastion", "Bastion", "None", "Approach", "A side door sandbagged shut, waist high.", [
    P("Sandbag_01_single_burlap.et", -0.3, 0.2), P("Sandbag_01_single_burlap.et", 0.3, 0.2), P("Sandbag_01_single_burlap.et", -0.3, 0.2, 0, 0.17),
    P("Sandbag_01_single_burlap.et", 0.3, 0.2, 0, 0.17), P("Sandbag_01_single_burlap.et", 0, 0.2, 0, 0.34)])
piece("BoardDoor_RedHand", "RedHand", "None", "Approach", "The shutter down: a rusty tin sheet across the door, a tyre against it.", [
    P("TinWall_01_3m_rusty.et", -1.5, 0.05, 0), P("Tire_UAZ469.et", 0.4, 0.5, 0, 0, 90)])
# Work and life: where the valuables are, once they're indoors
piece("StoresBoard", "Common", "None", "Centre", "The stores board by the door: a chalkboard with the counts, a crate for the clipboard.", [
    P("ChalkBoard_01.et", 0, 0, 0), P("Notice_A4_Vertical_V1.et", 0.2, -0.04, 180, 1.3), P("CrateWooden_01_plain.et", 1.0, -0.3, 20)])

# ------------------------------------------------------------------- Counters
# The quartermaster's counter: one in each kit, placed where the place's quartermaster stands (At
# COUNTER), not by the plan. He stands behind it, 0.95 m back from its middle (-Z), facing its front
# (+Z): the customers' side. Kept small and clear behind, so it can stand indoors too.
piece("Counter_Bastion", "Bastion", "None", "Centre", "The stores' window: a field table for a counter, the sign-out sheets and a lantern on it, two crates stacked at its end.", [
    P("TableMilitary_US_01.et", 0, 0), P("FileOffice_01_Stack.et", 0.45, 0.05, 6, 0.78), P("LanternMilitary_US_01.et", -0.7, 0.05, 0, 0.78),
    P("CrateWooden_02_1x1x1.et", 1.65, -0.1, 4), P("CrateWooden_01_plain.et", 1.65, -0.1, -6, 1.0)])
piece("Counter_RedHand", "RedHand", "None", "Centre", "The fence's table: a kitchen table between him and you, a lamp and bottles on it, a crate of what he's bought at his elbow.", [
    P("TableOld_01.et", 0, 0), P("LampKerosene_01_green.et", -0.4, 0.1, 0, 0.78), P("BeerBottle_01_group.et", 0.35, 0.05, 0, 0.78),
    P("CrateWooden_01_plain.et", 1.15, -0.2, 84)])
piece("Counter_Haven", "Haven", "None", "Centre", "The stores table: a white table with the book of counts and a lamp, a sack of seed beside it.", [
    P("TableOld_01_white.et", 0, 0), P("Notebook_01_Stack.et", 0.3, 0.05, 10, 0.78), P("LampKerosene_01_green.et", -0.4, 0.1, 0, 0.78),
    P("SackUniversal_01_Pile.et", 1.35, -0.1, 12)])
piece("Counter_Halcyon", "Halcyon", "None", "Centre", "The issue table: a field table with the ledger and a sample drum, a crate of cases at its end.", [
    P("TableMilitary_US_01.et", 0, 0), P("Notebook_01_Stack.et", -0.45, 0.05, 8, 0.78), P("ToolsSurgical_01_autoclavedrum.et", 0.4, 0, 0, 0.78),
    P("CrateWooden_02_1x1x1.et", 1.65, -0.1, 3)])

# Each piece's rules (docs/SITES.md, "The rules"): (tier, zone, rules). Tier: 1 security, 2 what the
# site is for, 3 life, 4 character, 5 clutter. Zone: yard (inside the wire), wire, gate, kill, court,
# stores, work, kitchen, sleep, sick, dump, dead. Rules, ";" between them: near/away <piece|A|B|use:X|
# water|gate|wire|door|road> <m>, door <front|side|back|any> <m>, wall <m>, window <m>, corner <m>, kind
# <garage|farm|church|well...> <m>, over <piece>, covers <gate|road|threat|out>, behind <threat>, faces
# <front|road|threat|out|in|gate|door|wall|piece>, guarded, high, bank, soil, jitter, noline, flat <deg>.
# The plan's spot is a preference: the solver lays the plan where the ground allows and bends it where
# it doesn't. A kit can give a piece another tier (KIT_TIER) and send its valuables indoors (DOCTRINE).
# ------------------------------------------------------------------- From the world-building kit (tools/camp_kit/world_kit.txt)
# Every size below is measured (footprints.json); each prop is placed by its faces.
_BANK = _BOX["DirtCover_01_short.et"]
piece("DugIn_Bastion", "Bastion", "None", "Approach", "An earth bank thrown up in front of a firing position after a bad night, the spent cases and ration wrappers still lying behind it.", [
    P("DirtCover_01_short.et", 0, 0, 0),
    P("GarbageAmmunition_US_01.et", -0.6, round(_BANK["z0"] - 0.6, 2), 15), P("GarbageFoodMilitaryUS_01.et", 0.9, round(_BANK["z0"] - 0.9, 2), 40)])
_STOVE = _BOX["StoveIndustrial_01.et"]
_STOVE_X = round(-(_STOVE["x0"] + _STOVE["x1"]) / 2, 3)
_STOVE_Z = round(-(_STOVE["z0"] + _STOVE["z1"]) / 2, 3)
piece("CanteenStove", "Common", "Cook", "Inner", "The canteen's gas stove dragged out to the kitchen door, the big pot on it, its gas bottle against its side.", [
    P("StoveIndustrial_01.et", _STOVE_X, _STOVE_Z, 0), P("PotIndustrialSet_01_Big.et", 0.2, 0, 0, 1.0),
    P("Propane.et", round(_STOVE["w"] / 2 + 0.2, 3), 0, 0),
    S("RZ_Cook", 0, round(_STOVE["d"] / 2 + 0.6, 2), "loiter", 0, 0)])
_BENCH = BESIDE("SawTable.et", "Workbench_02.et", 0, 0, 0, "right", gap=0.6)
piece("Workshop", "Common", "Work", "Inner", "A workbench carried out of a garage with a vice on it and a toolbox, the table saw a step off its end, the compressor at the other end: where what can't be replaced is mended.", [
    P("Workbench_02.et", 0, 0, 0), P("Workbench_Vice_01.et", -0.8, 0.25, 0, 1.0), P("ToolBox_01_red.et", 0.5, 0.3, 10, 1.0), _BENCH,
    BESIDE("AirCompressor_01.et", "Workbench_02.et", 0, 0, 0, "left", gap=0.3),
    S("RZ_Work", 0, round(_BOX["Workbench_02.et"]["z1"] + 0.6, 2), "loiter", 0, 0)])
_JUNK = _BOX["Junk_01_pile_medium.et"]
piece("ScrapHeap", "RedHand", "None", "Edge", "What was no use, thrown on a heap at the back: junk, the scrap off the cars, a pile of tyres beside it.", [
    P("Junk_01_pile_medium.et", 0, 0, 0), BESIDE("VehicleGarbage_01_pile_medium.et", "Junk_01_pile_medium.et", 0, 0, 0, "right", gap=0.0),
    BESIDE("TirePile_01_medium.et", "Junk_01_pile_medium.et", 0, 0, 0, "back", gap=0.4)])
piece("DrumStore", "RedHand", "None", "Edge", "The toll, paid in drums: closed fuel drums stood together against the wall, dented ones beside them.", [
    P("BarrelFuel_01_closed_old.et", 0, 0, 0), BESIDE("Barrel_01.et", "BarrelFuel_01_closed_old.et", 0, 0, 0, "right"),
    BESIDE("Barrel_02.et", "BarrelFuel_01_closed_old.et", 0, 0, 0, "left"), BESIDE("BarrelFuel_01_closed_old.et", "BarrelFuel_01_closed_old.et", 0, 0, 0, "back")])
_HAY = BESIDE("WheelBarrow_01.et", "HayBale_01_square_big_bottom.et", 0, 0, 0, "right", gap=0.4)
piece("HayStore", "Haven", "Work", "Edge", "Hay for the animals the farms still keep: a big bale on the grass, the barrow it came on beside it.", [
    P("HayBale_01_square_big_bottom.et", 0, 0, 0), _HAY])
piece("WoodBox", "Haven", "Work", "Edge", "Firewood split and stacked in a pallet box, the sawmill's hand cart beside it.", [
    P("PileOfWood_01.et", 0, 0, 0), BESIDE("CartSawmill_01_transport.et", "PileOfWood_01.et", 0, 0, 0, "right", gap=0.3)])
piece("Tent_Blue", "Haven", "Sleep", "Inner", "A small blue tent someone brought from home, a blanket airing on the grass at its door.", [
    P("TentSmall_02_blue.et", 0, 0, 0), P("BlanketBeach_01_green.et", 0, round(_BOX["TentSmall_02_blue.et"]["z1"] + 0.3 - _BOX["BlanketBeach_01_green.et"]["z0"], 2), 6)])
_COVER = _BOX["CanvasCover_Medium_US.et"]
_SURG = _BOX["TableSurgical_01.et"]
_BED = _BOX["BedMedical_01.et"]
_SURG_X = round(-0.4 - _SURG["x1"], 3)
_BED_X = round(0.4 - _BED["x0"], 3)
piece("FieldSurgery", "Halcyon", "Work", "Inner", "Surgery under canvas: what can't be made clean is treated outside the clean room. The surgical table under the lamp, a bed beside it, the steriliser and the drum on their stands, the litter of it on the ground.", [
    P("CanvasCover_Medium_US.et", 0, 0, 0), P("TableSurgical_01.et", _SURG_X, 0, 0), P("BedMedical_01.et", _BED_X, 0, 0),
    BESIDE("LampSurgical_01_on.et", "TableSurgical_01.et", _SURG_X, 0, 0, "left", gap=0.3),
    BESIDE("TableMedical_01_small.et", "TableSurgical_01.et", _SURG_X, 0, 0, "front", gap=0.3),
    BESIDE("ToolsSurgical_01_autoclavedrum_stand_V1.et", "BedMedical_01.et", _BED_X, 0, 0, "front", gap=0.3),
    P("GarbageMedicalUS_01.et", 0, -1.4, 30),
    S("RZ_Work", round(_SURG_X, 2), round(_SURG["z0"] - 0.6, 2), "loiter", round(_SURG_X, 2), 0)])
_TOOTH = _BOX["Dragontooth_01_V3.et"]
piece("Block_Bastion_Teeth", "Bastion", "None", "Approach", "Concrete dragon's teeth across a road, a stride apart: a body walks between them, nothing on wheels does.", [
    *ROW("Dragontooth_01_V3.et", round(-(_TOOTH["w"] + 0.6), 3), 0, 0, 3, gap=0.6)])

FIRES = "FirePit|FireBarrel|CookPot"
SEATS = "Table|CrateChairs|CardTable|CrateSeats|Sofa|FieldTable"
RULES = {
    "DugIn_Bastion": (4, "yard", "by SentryNest_Bastion|Bunker_Bastion|Watchtower_Bastion 10; faces threat; flat 6"),
    "CanteenStove": (3, "kitchen", "by CookPot 6; wall 3; flat 6"),
    "Workshop": (4, "work", "wall 4; away use:Sleep 8; away gate 6; flat 6"),
    "ScrapHeap": (5, "yard", "by ChopShop|FuelDump|TyreStack|JeepWreck 10; away Sofa|CardTable|FireBarrel 10; away gate 8"),
    "DrumStore": (4, "yard", "by FuelDump|FuelPoint 6; wall 8; away " + "FirePit|FireBarrel|CookPot" + " 8; flat 6"),
    "HayStore": (4, "yard", "by Garden 10; away CookPot|FirePit 10; soil"),
    "WoodBox": (4, "kitchen", "by CookPot|FirePit 8; flat 6"),
    "Tent_Blue": (3, "sleep", "away FirePit|FireBarrel|CookPot 6; flat 5; soil; noline"),
    "FieldSurgery": (3, "work", "wall 4; away gate 10; away CookPot 8; flat 6"),
    "Block_Bastion_Teeth": (1, "wire", ""),
    # Common
    "FirePit": (3, "yard", "near " + SEATS + " 8; away use:Sleep 6; away CookPot 4; flat 6; soil"),
    "Table": (3, "yard", "near CookPot|FirePit 8; flat 5; noline"),
    "Tarp": (3, "work", "wall 3; faces wall"),
    "FireBarrel": (3, "court", "door front 10; flat 8"),
    "Tent_Small_Civilian": (3, "sleep", "away " + FIRES + " 6; flat 5; soil; noline"),
    "Tent_Large_Civilian": (3, "sleep", "away " + FIRES + " 6; flat 5; soil"),
    "Firewood": (3, "kitchen", "by CookPot|FirePit 8; flat 6"),
    "CookPot": (3, "kitchen", "door side 4; away Latrine 15; away use:Sleep 6; flat 6"),
    "WashStand": (3, "kitchen", "near WaterBarrel 3; flat 6"),
    "Latrine": (3, "yard", "away CookPot|WaterBarrel 15; away Table|FirePit|CardTable|Sofa 10; away gate 10; away door 8; behind threat"),
    "DryingLine": (4, "sleep", "flat 5; soil"),
    "Duckboards": (4, "yard", "by use:Sleep 5; soil"),
    "CrateChairs": (3, "yard", "by FirePit|CookPot|Table 7; flat 6"),
    "Clutter": (5, "yard", "by use:Sit 4; jitter"),
    "RadioTable": (3, "work", "wall 3; faces in"),
    "ScrapShelter": (3, "sleep", "away " + FIRES + " 6; flat 6"),
    "Barricade_Fence": (1, "wire", "faces out"),
    "Kennel": (4, "gate", "faces road; flat 8"),
    # Bastion
    "Tent_Small_Bastion": (3, "sleep", "away FirePit|CookPot 6; flat 5; soil; row"),
    "Tent_Large_Bastion": (3, "sleep", "away FirePit|CookPot 6; flat 5; soil"),
    "SentryNest_Bastion": (1, "yard", "covers gate; covers road; faces road; away gate 3; bank"),
    "RosterBoard": (2, "court", "wall 2; door front 6; faces wall"),
    "Barricade_Sandbag": (1, "wire", "faces out"),
    "FlagPole": (4, "court", "near RosterBoard 5"),
    "ShavingStand": (3, "kitchen", "near WaterBarrel|CookPot 5; flat 6"),
    "CrateSeats": (3, "yard", "near FirePit 6; flat 6"),
    "DutyBoard": (2, "court", "wall 2; near RosterBoard 5; faces wall"),
    "Gate_Bastion": (1, "gate", "faces road; flat 8"),
    # The Red Hand
    "SentryNest_RedHand": (1, "yard", "covers gate; covers road; faces road; away gate 3; bank"),
    "CardTable": (3, "yard", "by FireBarrel|Sofa 7; flat 6"),
    "TollBarrier": (2, "gate", "faces road; flat 8"),
    "Barricade_Scrap": (1, "wire", "faces out"),
    "RedHandSheet": (4, "gate", "faces road; flat 8"),
    "TrophyWall": (4, "gate", "by TollBarrier|RedHandSheet|WreckGate 12; faces road; flat 8"),
    "Sofa": (3, "yard", "by FireBarrel 4; faces FireBarrel; flat 6"),
    # The Haven
    "WashingLine": (4, "sleep", "flat 5; soil"),
    "Garden": (4, "yard", "soil; flat 4; away gate 10; row"),
    "WaterBarrel": (3, "kitchen", "near CookPot 5; kind well 25; flat 6"),
    "BearDoor": (4, "court", "door front 3; faces out"),
    "Memorial": (2, "court", "wall 3; near Table 8; faces in"),
    "ChildrensCorner": (4, "sleep", "away gate 15; away Latrine 10; flat 5"),
    "VegStall": (4, "court", "faces front; flat 5"),
    "Gate_Haven": (1, "gate", "faces road; flat 8"),
    "Lookout_Haven": (1, "yard", "covers gate; covers road; faces road; away gate 3"),
    # Halcyon
    "Tent_Halcyon": (3, "sick", "away CookPot 10; flat 5; soil; row"),
    "FieldTable": (3, "work", "wall 4; faces in; flat 5"),
    "ColdBox": (2, "stores", "wall 2; guarded; faces wall"),
    "Antenna": (4, "yard", "high"),
    "Generator": (3, "yard", "wall 3; behind threat; near ColdBox 6; away use:Sleep 10; away door 6"),
    "SampleTable": (2, "work", "guarded; flat 5"),
    "DeconShower": (2, "court", "door front 3; faces out"),
    "PlagueBoard": (4, "gate", "faces road; flat 8"),
    "NotebookBench": (3, "work", "wall 3"),
    "GuardPost_Halcyon": (1, "yard", "covers gate; covers road; faces road; away gate 3"),
    "SmokingBench": (5, "yard", "door back 4; faces wall"),
    # The layers' own pieces
    "Wire_Bastion_L": (1, "wire", ""), "Wire_Bastion_M": (1, "wire", ""), "Wire_Bastion_S": (1, "wire", ""), "Wire_Bastion_XS": (1, "wire", "level"),
    "Sandbags_Bastion_L": (1, "wire", ""), "Sandbags_Bastion_M": (1, "wire", ""),
    "Scrap_RedHand_L": (1, "wire", ""), "Scrap_RedHand_M": (1, "wire", ""), "Scrap_RedHand_S": (1, "wire", "level"), "Scrap_RedHand_XS": (1, "wire", "level"),
    "Fence_Haven_L": (1, "wire", ""), "Fence_Haven_M": (1, "wire", ""), "Fence_Haven_S": (1, "wire", ""), "Fence_Haven_XS": (1, "wire", "level"),
    "Fence_Halcyon_L": (1, "wire", ""), "Fence_Halcyon_M": (1, "wire", ""), "Fence_Halcyon_S": (1, "wire", ""), "Fence_Halcyon_XS": (1, "wire", ""),
    "Floodlight": (3, "yard", "faces out"),
    "BoardWindow": (1, "yard", ""), "BoardDoor": (1, "yard", ""), "BoardDoor_Bastion": (1, "yard", ""), "BoardDoor_RedHand": (1, "yard", ""),
    "StoresBoard": (2, "court", "faces out"),
    # From the reference board
    "Bunker_Bastion": (1, "yard", "covers gate; covers road; faces road; away gate 3; bank"),
    "MotorPool": (3, "gate", "faces road; away gate 4; flat 6"),
    "FieldKitchen": (3, "kitchen", "door side 4; away Latrine 15; away use:Sleep 6; flat 6"),
    "AidPost": (3, "sick", "away CookPot 8; flat 5; soil"),
    "Floodlight_Bastion": (3, "yard", "faces out"),
    "Scrap_RedHand_Wreck": (1, "wire", "level"), "Scrap_RedHand_XXS": (1, "wire", "level"), "Fence_Halcyon_XXS": (1, "wire", "level"),
    "WreckGate": (2, "gate", "faces road; flat 8"),
    "FuelDump": (4, "yard", "wall 8; away " + FIRES + " 8; flat 6"),
    "Watchtower_Haven": (1, "yard", "covers gate; covers road; faces road; away gate 3"),
    "WorkBoard": (4, "gate", "faces road; flat 8"),
    "LanternPost": (4, "court", "door front 8; away LanternPost 7"),
    "Gate_Halcyon": (1, "gate", "faces road; flat 8"),
    "WarningSigns": (4, "gate", "faces road; flat 8"),
    "Loudspeaker_Halcyon": (4, "yard", "flat 8"),
    "Triage": (3, "gate", "faces road; flat 6"),
    # Dense sites' set dressing: every one has a rule that says where to look, so it is never left to its plan spot alone
    "CrateStack": (5, "work", "wall 3; faces wall"),
    "PalletStack": (5, "yard", "by SupplyYard|LootContainer|Tarp 9; flat 8"),
    "BarrelGroup": (5, "yard", "by MotorPool|FuelPoint|FuelDump|Generator|GeneratorPost|CookPot 8; flat 8"),
    "TyreStack": (5, "yard", "by MotorPool|JeepWreck|ChopShop|WreckGate|FuelDump|Barricade_Scrap 8"),
    "Bedrolls": (5, "sleep", "by use:Sleep 6; away " + FIRES + " 4; flat 5; soil"),
    "SandbagPile": (5, "yard", "by Bunker_Bastion|SentryNest_Bastion|SandbagWall|Barricade_Sandbag 7"),
    "SandbagWall": (4, "court", "door front 14; faces front; flat 8"),
    "MessTable": (4, "kitchen", "by CookPot 10; row; flat 5"),
    "CheckpointSign": (5, "gate", "by Gate_Bastion 10; faces road; flat 8"),
    "JeepWreck": (4, "gate", "by MotorPool 10; faces road; away gate 4; flat 6"),
    "GeneratorPost": (4, "yard", "wall 3; away use:Sleep 8"),
    "RadioMast": (4, "work", "wall 4; faces in; flat 5"),
    # From the mods the project loads already
    "Watchtower_Bastion": (2, "yard", "covers gate; faces road; away gate 4; flat 6"),
    "SupplyYard": (3, "work", "wall 12; faces wall; away gate 10; away door 4; flat 5"),
    "LootContainer": (3, "yard", "wall 10; faces wall; away gate 8; away door 4; flat 5"),
    "ChopShop": (3, "yard", "wall 10; away gate 8; away door 4; flat 6"),
    "Scarecrow": (4, "yard", "by Garden 3; soil"),
    "WaitingCots": (4, "gate", "by Triage 8; flat 6"),
    "SearchTable": (2, "gate", "by Gate_Bastion 12; faces road; flat 6"),
    "FuelPoint": (4, "yard", "by MotorPool|FuelDump 10; away " + FIRES + " 8; flat 6"),
    "RiotLine": (4, "gate", "by Triage 9; faces road; flat 8"),
    "SecFence_Halcyon_L": (1, "wire", ""), "SecFence_Halcyon_M": (1, "wire", ""), "SecFence_Halcyon_S": (1, "wire", ""),
    "Block_Bastion_L": (1, "wire", ""), "Block_Bastion_M": (1, "wire", ""), "Block_Halcyon_L": (1, "wire", ""), "Block_Halcyon_M": (1, "wire", ""),
    "Block_RedHand_L": (1, "wire", "level"), "Block_RedHand_M": (1, "wire", "level"), "Block_RedHand_S": (1, "wire", "level"),
    "Block_Haven_L": (1, "wire", "level"), "Block_Haven_M": (1, "wire", "level"), "Block_Haven_S": (1, "wire", "level"),
    # Counters (laid by the quartermaster, not by the solver)
    "Counter_Bastion": (1, "court", ""), "Counter_RedHand": (1, "court", ""), "Counter_Haven": (1, "court", ""), "Counter_Halcyon": (1, "court", ""),
}
# Bastion's field kitchen is its cook pot: every rule that names the pot means either
RULES = {name: (tier, zone, rules.replace("CookPot", "CookPot|FieldKitchen")) for name, (tier, zone, rules) in RULES.items()}
# A prop's footprint on the ground where its measured box says more than what stands on it (footprints.py): the guard
# tower's box takes in its platform and stair, 7.5 x 8.7 m; its legs stand on 4.6 m square.
FOOT_FORCED = {"GuardTower_destructible.et": (4.6, 4.6)}
# A kit's own tier for a piece (the Haven's long table is what the Haven is for)
KIT_TIER = {("SURVIVORS", "Table"): 2}

# Each kind of site's doctrine (docs/SITES.md, "Building in layers"): (faction, place) -> close (none,
# road, two, all), harden (none, doors, threat, all), metres to the wire round the building and at
# least on the yard's side, and its layers' pieces: the barrier's runs (longest first), the window
# and door boards, the lookout, the light, the stores board, and the plan's pieces that go indoors
# at a building (valuable or delicate).
WIRE_BASTION = ["Wire_Bastion_L", "Wire_Bastion_M", "Wire_Bastion_S", "Wire_Bastion_XS"]
WIRE_REDHAND = ["Scrap_RedHand_L", "Scrap_RedHand_M", "Scrap_RedHand_Wreck", "Scrap_RedHand_S", "Scrap_RedHand_XS", "Scrap_RedHand_XXS"]
WIRE_HAVEN = ["Fence_Haven_L", "Fence_Haven_M", "Fence_Haven_S", "Fence_Haven_XS"]
WIRE_HALCYON = ["Fence_Halcyon_L", "Fence_Halcyon_M", "Fence_Halcyon_S", "Fence_Halcyon_XS", "Fence_Halcyon_XXS"]
# The lab's own line, and each stronghold's road blocks (docs/SITES.md, "One way in")
WALL_HALCYON = ["SecFence_Halcyon_L", "SecFence_Halcyon_M", "SecFence_Halcyon_S", "Fence_Halcyon_XS", "Fence_Halcyon_XXS"]
BLOCK_BASTION = ["Block_Bastion_Teeth", "Block_Bastion_L", "Block_Bastion_M", "Wire_Bastion_S", "Wire_Bastion_XS"]
BLOCK_HALCYON = ["Block_Halcyon_L", "Block_Halcyon_M", "Fence_Halcyon_XS", "Fence_Halcyon_XXS"]
BLOCK_REDHAND = ["Block_RedHand_L", "Block_RedHand_M", "Block_RedHand_S", "Scrap_RedHand_XS"]
BLOCK_HAVEN = ["Block_Haven_L", "Block_Haven_M", "Block_Haven_S"]
DOCTRINE = {
    ("MILITARY", "STRONGHOLD"): dict(close="all", harden="all", ring=8, yard=14, wire=WIRE_BASTION, block=BLOCK_BASTION, window="BoardWindow", door="BoardDoor_Bastion",
                                     lookout=["SentryNest_Bastion"], light="Floodlight_Bastion", stores="StoresBoard", indoors=["Tarp"]),
    ("MILITARY", "OUTPOST"): dict(close="two", harden="all", ring=7, yard=12, wire=["Sandbags_Bastion_L", "Sandbags_Bastion_M", "Wire_Bastion_S", "Wire_Bastion_XS"],
                                  window="BoardWindow", door="BoardDoor_Bastion", lookout=["SentryNest_Bastion"]),
    ("MILITARY", "ROAD_POST"): dict(close="none", harden="none"),
    ("BANDITS", "STRONGHOLD"): dict(close="all", harden="doors", ring=8, yard=14, wire=WIRE_REDHAND, block=BLOCK_REDHAND, door="BoardDoor_RedHand", lookout=["SentryNest_RedHand"]),
    ("BANDITS", "OUTPOST"): dict(close="road", harden="none", ring=7, yard=12, wire=WIRE_REDHAND, lookout=["SentryNest_RedHand"]),
    ("BANDITS", "ROAD_POST"): dict(close="none", harden="none"),
    ("SURVIVORS", "STRONGHOLD"): dict(close="all", harden="threat", ring=8, yard=14, wire=WIRE_HAVEN, block=BLOCK_HAVEN, window="BoardWindow", door="BoardDoor", lookout=["Lookout_Haven"]),
    ("SURVIVORS", "OUTPOST"): dict(close="road", harden="doors", ring=7, yard=12, wire=WIRE_HAVEN, door="BoardDoor", lookout=["Lookout_Haven"]),
    ("SURVIVORS", "SAFEHOUSE"): dict(close="road", harden="all", ring=6, yard=10, wire=WIRE_HAVEN, window="BoardWindow", door="BoardDoor", lookout=["Lookout_Haven"]),
    ("INSTITUTE", "STRONGHOLD"): dict(close="all", harden="all", ring=9, yard=12, wire=WALL_HALCYON, block=BLOCK_HALCYON, window="BoardWindow", door="BoardDoor",
                                      lookout=["GuardPost_Halcyon"], light="Floodlight", stores="StoresBoard",
                                      indoors=["FieldTable", "Tent_Halcyon", "ColdBox", "SampleTable", "NotebookBench", "RadioTable", "Tarp", "CrateChairs"]),
    ("INSTITUTE", "OUTPOST"): dict(close="two", harden="doors", ring=7, yard=12, wire=WIRE_HALCYON, door="BoardDoor", lookout=["GuardPost_Halcyon"]),
}

# Where a piece stands when its place has moved into a building (docs/DESIGN.md, "Camp life":
# RZ_EDressAt, written as each kit piece's m_eAt). A piece not listed is Auto: the approach pieces go
# to the road entries (a watch or lean piece to the gate, the rest to the closed ones), everything
# else stays in the yard as the plan draws it. Spots: yard, front, door, gate, closed, counter.
AT = {
    "RedHandSheet": "gate", "PlagueBoard": "gate",
    "BearDoor": "door", "Memorial": "door", "DeconShower": "door",
    "TrophyWall": "gate", "VegStall": "front",
    "Gate_Bastion": "gate", "Gate_Haven": "gate", "Kennel": "gate",
    "WreckGate": "gate", "WorkBoard": "gate", "Gate_Halcyon": "gate", "Triage": "gate", "WarningSigns": "closed", "LanternPost": "door",
}
AT.update({name: "counter" for name in C if name.startswith("Counter_")})

# The kits on RZ_GameMode.et (Territory > Camp life > Kits): (faction, place, what it is like, pieces).
# A kit says only what the faction brings to that kind of place, most important first (a place takes
# them in this order until its piece count). Nothing is drawn: the solver finds each piece its spot by
# its zone and rules (RULES), in tier order, on the ground where the site is (docs/SITES.md, "The
# solver"). DENSE adds the extras with their reasons and the state they follow.
KITS = [
    ("MILITARY", "STRONGHOLD", "Military order gone a little ragged: tents in a loose row behind a sandbag line with a gap for the road, the board and the flag at the heart, the stores' window to one side.", [
        "Counter_Bastion", "Bunker_Bastion", "Gate_Bastion", "RosterBoard", "FlagPole", "FirePit",
        "Barricade_Sandbag", "Barricade_Sandbag", "Table", "MotorPool", "AidPost",
        "Tent_Small_Bastion", "Tent_Small_Bastion",
        "Tent_Large_Bastion", "FieldKitchen", "Firewood", "ShavingStand", "CrateSeats",
        "DutyBoard", "Tarp", "Tent_Small_Bastion", "DryingLine", "Latrine", "Duckboards"]),
    ("MILITARY", "OUTPOST", "A section at a farm on the junction: a nest and a sandbag run on the road, two tents squared to it, the fire, the table and the roll board in the yard.", [
        "Counter_Bastion", "SentryNest_Bastion", "FirePit", "Barricade_Sandbag", "Tent_Small_Bastion",
        "Tent_Small_Bastion", "RosterBoard", "Table", "CookPot", "Firewood",
        "ShavingStand", "FlagPole", "Barricade_Sandbag", "Duckboards"]),
    ("MILITARY", "ROAD_POST", "The checkpoint: a nest beside the barrier facing down the road, a sandbag run on the other verge, a fire barrel and a net to wait under.", [
        "SentryNest_Bastion", "Barricade_Sandbag", "FireBarrel", "Tarp"]),
    ("BANDITS", "STRONGHOLD", "A squat, not a camp: things where they were dropped, but round the fire. The toll on the road, the sofa round the barrel, tents tucked behind scrap, the fence's table by the way in.", [
        "Counter_RedHand", "TollBarrier", "FireBarrel", "Sofa", "SentryNest_RedHand",
        "CardTable", "RedHandSheet", "WreckGate", "FuelDump", "Barricade_Scrap", "Tent_Small_Civilian", "Tent_Small_Civilian",
        "ScrapShelter", "CookPot", "Firewood", "TrophyWall", "Clutter",
        "Kennel", "FirePit", "Tent_Large_Civilian", "Latrine", "Barricade_Scrap"]),
    ("BANDITS", "OUTPOST", "A crew camp on the road: the toll barrier and a lookout behind tyres, the fire barrel and the sofa, the card table, two tents at the back.", [
        "Counter_RedHand", "TollBarrier", "FireBarrel", "Sofa", "SentryNest_RedHand",
        "CardTable", "WreckGate", "Tent_Small_Civilian", "Tent_Small_Civilian", "CookPot", "Firewood",
        "RedHandSheet", "Clutter", "ScrapShelter"]),
    ("BANDITS", "ROAD_POST", "A roadblock for the toll: the fire barrel and the card table on one verge, scrap and a net on the other.", [
        "FireBarrel", "CardTable", "Barricade_Scrap", "Tarp"]),
    ("SURVIVORS", "STRONGHOLD", "A village kept going: the long table and the memorial in the yard, the cook pot and the water by the kitchen side, the washing and the garden behind, the bear at the gate.", [
        "Counter_Haven", "Watchtower_Haven", "Gate_Haven", "BearDoor", "Table", "CookPot", "Memorial",
        "WorkBoard", "LanternPost", "LanternPost",
        "WaterBarrel", "WashStand", "WashingLine", "Garden", "Barricade_Fence",
        "VegStall", "ChildrensCorner", "Firewood", "FirePit", "Tent_Small_Civilian",
        "CrateChairs", "Barricade_Fence", "Tent_Large_Civilian", "Kennel", "Latrine"]),
    ("SURVIVORS", "OUTPOST", "A farmyard: the table and the memorial, the cook pot and the water, the washing and the garden at the back, a tent for whoever came last.", [
        "Counter_Haven", "Lookout_Haven", "BearDoor", "Table", "CookPot", "WaterBarrel",
        "LanternPost", "WorkBoard",
        "WashStand", "WashingLine", "Garden", "Memorial", "Firewood",
        "Barricade_Fence", "Tent_Small_Civilian", "ChildrensCorner"]),
    ("SURVIVORS", "SAFEHOUSE", "A house that kept its door open: the bear at the gate, the table outside, the pot and the barrel by the kitchen, the line and the bed of vegetables in the garden.", [
        "Counter_Haven", "BearDoor", "Table", "CookPot", "WaterBarrel", "LanternPost",
        "WashingLine", "Garden", "Firewood"]),
    ("INSTITUTE", "STRONGHOLD", "The lab grounds: a clean room in a dirty world. The work goes on indoors; outside only the decon shower at the one door, the generators behind sandbags, the antenna, the guards at the gate, a bench for the night shift (in a building, the tables, the tents and the cold store are indoors, as data).", [
        "Counter_Halcyon", "GuardPost_Halcyon", "Gate_Halcyon", "PlagueBoard", "Triage", "WarningSigns",
        "FieldTable", "Tent_Halcyon", "FieldTable", "Loudspeaker_Halcyon",
        "DeconShower", "Generator", "ColdBox", "SampleTable", "Antenna",
        "Barricade_Fence", "Barricade_Fence", "NotebookBench", "RadioTable", "Tarp",
        "Latrine", "CrateChairs", "SmokingBench", "Generator"]),
    ("INSTITUTE", "OUTPOST", "A field station at a house: a table for the notes, the ward tent and the decon shower in the yard, the cold store on the generator, the antenna, the plague board at the gate.", [
        "Counter_Halcyon", "GuardPost_Halcyon", "PlagueBoard", "Gate_Halcyon", "WarningSigns",
        "FieldTable", "Tent_Halcyon", "Generator",
        "Antenna", "DeconShower", "ColdBox", "NotebookBench"]),
]

# Dense sites (docs/SITES.md, "Dense sites"): (faction, place) -> the kit's extras after its plan's own pieces: (piece, how
# many, why it's there, and when: left out = always; "supplied" while the faction is thriving or holding on, "thriving",
# "hit" after an attack, a fire or a death, or under pressure: docs/SITES.md, "What the state changes in the yard"). An
# extra has no spot in the plan (m_bExtra): the solver puts it where its zone and rules say (what it stands by, its
# ground, its row), or leaves it out. The plans page (render.py) draws the plan without them.
DENSE = {
    ("MILITARY", "STRONGHOLD"): [
        ("Watchtower_Bastion", 1, "the gate and the road in are watched from above"),
        ("SearchTable", 1, "nobody passes the checkpoint with a pack unopened"),
        ("CheckpointSign", 1, "the stop line, before the knife rest"),
        ("JeepWreck", 1, "parked by the dead truck: one more kept for parts"),
        ("FuelPoint", 1, "the motor pool's fuel, away from every fire", "supplied"),
        ("TyreStack", 1, "the motor pool's spares"),
        ("SupplyYard", 1, "the stores that don't fit indoors, against the building's wall"),
        ("PalletStack", 2, "what the supply yard hasn't broken down yet", "supplied"),
        ("CrateStack", 1, "handed out through the wall's windows"),
        ("CrateStack", 2, "more of it while the convoys get through", "supplied"),
        ("Tent_Small_Bastion", 4, "the sections the building has no room for, in a line on the grass"),
        ("Tent_Large_Bastion", 1, "the platoon tent"),
        ("Duckboards", 1, "the tent line's mud"),
        ("MessTable", 2, "the mess, in a row by the kitchen"),
        ("RadioMast", 1, "the set that still reaches the road posts"),
        ("GeneratorPost", 1, "power for the set and the lights, away from the tents"),
        ("LanternPost", 2, "light at the entrance"),
        ("SandbagWall", 1, "a fallback between the gate and the door, thrown up after a bad night", "hit"),
        ("DugIn_Bastion", 1, "an earth bank in front of the nest after the night it came close", "hit"),
        ("CanteenStove", 1, "the canteen's stove dragged out to the kitchen door"),
        ("SandbagPile", 2, "spare bags where the line is being built up", "hit"),
        ("Clutter", 2, "what's left where people sit")],
    ("MILITARY", "OUTPOST"): [
        ("Tent_Small_Bastion", 2, "the section's tents, in a line on the grass"),
        ("SandbagPile", 2, "spare bags by the nest", "hit"),
        ("DugIn_Bastion", 1, "an earth bank in front of the nest", "hit"),
        ("CrateStack", 1, "the section's stores against the wall"),
        ("CrateStack", 1, "more of it while the convoys get through", "supplied"),
        ("BarrelGroup", 1, "water and fuel by the cook pot", "supplied"),
        ("LanternPost", 1, "light at the entrance")],
    ("BANDITS", "STRONGHOLD"): [
        ("ChopShop", 1, "what the crew does with what it takes: a jeep off Bastion stripped for parts"),
        ("Workshop", 1, "where what they take is made to work again"),
        ("ScrapHeap", 1, "what's no use after the stripping, thrown on a heap at the back"),
        ("DrumStore", 1, "the toll, paid in drums", "supplied"),
        ("LootContainer", 1, "what the crew took, under lock, someone always sitting on it"),
        ("PalletStack", 1, "loot not yet sorted", "thriving"),
        ("FuelPoint", 1, "the toll is paid in fuel"),
        ("BarrelGroup", 2, "more of the toll", "supplied"),
        ("TyreStack", 2, "tyres are the crew's sandbags"),
        ("TyreStack", 1, "more of them after someone got in", "hit"),
        ("Tent_Small_Civilian", 3, "those the house has no room for, on the grass, any way round"),
        ("ScrapShelter", 1, "a lean-to for the watch"),
        ("Bedrolls", 2, "sleeping it off by the tents"),
        ("CrateStack", 1, "stolen stores against the wall"),
        ("CrateStack", 1, "more when the road pays", "supplied"),
        ("Clutter", 2, "bottles round every seat"),
        ("Clutter", 3, "more bottles when the crew is fat and loud", "thriving")],
    ("BANDITS", "OUTPOST"): [
        ("TyreStack", 2, "tyres are the crew's sandbags"),
        ("BarrelGroup", 1, "the toll", "supplied"),
        ("Tent_Small_Civilian", 1, "one tent on the grass"),
        ("Bedrolls", 1, "sleeping by the tent"),
        ("CrateStack", 1, "stolen stores against the wall"),
        ("Clutter", 1, "bottles round every seat"),
        ("Clutter", 2, "more bottles when the crew is fat and loud", "thriving")],
    ("SURVIVORS", "STRONGHOLD"): [
        ("Tent_Small_Civilian", 3, "families who came in late, on the grass"),
        ("Tent_Large_Civilian", 1, "the shared tent"),
        ("Bedrolls", 1, "airing by the tents"),
        ("Garden", 3, "beds dug in rows: the village feeds itself"),
        ("Scarecrow", 1, "keeps the birds off the beds"),
        ("HayStore", 1, "hay for the animals the farms still keep"),
        ("Workshop", 1, "mending what can't be replaced"),
        ("CanteenStove", 1, "the school's canteen stove, carried out to cook for everyone", "supplied"),
        ("WoodBox", 1, "split wood stacked for the winter", "supplied"),
        ("Tent_Blue", 1, "a family's own tent, brought from home"),
        ("WaterBarrel", 2, "water where it's used: the kitchen, the garden"),
        ("WashingLine", 1, "the wash, on the grass"),
        ("Firewood", 1, "cut for the kitchen"),
        ("Firewood", 1, "a second pile while there are hands to cut it", "supplied"),
        ("CrateChairs", 2, "more seats at the fire"),
        ("CrateStack", 1, "the stores against the wall"),
        ("CrateStack", 2, "a full store in a good season", "supplied"),
        ("LanternPost", 3, "light at the entrance"),
        ("Clutter", 3, "what's left where people sit")],
    ("SURVIVORS", "OUTPOST"): [
        ("Tent_Small_Civilian", 1, "one tent on the grass"),
        ("Garden", 2, "beds dug in a row"),
        ("WaterBarrel", 1, "water by the kitchen"),
        ("Firewood", 1, "cut for the kitchen", "supplied"),
        ("CrateStack", 1, "the stores against the wall"),
        ("CrateStack", 1, "a full store in a good season", "supplied"),
        ("LanternPost", 2, "light at the entrance"),
        ("Clutter", 1, "what's left where people sit")],
    ("SURVIVORS", "SAFEHOUSE"): [
        ("Garden", 1, "a bed by the house"),
        ("WaterBarrel", 1, "water by the kitchen"),
        ("Firewood", 1, "cut for the kitchen", "supplied"),
        ("CrateStack", 1, "the stores against the wall"),
        ("LanternPost", 1, "a light left on for whoever comes")],
    ("INSTITUTE", "STRONGHOLD"): [
        ("RiotLine", 1, "the queue for triage"),
        ("WaitingCots", 1, "those who couldn't stand in it"),
        ("WarningSigns", 2, "said again at every closed road"),
        ("SupplyYard", 1, "what the lab runs on, against the building's wall"),
        ("PalletStack", 2, "deliveries not yet opened", "supplied"),
        ("CrateStack", 1, "the stores against the wall"),
        ("CrateStack", 2, "more while the courier still comes", "supplied"),
        ("Tent_Halcyon", 2, "the ward that overflowed, in a line on the grass"),
        ("FieldSurgery", 1, "what can't be made clean is treated outside the clean room"),
        ("BarrelGroup", 1, "the generators' fuel", "supplied"),
        ("Loudspeaker_Halcyon", 1, "the order heard at every corner"),
        ("Clutter", 1, "the smokers' corner")],
    ("INSTITUTE", "OUTPOST"): [
        ("WarningSigns", 1, "said again at the closed road"),
        ("CrateStack", 1, "the stores against the wall"),
        ("CrateStack", 1, "more while the courier still comes", "supplied"),
        ("BarrelGroup", 1, "the generator's fuel", "supplied"),
        ("Clutter", 1, "the smokers' corner")],
}

# The Traces kit (World > Traces > Kit on RZ_GameMode.et): kit field -> (composition name, parts).
# Built into Prefabs/ReforgerZ/Traces/<name>.et. Seats aren't used: traces are scenery only.
TRACES = {
    'm_sGrave': ('RZ_Trace_Grave', [P('WoodenFenceOld_01_pole.et', 0, 0), P('Junk_01_single_planks.et', 0, 0.05, 0, 1.0, 0, 90), P('SackUniversal_01_Pile.et', 0, 1.0)]),
    'm_sPyre': ('RZ_Trace_Pyre', [P('Pallet_01.et', 0, 0), P('Tire_UAZ469.et', 0.8, 0.6, 0, 0, 90), P('WoodLog_01_A.et', -0.6, 0.3, 40), P('WoodLog_01_A.et', 0.2, -0.7, 110), P('WoodenFenceOld_01_pole.et', -1.4, 0)]),
    'm_sCandleJars': ('RZ_Trace_CandleJars', [P('CandleHolderDecorative_01.et', 0, 0), P('CandleHolderDecorative_01.et', 0.25, 0.1), P('CandleHolderDecorative_01.et', -0.22, 0.12), P('CandleHolderDecorative_01.et', 0.05, 0.3)]),
    'm_sSeasonSheet': ('RZ_SeasonMark_Sheet', [P('WoodenFenceOld_01_pole.et', -1.2, 0), P('WoodenFenceOld_01_pole.et', 1.2, 0), P('BlanketBeach_01_red.et', 0, 0, 0, 1.0, 90)]),
    'm_sSeasonBoard': ('RZ_SeasonMark_Board', [P('WoodenFenceOld_01_pole.et', 0, 0), P('Junk_01_single_planks.et', 0, 0.06, 0, 1.4), P('Notice_A4_Vertical_V1.et', 0, 0.1, 0, 1.45)]),
    'm_sSeasonStencil': ('RZ_SeasonMark_Stencil', [P('WoodenFenceOld_01_pole.et', -0.5, 0), P('WoodenFenceOld_01_pole.et', 0.5, 0), P('Junk_01_single_planks.et', 0, 0.06, 0, 1.2)]),
    'm_sSeasonTape': ('RZ_SeasonMark_Tape', [P('WoodenFenceOld_01_pole.et', -1.5, 0), P('WoodenFenceOld_01_pole.et', 1.5, 0), P('Notice_A4_Vertical_V1.et', -1.5, 0.06, 0, 1.2)]),
    'm_sSeasonFlowers': ('RZ_SeasonMark_Flowers', [P('WoodenFenceOld_01_pole.et', 0, 0), P('FlowerpotFlowers_01_begonia_white.et', 0.2, 0.2)]),
    'm_sQueueCarA': ('RZ_Trace_QueueCar_A', [P('S105_wreck_static.et', 0, 0), P('Suitcase_01.et', 0, -0.2, 80, 1.35)]),
    'm_sQueueCarB': ('RZ_Trace_QueueCar_B', [P('S1203_wreck_static.et', 0, 0)]),
    'm_sQueueCarC': ('RZ_Trace_QueueCar_C', [P('UAZ469_wreck_blue_static.et', 0, 0), P('BlanketFolded_01_green.et', 1.6, 0.5, 30)]),
    'm_sSuitcases': ('RZ_Trace_Suitcases', [P('Suitcase_01.et', 0, 0, 20), P('Suitcase_01.et', 0.6, 0.3, 70, 0, 90), P('Suitcase_04_open.et', -0.7, 0.2), P('SackUniversal_01.et', 0.2, 0.9), P('BlanketFolded_01_green.et', -0.3, -0.7)]),
    'm_sGymDoor': ('RZ_Trace_GymDoor', [P('BunkBed_01_Mattress.et', 0, 0), P('BunkBed_01_Mattress.et', 0, 0, 5, 0.15), P('BlanketFolded_01_green.et', 1.2, 0.2), P('BlanketFolded_01_green.et', 1.2, 0.2, 0, 0.15), P('ChalkBoard_01.et', -1.5, 0.3)]),
    'm_sBus': ('RZ_Trace_Bus', [P('UAZ452_wreck_static.et', 0, 0)]),
    'm_sChurchDoor': ('RZ_Trace_ChurchDoor', [P('ChalkBoard_01.et', 0, 0), P('ChairOld_01.et', 1.0, 0.4, 200), P('BlanketFolded_01_green.et', 1.0, 0.4, 0, 0.45)]),
    'm_sRoadblock': ('RZ_Trace_Roadblock_D1', [P('Sandbag_01_long_burlap.et', 0, 1.2), P('Sandbag_01_long_burlap.et', -1.1, 0.6, 55), P('Sandbag_01_long_burlap.et', 1.1, 0.6, -55), P('WoodLog_01_A.et', 2.6, 1.0, 90), P('DeckChair_01_ochre.et', 0, -0.6), P('LitterInterior_USSR_01_tincan_V1.et', 0.5, -0.9), P('CrateWooden_01_plain.et', -0.8, -0.6), P('RadioSmall_01_black.et', -0.8, -0.6, 0, 0.62)]),
    'm_sNoticeWall': ('RZ_Trace_NoticeWall', [P('WoodenFenceOld_01_pole.et', -0.8, 0), P('WoodenFenceOld_01_pole.et', 0.8, 0), P('Junk_01_single_planks.et', 0, 0.06, 0, 1.3), P('Notice_A4_Vertical_V1.et', -0.5, 0.1, 0, 1.35), P('Notice_A4_Vertical_V1.et', -0.2, 0.11, 0, 1.4, 0, 6), P('Notice_A4_Vertical_V1.et', 0.1, 0.12, 0, 1.33), P('Notice_A4_Vertical_V1.et', 0.35, 0.13, 0, 1.42, 0, -4), P('Notice_A4_Vertical_V1.et', 0.6, 0.14, 0, 1.36)]),
    'm_sShutter': ('RZ_Trace_Shutter', [P('WoodenFenceOld_01_pole.et', -1.5, 0), P('WoodenFenceOld_01_pole.et', 1.5, 0), P('TinWall_01_3m.et', 0, 0.05)]),
    'm_sObservationPost': ('RZ_Trace_OP', [P('BlanketBeach_01_brown.et', 0, 0), P('LitterInterior_USSR_01_tincan_V2.et', 0.6, 0.4), P('WoodenFenceOld_01_pole.et', -1.0, 0.8)]),
    'm_sLastMeal': ('RZ_Trace_LastMeal', [P('TableOld_01.et', 0, 0), *SEAT('Chair_01_white.et', 'TableOld_01.et', 0, 0, 0, 'front', along=-0.4, slot=''), *SEAT('Chair_01_white.et', 'TableOld_01.et', 0, 0, 0, 'front', along=0.4, slot=''), *SEAT('Chair_01_white.et', 'TableOld_01.et', 0, 0, 0, 'back', along=0.4, slot=''), *SEAT('Chair_01_white.et', 'TableOld_01.et', 0, 0, 0, 'back', along=-0.5, gap=SEAT_LEFT, turn=30, slot=''), P('LitterInterior_USSR_01_bowl.et', -0.3, 0.2, 0, 0.78), P('LitterInterior_USSR_01_bowl.et', 0.3, -0.2, 0, 0.78), P('LitterInterior_USSR_01_mug.et', 0, 0, 0, 0.78), P('RadioSmall_01_black.et', 0.8, 0, 0, 0.78)]),
    'm_sHeldOut': ('RZ_Trace_HeldOut', [P('Junk_01_single_planks.et', 0, 0, 0, 0, 0, 70), P('Junk_01_single_planks.et', 0.8, 0.1, 10, 0, 0, 65), P('Junk_01_single_planks.et', -0.9, 0.3)]),
    'm_sEdgeChair': ('RZ_Trace_EdgeChair', [P('ChairOld_01.et', 0, 0), P('BlanketFolded_01_green.et', 0, 0, 0, 0.45), P('CandleHolderDecorative_01.et', 0.5, 0.3)]),
    'm_sIOUs': ('RZ_Trace_IOUs', [P('WoodenFenceOld_01_pole.et', -0.6, 0), P('WoodenFenceOld_01_pole.et', 0.6, 0), P('Junk_01_single_planks.et', 0, 0.06, 0, 1.3), P('Notice_A4_Vertical_V1.et', -0.3, 0.1, 0, 1.35), P('Notice_A4_Vertical_V1.et', 0, 0.11, 0, 1.38), P('Notice_A4_Vertical_V1.et', 0.3, 0.12, 0, 1.34), P('CrateWooden_01_plain.et', 0, 0.8), P('LitterInterior_USSR_01_tincan_V1.et', 0, 0.8, 0, 0.62)]),
    'm_sDroppedRun': ('RZ_Trace_DroppedRun', [P('Suitcase_01.et', 0, 0, 30, 0, 90), P('Notebook_01_Stack.et', 0.6, 0.4, 50), P('Wheelbarrow_01.et', -1.2, 0.3, 60, 0, 0, 80)]),
    'm_sWaitingRoom': ('RZ_Trace_WaitingRoom', [P('Chair_01_white.et', -1.5, 0, 180), P('Chair_01_white.et', -0.9, 0, 180), P('Chair_01_white.et', -0.3, 0, 180), P('Chair_01_white.et', 0.3, 0, 180), P('Chair_01_white.et', 0.9, 0, 180), P('Chair_01_white.et', 1.5, 0, 180), P('ChalkBoard_01.et', 2.4, 0.5, -30)]),
    'm_sFallenPost': ('RZ_Trace_FallenPost', [P('ChalkBoard_01.et', 0, 0), P('Notice_A4_Vertical_V1.et', 0.3, -0.04, 180, 1.3), P('BarrelMetal_01_rust.et', 1.6, -0.8), P('Sandbag_01_single_burlap.et', -1.4, -0.6), P('Sandbag_01_single_burlap.et', -1.0, -0.9, 40)]),
    'm_sWaterQueue': ('RZ_Trace_WaterQueue', [P('Bucket_01.et', 0.0, 0), P('Bucket_01.et', 0.7, 0), P('Bucket_01.et', 1.4, 0), P('Bucket_02.et', 2.1, 0), P('Bucket_02.et', 2.8, 0)]),
    'm_sScorch': ('RZ_Trace_Scorch', [P('LitterInterior_USSR_01_pile_V1.et', 0, 0), P('WoodLog_01_A.et', 0.6, 0.3, 70), P('WoodLog_01_A.et', -0.5, -0.2, 20)]),
    'm_sTruckBastion': ('RZ_Trace_TruckWreck_Bastion', [P('M923A1_wreck_static.et', 0, 0), P('CrateWooden_02_1x1x1.et', 2.2, -1.5, 20), P('CrateWooden_02_1x1x1.et', 2.6, 0.2, 70)]),
    'm_sTruckRedHand': ('RZ_Trace_TruckWreck_RedHand', [P('Ural4320_wreck_static.et', 0, 0), P('CrateWooden_02_1x1x1.et', 2.3, -1.2, 35)]),
    'm_sTruckHaven': ('RZ_Trace_TruckWreck_Haven', [P('Ural4320_wreck_static.et', 0, 0), P('SackUniversal_01_Pile.et', 2.0, -1.0)]),
    'm_sTruckHalcyon': ('RZ_Trace_TruckWreck_Halcyon', [P('UAZ452_wreck_static.et', 0, 0), P('CrateWooden_02_1x1x1.et', 1.8, -0.8, 15)]),
    'm_sRoadPostRelic': ('RZ_Trace_RoadPostRelic', [P('Sandbag_01_long_burlap.et', 0, 1.2), P('Sandbag_01_long_burlap.et', -1.1, 0.6, 55), P('Sandbag_01_long_burlap.et', 1.1, 0.6, -55), P('SignNoEntryBothDir_01_pole.et', 1.8, -0.5, 30, 0.2, 0, 88)]),
    'm_sCross': ('RZ_Trace_Crosses', [P('WoodenFenceOld_01_pole.et', 0, 0), P('Junk_01_single_planks.et', 0, 0.05, 0, 0.9, 0, 90)]),
    'm_sSheets': ('RZ_Trace_Sheets', [P('Notice_A4_Vertical_V1.et', 0, 0, 20, 0.01, -90), P('Notice_A4_Vertical_V1.et', 0.35, 0.2, 75, 0.01, -90), P('Notice_A4_Vertical_V1.et', -0.3, 0.4, 140, 0.01, -90)]),
    'm_sLantern': ('RZ_Trace_Lantern_Lit', [P('LampKerosene_01_green.et', 0, 0)]),
}

# Openings (Guidance > Opening scenes on RZ_GameMode.et, docs/DESIGN.md "Openings"): key -> (composition
# name, parts). Built into Prefabs/ReforgerZ/Guidance/Openings/<name>.et. They stand outside the door you
# come out of, +Z away from the wall. A reason's three (hiding, group, fever: why today) stand in front of
# the door to one side; a background's id, its own things, on the other side.
OPENINGS = {
    'hiding.1': ('RZ_Opening_Hiding_Cellar', [P('Bucket_02.et', 0, 0), P('BlanketFolded_01_green.et', 0.8, 0.2, 20), P('BeerBottle_01_empty_brown.et', -0.5, 0.3, 0, 0, 90), P('LitterInterior_USSR_01_tincan_V1.et', -0.3, -0.4), P('LitterInterior_USSR_01_tincan_V2.et', 0.2, -0.6), P('CandleHolderDecorative_01.et', 1.2, -0.3)]),
    'hiding.2': ('RZ_Opening_Hiding_Pantry', [P('CrateWooden_01_plain.et', 0, 0, 5), P('LitterInterior_USSR_01_tincan_V2.et', -0.2, 0, 0, 0.62), P('LitterInterior_USSR_01_tincan_V1.et', 0.15, 0.1, 0, 0.62), P('LampKerosene_01_green.et', 0.3, -0.1, 0, 0.62), P('Junk_01_single_mattress.et', 1.6, 0.4, 85)]),
    'hiding.3': ('RZ_Opening_Hiding_Bolthole', [P('Junk_01_single_planks.et', 0, 0, 15), P('Junk_01_single_planks.et', 0.3, 0.5, 40), P('ChairOld_01.et', -1.1, 0.3, 70, 0.25, 0, 90), P('Bucket_01.et', 1.3, -0.2)]),
    'group.1': ('RZ_Opening_Group_Bedrolls', [P('BlanketBeach_01_brown.et', 0, 0, 5), P('BlanketBeach_01_brown.et', 1.1, 0.1, -5), P('BlanketBeach_01_red.et', -1.1, 0, 10), P('BlanketFolded_01_green.et', 2.1, 0.6), P('SackUniversal_01.et', 0.5, 1.4, 70)]),
    'group.2': ('RZ_Opening_Group_Table', [P('TableOld_01.et', 0, 0, 8), P('Notebook_01_Stack.et', -0.2, 0.05, 30, 0.78), P('LampKerosene_01_green.et', 0.4, -0.1, 0, 0.78), *SEAT('ChairOld_01.et', 'TableOld_01.et', 0, 0, 8, 'left', slot=''), *SEAT('ChairOld_02.et', 'TableOld_01.et', 0, 0, 8, 'right', slot=''), *SEAT('Chair_01_white.et', 'TableOld_01.et', 0, 0, 8, 'front', slot=''), *SEAT('Stool_01.et', 'TableOld_01.et', 0, 0, 8, 'back', slot='')]),
    'group.3': ('RZ_Opening_Group_Packs', [P('Suitcase_01.et', 0, 0, 10), P('Suitcase_01.et', 0.5, 0.4, 80, 0, 90), P('SackUniversal_01_Pile.et', -1.0, 0.2, 20), P('Wheelbarrow_01.et', 1.5, -0.2, 30), P('BlanketFolded_01_green.et', 0.1, -0.6)]),
    'fever.1': ('RZ_Opening_Fever_Barricade', [P('Junk_01_single_planks.et', 0, 0, 0), P('Junk_01_single_planks.et', 0.2, 0.3, 20), P('Junk_01_single_planks.et', -0.4, 0.5, 70), P('ChairOld_02.et', 1.2, 0.4, 200), P('Pallet_01.et', -1.3, 0.2, 90, 0.6, 0, 80), P('Bucket_01.et', 0.9, -0.6)]),
    'fever.2': ('RZ_Opening_Fever_Sickbed', [P('Junk_01_single_mattress.et', 0, 0, 90), P('Bucket_02.et', 1.3, 0.3), P('LitterInterior_USSR_01_mug.et', 1.1, -0.3), P('LitterInterior_USSR_01_bowl.et', 1.4, -0.5), P('BlanketFolded_01_green.et', -0.2, 0.1, 30, 0.12)]),
    'fever.3': ('RZ_Opening_Fever_Vigil', [P('ChairOld_01.et', 0, 0, 180), P('BlanketFolded_01_green.et', 0, 0, 0, 0.45), P('CandleHolderDecorative_01.et', 0.6, 0.2), P('LitterInterior_USSR_01_mug.et', 0.5, -0.2), P('LitterInterior_USSR_01_tincan_V1.et', -0.6, 0.3)]),
    'nurse': ('RZ_Opening_Props_Nurse', [P('BlanketFolded_01_green.et', 0, 0, 10), P('Bucket_01.et', 0.7, 0.3), P('ToolsSurgical_01_autoclavedrum.et', -0.6, 0.2), P('LitterInterior_USSR_01_mug.et', 0.3, -0.4)]),
    'doctor': ('RZ_Opening_Props_Doctor', [P('ChairOld_01.et', 0, 0, 160), P('FileOffice_01_Stack.et', 0, 0, 20, 0.45), P('ToolsSurgical_01_autoclavedrum.et', 0.8, 0.3), P('LampKerosene_01_green.et', -0.6, 0.2)]),
    'mechanic': ('RZ_Opening_Props_Mechanic', [P('Tire_UAZ469.et', 0, 0, 0, 0, 90), P('Junk_01_pile_small.et', 1.4, 0.4, 30), P('BarrelMetal_01_blue.et', -1.0, 0.3), P('CrateWooden_01_plain.et', 0.4, -0.9, 15)]),
    'hunter': ('RZ_Opening_Props_Hunter', [P('WoodLog_01_A.et', 0, 0, 80), P('SackUniversal_01.et', 0.6, 0.6, 30), P('DeerSkull_01_board.et', -0.8, 0.5, 200), P('BlanketFolded_01_green.et', 0.9, -0.3)]),
    'police': ('RZ_Opening_Props_Police', [P('ChairOld_02.et', 0, 0, 190), P('CrateWooden_01_plain.et', 0.8, 0.2, 5), P('FileOffice_01_Stack.et', 0.8, 0.2, 40, 0.62), P('WoodenFenceOld_01_pole.et', -0.9, 0.4), P('Notice_A4_Vertical_V1.et', -0.9, 0.48, 0, 1.3)]),
    'excon': ('RZ_Opening_Props_ExCon', [P('Junk_01_single_mattress.et', 0, 0, 85), P('BeerBottle_01_empty_brown.et', 1.2, 0.3), P('LitterInterior_USSR_01_tincan_V1.et', 1.0, -0.4), P('Stool_01.et', -1.3, 0.2)]),
    'labtech': ('RZ_Opening_Props_LabTech', [P('TableOld_01_white.et', 0, 0, 5), P('Notebook_01_Stack.et', -0.3, 0, 20, 0.78), P('ToolsSurgical_01_autoclavedrum.et', 0.35, 0.1, 0, 0.78), *SEAT('Chair_01_white.et', 'TableOld_01_white.et', 0, 0, 5, 'back', slot='')]),
    'radioham': ('RZ_Opening_Props_Radio', [P('CrateWooden_01_plain.et', 0, 0, 10), P('RadioSmall_01_black.et', 0, 0, 30, 0.62), P('Antenna_R161_01.et', -1.0, 0.4), P('GeneratorPortable_US_01_Off.et', 1.1, 0.2, 75)]),
    'unemployed': ('RZ_Opening_Props_Unemployed', [P('BlanketBeach_01_brown.et', 0, 0, 80), P('Suitcase_04_open.et', 1.3, 0.3, 20), P('LitterInterior_USSR_01_tincan_V2.et', -0.6, 0.6), P('LitterInterior_USSR_01_mug.et', -0.4, 0.8)]),
    'soldier': ('RZ_Opening_Props_Soldier', [P('Sandbag_01_single_burlap.et', 0, 0, 10), P('Sandbag_01_single_burlap.et', 0.1, 0, 30, 0.17), P('CrateWooden_02_1x1x1.et', 1.2, 0.4, 15), P('LanternMilitary_US_01.et', 1.2, 0.4, 0, 1.0)]),
    'raider': ('RZ_Opening_Props_Raider', [P('SackUniversal_01_Pile.et', 0, 0, 20), P('Suitcase_01.et', 1.2, 0.2, 70, 0, 90), P('BeerBottle_01_group.et', -0.9, 0.3), P('BarrelMetal_01_rust.et', -1.2, -0.6)]),
    'elder': ('RZ_Opening_Props_Elder', [P('Armchair_01.et', 0, 0, 170), P('Stool_01.et', 0.9, 0.2), P('TablePhotoFrame_01.et', 0.9, 0.2, 200, 0.45), P('CandleHolderDecorative_01.et', 0.75, 0.3, 0, 0.45), P('BlanketFolded_01_green.et', 0, 0, 0, 0.45)]),
    'drifter': ('RZ_Opening_Props_Drifter', [P('Suitcase_01.et', 0, 0, 30), P('SackUniversal_01.et', 0.8, 0.3, 60), P('BlanketBeach_01_red.et', -0.2, 1.0, 85)]),
}
