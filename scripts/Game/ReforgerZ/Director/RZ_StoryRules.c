//! Story rules next season (docs/QUESTLINES.md, "Next season: the story rules"): one small rule per
//! major that really ended last season (not by its defaults alone), read from its ending as it stood
//! at the cure, and only after a cure (Season > Story rules). They go on top of the controller's
//! rules and the finale's half rule set (RZ_Seasons.BuildMods), through RZ_SeasonMods.Add and Cap(),
//! and never move a faction's strength more than 2 in all.
//!
//! Most of a rule is a season rule set's numbers (strength, specials, infection, the share of road
//! posts). The rest is read where it lands: contract weights on the season budget, the Long Table's
//! chance, one stolen dose, a stronghold trait seeded, read-only legacy flags, a rumour and the
//! chair's line on the first day's boards. Applied once, when the fresh season starts (ApplyFresh).
//! Server only.
class RZ_StoryRules
{
	static const int STRENGTH_CAP = 2;

	protected static ref RZ_StoryRules s_Rules;

	ref array<string> m_aKeys = {};		// "major:ending" in force this season
	ref RZ_SeasonRuleSet m_Set;			// the numbers (null: none)
	float m_fTableChance = 1;
	int m_iStolenDoses;
	int m_iSeedFaction = -1;
	int m_iSeedTrait = -1;
	bool m_bChairNote;
	ref array<string> m_aRumours = {};
	ref array<string> m_aLegacy = {};
	ref array<string> m_aWeightTypes = {};
	ref array<float> m_aWeights = {};

	//------------------------------------------------------------------------------------------------
	//! The rules in force (never null).
	static RZ_StoryRules Get()
	{
		if (!s_Rules)
			s_Rules = new RZ_StoryRules();

		return s_Rules;
	}

