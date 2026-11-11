//! One helper for every check (docs/DESIGN.md, "Checks and ways"): Trust (the persuade score),
//! Fear (the one threat score), the price table, and Pass / Short / Refused. Missions,
//! questlines, the majors, the finale's stages, errands and dialogue all use it; none keeps its
//! own copy. Scores are fixed, never rolled, so a tag can say what an option needs before you
//! pick it. Tags show costs and requirements, never outcomes.
//!
//! The built RZ_QuestApproach.PersuadeScore stays as it is: questline checks don't change.
//! The disguise look (claims, PassAs) belongs to disguise's phase 1 and isn't built yet.
//! Server only.

//------------------------------------------------------------------------------------------------
enum RZ_ECheck
{
	PASS,		// at or over the difficulty
	SHORT,		// one under: a halfway deal at the price the option names
	REFUSED		// two or more under: that person won't hear that approach from you again this job
}

//------------------------------------------------------------------------------------------------
class RZ_Checks
{
	//! Never more than this from dialogue's remembered help or a vouch.
	static const int DIALOGUE_BONUS_MAX = 1;
	//! The Fear band and the backing each give at most this much.
	static const int FEAR_MAX = 2;
	//! People with no faction read your Trust with the Haven, at most this much.
	static const int NO_FACTION_TRUST_MAX = 2;
	//! Infamy a threat costs (the built FORCE threat's), and the most a scene may name.
	static const int THREAT_INFAMY = 3;
	static const int THREAT_INFAMY_MAX = 8;
	//! A failed threat bars threats at that speaker for a real day.
	static const int THREAT_BAR_SECONDS = 86400;
	//! One price value is 5 Favor.
	static const int FAVOR_PER_VALUE = 5;

	//! Backgrounds Fear lists by default (+1).
	static const string FEAR_BACKGROUNDS = "excon,raider,soldier";

	// ================================================================ Trust
	//------------------------------------------------------------------------------------------------
	//! The persuade score (decision 11): your access rank with `faction` (Stranger, Uneasy 0; Known,
	//! Tolerated, Wildcard 1; Trusted, Respected 2; Champion 3), +1 for a background in `favoured`
	//! (comma-separated ids), +bonus from dialogue (at most 1), -1 for the liar count, -1 for crossing
	//! lines. People with no faction (-1) read your score with the Haven, at most 2. Hunted or worse:
	//! -10 (they won't be persuaded).
	static int TrustScore(RZ_GameModeComponent gm, int playerId, int faction, string favoured, int dialogueBonus = 0)
	{
		if (!gm || playerId <= 0)
			return -10;

		int listener = faction;
		bool unaffiliated = !RZ_Factions.IsValid(faction);
		if (unaffiliated)
			listener = RZ_EFaction.SURVIVORS;

		RZ_ETier tier = gm.GetTier(playerId, listener);
		if (RZ_Reputation.IsHunted(tier))
			return -10;

		int score = Math.Max(0, RZ_Reputation.GetAccessRank(tier));
		if (RZ_QuestApproach.HasBackground(gm, playerId, favoured))
			score++;

		score += Math.ClampInt(dialogueBonus, 0, DIALOGUE_BONUS_MAX);
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (RZ_Deeds.HasLifeRecord(profile, "lied." + listener.ToString()))
			score--;

		if (RZ_Deeds.HasLifeRecord(profile, "crossed." + listener.ToString()))
			score--;

		if (unaffiliated)
			score = Math.Min(score, NO_FACTION_TRUST_MAX);

		return score;
	}

