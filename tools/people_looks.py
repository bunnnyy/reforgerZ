#!/usr/bin/env python3
"""ReforgerZ people's looks: writes the faction people's variant prefabs.

    py tools/people_looks.py            # write the prefabs (and their .meta) under Prefabs/ReforgerZ/People/
    py tools/people_looks.py --arrays   # also print the look arrays to paste into RZ_GameMode.et
    py tools/people_looks.py --check [--game <game folder> --mods <Workshop mods folder>]
                                        # fail if any faction person can spawn in anything but a look

Every faction person slot (a guard, an occupant, a quartermaster, a crew, a hunter squad, a council
head, an outpost or stronghold leader) can list variant prefabs; a body picks one by a stable seed
(scripts/Game/ReforgerZ/Factions/RZ_Looks.c, docs/DESIGN.md "People's looks"). This tool writes those
variants as text prefabs:

- a character look inherits the slot's own character prefab (or, for a group slot, the vanilla soldier
  its group spawns, for his weapon) and overrides clothing slots only (Hat, Face, Head, Jacket, Pants,
  Boots, Vest, ArmoredVest, Back, HandWear). It must set every garment you can see (DRESSED), so none
  of the parent's own kit shows through. Face and Head are Zelik's Character slots, which BanditGear,
  BasicHazmatSuit, MedicalAccessories and Domins bring with them.
- when a look changes a garment the parent's starting items are packed into, those item lists are
  pointed at the new garment (TargetStorage), or at another slot when the garment is gone (a suit
  that covers the legs blocks the trousers).
- a group look inherits the slot's group and lists other character looks as its members.
- the faction group prefabs themselves (BASES: the slots' own groups and the missions' faction groups)
  get looks as their members, so a group spawned without a look is dressed too.
- a look built on a parent with editable-entity variants switches them off (VARIANTS): a vanilla AI
  group spawns each member as a random variant of its prefab, so without it a look came out as the
  vanilla soldier it's built on half the time.
- a look dropped from the tables is deleted with its .meta.

--check writes nothing. It fails if the prefabs on disk aren't what the tables write; if a faction group
(the dressed groups, the group looks, the slots' groups) lists anything but character looks, its first
(leader) slot included; if a look leaves a random variant on; if RZ_GameMode.et's Looks lists aren't the
tables' looks, aren't registered in RegisterLooks, or sit next to a slot prefab other than the look's
parent (PickLook keys on it); or if a script spawns a faction person slot (a field RegisterLooks gives
looks, a function returning one, or a local set from one) without PickLook. With --game and --mods (or
RZ_GAME_DIR and RZ_MODS_DIR) it also reads the game's and the Workshop dependencies' .rdb and .pak files:
every garment and parent ResourceName must resolve by its GUID, every slot a look sets must be on its
parent (Zelik's Face and Head included), every item list packed into a garment a look changes must move
with it, and VARIANTS must be the parents' real variants.

Faces aren't set: the vanilla identity picks one from the faction's identity config at each spawn.

GUIDs: a new prefab's GUID is derived from its path (stable across runs). Every clothing ResourceName
below is checked against the Workbench resource index ($profile:RZ_ResourceIndex.txt, written by the
plugin "Write resource index"); a name missing from it stops the tool. Set RZ_RESOURCE_INDEX to point
elsewhere, or RZ_RESOURCE_INDEX=none to skip the check. No dependencies beyond Python 3.
"""
import hashlib
import os
import re
import struct
import sys
import zlib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = "Prefabs/ReforgerZ/People"
DEFAULT_INDEX = os.path.join(os.path.expanduser("~"), "OneDrive", "Documents", "My Games",
                             "ArmaReforgerWorkbench", "profile", "RZ_ResourceIndex.txt")

