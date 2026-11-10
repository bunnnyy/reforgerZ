//! A civilian background, chosen in the spawn (deploy) menu, like picking an
//! occupation in Project Zomboid.
//!
//! How choosing works: each background is a loadout in the player faction's
//! loadout list (SCR_LoadoutManager). The vanilla deploy menu already lets players
//! pick a loadout, so no new UI is needed to get started. The loadout gives the
//! starting gear. This class gives the perk and flaw, matched on m_sLoadoutName.
//!
//! Every modifier here feeds a ReforgerZ system (noise/heat, infection, research,
//! reputation), so the perks work no matter which other mods the server runs.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sName")]
class RZ_Background
{
	[Attribute("", desc: "Unique id, saved with the player profile")]
	string m_sId;

	[Attribute("", desc: "Display name")]
	string m_sName;

	[Attribute("", desc: "Must match the loadout name the player picks in the deploy menu")]
	string m_sLoadoutName;

	[Attribute("", UIWidgets.EditBoxMultiline, "Perk text shown on spawn")]
	string m_sPerk;

	[Attribute("", UIWidgets.EditBoxMultiline, "Flaw text shown on spawn")]
	string m_sFlaw;

	[Attribute("1", desc: "Multiplier on the heat (zombie attention) this player generates")]
	float m_fNoiseMult = 1;

	[Attribute("1", desc: "Multiplier on how long an infection takes to kill you")]
	float m_fInfectionTimeMult = 1;

	[Attribute("1", desc: "Multiplier on research points per delivered sample")]
	float m_fResearchMult = 1;

	[Attribute("1", desc: "Multiplier on Fame earned from missions")]
	float m_fMissionRewardMult = 1;

	[Attribute("0", desc: "Hears horde warnings and intel from anywhere on the map")]
	bool m_bRadioIntel;

	[Attribute("0", desc: "Starting standing with the Haven. Positive = Fame, negative = Infamy")]
	int m_iRepSurvivors;

	[Attribute("0", desc: "Starting standing with Task Force Bastion. Positive = Fame, negative = Infamy")]
	int m_iRepMilitary;

	[Attribute("0", desc: "Starting standing with the Red Hand. Positive = Fame, negative = Infamy")]
	int m_iRepBandits;

	[Attribute("0", desc: "Starting standing with Halcyon. Positive = Fame, negative = Infamy")]
	int m_iRepInstitute;

	[Attribute("0", desc: "Locked until the player unlocks it (Legacy). Picking it while locked falls back to the default background")]
	bool m_bLocked;

	[Attribute("", desc: "Unlock id saved on the player profile (e.g. soldier, doctor, raider, elder, drifter)")]
	string m_sUnlockId;

	[Attribute("", desc: "How to unlock it, shown when a player picks it while it's still locked")]
	string m_sUnlockText;