	// ================================================================ Fear
	//------------------------------------------------------------------------------------------------
	//! The one threat score: the higher of your Feared band with the listener's faction (Infamy
	//! earned this life: 15-39 gives 1, 40+ gives 2) and your backing (Earned Fame with the giver,
	//! when the giver isn't the listener's faction: 15-39 gives 1, 40+ gives 2), +1 for a background
	//! in `favoured` (empty: Ex-Con, Raider, Soldier). Carrying a gun adds nothing. People with no
	//! faction (-1) read your highest earned Infamy with anyone, at most 2.
	static int FearScore(RZ_GameModeComponent gm, int playerId, int listener, int giver, string favoured = "")
	{
		if (!gm || playerId <= 0)
			return -10;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		int feared = 0;
		if (RZ_Factions.IsValid(listener))
		{
			feared = FearBand(profile.Feared(listener));
		}
		else
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				feared = Math.Max(feared, FearBand(profile.Feared(f)));
			}
		}

		int backing = 0;
		if (RZ_Factions.IsValid(giver) && giver != listener)
			backing = FearBand(profile.EarnedFame(giver));

		int score = Math.Max(feared, backing);
		string bgs = favoured;
		if (bgs.IsEmpty())
			bgs = FEAR_BACKGROUNDS;

		if (RZ_QuestApproach.HasBackground(gm, playerId, bgs))
			score++;

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! 15-39 gives 1, 40 and over 2.
	static int FearBand(int value)
	{
		return Math.Min(FEAR_MAX, RZ_Reputation.Band(value));
	}

	// ================================================================ Results
	//------------------------------------------------------------------------------------------------
	//! Pass at or over the difficulty (1 easy, 2 normal, 3 hard), Short one under, Refused two under.
	static RZ_ECheck Result(int score, int difficulty)
	{
		if (score >= difficulty)
			return RZ_ECheck.PASS;

		if (score == difficulty - 1)
			return RZ_ECheck.SHORT;

		return RZ_ECheck.REFUSED;
	}

	//------------------------------------------------------------------------------------------------
	static string ResultName(RZ_ECheck result)
	{
		switch (result)
		{
			case RZ_ECheck.PASS: return "pass";
			case RZ_ECheck.SHORT: return "short";
		}

		return "refused";
	}

	// ================================================================ Threats
	//------------------------------------------------------------------------------------------------
	//! A threat at `person` (a stable key: "leader:12", "wanderer:5", "<story key>.<who>"). Pass,
	//! Short or Refused by FearScore against `difficulty`. Refused without a roll when:
	//! - `stronghold` (never on stronghold leaders);
	//! - that person was already threatened this season, by anyone;
	//! - this player failed a threat at them less than a real day ago;
	//! - this life has used its threats with that faction (Reputation > Threats per life).
	//! Every threat, whatever the result, costs `infamy` (3 to 8) with the listener's faction and
	//! the life record "pressed". A refused threat bars that speaker for a real day.
	static RZ_ECheck Threaten(RZ_GameModeComponent gm, int playerId, string person, int listener, int giver, int difficulty, bool stronghold, int infamy = 3, string favoured = "")
	{
		if (!gm || playerId <= 0 || person.IsEmpty())
			return RZ_ECheck.REFUSED;

		if (stronghold)
			return RZ_ECheck.REFUSED;

		RZ_WorldFacts facts = gm.GetFacts();
		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		string ident = profile.m_sIdentity;
		if (facts && facts.Flag("threat." + person) > 0)
			return RZ_ECheck.REFUSED;

		string barKey = string.Format("threatbar.%1.%2", ident, person);
		int now = RZ_SeasonClock.Now();
		if (facts && now - facts.Flag(barKey) < THREAT_BAR_SECONDS)
			return RZ_ECheck.REFUSED;

		RZ_ECheck result = RZ_ECheck.REFUSED;
		int lf = listener;
		if (!RZ_Factions.IsValid(listener))
			lf = RZ_EFaction.SURVIVORS;

		if (profile.ThreatsUsed(lf) < RZ_ReputationSettings.Get().m_iThreatsPerLife)
			result = Result(FearScore(gm, playerId, listener, giver, favoured), difficulty);

		profile.UseThreat(lf);
		gm.AddInfamy(playerId, lf, Math.ClampInt(infamy, THREAT_INFAMY, THREAT_INFAMY_MAX), "threat", RZ_RepFlag.ACT);
		RZ_Deeds.AddLifeRecord(profile, "pressed", true);
		if (facts)
		{
			facts.SetFlag("threat." + person, 1);
			if (result == RZ_ECheck.REFUSED)
				facts.SetFlag(barKey, now);
		}

		RZ_Log.Info(string.Format("Checks: %1 threatened %2 (%3)", RZ_Util.PlayerName(playerId), person, ResultName(result)));
		return result;
	}

	// ================================================================ Prices
	//------------------------------------------------------------------------------------------------
	//! The price table: one value is 5 Favor; food 1, medical 2, a car part 3, anything else 1.
	//! kind: "food", "medical", "part".
	static int ValueOf(string kind)
	{
		if (kind == "medical")
			return 2;

		if (kind == "part")
			return 3;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Favor that matches `value` price values (a Trade's matching bribe).
	static int FavorFor(int value)
	{
		return Math.Max(0, value) * FAVOR_PER_VALUE;
	}

	// ================================================================ Tags
	//------------------------------------------------------------------------------------------------
	//! "[Trusted - Haven]": what a Trust option needs (difficulty 1 Known, 2 Trusted, 3 Champion).
	static string TrustTag(int faction, int difficulty)
	{
		string need = "Known";
		if (difficulty >= 3)
			need = "Champion";
		else if (difficulty == 2)
			need = "Trusted";

		if (!RZ_Factions.IsValid(faction))
			return "[" + need + "]";

		return string.Format("[%1 - %2]", need, RZ_Factions.GetName(faction));
	}

	//------------------------------------------------------------------------------------------------
	//! "[Feared | +3 Infamy]", with the Hunted-cliff warning when the cost would take the player
	//! into Hunted with the listener's faction.
	static string FearTag(RZ_GameModeComponent gm, int playerId, int listener, int infamy = 3)
	{
		string tag = string.Format("[Feared | +%1 Infamy]", infamy);
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(listener))
			return tag;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		RZ_ETier now = profile.GetTier(listener);
		RZ_ETier after = RZ_Reputation.GetTier(profile.GetFame(listener), profile.GetInfamy(listener) + infamy);
		if (!RZ_Reputation.IsHunted(now) && RZ_Reputation.IsHunted(after))
			tag = string.Format("[Feared | +%1 Infamy: you'd be Hunted]", infamy);

		return tag;
	}

	//------------------------------------------------------------------------------------------------
	//! "[4 food | 20 Favor]": a Trade's items and the matching bribe.
	static string PriceTag(int count, string kind)
	{
		string word = kind;
		if (kind == "part")
			word = "parts";

		return string.Format("[%1 %2 | %3 Favor]", count, word, FavorFor(count * ValueOf(kind)));
	}
}
