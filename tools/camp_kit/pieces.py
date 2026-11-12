"""The camp-life pieces: one source for their prefabs (build.py) and their drawings (render.py).
Edit a piece here, then run build.py (prefabs) and render.py (the plans page). Units: metres. x right, z forward; +Z is the piece's front
(toward the camp centre for inner pieces, outward for perimeter pieces). Yaw in degrees.

FP: hand-typed prop sizes (w, d, shape, fill-kind): the drawings' shapes and fills, and the
fallback size of a prop footprints.json hasn't measured (each piece's footprint is its props'
measured boxes: footprints.py, docs/SITES.md).
P(name, x, z, yaw, y=0, pitch=0, roll=0) places a prop; S(tag, x, z, anim, look_x, look_z) a seat.

Every piece stands outdoors (camp life never puts a piece inside a building; only a quartermaster's
counter stands where he does, indoors too when it fits): outdoor props only. No cabinets, shelves,
sinks, towel rails, shower curtains or hospital furniture; tables and chairs only where people
dragged them out to use them (a table, a sofa round a fire).
"""

FP = {
    "Fireplace_01.et": (1.2, 1.2, "c", "fire"), "Campfire_burning.et": (0.5, 0.5, "c", "fire"),
    "WoodLog_01_A.et": (0.3, 2.0, "r", "wood"), "WoodPile_01_A.et": (1.0, 2.6, "r", "wood"), "LogChopping_01.et": (0.5, 0.5, "c", "wood"), "WoodKindling_01.et": (0.6, 0.5, "r", "wood"),
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
}

def P(name, x, z, yaw=0, y=0, pitch=0, roll=0):
    return ("p", name, x, z, yaw, y, pitch, roll)

def S(tag, x, z, anim, lx, lz):
    return ("s", tag, x, z, anim, lx, lz)

# Each piece: folder, use, role, one line of intent, [parts]
C = {}
def piece(name, folder, use, role, intent, parts):
    C[name] = (folder, use, role, intent, parts)

# ------------------------------------------------------------------- Common
piece("FirePit", "Common", "Sit", "Centre", "A fire ringed by whatever people sat on: logs, a crate, a sack. Wood stacked to one side, a kettle bucket.", [
    P("Fireplace_01.et", 0, 0), P("WoodLog_01_A.et", 1.7, 0.3, 74), P("WoodLog_01_A.et", -1.5, -0.7, 18), P("CrateWooden_01_plain.et", 0.4, -1.8, 8),
    P("SackUniversal_01.et", -0.6, 1.8, 140), P("WoodPile_01_A.et", 3.1, -1.2, 102), P("Bucket_01.et", 0.9, 0.9), P("LitterInterior_USSR_01_mug.et", -1.1, 0.6),
    P("LitterInterior_USSR_01_tincan_V1.et", 1.2, -1.1),
    S("RZ_Sit", 1.7, 0.4, "sit", 0, 0), S("RZ_Sit", -1.5, -0.6, "sit", 0, 0), S("RZ_Sit", 0.4, -1.8, "sit", 0, 0), S("RZ_Sit", -0.6, 1.7, "sit", 0, 0)])
piece("Table", "Common", "Sit", "Centre", "The meal's table: a picnic table set a little crooked, a lamp, tins and mugs left on it, a bench of crates at the end.", [
    P("PicnicTable_01.et", 0, 0, 7), P("LampKerosene_01_green.et", 0.4, 0.05, 0, 0.78), P("LitterInterior_USSR_01_bowl.et", -0.4, 0.1, 0, 0.78),
    P("LitterInterior_USSR_01_mug.et", 0.1, -0.15, 0, 0.78), P("CrateWooden_01_plain.et", 1.6, 0.2, 95), P("Bucket_02.et", -1.5, -1.1),
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
    P("TentSmall_02_red.et", 0, 0, -6), P("SackUniversal_01.et", 1.4, 0.9, 60), P("BlanketFolded_01_green.et", -1.3, 1.0, 20), P("CrateWooden_01_plain.et", -1.3, 1.0, 15),
    P("Bucket_02.et", 1.2, -1.0)])
piece("Tent_Large_Civilian", "Common", "Sleep", "Inner", "A big family tent, a chair at the door with a blanket over it, the sacks and the water bucket by its side.", [
    P("TentBig_01_orange.et", 0, 0, 4), P("ChairOld_02.et", 1.0, 2.6, 200), P("BlanketFolded_01_green.et", 1.0, 2.6, 200, 0.45),
    P("SackUniversal_01_Pile.et", 2.4, -1.4, 30), P("Bucket_01.et", -1.9, -1.8)])
piece("Firewood", "Common", "Chop", "Inner", "A chopping block in a scatter of split logs, the stack behind it.", [
    P("WoodPile_01_A.et", -1.6, 0.2, 86), P("LogChopping_01.et", 0.7, 0.3), P("WoodKindling_01.et", 1.6, -0.5, 31), P("WoodKindling_01.et", 1.5, 0.9, 77),
    P("WoodKindling_01.et", 0.2, -1.2, 120), P("Wheelbarrow_01.et", -0.4, 1.8, 150),
    S("RZ_Chop", 0.7, 1.1, "loiter", 0, -0.8)])
