//! One helper for every check (docs/DESIGN.md, "Checks and ways"): Trust (the persuade score),
//! Fear (the one threat score), the price table, and Pass / Short / Refused. Missions,
//! questlines, the majors, the finale's stages, errands and dialogue all use it; none keeps its
//! own copy. Scores are fixed, never rolled, so a tag can say what an option needs before you
//! pick it. Tags show costs and requirements, never outcomes.
//!
//! The built RZ_QuestApproach.PersuadeScore stays as it is: questline checks don't change.
//! The disguise look (LookScore, judged by RZ_Disguise.Look; missions reach it through RZ_Ways.PassAs)
//! and the lie check (Lie) live here too.
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

		// Passing as one of them: your persuade score with them counts as 0
		RZ_Hostility hostility = gm.GetHostility();
		if (hostility && hostility.GetDisguise().IsPassing(playerId, listener))
			return 0;

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

	//------------------------------------------------------------------------------------------------
	//! A threat's difficulty at a leader or a leader's people: a Ruthless listener one harder, a
	//! Paranoid one one easier, and one harder at a faction this player worked against in a
	//! questline ending (the life record "crossed.<faction>"). Never under 1.
	static int FearDifficulty(RZ_GameModeComponent gm, int playerId, int listener, int baseDifficulty, RZ_Leader leader = null)
	{
		int d = baseDifficulty;
		if (leader && leader.HasTrait(RZ_ELeaderTrait.RUTHLESS))
			d++;

		if (leader && leader.HasTrait(RZ_ELeaderTrait.PARANOID))
			d--;

		if (gm && playerId > 0 && RZ_Factions.IsValid(listener) && RZ_Deeds.HasLifeRecord(gm.GetProfile(playerId), "crossed." + listener.ToString()))
			d++;

		return Math.Max(1, d);
	}

	//------------------------------------------------------------------------------------------------
	//! Free-standing talk threats (not one a mission writes into its own scene) are never made in a
	//! safe zone (a stronghold or the lab) or at a safehouse. "" if allowed here, else the line.
	static string ThreatRefusedAt(RZ_GameModeComponent gm, vector pos)
	{
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.FindNearest(pos, 60) >= 0)
			return "\"Not here. Not under this roof.\"";

		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (h.m_bStronghold && RZ_Util.DistXZ(h.m_vPos, pos) <= 80)
					return "\"Inside the wire? Try it.\"";
			}
		}

		if (gm.HasLab() && RZ_Util.DistXZ(gm.GetLabPosition(), pos) <= 80)
			return "\"Inside the wire? Try it.\"";

		return "";
	}

	// ================================================================ The look (borrowed kit)
	//------------------------------------------------------------------------------------------------
	//! The disguise look (docs/REPUTATION.md, "Borrowed kit"): fixed, so the same player in the same
	//! situation always gets the same answer, shown with its tags before they commit.
	//! Plus: their colours +1 / full kit +2; a matching background +1 (Soldier or Police for Bastion,
	//! Ex-Con or Raider for the Red Hand, Lab Technician or Doctor for Halcyon); one cover detail +1
	//! (the word at the wire, a crew name, a name you've heard, the mission's paper); dark or fog +1.
	//! Minus (each): they know your face; they caught you earlier this life; you're on the bounty
	//! notices (Kill on Sight with anyone); each look already passed at this place this visit; each tell;
	//! each point-out (two at most); under half health; the infection showing.
	//! Instant fail: Hunted or Kill on Sight with them, the infection showing at Halcyon's look,
	//! two tells, the catches used up this life.
	static int LookScore(RZ_GameModeComponent gm, int playerId, int f, string place, string paper, out string tags, out bool instant)
	{
		tags = "";
		instant = false;
		RZ_Hostility hostility = gm.GetHostility();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!hostility || !me || !RZ_Factions.IsValid(f))
		{
			instant = true;
			return 0;
		}

		RZ_Disguise d = hostility.GetDisguise();
		string no = d.CannotClaim(playerId, f);
		if (!no.IsEmpty())
		{
			instant = true;
			tags = "[" + no + "]";
			return 0;
		}

		int score = 0;
		int kit = d.KitLevel(playerId, f);
		if (kit >= 2)
		{
			score += 2;
			tags += "[Full kit +2]";
		}
		else
		{
			score += 1;
			tags += "[Their colours +1]";
		}

		string bgs = "soldier,police";
		if (f == RZ_EFaction.BANDITS)
			bgs = "excon,raider";
		else if (f == RZ_EFaction.INSTITUTE)
			bgs = "labtech,doctor";

		if (RZ_QuestApproach.HasBackground(gm, playerId, bgs))
		{
			score++;
			tags += " [Background +1]";
		}

		string cover = d.CoverDetail(playerId, f, paper);
		if (!cover.IsEmpty())
		{
			score++;
			tags += string.Format(" [%1 +1]", cover);
		}

		if (d.IsDark(me))
		{
			score++;
			tags += " [Dark +1]";
		}

		if (d.KnowsFace(playerId, f))
		{
			score--;
			tags += " [They know your face -1]";
		}

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		if (profile.m_aCatches[f] > 0)
		{
			score--;
			tags += " [Caught before -1]";
		}

		for (int g = 0; g < RZ_Factions.COUNT; g++)
		{
			if (gm.GetTier(playerId, g) == RZ_ETier.KILL_ON_SIGHT)
			{
				score--;
				tags += " [On the notices -1]";
				break;
			}
		}

		int passed = d.PassedHere(playerId, place);
		if (passed > 0)
		{
			score -= passed;
			tags += string.Format(" [Looked at already -%1]", passed);
		}

		int tells = d.Tells(playerId);
		if (tells > 0)
		{
			score -= tells;
			tags += string.Format(" [Tells -%1]", tells);
		}

		int pointed = d.PointedOut(playerId, f);
		if (pointed > 0)
		{
			score -= pointed;
			tags += string.Format(" [Pointed out -%1]", pointed);
		}

		float health = RZ_HealthCost.Share(me);
		if (health >= 0 && health < 0.5)
		{
			score--;
			tags += " [Hurt -1]";
		}

		// Kit that remembers whose it was, word gets round, seen putting it on, running, a gun in
		// your hands (RZ_Disguise.MoreTags)
		string more;
		bool fails;
		score += d.MoreTags(playerId, f, place, me, more, fails);
		tags += more;
		if (fails)
			instant = true;

		if (profile.m_iInfectionStage >= RZ_EInfectionStage.WEAKNESS)
		{
			if (f == RZ_EFaction.INSTITUTE)
			{
				instant = true;
				tags += " [They see bites for a living]";
			}
			else
			{
				score--;
				tags += " [The infection shows -1]";
			}
		}

		return score;
	}

	// ================================================================ Lies
	//------------------------------------------------------------------------------------------------
	//! A lie told to `faction`'s people (dialogue's and errands' [Lie] choices): a tell catches it first,
	//! whatever the score (`tell` true: symptoms showing for a bite lie, another faction's colours for
	//! "I'm with them", your own kit for "I'm not with them", a different claim already held, or a lie
	//! nobody could get away with). Without a tell, the lie check is the Trust score against the
	//! difficulty; the liar count (life record "lied.<faction>") already takes 1 off it.
	static RZ_ECheck Lie(RZ_GameModeComponent gm, int playerId, int faction, int difficulty, bool tell, int bonus = 0)
	{
		if (tell)
			return RZ_ECheck.REFUSED;

		return Result(TrustScore(gm, playerId, faction, "", bonus), difficulty);
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

		// Passing as one of them, you can't threaten their people
		RZ_Hostility hostility = gm.GetHostility();
		if (hostility && RZ_Factions.IsValid(listener) && hostility.GetDisguise().IsPassing(playerId, listener))
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
		// A background's service on the spot (a talk scene's): two, plus the time
		if (kind == "medical" || kind == "service")
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
	//! "[Threaten - +3 Bastion Infamy - Fear 2 of 2]", or "... - this makes you Hunted]" when the cost
	//! would take the player into Hunted with the listener's faction. The score is shown, never the
	//! outcome. difficulty 0: no score.
	static string FearTag(RZ_GameModeComponent gm, int playerId, int listener, int infamy = 3, int difficulty = 0)
	{
		string who = "";
		if (RZ_Factions.IsValid(listener))
			who = RZ_Factions.GetShortName(listener) + " ";

		string tag = string.Format("[Threaten - +%1 %2Infamy", infamy, who);
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(listener))
			return tag + "]";

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		RZ_ETier now = profile.GetTier(listener);
		RZ_ETier after = RZ_Reputation.GetTier(profile.GetFame(listener), profile.GetInfamy(listener) + infamy);
		if (!RZ_Reputation.IsHunted(now) && RZ_Reputation.IsHunted(after))
			return tag + " - this makes you Hunted]";

		if (difficulty > 0)
			tag += string.Format(" - Fear %1 of %2", FearScore(gm, playerId, listener, -1), difficulty);

		return tag + "]";
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