# ------------------------------------------------------------------------------------------ clothing
# Short name -> ResourceName. Vanilla unless marked: [L] Lamas vanilla plus gear pack, [B] BanditGear,
# [H] BasicHazmatSuit, [M] MedicalAccessories, [C] Clothing_pack_v2 (no hats: wrong GUIDs), [D] Domins.
I = {
    # hats
    "flatcap_blue": "{DD565E2C7E00A5BB}Prefabs/Characters/HeadGear/Hat_FlatCap_01/Hat_FlatCap_01_blue.et",
    "flatcap_darkgrey": "{67662815EFF734DD}Prefabs/Characters/HeadGear/Hat_FlatCap_01/Hat_FlatCap_01_darkgrey.et",
    "flatcap_lightgrey": "{6FEB0D9130EC400E}Prefabs/Characters/HeadGear/Hat_FlatCap_01/Hat_FlatCap_01_lightgrey.et",
    "knit_black": "{BE19A15F1073E7BD}Prefabs/Characters/HeadGear/Hat_Knitted_01/Hat_Knitted_01_black.et",
    "knit_blue": "{6B2256EBB8E267DD}Prefabs/Characters/HeadGear/Hat_Knitted_01/Hat_Knitted_01_blue.et",
    "knit_brown": "{B9687B8829272E14}Prefabs/Characters/HeadGear/Hat_Knitted_01/Hat_Knitted_01_brown.et",
    "knit_red": "{D8D9EE7F3966589E}Prefabs/Characters/HeadGear/Hat_Knitted_01/Hat_Knitted_01_red.et",
    "trucker_everon": "{6D08415FD18F2775}Prefabs/Characters/HeadGear/Hat_TruckerCap_01/Hat_TruckerCap_01_everon.et",
    "trucker_pohona": "{477D52F01DC45C55}Prefabs/Characters/HeadGear/Hat_TruckerCap_01/Hat_TruckerCap_01_pohona.et",
    "trucker_sawmill": "{554D761A11D3D7EC}Prefabs/Characters/HeadGear/Hat_TruckerCap_01/Hat_TruckerCap_01_sawmillregina.et",
    "hardhat_yellow": "{8F5261C6B57B95C8}Prefabs/Characters/HeadGear/Hat_Hard_01/Hat_Hard_01_yellow.et",
    "hardhat_white": "{DF26044449C06EAB}Prefabs/Characters/HeadGear/Hat_Hard_01/Hat_Hard_01_white.et",
    "ushanka_folded": "{A22C976003AA5BA3}Prefabs/Characters/HeadGear/Hat_Ushanka_01/Hat_Ushanka_01_folded.et",
    "beret_brown": "{6E0EC7AC1F6D748F}Prefabs/Characters/HeadGear/Hat_RadiovkaBeret_01/Hat_RadiovkaBeret_01_brown.et",
    "beret_black_dirty": "{5D7350496AEA24C8}Prefabs/Characters/HeadGear/Hat_RadiovkaBeret_01/Hat_RadiovkaBeret_01_black_Dirty.et",
    "boonie_olive": "{8D75122D5BF561FB}Prefabs/Characters/HeadGear/Hat_Boonie_US_01/Hat_Boonie_US_olive.et",
    "pasgt_goggles": "{79D240C0D2390F4A}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_01_goggles.et",
    "m1_nostrap": "{1D0B60CC90DA7330}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_No Strap.et",  # [L]
    "cap_plain_back": "{F42EB93651D94D60}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_02_backwards.et",  # [B]
    "cap_trophy": "{B5A0E12587A00B68}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_03.et",  # [B]
    "pasgt_black": "{E503F84CD6E31B21}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_Black.et",  # [B]
    # face (Zelik's Face slot) and head (Zelik's Head slot)
    "balaclava_black": "{C500301CF7AE1ACB}Prefabs/Characters/HeadGear/Balaclava/Balaclava_Wool_Black.et",  # [B]
    "balaclava_olive": "{09843B703114507C}Prefabs/Characters/HeadGear/Balaclava/Balaclava_Wool_Olive.et",  # [B]
    "balaclava3_black": "{907106BB556066E9}Prefabs/Characters/HeadGear/Balaclava/Balaclava_Three-Hole_Black.et",  # [B]
    "balaclava3_olive": "{F3DF25E70C82E9CA}Prefabs/Characters/HeadGear/Balaclava/Balaclava_Three-Hole_Olive.et",  # [B]
    "hazmask": "{85C22207DAACE451}Prefabs/Characters/HeadGear/Mask_Hazmat/Mask_Hazmat_Gas_01.et",  # [H]
    "hazmask_tinted": "{E99616E766686B7F}Prefabs/Characters/HeadGear/Mask_Hazmat/Mask_Hazmat_Gas_02.et",  # [H]
    "medmask": "{C07A52D13353AB2F}Prefabs/Clothing/ZEL_FaceClothing_Base.et",  # [M] the medical mask
    "gasmask": "{FB66C1EC2E5DB34C}Prefabs/Clothing/Gas_Mask.et",  # [D] Head slot
    # jackets, shirts, suits
    "denim_brown": "{27162F1F6EE6581A}Prefabs/Characters/Uniforms/Jacket_Denim_01/Jacket_Denim_01_brownShirt.et",
    "raincoat_green": "{7F1451790F3D75BD}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_green.et",
    "raincoat_orange": "{690819A92A3A8D14}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_orange.et",
    "raincoat_yellow": "{FD39B0B833BB927E}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_yellow.et",
    "suit_grey": "{E652D625A46A00FC}Prefabs/Characters/Uniforms/Jacket_Suit_01/Jacket_Suit_01_Grey_V1.et",
    "suit_brown": "{52AB6EC3FE535303}Prefabs/Characters/Uniforms/Jacket_Suit_01/Jacket_Suit_01_Brown_V1.et",
    "suit_black": "{AE93D885FDF713D4}Prefabs/Characters/Uniforms/Jacket_Suit_01/Jacket_Suit_01_black_V1.et",
    "turtle_grey": "{B0F4EAC29E9058EA}Prefabs/Characters/Uniforms/Shirt_Turtleneck_01/Shirt_Turtleneck_01_grey.et",
    "turtle_beige": "{50F8B1F928F93ABD}Prefabs/Characters/Uniforms/Shirt_Turtleneck_01/Shirt_Turtleneck_01_beige.et",
    "bdu_rolled_02": "{A080C1AEBF921DC4}Prefabs/Characters/Uniforms/Jacket_US_BDU_rolledup_02.et",
    "us_raincoat": "{7B8BBB818BD62015}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/US RAINCOAT.et",  # [L]
    "bandit_1": "{DB053443800A3FCB}Prefabs/Characters/Uniforms/Jacket_Bandit/Jacket_Bandit_01.et",  # [B]
    "bandit_2": "{5FA71197901F8EEF}Prefabs/Characters/Uniforms/Jacket_Bandit/Jacket_Bandit_02.et",  # [B]
    "track_black": "{7CC37DEB059F14CE}Prefabs/Characters/Uniforms/Jacket_Tracksuit/Jacket_Tracksuit_Black.et",  # [B]
    "track_navy": "{3A948BD491BC06DE}Prefabs/Characters/Uniforms/Jacket_Tracksuit/Jacket_Tracksuit_Navy.et",  # [B]
    "sweat_dirty": "{A6E6212E02313CAD}Prefabs/Characters/Uniforms/Sweatshirt/Sweatshirt_Dirty.et",  # [B]
    "sweat_red": "{3DA87A330B897C58}Prefabs/Characters/Uniforms/Sweatshirt/Sweatshirt_Red.et",  # [B]
    "hazmat_olive": "{13902C1038AEBBAE}Prefabs/Characters/Uniforms/Suit_Hazmat/Suit_Hazmat_v2_Olive.et",  # [H] blocks Pants
    "hazmat_black": "{703E0F4C614C348D}Prefabs/Characters/Uniforms/Suit_Hazmat/Suit_Hazmat_v2_Black.et",  # [H] blocks Pants
    "hazmat_hivis": "{27CD85E26D10930F}Prefabs/Characters/Uniforms/Suit_Hazmat/Suit_Hazmat_v2_HiVis.et",  # [H] blocks Pants
    "overalls": "{50C997103F92E752}Prefabs/Characters/Uniforms/Overalls/Overalls1.et",  # [C] blocks Pants
    "overalls_denim": "{C717AE6CBE1C162E}Prefabs/Characters/Uniforms/Overalls/Denim_overalls2.et",  # [C] blocks Pants
    "mechanic": "{DFE230A386D20BFB}Prefabs/Characters/Uniforms/Overalls/Mechanic_uniform.et",  # [C] blocks Pants
    "shirt_1": "{CD87BF4D0F940AB3}Prefabs/Characters/Uniforms/Shirt/Shirt.et",  # [C]
    "shirt_2": "{2085CF5A65EFCD11}Prefabs/Characters/Uniforms/Shirt/Shirt2.et",  # [C]
    "shirt_3": "{C51983A3690A5A7A}Prefabs/Characters/Uniforms/Shirt/Shirt3.et",  # [C]
    "ems_jacket": "{C1FF82A0F6813A48}Prefabs/Characters/Uniforms/Jacket_EMS.et",  # [D]
    # trousers
    "trousers_lightbrown": "{65B7093231FBF67E}Prefabs/Characters/Uniforms/Pants_Trousers_01/Pants_Trousers_01_LightBrown.et",
    "trousers_darkbrown": "{6925E6C63EBCF623}Prefabs/Characters/Uniforms/Pants_Trousers_01/Pants_Trousers_01_DarkBrown.et",
    "trousers_darkgrey": "{5A8C9A020BD7ABA6}Prefabs/Characters/Uniforms/Pants_Trousers_01/Pants_Trousers_01_DarkGrey.et",
    "fisher_green": "{0B1F0174869E0351}Prefabs/Characters/Uniforms/Pants_Fisherman_01/Pants_Fisherman_01_green.et",
    "fisher_yellow": "{CF185061C5C13DBF}Prefabs/Characters/Uniforms/Pants_Fisherman_01/Pants_Fisherman_01_yellow.et",
    "suitpants_grey": "{401072A652DD9849}Prefabs/Characters/Uniforms/Pants_Suit_01/Pants_Suit_01_grey_V1.et",
    "suitpants_black": "{1A65EADE4B08583E}Prefabs/Characters/Uniforms/Pants_Suit_01/Pants_Suit_01_black_V1.et",
    "suitpants_brown": "{E65D5C9848AC18E9}Prefabs/Characters/Uniforms/Pants_Suit_01/Pants_Suit_01_brown_V1.et",
    "m65": "{86BB79B9A2233780}Prefabs/Characters/Uniforms/Pants_US_M65.et",
    "cargo_black": "{21DC09217E13C1C7}Prefabs/Characters/Uniforms/Pants_Cargo/Pants_Cargo_Black.et",  # [B]
    "cargo_tan": "{C542EA4B3ED3B61B}Prefabs/Characters/Uniforms/Pants_Cargo/Pants_Cargo_Tan.et",  # [B]
    "jeans_1": "{C40CEB198DAABF5B}Prefabs/Characters/Uniforms/Pants_Jeans/Pants_Jeans_01.et",  # [B]
    "jeans_2": "{A858DFF9316E3075}Prefabs/Characters/Uniforms/Pants_Jeans/Pants_Jeans_02.et",  # [B]
    "trackpants_black": "{AC7F9BCA3DA1002D}Prefabs/Characters/Uniforms/Pants_Tracksuit/Pants_Tracksuit_Black.et",  # [B]
    "trackpants_navy": "{821386465B3C63AB}Prefabs/Characters/Uniforms/Pants_Tracksuit/Pants_Tracksuit_Navy.et",  # [B]
    "ems_pants": "{B8F5E8DB8D633E31}Prefabs/Characters/Uniforms/Pants_EMS.et",  # [D]
    # boots
    "boots_soviet_dirty": "{4C6029AB8BF5C044}Prefabs/Characters/Footwear/CombatBoots_Soviet_01_Dirty.et",
    "sneakers_dirty": "{CABF701DF5212647}Prefabs/Characters/Footwear/Sneakers/Sneakers_Dirty.et",  # [B]
    "sneakers_black": "{553F83578D29670C}Prefabs/Characters/Footwear/Sneakers/Sneakers_Black.et",  # [B]
    "boots_paramedic": "{A38C67B3E56B7D26}Prefabs/Characters/Footwear/paramedic_boots.et",  # [D]
    # gloves
    "gloves_leather": "{4F71E7468E78DDD2}Prefabs/Characters/Handwear/Gloves_Leather_01/Gloves_Leather_01.et",
    "gloves_wool": "{8266820FFDE17477}Prefabs/Characters/Handwear/Gloves_Wool_01/Gloves_Wool_01.et",
    "gloves_pilot": "{D19C474FA0F42CC4}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_US_01.et",
    "gloves_wool_dirty": "{7277FBDCDC013DDF}Prefabs/Characters/Handwear/Gloves_Wool_01/Gloves_Wool_Dirty.et",  # [B]
    "gloves_wool_black": "{3F155F317D83D375}Prefabs/Characters/Handwear/Gloves_Wool_01/Gloves_Wool_Black.et",  # [B]
    "gloves_skeleton": "{7A32D0C6743CCEBE}Prefabs/Characters/Handwear/Gloves_Wool_Skeleton/Gloves_Wool_Skeleton.et",  # [B]
    "gloves_leather_black": "{CDDA2A133111E1A6}Prefabs/Characters/Handwear/Gloves_Leather_01/Gloves_Leather_Black.et",  # [B]
    "gloves_pilot_black": "{EF604A05A5998102}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_US_Black.et",  # [H]
    "nitrile_blue": "{D8326FEE52670D2A}Prefabs/Characters/Handwear/Gloves_Blue/Medical_Gloves_Blue_Base.et",  # [M]
    "nitrile_purple": "{395D1185E649EDE2}Prefabs/Characters/Handwear/Gloves_Purple/Medical_Gloves_Purple_Base.et",  # [M]
    "nitrile_white": "{D90A181625AD1F9A}Prefabs/Characters/Handwear/Gloves_White/Medical_Gloves_White_Base.et",  # [M]
    "nitrile_yellow": "{4B4ED1DF956F00D9}Prefabs/Characters/Handwear/Gloves_yellow/Medical_Gloves_yellow_Base.et",  # [M]
    # Bastion's olive kit (vanilla): the M65 field jacket (sleeves down, or rolled: bdu_rolled_02),
    # M65 trousers, US boots, the PASGT flak vest, plain helmets and olive caps
    "m65_jacket": "{293F577C298061E3}Prefabs/Characters/Uniforms/Jacket_US_BDU_02.et",
    "pasgt": "{B74A4FF0DD8BB116}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_01.et",
    "patrol_olive": "{9911F55EC05D1A64}Prefabs/Characters/HeadGear/Hat_Patrol_US_01/Hat_Patrol_US_02.et",
    "boots_us": "{DAAFD15478BDE1C3}Prefabs/Characters/Footwear/CombatBoots_US_01.et",
    "pasgt_vest": "{4B57C11AA5161760}Prefabs/Characters/Vests/Vest_PASGT/Vest_PASGT.et",
    # the Red Hand's red (vanilla)
    "zcap_red": "{1B568A254D1E519B}Prefabs/Characters/HeadGear/Hat_ZmijovkaCap_01/Hat_ZmijovkaCap_01_red.et",
    # packs
    "veshmeshok": "{3DE0155EC9767B98}Prefabs/Items/Equipment/Backpacks/Backpack_Veshmeshok.et",
    "": "",
}

# Garments that cover the legs: the look's Pants slot is emptied with them.
BLOCKS_PANTS = {"hazmat_olive", "hazmat_black", "hazmat_hivis", "overalls", "overalls_denim", "mechanic"}

# Slot -> the class name of its entry in BaseLoadoutManagerComponent.Slots (vanilla Character_Base;
# Face and Head come from Zelik's Character).
SLOT_CLASS = {"Boots": "SCR_FeetLoadoutSlotInfo"}
SLOT_ORDER = ["Hat", "Face", "Head", "Jacket", "ArmoredVest", "Pants", "Boots", "Back", "Vest", "HandWear"]