piece("CookPot", "Common", "Cook", "Inner", "A pot over a fire on a pole tripod, a grill for meat, crates as a counter, bowls drying.", [
    P("Fireplace_01.et", 0, 0), P("WoodenFenceOld_01_pole.et", 0.5, 0, 0, 0, 0, -20), P("WoodenFenceOld_01_pole.et", -0.25, 0.43, 0, 0, 18, 10), P("WoodenFenceOld_01_pole.et", -0.25, -0.43, 0, 0, -18, 10),
    P("Bucket_01.et", 0, 0, 0, 0.5), P("SkewerGrill_01.et", 1.4, 0.6, 28), P("CrateWooden_01_plain.et", -1.6, 0.4, 82), P("CrateWooden_01_plain.et", -1.7, -0.6, 95),
    P("LitterInterior_USSR_01_bowl.et", -1.6, 0.3, 0, 0.62), P("LitterInterior_USSR_01_bowl.et", -1.7, -0.5, 0, 0.62), P("SackUniversal_01.et", 1.1, -1.4, 40),
    S("RZ_Cook", 0.1, 1.3, "loiter", 0, -1.3)])
piece("WashStand", "Common", "Wash", "Inner", "A mirror nailed to a post, a basin and a folded towel on a crate below it, a bucket of water; puddled ground.", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("MirrorBathroom_01.et", 0, 0.08, 0, 1.5), P("CrateWooden_01_plain.et", 0.1, 0.55, 4), P("Bucket_02.et", -0.05, 0.55, 0, 0.62),
    P("BlanketFolded_01_green.et", 0.35, 0.55, 8, 0.62), P("Bucket_01.et", -0.6, 0.7),
    S("RZ_Wash", 0.05, 1.2, "loiter", 0, -1.2)])
piece("Latrine", "Common", "None", "Edge", "Two tin sheets in an L for privacy, a plank over a bucket, a sack of lime.", [
    P("TinWall_01_3m.et", 0, 1.5, 3), P("TinWall_01_3m.et", -1.5, 0, 88), P("Bucket_01.et", -0.6, 0.8), P("Junk_01_single_planks.et", -0.6, 0.8, 5, 0.35),
    P("SackUniversal_01.et", 0.6, 0.9, 70)])
piece("DryingLine", "Common", "Work", "Edge", "A clothesline frame with its lines strung, a laundry basket and a stool to stand on.", [
    P("ClotheslineFrame_01.et", 0, 0, 4), P("BasketLaundry_01.et", -0.4, 0.8, 18), P("Stool_01.et", 0.7, 0.9),
    S("RZ_Work", 0.2, 1.0, "loiter", 0, -1.0)])

piece("Duckboards", "Common", "None", "Inner", "Pallets laid flat end to end: a dry walk from the tents to the fire, the ground under it trodden bare.", [
    P("Pallet_01.et", 0, -1.3, 90), P("Pallet_01.et", 0.05, 0, 92), P("Pallet_01.et", -0.05, 1.3, 88)])
piece("CrateChairs", "Common", "Sit", "Centre", "Mismatched chairs round a crate table, a lantern on it, two more lanterns on poles.", [
    P("CrateWooden_01_plain.et", 0, 0, 14), P("ChairOld_01.et", 0.1, 1.0, 190), P("ChairOld_02.et", 1.0, -0.3, 255), P("Stool_01.et", -1.0, -0.4),
    P("LanternMilitary_US_01.et", 0.1, 0, 0, 0.62), P("WoodenFenceOld_01_pole.et", 1.9, 1.1), P("WoodenFenceOld_01_pole.et", -2.1, 1.3),
    P("LanternMilitary_US_01.et", 1.9, 1.1, 0, 1.6), P("LanternMilitary_US_01.et", -2.1, 1.3, 0, 1.6), P("BeerBottle_01_empty_brown.et", -0.3, 0.2, 0, 0.62),
    S("RZ_Sit", 0.1, 1.0, "sit", 0, -1.0), S("RZ_Sit", 1.0, -0.3, "sit", -1.0, 0.3), S("RZ_Sit", -1.0, -0.4, "sit", 1.0, 0.4)])
piece("Clutter", "Common", "None", "Inner", "Life left lying about: sacks, a guitar on one, tins, bottles, a wheelbarrow tipped on a crate.", [
    P("SackUniversal_01.et", 0, 0, 30), P("SackUniversal_01.et", 0.5, 0.4, 70), P("BlanketFolded_01_green.et", 1.1, -0.3, 10), P("Guitar_01.et", 0.3, 0.2, 55, 0.25, 30, 0),
    P("LitterInterior_USSR_01_tincan_V1.et", -0.6, 0.5), P("LitterInterior_USSR_01_tincan_V2.et", -0.75, 0.3), P("BeerBottle_01_empty_brown.et", -0.4, -0.5),
    P("CrateWooden_01_plain.et", 1.9, -1.0, 40), P("Wheelbarrow_01.et", 1.6, -0.2, 120, 0, 0, 25)])
piece("RadioTable", "Common", "Sit", "Centre", "A kitchen table with a radio, a lamp and a chair pulled out; a notebook of frequencies.", [
    P("TableOld_01.et", 0, 0, -4), P("RadioSmall_01_black.et", 0, -0.1, 180, 0.78), P("LampKerosene_01_green.et", 0.45, 0.1, 0, 0.78), P("Notebook_01_Stack.et", -0.35, 0.05, 20, 0.78),
    P("ChairOld_01.et", 0.05, 0.85, 172), P("CrateWooden_01_plain.et", -1.2, -0.4, 80),
    S("RZ_Sit", 0.05, 0.85, "sit", 0, -0.8)])
piece("ScrapShelter", "Common", "Sleep", "Inner", "A lean-to of pallets and a tin roof over a mattress; a blanket and a lamp inside.", [
    P("Pallet_01.et", -0.9, 0, 88, 0.6, 0, 90), P("Pallet_01.et", 0.95, 0.1, 94, 0.6, 0, 90), P("TinWall_01_3m.et", 0, -0.1, 2, 1.3, 75, 0),
    P("Junk_01_single_mattress.et", 0, 0, 4), P("BlanketBeach_01_brown.et", 0.15, 0.1, 10, 0.08), P("LampKerosene_01_green.et", 0.6, 0.9)])
