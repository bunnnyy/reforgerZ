//! The loot pool's shared groups and each building kind's mix of them (docs/DESIGN.md, "Loot in
//! houses", "Groups and mixes"). Everyday finds are picked in two steps: a group by the kind's
//! weights (food 4, drink 2, clothes 3...), then an item of that group. Food, drink and the low
//! medicine are shared: every kind that draws them draws from one list (every food item any table
//! or the pool has), so a mod's new food reaches every kind at once. The other groups are the
//! kind's own items of that group (a garage's tools, a police station's ammunition).
//!
//! Groups are read from the prefab's path and name (GroupOf). Medicine has two levels: the high
//! level (morphine, saline, first-aid kits) only at a hospital or a locked pharmacy (MedHigh).
//! The rare things capped across the map (RZ_Loot's caps) are named by CapKind.

//------------------------------------------------------------------------------------------------
//! One kind's everyday mix, set by hand (Interactions > Search > Loot mixes). Empty: the built-in mix.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sType")]
class RZ_LootMix
{
	[Attribute("house", desc: "Building kind: house, shop, farm, pharmacy, hospital, garage, firestation, police, school, office, church, industrial, hunting, coast, prison, army, body, car, drone or evac")]
	string m_sType;

	[Attribute("", desc: "Everyday mix: group and weight pairs, e.g. \"food 4, drink 2, clothes 3, medlow 1\". Groups: food, drink, medlow, medhigh, clothes, tools, gear, fence, electronics, ammo, armour, weapon. A group the kind has no item of is skipped")]
	string m_sMix;
}

//------------------------------------------------------------------------------------------------
class RZ_LootGroups
{
	// The groups, in the order a prefab is tested (GroupOf)
	static const string FOOD = "food";
	static const string DRINK = "drink";
	static const string MED_LOW = "medlow";
	static const string MED_HIGH = "medhigh";
	static const string CLOTHES = "clothes";
	static const string TOOLS = "tools";
	static const string GEAR = "gear";
	static const string FENCE = "fence";
	static const string ELECTRONICS = "electronics";
	static const string AMMO = "ammo";
	static const string ARMOUR = "armour";
	static const string WEAPON = "weapon";

	// The capped categories (RZ_Loot's map-wide caps)
	static const string CAP_RIFLE = "rifle";
	static const string CAP_PISTOL = "pistol";
	static const string CAP_ARMOUR = "armour";
	static const string CAP_TREATMENT = "treatment";

	protected static ref map<string, string> s_mCache;