# ------------------------------------------------------------------------------------------ parents
# Each parent: its ResourceName, its entity ID line, and the item lists (ItemsInitConfigurationItem
# ids) it packs into each garment slot after inheritance (read from the vanilla prefabs). Lists the
# parent empties are left out: nothing to move.
PARENTS = {
    # occupants: every starting list emptied, nothing to move
    "occ_haven": ("{6A06892169C48E4F}Prefabs/ReforgerZ/Territory/RZ_Occupant_Haven.et", "520EC961A090B1EE", {}),
    "occ_bastion": ("{9ECF9635E354CC50}Prefabs/ReforgerZ/Territory/RZ_Occupant_Bastion.et", "52092032B0E62200", {}),
    "occ_redhand": ("{EB21A65C09239EBE}Prefabs/ReforgerZ/Territory/RZ_Occupant_RedHand.et", "520EC961A090B1EE", {}),
    "occ_halcyon": ("{A3AF19F66C7846C1}Prefabs/ReforgerZ/Territory/RZ_Occupant_Halcyon.et", "520EC961A090B1EE", {}),
    # the soldiers the guard groups spawn
    "fia_ac": ("{9503CB9B3463BA1E}Prefabs/Characters/Factions/INDFOR/FIA/Character_FIA_AC.et", "520EC961A090B1EE",
               {"Pants": ["{5D7CC173FAA7C00E}", "{5D7CC173FAA7C00C}"], "Jacket": ["{608AC3CC37F93DCA}"],
                "Back": ["{7A13C0A0F1A85220}"]}),
    "fia_partisan": ("{254F12535581E4C2}Prefabs/Characters/Factions/INDFOR/FIA/Special Units/Character_FIA_AC_Partisan.et",
                     "520EC961A090B1EE", {"Pants": ["{5D7CC173FAA7C00E}", "{5D7CC173FAA7C00C}"]}),
    "us_rifleman": ("{26A9756790131354}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman.et", "52092032B0E62200",
                    {"Pants": ["{5D7CAAFB9235E902}"]}),
    "us_rifleman_v1": ("{EA158B6EB6A24B4B}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman_Variant_1.et",
                       "52092032B0E62200", {"Pants": ["{5D7CAAFB9235E902}"]}),
    # quartermasters
    "qm_haven": ("{49D699E428FAE3A3}Prefabs/ReforgerZ/Factions/RZ_QM_Haven.et", "520EC961A090B1EE",
                 {"Pants": ["{60DCBCECAC6C6E65}"]}),
    "qm_bastion": ("{FA6C654C92ECF556}Prefabs/ReforgerZ/Factions/RZ_QM_Bastion.et", "520EC961A090BBD5",
                   {"Pants": ["{5D7CAAFB9235E902}", "{7A13C0A0F1A85210}"], "Jacket": ["{5D7CAAFB9235E903}"]}),
    "qm_redhand": ("{8F825525789BA7B8}Prefabs/ReforgerZ/Factions/RZ_QM_RedHand.et", "520EC961A090B1EE",
                   {"Back": ["{7A13C0A0F1A85008}", "{5D7CC173FAA7C00E}"], "Jacket": ["{5D7CC173FAA7C00C}"]}),
    "qm_halcyon": ("{C70CEA8F1DC07FC7}Prefabs/ReforgerZ/Factions/RZ_QM_Halcyon.et", "520EC961A090B1EE",
                   {"Pants": ["{5D7CC173FAA7C00E}"], "Jacket": ["{5D7CC173FAA7C00C}"],
                    "Back": ["{7A13C0A0F1A85008}", "{5DF8B4EAF98C1278}"]}),
    "us_tl": ("{E398E44759DA1A43}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_TL.et", "51FA90A5D40EA513",
              {"Pants": ["{5D7CAAFB9235E902}"]}),
    "fia_medic": ("{4E29194BA809DF32}Prefabs/Characters/Factions/INDFOR/FIA/Character_FIA_AC_Medic.et", "520EC961A090B1EE",
                  {"Pants": ["{5D7CC173FAA7C00E}"], "Jacket": ["{5D7CC173FAA7C00C}"],
                   "Back": ["{7A13C0A0F1A85008}", "{5DF8B4EAF98C1278}"]}),
    # the survivor group's two civilians
    "civ_turtleneck": ("{1FFE2B88BEF51840}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_Turtleneck_2.et",
                       "520EC961A090B1EE", {"Pants": ["{60DCBCECAC6C6E65}"]}),
    "civ_cottonshirt": ("{408B8BD5E3F09FF3}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_3.et",
                        "520EC961A090B1EE", {"Pants": ["{60DCBCECAC6C6E65}"]}),
    # the stronghold leaders
    "leader_haven": ("{D5F88991398C10EA}Prefabs/ReforgerZ/Leaders/RZ_Leader_Haven.et", "520EC961A090B1EE",
                     {"Pants": ["{60DCBCECAC6C6E65}"]}),
    "leader_bastion": ("{6592F081196646EB}Prefabs/ReforgerZ/Leaders/RZ_Leader_Bastion.et", "520EC961A090B1EE",
                       {"Pants": ["{5D7CAAFB9235E902}"]}),
    "leader_redhand": ("{107CC0E8F3111405}Prefabs/ReforgerZ/Leaders/RZ_Leader_RedHand.et", "520EC961A090B1EE",
                       {"Pants": ["{5D7CC173FAA7C00C}"], "Jacket": ["{5F28BB79AC280275}"],
                        "Back": ["{5D7CC173FAA7C00E}", "{7A13C0A0F1A85008}"]}),
    "leader_halcyon": ("{58F27F42964ACC7A}Prefabs/ReforgerZ/Leaders/RZ_Leader_Halcyon.et", "520EC961A090B1EE",
                       {"Pants": ["{5D7CC173FAA7C00E}"], "Jacket": ["{5D7CC173FAA7C00C}"],
                        "Back": ["{7A13C0A0F1A85008}", "{5DF8B4EAF98C1278}"]}),
    # leaders who stand at more than one place
    "council":("{70CB458FA07F01CC}Prefabs/ReforgerZ/Leaders/RZ_CouncilHead.et", "520EC961A090B1EE",
                {"Pants": ["{60DCBCECAC6C6E65}"]}),
    "outpost_bastion": ("{AF96D259B26C1DA4}Prefabs/ReforgerZ/Leaders/RZ_Outpost_Bastion.et", "520EC961A090B1EE",
                        {"Pants": ["{5D7CAAFB9235E902}"]}),
    "outpost_redhand": ("{DA78E230581B4F4A}Prefabs/ReforgerZ/Leaders/RZ_Outpost_RedHand.et", "520EC961A090B1EE",
                        {"Pants": ["{5D7CC173FAA7C00E}", "{5D7CC173FAA7C00C}"], "Jacket": ["{608AC3CC37F93DCA}"],
                         "Back": ["{7A13C0A0F1A85220}"]}),
    "outpost_halcyon": ("{92F65D9A3D409735}Prefabs/ReforgerZ/Leaders/RZ_Outpost_Halcyon.et", "520EC961A090B1EE",
                        {"Pants": ["{5D7CC173FAA7C00E}"], "Jacket": ["{5D7CC173FAA7C00C}"],
                         "Back": ["{7A13C0A0F1A85008}", "{5DF8B4EAF98C1278}"]}),
    # groups
    "grp_guard_haven": ("{AD57D562AAA1047D}Prefabs/ReforgerZ/Territory/RZ_Guard_Haven.et", "0000000000000001", None),
    "grp_guard_bastion": ("{5E86B5DA97C6C436}Prefabs/ReforgerZ/Territory/RZ_Guard_Bastion.et", "0000000000000001", None),
    "grp_guard_redhand": ("{2B6885B37DB196D8}Prefabs/ReforgerZ/Territory/RZ_Guard_RedHand.et", "0000000000000001", None),
    "grp_guard_halcyon": ("{63E63A1918EA4EA7}Prefabs/ReforgerZ/Territory/RZ_Guard_Halcyon.et", "0000000000000001", None),
    "grp_safehouse_guard": ("{89F63D74B915A25E}Prefabs/ReforgerZ/Haven/RZ_SafehouseGuard.et", "0000000000000001", None),
    "grp_post_bastion": ("{C02A0C68FB9A45BD}Prefabs/ReforgerZ/Factions/RZ_BastionPostCrew.et", "0000000000000001", None),
    "grp_post_redhand": ("{3790F412BF585C32}Prefabs/ReforgerZ/Factions/RZ_RedHandPostCrew.et", "0000000000000001", None),
    "grp_hunters_bastion": ("{291AFE2B1958D55B}Prefabs/ReforgerZ/Factions/RZ_BastionHunters.et", "0000000000000001", None),
    "grp_hunters_redhand": ("{11D56596E9B1B015}Prefabs/ReforgerZ/Factions/RZ_RedHandHunters.et", "0000000000000001", None),
    "grp_truck_haven": ("{737D90BDBB9E563D}Prefabs/ReforgerZ/Supply/RZ_TruckCrew_Haven.et", "0000000000000001", None),
    "grp_truck_bastion": ("{995C76BB993F5840}Prefabs/ReforgerZ/Supply/RZ_TruckCrew_Bastion.et", "0000000000000001", None),
    "grp_truck_redhand": ("{ECB246D273480AAE}Prefabs/ReforgerZ/Supply/RZ_TruckCrew_RedHand.et", "0000000000000001", None),
    "grp_truck_halcyon": ("{A43CF9781613D2D1}Prefabs/ReforgerZ/Supply/RZ_TruckCrew_Halcyon.et", "0000000000000001", None),
}

# The editable-entity variants a parent inherits (SCR_EditableCharacterComponent's m_VariantData):
# parent key -> (component line, variant data line, [variant ids]). A vanilla AI group spawns each
# member with SpawnEntityPrefabEx(prefab, true): it swaps the prefab for one of its enabled variants
# at random, the prefab itself counting as one. The US rifleman and his variant 1 list each other, so
# a look built on either came out of a group as the plain US rifleman half the time (the play-test's
# Halcyon pair: one in hazmat, one in woodland). Every look switches its parent's variants off.
# Read from the game by --check, which fails if this table falls behind.
EDITABLE = 'SCR_EditableCharacterComponent "{520EA1D2F659C85C}"'
US_VARIANTS = (EDITABLE, 'SCR_EditableEntityVariantData "{60F6B2B4BAE04806}"', ["{60F6B2B4B9D3CC9B}"])
VARIANTS = {
    "us_rifleman": US_VARIANTS,
    "us_rifleman_v1": US_VARIANTS,
    "occ_bastion": US_VARIANTS,
}

# ------------------------------------------------------------------------------------------ the looks
# (folder, file stem, parent, {slot: item}, {slot whose items move: to slot}). "" empties a slot.
# Every look sets every garment you can see (DRESSED below), so nothing of the parent's own kit (a US
# soldier's woodland helmet, flak vest or trousers) shows through; the tool stops if one is missing.
# Each faction dresses as one people, and its armed people as a unit: one base, details that vary.
#   The Haven: civilians in work clothes. Its watch (guards, safehouse guards) in green oilskins and
#     fisherman's trousers; its drivers in overalls and work clothes; nobody in red.
#   Task Force Bastion: one olive kit, the M65 jacket (sleeves down or rolled) and trousers, US
#     boots; on duty the PASGT vest and a plain helmet or olive cap; off duty no webbing.
#   The Red Hand: scavenged and mismatched, dark trousers, and every armed one wears red (a red
#     knitted hat or cap, or a red sweatshirt).
#   Halcyon: its security in black hooded hazmat suits, masks and black gloves; its field crews in
#     olive hazmat; its staff in suits, turtlenecks, paramedic kit and the hazmat with masks and
#     nitrile gloves. Nobody of Halcyon's in US Army kit.
DRESSED = ["Hat", "Jacket", "Pants", "Boots", "ArmoredVest", "HandWear"]