piece("Barricade_Fence", "Common", "None", "Approach", "An old fence panel braced with stood-up pallets, a pile of spares.", [
    P("WoodenFenceOld_01_4m.et", 0, 0, 3), P("Pallet_01.et", 2.3, 0.1, 96, 0.6, 0, 90), P("Pallet_01.et", 2.5, 0.5, 110, 0.6, 0, 90), P("Pallet_01_Pile.et", -2.7, 0.7, 20),
    P("TirePile_01_small.et", -1.0, 1.0)])
piece("Kennel", "Common", "None", "Edge", "A dog house with a bowl and a chewed blanket. No dog.", [
    P("DogHouse_01.et", 0, 0, 12), P("LitterInterior_USSR_01_bowl.et", 0.6, 0.9), P("BlanketFolded_01_green.et", -0.6, 0.8, 40)])

# ------------------------------------------------------------------- Bastion
piece("Tent_Small_Bastion", "Bastion", "Sleep", "Inner", "A two-man tent squared to the line, kit by the flap, a sandbag holding the corner.", [
    P("TentSmallUS_01.et", 0, 0), P("CrateWooden_02_1x1x1.et", 1.4, 0.9, 0), P("Sandbag_01_single_burlap.et", -1.1, -1.3, 90), P("BlanketFolded_01_green.et", 1.4, 0.9, 0, 1.0)])
piece("Tent_Large_Bastion", "Bastion", "Sleep", "Inner", "The barrack tent, a duckboard of pallets at its door, a rack of crates down one side.", [
    P("TentUS_01_Assembled.et", 0, 0), P("Pallet_01.et", 0, 3.9, 0), P("Pallet_01.et", 0, 4.7, 0), P("CrateWooden_02_1x1x1.et", 3.0, 1.5, 0), P("CrateWooden_02_1x1x1.et", 3.0, 0.4, 0),
    P("BarrelMetal_01_grey.et", -3.0, 2.6)])
piece("SentryNest_Bastion", "Bastion", "Watch", "Approach", "A curved sandbag nest, two layers high at the front, a crate seat and an ammo-coloured crate.", [
    P("Sandbag_01_long_burlap.et", 0, 1.3, 0), P("Sandbag_01_long_burlap.et", 0, 1.3, 0, 0.45), P("Sandbag_01_long_burlap.et", -1.4, 0.75, 48), P("Sandbag_01_long_burlap.et", 1.4, 0.75, -48),
    P("Sandbag_01_end_burlap.et", -2.0, -0.2, 80), P("Sandbag_01_end_burlap.et", 2.0, -0.2, -80), P("CrateWooden_01_plain.et", 0.7, -0.5, 10), P("LitterInterior_USSR_01_mug.et", 0.6, -0.5, 0, 0.62),
    S("RZ_Watch", 0, 0.2, "loiter", 0, 10)])
piece("RosterBoard", "Bastion", "Gather", "Centre", "The roll-call board, sheets pinned unevenly, a crate with the clipboard; people stand in a loose arc.", [
    P("ChalkBoard_01.et", 0, 0), P("Notice_A4_Vertical_V1.et", 0.3, -0.04, 180, 1.3), P("Notice_A4_Vertical_V1.et", -0.25, -0.04, 176, 1.25), P("CrateWooden_01_plain.et", 1.2, -0.6, 25),
    P("FileOffice_01_Stack.et", 1.2, -0.6, 10, 0.62),
    S("RZ_Gather", -1.3, -1.7, "loiter", 1.3, 1.7), S("RZ_Gather", -0.4, -2.0, "loiter", 0.4, 2.0), S("RZ_Gather", 0.6, -1.9, "loiter", -0.6, 1.9), S("RZ_Gather", 1.5, -1.5, "loiter", -1.5, 1.5)])
piece("Barricade_Sandbag", "Bastion", "None", "Approach", "A sandbag wall with a slight dog-leg, ends tapered, a spare bag dropped.", [
    P("Sandbag_01_long_burlap.et", -1.0, 0, 3), P("Sandbag_01_long_burlap.et", 1.0, 0.1, -6), P("Sandbag_01_long_burlap.et", -1.0, 0, 3, 0.45), P("Sandbag_01_end_burlap.et", 2.2, 0.35, -30),
    P("Sandbag_01_end_burlap.et", -2.2, 0.05, 185), P("Sandbag_01_single_burlap.et", 0.3, -0.8, 40)])
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
    P("TirePile_01_small.et", -0.9, 1.0), P("Pallet_01.et", 0.6, 1.3, -8, 0.6, 0, 85), P("Sandbag_01_long_burlap.et", 1.7, 0.6, -60), P("Armchair_01.et", 0, 0, 175),
    P("BeerBottle_01_group.et", 0.9, -0.4),
    S("RZ_Watch", 0, 0.1, "sit", 0, 10)])
piece("CardTable", "RedHand", "Sit", "Centre", "A kitchen table, crates and a chair round it, bottles and a lamp; a barrel as a bin.", [
    P("TableOld_01.et", 0, 0, 12), P("CrateWooden_01_plain.et", 0.1, 1.0, 5), P("ChairOld_02.et", 1.1, -0.2, 260), P("CrateWooden_01_plain.et", -1.1, -0.4, 80),
    P("BeerBottle_01_group.et", 0.25, 0.1, 0, 0.78), P("LampKerosene_01_green.et", -0.35, -0.1, 0, 0.78), P("BarrelMetal_01_rust.et", -1.6, 1.2),
    S("RZ_Sit", 0.1, 1.0, "sit", 0, -1.0), S("RZ_Sit", 1.1, -0.2, "sit", -1.1, 0.2), S("RZ_Sit", -1.1, -0.4, "sit", 1.1, 0.4)])
