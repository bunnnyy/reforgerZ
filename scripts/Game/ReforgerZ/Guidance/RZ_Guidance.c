//! Quiet in-world guidance for new players (docs/DESIGN.md, "New-player guidance").
//!
//! The world teaches itself through people and things, not tutorial pop-ups:
//! - an experience level per player (0 brand new .. 3 veteran) from lives that lasted
//!   long enough to count, with a refresher after a long time away. Guidance fades with
//!   it but never disappears: critical tips keep a short line;
//! - contextual tips: when something happens, the nearest plausible speaker says one
//!   line in their voice (a leader or safehouse occupant, else a wanderer, else the radio,
//!   else a plain hint). The table is data (RZ_GuidanceTip);
//! - a gentler first 15 minutes (fewer zombies near a brand-new player);
//! - the first-life goal "Find the Haven" (RZ_Objectives);
//! - the world half (RZ_GuidanceWorld): spawn scenarios, the wind-up radio and its open
//!   channels, voices near faction seats, safehouse flyers, gossip, a wanderer who finds you.
//!
//! Server only. Event-driven where a hook exists, otherwise a light poll every 10 s.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Guidance.
[BaseContainerProps()]
class RZ_GuidanceSettings
{
	[Attribute("1", desc: "New-player guidance on: tips, the first-life goal, the gentle start")]
	bool m_bEnabled;

	[Attribute("10", desc: "Minutes a life must last to count as experience")]
	float m_fMeaningfulMinutes;

	[Attribute("1", desc: "Counted lives for level 1 (learning). Below this a player is brand new (level 0)")]
	int m_iLivesLearning;

	[Attribute("3", desc: "Counted lives for level 2 (familiar)")]
	int m_iLivesFamiliar;

	[Attribute("8", desc: "Counted lives for level 3 (veteran: only critical tips, short)")]
	int m_iLivesVeteran;

	[Attribute("30", desc: "Real days away that bring guidance back for the next two counted lives (a refresher). 0 = never")]
	int m_iRefresherDays;

	[Attribute("0.5", desc: "Zombie density near a brand-new player during the gentle start (1 = off)")]
	float m_fGentleMult;

	[Attribute("15", desc: "Minutes of each brand-new life with the gentle density")]
	float m_fGentleMinutes;

	[Attribute("90", desc: "Seconds between two tips to the same player (critical tips don't wait)")]
	float m_fTipGapSeconds;

	[Attribute("45", desc: "Heat around a player that counts as too loud (the 'loud' tip)")]
	float m_fLoudHeat;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Wind-up radio item put in every new life's inventory (or at their feet). Its prefab name must contain 'radio' or it needs a radio component, so RZ_Radio counts it. Empty = none given", "et")]
	ResourceName m_sRadioPrefab;

	[Attribute("12", desc: "Minutes between open-channel radio messages for a brand-new player (x1.5, x2.5, x3.5 with experience). 0 = off")]
	float m_fRadioMinutes;

	[Attribute("1", desc: "Spawn scenarios: every new life starts near a small clue (a note, a flyer, a body, a radio)")]
	bool m_bScenarios;

	[Attribute("20", desc: "Minutes a scenario clue stays; it's removed after this once nobody is within 30 m, and after twice this anyway")]
	float m_fClueMinutes;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Crashed car (optional scenery for the back-road scenario; a note lies next to it unless it has RZ_ClueComponent)", "et")]
	ResourceName m_sCarWreckPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Scratched door or wall panel (optional, basement scenario; readable with RZ_ClueComponent + RZ_ReadClueAction, else the note is used)", "et")]
	ResourceName m_sDoorPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon flyer (optional, church scenario and safehouse walls; falls back to Puzzles > Note)", "et")]
	ResourceName m_sFlyerPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Table radio (optional, farmhouse scenario; falls back to the wind-up radio item)", "et")]
	ResourceName m_sTableRadioPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Dead raider with a red hand on his jacket (optional, attic scenario; falls back to Puzzles > Body)", "et")]
	ResourceName m_sRaiderBodyPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Dead Bastion soldier (optional, barricade scenario; falls back to Puzzles > Body)", "et")]
	ResourceName m_sSoldierBodyPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion barricade (optional scenery for the barricade scenario)", "et")]
	ResourceName m_sBarricadePrefab;