	//------------------------------------------------------------------------------------------------
	//! Signed starting standing: positive values are Fame, negative values Infamy.
	int GetStartRep(RZ_EFaction f)
	{
		switch (f)
		{
			case RZ_EFaction.SURVIVORS: return m_iRepSurvivors;
			case RZ_EFaction.MILITARY: return m_iRepMilitary;
			case RZ_EFaction.BANDITS: return m_iRepBandits;
			case RZ_EFaction.INSTITUTE: return m_iRepInstitute;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	int GetStartFame(RZ_EFaction f)
	{
		return Math.Max(0, GetStartRep(f));
	}

	//------------------------------------------------------------------------------------------------
	int GetStartInfamy(RZ_EFaction f)
	{
		return Math.Max(0, -GetStartRep(f));
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_Background Make(string id, string name, string perk, string flaw)
	{
		RZ_Background b = new RZ_Background();
		b.m_sId = id;
		b.m_sName = name;
		b.m_sLoadoutName = name;
		b.m_sPerk = perk;
		b.m_sFlaw = flaw;
		return b;
	}

	//------------------------------------------------------------------------------------------------
	//! Built-in set, used when none are configured in Workbench.
	//! Loadout names must match these names (e.g. a loadout called "Nurse").
	static void CreateDefaults(notnull array<ref RZ_Background> outList)
	{
		RZ_Background b;

		b = Make("nurse", "Nurse",
			"Starts with medical supplies. Infection takes 50% longer to kill you.",
			"Starts unarmed.");
		b.m_fInfectionTimeMult = 1.5;
		b.m_iRepSurvivors = 10;
		outList.Insert(b);

		b = Make("mechanic", "Mechanic",
			"Starts with a toolbox and a jerrycan, enough to get a car running.",
			"Clumsy and loud: draws 25% more zombie attention.");
		b.m_fNoiseMult = 1.25;
		outList.Insert(b);

		b = Make("hunter", "Hunter",
			"Starts with a hunting rifle. Moves quietly: 25% less zombie attention.",
			"Very little ammo. The military doesn't trust armed civilians.");
		b.m_fNoiseMult = 0.75;
		b.m_iRepMilitary = -10;
		outList.Insert(b);

		b = Make("police", "Police Officer",
			"Starts with a sidearm. The military trusts you.",
			"Bandits hate cops.");
		b.m_iRepMilitary = 20;
		b.m_iRepBandits = -25;
		outList.Insert(b);

		b = Make("excon", "Ex-Con",
			"Bandits treat you as one of their own. Starts with a knife.",
			"Military and survivors are wary of you. Poor health: infection kills 20% faster.");
		b.m_iRepBandits = 25;
		b.m_iRepMilitary = -25;
		b.m_iRepSurvivors = -10;
		b.m_fInfectionTimeMult = 0.8;
		outList.Insert(b);

		b = Make("labtech", "Lab Technician",
			"Samples you deliver are worth double research. The Institute trusts you.",
			"Starts unarmed and draws a little extra attention.");
		b.m_fResearchMult = 2;
		b.m_iRepInstitute = 20;
		b.m_fNoiseMult = 1.15;
		outList.Insert(b);

		b = Make("radioham", "Radio Operator",
			"Starts with a radio. Hears horde warnings and intel from anywhere.",
			"Talks too much: draws 20% more zombie attention.");
		b.m_bRadioIntel = true;
		b.m_iRepSurvivors = 10;
		b.m_fNoiseMult = 1.2;
		outList.Insert(b);

		b = Make("unemployed", "Unemployed",
			"Eager to prove yourself: +50% Fame from missions.",
			"Starts with nothing but the clothes on your back.");
		b.m_fMissionRewardMult = 1.5;
		outList.Insert(b);

		// ---- Unlockable (Legacy). See docs/REPUTATION.md.
		b = MakeLocked("soldier", "Soldier", "Reach Champion with Task Force Bastion in any life.",
			"Starts with a service rifle and armour. Bastion knows you.",
			"The Red Hand starts out watching you.");
		b.m_iRepMilitary = 20;
		b.m_iRepBandits = -15;
		outList.Insert(b);

		b = MakeLocked("doctor", "Doctor", "Reach Champion with Halcyon in any life.",
			"Can treat bites: an infection takes two and a half times as long to kill you. Halcyon knows you.",
			"Starts unarmed.");
		b.m_fInfectionTimeMult = 2.5;
		b.m_iRepInstitute = 20;
		outList.Insert(b);

		b = MakeLocked("raider", "Raider", "Reach Champion with the Red Hand in any life.",
			"The Red Hand trusts you: its black market is open from the start.",
			"The Haven and Bastion start out watching you.");
		b.m_iRepBandits = 40;
		b.m_iRepSurvivors = -15;
		b.m_iRepMilitary = -15;
		outList.Insert(b);

		b = MakeLocked("elder", "Elder", "Reach Champion with the Haven in any life.",
			"The Haven trusts you from the start.",
			"Old and slow: draws 30% more zombie attention.");
		b.m_iRepSurvivors = 40;
		b.m_fNoiseMult = 1.3;
		outList.Insert(b);

		b = MakeLocked("drifter", "Drifter", "Earn 10 Legacy points (personal goals).",
			"Every faction has heard of you: starts Known with all four.",
			"No gear at all.");
		b.m_iRepSurvivors = 15;
		b.m_iRepMilitary = 15;
		b.m_iRepBandits = 15;
		b.m_iRepInstitute = 15;
		outList.Insert(b);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_Background MakeLocked(string id, string name, string unlockText, string perk, string flaw)
	{
		RZ_Background b = Make(id, name, perk, flaw);
		b.m_bLocked = true;
		b.m_sUnlockId = id;
		b.m_sUnlockText = unlockText;
		return b;
	}
}
