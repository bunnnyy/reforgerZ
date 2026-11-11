//! Borrowed kit (docs/REPUTATION.md, "Borrowed kit"): disguise phase 1. Wearing a faction's kit does
//! nothing by itself. You claim to be one of them (walking up to a barrier in full kit, talking to a
//! post's speaker, a mission's trick) and they look at you: a fixed score, shown before you commit
//! (RZ_Checks.LookScore). A pass buys passage and a moment, never goods, standing or services.
//! Caught costs the one price: +3 Infamy (never taking you to 15 on its own), the kit taken, one of
//! three catches per faction per life, and blown with them for 60 minutes of play.
//!
//! What you wear is read from the loadout slots every few seconds (RZ_Util.GetWornKit):
//! - their colours: 2 pieces matching a faction's keywords; full kit: 3, one of them on the head;
//! - own colours: their kit while you're Known or better with them (access rank 1+), never a
//!   disguise; in their kit: colours while you're below Known with them;
//! - tells (-1 each; two and you can't claim): another faction's colours at the same time, the
//!   wrong weapon (Bastion: not a service weapon; Halcyon: any long gun).
//! The Hunted and Kill on Sight can't claim. Leaders, everyone inside a stronghold's or the lab's
//! wire, and anything within the stand radius of the lab during the final stand see through it.
//! Fights (hunter squads, mission groups, grudges) are never fooled: only standing bits are cleared.
//! Server only.
class RZ_Disguise
{
	static const float KNOW_RADIUS = 150;	// "near their people or holdings": doubling and firing
	static const float CATCH_RADIUS = 35;	// an act this close to their people is a catch, not an exposure
	static const float LEADER_RADIUS = 15;	// a leader of theirs this close sees through you
	static const float WIRE_RADIUS = 80;	// inside a stronghold's or the lab's wire
	static const float HELD_SECONDS = 25;	// the stand-down while you're held
	static const float NOTICE_GAP = 120;

	protected RZ_GameModeComponent m_GM;
	protected RZ_HostilitySettings m_Settings;
	protected ref array<ref array<string>> m_aWords = {};		// per faction: the uniform keywords
	protected ref array<ref array<string>> m_aHeadWords = {};	// per faction: head pieces
	protected ref array<string> m_aServiceWords = {};
	protected ref map<int, int> m_mWearing = new map<int, int>();	// playerId -> faction whose kit (best level), -1 none
	protected ref map<int, int> m_mLevel = new map<int, int>();		// playerId -> that kit's level: 1 colours, 2 full
	protected ref map<int, int> m_mTells = new map<int, int>();		// playerId -> tells right now
	protected ref map<int, int> m_mOther = new map<int, int>();		// playerId -> bitmask of every faction whose colours they wear
	protected ref map<string, float> m_mPassUntil = new map<string, float>();	// "ident|f" -> clock
	protected ref map<string, int> m_mPassed = new map<string, int>();		// "ident|place" -> looks passed there this visit
	protected ref map<string, float> m_mTold = new map<string, float>();		// "id|what" -> clock
	protected ref map<string, int> m_mWord = new map<string, int>();			// ident -> season day the word was heard
	protected ref map<string, int> m_mPointed = new map<string, int>();		// "ident|f" -> point-outs at the current look
	protected ref map<string, float> m_mHourStart = new map<string, float>();	// "ident|f" -> clock the impersonation hour began
	protected ref map<string, int> m_mHourInfamy = new map<string, int>();		// "ident|f" -> impersonation Infamy this hour
	protected ref array<IEntity> m_aWatchers = {};		// a mission's own people (AddWatcher), weak
	protected ref array<int> m_aWatcherFaction = {};
	protected float m_fClock;
	protected float m_fCheckTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Disguise(RZ_GameModeComponent gm, RZ_HostilitySettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			array<string> words = {};
			array<string> heads = {};
			m_aWords.Insert(words);
			m_aHeadWords.Insert(heads);
		}