CHARS = [
    # ---- The Haven: occupants (also the safehouses' occupants), quartermasters, council heads, leader
    ("Haven", "RZ_Occupant_Haven_Farmer", "occ_haven", {"Hat": "knit_brown", "Jacket": "overalls", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather"}, {}),
    ("Haven", "RZ_Occupant_Haven_Dungarees", "occ_haven", {"Hat": "trucker_sawmill", "Jacket": "overalls_denim", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_Occupant_Haven_Mechanic", "occ_haven", {"Hat": "", "Jacket": "mechanic", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_wool"}, {}),
    ("Haven", "RZ_Occupant_Haven_Fisher", "occ_haven", {"Hat": "knit_blue", "Jacket": "raincoat_yellow", "Pants": "fisher_green", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_Occupant_Haven_Builder", "occ_haven", {"Hat": "hardhat_yellow", "Jacket": "shirt_2", "Pants": "trousers_lightbrown", "Boots": "boots_soviet_dirty", "Back": "veshmeshok", "ArmoredVest": "", "HandWear": "gloves_leather"}, {}),
    ("Haven", "RZ_Occupant_Haven_Carer", "occ_haven", {"Hat": "", "Face": "medmask", "Jacket": "turtle_grey", "Pants": "trousers_darkbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Haven", "RZ_QM_Haven_Storekeeper", "qm_haven", {"Hat": "flatcap_lightgrey", "Jacket": "shirt_1", "Pants": "trousers_lightbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_QM_Haven_Dungarees", "qm_haven", {"Hat": "knit_brown", "Jacket": "overalls_denim", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather"}, {"Pants": "Jacket"}),
    ("Haven", "RZ_QM_Haven_Raincoat", "qm_haven", {"Hat": "trucker_pohona", "Jacket": "raincoat_orange", "Pants": "fisher_yellow", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_CouncilHead_Brown", "council", {"Hat": "flatcap_darkgrey", "Jacket": "suit_brown", "Pants": "suitpants_brown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_CouncilHead_Denim", "council", {"Hat": "", "Jacket": "denim_brown", "Pants": "trousers_darkbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_CouncilHead_Beret", "council", {"Hat": "beret_brown", "Jacket": "turtle_beige", "Pants": "suitpants_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_Leader_Haven_Denim", "leader_haven", {"Hat": "flatcap_darkgrey", "Jacket": "denim_brown", "Pants": "trousers_darkbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_Leader_Haven_Knit", "leader_haven", {"Hat": "knit_brown", "Jacket": "turtle_beige", "Pants": "trousers_lightbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_wool"}, {}),
    # The survivors' two civilians (the survivor group): the Haven's people, unarmed
    ("Haven", "RZ_Civ_Haven_Knit", "civ_turtleneck", {"Hat": "knit_blue", "Jacket": "turtle_beige", "Pants": "trousers_darkbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_wool"}, {}),
    ("Haven", "RZ_Civ_Haven_Denim", "civ_cottonshirt", {"Hat": "flatcap_lightgrey", "Jacket": "denim_brown", "Pants": "trousers_lightbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    # The Haven's watch (guards, safehouse guards): green oilskins and fisherman's trousers, the hat varies
    ("Haven", "RZ_Armed_Haven_Hunter", "fia_ac", {"Hat": "knit_black", "Jacket": "raincoat_green", "Pants": "fisher_green", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_Armed_Haven_Watch", "fia_ac", {"Hat": "trucker_everon", "Jacket": "raincoat_green", "Pants": "fisher_green", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather"}, {}),
    ("Haven", "RZ_Armed_Haven_Winter", "fia_ac", {"Hat": "ushanka_folded", "Jacket": "raincoat_green", "Pants": "fisher_green", "Boots": "boots_soviet_dirty", "Back": "veshmeshok", "ArmoredVest": "", "HandWear": "gloves_wool"}, {}),
    ("Haven", "RZ_Armed_Haven_Lookout", "fia_ac", {"Hat": "flatcap_blue", "Jacket": "raincoat_green", "Pants": "fisher_green", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    # The Haven's drivers and the survivors' armed men: work clothes
    ("Haven", "RZ_Armed_Haven_Farmhand", "fia_ac", {"Hat": "trucker_everon", "Jacket": "denim_brown", "Pants": "trousers_darkbrown", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Haven", "RZ_Armed_Haven_Worker", "fia_ac", {"Hat": "hardhat_white", "Jacket": "shirt_3", "Pants": "trousers_darkgrey", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather"}, {}),
    ("Haven", "RZ_Armed_Haven_Overalls", "fia_ac", {"Hat": "flatcap_blue", "Jacket": "overalls", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {"Pants": "Back"}),
    ("Haven", "RZ_Armed_Haven_Mechanic", "fia_ac", {"Hat": "", "Jacket": "mechanic", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_wool"}, {"Pants": "Back"}),

    # ---- Task Force Bastion: one olive kit
    # off duty (occupants): no webbing, a cap or bare-headed
    ("Bastion", "RZ_Occupant_Bastion_Rolled", "occ_bastion", {"Hat": "patrol_olive", "Jacket": "bdu_rolled_02", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "Vest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_Occupant_Bastion_Boonie", "occ_bastion", {"Hat": "boonie_olive", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "Vest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_Occupant_Bastion_Raincoat", "occ_bastion", {"Hat": "patrol_olive", "Jacket": "us_raincoat", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "Vest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_Occupant_Bastion_Jacket", "occ_bastion", {"Hat": "", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "Vest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_Occupant_Bastion_OffDuty", "occ_bastion", {"Hat": "", "Jacket": "bdu_rolled_02", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "Vest": "", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_QM_Bastion_Patrol", "qm_bastion", {"Hat": "patrol_olive", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_QM_Bastion_Rolled", "qm_bastion", {"Hat": "", "Jacket": "bdu_rolled_02", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_QM_Bastion_Gloves", "qm_bastion", {"Hat": "boonie_olive", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_Outpost_Bastion_Patrol", "outpost_bastion", {"Hat": "patrol_olive", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_Outpost_Bastion_Helmet", "outpost_bastion", {"Hat": "pasgt", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_Leader_Bastion_Patrol", "leader_bastion", {"Hat": "patrol_olive", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "HandWear": ""}, {}),
    ("Bastion", "RZ_Leader_Bastion_Rolled", "leader_bastion", {"Hat": "patrol_olive", "Jacket": "bdu_rolled_02", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "gloves_pilot"}, {}),
    # on duty (guards, road posts, hunters, truck crews, the mission squads): flak vest and webbing
    ("Bastion", "RZ_Armed_Bastion_Helmet", "us_rifleman", {"Hat": "pasgt", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": ""}, {}),
    ("Bastion", "RZ_Armed_Bastion_Goggles", "us_rifleman", {"Hat": "pasgt_goggles", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_Armed_Bastion_M1", "us_rifleman", {"Hat": "m1_nostrap", "Jacket": "bdu_rolled_02", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": ""}, {}),
    ("Bastion", "RZ_Armed_Bastion_Patrol", "us_rifleman", {"Hat": "patrol_olive", "Jacket": "bdu_rolled_02", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": ""}, {}),
    ("Bastion", "RZ_Armed_Bastion_Boonie", "us_rifleman", {"Hat": "boonie_olive", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_Armed_Bastion_Lead", "us_tl", {"Hat": "pasgt_goggles", "Jacket": "m65_jacket", "Pants": "m65", "Boots": "boots_us", "ArmoredVest": "pasgt_vest", "HandWear": "gloves_pilot"}, {}),

    # ---- The Red Hand: scavenged and mismatched, every armed one in red
    ("RedHand", "RZ_Occupant_RedHand_Balaclava", "occ_redhand", {"Hat": "", "Face": "balaclava_black", "Jacket": "bandit_1", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_wool_dirty"}, {}),
    ("RedHand", "RZ_Occupant_RedHand_Tracksuit", "occ_redhand", {"Hat": "cap_plain_back", "Jacket": "track_black", "Pants": "trackpants_black", "Boots": "sneakers_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("RedHand", "RZ_Occupant_RedHand_Sweatshirt", "occ_redhand", {"Hat": "knit_red", "Jacket": "sweat_dirty", "Pants": "jeans_1", "Boots": "sneakers_black", "ArmoredVest": "", "HandWear": "gloves_skeleton"}, {}),
    ("RedHand", "RZ_Occupant_RedHand_ThreeHole", "occ_redhand", {"Hat": "", "Face": "balaclava3_olive", "Jacket": "bandit_2", "Pants": "cargo_tan", "Boots": "sneakers_black", "ArmoredVest": "", "HandWear": ""}, {}),
    ("RedHand", "RZ_Occupant_RedHand_RedTop", "occ_redhand", {"Hat": "cap_trophy", "Jacket": "sweat_red", "Pants": "jeans_2", "Boots": "sneakers_dirty", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_QM_RedHand_Cap", "qm_redhand", {"Hat": "zcap_red", "Jacket": "bandit_1", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_QM_RedHand_Balaclava", "qm_redhand", {"Hat": "knit_red", "Face": "balaclava_black", "Jacket": "track_black", "Pants": "trackpants_black", "Boots": "sneakers_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("RedHand", "RZ_QM_RedHand_Beret", "qm_redhand", {"Hat": "beret_black_dirty", "Jacket": "sweat_red", "Pants": "jeans_2", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("RedHand", "RZ_Outpost_RedHand_Balaclava", "outpost_redhand", {"Hat": "knit_red", "Face": "balaclava_olive", "Jacket": "bandit_2", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_Outpost_RedHand_Tracksuit", "outpost_redhand", {"Hat": "zcap_red", "Jacket": "track_navy", "Pants": "trackpants_navy", "Boots": "sneakers_dirty", "ArmoredVest": "", "HandWear": ""}, {}),
    ("RedHand", "RZ_Leader_RedHand_Boss", "leader_redhand", {"Hat": "knit_red", "Jacket": "bandit_1", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_Leader_RedHand_Beret", "leader_redhand", {"Hat": "beret_black_dirty", "Jacket": "sweat_red", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Balaclava", "fia_partisan", {"Hat": "knit_red", "Face": "balaclava_black", "Jacket": "bandit_1", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_wool_dirty"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Tracksuit", "fia_partisan", {"Hat": "zcap_red", "Jacket": "track_black", "Pants": "trackpants_black", "Boots": "sneakers_dirty", "ArmoredVest": "", "HandWear": "gloves_wool_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_RedTop", "fia_partisan", {"Hat": "", "Face": "balaclava3_black", "Jacket": "sweat_red", "Pants": "jeans_1", "Boots": "sneakers_black", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Hood", "fia_partisan", {"Hat": "knit_red", "Face": "balaclava3_olive", "Jacket": "bandit_2", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_skeleton"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Helmet", "fia_partisan", {"Hat": "pasgt_black", "Jacket": "sweat_red", "Pants": "cargo_black", "Boots": "boots_soviet_dirty", "ArmoredVest": "", "HandWear": "gloves_leather_black"}, {}),

    # ---- Halcyon: staff (occupants), quartermasters, field leads, the director
    ("Halcyon", "RZ_Occupant_Halcyon_Beak", "occ_halcyon", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_olive", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "nitrile_blue"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Clerk", "occ_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "suit_black", "Pants": "suitpants_black", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Technician", "occ_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "turtle_grey", "Pants": "suitpants_grey", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_purple"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Paramedic", "occ_halcyon", {"Hat": "", "Jacket": "ems_jacket", "Pants": "ems_pants", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Researcher", "occ_halcyon", {"Hat": "", "Head": "gasmask", "Jacket": "suit_grey", "Pants": "suitpants_black", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_HiVis", "occ_halcyon", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_hivis", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "nitrile_yellow"}, {}),
    ("Halcyon", "RZ_QM_Halcyon_Coat", "qm_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "suit_grey", "Pants": "suitpants_grey", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_QM_Halcyon_Paramedic", "qm_halcyon", {"Hat": "", "Jacket": "ems_jacket", "Pants": "ems_pants", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_blue"}, {}),
    ("Halcyon", "RZ_QM_Halcyon_GasMask", "qm_halcyon", {"Hat": "", "Head": "gasmask", "Jacket": "turtle_grey", "Pants": "suitpants_black", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Outpost_Halcyon_Paramedic", "outpost_halcyon", {"Hat": "", "Jacket": "ems_jacket", "Pants": "ems_pants", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Outpost_Halcyon_Coat", "outpost_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "suit_black", "Pants": "suitpants_black", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Leader_Halcyon_Coat", "leader_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "suit_grey", "Pants": "suitpants_grey", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Leader_Halcyon_Black", "leader_halcyon", {"Hat": "", "Jacket": "suit_black", "Pants": "suitpants_black", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    # the escorted scientist: a researcher, not a soldier
    ("Halcyon", "RZ_Staff_Halcyon_Researcher", "fia_medic", {"Hat": "", "Face": "medmask", "Jacket": "suit_grey", "Pants": "suitpants_grey", "Boots": "boots_paramedic", "ArmoredVest": "", "HandWear": "nitrile_white"}, {}),
    # Halcyon's security (guards, the lab's extras): black hooded hazmat, black gloves, the mask varies
    ("Halcyon", "RZ_Armed_Halcyon_BlackSuit", "us_rifleman_v1", {"Hat": "", "Face": "hazmask_tinted", "Jacket": "hazmat_black", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "gloves_pilot_black"}, {"Pants": "Vest"}),
    ("Halcyon", "RZ_Armed_Halcyon_Hood", "us_rifleman_v1", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_black", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "gloves_pilot_black"}, {"Pants": "Vest"}),
    ("Halcyon", "RZ_Armed_Halcyon_GasMask", "us_rifleman_v1", {"Hat": "", "Head": "gasmask", "Jacket": "hazmat_black", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "gloves_pilot_black"}, {"Pants": "Vest"}),
    # Halcyon's field crews (supply trucks): olive hazmat
    ("Halcyon", "RZ_Armed_Halcyon_Hazmat", "us_rifleman_v1", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_olive", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "nitrile_blue"}, {"Pants": "Vest"}),
    ("Halcyon", "RZ_Armed_Halcyon_Field", "us_rifleman_v1", {"Hat": "", "Face": "hazmask_tinted", "Jacket": "hazmat_olive", "Boots": "boots_us", "ArmoredVest": "", "HandWear": "gloves_pilot_black"}, {"Pants": "Vest"}),
]

# (folder, file stem, parent group, [member stems]): no two groups of a slot share their mix.
GROUPS = [
    ("Haven", "RZ_Guard_Haven_1", "grp_guard_haven", ["RZ_Armed_Haven_Hunter", "RZ_Armed_Haven_Watch"]),
    ("Haven", "RZ_Guard_Haven_2", "grp_guard_haven", ["RZ_Armed_Haven_Winter", "RZ_Armed_Haven_Lookout"]),
    ("Haven", "RZ_Guard_Haven_3", "grp_guard_haven", ["RZ_Armed_Haven_Watch", "RZ_Armed_Haven_Winter"]),
    ("Haven", "RZ_SafehouseGuard_1", "grp_safehouse_guard", ["RZ_Armed_Haven_Watch"]),
    ("Haven", "RZ_SafehouseGuard_2", "grp_safehouse_guard", ["RZ_Armed_Haven_Lookout"]),
    ("Haven", "RZ_SafehouseGuard_3", "grp_safehouse_guard", ["RZ_Armed_Haven_Hunter"]),
    ("Haven", "RZ_TruckCrew_Haven_1", "grp_truck_haven", ["RZ_Armed_Haven_Farmhand", "RZ_Armed_Haven_Mechanic"]),
    ("Haven", "RZ_TruckCrew_Haven_2", "grp_truck_haven", ["RZ_Armed_Haven_Overalls", "RZ_Armed_Haven_Worker"]),
    ("Bastion", "RZ_Guard_Bastion_1", "grp_guard_bastion", ["RZ_Armed_Bastion_Helmet", "RZ_Armed_Bastion_M1"]),
    ("Bastion", "RZ_Guard_Bastion_2", "grp_guard_bastion", ["RZ_Armed_Bastion_Goggles", "RZ_Armed_Bastion_Helmet"]),
    ("Bastion", "RZ_Guard_Bastion_3", "grp_guard_bastion", ["RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Goggles"]),
    ("Bastion", "RZ_BastionPostCrew_1", "grp_post_bastion", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Helmet", "RZ_Armed_Bastion_M1"]),
    ("Bastion", "RZ_BastionPostCrew_2", "grp_post_bastion", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Goggles", "RZ_Armed_Bastion_Helmet"]),
    ("Bastion", "RZ_BastionPostCrew_3", "grp_post_bastion", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Patrol"]),
    ("Bastion", "RZ_BastionHunters_1", "grp_hunters_bastion", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Boonie", "RZ_Armed_Bastion_Patrol", "RZ_Armed_Bastion_Helmet"]),
    ("Bastion", "RZ_BastionHunters_2", "grp_hunters_bastion", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Patrol", "RZ_Armed_Bastion_Boonie", "RZ_Armed_Bastion_M1"]),
    ("Bastion", "RZ_TruckCrew_Bastion_1", "grp_truck_bastion", ["RZ_Armed_Bastion_Patrol", "RZ_Armed_Bastion_Boonie"]),
    ("Bastion", "RZ_TruckCrew_Bastion_2", "grp_truck_bastion", ["RZ_Armed_Bastion_Boonie", "RZ_Armed_Bastion_Helmet"]),
    ("RedHand", "RZ_Guard_RedHand_1", "grp_guard_redhand", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit"]),
    ("RedHand", "RZ_Guard_RedHand_2", "grp_guard_redhand", ["RZ_Armed_RedHand_Hood", "RZ_Armed_RedHand_RedTop"]),
    ("RedHand", "RZ_Guard_RedHand_3", "grp_guard_redhand", ["RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_Tracksuit"]),
    ("RedHand", "RZ_RedHandPostCrew_1", "grp_post_redhand", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Hood"]),
    ("RedHand", "RZ_RedHandPostCrew_2", "grp_post_redhand", ["RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_RedTop", "RZ_Armed_RedHand_Balaclava"]),
    ("RedHand", "RZ_RedHandPostCrew_3", "grp_post_redhand", ["RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Hood", "RZ_Armed_RedHand_Helmet"]),
    ("RedHand", "RZ_RedHandHunters_1", "grp_hunters_redhand", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Hood", "RZ_Armed_RedHand_RedTop"]),
    ("RedHand", "RZ_RedHandHunters_2", "grp_hunters_redhand", ["RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_RedTop", "RZ_Armed_RedHand_Hood", "RZ_Armed_RedHand_Balaclava"]),
    ("RedHand", "RZ_TruckCrew_RedHand_1", "grp_truck_redhand", ["RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Helmet"]),
    ("RedHand", "RZ_TruckCrew_RedHand_2", "grp_truck_redhand", ["RZ_Armed_RedHand_RedTop", "RZ_Armed_RedHand_Balaclava"]),
    ("Halcyon", "RZ_Guard_Halcyon_1", "grp_guard_halcyon", ["RZ_Armed_Halcyon_BlackSuit", "RZ_Armed_Halcyon_GasMask"]),
    ("Halcyon", "RZ_Guard_Halcyon_2", "grp_guard_halcyon", ["RZ_Armed_Halcyon_Hood", "RZ_Armed_Halcyon_BlackSuit"]),
    ("Halcyon", "RZ_Guard_Halcyon_3", "grp_guard_halcyon", ["RZ_Armed_Halcyon_GasMask", "RZ_Armed_Halcyon_Hood"]),
    ("Halcyon", "RZ_TruckCrew_Halcyon_1", "grp_truck_halcyon", ["RZ_Armed_Halcyon_Hazmat", "RZ_Armed_Halcyon_Field"]),
    ("Halcyon", "RZ_TruckCrew_Halcyon_2", "grp_truck_halcyon", ["RZ_Armed_Halcyon_Field", "RZ_Armed_Halcyon_Hazmat"]),
]

# The faction group prefabs themselves, dressed: their members are rewritten to these looks (a stem)
# or kept as given (a full ResourceName). Every path that spawns a slot's own prefab, not a look of it
# (a mission's group, a slot whose Looks list is empty), then still puts the faction's people in the
# field, never a vanilla US or FIA soldier. (base prefab path, [members])
BASES = [
    ("Prefabs/ReforgerZ/Territory/RZ_Guard_Haven.et", ["RZ_Armed_Haven_Hunter", "RZ_Armed_Haven_Watch"]),
    ("Prefabs/ReforgerZ/Territory/RZ_Guard_Bastion.et", ["RZ_Armed_Bastion_Helmet", "RZ_Armed_Bastion_M1"]),
    ("Prefabs/ReforgerZ/Territory/RZ_Guard_RedHand.et", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Hood"]),
    ("Prefabs/ReforgerZ/Territory/RZ_Guard_Halcyon.et", ["RZ_Armed_Halcyon_BlackSuit", "RZ_Armed_Halcyon_Hood"]),
    ("Prefabs/ReforgerZ/Haven/RZ_SafehouseGuard.et", ["RZ_Armed_Haven_Watch"]),
    ("Prefabs/ReforgerZ/Factions/RZ_BastionPostCrew.et", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Helmet", "RZ_Armed_Bastion_Goggles"]),
    ("Prefabs/ReforgerZ/Factions/RZ_RedHandPostCrew.et", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Hood", "RZ_Armed_RedHand_Tracksuit"]),
    ("Prefabs/ReforgerZ/Factions/RZ_BastionHunters.et", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Boonie", "RZ_Armed_Bastion_Patrol", "RZ_Armed_Bastion_Helmet"]),
    ("Prefabs/ReforgerZ/Factions/RZ_RedHandHunters.et", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Hood", "RZ_Armed_RedHand_RedTop"]),
    ("Prefabs/ReforgerZ/Supply/RZ_TruckCrew_Haven.et", ["RZ_Armed_Haven_Farmhand", "RZ_Armed_Haven_Overalls"]),
    ("Prefabs/ReforgerZ/Supply/RZ_TruckCrew_Bastion.et", ["RZ_Armed_Bastion_Patrol", "RZ_Armed_Bastion_Boonie"]),
    ("Prefabs/ReforgerZ/Supply/RZ_TruckCrew_RedHand.et", ["RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Helmet"]),
    ("Prefabs/ReforgerZ/Supply/RZ_TruckCrew_Halcyon.et", ["RZ_Armed_Halcyon_Hazmat", "RZ_Armed_Halcyon_Field"]),
    # the missions' faction groups (no Looks list: they spawn as themselves)
    ("Prefabs/ReforgerZ/Missions/RZ_BastionSquad.et", ["RZ_Armed_Bastion_Lead", "RZ_Armed_Bastion_Helmet", "RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Goggles"]),
    ("Prefabs/ReforgerZ/Missions/RZ_EscortSoldier.et", ["RZ_Armed_Bastion_Patrol"]),
    ("Prefabs/ReforgerZ/Missions/RZ_RedHandGroup.et", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Hood"]),
    ("Prefabs/ReforgerZ/Missions/RZ_EscortScientist.et", ["RZ_Staff_Halcyon_Researcher"]),
    # the survivors (the Haven's side): their armed men and two unarmed civilians, in work clothes
    ("Prefabs/ReforgerZ/Missions/RZ_SurvivorGroup.et", ["RZ_Armed_Haven_Farmhand", "RZ_Civ_Haven_Knit",
                                                        "RZ_Armed_Haven_Worker", "RZ_Civ_Haven_Denim"]),
]

# The arrays on RZ_GameMode.et: (settings block, field, parent key, stem prefix).
ARRAYS = [
    ("m_TerritorySettings", "m_aHavenGuardLooks", "grp_guard_haven", "RZ_Guard_Haven_"),
    ("m_TerritorySettings", "m_aBastionGuardLooks", "grp_guard_bastion", "RZ_Guard_Bastion_"),
    ("m_TerritorySettings", "m_aRedHandGuardLooks", "grp_guard_redhand", "RZ_Guard_RedHand_"),
    ("m_TerritorySettings", "m_aHalcyonGuardLooks", "grp_guard_halcyon", "RZ_Guard_Halcyon_"),
    ("m_TerritorySettings", "m_aHavenOccupantLooks", "occ_haven", "RZ_Occupant_Haven_"),
    ("m_TerritorySettings", "m_aBastionOccupantLooks", "occ_bastion", "RZ_Occupant_Bastion_"),
    ("m_TerritorySettings", "m_aRedHandOccupantLooks", "occ_redhand", "RZ_Occupant_RedHand_"),
    ("m_TerritorySettings", "m_aHalcyonOccupantLooks", "occ_halcyon", "RZ_Occupant_Halcyon_"),
    ("m_SafehouseSettings", "m_aGuardLooks", "grp_safehouse_guard", "RZ_SafehouseGuard_"),
    ("m_SafehouseSettings", "m_aOccupantPrefabs (append)", "occ_haven", "RZ_Occupant_Haven_"),
    ("m_LeaderSettings", "m_aHavenLeaderLooks", "leader_haven", "RZ_Leader_Haven_"),
    ("m_LeaderSettings", "m_aBastionLeaderLooks", "leader_bastion", "RZ_Leader_Bastion_"),
    ("m_LeaderSettings", "m_aRedHandLeaderLooks", "leader_redhand", "RZ_Leader_RedHand_"),
    ("m_LeaderSettings", "m_aHalcyonLeaderLooks", "leader_halcyon", "RZ_Leader_Halcyon_"),
    ("m_LeaderSettings", "m_aCouncilHeadLooks", "council", "RZ_CouncilHead_"),
    ("m_LeaderSettings", "m_aBastionOutpostLooks", "outpost_bastion", "RZ_Outpost_Bastion_"),
    ("m_LeaderSettings", "m_aRedHandOutpostLooks", "outpost_redhand", "RZ_Outpost_RedHand_"),
    ("m_LeaderSettings", "m_aHalcyonOutpostLooks", "outpost_halcyon", "RZ_Outpost_Halcyon_"),
    ("m_SupplySettings", "m_aHavenCrewLooks", "grp_truck_haven", "RZ_TruckCrew_Haven_"),
    ("m_SupplySettings", "m_aBastionCrewLooks", "grp_truck_bastion", "RZ_TruckCrew_Bastion_"),
    ("m_SupplySettings", "m_aRedHandCrewLooks", "grp_truck_redhand", "RZ_TruckCrew_RedHand_"),
    ("m_SupplySettings", "m_aHalcyonCrewLooks", "grp_truck_halcyon", "RZ_TruckCrew_Halcyon_"),
    ("(game mode)", "m_aQuartermasterHavenLooks", "qm_haven", "RZ_QM_Haven_"),
    ("(game mode)", "m_aQuartermasterBastionLooks", "qm_bastion", "RZ_QM_Bastion_"),
    ("(game mode)", "m_aQuartermasterRedHandLooks", "qm_redhand", "RZ_QM_RedHand_"),
    ("(game mode)", "m_aQuartermasterHalcyonLooks", "qm_halcyon", "RZ_QM_Halcyon_"),
    ("(game mode)", "m_aBastionHunterLooks", "grp_hunters_bastion", "RZ_BastionHunters_"),
    ("(game mode)", "m_aRedHandHunterLooks", "grp_hunters_redhand", "RZ_RedHandHunters_"),
    ("m_CheckpointSettings", "m_aBastionCrewLooks", "grp_post_bastion", "RZ_BastionPostCrew_"),
    ("m_CheckpointSettings", "m_aRedHandCrewLooks", "grp_post_redhand", "RZ_RedHandPostCrew_"),
]

META = """MetaFileClass {
 Name "%s"
 Configurations {
  EntityTemplateResourceClass PC {
  }
  EntityTemplateResourceClass XBOX_ONE : PC {
  }
  EntityTemplateResourceClass XBOX_SERIES : PC {
  }
  EntityTemplateResourceClass PS4 : PC {
  }
  EntityTemplateResourceClass PS5 : PC {
  }
  EntityTemplateResourceClass HEADLESS : PC {
  }
 }
}
"""


def guid_for(path):
    return hashlib.md5(("ReforgerZ:" + path).encode()).hexdigest()[:16].upper()


def resource(folder, stem):
    path = "%s/%s/%s.et" % (OUT, folder, stem)
    return "{%s}%s" % (guid_for(path), path), path


def file_name(rn):
    return rn.split("/")[-1]


def char_text(parent_key, slots, moves):
    parent, entity_id, packed = PARENTS[parent_key]
    worn = dict(slots)
    for slot, item in slots.items():
        if item not in I:
            sys.exit("people_looks: unknown item %r" % item)
        if item in BLOCKS_PANTS:
            worn["Pants"] = ""
    # Every garment you can see is the look's own: a slot left to the parent shows its vanilla kit
    missing = [slot for slot in DRESSED if slot not in worn]
    if missing:
        sys.exit("people_looks: a look on %s leaves %s to the parent: set them (\"\" empties one)" % (parent_key, ", ".join(missing)))
    lines = ['SCR_ChimeraCharacter : "%s" {' % parent, ' ID "%s"' % entity_id, " components {"]
    # No random variant: a group spawns the look itself, never the vanilla soldier it's built on
    if parent_key in VARIANTS:
        component, data, ids = VARIANTS[parent_key]
        lines += ["  %s {" % component, "   m_VariantData %s {" % data, "    m_aVariants {"]
        for vid in ids:
            lines += ['     SCR_EditableEntityVariant "%s" {' % vid, "      m_bEnabled 0", "     }"]
        lines += ["    }", "   }", "  }"]
    lines += ['  BaseLoadoutManagerComponent "{520EA1D2DB11821E}" {', "   Slots {"]
    for slot in SLOT_ORDER:
        if slot not in worn:
            continue
        lines += ["    %s %s {" % (SLOT_CLASS.get(slot, "LoadoutSlotInfo"), slot),
                  '     Prefab "%s"' % I[worn[slot]], "    }"]
    lines += ["   }", "  }"]
    # Point the parent's packed item lists at the garment that replaced theirs
    retarget = []
    for slot, ids in sorted(packed.items()):
        if slot not in worn:
            continue
        dest = slot
        if worn[slot] == "":
            dest = moves.get(slot)
            if not dest:
                sys.exit("people_looks: a look empties %s, which holds the parent's items: say where they go" % slot)
            if not worn.get(dest) and dest not in ("Back", "Vest"):
                sys.exit("people_looks: items moved to %s, which the look doesn't set" % dest)
        if dest in worn and worn[dest]:
            target = "%s/%s" % (dest, file_name(I[worn[dest]]))
        else:
            target = None  # the parent's own garment in that slot (Back, Vest): its own name
        for cid in ids:
            retarget.append((cid, dest, target))
    if retarget:
        lines += ['  SCR_InventoryStorageManagerComponent "{520EA1D2DB118DE5}" {', "   InitialInventoryItems {"]
        for cid, dest, target in retarget:
            if target is None:
                target = PARENT_SLOT_ITEM[(parent_key, dest)]
            lines += ['    ItemsInitConfigurationItem "%s" {' % cid, '     TargetStorage "%s"' % target, "    }"]
        lines += ["   }", "  }"]
    lines += [" }", " coords 0 0 0", "}"]
    return "\n".join(lines) + "\n"


# The parent's own garment in a slot, for items moved onto it (the slot the look keeps).
PARENT_SLOT_ITEM = {
    ("fia_ac", "Back"): "Back/Backpack_Medical_Soviet_NoCross_01.et",
    ("us_rifleman_v1", "Vest"): "Vest/Vest_ALICE_rifleman.et",
    ("us_rifleman", "Vest"): "Vest/Vest_ALICE_rifleman.et",
}


def group_text(parent_key, members):
    parent, entity_id, _ = PARENTS[parent_key]
    lines = ['SCR_AIGroup : "%s" {' % parent, ' ID "%s"' % entity_id, " coords 0 0 0", " m_aUnitPrefabSlots {",
             "  " + " ".join('"%s"' % m for m in members), " }", "}"]
    return "\n".join(lines) + "\n"


def base_text(path, members):
    """The slot's own group prefab with its members replaced (BASES)."""
    full = os.path.join(ROOT, path)
    text = open(full, encoding="utf-8").read()
    m = re.search(r"( m_aUnitPrefabSlots \{\n)  [^\n]*\n( \})", text)
    if not m:
        sys.exit("people_looks: no m_aUnitPrefabSlots in %s" % path)
    line = "  " + " ".join('"%s"' % n for n in members) + "\n"
    return text[:m.start()] + m.group(1) + line + m.group(2) + text[m.end():]


def check_index(names):
    path = os.environ.get("RZ_RESOURCE_INDEX", DEFAULT_INDEX)
    if path == "none":
        return
    if not os.path.exists(path):
        print("people_looks: no resource index at %s: GUIDs not checked" % path)
        return
    known = set(l.strip() for l in open(path, encoding="utf-8", errors="replace"))
    missing = [n for n in names if n and n not in known]
    if missing:
        sys.exit("people_looks: not in the resource index:\n  " + "\n  ".join(missing))


def write(path, text):
    full = os.path.join(ROOT, path)
    os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "w", encoding="utf-8", newline="\n") as f:
        f.write(text)


def build():
    """Every file the tool owns ({path: text}: the looks, their .meta and the dressed groups) and the
    looks' ResourceNames by stem."""
    files = {}
    made = {}
    for folder, stem, parent, slots, moves in CHARS:
        rn, path = resource(folder, stem)
        files[path] = char_text(parent, slots, moves)
        files[path + ".meta"] = META % rn
        made[stem] = rn
    for folder, stem, parent, members in GROUPS:
        rn, path = resource(folder, stem)
        for m in members:
            if m not in made:
                sys.exit("people_looks: group %s lists unknown look %s" % (stem, m))
        files[path] = group_text(parent, [made[m] for m in members])
        files[path + ".meta"] = META % rn
        made[stem] = rn
    for path, members in BASES:
        names = []
        for m in members:
            if m.startswith("{"):
                names.append(m)
            elif m in made:
                names.append(made[m])
            else:
                sys.exit("people_looks: %s lists unknown look %s" % (path, m))
        files[path] = base_text(path, names)
    return files, made


def looks_on_disk():
    """The .et files under OUT now, relative to ROOT."""
    found = []
    for dirpath, _, names in os.walk(os.path.join(ROOT, OUT)):
        for f in names:
            if f.endswith(".et"):
                found.append(os.path.relpath(os.path.join(dirpath, f), ROOT).replace(os.sep, "/"))
    return sorted(found)


def main():
    if "--check" in sys.argv:
        sys.exit(check())
    check_index([v for v in I.values()] + [PARENTS[k][0] for k in PARENTS if not PARENTS[k][0].split("}")[1].startswith("Prefabs/ReforgerZ")])
    files, made = build()
    for path, text in files.items():
        write(path, text)
    # Looks no longer in the tables go, with their .meta
    gone = 0
    for rel in looks_on_disk():
        if rel not in files:
            os.remove(os.path.join(ROOT, rel))
            if os.path.exists(os.path.join(ROOT, rel + ".meta")):
                os.remove(os.path.join(ROOT, rel + ".meta"))
            gone += 1
    print("people_looks: wrote %d prefabs under %s, dressed %d faction groups, removed %d old looks"
          % (len(made), OUT, len(BASES), gone))
    if "--arrays" in sys.argv:
        for block, field, _, prefix in ARRAYS:
            names = [made[s] for s in made if s.startswith(prefix)]
            print("%s.%s {\n %s\n}" % (block, field, " ".join('"%s"' % n for n in names)))


# ------------------------------------------------------------------------------------------ prefab text
def parse_et(text):
    """An Enfusion text prefab as a tree: {"h": header, "p": [property lines], "c": [child nodes]}.
    Line based, which is how Workbench writes them: a line ending in { opens a node, } closes it."""
    root = {"h": "", "p": [], "c": []}
    stack = [root]
    for line in text.split("\n"):
        s = line.strip()
        if not s:
            continue
        if s.endswith("{"):
            node = {"h": s[:-1].strip(), "p": [], "c": []}
            stack[-1]["c"].append(node)
            stack.append(node)
        elif s == "}" and len(stack) > 1:
            stack.pop()
        else:
            stack[-1]["p"].append(s)
    return root["c"][0] if root["c"] else None


def child(node, start):
    for c in node["c"] if node else []:
        if c["h"] == start or c["h"].startswith(start + " "):
            return c
    return None


def strings(node):
    """The quoted values of a node's lines (an array of ResourceNames)."""
    found = []
    for line in node["p"] if node else []:
        found += re.findall(r'"([^"]*)"', line)
    return found


def prop(node, key):
    for line in node["p"] if node else []:
        if line == key or line.startswith(key + " "):
            return line[len(key):].strip().strip('"')
    return None


def parent_of(node):
    m = re.search(r':\s*"(\{[0-9A-F]{16}\}[^"]+)"', node["h"]) if node else None
    return m.group(1) if m else None


def rn_path(rn):
    return rn.split("}", 1)[1] if rn.startswith("{") else rn


def rn_guid(rn):
    return rn[1:17] if rn.startswith("{") else None


def disk_guid(path):
    """A file of ours: the GUID its .meta gives it."""
    meta = os.path.join(ROOT, path + ".meta")
    if not os.path.exists(meta):
        return None
    m = re.search(r'Name "\{([0-9A-F]{16})\}', open(meta, encoding="utf-8").read())
    return m.group(1) if m else None


# ------------------------------------------------------------------------------------------ game data
class GameData:
    """The game's and the Workshop dependencies' packed data, for --check: each resource's GUID from
    the resourceDatabase.rdb files, and prefab text from the .pak files (read as tools/pak_extract.py
    does). game: the game's or the server's folder (or its addons folder); mods: the folder that holds
    the downloaded Workshop mods (BaconZombies_622120A5448725E3 and the rest)."""

    def __init__(self, game, mods):
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        from pak_extract import read_tree
        addons = os.path.join(game, "addons") if os.path.isdir(os.path.join(game, "addons")) else game
        self.rdbs = []
        self.files = {}  # path -> [(pak, offset, packed size, size)]: the game's first, then the mods'
        for sub in ("core", "data"):
            self._add(os.path.join(addons, sub), read_tree)
        if not self.rdbs:
            sys.exit("people_looks: no resourceDatabase.rdb under %s" % addons)
        # Only the mods the project loads: addon.gproj's dependencies, and theirs
        by_guid = {}
        for d in sorted(os.listdir(mods)) if mods and os.path.isdir(mods) else []:
            m = re.search(r"_([0-9A-F]{16})$", d)
            if m:
                by_guid[m.group(1)] = os.path.join(mods, d)
        wanted = self._deps(os.path.join(ROOT, "addon.gproj"))
        seen = set()
        while wanted:
            g = wanted.pop()
            if g in seen or g not in by_guid:
                continue
            seen.add(g)
            self._add(by_guid[g], read_tree)
            wanted += self._deps(os.path.join(by_guid[g], "addon.gproj"))
        self.mod_count = len(seen)

    @staticmethod
    def _deps(gproj):
        if not os.path.exists(gproj):
            return []
        text = open(gproj, encoding="utf-8", errors="replace").read()
        m = re.search(r"Dependencies \{([^}]*)\}", text)
        return re.findall(r'"([0-9A-F]{16})"', m.group(1)) if m else []

    def _add(self, folder, read_tree):
        if not os.path.isdir(folder):
            return
        rdb = os.path.join(folder, "resourceDatabase.rdb")
        if os.path.exists(rdb):
            self.rdbs.append(open(rdb, "rb").read())
        for pak in sorted(f for f in os.listdir(folder) if f.endswith(".pak")):
            handle, entries = read_tree(os.path.join(folder, pak))
            handle.close()
            for name, off, packed, size in entries:
                self.files.setdefault(name.lstrip("/"), []).append((os.path.join(folder, pak), off, packed, size))

    def guids(self, path):
        """The GUIDs the game knows a path by: usually one, but two mods can each ship a file at the
        same path (MedicalAccessories' mask sits at Zelik's ZEL_FaceClothing_Base.et), and the game
        tells them apart by GUID. An .rdb entry is the length (u32) of the path and its closing zero,
        the path, the zero, six bytes, then the GUID (eight bytes, little-endian)."""
        key = struct.pack("<I", len(path.encode()) + 1) + path.encode() + b"\x00"
        found = []
        for data in self.rdbs:
            i = data.find(key)
            if i >= 0:
                j = i + len(key) + 6
                found.append(data[j:j + 8][::-1].hex().upper())
        return found

    def texts(self, path):
        """A prefab's text, then each override of the same path a mod brings (Zelik's Character adds
        its Face and Head slots to Character_Base that way)."""
        local = os.path.join(ROOT, path)
        if path.startswith("Prefabs/ReforgerZ/") and os.path.exists(local):
            return [open(local, encoding="utf-8").read()]
        found = []
        for pak, off, packed, size in self.files.get(path, []):
            with open(pak, "rb") as f:
                f.seek(off)
                blob = f.read(packed)
            if packed != size:
                blob = zlib.decompress(blob)
            found.append(blob.decode("utf-8", "replace"))
        return found

    def resolves(self, rn):
        """Whether the game finds this ResourceName by its GUID: ours by the .meta, the rest by .rdb."""
        path = rn_path(rn)
        if path.startswith("Prefabs/ReforgerZ/"):
            return disk_guid(path) == rn_guid(rn)
        return rn_guid(rn) in self.guids(path)

    def view(self, rn):
        """What a prefab ends up with, merged down its chain: its loadout slots {name: prefab}, its
        starting item lists {id: [target storage, items]} and its editable-entity variants
        {id: [prefab, enabled]}. None if a prefab of the chain can't be read."""
        chain = []
        while rn:
            texts = self.texts(rn_path(rn))
            if not texts:
                return None
            nodes = [parse_et(t) for t in texts]
            chain.append(nodes)
            rn = parent_of(nodes[0])
        out = {"slots": {}, "items": {}, "variants": {}, "variant_lines": None}
        for nodes in reversed(chain):
            for node in nodes:
                comps = child(node, "components")
                loadout = child(child(comps, "BaseLoadoutManagerComponent"), "Slots")
                for slot in loadout["c"] if loadout else []:
                    name = slot["h"].split()[-1]
                    out["slots"].setdefault(name, "")
                    if prop(slot, "Prefab") is not None:
                        out["slots"][name] = prop(slot, "Prefab")
                inv = child(child(comps, "SCR_InventoryStorageManagerComponent"), "InitialInventoryItems")
                for item in inv["c"] if inv else []:
                    m = re.search(r'"(\{[0-9A-F]{16}\})"', item["h"])
                    if not m:
                        continue
                    entry = out["items"].setdefault(m.group(1), ["", []])
                    if prop(item, "TargetStorage") is not None:
                        entry[0] = prop(item, "TargetStorage")
                    spawn = child(item, "PrefabsToSpawn")
                    if spawn is not None:
                        entry[1] = (entry[1] if spawn["h"].endswith("+") else []) + strings(spawn)
                editable = child(comps, EDITABLE.split()[0])
                data = child(editable, "m_VariantData")
                if data is not None:
                    out["variant_lines"] = (editable["h"], data["h"][len("m_VariantData "):])
                variants = child(data, "m_aVariants")
                for v in variants["c"] if variants else []:
                    m = re.search(r'"(\{[0-9A-F]{16}\})"', v["h"])
                    if not m:
                        continue
                    entry = out["variants"].setdefault(m.group(1), ["", True])
                    if prop(v, "m_sVariantPrefab") is not None:
                        entry[0] = prop(v, "m_sVariantPrefab")
                    if prop(v, "m_bEnabled") is not None:
                        entry[1] = prop(v, "m_bEnabled") != "0"
        return out


def game_data():
    """--game <folder> and --mods <folder> (or RZ_GAME_DIR and RZ_MODS_DIR), else None."""
    found = {"--game": os.environ.get("RZ_GAME_DIR", ""), "--mods": os.environ.get("RZ_MODS_DIR", "")}
    args = sys.argv[1:]
    for flag in found:
        if flag in args and args.index(flag) + 1 < len(args):
            found[flag] = args[args.index(flag) + 1]
    if not found["--game"]:
        return None
    return GameData(found["--game"], found["--mods"])


def check_game(game):
    """Against the game's own data: every ResourceName a look uses resolves by its GUID, every slot it
    sets is on its parent, every item list packed into a garment it changes moves with it, and the
    VARIANTS table is the parents' real variants."""
    errors = []
    for short, rn in sorted(I.items()):
        if rn and not game.resolves(rn):
            known = ", ".join("{%s}" % g for g in game.guids(rn_path(rn)))
            errors.append("garment %s: %s doesn't resolve (the game has that path as %s)"
                          % (short, rn, known if known else "nothing"))
    views = {}
    for key, (rn, _, packed) in sorted(PARENTS.items()):
        if not game.resolves(rn):
            errors.append("parent %s: %s doesn't resolve" % (key, rn))
            continue
        if packed is None:
            continue
        view = game.view(rn)
        if view is None:
            errors.append("parent %s: a prefab of its chain can't be read (a Workshop mod missing?)" % key)
            continue
        views[key] = view
        on = sorted(vid for vid, (_, enabled) in view["variants"].items() if enabled)
        table = VARIANTS.get(key)
        if on and (not table or sorted(table[2]) != on or (table[0], table[1]) != view["variant_lines"]):
            errors.append("parent %s has random variants %s (%s): put them in VARIANTS"
                          % (key, on, ", ".join(view["variants"][v][0] for v in on)))
        if table and not on:
            errors.append("parent %s has no random variant: take it out of VARIANTS" % key)
    for folder, stem, parent, slots, moves in CHARS:
        view = views.get(parent)
        if view is None:
            continue
        worn = dict(slots)
        for item in slots.values():
            if item in BLOCKS_PANTS:
                worn["Pants"] = ""
        for slot in sorted(worn):
            if slot not in view["slots"]:
                errors.append("%s sets %s, a slot its parent hasn't (a Workshop mod missing?)" % (stem, slot))
        # Every item list the parent packs into a garment the look changes must move with it
        packed = PARENTS[parent][2] or {}
        for cid, (target, items) in sorted(view["items"].items()):
            slot = target.split("/")[0]
            if not items or slot not in worn or I.get(worn[slot]) == view["slots"].get(slot):
                continue
            if cid not in packed.get(slot, []):
                errors.append("%s changes %s, but its parent's items %s (in %s) don't move: add them to PARENTS[%r]"
                              % (stem, slot, cid, target, parent))
    return errors


# ------------------------------------------------------------------------------------------ spawn paths
SCRIPTS = os.path.join(ROOT, "scripts", "Game", "ReforgerZ")
GAME_MODE_CODE = os.path.join(SCRIPTS, "Core", "RZ_GameModeComponent.c")
# A call that puts a body in the world, or hands a prefab on to one that does
SPAWN_CALL = re.compile(r"\b(\w*Spawn\w*|Place|WrapInGroup)\s*\(")


def strip_code(text):
    """Comments and string contents out; line breaks kept, so line numbers still match."""
    text = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"), text, flags=re.S)
    text = re.sub(r'"(\\.|[^"\\\n])*"', '""', text)
    return re.sub(r"//[^\n]*", "", text)


def functions(text):
    """(class, function, parameters, body, body offset) for every function of every class."""
    found = []
    cls = None
    depth = 0
    pending = None
    start = None
    last = 0
    for m in re.finditer(r"[{}]", text):
        before = text[last:m.start()]
        last = m.end()
        if m.group(0) == "{":
            if depth == 0:
                c = re.search(r"\bclass\s+(\w+)[^;{}]*$", before)
                cls = c.group(1) if c else None
            elif depth == 1:
                f = re.search(r"\b(\w+)\s*\(([^;{}]*)\)\s*$", before)
                start = (f.group(1), f.group(2), m.end()) if f else None
            depth += 1
        else:
            depth -= 1
            if depth == 1 and start:
                name, params, at = start
                found.append((cls, name, params, text[at:m.start()], at))
                start = None
    return found


def slot_fields(code):
    """RegisterLooks: the faction person slots, as {(owner class, slot field)}, and the
    (settings block, slot field, looks field) of each m_Looks.Add."""
    body = re.search(r"void RegisterLooks\(\)\s*\{(.*?)\n\t\}", code, re.S)
    if not body:
        return set(), []
    local = dict((v, (t, b)) for t, v, b in re.findall(r"(\w+) (\w+) = (m_\w+Settings);", body.group(1)))
    fields = set()
    pairs = []
    for recv, field, looks in re.findall(r"m_Looks\.Add\((?:(\w+)\.)?(\w+), (?:\w+\.)?(\w+)\)", body.group(1)):
        owner, block = "RZ_GameModeComponent", "(game mode)"
        if recv in local:
            owner, block = local[recv]
        elif recv:
            decl = re.search(r"\b(RZ_\w+) %s;" % recv, code)
            owner, block = (decl.group(1) if decl else "?"), recv
        fields.add((owner, field))
        pairs.append((block, field, looks))
    return fields, pairs


def scan_spawns():
    """Every spawn of a faction person slot goes through PickLook, or through a helper that calls it
    on the prefab it's given. A slot source is a slot field, a function that returns one, or a local
    set from one."""
    scripts = {}
    for dirpath, _, names in os.walk(SCRIPTS):
        for f in sorted(names):
            if f.endswith(".c"):
                path = os.path.join(dirpath, f)
                scripts[path] = strip_code(open(path, encoding="utf-8").read())
    sources, _ = slot_fields(open(GAME_MODE_CODE, encoding="utf-8").read())
    bases = {}
    classes = set()
    for text in scripts.values():
        for c, b in re.findall(r"\bclass (\w+)(?:\s*:\s*(\w+))?", text):
            classes.add(c)
            if b:
                bases[c] = b

    def kin(a, b):
        for x, y in ((a, b), (b, a)):
            while x:
                if x == y:
                    return True
                x = bases.get(x)
        return False

    def type_of(recv, body, text):
        if recv in classes:
            return recv
        for where in (body, text):
            d = re.search(r"\b([A-Z]\w+)\s+%s\s*[;=,)]" % re.escape(recv), where)
            if d:
                return d.group(1)
        return None

    def uses(expr, cls, body, text, tainted):
        """The slot sources an expression reads."""
        hits = []
        for owner, name in sources:
            if name not in expr:
                continue
            for m in re.finditer(r"(?:(\w+)\s*(?:\(\))?\s*\.\s*)?\b%s\b" % name, expr):
                if m.group(1):
                    t = type_of(m.group(1), body, text)
                    if t is None or kin(t, owner):
                        hits.append(name)
                elif cls and kin(cls, owner):
                    hits.append(name)
        for var in tainted:
            if re.search(r"(?<![.\w])%s\b" % var, expr):
                hits.append(var)
        return hits

    def mentions(body):
        return any(name in body for _, name in sources)

    def tainted_in(cls, body, text):
        tainted = set()
        if not mentions(body):
            return tainted
        for _ in range(3):
            for stmt in body.split(";"):
                if "=" not in stmt:
                    continue
                a = re.search(r"\b(\w+)\s*=\s*([^=].*)$", stmt.strip(), re.S)
                if a and "PickLook" not in a.group(2) and uses(a.group(2), cls, body, text, tainted):
                    tainted.add(a.group(1))
        return tainted

    funcs = [(path, cls, name, params, body, at) for path, text in scripts.items()
             for cls, name, params, body, at in functions(text)]
    # Getters: functions that return a source, to a fixed point
    grown = True
    while grown:
        grown = False
        for path, cls, name, params, body, at in funcs:
            if not cls or (cls, name) in sources or not mentions(body):
                continue
            tainted = tainted_in(cls, body, scripts[path])
            for ret in re.findall(r"\breturn\b([^;]*);", body):
                if "PickLook" not in ret and uses(ret, cls, body, scripts[path], tainted):
                    sources.add((cls, name))
                    grown = True
                    break
    # Helpers that dress the prefab they're given: a ResourceName parameter goes into PickLook
    helpers = set()
    for path, cls, name, params, body, at in funcs:
        for p in re.findall(r"\bResourceName\s+(\w+)", params):
            if re.search(r"PickLook\([^;]*\b%s\b" % p, body):
                helpers.add(name)
    errors = []
    for path, cls, name, params, body, at in funcs:
        if (cls, name) in sources or name in helpers or not mentions(body):
            continue
        text = scripts[path]
        tainted = tainted_in(cls, body, text)
        offset = at
        for stmt in body.split(";"):
            here = offset
            offset += len(stmt) + 1
            if "PickLook" in stmt or not [c for c in SPAWN_CALL.findall(stmt) if c not in helpers]:
                continue
            hit = uses(stmt, cls, body, text, tainted)
            if hit:
                line = text[:here + SPAWN_CALL.search(stmt).start()].count("\n") + 1
                errors.append("%s:%d %s.%s spawns a faction person slot (%s) without PickLook"
                              % (os.path.relpath(path, ROOT).replace(os.sep, "/"), line, cls, name,
                                 ", ".join(sorted(set(hit)))))
    return errors


# ------------------------------------------------------------------------------------------ the check
def check():
    """Exit 1 if any faction person can come out in anything but a faction look."""
    errors = []
    files, made = build()
    chars = set(made[stem] for _, stem, *_ in CHARS)
    # 1. The prefabs on disk are what the tables write
    for path, text in sorted(files.items()):
        full = os.path.join(ROOT, path)
        if not os.path.exists(full) or open(full, encoding="utf-8").read() != text:
            errors.append("%s isn't what the tables write: run py tools/people_looks.py" % path)
    for rel in looks_on_disk():
        if rel not in files:
            errors.append("%s isn't in the tables: run py tools/people_looks.py" % rel)
    # 2. Every faction group lists character looks only, its first (leader) slot included
    groups = set(path for path, _ in BASES)
    groups |= set(resource(f, st)[1] for f, st, *_ in GROUPS)
    groups |= set(rn_path(rn) for rn, _, packed in PARENTS.values() if packed is None)
    for path in sorted(groups):
        node = parse_et(open(os.path.join(ROOT, path), encoding="utf-8").read())
        members = strings(child(node, "m_aUnitPrefabSlots"))
        if not members:
            errors.append("%s: no members" % path)
        for m in members:
            if m not in chars:
                errors.append("%s lists %s, not a faction look" % (path, m))
            elif disk_guid(rn_path(m)) != rn_guid(m):
                errors.append("%s lists %s, whose .meta has another GUID" % (path, m))
    # 3. No look can be swapped for a random variant of its parent
    for folder, stem, parent, slots, moves in CHARS:
        if parent not in VARIANTS:
            continue
        node = parse_et(files[resource(folder, stem)[1]])
        data = child(child(child(node, "components"), EDITABLE.split()[0]), "m_VariantData")
        off = set()
        for v in child(data, "m_aVariants")["c"] if child(data, "m_aVariants") else []:
            if prop(v, "m_bEnabled") == "0":
                off.add(re.search(r'"(\{[0-9A-F]{16}\})"', v["h"]).group(1))
        if off != set(VARIANTS[parent][2]):
            errors.append("%s leaves its parent's random variants on" % stem)
    # 4. The game mode registers every slot's looks, under the slot's own prefab
    _, pairs = slot_fields(open(GAME_MODE_CODE, encoding="utf-8").read())
    registered = dict(((block, looks), field) for block, field, looks in pairs)
    gm = parse_et(open(os.path.join(ROOT, "Prefabs/ReforgerZ/Core/RZ_GameMode.et"), encoding="utf-8").read())
    comp = child(child(gm, "components"), "RZ_GameModeComponent")
    for block, field, parent, prefix in ARRAYS:
        append = field.endswith(" (append)")
        field = field.replace(" (append)", "")
        holder = comp if block == "(game mode)" else child(comp, block)
        listed = strings(child(holder, field))
        want = [made[s] for s in made if s.startswith(prefix)]
        if append:
            missing = [w for w in want if w not in listed]
            if missing:
                errors.append("RZ_GameMode.et %s.%s misses %s" % (block, field, ", ".join(missing)))
            continue
        if sorted(listed) != sorted(want):
            errors.append("RZ_GameMode.et %s.%s isn't the %s* looks: paste people_looks.py --arrays" % (block, field, prefix))
        slot = registered.get((block, field))
        if not slot:
            errors.append("RegisterLooks doesn't register %s.%s" % (block, field))
            continue
        value = prop(holder, slot) if holder else None
        if value != PARENTS[parent][0]:
            errors.append("RZ_GameMode.et %s.%s is %s, not %s: PickLook would find no looks for it"
                          % (block, slot, value, PARENTS[parent][0]))
    # 5. Every spawn of a slot picks a look
    errors += scan_spawns()
    # 6. Against the game's own data, when it's given
    game = game_data()
    if game:
        errors += check_game(game)
        where = "read against the game and %d Workshop mods" % game.mod_count
    else:
        where = "no game data given (--game, --mods): GUIDs, slots and variants not read"
    for e in errors:
        print("people_looks: " + e)
    print("people_looks --check: %d looks, %d faction groups, %s: %s"
          % (len(made), len(groups), where, "FAILED (%d)" % len(errors) if errors else "OK"))
    return 1 if errors else 0


if __name__ == "__main__":
    main()