	[Attribute("1", desc: "Halcyon flyers on the walls of Haven safehouses (they mention the Haven clinic and Bastion's curfew)")]
	bool m_bSafehouseFlyers;

	[Attribute("30", desc: "Minutes between a Bastion loudspeaker or a Red Hand taunt near one of their seats, per player and faction (doubled for veterans). 0 = off")]
	float m_fShoutMinutes;

	[Attribute("1", desc: "A Courier or Scavenger wanderer comes looking for brand-new players in their first 15 minutes")]
	bool m_bWandererFinds;

	[Attribute(desc: "Tips: one line from the nearest plausible speaker when something happens. Empty = the built-in table (RZ_GuidanceTips)")]
	ref array<ref RZ_GuidanceTip> m_aTips;
}

//------------------------------------------------------------------------------------------------
//! Runtime state per player for the current life.
class RZ_GuidePlayer
{
	float m_fLastTip = -1000;
	ref array<string> m_aLifeTips = {};
	float m_fSampleTimer;
	float m_fRadioTimer;
	int m_iRadioStep;
	float m_fNudgeAt;
	bool m_bNudged;
}

//------------------------------------------------------------------------------------------------
class RZ_Guidance
{
	static const string PLAIN_TITLE = "Remember";
	static const string RADIO_TITLE = "Wind-up radio";

	static const int LEVEL_NEW = 0;
	static const int LEVEL_VETERAN = 3;

	protected static const float POLL_SECONDS = 10;
	protected static const float SPEAKER_RADIUS = 40;		// a leader or safehouse occupant this close says the tip
	protected static const float WANDERER_RADIUS = 60;
	protected static const float SPECIAL_RADIUS = 80;		// players this close to a special's death hear about samples
	protected static const float SAMPLE_CHECK = 30;
	protected static const float PERIMETER_AHEAD = 150;	// the tip comes this far before the loudspeaker does
	protected static const float CHECKPOINT_AHEAD = 350;
	protected static const float UNIX_INTERVAL = 300;
	protected static const int DAY_SECONDS = 86400;
	protected static const int REFRESH_LIVES = 2;