		RZ_ItemHelper.Words(settings.m_sBastionUniformWords, m_aWords[RZ_EFaction.MILITARY]);
		RZ_ItemHelper.Words(settings.m_sRedHandUniformWords, m_aWords[RZ_EFaction.BANDITS]);
		RZ_ItemHelper.Words(settings.m_sHalcyonUniformWords, m_aWords[RZ_EFaction.INSTITUTE]);
		RZ_ItemHelper.Words(settings.m_sBastionHeadWords, m_aHeadWords[RZ_EFaction.MILITARY]);
		RZ_ItemHelper.Words(settings.m_sRedHandHeadWords, m_aHeadWords[RZ_EFaction.BANDITS]);
		RZ_ItemHelper.Words(settings.m_sHalcyonHeadWords, m_aHeadWords[RZ_EFaction.INSTITUTE]);
		RZ_ItemHelper.Words(settings.m_sServiceWeaponWords, m_aServiceWords);
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_Settings.m_bDisguises;
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	// ================================================================ Reading the kit
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		if (!IsOn())
			return;

		m_fCheckTimer += dt;
		if (m_fCheckTimer < Math.Max(2, m_Settings.m_fDisguiseCheckSeconds))
			return;

		m_fCheckTimer = 0;
		foreach (int i, IEntity p : players)
		{
			int before = Wearing(ids[i]);
			Read(ids[i], p);
			int now = Wearing(ids[i]);
			if (now >= 0 && now != before)
				OnPutOn(ids[i], now);

			Breakers(ids[i], p);
		}

		array<int> gone = {};
		foreach (int playerId, int worn : m_mWearing)
		{
			if (!ids.Contains(playerId))
				gone.Insert(playerId);
		}