piece("TollBarrier", "RedHand", "Lean", "Approach", "Logs dragged across the road on tyres, a chair and a barrel fire for whoever takes the toll.", [
    P("WoodLog_01_A.et", 0, 1.5, 92), P("WoodLog_01_A.et", 0.2, 1.5, 86, 0.35), P("Tire_UAZ469.et", -1.6, 1.5, 0, 0, 90), P("Tire_UAZ469.et", 1.7, 1.4, 0, 0, 90),
    P("ChairOld_01.et", 2.3, -0.2, 205), P("BarrelMetal_01_rust.et", 2.9, 0.7), P("Campfire_burning.et", 2.9, 0.7, 0, 0.9),
    S("RZ_Lean", -1.8, 0.5, "lean_l", 0, 3), S("RZ_Lean", 2.3, -0.2, "smoke", 0, 3)])
piece("Barricade_Scrap", "RedHand", "None", "Approach", "Tin sheets lashed at angles, a tyre pile and junk, a pallet wedged in.", [
    P("TinWall_01_3m.et", -0.8, 0, 6), P("TinWall_01_3m.et", 1.4, 0.4, -18), P("TirePile_01_small.et", 3.2, 0.9), P("Junk_01_pile_small.et", -2.9, 0.5, 30), P("Pallet_01.et", 0.4, -0.3, 10, 0.6, 0, 80)])
piece("RedHandSheet", "RedHand", "None", "Approach", "A tin sheet stood on two tyre piles, facing the road (the red hand decal still to come).", [
    P("TirePile_01_small.et", -1.0, 0), P("TirePile_01_small.et", 1.1, 0.1), P("TinWall_01_3m.et", 0, 0.05, 2, 0.6), P("Junk_01_single_planks.et", 0.2, -0.5, 25)])
piece("TrophyWall", "RedHand", "None", "Edge", "A long tin wall hung with skulls on boards and junk; a tyre leaning on it.", [
    P("TinWall_01_6m_A.et", 0, 0, 4), P("DeerSkull_01_board.et", -1.6, 0.1, 4, 1.6), P("DeerSkull_01_board.et", 0.3, 0.12, 4, 1.8), P("DeerSkull_01_board.et", 1.9, 0.14, 4, 1.5),
    P("Tire_UAZ469.et", -2.6, 0.5, 0, 0.3, 75), P("Junk_01_pile_small.et", 2.4, 0.9, 20)])
piece("Sofa", "RedHand", "Sit", "Centre", "A sofa and armchair round a barrel side table, bottles everywhere.", [
    P("Sofa_02_green.et", 0, 0, 4), P("Armchair_01.et", 1.9, 1.3, 238), P("BarrelMetal_01_rust.et", 0.9, 1.5), P("BeerBottle_01_group.et", 0.9, 1.5, 0, 0.9),
    P("BeerBottle_01_empty_brown.et", -0.6, 1.1), P("BeerBottle_01_empty_brown.et", 1.4, 0.4), P("Tire_UAZ469.et", -1.6, 1.4, 0, 0, 90),
    S("RZ_Sit", -0.5, 0.4, "sit", 0.9, 1.5), S("RZ_Sit", 0.5, 0.4, "sit", 0.9, 1.5), S("RZ_Sit", 1.9, 1.3, "sit", -0.9, 0.2)])

# ------------------------------------------------------------------- Haven
piece("WashingLine", "Haven", "Work", "Edge", "Two clothesline frames end to end across the yard, the washing on them, a basket and a bucket.", [
    P("ClotheslineFrame_01.et", -1.7, 0, 2), P("ClotheslineFrame_01.et", 1.7, 0.1, -2), P("BasketLaundry_01.et", -0.7, 0.9, 15), P("Bucket_01.et", 0.5, 1.0),
    S("RZ_Work", -0.2, 1.0, "loiter", 0, -1.0)])
piece("Garden", "Haven", "Work", "Edge", "Two vegetable beds side by side, sacks of seed and a bucket at their end, the wheelbarrow beside them.", [
    P("VegetableBed_01_small.et", -1.0, 0, 3), P("VegetableBed_01_small.et", 0.6, 0.2, -4), P("SackUniversal_01.et", 2.0, 1.1, 30), P("SackUniversal_01.et", 2.3, 0.6, 75),
    P("Wheelbarrow_01.et", 1.6, -1.9, 60), P("Bucket_02.et", -0.2, 1.8),
    S("RZ_Work", -0.2, 1.5, "loiter", 0, -1.0)])
piece("WaterBarrel", "Haven", "Work", "Inner", "A rain barrel and a second one beside it, buckets, a ladle mug on the lid.", [
    P("BarrelMetal_01_blue.et", 0, 0), P("BarrelMetal_01_grey.et", 0.7, 0.2), P("Bucket_01.et", -0.6, 0.6), P("Bucket_02.et", 0.4, 0.8), P("LitterInterior_USSR_01_mug.et", 0, 0, 0, 0.9),
    S("RZ_Work", 0, 1.0, "loiter", 0, -1.0)])
