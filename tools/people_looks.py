#!/usr/bin/env python3
"""ReforgerZ people's looks: writes the faction people's variant prefabs.

    py tools/people_looks.py            # write the prefabs (and their .meta) under Prefabs/ReforgerZ/People/
    py tools/people_looks.py --arrays   # also print the look arrays to paste into RZ_GameMode.et

Every faction person slot (a guard, an occupant, a quartermaster, a crew, a hunter squad, a council
head, an outpost leader) can list variant prefabs; a body picks one by a stable seed
(scripts/Game/ReforgerZ/Factions/RZ_Looks.c, docs/DESIGN.md "People's looks"). This tool writes those
variants as text prefabs:

- a character look inherits the slot's own character prefab (or, for a group slot, the vanilla soldier
  its group spawns) and overrides clothing slots only (Hat, Face, Head, Jacket, Pants, Boots, Vest,
  ArmoredVest, Back, HandWear). Face and Head are Zelik's Character slots, which BanditGear,
  BasicHazmatSuit, MedicalAccessories and Domins bring with them.
- when a look changes a garment the parent's starting items are packed into, those item lists are
  pointed at the new garment (TargetStorage), or at another slot when the garment is gone (a suit
  that covers the legs blocks the trousers).
- a group look inherits the slot's group and lists other character looks as its members.

Faces aren't set: the vanilla identity picks one from the faction's identity config at each spawn.

GUIDs: a new prefab's GUID is derived from its path (stable across runs). Every clothing ResourceName
below is checked against the Workbench resource index ($profile:RZ_ResourceIndex.txt, written by the
plugin "Write resource index"); a name missing from it stops the tool. Set RZ_RESOURCE_INDEX to point
elsewhere, or RZ_RESOURCE_INDEX=none to skip the check. No dependencies beyond Python 3.
"""
import hashlib
import os
import sys

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
    "pasgt_scrim": "{03C1F3DB6BB9796A}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_01_cover_w_scrim.et",
    "pasgt_goggles": "{79D240C0D2390F4A}Prefabs/Characters/HeadGear/Helmet_PASGT_01/Helmet_PASGT_01_goggles.et",
    "patrol_dark": "{7891C761B31741BD}Prefabs/Characters/HeadGear/Hat_Patrol_US_01/Hat_Patrol_US_dark.et",  # [L]
    "boonie_dark": "{1E8E0AF46736D299}Prefabs/Characters/HeadGear/Hat_Boonie_US_01/Hat_Boonie_US_dark.et",  # [L]
    "woodland_cap": "{129457E7C45EB342}Prefabs/Characters/HeadGear/Hat_M70_01/Hat_woodland_cap.et",  # [L]
    "m1_nostrap": "{1D0B60CC90DA7330}Prefabs/Characters/HeadGear/Helmet_M1_01/Helmet_M1_No Strap.et",  # [L]
    "cap_tourist_back": "{BDEC5F396E369CEA}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_Backwards.et",  # [B]
    "cap_plain": "{503CADDC8B459C03}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_02.et",  # [B]
    "cap_plain_back": "{F42EB93651D94D60}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_02_backwards.et",  # [B]
    "cap_trophy": "{B5A0E12587A00B68}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_03.et",  # [B]
    "cap_trophy_back": "{DD43DAD304AC5085}Prefabs/Characters/HeadGear/Baseball_Cap/Baseball_Cap_03_backwards.et",  # [B]
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
    "denim_striped": "{EB0D74A43F74B522}Prefabs/Characters/Uniforms/Jacket_Denim_01/Jacket_Denim_01_strippedShirt.et",
    "denim_striped_dirty": "{23A15812C40D34C2}Prefabs/Characters/Uniforms/Jacket_Denim_01/Jacket_Denim_01_strippedShirt_dirty.et",
    "raincoat_green": "{7F1451790F3D75BD}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_green.et",
    "raincoat_orange": "{690819A92A3A8D14}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_orange.et",
    "raincoat_yellow": "{FD39B0B833BB927E}Prefabs/Characters/Uniforms/Jacket_Raincoat_01/Jacket_Raincoat_01_yellow.et",
    "suit_grey": "{E652D625A46A00FC}Prefabs/Characters/Uniforms/Jacket_Suit_01/Jacket_Suit_01_Grey_V1.et",
    "suit_brown": "{52AB6EC3FE535303}Prefabs/Characters/Uniforms/Jacket_Suit_01/Jacket_Suit_01_Brown_V1.et",
    "suit_black": "{AE93D885FDF713D4}Prefabs/Characters/Uniforms/Jacket_Suit_01/Jacket_Suit_01_black_V1.et",
    "turtle_grey": "{B0F4EAC29E9058EA}Prefabs/Characters/Uniforms/Shirt_Turtleneck_01/Shirt_Turtleneck_01_grey.et",
    "turtle_blue": "{6A3F994EFFE22995}Prefabs/Characters/Uniforms/Shirt_Turtleneck_01/Shirt_Turtleneck_01_DarkBlue.et",
    "turtle_beige": "{50F8B1F928F93ABD}Prefabs/Characters/Uniforms/Shirt_Turtleneck_01/Shirt_Turtleneck_01_beige.et",
    "bdu": "{C7861F11D5334C0E}Prefabs/Characters/Uniforms/Jacket_US_BDU.et",
    "bdu_rolled": "{3CCA7A9BB4FD3197}Prefabs/Characters/Uniforms/Jacket_US_BDU_rolledup.et",
    "bdu_rolled_02": "{A080C1AEBF921DC4}Prefabs/Characters/Uniforms/Jacket_US_BDU_rolledup_02.et",
    "bdu_darklong": "{C890D96D16AD5EEF}Prefabs/Characters/Uniforms/Jacket_US_BDU_darklong.et",  # [L]
    "bdu_rolled_dark": "{28372136A9F0818A}Prefabs/Characters/Uniforms/Jacket_US_BDU_rolledup_dark.et",  # [L]
    "night_camo": "{D24304848E14A064}Prefabs/Characters/Uniforms/Jacket_KZS/Jacket_night_camo.et",  # [L]
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
    "pants_bdu": "{604BB72BE8E023C2}Prefabs/Characters/Uniforms/Pants_US_BDU.et",
    "m65": "{86BB79B9A2233780}Prefabs/Characters/Uniforms/Pants_US_M65.et",
    "m65_dark": "{F2ACE9E8181679BB}Prefabs/Characters/Uniforms/Pants_US_M65_dark.et",  # [L]
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
    "gloves_rally_black": "{EC5F60A22D3C2236}Prefabs/Characters/Handwear/Gloves_Rally_01/Gloves_Rally_01_black.et",
    "gloves_wool_dirty": "{7277FBDCDC013DDF}Prefabs/Characters/Handwear/Gloves_Wool_01/Gloves_Wool_Dirty.et",  # [B]
    "gloves_wool_black": "{3F155F317D83D375}Prefabs/Characters/Handwear/Gloves_Wool_01/Gloves_Wool_Black.et",  # [B]
    "gloves_skeleton": "{7A32D0C6743CCEBE}Prefabs/Characters/Handwear/Gloves_Wool_Skeleton/Gloves_Wool_Skeleton.et",  # [B]
    "gloves_leather_black": "{CDDA2A133111E1A6}Prefabs/Characters/Handwear/Gloves_Leather_01/Gloves_Leather_Black.et",  # [B]
    "gloves_pilot_black": "{EF604A05A5998102}Prefabs/Characters/Handwear/Gloves_Pilot_US_01/Gloves_Pilot_US_Black.et",  # [H]
    "nitrile_blue": "{D8326FEE52670D2A}Prefabs/Characters/Handwear/Gloves_Blue/Medical_Gloves_Blue_Base.et",  # [M]
    "nitrile_purple": "{395D1185E649EDE2}Prefabs/Characters/Handwear/Gloves_Purple/Medical_Gloves_Purple_Base.et",  # [M]
    "nitrile_white": "{D90A181625AD1F9A}Prefabs/Characters/Handwear/Gloves_White/Medical_Gloves_White_Base.et",  # [M]
    "nitrile_yellow": "{4B4ED1DF956F00D9}Prefabs/Characters/Handwear/Gloves_yellow/Medical_Gloves_yellow_Base.et",  # [M]
    # packs
    "veshmeshok": "{3DE0155EC9767B98}Prefabs/Items/Equipment/Backpacks/Backpack_Veshmeshok.et",
    "medpack_nocross_1": "{AC298BA8A7D27007}Prefabs/Items/Equipment/Backpacks/Backpack_Medical_Soviet_NoCross_01.et",
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
    # leaders who stand at more than one place
    "council": ("{70CB458FA07F01CC}Prefabs/ReforgerZ/Leaders/RZ_CouncilHead.et", "520EC961A090B1EE",
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

# ------------------------------------------------------------------------------------------ the looks
# (folder, file stem, parent, {slot: item}, {slot whose items move: to slot}). "" empties a slot.
# Each faction keeps one identity: the Haven in farm and work clothes, Bastion in worn US kit, the
# Red Hand in dirty scavenged clothes and balaclavas, Halcyon in lab clothes, masks and hazmat.
CHARS = [
    # The Haven: occupants (also the safehouses' occupants), armed (guards, crews), quartermasters
    ("Haven", "RZ_Occupant_Haven_Farmer", "occ_haven", {"Hat": "knit_brown", "Jacket": "overalls", "HandWear": "gloves_leather"}, {}),
    ("Haven", "RZ_Occupant_Haven_Dungarees", "occ_haven", {"Hat": "trucker_sawmill", "Jacket": "overalls_denim"}, {}),
    ("Haven", "RZ_Occupant_Haven_Mechanic", "occ_haven", {"Hat": "", "Jacket": "mechanic", "HandWear": "gloves_wool"}, {}),
    ("Haven", "RZ_Occupant_Haven_Fisher", "occ_haven", {"Hat": "knit_blue", "Jacket": "raincoat_yellow", "Pants": "fisher_green", "Boots": "boots_soviet_dirty"}, {}),
    ("Haven", "RZ_Occupant_Haven_Builder", "occ_haven", {"Hat": "hardhat_yellow", "Jacket": "shirt_2", "Pants": "trousers_lightbrown", "Back": "veshmeshok"}, {}),
    ("Haven", "RZ_Occupant_Haven_Carer", "occ_haven", {"Hat": "", "Face": "medmask", "Jacket": "turtle_grey", "Pants": "trousers_darkbrown", "HandWear": "nitrile_white"}, {}),
    ("Haven", "RZ_Armed_Haven_Hunter", "fia_ac", {"Hat": "knit_black", "Jacket": "raincoat_green", "Pants": "fisher_green"}, {}),
    ("Haven", "RZ_Armed_Haven_Farmhand", "fia_ac", {"Hat": "trucker_everon", "Jacket": "denim_brown", "Pants": "trousers_darkbrown"}, {}),
    ("Haven", "RZ_Armed_Haven_Worker", "fia_ac", {"Hat": "hardhat_white", "Jacket": "shirt_3", "Pants": "trousers_darkgrey", "HandWear": "gloves_leather"}, {}),
    ("Haven", "RZ_Armed_Haven_Winter", "fia_ac", {"Hat": "ushanka_folded", "Jacket": "denim_striped", "Back": "veshmeshok"}, {}),
    ("Haven", "RZ_Armed_Haven_Overalls", "fia_ac", {"Hat": "flatcap_blue", "Jacket": "overalls"}, {"Pants": "Back"}),
    ("Haven", "RZ_QM_Haven_Storekeeper", "qm_haven", {"Hat": "flatcap_lightgrey", "Jacket": "shirt_1", "Pants": "trousers_lightbrown"}, {}),
    ("Haven", "RZ_QM_Haven_Dungarees", "qm_haven", {"Hat": "knit_red", "Jacket": "overalls_denim"}, {"Pants": "Jacket"}),
    ("Haven", "RZ_QM_Haven_Raincoat", "qm_haven", {"Hat": "trucker_pohona", "Jacket": "raincoat_orange", "Pants": "fisher_yellow"}, {}),
    ("Haven", "RZ_CouncilHead_Brown", "council", {"Hat": "flatcap_darkgrey", "Jacket": "suit_brown", "Pants": "suitpants_brown"}, {}),
    ("Haven", "RZ_CouncilHead_Denim", "council", {"Jacket": "denim_brown", "Pants": "trousers_darkbrown"}, {}),
    ("Haven", "RZ_CouncilHead_Beret", "council", {"Hat": "beret_brown", "Jacket": "turtle_beige", "Pants": "suitpants_black"}, {}),
    # Task Force Bastion
    ("Bastion", "RZ_Occupant_Bastion_Rolled", "occ_bastion", {"Hat": "patrol_dark", "Jacket": "bdu_rolled_dark", "Pants": "m65_dark"}, {}),
    ("Bastion", "RZ_Occupant_Bastion_Night", "occ_bastion", {"Hat": "woodland_cap", "Jacket": "night_camo"}, {}),
    ("Bastion", "RZ_Occupant_Bastion_Raincoat", "occ_bastion", {"Hat": "", "Jacket": "us_raincoat", "Pants": "m65"}, {}),
    ("Bastion", "RZ_Occupant_Bastion_Long", "occ_bastion", {"Hat": "boonie_dark", "Jacket": "bdu_darklong"}, {}),
    ("Bastion", "RZ_Occupant_Bastion_OffDuty", "occ_bastion", {"Hat": "", "Jacket": "bdu_rolled_02", "Pants": "m65_dark"}, {}),
    ("Bastion", "RZ_Armed_Bastion_Scrim", "us_rifleman", {"Hat": "pasgt_scrim", "Jacket": "bdu_rolled"}, {}),
    ("Bastion", "RZ_Armed_Bastion_M1", "us_rifleman", {"Hat": "m1_nostrap", "Jacket": "bdu_darklong", "Pants": "m65_dark"}, {}),
    ("Bastion", "RZ_Armed_Bastion_Cap", "us_rifleman", {"Hat": "patrol_dark", "Jacket": "bdu_rolled_dark", "ArmoredVest": ""}, {}),
    ("Bastion", "RZ_Armed_Bastion_Night", "us_rifleman", {"Hat": "boonie_olive", "Jacket": "night_camo", "Pants": "m65", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_Armed_Bastion_Raincoat", "us_rifleman", {"Hat": "pasgt_goggles", "Jacket": "us_raincoat"}, {}),
    ("Bastion", "RZ_QM_Bastion_Long", "qm_bastion", {"Hat": "patrol_dark", "Jacket": "bdu_darklong"}, {}),
    ("Bastion", "RZ_QM_Bastion_Woodland", "qm_bastion", {"Hat": "woodland_cap", "Jacket": "bdu", "Pants": "m65_dark"}, {}),
    ("Bastion", "RZ_QM_Bastion_Gloves", "qm_bastion", {"Hat": "", "Jacket": "bdu_rolled_dark", "HandWear": "gloves_pilot"}, {}),
    ("Bastion", "RZ_Outpost_Bastion_Long", "outpost_bastion", {"Hat": "patrol_dark", "Jacket": "bdu_darklong"}, {}),
    ("Bastion", "RZ_Outpost_Bastion_Woodland", "outpost_bastion", {"Hat": "woodland_cap", "Jacket": "bdu", "Pants": "m65_dark"}, {}),
    # The Red Hand
    ("RedHand", "RZ_Occupant_RedHand_Balaclava", "occ_redhand", {"Hat": "", "Face": "balaclava_black", "Jacket": "bandit_1", "Pants": "cargo_black", "HandWear": "gloves_wool_dirty"}, {}),
    ("RedHand", "RZ_Occupant_RedHand_Tracksuit", "occ_redhand", {"Hat": "cap_plain_back", "Jacket": "track_black", "Pants": "trackpants_black", "Boots": "sneakers_dirty"}, {}),
    ("RedHand", "RZ_Occupant_RedHand_Sweatshirt", "occ_redhand", {"Hat": "knit_black", "Jacket": "sweat_dirty", "Pants": "jeans_1", "HandWear": "gloves_skeleton"}, {}),
    ("RedHand", "RZ_Occupant_RedHand_ThreeHole", "occ_redhand", {"Hat": "", "Face": "balaclava3_olive", "Jacket": "bandit_2", "Pants": "cargo_tan", "Boots": "sneakers_black"}, {}),
    ("RedHand", "RZ_Occupant_RedHand_RedTop", "occ_redhand", {"Hat": "cap_trophy", "Jacket": "sweat_red", "Pants": "jeans_2", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Balaclava", "fia_partisan", {"Hat": "", "Face": "balaclava_black", "Jacket": "bandit_1", "Pants": "cargo_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Tracksuit", "fia_partisan", {"Hat": "cap_tourist_back", "Jacket": "track_navy", "Pants": "trackpants_navy", "HandWear": "gloves_wool_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_ThreeHole", "fia_partisan", {"Hat": "", "Face": "balaclava3_black", "Jacket": "sweat_dirty", "Pants": "jeans_1", "Boots": "sneakers_dirty"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Beret", "fia_partisan", {"Hat": "beret_black_dirty", "Jacket": "bandit_2", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_Armed_RedHand_Helmet", "fia_partisan", {"Hat": "pasgt_black", "Jacket": "denim_striped_dirty", "Pants": "cargo_tan"}, {}),
    ("RedHand", "RZ_QM_RedHand_Cap", "qm_redhand", {"Hat": "cap_plain", "Jacket": "bandit_1", "HandWear": "gloves_leather_black"}, {}),
    ("RedHand", "RZ_QM_RedHand_Balaclava", "qm_redhand", {"Hat": "", "Face": "balaclava_black", "Jacket": "track_black", "Pants": "trackpants_black"}, {}),
    ("RedHand", "RZ_QM_RedHand_Beret", "qm_redhand", {"Hat": "beret_black_dirty", "Jacket": "sweat_red", "Pants": "jeans_2"}, {}),
    ("RedHand", "RZ_Outpost_RedHand_Balaclava", "outpost_redhand", {"Hat": "", "Face": "balaclava_olive", "Jacket": "bandit_2"}, {}),
    ("RedHand", "RZ_Outpost_RedHand_Tracksuit", "outpost_redhand", {"Hat": "cap_trophy_back", "Jacket": "track_navy", "Pants": "trackpants_navy"}, {}),
    # Halcyon
    ("Halcyon", "RZ_Occupant_Halcyon_Beak", "occ_halcyon", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_olive", "HandWear": "nitrile_blue"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_BlackSuit", "occ_halcyon", {"Hat": "", "Face": "hazmask_tinted", "Jacket": "hazmat_black", "HandWear": "gloves_pilot_black"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Technician", "occ_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "turtle_grey", "Pants": "suitpants_grey", "HandWear": "nitrile_purple"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Paramedic", "occ_halcyon", {"Hat": "", "Jacket": "ems_jacket", "Pants": "ems_pants", "Boots": "boots_paramedic", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_Researcher", "occ_halcyon", {"Hat": "", "Head": "gasmask", "Jacket": "suit_grey", "Pants": "suitpants_black", "HandWear": "nitrile_white"}, {}),
    ("Halcyon", "RZ_Occupant_Halcyon_HiVis", "occ_halcyon", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_hivis", "HandWear": "nitrile_yellow"}, {}),
    ("Halcyon", "RZ_Armed_Halcyon_GasMask", "us_rifleman_v1", {"Hat": "", "Head": "gasmask", "Jacket": "bdu_darklong", "HandWear": "gloves_pilot_black"}, {}),
    ("Halcyon", "RZ_Armed_Halcyon_Hazmat", "us_rifleman_v1", {"Hat": "", "Face": "hazmask", "Jacket": "hazmat_olive", "ArmoredVest": ""}, {"Pants": "Vest"}),
    ("Halcyon", "RZ_Armed_Halcyon_Masked", "us_rifleman_v1", {"Hat": "", "Face": "medmask", "Jacket": "turtle_blue", "Pants": "m65_dark", "HandWear": "gloves_rally_black"}, {}),
    ("Halcyon", "RZ_Armed_Halcyon_BlackSuit", "us_rifleman_v1", {"Hat": "", "Face": "hazmask_tinted", "Jacket": "hazmat_black", "ArmoredVest": "", "HandWear": "gloves_pilot_black"}, {"Pants": "Vest"}),
    ("Halcyon", "RZ_Armed_Halcyon_Medic", "us_rifleman_v1", {"Hat": "", "Face": "medmask", "Jacket": "ems_jacket", "Pants": "ems_pants"}, {}),
    ("Halcyon", "RZ_QM_Halcyon_Coat", "qm_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "suit_grey", "Pants": "suitpants_grey"}, {}),
    ("Halcyon", "RZ_QM_Halcyon_Paramedic", "qm_halcyon", {"Hat": "", "Jacket": "ems_jacket", "Pants": "ems_pants", "HandWear": "nitrile_blue"}, {}),
    ("Halcyon", "RZ_QM_Halcyon_GasMask", "qm_halcyon", {"Hat": "", "Head": "gasmask", "Jacket": "turtle_grey", "Pants": "suitpants_black"}, {}),
    ("Halcyon", "RZ_Outpost_Halcyon_Paramedic", "outpost_halcyon", {"Hat": "", "Jacket": "ems_jacket", "Pants": "ems_pants"}, {}),
    ("Halcyon", "RZ_Outpost_Halcyon_Coat", "outpost_halcyon", {"Hat": "", "Face": "medmask", "Jacket": "suit_black", "Pants": "suitpants_black"}, {}),
]

# (folder, file stem, parent group, [member stems]): no two groups of a slot share their mix.
GROUPS = [
    ("Haven", "RZ_Guard_Haven_1", "grp_guard_haven", ["RZ_Armed_Haven_Hunter", "RZ_Armed_Haven_Farmhand"]),
    ("Haven", "RZ_Guard_Haven_2", "grp_guard_haven", ["RZ_Armed_Haven_Worker", "RZ_Armed_Haven_Winter"]),
    ("Haven", "RZ_Guard_Haven_3", "grp_guard_haven", ["RZ_Armed_Haven_Overalls", "RZ_Armed_Haven_Worker"]),
    ("Haven", "RZ_SafehouseGuard_1", "grp_safehouse_guard", ["RZ_Armed_Haven_Farmhand"]),
    ("Haven", "RZ_SafehouseGuard_2", "grp_safehouse_guard", ["RZ_Armed_Haven_Winter"]),
    ("Haven", "RZ_SafehouseGuard_3", "grp_safehouse_guard", ["RZ_Armed_Haven_Overalls"]),
    ("Haven", "RZ_TruckCrew_Haven_1", "grp_truck_haven", ["RZ_Armed_Haven_Farmhand", "RZ_Armed_Haven_Winter"]),
    ("Haven", "RZ_TruckCrew_Haven_2", "grp_truck_haven", ["RZ_Armed_Haven_Overalls", "RZ_Armed_Haven_Hunter"]),
    ("Bastion", "RZ_Guard_Bastion_1", "grp_guard_bastion", ["RZ_Armed_Bastion_Scrim", "RZ_Armed_Bastion_M1"]),
    ("Bastion", "RZ_Guard_Bastion_2", "grp_guard_bastion", ["RZ_Armed_Bastion_Cap", "RZ_Armed_Bastion_Night"]),
    ("Bastion", "RZ_Guard_Bastion_3", "grp_guard_bastion", ["RZ_Armed_Bastion_Raincoat", "RZ_Armed_Bastion_Scrim"]),
    ("Bastion", "RZ_BastionPostCrew_1", "grp_post_bastion", ["RZ_Armed_Bastion_Scrim", "RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Cap"]),
    ("Bastion", "RZ_BastionPostCrew_2", "grp_post_bastion", ["RZ_Armed_Bastion_Night", "RZ_Armed_Bastion_Raincoat", "RZ_Armed_Bastion_Scrim"]),
    ("Bastion", "RZ_BastionPostCrew_3", "grp_post_bastion", ["RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Cap", "RZ_Armed_Bastion_Raincoat"]),
    ("Bastion", "RZ_BastionHunters_1", "grp_hunters_bastion", ["RZ_Armed_Bastion_Scrim", "RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Cap", "RZ_Armed_Bastion_Night"]),
    ("Bastion", "RZ_BastionHunters_2", "grp_hunters_bastion", ["RZ_Armed_Bastion_Raincoat", "RZ_Armed_Bastion_Night", "RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Scrim"]),
    ("Bastion", "RZ_TruckCrew_Bastion_1", "grp_truck_bastion", ["RZ_Armed_Bastion_M1", "RZ_Armed_Bastion_Raincoat"]),
    ("Bastion", "RZ_TruckCrew_Bastion_2", "grp_truck_bastion", ["RZ_Armed_Bastion_Night", "RZ_Armed_Bastion_Cap"]),
    ("RedHand", "RZ_Guard_RedHand_1", "grp_guard_redhand", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit"]),
    ("RedHand", "RZ_Guard_RedHand_2", "grp_guard_redhand", ["RZ_Armed_RedHand_ThreeHole", "RZ_Armed_RedHand_Beret"]),
    ("RedHand", "RZ_Guard_RedHand_3", "grp_guard_redhand", ["RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_Tracksuit"]),
    ("RedHand", "RZ_RedHandPostCrew_1", "grp_post_redhand", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_ThreeHole"]),
    ("RedHand", "RZ_RedHandPostCrew_2", "grp_post_redhand", ["RZ_Armed_RedHand_Beret", "RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_Balaclava"]),
    ("RedHand", "RZ_RedHandPostCrew_3", "grp_post_redhand", ["RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_ThreeHole", "RZ_Armed_RedHand_Helmet"]),
    ("RedHand", "RZ_RedHandHunters_1", "grp_hunters_redhand", ["RZ_Armed_RedHand_Balaclava", "RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_ThreeHole", "RZ_Armed_RedHand_Beret"]),
    ("RedHand", "RZ_RedHandHunters_2", "grp_hunters_redhand", ["RZ_Armed_RedHand_Helmet", "RZ_Armed_RedHand_Beret", "RZ_Armed_RedHand_ThreeHole", "RZ_Armed_RedHand_Balaclava"]),
    ("RedHand", "RZ_TruckCrew_RedHand_1", "grp_truck_redhand", ["RZ_Armed_RedHand_Tracksuit", "RZ_Armed_RedHand_Helmet"]),
    ("RedHand", "RZ_TruckCrew_RedHand_2", "grp_truck_redhand", ["RZ_Armed_RedHand_Beret", "RZ_Armed_RedHand_Balaclava"]),
    ("Halcyon", "RZ_Guard_Halcyon_1", "grp_guard_halcyon", ["RZ_Armed_Halcyon_GasMask", "RZ_Armed_Halcyon_Hazmat"]),
    ("Halcyon", "RZ_Guard_Halcyon_2", "grp_guard_halcyon", ["RZ_Armed_Halcyon_Masked", "RZ_Armed_Halcyon_BlackSuit"]),
    ("Halcyon", "RZ_Guard_Halcyon_3", "grp_guard_halcyon", ["RZ_Armed_Halcyon_Medic", "RZ_Armed_Halcyon_GasMask"]),
    ("Halcyon", "RZ_TruckCrew_Halcyon_1", "grp_truck_halcyon", ["RZ_Armed_Halcyon_Medic", "RZ_Armed_Halcyon_Masked"]),
    ("Halcyon", "RZ_TruckCrew_Halcyon_2", "grp_truck_halcyon", ["RZ_Armed_Halcyon_Hazmat", "RZ_Armed_Halcyon_Medic"]),
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
    lines = ['SCR_ChimeraCharacter : "%s" {' % parent, ' ID "%s"' % entity_id, " components {",
             '  BaseLoadoutManagerComponent "{520EA1D2DB11821E}" {', "   Slots {"]
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


def main():
    check_index([v for v in I.values()] + [PARENTS[k][0] for k in PARENTS if not PARENTS[k][0].split("}")[1].startswith("Prefabs/ReforgerZ")])
    made = {}
    for folder, stem, parent, slots, moves in CHARS:
        rn, path = resource(folder, stem)
        write(path, char_text(parent, slots, moves))
        write(path + ".meta", META % rn)
        made[stem] = rn
    for folder, stem, parent, members in GROUPS:
        rn, path = resource(folder, stem)
        for m in members:
            if m not in made:
                sys.exit("people_looks: group %s lists unknown look %s" % (stem, m))
        write(path, group_text(parent, [made[m] for m in members]))
        write(path + ".meta", META % rn)
        made[stem] = rn
    print("people_looks: wrote %d prefabs under %s" % (len(made), OUT))
    if "--arrays" in sys.argv:
        for block, field, _, prefix in ARRAYS:
            names = [made[s] for s in made if s.startswith(prefix)]
            print("%s.%s {\n %s\n}" % (block, field, " ".join('"%s"' % n for n in names)))


if __name__ == "__main__":
    main()