	protected RZ_GameModeComponent m_GM;
	protected RZ_GuidanceSettings m_Settings;
	protected ref RZ_GuidanceWorld m_World;
	protected ref array<ref RZ_GuidanceTip> m_aTips = {};
	protected ref map<int, ref RZ_GuidePlayer> m_mPlayers = new map<int, ref RZ_GuidePlayer>();
	protected ref map<string, float> m_mTipAt = new map<string, float>();
	protected ref array<vector> m_aGentlePos = {};
	protected ref array<float> m_aGentleMult = {};
	protected float m_fClock;
	protected float m_fPollTimer;
	protected float m_fUnixTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_Guidance(RZ_GameModeComponent gm, RZ_GuidanceSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		if (m_Settings.m_aTips && !m_Settings.m_aTips.IsEmpty())
		{
			foreach (RZ_GuidanceTip t : m_Settings.m_aTips)
			{
				m_aTips.Insert(t);
			}
		}
		else
		{
			RZ_GuidanceTips.CreateDefaults(m_aTips);
		}

		m_World = new RZ_GuidanceWorld(gm, this, settings);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	RZ_GuidanceSettings GetSettings() { return m_Settings; }
	float GetClock() { return m_fClock; }

	//------------------------------------------------------------------------------------------------
	//! Real-world seconds since 1970, 0 if unknown.
	static int UnixNow()
	{
		return System.GetUnixTime(); // VERIFY: System.GetUnixTime() (real-world time, used for the 30-day refresher)
	}

	// ================================================================ Experience
	//------------------------------------------------------------------------------------------------
	//! 0 brand new, 1 learning, 2 familiar, 3 veteran. A refresher lowers it by two.
	int GetLevel(RZ_PlayerProfile p)
	{
		int lives = p.m_iGuideLives;
		int level = LEVEL_NEW;
		if (lives >= m_Settings.m_iLivesVeteran)
			level = LEVEL_VETERAN;
		else if (lives >= m_Settings.m_iLivesFamiliar)
			level = 2;
		else if (lives >= m_Settings.m_iLivesLearning)
			level = 1;

		if (p.m_iGuideRefreshLeft > 0)
			level = Math.Max(LEVEL_NEW, level - 2);

		return level;
	}

	//------------------------------------------------------------------------------------------------
	int GetPlayerLevel(int playerId)
	{
		return GetLevel(m_GM.GetProfile(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! The first meaningful life gets the goal "Find the Haven" (RZ_Objectives).
	bool WantsHavenGoal(RZ_PlayerProfile p)
	{
		return m_Settings.m_bEnabled && p.m_iGuideLives == 0;
	}

	//------------------------------------------------------------------------------------------------
	//! From OnPlayerSpawned, after the profile started its new life and before the objective is
	//! picked. Checks for a refresher (a long time away) and resets this life's tips.
	void BeginLife(int playerId, RZ_PlayerProfile p)
	{
		p.EnsureArrays();
		int now = UnixNow();
		int away = m_Settings.m_iRefresherDays * DAY_SECONDS;
		if (now > 0 && away > 0 && p.m_iGuideLastUnix > 0 && p.m_iGuideLives > 0 && now - p.m_iGuideLastUnix >= away)
		{
			p.m_iGuideRefreshLeft = REFRESH_LIVES;
			p.m_aGuideSeen.Clear();
			RZ_Log.Info(string.Format("Guidance: refresher for %1 (away %2 days).", RZ_Util.PlayerName(playerId), (now - p.m_iGuideLastUnix) / DAY_SECONDS));
		}

		if (now > 0)
			p.m_iGuideLastUnix = now;

		m_mPlayers.Set(playerId, new RZ_GuidePlayer());
	}

	//------------------------------------------------------------------------------------------------
	//! From OnPlayerSpawned, after the player was told who they are this life: the wind-up
	//! radio and the spawn scenario.
	void OnPlayerSpawned(int playerId, IEntity ent)
	{
		if (!m_Settings.m_bEnabled || !ent)
			return;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		m_World.OnPlayerSpawned(playerId, ent, p, GetState(playerId), GetLevel(p));
	}

	//------------------------------------------------------------------------------------------------
	//! A guidance clue or flyer was read (RZ_PuzzleActionHelper.RouteRead).
	void HandleRead(int playerId, int idx, IEntity owner)
	{
		m_World.HandleRead(playerId, idx, owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's flyer for a safehouse wall (RZ_Safehouses spawns it with the occupants). Null if off.
	IEntity SpawnSafehouseFlyer(int safehouseIdx, vector center)
	{
		return m_World.SpawnSafehouseFlyer(safehouseIdx, center);
	}

	//------------------------------------------------------------------------------------------------
	//! A line of faction gossip for a wanderer's "Talk", "" if none this time.
	string WandererGossip(int playerId, int role, vector from)
	{
		return m_World.WandererGossip(playerId, role, from);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_GuidePlayer GetState(int playerId)
	{
		RZ_GuidePlayer st;
		if (!m_mPlayers.Find(playerId, st))
		{
			st = new RZ_GuidePlayer();
			m_mPlayers.Set(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! A life counts once it has lasted m_fMeaningfulMinutes.
	protected void CountLife(RZ_PlayerProfile p)
	{
		if (p.m_bGuideCounted || p.m_fLifeSeconds < m_Settings.m_fMeaningfulMinutes * 60)
			return;

		p.m_bGuideCounted = true;
		p.m_iGuideLives++;
		if (p.m_iGuideRefreshLeft > 0)
			p.m_iGuideRefreshLeft--;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! Server tick, after the objectives (which advance each life's clock).
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		m_aGentlePos.Clear();
		m_aGentleMult.Clear();

		m_fPollTimer += dt;
		bool poll = m_fPollTimer >= POLL_SECONDS;
		if (poll)
			m_fPollTimer = 0;

		m_fUnixTimer += dt;
		bool stamp = m_fUnixTimer >= UNIX_INTERVAL;
		if (stamp)
			m_fUnixTimer = 0;

		int now = 0;
		if (stamp)
			now = UnixNow();

		bool night = false;
		if (poll)
			night = RZ_GameTime.IsNight();

		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			RZ_PlayerProfile p = m_GM.GetProfile(playerId);
			CountLife(p);
			if (stamp && now > 0)
				p.m_iGuideLastUnix = now;

			if (!m_Settings.m_bEnabled)
				continue;

			int level = GetLevel(p);
			if (level == LEVEL_NEW && m_Settings.m_fGentleMult < 1 && p.m_fLifeSeconds < m_Settings.m_fGentleMinutes * 60)
			{
				m_aGentlePos.Insert(ent.GetOrigin());
				m_aGentleMult.Insert(Math.Max(0, m_Settings.m_fGentleMult));
			}

			if (poll)
			{
				RZ_GuidePlayer st = GetState(playerId);
				Poll(playerId, ent, st, night);
				m_World.Poll(playerId, ent, p, st, level, POLL_SECONDS);
			}
		}

		if (m_Settings.m_bEnabled)
			m_World.Tick(players, poll);
	}

	//------------------------------------------------------------------------------------------------
	//! Zombie density multiplier for the spawner: the lowest gentle multiplier of any
	//! brand-new player within radius of pos, 1 if none.
	float GetDensityMult(vector pos, float radius)
	{
		float mult = 1;
		foreach (int i, vector g : m_aGentlePos)
		{
			if (m_aGentleMult[i] < mult && RZ_Util.DistXZ(pos, g) <= radius)
				mult = m_aGentleMult[i];
		}

		return mult;
	}

	//------------------------------------------------------------------------------------------------
	//! Triggers without an event: the line, noise, night, a safehouse, samples in the pack.
	protected void Poll(int playerId, IEntity ent, RZ_GuidePlayer st, bool night)
	{
		vector pos = ent.GetOrigin();

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (perimeter && perimeter.IsActive())
		{
			if (perimeter.EdgeDistance(pos) < perimeter.GetWarnDistance() + PERIMETER_AHEAD || perimeter.GetCheckpointDistance(pos) < CHECKPOINT_AHEAD)
				Fire(playerId, "perimeter");
		}

		if (m_GM.GetHeat().GetHeat(pos) >= m_Settings.m_fLoudHeat)
			Fire(playerId, "loud");

		if (night)
			Fire(playerId, "night");

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && !houses.IsExiled(playerId))
		{
			RZ_Safehouse h = houses.Get(houses.FindAt(pos));
			if (h)
			{
				Fire(playerId, "safehouse", h.m_sName);
				if (h.m_bShort)
					Fire(playerId, "shortage", h.m_sName);
			}
		}

		st.m_fSampleTimer = st.m_fSampleTimer + POLL_SECONDS;
		if (st.m_fSampleTimer >= SAMPLE_CHECK && m_GM.HasLab() && !st.m_aLifeTips.Contains("samples"))
		{
			st.m_fSampleTimer = 0;
			array<IEntity> samples = {};
			RZ_LabHelper.FindSamples(ent, samples);
			if (!samples.IsEmpty())
				Fire(playerId, "samples", LabText(pos));
		}
	}

	// ================================================================ Events
	//------------------------------------------------------------------------------------------------
	void OnBitten(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (ent)
			Fire(playerId, "bitten", SafehouseText(ent.GetOrigin()));
	}

	//------------------------------------------------------------------------------------------------
	//! A special died at pos: players close by hear what its sample is for.
	void OnSpecialKilled(vector pos)
	{
		if (!m_Settings.m_bEnabled || !m_GM.HasLab())
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= SPECIAL_RADIUS)
				Fire(ids[i], "special", LabText(p.GetOrigin()));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Standing changed: the first rise with a faction, becoming Watched, becoming Hunted.
	void OnTierChanged(int playerId, RZ_EFaction faction, RZ_ETier before, RZ_ETier after)
	{
		string name = RZ_Factions.GetName(faction);
		int rankBefore = PositiveRank(before);
		int rankAfter = PositiveRank(after);
		if (rankAfter >= 1 && rankBefore >= 0 && rankAfter > rankBefore)
		{
			Fire(playerId, "tier", name, RZ_Reputation.GetTierName(after));
			return;
		}

		if (RZ_Reputation.IsHunted(after) && !RZ_Reputation.IsHunted(before))
		{
			Fire(playerId, "hunted", name);
			return;
		}

		if (after == RZ_ETier.WATCHED && before != RZ_ETier.WATCHED)
			Fire(playerId, "watched", name);
	}

	//------------------------------------------------------------------------------------------------
	//! 0 Stranger, 1 Known, 2 Trusted, 3 Champion, -1 anything mixed or negative.
	protected int PositiveRank(RZ_ETier tier)
	{
		switch (tier)
		{
			case RZ_ETier.STRANGER: return 0;
			case RZ_ETier.KNOWN: return 1;
			case RZ_ETier.TRUSTED: return 2;
			case RZ_ETier.CHAMPION: return 3;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! A puzzle clue (or a guidance note) was read.
	void OnClueRead(int playerId)
	{
		Fire(playerId, "clue");
	}

	//------------------------------------------------------------------------------------------------
	//! Public hook for systems that don't exist yet and for other mods:
	//! RZ_Guidance.Report(playerId, "hungry"), "car" (a fixable car nearby), "channel"
	//! (a secure channel nearby, Radio Operators), "shortage" (%1 the place), or any id
	//! added to the tip table. For "hungry" and "bitten", %1 defaults to the nearest safehouse.
	static void Report(int playerId, string tipId, string a = "", string b = "")
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetGuidance())
			return;

		RZ_Guidance guidance = gm.GetGuidance();
		if (a.IsEmpty() && (tipId == "hungry" || tipId == "bitten"))
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (ent)
				a = guidance.SafehouseText(ent.GetOrigin());
		}

		guidance.Fire(playerId, tipId, a, b);
	}

	// ================================================================ Tips
	//------------------------------------------------------------------------------------------------
	protected RZ_GuidanceTip FindTip(string id)
	{
		foreach (RZ_GuidanceTip t : m_aTips)
		{
			if (t && t.m_sId == id)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Says tip `tipId` to the player if it's due: not yet this life, not past its cooldown,
	//! not a once-only tip they've had, and not right after another tip. Past its level only
	//! a critical tip is said, and only its short line. True if something was said.
	bool Fire(int playerId, string tipId, string a = "", string b = "")
	{
		if (!m_Settings.m_bEnabled || playerId <= 0)
			return false;

		RZ_GuidanceTip tip = FindTip(tipId);
		if (!tip)
			return false;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return false;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		p.EnsureArrays();
		RZ_GuidePlayer st = GetState(playerId);
		if (st.m_aLifeTips.Contains(tipId))
			return false;

		if (tip.m_bOnce && p.m_aGuideSeen.Contains(tipId))
			return false;

		string key = p.m_sIdentity + ":" + tipId;
		float last;
		if (tip.m_fCooldownMinutes > 0 && m_mTipAt.Find(key, last) && m_fClock - last < tip.m_fCooldownMinutes * 60)
			return false;

		bool full = GetLevel(p) <= tip.m_iMaxLevel;
		if (!full && !tip.m_bCritical)
			return false;

		if (!tip.m_bCritical && m_fClock - st.m_fLastTip < m_Settings.m_fTipGapSeconds)
			return false;

		bool said;
		if (full)
		{
			said = Speak(playerId, ent, tip, a, b);
		}
		else
		{
			string brief = tip.m_sShort;
			if (brief.IsEmpty())
				brief = tip.m_sPlain;

			if (!brief.IsEmpty())
			{
				m_GM.NotifyPlayer(playerId, PLAIN_TITLE, string.Format(brief, a, b));
				said = true;
			}
		}

		if (!said)
			return false;

		st.m_aLifeTips.Insert(tipId);
		st.m_fLastTip = m_fClock;
		m_mTipAt.Set(key, m_fClock);
		if (tip.m_bOnce)
			p.m_aGuideSeen.Insert(tipId);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest plausible speaker: a leader (or the Haven's safehouse people) within 40 m,
	//! else a wanderer within 60 m, else the radio if they carry one, else a plain hint.
	protected bool Speak(int playerId, IEntity ent, RZ_GuidanceTip tip, string a, string b)
	{
		vector pos = ent.GetOrigin();

		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders)
		{
			RZ_Leader l = leaders.FindBodyNear(pos, SPEAKER_RADIUS);
			if (l && leaders.Attitude(l, playerId) > -2)
			{
				string leaderLine = tip.GetFactionLine(l.m_eFaction);
				if (!leaderLine.IsEmpty())
				{
					m_GM.NotifyPlayer(playerId, l.GetHeader(), string.Format(leaderLine, a, b));
					return true;
				}
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses && !tip.m_sHaven.IsEmpty() && !houses.IsExiled(playerId))
		{
			RZ_Safehouse h = houses.Get(houses.FindPeopleNear(pos, SPEAKER_RADIUS));
			if (h)
			{
				m_GM.NotifyPlayer(playerId, "Someone at the safehouse in " + h.m_sName, string.Format(tip.m_sHaven, a, b));
				return true;
			}
		}

		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (wanderers && !tip.m_sWanderer.IsEmpty())
		{
			string who = wanderers.SpeakerNear(playerId, pos, WANDERER_RADIUS);
			if (!who.IsEmpty())
			{
				m_GM.NotifyPlayer(playerId, who, string.Format(tip.m_sWanderer, a, b));
				return true;
			}
		}

		if (!tip.m_sRadio.IsEmpty() && RZ_Radio.HasRadio(m_GM, playerId, ent))
		{
			m_GM.NotifyPlayer(playerId, RADIO_TITLE, string.Format(tip.m_sRadio, a, b));
			return true;
		}

		string plain = tip.m_sPlain;
		if (plain.IsEmpty())
			plain = tip.m_sShort;

		if (plain.IsEmpty())
			return false;

		m_GM.NotifyPlayer(playerId, PLAIN_TITLE, string.Format(plain, a, b));
		return true;
	}

	// ================================================================ Places
	//------------------------------------------------------------------------------------------------
	//! The nearest safehouse's town ("Morton"), or a generic phrase if there are none.
	string SafehouseText(vector from)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			RZ_Safehouse h = houses.Get(houses.FindNearest(from));
			if (h)
				return h.m_sName;
		}

		return "the nearest town";
	}

	//------------------------------------------------------------------------------------------------
	//! "Morton, 1.2 km NE" for the lab, "" without one.
	string LabText(vector from)
	{
		if (!m_GM.HasLab())
			return "";

		string place = "the lab town";
		RZ_Settlement s = m_GM.GetSettlement(m_GM.GetLabSettlement());
		if (s)
			place = s.m_sName;

		return place + ", " + RZ_Util.DirectionText(from, m_GM.GetLabPosition());
	}
}