	//------------------------------------------------------------------------------------------------
	//! Every building kind loose loot knows, in BuildingType's order of checks (the tables built at
	//! setup; "body", "car", "drone" and "evac" aren't buildings).
	static void Kinds(notnull array<string> into)
	{
		array<string> kinds = {"pharmacy", "hospital", "firestation", "garage", "police", "prison", "army", "school", "office", "church", "industrial", "coast", "hunting", "farm", "shop", "house", "body", "car", "drone", "evac"};
		foreach (string k : kinds)
		{
			into.Insert(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A prefab's group, from its lower-case path ("" never: anything else is gear).
	static string GroupOf(ResourceName prefab)
	{
		if (!s_mCache)
			s_mCache = new map<string, string>();

		string key = prefab;
		string found;
		if (s_mCache.Find(key, found))
			return found;

		string p = prefab;
		p.ToLower();
		found = Classify(p);
		s_mCache.Set(key, found);
		return found;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Classify(string p)
	{
		if (p.Contains("/weapons/rifles/") || p.Contains("/weapons/handguns/") || p.Contains("/weapons/shotguns/") || p.Contains("/weapons/machineguns/") || p.Contains("/weapons/launchers/"))
			return WEAPON;

		if (p.Contains("/weapons/magazines/") || p.Contains("/weapons/grenades/") || p.Contains("ammo"))
			return AMMO;

		if (IsArmour(p))
			return ARMOUR;

		if (p.Contains("morphine") || p.Contains("saline") || p.Contains("medicalkit") || p.Contains("medkit") || p.Contains("antibiotic") || p.Contains("epinephrine") || p.Contains("adrenaline"))
			return MED_HIGH;

		if (p.Contains("suspicious items") || p.Contains("/watches/") || p.Contains("personalbelongings") || p.Contains("cigarette") || p.Contains("goldbrick") || p.Contains("stackofcash") || p.Contains("creditcard") || p.Contains("necklace"))
			return FENCE;

		if (p.Contains("/food/") || p.Contains("rz_stew") || p.Contains("rz_salt"))
			return FOOD;

		if (p.Contains("/water/") || p.Contains("canteen") || p.Contains("waterbottle") || p.Contains("vodka"))
			return DRINK;

		if (p.Contains("/medicine/") || p.Contains("dressing") || p.Contains("gauze") || p.Contains("bandage") || p.Contains("aspirin") || p.Contains("tourniquet") || p.Contains("bleach") || p.Contains("iodine") || p.Contains("purification"))
			return MED_LOW;

		if (p.Contains("/radios/") || p.Contains("radio") || p.Contains("/flashlights/") || p.Contains("battery") || p.Contains("rz_cells"))
			return ELECTRONICS;

		if (p.Contains("/characters/uniforms/") || p.Contains("/characters/headgear/") || p.Contains("/characters/handwear/") || p.Contains("/characters/footwear/") || p.Contains("/characters/waist/") || p.Contains("/clothing/") || p.Contains("/characters/vests/"))
			return CLOTHES;

		if (p.Contains("/equipment/accessories/") || p.Contains("/equipment/kits/repairkit") || p.Contains("/fuel/") || p.Contains("/melee/") || p.Contains("woodstick") || p.Contains("rz_wire") || p.Contains("rz_nails") || p.Contains("rz_scrap") || p.Contains("rz_fuse") || p.Contains("rz_tincan") || p.Contains("rz_planks") || p.Contains("rope") || p.Contains("toolbox") || p.Contains("crowbar"))
			return TOOLS;

		return GEAR;
	}

	//------------------------------------------------------------------------------------------------
	//! A vest or a helmet that stops something (a firefighter's or a hard hat doesn't count).
	protected static bool IsArmour(string p)
	{
		if (p.Contains("firefighter") || p.Contains("hat_hard") || p.Contains("paramedic"))
			return false;

		if (p.Contains("/characters/vests/vest_pasgt") || p.Contains("/characters/vests/vest_6b") || p.Contains("platecarrier") || p.Contains("/characters/vests/vest_m69"))
			return true;

		return p.Contains("/characters/headgear/helmet_");
	}

	//------------------------------------------------------------------------------------------------
	//! The high level of medicine: only at a hospital or a locked pharmacy.
	static bool IsMedHigh(ResourceName prefab)
	{
		return GroupOf(prefab) == MED_HIGH;
	}

	//------------------------------------------------------------------------------------------------
	//! May this lie in a building of this kind? The high medicine only at a hospital or a locked
	//! pharmacy (and the downed drone, the evacuation road's boots: neither has any).
	static bool AllowedAt(ResourceName prefab, string kind, bool locked)
	{
		if (!IsMedHigh(prefab))
			return true;

		return kind == "hospital" || (kind == "pharmacy" && locked) || kind == "evac";
	}

	//------------------------------------------------------------------------------------------------
	//! The map-wide cap a prefab counts against: rifle, pistol, armour, or "" for none.
	static string CapKind(ResourceName prefab)
	{
		string p = prefab;
		p.ToLower();
		if (p.Contains("/weapons/rifles/") || p.Contains("/weapons/shotguns/") || p.Contains("/weapons/machineguns/"))
			return CAP_RIFLE;

		if (p.Contains("/weapons/handguns/"))
			return CAP_PISTOL;

		if (IsArmour(p))
			return CAP_ARMOUR;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A kind's built-in everyday mix ("group weight, ..."). Food, drink and the low medicine are
	//! drawn from the shared lists; the rest from the kind's own items.
	static string DefaultMix(string kind)
	{
		switch (kind)
		{
			case "house": return "food 4, drink 2, medlow 2, clothes 3, gear 1, tools 1, fence 1";
			case "shop": return "food 5, drink 3, gear 2, clothes 1, fence 2, tools 1";
			case "farm": return "food 4, drink 2, tools 3, clothes 2, gear 1";
			case "pharmacy": return "medlow 6, clothes 2, gear 1";
			case "hospital": return "medlow 5, medhigh 2, clothes 2, gear 1";
			case "garage": return "tools 5, clothes 2, drink 1, gear 1";
			case "firestation": return "tools 3, clothes 3, gear 1";
			case "police": return "ammo 4, clothes 2, medlow 1, gear 2";
			case "school": return "food 3, drink 2, gear 3, clothes 1";
			case "office": return "gear 4, fence 3, food 1, drink 1";
			case "church": return "food 2, drink 2, gear 1";
			case "industrial": return "tools 6, clothes 2, drink 1";
			case "hunting": return "gear 3, tools 2, clothes 2, food 1, drink 1";
			case "coast": return "clothes 3, tools 2, food 2, drink 1";
			case "prison": return "clothes 3, food 2";
			case "army": return "ammo 4, clothes 3, food 2, drink 2, gear 1";
			case "body": return "fence 2, medlow 2, ammo 2, food 1, clothes 1, tools 1";
			case "car": return "tools 4, drink 2, food 1, gear 1";
			case "drone": return "electronics 4, tools 3";
			case "evac": return "food 4, drink 3, medlow 2, gear 1";
		}

		return "food 4, drink 2, medlow 2, clothes 3, gear 1, tools 1";
	}

	//------------------------------------------------------------------------------------------------
	//! Food, drink and the low medicine come from the shared lists.
	static bool IsShared(string group)
	{
		return group == FOOD || group == DRINK || group == MED_LOW;
	}

	//------------------------------------------------------------------------------------------------
	//! "food 4, drink 2" into its groups and weights (a pair without a number weighs 1).
	static void ParseMix(string mix, notnull array<string> groups, notnull array<float> weights)
	{
		array<string> parts = {};
		mix.Split(",", parts, true);
		foreach (string part : parts)
		{
			string t = part.Trim();
			if (t.IsEmpty())
				continue;

			array<string> words = {};
			t.Split(" ", words, true);
			if (words.IsEmpty())
				continue;

			string g = words[0];
			g.ToLower();
			float w = 1;
			if (words.Count() > 1)
				w = words[1].ToFloat();

			if (w <= 0)
				continue;

			groups.Insert(g);
			weights.Insert(w);
		}
	}
}