piece("BearDoor", "Haven", "None", "Approach", "A door-sized plank board leaned on a post at the way in (the bear decal still to come), a flowerpot.", [
    P("WoodenFenceOld_01_pole.et", 0, 0), P("Junk_01_single_planks.et", 0, 0.3, 2, 0.9, 0, 80), P("Junk_01_single_planks.et", 0.3, 0.3, 4, 0.9, 0, 82),
    P("FlowerpotFlowers_01_begonia_white.et", 0.9, 0.6)])
piece("Memorial", "Haven", "Work", "Centre", "A white table with candles, photos and names; flowers in front.", [
    P("TableOld_01_white.et", 0, 0, -3), P("CandleHolderDecorative_01.et", -0.4, 0, 0, 0.78), P("CandleHolderDecorative_01.et", 0, 0.12, 0, 0.78), P("CandleHolderDecorative_01.et", 0.4, -0.05, 0, 0.78),
    P("TablePhotoFrame_01.et", 0.2, -0.2, 180, 0.78), P("Photo_Square_01_V1.et", -0.2, -0.2, 10, 0.79), P("Notice_A4_Vertical_V1.et", 0, 0.25, 0, 0.79),
    P("FlowerpotFlowers_01_begonia_white.et", 0.8, 0.7), P("FlowerpotFlowers_01_begonia_white.et", -0.7, 0.8),
    S("RZ_Work", 0, 1.1, "loiter", 0, -1.1)])
piece("ChildrensCorner", "Haven", "None", "Inner", "A low chalkboard, a blanket on the ground, two stools, chalk tins.", [
    P("ChalkBoard_01.et", 0, 0, 5), P("BlanketBeach_01_red.et", 0.1, 1.3, 70), P("Stool_01.et", -0.9, 1.0), P("Stool_01.et", 0.9, 1.2),
    P("LitterInterior_USSR_01_tincan_V2.et", 0.3, 0.6)])
piece("VegStall", "Haven", "Work", "Edge", "A market stand with crates and sacks, a bucket of water, a chair behind.", [
    P("MarketStand_01.et", 0, 0), P("CrateWooden_01_plain.et", -0.6, -0.2, 6), P("CrateWooden_01_plain.et", 0.6, -0.2, -4), P("SackUniversal_01_Pile.et", 1.8, 0.5, 20),
    P("Bucket_01.et", -1.5, 0.3), P("ChairOld_01.et", 0.3, -1.1, 10),
    S("RZ_Work", 0, -1.0, "loiter", 0, 1.0)])

# ------------------------------------------------------------------- Halcyon
piece("Tent_Halcyon", "Halcyon", "Work", "Inner", "The ward tent, its flap to the yard: a field table out front with a sample drum and a lamp, a crate of cases by the door.", [
    P("TentFIA_01.et", 0, 0), P("TableMilitary_US_01.et", 0, 3.4), P("ToolsSurgical_01_autoclavedrum.et", 0.5, 3.4, 0, 0.78),
    P("LampKerosene_01_green.et", -0.6, 3.45, 0, 0.78), P("CrateWooden_02_1x1x1.et", 2.0, 2.9, 6),
    S("RZ_Work", 0, 4.2, "loiter", 0, -1.0)])
piece("FieldTable", "Halcyon", "Sit", "Centre", "A field table with notebooks and a sample drum, two stools, the cases stacked at its end with a lantern on top.", [
    P("TableMilitary_US_01.et", 0, 0), P("Notebook_01_Stack.et", 0.4, 0, 10, 0.78), P("FileOffice_01_Stack.et", -0.5, 0.1, -5, 0.78), P("ToolsSurgical_01_autoclavedrum.et", 0, -0.1, 0, 0.78),
    P("Stool_01.et", 0.1, 0.8), P("Stool_01.et", -0.2, -0.8), P("CrateWooden_02_1x1x1.et", 1.7, -0.1, 4), P("LanternMilitary_US_01.et", 1.7, -0.1, 0, 1.0),
    S("RZ_Sit", 0.1, 0.8, "sit", 0, -0.8), S("RZ_Sit", -0.2, -0.8, "sit", 0, 0.8)])
piece("ColdBox", "Halcyon", "Work", "Inner", "The cold store: sample cases stacked on a pallet off the wet ground, a drum on top, a barrel of cooling water beside it.", [
    P("Pallet_01.et", 0, 0, 3), P("CrateWooden_02_1x1x1.et", -0.1, 0, 3, 0.15), P("CrateWooden_01_plain.et", 0.5, 0.05, -8, 1.15), P("ToolsSurgical_01_autoclavedrum.et", -0.2, 0, 0, 1.15),
    P("BarrelMetal_01_blue.et", 1.2, 0.1),
    S("RZ_Work", 0, 1.0, "loiter", 0, -1.0)])
piece("Antenna", "Halcyon", "Work", "Edge", "The antenna on a stand with a radio on a crate and a cable run to the generator.", [
    P("Antenna_R161_01.et", 0, 0), P("CrateWooden_01_plain.et", 1.0, 0.6, 15), P("RadioSmall_01_black.et", 1.0, 0.6, 195, 0.62), P("Stool_01.et", 1.1, 1.3),
    S("RZ_Work", 1.1, 1.3, "loiter", -0.1, -0.7)])
piece("Generator", "Halcyon", "Work", "Edge", "The generator on a pallet, a fuel-coloured barrel, a toolbox crate.", [
    P("Pallet_01.et", 0, 0, 5), P("GeneratorPortable_US_01_Off.et", 0, 0, 5, 0.15), P("BarrelMetal_01_grey.et", -1.1, 0.4), P("CrateWooden_01_plain.et", 1.0, -0.6, 70),
    S("RZ_Work", 0, 1.0, "loiter", 0, -1.0)])