		foreach (int goneId : gone)
		{
			m_mWearing.Remove(goneId);
			m_mLevel.Remove(goneId);
			m_mTells.Remove(goneId);
			m_mOther.Remove(goneId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Which faction's kit this character wears, at what level, with which tells.
	protected void Read(int playerId, IEntity character)
	{
		array<string> names = {};
		array<string> heads = {};
		RZ_Util.GetWornKit(character, names, heads);
		int best = -1;
		int bestLevel = 0;
		int bestCount = 0;
		int colours = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			array<string> words = m_aWords[f];
			if (words.IsEmpty())
				continue;

			int count = 0;
			bool head = false;
			foreach (string name : names)
			{
				if (!RZ_ItemHelper.NameHas(name, words) && !RZ_ItemHelper.NameHas(name, m_aHeadWords[f]))
					continue;

				count++;
				if (heads.Contains(name) || RZ_ItemHelper.NameHas(name, m_aHeadWords[f]))
					head = true;
			}

			int level = 0;
			if (count >= Math.Max(1, m_Settings.m_iDisguiseItems))
				level = 1;

			if (level == 1 && head && count >= Math.Max(1, m_Settings.m_iDisguiseItems) + 1)
				level = 2;

			if (level > 0)
				colours = colours | RZ_Factions.Bit(f);

			if (level > bestLevel || (level == bestLevel && level > 0 && count > bestCount))
			{
				best = f;
				bestLevel = level;
				bestCount = count;
			}
		}

		m_mWearing.Set(playerId, best);
		m_mLevel.Set(playerId, bestLevel);
		m_mOther.Set(playerId, colours);
		m_mTells.Set(playerId, CountTells(playerId, character, best, colours));
	}

	//------------------------------------------------------------------------------------------------
	//! Another faction's colours worn at the same time; the wrong weapon.
	protected int CountTells(int playerId, IEntity character, int f, int colours)
	{
		if (f < 0)
			return 0;

		int tells = 0;
		if ((colours & ~RZ_Factions.Bit(f)) != 0)
			tells++;

		array<string> guns = {};
		bool longGun = RZ_Util.GetFirearms(character, guns);
		if (f == RZ_EFaction.INSTITUTE && longGun)
			tells++;

		if (f == RZ_EFaction.MILITARY && !m_aServiceWords.IsEmpty())
		{
			foreach (string gun : guns)
			{
				if (!RZ_ItemHelper.NameHas(gun, m_aServiceWords))
				{
					tells++;
					break;
				}
			}
		}

		return tells;
	}

	//------------------------------------------------------------------------------------------------
	//! The first time you wear their kit this life: the rule of thumb, once.
	protected void OnPutOn(int playerId, int f)
	{
		if (IsOwnColours(playerId, f) || !Tell(playerId, "puton" + f.ToString(), 3600))
			return;

		string text = string.Format("In %1's kit, their people look at your face up close. Don't run, sling your rifle, keep away from officers and never go inside the wire.", RZ_Factions.GetNameInText(f));
		if (f == RZ_EFaction.MILITARY)
			text += " The Red Hand will treat you as Bastion.";
		else if (f == RZ_EFaction.BANDITS)
			text += " Bastion will treat you as Red Hand.";
		else if (f == RZ_EFaction.INSTITUTE)
			text += " The Red Hand charges beaks double.";

		if (IsBlown(playerId, f))
			text = string.Format("%1 knows your face now. Their kit is only colours to them for a while.", RZ_Factions.GetName(f));

		m_GM.NotifyPlayer(playerId, "Borrowed kit", text);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Tell(int playerId, string what, float gap)
	{
		string key = playerId.ToString() + "|" + what;
		float last;
		if (m_mTold.Find(key, last) && m_fClock - last < gap)
			return false;

		m_mTold.Set(key, m_fClock);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose kit the player wears, -1 if none (as of the last check).
	int Wearing(int playerId)
	{
		int f;
		if (m_mWearing.Find(playerId, f))
			return f;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 none, 1 their colours, 2 full kit (of `f`).
	int KitLevel(int playerId, int f)
	{
		if (Wearing(playerId) == f)
		{
			int level;
			m_mLevel.Find(playerId, level);
			return level;
		}

		int colours;
		m_mOther.Find(playerId, colours);
		if ((colours & RZ_Factions.Bit(f)) != 0)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Wearing `f`'s colours at all (their enemies react to it, friends of `f` included).
	bool WearsColours(int playerId, int f)
	{
		return IsOn() && m_Settings.m_bColours && KitLevel(playerId, f) > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Their kit while you're Known or better with them (mixed tiers by access): just your gear.
	bool IsOwnColours(int playerId, int f)
	{
		return RZ_Reputation.GetAccessRank(m_GM.GetTier(playerId, f)) >= 1;
	}

	//------------------------------------------------------------------------------------------------
	//! In `f`'s kit and not your own colours: nothing happens until you claim.
	bool InTheirKit(int playerId, int f)
	{
		return IsOn() && KitLevel(playerId, f) > 0 && !IsOwnColours(playerId, f);
	}

	//------------------------------------------------------------------------------------------------
	int Tells(int playerId)
	{
		int n;
		m_mTells.Find(playerId, n);
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Blown with `f`: for 60 minutes of play after a catch or an exposure.
	bool IsBlown(int playerId, int f)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		return RZ_Factions.IsValid(f) && profile.m_fLifeSeconds < profile.m_aBlownUntil[f];
	}

	//------------------------------------------------------------------------------------------------
	//! Passed a look and still inside its window, not blown since.
	bool IsPassing(int playerId, int f)
	{
		if (!IsOn() || !RZ_Factions.IsValid(f) || IsBlown(playerId, f) || !InTheirKit(playerId, f))
			return false;

		float until;
		return m_mPassUntil.Find(Ident(playerId) + "|" + f.ToString(), until) && m_fClock < until;
	}

	//------------------------------------------------------------------------------------------------
	//! Kept for missions written before the look: passing as `f`.
	bool IsDisguisedAs(int playerId, int f)
	{
		return IsPassing(playerId, f);
	}

	//------------------------------------------------------------------------------------------------
	//! `f`'s AI looks past what it holds against you: passing, not Hunted, and not within the stand
	//! radius of the lab during the final stand.
	bool Covers(int playerId, int f)
	{
		if (!IsPassing(playerId, f) || RZ_Reputation.IsHunted(m_GM.GetTier(playerId, f)))
			return false;

		return !InStandRadius(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected bool InStandRadius(int playerId)
	{
		if (!m_GM.HasLab() || !m_GM.GetCure() || !m_GM.GetCure().IsFinalStand())
			return false;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		return me && RZ_Util.DistXZ(me.GetOrigin(), m_GM.GetLabPosition()) <= m_Settings.m_fStandRadius;
	}

	// ================================================================ Claims and looks
	//------------------------------------------------------------------------------------------------
	//! Can this player claim to be one of `f` at all? "" if so, else why not (in their voice).
	string CannotClaim(int playerId, int f)
	{
		if (!IsOn())
			return "Off";

		RZ_ETier tier = m_GM.GetTier(playerId, f);
		if (RZ_Reputation.IsHunted(tier))
			return "They know a Hunted face. The kit buys you nothing.";

		if (KitLevel(playerId, f) <= 0)
			return "You're not in their kit.";

		if (IsOwnColours(playerId, f))
			return "It's your own gear. They know you.";

		if (Tells(playerId) >= 2)
			return "Too much about you is wrong. Nobody would believe it.";

		if (m_GM.GetProfile(playerId).m_aCatches[f] >= Math.Max(1, m_Settings.m_iCatchesPerLife))
			return "Your description is on the board at every post.";

		if (IsBlown(playerId, f))
			return "They know your face now. Not for a while.";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The word at the wire today (Bastion's, changed at the 05:30 shift; read on its secure net).
	string WordToday()
	{
		array<string> words = {"KETTLE", "LANTERN", "HARROW", "PIKESTAFF", "MILLRACE", "FLINT", "ORCHARD", "CANDLE", "BRIDLE", "SPARROW", "TALLOW", "HOLLY"};
		int day = RZ_SeasonClock.Day();
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + day * 131 + 7);
		return words[rng.Int(0, words.Count())];
	}

	//------------------------------------------------------------------------------------------------
	//! Heard on Bastion's secure net (or told by a friend): +1 at Bastion's looks today.
	void LearnWord(int playerId)
	{
		m_mWord.Set(Ident(playerId), RZ_SeasonClock.Day());
	}

	//------------------------------------------------------------------------------------------------
	//! The best cover detail this player can say to `f` ("" none): the word at the wire, a crew name,
	//! a real name (a leader of theirs met this life), or the mission's paper.
	string CoverDetail(int playerId, int f, string paper)
	{
		if (!paper.IsEmpty())
			return paper;

		if (f == RZ_EFaction.MILITARY)
		{
			int day;
			if (m_mWord.Find(Ident(playerId), day) && day == RZ_SeasonClock.Day())
				return "Word: " + WordToday();
		}

		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (f == RZ_EFaction.BANDITS && !profile.m_sCrewName.IsEmpty())
			return "Crew name: " + profile.m_sCrewName;

		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			array<RZ_Leader> met = {};
			leaders.ListMet(playerId, met);
			foreach (RZ_Leader l : met)
			{
				if (l.m_eFaction == f)
					return "Name: " + l.m_sShort;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! "They know your face": Watched or Uneasy with them, or a leader of theirs met undisguised this life.
	bool KnowsFace(int playerId, int f)
	{
		RZ_ETier tier = m_GM.GetTier(playerId, f);
		if (tier == RZ_ETier.WATCHED || tier == RZ_ETier.UNEASY)
			return true;

		RZ_Leaders leaders = m_GM.GetLeaders();
		if (!leaders)
			return false;

		array<RZ_Leader> met = {};
		leaders.ListMet(playerId, met);
		foreach (RZ_Leader l : met)
		{
			if (l.m_eFaction == f)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Dark: low light where they stand (as stealth reads it), else 20:30-05:00, or fog.
	bool IsDark(IEntity character)
	{
		float light = RZ_Senses.GetIllumination(character);
		if (light >= 0)
			return light < 0.25;

		float hour = RZ_GameTime.GetHour();
		if (hour >= 20.5 || (hour >= 0 && hour < 5))
			return true;

		RZ_Environment env = m_GM.GetEnvironment();
		return env && env.IsFoggy();
	}

	//------------------------------------------------------------------------------------------------
	//! Looks already passed at `place` this visit (each is -1 at the next).
	int PassedHere(int playerId, string place)
	{
		int n;
		m_mPassed.Find(Ident(playerId) + "|" + place, n);
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A player of theirs points this claimant out (dialogue's point-out): -1 at this look, two at most.
	void PointOut(int playerId, int f)
	{
		string key = Ident(playerId) + "|" + f.ToString();
		int n;
		m_mPointed.Find(key, n);
		m_mPointed.Set(key, Math.Min(2, n + 1));
	}

	//------------------------------------------------------------------------------------------------
	int PointedOut(int playerId, int f)
	{
		int n;
		m_mPointed.Find(Ident(playerId) + "|" + f.ToString(), n);
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A look by `f` at this player (RZ_Checks.LookScore) at `place`, against `difficulty`. Pass: they
	//! pass for `seconds` (0: while the place stands). Fail: caught. Returns the result (Short counts
	//! as a fail: there's no halfway at a look).
	bool Look(int playerId, int f, int difficulty, string place, string paper, float seconds)
	{
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me)
			return false;

		string tags;
		bool instant;
		int score = RZ_Checks.LookScore(m_GM, playerId, f, place, paper, tags, instant);
		bool pass = !instant && score >= difficulty;
		RZ_Log.Info(string.Format("Disguise: %1 look by %2 at %3: %4 of %5 (%6)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), place, score, difficulty, Pass(pass)));
		m_mPointed.Remove(Ident(playerId) + "|" + f.ToString());
		if (!pass)
		{
			Caught(playerId, f, me, "They look at you a moment too long.");
			return false;
		}

		string key = Ident(playerId) + "|" + place;
		int n;
		m_mPassed.Find(key, n);
		m_mPassed.Set(key, n + 1);
		float until = m_fClock + 86400;
		if (seconds > 0)
			until = m_fClock + seconds;

		m_mPassUntil.Set(Ident(playerId) + "|" + f.ToString(), until);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Pass(bool pass)
	{
		if (pass)
			return "pass";

		return "caught";
	}

	//------------------------------------------------------------------------------------------------
	//! What they'll see, before you commit, as one owner-only notice: "[Full kit +2] [Watched -1]
	//! Pass as one of them: 2 of 2." Once a minute per place.
	void ShowComing(int playerId, int f, int difficulty, string place, string paper, string title)
	{
		if (!Tell(playerId, "coming" + place, NOTICE_GAP))
			return;

		string tags;
		bool instant;
		int score = RZ_Checks.LookScore(m_GM, playerId, f, place, paper, tags, instant);
		string result = string.Format("Pass as one of them: %1 of %2.", score, difficulty);
		if (instant)
			result = "Pass as one of them: they'd see through it.";

		m_GM.Say(playerId, title, f, tags + " " + result);
	}

	// ================================================================ Caught, exposed, held
	//------------------------------------------------------------------------------------------------
	//! Caught face to face: blown 60 minutes, one of three catches, +3 Infamy (capped, never into 15
	//! on its own), the kit taken, and held for a moment while they deal with you (never while one of
	//! their fights or grudges is on you: then it's their grudge, and the fight carries on).
	void Caught(int playerId, int f, IEntity me, string line)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		string ident = profile.m_sIdentity;
		m_mPassUntil.Remove(ident + "|" + f.ToString());
		profile.m_aBlownUntil[f] = profile.m_fLifeSeconds + Math.Max(1, m_Settings.m_fDisguiseBlownMinutes) * 60;
		profile.m_aCatches[f] = profile.m_aCatches[f] + 1;
		int catches = profile.m_aCatches[f];

		// +3, at most 6 an hour, never from below 15 to 15 or more on its own
		string hourKey = ident + "|" + f.ToString();
		float start;
		if (!m_mHourStart.Find(hourKey, start) || m_fClock - start > 3600)
		{
			m_mHourStart.Set(hourKey, m_fClock);
			m_mHourInfamy.Set(hourKey, 0);
		}

		int used;
		m_mHourInfamy.Find(hourKey, used);
		int infamy = Math.Min(m_Settings.m_iImpersonationInfamy, Math.Max(0, m_Settings.m_iImpersonationCap - used));
		int now = profile.GetInfamy(f);
		if (now < 15)
			infamy = Math.Min(infamy, 14 - now);

		if (infamy > 0)
		{
			m_GM.AddInfamy(playerId, f, infamy, "caught in their kit", RZ_RepFlag.SHARE);
			m_mHourInfamy.Set(hourKey, used + infamy);
		}

		string taken = TakeKit(me, f);
		string text = line + " " + CaughtLine(f);
		if (!taken.IsEmpty())
			text += " " + taken;

		if (infamy > 0)
			text += string.Format(" +%1 Infamy with %2.", infamy, RZ_Factions.GetNameInText(f));

		if (catches >= Math.Max(1, m_Settings.m_iCatchesPerLife))
			text += " \"Your description is on the board at every post.\"";

		// Held: a moment's stand-down while they deal with you, never over a fight
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility && me && !hostility.IsFightFor(playerId, f, me.GetOrigin(), 300))
			hostility.StandDown(playerId, f, HELD_SECONDS, false);
		else if (hostility && me)
			hostility.AddGrudge(playerId, f, me.GetOrigin());

		text += Penalty(playerId, f, me);
		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), text);
		Public(f);
		RZ_Log.Info(string.Format("Disguise: %1 caught by %2 (catch %3 this life)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), catches));
	}

	//------------------------------------------------------------------------------------------------
	//! Exposed at range (firing near them, a hit): blown, nothing else.
	void Expose(int playerId, int f, string why)
	{
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		m_mPassUntil.Remove(profile.m_sIdentity + "|" + f.ToString());
		if (IsBlown(playerId, f))
			return;

		profile.m_aBlownUntil[f] = profile.m_fLifeSeconds + Math.Max(1, m_Settings.m_fDisguiseBlownMinutes) * 60;
		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("%1 They know who's inside %2's kit now.", why, RZ_Factions.GetNameInText(f)));
		RZ_Log.Info(string.Format("Disguise: %1 exposed to %2 (%3)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), why));
	}

	//------------------------------------------------------------------------------------------------
	protected static string CaughtLine(int f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "\"Name? Doesn't matter. You're on the board now.\"";
			case RZ_EFaction.BANDITS: return "\"Nobody calls you anything. That's the problem. Wallet or teeth?\"";
			case RZ_EFaction.INSTITUTE: return "\"It's numbered. Please take the coat off. Slowly.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The kit is taken (Confiscation on): their pieces you wear. VERIFY: deleting a worn item.
	protected string TakeKit(IEntity me, int f)
	{
		if (!m_Settings.m_bConfiscation || !me)
			return "";

		array<IEntity> pieces = {};
		RZ_Util.GetWornMatching(me, m_aWords[f], pieces);
		RZ_Util.GetWornMatching(me, m_aHeadWords[f], pieces);
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(me.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv || pieces.IsEmpty())
			return "";

		int taken = 0;
		foreach (IEntity piece : pieces)
		{
			if (piece && inv.TryDeleteItem(piece))
				taken++;
		}

		if (taken == 0)
			return "";

		if (f == RZ_EFaction.MILITARY)
			return "\"Army property.\" The kit goes in the crate.";

		return "The kit is taken off you.";
	}

	//------------------------------------------------------------------------------------------------
	//! Each faction's way of dealing with a liar: Bastion searches, the Red Hand fines or beats,
	//! Halcyon walks you off politely.
	protected string Penalty(int playerId, int f, IEntity me)
	{
		if (f == RZ_EFaction.MILITARY)
		{
			RZ_Checkpoints posts = m_GM.GetCheckpoints();
			int found = 0;
			if (posts && me)
				found = posts.Search(me);

			if (found > 0)
				return string.Format(" They search you while they're at it: %1 things into the crate.", found);

			return "";
		}

		if (f == RZ_EFaction.BANDITS)
		{
			int fine = Math.Max(0, m_Settings.m_iRedHandFine);
			if (m_GM.SpendFavor(playerId, f, fine))
				return string.Format(" The toll for liars: %1 Red Hand Favor.", fine);

			// A beating, each catch its own key; never with an infected near: then Infamy instead
			if (me && m_GM.GetSpawner().IsZombieNear(me.GetOrigin(), 40))
			{
				m_GM.AddInfamy(playerId, f, 2, "liar's toll", RZ_RepFlag.SHARE);
				return " No time for a beating with that lot close. +2 Infamy instead.";
			}

			string key = "rh_beating_" + m_GM.GetProfile(playerId).m_aCatches[f].ToString();
			string no = RZ_HealthCost.Take(m_GM, playerId, me, key, m_Settings.m_fBeatingShare, 0.3);
			if (no.IsEmpty())
				return " No Favor, so it's teeth.";

			return "";
		}

		return " They walk you off the gate, very politely.";
	}

	//------------------------------------------------------------------------------------------------
	//! The open channel tells it without a name (the Red Hand laughs; Halcyon reads it out).
	protected void Public(int f)
	{
		string line = "";
		if (f == RZ_EFaction.BANDITS)
			line = "\"Some clown tried to pay the toll with a salute. Drinks are on him.\"";
		else if (f == RZ_EFaction.INSTITUTE)
			line = "\"Security notice: a coat was presented by someone not in the record. Please check numbers.\"";
		else if (f == RZ_EFaction.MILITARY)
			line = "\"All stations: civilian in our kit. Check faces. Bastion out.\"";

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life && !line.IsEmpty())
			life.OpenChannelLine(f, line);
	}

	// ================================================================ Breakers
	//------------------------------------------------------------------------------------------------
	//! While passing: a leader of theirs within 15 m, or inside a stronghold's or the lab's wire,
	//! and you're caught.
	protected void Breakers(int playerId, IEntity me)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!IsPassing(playerId, f))
				continue;

			RZ_Leaders leaders = m_GM.GetLeaders();
			RZ_Leader l;
			if (leaders)
				l = leaders.FindBodyNear(me.GetOrigin(), LEADER_RADIUS);

			if (l && l.m_eFaction == f)
			{
				Caught(playerId, f, me, "\"I know every face under my command, and yours isn't one of them.\"");
				continue;
			}

			if (InsideWire(f, me.GetOrigin()))
				Caught(playerId, f, me, "Everyone in here knows everyone.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Inside a stronghold's or (Halcyon) the lab's wire.
	bool InsideWire(int f, vector pos)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding h = territory.GetStronghold(f);
			if (h && RZ_Util.DistXZ(h.m_vPos, pos) <= WIRE_RADIUS)
				return true;
		}

		return f == RZ_EFaction.INSTITUTE && m_GM.HasLab() && RZ_Util.DistXZ(m_GM.GetLabPosition(), pos) <= WIRE_RADIUS;
	}

	//------------------------------------------------------------------------------------------------
	//! Within `radius` of their people or holdings: their holdings, road posts and a mission's watchers.
	bool NearTheirs(int f, vector pos, float radius)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive() && territory.FindHolding(f, pos, radius))
			return true;

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(f, pos, radius, near);
			if (!near.IsEmpty())
				return true;
		}

		foreach (int i, IEntity w : m_aWatchers)
		{
			if (w && m_aWatcherFaction[i] == f && RZ_Util.DistXZ(w.GetOrigin(), pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A mission's own people watch for its trick (weak references; they go with the mission).
	void AddWatcher(IEntity e, int f)
	{
		if (!e)
			return;

		for (int i = m_aWatchers.Count() - 1; i >= 0; i--)
		{
			if (!m_aWatchers[i])
			{
				m_aWatchers.Remove(i);
				m_aWatcherFaction.Remove(i);
			}
		}

		m_aWatchers.Insert(e);
		m_aWatcherFaction.Insert(f);
	}

	//------------------------------------------------------------------------------------------------
	//! From the gunfire hook: firing within 150 m of their people with no infected within 40 m of you.
	void OnPlayerShot(int playerId, vector pos)
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!IsPassing(playerId, f) || !NearTheirs(f, pos, KNOW_RADIUS))
				continue;

			if (m_GM.GetSpawner().IsZombieNear(pos, 40))
				continue;

			Expose(playerId, f, "A shot near their people.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.AddInfamy: only an act (ACT, never a share) done while passing as
	//! `f` within 150 m of their people or holdings doubles; within 35 m it's a catch, else exposed.
	int ScaleInfamy(int playerId, int f, int amount)
	{
		if (amount <= 0 || !IsPassing(playerId, f))
			return amount;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!me || !NearTheirs(f, me.GetOrigin(), KNOW_RADIUS))
			return amount;

		if (NearTheirs(f, me.GetOrigin(), CATCH_RADIUS))
			Caught(playerId, f, me, "They saw who did it.");
		else
			Expose(playerId, f, "They saw who did it.");

		return amount * 2;
	}
}