	//------------------------------------------------------------------------------------------------
	//! The keys a finished season leaves: each major that ended live before the cure. A season with
	//! no cure leaves none.
	static void FromRecord(RZ_SeasonRecord last, notnull array<string> result)
	{
		if (!last || last.winner < 0 || last.endedBy != "cure" || !last.stories)
			return;

		foreach (RZ_StoryRecord r : last.stories)
		{
			if (!r || r.fallback || r.atCure < 0 || r.major.IsEmpty())
				continue;

			result.Insert(r.major + ":" + r.atCure.ToString());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Builds the season's rules from its keys (a fresh season's, or the saved ones after a restart).
	static RZ_StoryRules Build(array<string> keys)
	{
		s_Rules = new RZ_StoryRules();
		if (!keys)
			return s_Rules;

		array<int> strength = {0, 0, 0, 0};
		array<string> lines = {};
		foreach (string key : keys)
		{
			s_Rules.m_aKeys.Insert(key);
			s_Rules.AddRule(key, strength, lines);
		}

		if (lines.IsEmpty())
			return s_Rules;

		RZ_SeasonRuleSet r = s_Rules.Set();
		for (int f = 0; f < strength.Count(); f++)
		{
			strength[f] = Math.ClampInt(strength[f], -STRENGTH_CAP, STRENGTH_CAP);
		}

		r.m_iHavenStrength = strength[RZ_EFaction.SURVIVORS];
		r.m_iBastionStrength = strength[RZ_EFaction.MILITARY];
		r.m_iRedHandStrength = strength[RZ_EFaction.BANDITS];
		r.m_iHalcyonStrength = strength[RZ_EFaction.INSTITUTE];
		string text = "";
		foreach (string line : lines)
		{
			if (!text.IsEmpty())
				text += " ";

			text += line;
		}

		r.m_sDescription = text;
		RZ_Log.Info(string.Format("Story rules: %1 from last season (%2).", keys.Count(), text));
		return s_Rules;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SeasonRuleSet Set()
	{
		if (!m_Set)
		{
			m_Set = RZ_SeasonRuleSet.Cast(RZ_Util.NewWithDefaults("RZ_SeasonRuleSet"));
			m_Set.m_sName = "Last season's stories";
		}

		return m_Set;
	}

	//------------------------------------------------------------------------------------------------
	//! One major's rule (docs/QUESTLINES.md, each ending's "Next season").
	protected void AddRule(string key, notnull array<int> strength, notnull array<string> lines)
	{
		switch (key)
		{
			// The Book Has to Balance
			case "book:10":
			{
				strength[RZ_EFaction.BANDITS] = strength[RZ_EFaction.BANDITS] + 2;
				lines.Insert("The Red Hand's book balanced, and the crews remember it.");
				break;
			}
			case "book:11":
			{
				Weight("RZ_Mission_TraderCaravan", 1.1);
				lines.Insert("Everyone bought their page back from the Red Hand, so traders cross the lines a bit more.");
				break;
			}
			case "book:13":
			{
				strength[RZ_EFaction.MILITARY] = strength[RZ_EFaction.MILITARY] + 2;
				strength[RZ_EFaction.BANDITS] = strength[RZ_EFaction.BANDITS] - 2;
				lines.Insert("Bastion has last season's ledger. The crews are thinner for it.");
				break;
			}
			case "book:14":
			{
				m_iSeedFaction = RZ_EFaction.BANDITS;
				m_iSeedTrait = RZ_ELeaderTrait.PARANOID;
				Weight("RZ_Mission_TheRat", 1.1);
				lines.Insert("The Red Hand's book never balanced. The new boss counts twice.");
				break;
			}
			// Await Relief
			case "orders:10":
			{
				Set().m_fBastionPostShare = 0.6;
				lines.Insert("Bastion kept the call last season. More of the road posts are theirs.");
				break;
			}
			case "orders:11":
			{
				Weight("RZ_Mission_Requisition", 0.9);
				lines.Insert("Bastion told its men the truth last season. It asks a little less of the houses.");
				break;
			}
			case "orders:12":
			{
				strength[RZ_EFaction.MILITARY] = strength[RZ_EFaction.MILITARY] + 2;
				lines.Insert("Bastion answered for Command last season, and still does.");
				break;
			}
			case "orders:13":
			{
				strength[RZ_EFaction.MILITARY] = strength[RZ_EFaction.MILITARY] - 2;
				lines.Insert("Bastion was told it was expended. Some of it believed that.");
				break;
			}
			// Every Name
			case "every_name:10":
			{
				m_fTableChance = m_fTableChance * 0.5;
				m_bChairNote = true;
				lines.Insert("Last season the Haven read every name. People knock on doors before they knock on farms.");
				break;
			}
			case "every_name:11":
			{
				m_aRumours.Insert("Somebody in the houses kept a book last season and never opened it. People still ask what was in it.");
				lines.Insert("The Haven kept its book shut last season.");
				break;
			}
			case "every_name:12":
			{
				m_fTableChance = m_fTableChance * 1.5;
				lines.Insert("Last season the Haven fed the ones who worked first. Some of the others went looking for a kinder table.");
				break;
			}
			// Proof
			case "proof:10":
			{
				Set().m_fStartInfectionMult = 0.95;
				m_aLegacy.Insert("legacy.records");
				lines.Insert("Last season's paper started three weeks early. The lab knows where to look this time.");
				break;
			}
			case "proof:11":
			{
				strength[RZ_EFaction.MILITARY] = strength[RZ_EFaction.MILITARY] + 2;
				m_aLegacy.Insert("legacy.terms");
				lines.Insert("Bastion signed last season's paper. Its terms with the lab still stand.");
				break;
			}
			case "proof:12":
			{
				Set().m_fSpecialMult = 1.05;
				m_aRumours.Insert("Last season's paper said Day 1. Some people never believed it.");
				lines.Insert("Last season's paper said Day 1, and the infected took no notice.");
				break;
			}
			case "proof:14":
			{
				strength[RZ_EFaction.BANDITS] = strength[RZ_EFaction.BANDITS] + 2;
				m_iStolenDoses = 1;
				lines.Insert("The Red Hand sold the lab's first page last season, two tins a copy. Its market starts with a dose.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Weight(string type, float mult)
	{
		m_aWeightTypes.Insert(type);
		m_aWeights.Insert(mult);
	}

	//------------------------------------------------------------------------------------------------
	//! The numbers, for RZ_Seasons.BuildMods (null: none).
	RZ_SeasonRuleSet RuleSet()
	{
		return m_Set;
	}

	//------------------------------------------------------------------------------------------------
	//! The Long Table's chance x this (its roll, once a season; never over 1).
	static float TableChance()
	{
		return Get().m_fTableChance;
	}

	//------------------------------------------------------------------------------------------------
	//! A trait seeded into a stronghold leader's first generation (-1: none).
	static int SeedTrait(int faction)
	{
		RZ_StoryRules r = Get();
		if (r.m_iSeedFaction != faction)
			return -1;

		return r.m_iSeedTrait;
	}

	//------------------------------------------------------------------------------------------------
	//! A fresh season, once everything is set up: the weights, the dose, the legacy flags and the
	//! rumours.
	static void ApplyFresh(RZ_GameModeComponent gm)
	{
		RZ_StoryRules r = Get();
		if (r.m_aKeys.IsEmpty())
			return;

		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		for (int i = 0; i < r.m_aWeightTypes.Count(); i++)
		{
			if (budget)
				budget.SetContractWeight(r.m_aWeightTypes[i], r.m_aWeights[i]);
		}

		if (r.m_iStolenDoses > 0 && gm.GetCure())
			gm.GetCure().SeedStolen(r.m_iStolenDoses);

		foreach (string flag : r.m_aLegacy)
		{
			RZ_MajorHelp.SetWorld(gm, flag, 1);
		}

		foreach (string rumour : r.m_aRumours)
		{
			if (gm.GetQuestlines())
				gm.GetQuestlines().AddRumour(rumour, 48);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every Name: the chair's line on every house board on the season's first real day.
	static void BoardNotes(RZ_GameModeComponent gm, notnull array<string> authors, notnull array<string> texts)
	{
		RZ_StoryRules r = Get();
		if (!r.m_bChairNote || !gm.GetSeasons() || gm.GetSeasons().GetRealDays() != 0)
			return;

		string author = "the chair";
		RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
		if (chair)
			author = chair.m_sName;

		authors.Insert(author);
		texts.Insert("Last season this house read every name. If it comes to it, we will again.");
	}
}