piece("SampleTable", "Halcyon", "Work", "Inner", "The sorting table under a net: sample drums and the log on a field table, the cases under it, worked standing.", [
    P("CamoNet_Small_US.et", 0, 0, 4), P("TableMilitary_US_01.et", 0, -0.3), P("ToolsSurgical_01_autoclavedrum.et", -0.5, -0.3, 0, 0.78), P("ToolsSurgical_01_autoclavedrum.et", -0.2, -0.35, 0, 0.78),
    P("Notebook_01_Stack.et", 0.5, -0.25, 12, 0.78), P("CrateWooden_01_plain.et", 0.3, -0.35, 2), P("LanternMilitary_US_01.et", 1.6, -1.2),
    S("RZ_Work", -0.3, 0.6, "loiter", 0, -1.0), S("RZ_Work", 0.6, 0.6, "loiter", 0, -1.0)])
piece("DeconShower", "Halcyon", "Wash", "Inner", "A shower on a pallet floor, a tin sheet stood up on each side for a screen, buckets and a bench for the clothes.", [
    P("Pallet_01.et", 0, 0), P("ShowerBeach_01.et", 0, 0, 0, 0.15), P("TinWall_01_3m.et", 0.75, -0.2, 90), P("TinWall_01_3m.et", -0.75, -0.2, 90),
    P("Bucket_02.et", -1.2, 1.1), P("ForestBench_01.et", 0.2, 1.7, 4),
    S("RZ_Wash", 0, 0, "loiter", 0, 1.0)])
piece("PlagueBoard", "Halcyon", "None", "Approach", "A plank board on two posts at the way in (the beak stencil still to come), a white crate.", [
    P("WoodenFenceOld_01_pole.et", -0.8, 0), P("WoodenFenceOld_01_pole.et", 0.8, 0.05), P("Junk_01_single_planks.et", 0, 0.06, 2, 0.9), P("Junk_01_single_planks.et", 0, 0.07, 2, 1.25),
    P("CrateWooden_01_plain.et", 1.3, 0.6, 20)])
piece("NotebookBench", "Halcyon", "Sit", "Centre", "A bench with notebooks and files, a lamp and a crate of sample drums.", [
    P("ForestBench_01.et", 0, 0, 3), P("Notebook_01_Stack.et", 0.7, 0.05, 10, 0.45), P("FileOffice_01_Stack.et", -0.7, 0, 0, 0.45), P("LampKerosene_01_green.et", 1.2, 0.3),
    P("CrateWooden_01_plain.et", -1.3, 0.8, 25), P("ToolsSurgical_01_autoclavedrum.et", -1.3, 0.8, 0, 0.62),
    S("RZ_Sit", -0.35, 0.3, "sit", 0, 2.0), S("RZ_Sit", 0.35, 0.3, "sit", 0, 2.0)])

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

# Where a piece stands when its place has moved into a building (docs/DESIGN.md, "Camp life":
# RZ_EDressAt, written as each kit piece's m_eAt). A piece not listed is Auto: the approach pieces go
# to the road entries (a watch or lean piece to the gate, the rest to the closed ones), everything
# else stays in the yard as the plan draws it. Spots: yard, front, door, gate, closed, counter.
AT = {
    "RedHandSheet": "gate", "PlagueBoard": "gate",
    "BearDoor": "door", "Memorial": "door", "DeconShower": "door",
    "TrophyWall": "front", "VegStall": "front",
}
AT.update({name: "counter" for name in C if name.startswith("Counter_")})

# The kits on RZ_GameMode.et (Territory > Camp life > Kits): (faction, place, what it is, plan). A
# plan is the camp drawn whole: (piece, x, z, yaw), metres from the camp's middle, +Z toward its front
# (the road in), yaw the piece's turn from facing the front. Most important first: a place takes them
# in this order until its piece count (stronghold 20, outpost 13, safehouse 8, field station 9, road
# post 4). The game lays the plan down whole, turned to face the road (or, by a building, in its yard
# with its back to the wall), and leaves out or nudges a piece that doesn't fit (docs/DESIGN.md,
# "Camp life"). A road post's plan has the road running up the middle (x = 0): pieces stand beside it.
# build.py refuses a plan whose pieces overlap.
KITS = [
    ("MILITARY", "STRONGHOLD", "Military order gone a little ragged: tents in a loose row behind a sandbag line with a gap for the road, the board and the flag at the heart, the stores' window to one side.", [
        ("Counter_Bastion", -10, 9, 0), ("SentryNest_Bastion", 0, 26, 0), ("RosterBoard", -3, 5, 8), ("FlagPole", 2, 3, 0), ("FirePit", 5, -3, 0),
        ("Barricade_Sandbag", -7, 24.5, 4), ("Barricade_Sandbag", 7, 24.5, -6), ("Table", -4, -4, 12), ("Tent_Small_Bastion", -14, -10, 4), ("Tent_Small_Bastion", -9, -12.5, -3),
        ("Tent_Large_Bastion", 1, -16, 2), ("CookPot", 15, -3, 30), ("Firewood", 18.5, -10.5, 60), ("ShavingStand", -16, -3, 90), ("CrateSeats", 10, 3, 20),
        ("DutyBoard", -8, 4, 10), ("Tarp", 15, 8, 25), ("Tent_Small_Bastion", 10, -13, -8), ("DryingLine", 21.5, -16.5, 35), ("Latrine", -22, -18, 140), ("Duckboards", -5, -8.5, 0)]),
    ("MILITARY", "OUTPOST", "A section at a farm on the junction: a nest and a sandbag run on the road, two tents squared to it, the fire, the table and the roll board in the yard.", [
        ("Counter_Bastion", -10, 7, 0), ("SentryNest_Bastion", 0, 20, 0), ("FirePit", 4, 0, 0), ("Barricade_Sandbag", -7, 18.5, 6), ("Tent_Small_Bastion", -8, -8, 6),
        ("Tent_Small_Bastion", -3, -10.5, -4), ("RosterBoard", -4, 4, 6), ("Table", -5, -2, 10), ("CookPot", 14, -3, 30), ("Firewood", 13, -11, 60),
        ("ShavingStand", -12, -2, 90), ("FlagPole", 2, 6, 0), ("Barricade_Sandbag", 7, 19.5, -6), ("Duckboards", -4.5, -6, 0)]),
    ("MILITARY", "ROAD_POST", "The checkpoint: a nest beside the barrier facing down the road, a sandbag run on the other verge, a fire barrel and a net to wait under.", [
        ("SentryNest_Bastion", 7.5, 2, 0), ("Barricade_Sandbag", -7.5, 2.5, 0), ("FireBarrel", 7.5, -4.5, 0), ("Tarp", -8.5, -4.5, 10)]),
    ("BANDITS", "STRONGHOLD", "A squat, not a camp: things where they were dropped, but round the fire. The toll on the road, the sofa round the barrel, tents tucked behind scrap, the fence's table by the way in.", [
        ("Counter_RedHand", -9, 4, 20), ("TollBarrier", -2, 25, 6), ("FireBarrel", 1, 2, 0), ("Sofa", -4, -2.5, 160), ("SentryNest_RedHand", 9, 22, -20),
        ("CardTable", 6, 6, 35), ("RedHandSheet", -10, 21, 25), ("Barricade_Scrap", 15, 16, -40), ("Tent_Small_Civilian", -12, -9, 30), ("Tent_Small_Civilian", -6, -13, -15),
        ("ScrapShelter", 4, -14, 10), ("CookPot", 13, -5, 60), ("Firewood", 17.5, -12, 80), ("TrophyWall", -19, 4, 95), ("Clutter", 8, -1, 0),
        ("Kennel", -16, 12, 40), ("FirePit", -6, 10, 0), ("Tent_Large_Civilian", 10, -17, -20), ("Latrine", 22.5, -20.5, 200), ("Barricade_Scrap", -17, 17, 30)]),
    ("BANDITS", "OUTPOST", "A crew camp on the road: the toll barrier and a lookout behind tyres, the fire barrel and the sofa, the card table, two tents at the back.", [
        ("Counter_RedHand", -7, 3, 20), ("TollBarrier", 0, 20, 0), ("FireBarrel", 1, 2, 0), ("Sofa", -3.5, -2.5, 160), ("SentryNest_RedHand", 8, 17, -20),
        ("CardTable", 6, 5, 35), ("Tent_Small_Civilian", -9, -9, 30), ("Tent_Small_Civilian", -3, -11, -15), ("CookPot", 10.5, -4, 60), ("Firewood", 14.5, -11.5, 80),
        ("RedHandSheet", -8, 17, 25), ("Clutter", 4.5, -2, 0), ("ScrapShelter", 3, -12, 10)]),
    ("BANDITS", "ROAD_POST", "A roadblock for the toll: the fire barrel and the card table on one verge, scrap and a net on the other.", [
        ("FireBarrel", 7, -2, 0), ("CardTable", 8, 4, 90), ("Barricade_Scrap", -8, 4, 0), ("Tarp", -8.5, -3, 15)]),
    ("SURVIVORS", "STRONGHOLD", "A village kept going: the long table and the memorial in the yard, the cook pot and the water by the kitchen side, the washing and the garden behind, the bear at the gate.", [
        ("Counter_Haven", 5, 11, -10), ("BearDoor", 0, 24, 0), ("Table", 2, 3, 6), ("CookPot", 9, -2, 40), ("Memorial", -6, 6, 20),
        ("WaterBarrel", 6, -9, 0), ("WashStand", 3, -11, 10), ("WashingLine", -9, -14, 12), ("Garden", -16, -6, 80), ("Barricade_Fence", -8, 23, 8),
        ("VegStall", 14, 9, -30), ("ChildrensCorner", -10, 2, 35), ("Firewood", 13.5, -9, 70), ("FirePit", -2, -4, 0), ("Tent_Small_Civilian", 15.5, -16, -25),
        ("CrateChairs", -3, -10, 10), ("Barricade_Fence", 8, 23, -6), ("Tent_Large_Civilian", 8, -17, 10), ("Kennel", -14, 12, 30), ("Latrine", -20, -18, 130)]),
    ("SURVIVORS", "OUTPOST", "A farmyard: the table and the memorial, the cook pot and the water, the washing and the garden at the back, a tent for whoever came last.", [
        ("Counter_Haven", 5, 9, -10), ("BearDoor", 0, 20, 0), ("Table", 2, 2, 6), ("CookPot", 9, -3, 40), ("WaterBarrel", 5, -9, 0),
        ("WashStand", 2, -11, 10), ("WashingLine", -8, -13, 12), ("Garden", -13, -4, 80), ("Memorial", -6, 5, 20), ("Firewood", 12.5, -10.5, 70),
        ("Barricade_Fence", -7, 19, 8), ("Tent_Small_Civilian", 14.5, -17.5, -25), ("ChildrensCorner", -7.5, 0.5, 35)]),
    ("SURVIVORS", "SAFEHOUSE", "A house that kept its door open: the bear at the gate, the table outside, the pot and the barrel by the kitchen, the line and the bed of vegetables in the garden.", [
        ("Counter_Haven", -4, 6, 10), ("BearDoor", 0, 14, 0), ("Table", 2, 2, 6), ("CookPot", 8, -2, 40), ("WaterBarrel", 5, -8, 0),
        ("WashingLine", -7, -9, 12), ("Garden", -11, -2, 80), ("Firewood", 11.5, -9.5, 70)]),
    ("INSTITUTE", "STRONGHOLD", "The lab grounds: tidy and compact, everything within reach of the generators; the tables in front, the ward tent and the decon shower behind, the plague board where the track comes in.", [
        ("Counter_Halcyon", -10, 12, 10), ("PlagueBoard", 0, 22, 0), ("FieldTable", -2, 4, 3), ("Tent_Halcyon", 0, -11, 0), ("FieldTable", 3.5, 3, -8),
        ("DeconShower", -8, -9, 10), ("Generator", 14, -10, 20), ("ColdBox", 10, -1, -30), ("SampleTable", 7, -7, -15), ("Antenna", 16, 3, 0),
        ("Barricade_Fence", -7, 21, 3), ("Barricade_Fence", 7, 21, -3), ("NotebookBench", -7, 7, 15), ("RadioTable", 10, 8, -20), ("Tarp", -12, -1, 35),
        ("Latrine", -18, -16, 120), ("CrateChairs", -3, -2, 0), ("DryingLine", -16, -9, 70), ("Generator", 17, -14, 10)]),
    ("INSTITUTE", "OUTPOST", "A field station at a house: a table for the notes, the ward tent and the decon shower in the yard, the cold store on the generator, the antenna, the plague board at the gate.", [
        ("Counter_Halcyon", -6, 8, 10), ("PlagueBoard", 0, 16, 0), ("FieldTable", -2, 3, 3), ("Tent_Halcyon", 1, -9, 0), ("Generator", 10, -6, 20),
        ("Antenna", 11, 2, 0), ("DeconShower", -7, -6, 10), ("ColdBox", 5, 0.5, -30), ("NotebookBench", -7, 2, 15)]),
]

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
    'm_sLastMeal': ('RZ_Trace_LastMeal', [P('PicnicTable_01.et', 0, 0), P('Chair_01_white.et', -0.5, 1.1, 180), P('Chair_01_white.et', 0.5, 1.1, 180), P('Chair_01_white.et', -0.5, -1.1), P('Chair_01_white.et', 0.6, -1.2, 30), P('LitterInterior_USSR_01_bowl.et', -0.3, 0.2, 0, 0.78), P('LitterInterior_USSR_01_bowl.et', 0.3, -0.2, 0, 0.78), P('LitterInterior_USSR_01_mug.et', 0, 0, 0, 0.78), P('RadioSmall_01_black.et', 0.8, 0, 0, 0.78)]),
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
    'group.2': ('RZ_Opening_Group_Table', [P('TableOld_01.et', 0, 0, 8), P('Notebook_01_Stack.et', -0.2, 0.05, 30, 0.78), P('LampKerosene_01_green.et', 0.4, -0.1, 0, 0.78), P('ChairOld_01.et', -0.8, 0.1, 90), P('ChairOld_02.et', 0.8, 0.2, -90), P('Chair_01_white.et', 0.1, 0.8, 180), P('Stool_01.et', 0, -0.8)]),
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
    'labtech': ('RZ_Opening_Props_LabTech', [P('TableOld_01_white.et', 0, 0, 5), P('Notebook_01_Stack.et', -0.3, 0, 20, 0.78), P('ToolsSurgical_01_autoclavedrum.et', 0.35, 0.1, 0, 0.78), P('Chair_01_white.et', 0, -0.8)]),
    'radioham': ('RZ_Opening_Props_Radio', [P('CrateWooden_01_plain.et', 0, 0, 10), P('RadioSmall_01_black.et', 0, 0, 30, 0.62), P('Antenna_R161_01.et', -1.0, 0.4), P('GeneratorPortable_US_01_Off.et', 1.1, 0.2, 75)]),
    'unemployed': ('RZ_Opening_Props_Unemployed', [P('BlanketBeach_01_brown.et', 0, 0, 80), P('Suitcase_04_open.et', 1.3, 0.3, 20), P('LitterInterior_USSR_01_tincan_V2.et', -0.6, 0.6), P('LitterInterior_USSR_01_mug.et', -0.4, 0.8)]),
    'soldier': ('RZ_Opening_Props_Soldier', [P('Sandbag_01_single_burlap.et', 0, 0, 10), P('Sandbag_01_single_burlap.et', 0.1, 0, 30, 0.17), P('CrateWooden_02_1x1x1.et', 1.2, 0.4, 15), P('LanternMilitary_US_01.et', 1.2, 0.4, 0, 1.0)]),
    'raider': ('RZ_Opening_Props_Raider', [P('SackUniversal_01_Pile.et', 0, 0, 20), P('Suitcase_01.et', 1.2, 0.2, 70, 0, 90), P('BeerBottle_01_group.et', -0.9, 0.3), P('BarrelMetal_01_rust.et', -1.2, -0.6)]),
    'elder': ('RZ_Opening_Props_Elder', [P('Armchair_01.et', 0, 0, 170), P('Stool_01.et', 0.9, 0.2), P('TablePhotoFrame_01.et', 0.9, 0.2, 200, 0.45), P('CandleHolderDecorative_01.et', 0.75, 0.3, 0, 0.45), P('BlanketFolded_01_green.et', 0, 0, 0, 0.45)]),
    'drifter': ('RZ_Opening_Props_Drifter', [P('Suitcase_01.et', 0, 0, 30), P('SackUniversal_01.et', 0.8, 0.3, 60), P('BlanketBeach_01_red.et', -0.2, 1.0, 85)]),
}
