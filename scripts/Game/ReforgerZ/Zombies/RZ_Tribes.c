//! The infected tribes (docs/INFECTED.md, "Tribes"): packs in the wild land, each with a lair, a
//! range, trails and a leader, living by the time of day. As data on the server: the season's seeds,
//! the daily rhythm (the day party, the dusk call, the night's hunt, the dawn walk and count),
//! hunger per counted night, the taken, the ones who ran, takings and hearing as data, post-feed
//! sleep and weakened tribes. Their bodies (near players only) are RZ_TribeBodies; how the bodies
//! move and hunt is the behaviour layer (Zombies/Behaviour). The life cycle is RZ_TribeLife (with
//! RZ_TribeBorders and RZ_TribeWays). Server only.

//------------------------------------------------------------------------------------------------
//! Zombies > Tribes. Off: no land grid and no tribes; the countryside's roamers and heat hordes are
//! as before. The agent budget here also governs the crawlers (Zombies > Crawlers).
[BaseContainerProps()]
class RZ_TribeSettings
{
	[Attribute("1", desc: "Tribes: read the land grid (about 5 minutes the first time on a big map, then cached), seed packs in the wild land with lairs, scouts, day parties, dusk calls and night hunts, raise the engine's active-AI limit, send loose ones walking somewhere, bring heat hordes from the nearest town. Off: none of this; the countryside's roamers and heat hordes as before")]
	bool m_bEnabled;

	[Attribute("0", desc: "Slots: tribes on this map whatever its wild land, 1 to 15 (the runtime test uses 2 on Arland). 0 = from the land, at Wild km2 per slot")]
	int m_iSlots;

	[Attribute("5", desc: "Wild km2 per slot: one tribe per this much wild land (phase 0 measured Everon at 24 km2: 5 slots; Cain 20 km2: 4; Arland under the floor: none)")]
	float m_fWildKm2PerSlot;

	[Attribute("100", desc: "Agent budget: living infected on the server, counting every group member (with Crawlers or Tribes on). Phase 0 measured 100 agents at a p99 frame of 11.6 ms. Mission infected count toward it but are never removed for it")]
	int m_iAgentBudget;

	[Attribute("28", desc: "Cluster share: agents near one player cluster (players within 600 m share one), or the budget divided among the clusters if that's less")]
	int m_iClusterShare;

	[Attribute("8", desc: "Agents spawned a tick at most (not entries), so a full town or a horde arrives over a few ticks")]
	int m_iAgentsPerTick;

	[Attribute("160", desc: "Active-AI limit: the engine's limit on active AI, for all AI (faction AI, wanderers and trucks share it with the infected). Raised to this at start when it's lower (default 128). 0 = leave the engine's")]
	int m_iActiveAILimit;

	[Attribute("20", UIWidgets.Slider, "Land grid: land squares (250 m) read a second while the grid is first built. Sea squares cost almost nothing, and a second's reading stops after 8 ms whatever this says", "1 100 1")]
	int m_iLandCellsPerSecond;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Alpha prefabs: a tribe's leader, a heavy character (Bacon's Zombie_Heavy_Tank, about 4.9 m/s). Put in a one-member runtime group. Empty: the Special prefabs; both empty: the alpha is data only", "et")]
	ref array<ResourceName> m_aAlphaPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Guard prefabs: the alpha's guard (Bacon's military and helmeted characters). Empty: the Special prefabs", "et")]
	ref array<ResourceName> m_aGuardPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Hunter prefabs: grown members (Bacon's civilian and military characters). Empty: the Common prefabs", "et")]
	ref array<ResourceName> m_aHunterPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Scout prefabs: the lightest hunters, on the ridges. Empty: the Hunter prefabs", "et")]
	ref array<ResourceName> m_aScoutPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Young prefabs: the recently taken and the ones who ran. Empty: the Hunter prefabs", "et")]
	ref array<ResourceName> m_aYoungPrefabs;

	[Attribute("0.2", desc: "Day party share: of a tribe's members, the hunters and young out by day in twos and threes (never the alpha, a guard or a scout). About twice this when hungry, never over half; a tribe under 8 sends none. A quarter at most is the design; 0 = no day parties")]
	float m_fDayPartyShare;

	[Attribute("2", desc: "Day party group, smallest")]
	int m_iDayGroupMin;

	[Attribute("3", desc: "Day party group, largest (4 at most)")]
	int m_iDayGroupMax;

	[Attribute("6", desc: "Day party bodies near one player cluster at most, from every tribe together, out of the cluster's share")]
	int m_iDayBodies;

	[Attribute("20", desc: "Counted night minutes: a game night (20:30-05:00) counts for hunger and post-feed sleep only if someone was online this many real minutes of it")]
	float m_fCountedNightMinutes;

	[Attribute("120", desc: "Scout sight (m): a scout sees a player this far at normal visibility (stealth's sight radius shortens it for a hidden player). 5 seconds in sight and in terrain line of sight and it calls")]
	float m_fScoutSight;

	[Attribute("1", desc: "Life cycle: successions with the guard fight and splinters, lone ones, ageing and challenges (the Clash), borders and their fights (Border Nights), splits and clans, lair moves and displacement, vacuums, new tribes, burned land, starvation, fear places, habits, marks, worn trails, bonds and echoes. Off: tribes as in phase 2 (one of the guard takes the call at dawn; a tribe that dies out leaves its slot free)")]
	bool m_bLifeCycle;

	[Attribute("3", desc: "Hunger a counted night: how much a tribe's hunger (0-100) rises each counted night it took nobody (2 more with its alpha in Decline, 2 more while restless). The design said 5; the tribe simulator on the measured land had tribes hungry 60-90% of the time and starving 8-13 times a season on Everon at 5, so 3 (tools/tribe_sim.py)")]
	float m_fHungerPerNight;

	[Attribute("24", desc: "Split at: a tribe this big splits when there's a free slot (a kept one first) and room 1.5-3 km away; without, it's restless (two callers, hungrier, more border fights and challenges). 0 = never")]
	int m_iSplitAt;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Military prefabs (look): a tribe that grew up by a Bastion post or a checkpoint, and the young who were soldiers. Empty: no military look", "et")]
	ref array<ResourceName> m_aMilitaryPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Work clothes prefabs (look, optional): a tribe from a farm valley (a barn, a silo, a mill). Empty: civilian", "et")]
	ref array<ResourceName> m_aWorkPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Dust-grey prefabs (look, optional): a tribe from a quarry. Empty: civilian", "et")]
	ref array<ResourceName> m_aDustPrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Outdoor prefabs (look, optional): a tribe from the woods. Empty: civilian", "et")]
	ref array<ResourceName> m_aOutdoorPrefabs;

	[Attribute("1", UIWidgets.ComboBox, "Your dead run: a character a tribe's bodies kill in the open (with 10 minutes or more of play as that character) comes back as one of its young at the next dusk. Off: player deaths feed it but make no young. Described: public lines use a description. Named: the character's name", "", ParamEnumArray.FromEnum(RZ_EDeadRun))]
	RZ_EDeadRun m_eYourDeadRun;

	[Attribute("1", desc: "People: each faction's words, voices and marks for the packs, what each character knows of them and the ways to learn it, skills by background, the packs' contracts and events, your dead run's named young and ID cards, the fat moat, the moat rule, the call box, the final stand's waves from the lab's neighbours, and the last week's run-down (docs/INFECTED.md, People). Off: tribes as in phase 3")]
	bool m_bPeople;

	[Attribute("", desc: "Name filter: words (comma-separated, lower case). A player's name that contains one is never used in public text, even with Your dead run on Named: the description is")]
	string m_sNameFilter;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "ID card prefab (optional): a small readable item dropped where a named young is put down (its text is told to whoever is near). Empty: the line only", "et")]
	ResourceName m_sIdCardPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Tag prefab: Halcyon's tag, an item with RZ_TagAction (Tag it). Plant a Tag needs it; the Red Hand's fence sells a stolen one with the frequencies. Empty: no Plant a Tag", "et")]
	ResourceName m_sTagPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Call box prefab: an item with RZ_CallBoxAction (Play the recorded call), lent by Halcyon's quartermaster from 50%. Empty: no call box", "et")]
	ResourceName m_sCallBoxPrefab;

	[Attribute("15", desc: "The map of the crews: Red Hand Favor the fence asks for it")]
	int m_iCrewMapFavor;

	[Attribute("25", desc: "The tag frequencies: Red Hand Favor the fence asks for Halcyon's (and a stolen tag, if the server has the tag item)")]
	int m_iFrequencyFavor;

	// Phase 5 (docs/INFECTED.md, "The great one" and "Signs and sounds")
	[Attribute("1", desc: "The great one: a clan of 3 or more packs that holds together 8 season days, from Overrun, makes its oldest leader (in its prime or older) the great one, one on the map at a time. Its guard is 4, its dusk call is the server's one dusk line that evening, its clan gathers every 4 season days and on frenzy nights (the Gathering), and when it dies the clan breaks up and every border reopens (the Night of Many Calls). Needs Life cycle. Off: none of this")]
	bool m_bGreatOne;

	[Attribute("0.85", desc: "Bright moon: from this moon phase (0 new, 1 full) on a clear night (no rain, fog or storm), packs hunt with their range x1.25 and stay out until 05:30. Read from the engine's moon. 0 = off")]
	float m_fBrightMoon;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Kill-site prefabs (optional): small things left where someone was taken (a bicycle on its side, a rucksack with its zip open, one shoe; no gore). One at each kill site while a player is within 200 m. Empty: the lines only", "et")]
	ref array<ResourceName> m_aKillSitePrefabs;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Abandoned lair prefab (optional): flattened bedding or somebody's things at the back of a culvert, where a pack lay up before it moved. Shown within 200 m of a player. Empty: the line only", "et")]
	ResourceName m_sOldLairPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Haven mark prefab (optional): a post with chalk on it, where a road enters a pack's ground the Haven knows. Empty: the mark is text only", "et")]
	ResourceName m_sHavenMarkPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bastion mark prefab (optional): a stencilled plate. Empty: the mark is text only", "et")]
	ResourceName m_sBastionMarkPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand mark prefab (optional): a red hand with a line through it. Empty: the mark is text only", "et")]
	ResourceName m_sRedHandMarkPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Halcyon mark prefab (optional): a plate with the plague doctor stencilled on it. Empty: the mark is text only", "et")]
	ResourceName m_sHalcyonMarkPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Call sound prefab (optional): an entity with a SoundComponent and RZ_CallSoundComponent, which plays its sound event once when it appears. Put at a lair when it calls at dusk with a player within 1.5 km, at a played call box, and at the last call; removed after 40 s. Empty: the lines only", "et")]
	ResourceName m_sCallSoundPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Great call sound prefab (optional): the great one's dusk call, deeper. Empty: the call sound", "et")]
	ResourceName m_sGreatCallSoundPrefab;

	// The small rules (Zombies/RZ_TribeRest.c)
	[Attribute("0", desc: "Prime after Halcyon: after a season Halcyon won, the next season's seeded packs start with leaders in their prime (6-9 season days into leading) instead of Rising. Off by default")]
	bool m_bPrimeAfterHalcyon;

	[Attribute("", desc: "Rival faction (optional): a faction key in the faction manager, hostile to the infected's own. During a staged Border Night the second side's hunter bodies at the border join it, so the two sides really fight; they go back when it's settled. Empty: the fight is heard, then found, as before")]
	string m_sRivalFaction;
}

//------------------------------------------------------------------------------------------------
enum RZ_EDeadRun
{
	OFF,
	DESCRIBED,
	NAMED
}

//------------------------------------------------------------------------------------------------
class RZ_Tribes
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_TribeSettings m_Settings;
	protected int m_iLimitBefore = -1;
	protected ref RZ_TribeBodies m_Bodies;
	protected ref RZ_TribeLife m_Life;
	protected ref RZ_TribeBorders m_Borders;
	protected ref RZ_TribeWays m_Ways;
	protected ref RZ_TribePeople m_People;
	protected ref RZ_TribeGreat m_Great;
	protected ref RZ_TribeProps m_Props;
	protected ref RZ_TribeRest m_Rest;
	protected RZ_Tribe m_ScoreFor;			// weak: the tribe whose trails are being worked out (fear, burned land)
	protected int m_iLastSeasonDay = -1;
	protected bool m_bContested;
	protected bool m_bVacuumTold;
	protected ref array<ref RZ_Tribe> m_aTribes = {};
	protected bool m_bSeeded;
	protected int m_iNextId = 1;
	protected float m_fClock;				// played seconds (only while someone is online)
	protected float m_fDataTimer;
	protected RZ_ETribePhase m_ePhase = RZ_ETribePhase.DAY;
	protected bool m_bPhaseKnown;
	protected float m_fNightOnline;			// seconds online in this game night
	protected int m_iCountedNights;
	protected ref array<float> m_aLastInfection = {};
	protected float m_fTribeSamples;		// samples dropped by tribe bodies, fading over 10 played hours
	protected float m_fAllSamples;
	protected ref array<float> m_aGuardSampleAt = {};	// played minutes of guard samples in the last hour
	protected float m_fGuardSamples;		// since the start, for samples per player-hour
	protected float m_fPackSamples;
	protected float m_fPlayerHours;
	protected ref RZ_TribesSave m_Restored;
	protected bool m_bToldDormant;
	protected bool m_bToldNone;
	protected ref map<int, float> m_mTrackToldAt = new map<int, float>();
	protected ref map<int, int> m_mTakenDay = new map<int, int>();		// player: the season day they last fed a tribe
	protected ref map<int, bool> m_mCalledTo = new map<int, bool>();	// players told a dusk call this dusk
	protected bool m_bMoonTell;				// a bright moon tonight: told to players near a pack once

	static const float DATA_SECONDS = 60;
	static const float CALL_RANGE = 3000;		// a dusk call carries this far
	static const float DAWN_RANGE = 2000;
	static const float LONG_CALL_RANGE = 1500;
	static const float TRACK_READ = 40;
	static const float TRACK_KEEP = 43200;		// played seconds a track lasts
	static const float GROUND_RANGE = 2000;

	//------------------------------------------------------------------------------------------------
	void RZ_Tribes(RZ_GameModeComponent gm, RZ_TribeSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Bodies = new RZ_TribeBodies(gm, this);
		m_Life = new RZ_TribeLife(gm, this);
		m_Borders = new RZ_TribeBorders(gm, this);
		m_Ways = new RZ_TribeWays(gm, this);
		m_People = new RZ_TribePeople(gm, this);
		m_Great = new RZ_TribeGreat(gm, this);
		m_Props = new RZ_TribeProps(gm, this);
		m_Rest = new RZ_TribeRest(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings && m_Settings.m_bEnabled; }
	RZ_TribeSettings GetSettings() { return m_Settings; }
	array<ref RZ_Tribe> GetTribeList() { return m_aTribes; }
	float GetClock() { return m_fClock; }
	RZ_ETribePhase GetPhase() { return m_ePhase; }
	RZ_TribeBodies GetBodies() { return m_Bodies; }
	RZ_TribeLife GetLife() { return m_Life; }
	RZ_TribeBorders GetBorders() { return m_Borders; }
	RZ_TribeWays GetWays() { return m_Ways; }
	RZ_TribePeople GetPeople() { return m_People; }
	RZ_TribeGreat GetGreat() { return m_Great; }
	RZ_TribeProps GetProps() { return m_Props; }
	RZ_TribeRest GetRest() { return m_Rest; }
	array<ref RZ_Tribe> GetRovers() { return m_Life.GetRovers(); }
	int CountedNights() { return m_iCountedNights; }

	//------------------------------------------------------------------------------------------------
	int NextId()
	{
		int id = m_iNextId;
		m_iNextId++;
		return id;
	}

	//------------------------------------------------------------------------------------------------
	//! This player hasn't had a dusk line yet this dusk: now they have.
	bool MarkCalled(int playerId)
	{
		if (m_mCalledTo.Contains(playerId))
			return false;

		m_mCalledTo.Set(playerId, true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Tribe Find(int id)
	{
		foreach (RZ_Tribe t : m_aTribes)
		{
			if (t.m_iId == id)
				return t;
		}

		foreach (RZ_Tribe r : m_Life.GetRovers())
		{
			if (r.m_iId == id)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! At start: the engine's active-AI limit covers every AI on the server, so it's raised when
	//! it's lower than the setting (phase 0: 128 by default).
	void Setup()
	{
		if (!IsEnabled() || m_Settings.m_iActiveAILimit <= 0)
			return;

		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw)
			return;

		m_iLimitBefore = aiw.GetLimitOfActiveAIs();
		if (m_iLimitBefore >= m_Settings.m_iActiveAILimit)
			return;

		aiw.SetLimitOfActiveAIs(m_Settings.m_iActiveAILimit);
		RZ_Log.Info(string.Format("Tribes: the engine's active-AI limit raised from %1 to %2 (it covers faction AI, wanderers and trucks too).",
			m_iLimitBefore, aiw.GetLimitOfActiveAIs()));
	}

	//------------------------------------------------------------------------------------------------
	//! "active AI 74/160"
	static string ActiveAIText()
	{
		AIWorld aiw = GetGame().GetAIWorld();
		if (!aiw)
			return "active AI unknown";

		return string.Format("active AI %1/%2", aiw.GetCurrentNumOfActiveAIs(), aiw.GetLimitOfActiveAIs());
	}

	//------------------------------------------------------------------------------------------------
	//! The land and the slots, for the status line.
	string LandText()
	{
		RZ_Land land = m_GM.GetLand();
		if (!IsEnabled() || !land)
			return "tribes off";

		if (land.IsBuilding())
			return "land: reading the map (tribes dormant)";

		if (!land.IsReady())
			return "land: not read";

		return string.Format("land: %1 km2 wild, %2 slots, %3 packs, counted nights %4", RZ_Util.RoundToInt(land.GetWildKm2()), land.Slots(), m_aTribes.Count(), m_iCountedNights);
	}

	// ================================================================ The clock
	//------------------------------------------------------------------------------------------------
	//! The tribes' time of day: day 06:00-18:30, dusk to 20:30, night to 05:00, dawn to 06:00. With
	//! no game clock it's always day: day parties walk, scouts post, nobody hunts.
	static RZ_ETribePhase PhaseAt(float h)
	{
		if (h < 0)
			return RZ_ETribePhase.DAY;

		if (h >= RZ_GameTime.NIGHT_START || h < RZ_GameTime.NIGHT_END)
			return RZ_ETribePhase.NIGHT;

		if (h < 6)
			return RZ_ETribePhase.DAWN;

		if (h < 18.5)
			return RZ_ETribePhase.DAY;

		return RZ_ETribePhase.DUSK;
	}

	//------------------------------------------------------------------------------------------------
	//! Fog counts as dusk (the scouts' pack is the dusk pack, docs/INFECTED.md, Day parties).
	bool DuskLike()
	{
		if (m_ePhase == RZ_ETribePhase.DUSK)
			return true;

		RZ_Environment env = m_GM.GetEnvironment();
		return m_ePhase == RZ_ETribePhase.DAY && env && env.IsFoggy();
	}

	//------------------------------------------------------------------------------------------------
	bool IsStorm()
	{
		RZ_Environment env = m_GM.GetEnvironment();
		return env && env.IsStorm();
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Every server tick, after the spawner: the clock and its events, the data tick once a minute
	//! (played), and the bodies.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsEnabled())
			return;

		m_fPlayerHours += players.Count() * dt / 3600;

		RZ_Land land = m_GM.GetLand();
		if (!land || !land.IsReady())
		{
			if (land && land.IsBuilding() && !m_bToldDormant)
			{
				m_bToldDormant = true;
				RZ_Log.Info("Tribes: dormant until the land grid is read.");
			}

			return;
		}

		bool online = !ids.IsEmpty();
		if (online)
			m_fClock += dt;

		if (!m_bSeeded)
			Seed();

		if (m_aTribes.IsEmpty() && m_Life.GetRovers().IsEmpty() && (!m_Settings.m_bLifeCycle || land.Slots() <= 0))
			return;

		float h = RZ_GameTime.GetHour();
		RZ_ETribePhase phase = PhaseAt(h);
		if (!m_bPhaseKnown)
		{
			m_bPhaseKnown = true;
			m_ePhase = phase;
			StartPhase(phase, h);
		}
		else if (phase != m_ePhase)
		{
			RZ_ETribePhase was = m_ePhase;
			m_ePhase = phase;
			EndPhase(was);
			StartPhase(phase, h);
		}

		if (phase == RZ_ETribePhase.NIGHT && online)
			m_fNightOnline += dt;

		TimedEvents(h, players, ids);
		if (online)
		{
			m_fDataTimer += dt;
			if (m_fDataTimer >= DATA_SECONDS)
			{
				m_fDataTimer = 0;
				DataTick(DATA_SECONDS, ids, players);
			}
		}

		m_Bodies.Tick(dt, ids, players);
		m_People.Tick(dt, ids, players);
		m_Props.Tick(dt, players);
		m_Rest.RivalTick(dt);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second (the bite check's callback): the scout rule.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (!IsEnabled() || players.IsEmpty())
			return;

		if (m_aTribes.IsEmpty() && m_Life.GetRovers().IsEmpty())
			return;

		m_Bodies.ScoutRule(ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A phase begins (at start too, so a restart picks the rhythm up where the clock is).
	protected void StartPhase(RZ_ETribePhase phase, float h)
	{
		switch (phase)
		{
			case RZ_ETribePhase.DUSK: OnDusk(); break;
			case RZ_ETribePhase.NIGHT: OnNight(); break;
			case RZ_ETribePhase.DAWN: OnDawn(); break;
			case RZ_ETribePhase.DAY: OnDay(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EndPhase(RZ_ETribePhase phase)
	{
		if (phase == RZ_ETribePhase.NIGHT)
			CountNight();
	}

	//------------------------------------------------------------------------------------------------
	//! The game hour now (-1 with no clock).
	protected static float Hour()
	{
		return RZ_GameTime.GetHour();
	}

	//------------------------------------------------------------------------------------------------
	//! What happens at a set hour inside a phase: the day party out at 07:00 and back from 18:00;
	//! the call at 18:45; the set-out between 19:30 and 20:30; rain sends the hunt home at 03:00 and
	//! a Declining alpha at 02:00; the dawn count once the walk home is done (06:00).
	protected void TimedEvents(float h, array<IEntity> players, array<int> ids)
	{
		bool noClock = h < 0;
		RZ_Environment env = m_GM.GetEnvironment();
		bool rain = env && env.IsRaining();

		// The great one's call (the server's one dusk line) and the Night of Many Calls, before the rest
		if (m_ePhase == RZ_ETribePhase.DUSK && h >= 18.75)
			m_Great.Dusk(ids, players);

		if (m_bMoonTell)
		{
			m_bMoonTell = false;
			TellMoon(ids, players);
		}

		foreach (RZ_Tribe t : m_aTribes)
		{
			if (m_ePhase == RZ_ETribePhase.DAY && !t.m_bDayOut && (noClock || h >= m_Rest.DayOutHour(t)))
				DayOut(t);

			if (m_ePhase == RZ_ETribePhase.DAY && t.m_bDayOut && !t.m_bDayBack && h >= 18)
				DayBack(t);

			if (m_ePhase == RZ_ETribePhase.DUSK && !t.m_bCalled && h >= 18.75)
			{
				if (!m_bContested)
				{
					m_bContested = true;
					m_Borders.Contests(ids, players);
				}

				Call(t, players, ids);
			}

			// A soldier's tribe sets out on the 17:30 shift change (its echo)
			if (m_ePhase == RZ_ETribePhase.DAY && !t.m_bSetOut && h >= 17.5 && t.Echo(RZ_EAlphaKind.SOLDIER))
				SetOut(t);

			if (m_ePhase == RZ_ETribePhase.DUSK && !t.m_bSetOut && h >= t.m_fSetOutHour)
				SetOut(t);

			if (m_ePhase == RZ_ETribePhase.NIGHT && !t.m_bSetOut)
				SetOut(t);

			if (m_ePhase == RZ_ETribePhase.NIGHT && h >= 2 && h < 5)
			{
				if (t.Stage() == 2)
					SendHome(t, true);

				if (rain && h >= 3 && t.m_eTemper != RZ_ETemperament.BOLD)
					SendHome(t, false);
			}

			// Echoes: a soldier's tribe walks home at 05:30, a farmer's at first light by the fields; under a
			// bright moon every pack stays out until 05:30
			if (t.m_bLateHome && m_ePhase == RZ_ETribePhase.DAWN && h >= 5.5 && (t.Echo(RZ_EAlphaKind.SOLDIER) || t.m_bMoon))
			{
				t.m_bLateHome = false;
				if (t.m_bMoon)
					m_Bodies.EndHunt(t, false);

				SendHome(t, false);
			}

			if (t.m_bLateHome && m_ePhase == RZ_ETribePhase.DAY)
			{
				t.m_bLateHome = false;
				if (t.Echo(RZ_EAlphaKind.FARMER))
					m_Ways.FarmerHome(t);
				else
					SendHome(t, false);
			}

			if (m_ePhase == RZ_ETribePhase.DAY && !t.m_bDawnCalled && h < 7)
				DawnCount(t, players, ids);
		}

		// The stragglers after a bad night come home between 06:00 and 07:30
		if (m_ePhase == RZ_ETribePhase.DAY)
			m_Rest.StragglerTick(h, ids, players);

		// A vacuum's silence, for players near it who heard no call
		if (m_ePhase == RZ_ETribePhase.DUSK && h >= 18.8 && m_bContested && !m_bVacuumTold)
		{
			m_bVacuumTold = true;
			m_People.AfterCalls(ids, players);
			m_Rest.ClanRelay(ids, players);
			foreach (int i, IEntity p : players)
			{
				string quiet = m_Life.VacuumLine(p.GetOrigin());
				if (!quiet.IsEmpty() && MarkCalled(ids[i]))
					m_GM.NotifyPlayer(ids[i], "At dusk", quiet);
			}
		}

		if (m_ePhase != RZ_ETribePhase.DUSK)
		{
			m_mCalledTo.Clear();
			m_bContested = false;
			m_bVacuumTold = false;
			m_Great.NotDusk();
		}
	}

	// ================================================================ Seeds and places
	//------------------------------------------------------------------------------------------------
	//! The season's seeds (docs/INFECTED.md, Seeds): slots / 3, rounded up, at most 4 and never more
	//! than the slots not kept for a split. The column first, at the canon spot origin.column on the
	//! road in from the northernmost sealed checkpoint (an island's: the main road toward the line);
	//! the site tribe at origin.site if that's wild; then the best lair candidates far from the lab.
	protected void Seed()
	{
		m_bSeeded = true;
		RZ_Land land = m_GM.GetLand();
		if (m_Restored)
		{
			RestoreTribes();
			if (!m_aTribes.IsEmpty() || m_Restored.seeded)
			{
				LogTribes("restored");
				return;
			}
		}

		int slots = land.Slots();
		if (slots <= 0)
		{
			RZ_Log.Info("Tribes: no slots on this map (too little wild land; Zombies > Tribes > Slots overrides it). Crawlers only.");
			return;
		}

		int kept = RZ_Land.KeptForSplits(slots);
		int seeds = Math.Min(Math.Min(4, (slots + 2) / 3), Math.Max(1, slots - kept));
		array<vector> taken = {};
		RZ_WorldFacts facts = m_GM.GetFacts();
		for (int i = 0; i < seeds; i++)
		{
			vector lair;
			vector road;
			bool column = false;
			if (i == 0 && ColumnSpot(road) && NearLair(road, 1500, taken, lair))
			{
				column = true;
				if (facts && !facts.HasCanon("origin.column"))
					facts.SetCanon("origin.column", RZ_WorldFacts.PosText(road));
			}
			else if (i == 1 && facts && facts.HasCanon("origin.site") && land.IsWild(facts.CanonPos("origin.site")) && NearLair(facts.CanonPos("origin.site"), 1000, taken, lair))
			{
				RZ_Log.Info("Tribes: the site's own lies up near the closed site.");
			}
			else if (!BestLair(taken, lair))
			{
				break;
			}

			taken.Insert(lair);
			RZ_Tribe t = NewTribe(lair, Math.RandomIntInclusive(10, 14), column);
			t.m_fLedSince = m_Rest.SeedLedSince();
		}

		LogTribes("seeded");
	}

	//------------------------------------------------------------------------------------------------
	//! The column's spot: the road 1-3 km in from the northernmost checkpoint toward the largest
	//! town; with no checkpoint, the largest town's road toward the nearest point of the line.
	protected bool ColumnSpot(out vector spot)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts && facts.HasCanon("origin.column"))
		{
			spot = facts.CanonPos("origin.column");
			return true;
		}

		RZ_Settlement big;
		foreach (RZ_Settlement s : m_GM.GetSettlements())
		{
			if (!big || s.m_iSize > big.m_iSize)
				big = s;
		}

		if (!big)
			return false;

		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		if (!perimeter || !perimeter.IsActive())
			return false;

		array<vector> checkpoints = {};
		perimeter.GetCheckpoints(checkpoints);
		vector from;
		bool have = false;
		foreach (vector c : checkpoints)
		{
			if (!have || c[2] > from[2])
			{
				from = c;
				have = true;
			}
		}

		if (!have)
			return false;

		vector dir = big.m_vCenter - from;
		dir[1] = 0;
		if (dir.Length() < 100)
			return false;

		dir.Normalize();
		spot = RZ_Util.GroundPos(from + dir * Math.RandomFloat(1000, 3000));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The lair candidate nearest pos within reach, kept 3 km from the lairs already taken.
	protected bool NearLair(vector pos, float reach, array<vector> taken, out vector lair)
	{
		RZ_Land land = m_GM.GetLand();
		vector found;
		if (!land.NearestCellWith(pos, RZ_Land.BIT_LAIR, 0, reach, found))
			return false;

		foreach (vector t : taken)
		{
			if (RZ_Util.DistXZ(t, found) < 3000)
				return false;
		}

		lair = RZ_Util.GroundPos(found);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The best free lair candidate: far from the lab and 3 km from every other lair, in the most
	//! wild land.
	protected bool BestLair(array<vector> taken, out vector lair)
	{
		RZ_Land land = m_GM.GetLand();
		float best = -1;
		bool any = false;
		vector lab = m_GM.GetLabPosition();
		bool hasLab = m_GM.HasLab();
		int cells = land.GetCellCount();
		for (int i = 0; i < cells; i++)
		{
			if (!land.HasBit(i, RZ_Land.BIT_LAIR))
				continue;

			vector c = land.CellCenter(i);
			bool clear = true;
			foreach (vector t : taken)
			{
				if (RZ_Util.DistXZ(t, c) < 3000)
				{
					clear = false;
					break;
				}
			}

			if (!clear)
				continue;

			float score = WildAround(c, 1000) + Math.RandomFloat(0, 3);
			if (hasLab)
				score += Math.Min(RZ_Util.DistXZ(c, lab), 4000) / 500;

			if (score > best)
			{
				best = score;
				lair = RZ_Util.GroundPos(c);
				any = true;
			}
		}

		return any;
	}

	//------------------------------------------------------------------------------------------------
	//! Wild squares within radius of pos.
	protected int WildAround(vector pos, float radius)
	{
		RZ_Land land = m_GM.GetLand();
		int n = 0;
		int reach = radius / RZ_Land.CELL;
		for (int dz = -reach; dz <= reach; dz++)
		{
			for (int dx = -reach; dx <= reach; dx++)
			{
				vector p = pos + Vector(dx * RZ_Land.CELL, 0, dz * RZ_Land.CELL);
				if (RZ_Util.DistXZ(p, pos) > radius)
					continue;

				int idx = land.CellAt(p);
				if (idx >= 0 && land.HasBit(idx, RZ_Land.BIT_WILD))
					n++;
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A new tribe at a lair: its place, range, temperament, call, alpha, members and places.
	RZ_Tribe NewTribe(vector lair, int members, bool column)
	{
		RZ_Tribe t = new RZ_Tribe();
		t.m_iId = m_iNextId;
		m_iNextId++;
		t.m_iSeed = Math.RandomInt(1, 1000000);
		t.m_vLair = lair;
		t.m_bColumn = column;
		t.m_fFounded = RZ_SeasonClock.Days();
		RZ_Land land = m_GM.GetLand();
		int idx = land.CellAt(lair);
		t.m_sLairKind = "woods";
		if (idx >= 0 && land.GetMark(idx) > 0)
			t.m_sLairKind = land.GetLairKindName(land.GetMark(idx));

		t.m_sPlace = PlaceName(t);
		t.m_eTemper = RollTemper(t, column);
		t.m_sPattern = Pattern(t.m_iSeed);
		t.m_bHasAlpha = true;
		t.m_sAlphaWho = AlphaWho(t, column);
		t.m_eLook = m_Ways.LookFor(t);
		m_Ways.NameAlpha(t);
		t.m_fLedSince = RZ_SeasonClock.Days();
		t.m_fAlphaHealth = 1;
		t.m_iGuards = 2;
		t.m_iScouts = 1;
		if (members >= 15)
			t.m_iScouts = 2;

		t.m_iYoung = Math.Min(2, Math.Max(0, members - 8));
		t.m_iHunters = Math.Max(1, members - 1 - t.m_iGuards - t.m_iScouts - t.m_iYoung);
		BuildPlaces(t);
		BuildParts(t);
		m_aTribes.Insert(t);
		if (t.m_eTemper == RZ_ETemperament.NOMADIC)
			m_Life.BuildLoop(t);

		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! "the woods north of Levie", "the quarry by Morton": the place every line carries.
	string PlaceName(RZ_Tribe t)
	{
		int idx = m_GM.FindNearestSettlement(t.m_vLair);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		string town = RZ_SettlementScanner.GridReference(t.m_vLair);
		if (s)
			town = s.m_sName;

		string name = string.Format("the %1 by %2", t.m_sLairKind, town);
		if (t.m_sLairKind == "woods" && s)
			name = string.Format("the woods %1 of %2", CompassWord(s.m_vCenter, t.m_vLair), town);

		foreach (RZ_Tribe other : m_aTribes)
		{
			if (other != t && other.m_sPlace == name && s)
				name = string.Format("the %1 %2 of %3", t.m_sLairKind, CompassWord(s.m_vCenter, t.m_vLair), town);
		}

		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! "north", "south-east"... the way from a to b.
	static string CompassWord(vector a, vector b)
	{
		float deg = Math.Atan2(b[0] - a[0], b[2] - a[2]) * Math.RAD2DEG;
		if (deg < 0)
			deg += 360;

		array<string> names = {"north", "north-east", "east", "south-east", "south", "south-west", "west", "north-west"};
		int i = RZ_Util.RoundToInt(deg / 45);
		return names[i % 8];
	}

	//------------------------------------------------------------------------------------------------
	//! Temperament (docs/INFECTED.md): bold 25%, cautious, cunning and territorial 20% each, nomadic
	//! 15%; a quarry leans cunning, the column nomadic, a farm's barn or silo cautious.
	protected RZ_ETemperament RollTemper(RZ_Tribe t, bool column)
	{
		array<float> w = {25, 20, 20, 20, 15};
		if (column)
			w[4] = w[4] + 20;

		if (t.m_sLairKind == "quarry")
			w[2] = w[2] + 20;

		if (t.m_sLairKind == "barn" || t.m_sLairKind == "silo")
			w[1] = w[1] + 20;

		float total = 0;
		foreach (float f : w)
		{
			total += f;
		}

		float roll = Math.RandomFloat(0, total);
		for (int i = 0; i < 5; i++)
		{
			roll -= w[i];
			if (roll <= 0)
				return i;
		}

		return RZ_ETemperament.BOLD;
	}

	//------------------------------------------------------------------------------------------------
	//! Its call, from its seed: "three short and a long".
	static string Pattern(int seed)
	{
		array<string> calls = {"three short and a long", "two long", "one long and two short", "a long rising call", "four short", "a long and a short", "two short, a pause, two short"};
		return calls[RZ_Util.PickIndex(seed, calls.Count())];
	}

	//------------------------------------------------------------------------------------------------
	//! One line on who the alpha was (for the log; the factions' words for it are planned).
	protected string AlphaWho(RZ_Tribe t, bool column)
	{
		int idx = m_GM.FindNearestSettlement(t.m_vLair);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		string town = "the roads";
		if (s)
			town = s.m_sName;

		if (column)
			return "a driver from the Day 2 column";

		array<string> kinds = {"a farmer", "a driver", "a runner", "a soldier", "a courier", "someone"};
		return string.Format("%1 from %2", kinds[RZ_Util.PickIndex(t.m_iSeed, kinds.Count())], town);
	}

	//------------------------------------------------------------------------------------------------
	//! The range (500-1,000 m by the wild land round the lair; 350-700 on small maps), its hunting
	//! grounds (towns' edges, crossings and fields within reach), its trails (4-6 points each, from
	//! the land grid) and its two scout posts (the highest wild squares in the range).
	void BuildPlaces(RZ_Tribe t)
	{
		RZ_Land land = m_GM.GetLand();
		int wild = WildAround(t.m_vLair, 1000);
		float lo = 500;
		float hi = 1000;
		if (land.GetLandKm2() < 25)
		{
			lo = 350;
			hi = 700;
		}

		if (t.m_fRadius <= 0 || !m_Restored)
			t.m_fRadius = m_Rest.LookRange(t, Math.Clamp(lo + wild * 25, lo, hi), hi);

		t.m_aGrounds.Clear();
		t.m_aTrails.Clear();
		t.m_aPosts.Clear();
		// Towns' edges facing the lair (not lived-in: their wire is only for hungry nights)
		RZ_WorldFacts facts = m_GM.GetFacts();
		foreach (int si, RZ_Settlement s : m_GM.GetSettlements())
		{
			float d = RZ_Util.DistXZ(s.m_vCenter, t.m_vLair);
			if (d > GROUND_RANGE || d < s.m_fRadius + 100)
				continue;

			if (facts && facts.IsLivedIn(si))
				continue;

			vector toLair = t.m_vLair - s.m_vCenter;
			toLair[1] = 0;
			toLair.Normalize();
			AddGround(t, RZ_Util.GroundPos(s.m_vCenter + toLair * (s.m_fRadius * 0.8)));
		}

		for (int ci = 0; ci < land.GetCrossingCount(); ci++)
		{
			vector cp = land.GetCrossing(ci);
			if (RZ_Util.DistXZ(cp, t.m_vLair) <= t.m_fRadius * 1.3)
				AddGround(t, RZ_Util.GroundPos(cp));
		}

		vector field;
		if (land.NearestCellWith(t.m_vLair, RZ_Land.BIT_FIELD, 200, t.m_fRadius * 1.5, field))
			AddGround(t, RZ_Util.GroundPos(field));

		if (t.m_aGrounds.IsEmpty())
		{
			vector edge = RZ_Util.GroundPos(t.m_vLair + RZ_Behaviour.RandomDir() * t.m_fRadius);
			AddGround(t, edge);
		}

		// The grounds it followed its prey to
		t.m_aPrey.Clear();
		m_Rest.PreyGrounds(t);
		BuildTrails(t);
		Posts(t);
	}

	//------------------------------------------------------------------------------------------------
	//! Its trails from the lair to each hunting ground, worked out again (a fear place came or went,
	//! land burned): they go round fear places and burned land.
	void BuildTrails(RZ_Tribe t)
	{
		m_ScoreFor = t;
		t.m_aTrails.Clear();
		foreach (vector g : t.m_aGrounds)
		{
			array<vector> trail = {};
			BuildTrail(t.m_vLair, g, trail);
			m_People.TableTurnsBack(trail);
			t.m_aTrails.Insert(trail);
		}

		while (t.m_aWear.Count() < t.m_aTrails.Count())
		{
			t.m_aWear.Insert(0);
		}

		m_ScoreFor = null;
		m_Rest.RoadCrossings(t);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddGround(RZ_Tribe t, vector g)
	{
		if (t.m_aGrounds.Count() >= 4 || RZ_Util.IsWater(g))
			return;

		foreach (vector known : t.m_aGrounds)
		{
			if (RZ_Util.DistXZ(known, g) < 300)
				return;
		}

		t.m_aGrounds.Insert(g);
	}

	//------------------------------------------------------------------------------------------------
	//! A trail from a to b (docs/INFECTED.md, Trails): a point every 250 m, each the best of the
	//! square on the line and the two beside it: dry land, a forest edge or sparse trees preferred,
	//! out of the towns and no man's land, water crossed only at a crossing. Thinned to 4-6 points.
	void BuildTrail(vector a, vector b, array<vector> trail)
	{
		RZ_Land land = m_GM.GetLand();
		trail.Clear();
		trail.Insert(a);
		float total = RZ_Util.DistXZ(a, b);
		vector dir = b - a;
		dir[1] = 0;
		if (total < 1)
		{
			trail.Insert(b);
			return;
		}

		dir.Normalize();
		vector side = Vector(dir[2], 0, -dir[0]);
		int steps = Math.Max(2, total / 250);
		array<vector> raw = {};
		for (int k = 1; k < steps; k++)
		{
			float fk = k;
			vector base = a + dir * (total * fk / steps);
			vector best = base;
			float bestScore = -100;
			for (int o = -2; o <= 2; o++)
			{
				vector cand = base + side * (o * 120);
				float score = PointScore(land, cand);
				if (o == 0)
					score += 0.5;

				if (o == -2 || o == 2)
					score -= 1;

				if (score > bestScore)
				{
					bestScore = score;
					best = cand;
				}
			}

			if (bestScore > -50)
				raw.Insert(RZ_Util.GroundPos(best));
		}

		int keep = Math.Min(4, raw.Count());
		for (int j = 0; j < keep; j++)
		{
			float fj = j + 1;
			int at = Math.Floor(fj * raw.Count() / (keep + 1));
			at = Math.ClampInt(at, 0, raw.Count() - 1);
			trail.Insert(raw[at]);
		}

		trail.Insert(b);
	}

	//------------------------------------------------------------------------------------------------
	//! How good a spot is for a trail or a day walk: -100 water or a town or no man's land; +2 a
	//! forest edge, +1 sparse trees; -5 a river square with no crossing.
	float PointScore(RZ_Land land, vector p)
	{
		int idx = land.CellAt(p);
		if (idx < 0 || !land.IsDry(idx) || RZ_Util.IsWater(RZ_Util.GroundPos(p)))
			return -100;

		if (land.InNoMansLand(p))
			return -100;

		// Fear places and burned land (the tribe whose trails these are)
		if (m_ScoreFor && (m_Ways.FearAvoids(m_ScoreFor, p) || m_Life.IsBurned(p, true)))
			return -100;

		int town = m_GM.FindNearestSettlement(p);
		RZ_Settlement s = m_GM.GetSettlement(town);
		if (s && RZ_Util.DistXZ(s.m_vCenter, p) < s.m_fRadius)
			return -60;

		float score = 0;
		int trees = land.GetTreeClass(idx);
		if (trees == RZ_Land.TREES_SPARSE)
			score += 1;

		if (trees != RZ_Land.TREES_NONE && IsEdge(land, p))
			score += 2;

		if (land.HasBit(idx, RZ_Land.BIT_RIVER) && !land.HasBit(idx, RZ_Land.BIT_BRIDGE) && !land.HasBit(idx, RZ_Land.BIT_FORD))
			score -= 5;

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! A wooded square beside an open one: a forest edge (where trails and stalks keep to cover).
	static bool IsEdge(RZ_Land land, vector p)
	{
		array<vector> offs = {Vector(RZ_Land.CELL, 0, 0), Vector(-RZ_Land.CELL, 0, 0), Vector(0, 0, RZ_Land.CELL), Vector(0, 0, -RZ_Land.CELL)};
		foreach (vector o : offs)
		{
			int n = land.CellAt(p + o);
			if (n >= 0 && land.IsDry(n) && land.GetTreeClass(n) == RZ_Land.TREES_NONE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Two scout posts: the highest wild squares in the range (a ridge first), 300 m apart.
	protected void Posts(RZ_Tribe t)
	{
		RZ_Land land = m_GM.GetLand();
		float postRange = t.m_fRadius * m_Rest.PostReach(t);
		int reach = postRange / RZ_Land.CELL;
		reach++;
		array<vector> best = {};
		array<float> bestH = {};
		for (int dz = -reach; dz <= reach; dz++)
		{
			for (int dx = -reach; dx <= reach; dx++)
			{
				vector p = t.m_vLair + Vector(dx * RZ_Land.CELL, 0, dz * RZ_Land.CELL);
				if (RZ_Util.DistXZ(p, t.m_vLair) > postRange || RZ_Util.DistXZ(p, t.m_vLair) < 150)
					continue;

				int idx = land.CellAt(p);
				if (idx < 0 || !land.HasBit(idx, RZ_Land.BIT_WILD) || !land.IsDry(idx))
					continue;

				float hgt = land.GetHeight(idx);
				if (land.HasBit(idx, RZ_Land.BIT_RIDGE))
					hgt += 50;

				best.Insert(RZ_Util.GroundPos(land.CellCenter(idx)));
				bestH.Insert(hgt);
			}
		}

		for (int pick = 0; pick < 2; pick++)
		{
			int top = -1;
			for (int i = 0; i < best.Count(); i++)
			{
				bool apart = true;
				foreach (vector post : t.m_aPosts)
				{
					if (RZ_Util.DistXZ(post, best[i]) < 300)
						apart = false;
				}

				if (apart && (top < 0 || bestH[i] > bestH[top]))
					top = i;
			}

			if (top < 0)
				break;

			t.m_aPosts.Insert(best[top]);
		}

		while (t.m_aPosts.Count() < 2)
		{
			t.m_aPosts.Insert(RZ_Util.GroundPos(t.m_vLair + RZ_Behaviour.RandomDir() * (t.m_fRadius * 0.6)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The parts, from the members: the alpha alone, the guard, hunters in groups of 3-5, the young,
	//! each scout. All at the lair, at their spots in the resting ring (scouts at their posts).
	void BuildParts(RZ_Tribe t)
	{
		t.m_aParts.Clear();
		if (t.m_bHasAlpha)
			AddPart(t, RZ_ETribePart.ALPHA, 1, 0);

		if (t.m_iGuards > 0)
			AddPart(t, RZ_ETribePart.GUARD, t.m_iGuards, 0);

		int hunters = t.m_iHunters;
		while (hunters > 0)
		{
			int size = Math.Min(hunters, 4);
			if (hunters - size > 0 && hunters - size < 3)
				size = Math.Min(hunters, 3);

			AddPart(t, RZ_ETribePart.HUNTERS, size, 0);
			hunters -= size;
		}

		int young = t.m_iYoung;
		while (young > 0)
		{
			int ysize = Math.Min(young, 4);
			AddPart(t, RZ_ETribePart.YOUNG, ysize, 0);
			young -= ysize;
		}

		int posts = Math.Max(1, t.m_aPosts.Count());
		for (int s = 0; s < t.m_iScouts; s++)
		{
			RZ_TribePart scout = AddPart(t, RZ_ETribePart.SCOUT, 1, 0);
			scout.m_iPost = s % posts;
			if (!t.m_aPosts.IsEmpty())
			{
				scout.m_vRest = t.m_aPosts[scout.m_iPost];
				scout.m_vPos = scout.m_vRest;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A new part at the lair, at its spot in the resting ring.
	RZ_TribePart AddPart(RZ_Tribe t, RZ_ETribePart kind, int count, int young)
	{
		RZ_TribePart p = new RZ_TribePart();
		p.m_iId = RZ_InfectedBodies.NewPartId();
		p.m_iTribe = t.m_iId;
		p.m_eKind = kind;
		p.m_eRole = p.Role();
		p.m_iCount = count;
		p.m_iYoung = young;
		float r = 40;
		switch (kind)
		{
			case RZ_ETribePart.ALPHA: r = 0; break;
			case RZ_ETribePart.GUARD: r = 14; break;
			case RZ_ETribePart.YOUNG: r = 55; break;
		}

		// A dust-grey pack lies up close, under cover
		r = r * m_Rest.RingScale(t);
		p.m_vRest = RZ_Util.GroundPos(t.m_vLair + RZ_Behaviour.RandomDir() * r);
		p.m_vPos = p.m_vRest;
		p.m_sNote = t.m_sPlace;
		t.m_aParts.Insert(p);
		return p;
	}

	// ================================================================ The day
	//------------------------------------------------------------------------------------------------
	//! Day (06:00): everyone at the lair or walking home to it.
	protected void OnDay()
	{
		foreach (RZ_Tribe t : m_aTribes)
		{
			t.m_bDayOut = false;
			t.m_bDayBack = false;
			t.m_bCalled = false;
			t.m_bSetOut = false;
			t.m_fSetOutHour = m_Rest.SetOutHour(t);
			if (!t.m_bLateHome)
				SendHome(t, false);

			t.m_bMoon = false;
			m_Rest.Posts(t, false);
		}

		m_Life.RoversAtPhase(RZ_ETribePhase.DAY);
	}

	//------------------------------------------------------------------------------------------------
	//! Empty nights (rule 9): a pack that set out three nights running and took nobody drops its
	//! least-worn trail (worked out again with its places), so its hunting moves somewhere else.
	protected void EmptyNight(RZ_Tribe t)
	{
		if (!t.m_bHunted)
			return;

		if (t.m_bTookTonight)
		{
			t.m_iEmptyNights = 0;
			return;
		}

		t.m_iEmptyNights++;
		if (t.m_iEmptyNights < 3 || t.m_aTrails.Count() < 2)
			return;

		t.m_iEmptyNights = 0;
		int worst = 0;
		for (int i = 1; i < t.m_aTrails.Count(); i++)
		{
			if (i < t.m_aWear.Count() && worst < t.m_aWear.Count() && t.m_aWear[i] < t.m_aWear[worst])
				worst = i;
		}

		t.m_aTrails.Remove(worst);
		if (worst < t.m_aWear.Count())
			t.m_aWear.Remove(worst);

		RZ_Log.Info(string.Format("Tribes: %1 took nobody three nights running and drops a trail (%2 left).", t.m_sPlace, t.m_aTrails.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! The day party (docs/INFECTED.md, Day parties), from 07:00: a fifth of the tribe by default
	//! (twice that when hungry, never over half), hunters and young only, in groups of 2-3, each
	//! walking its own day walk by hop and hold. Nobody the day after a feed, in a storm, during the
	//! vigil's morning, or from a tribe under 8.
	protected void DayOut(RZ_Tribe t)
	{
		t.m_bDayOut = true;
		bool splinterDay = t.m_bNoDayParty;
		t.m_bNoDayParty = false;
		if (t.m_bFed || IsStorm() || t.IsWeakened() || t.m_bVigil || !t.m_bHasAlpha || m_Settings.m_fDayPartyShare <= 0 || splinterDay)
			return;

		float share = Math.Clamp(m_Settings.m_fDayPartyShare, 0, 0.25);
		bool hungry = t.Band() >= 2;
		if (hungry)
		{
			share = Math.Min(0.5, share * 2);
			// The Haven's warning: hungry walkers come right down to the fields by day
			if (m_People.IsOn())
				Tell(t.RangeCentre(), t.HuntRadius() + 1500, "The Haven", string.Format("%1 are hungry. Their walkers will come right down to the fields today, in broad daylight. Nobody past the hedges.", Capitalize(m_People.NameAt(t, RZ_EFaction.SURVIVORS))));
		}

		// The two weak days after a succession: half a day party
		if (t.IsWeakDays())
			share = share * 0.5;

		int want = RZ_Util.RoundToInt(t.Members() * share);
		if (want < 2)
			return;

		int lo = Math.Clamp(m_Settings.m_iDayGroupMin, 2, 4);
		int hi = Math.Clamp(m_Settings.m_iDayGroupMax, lo, 4);
		int maxGroups = 3;
		if (hungry)
			maxGroups = 5;

		int groups = 0;
		while (want >= 2 && groups < maxGroups)
		{
			int size = Math.Min(want, Math.RandomIntInclusive(lo, hi));
			if (size < 2)
				break;

			int young = 0;
			int got = TakeFromRest(t, RZ_ETribePart.YOUNG, 1);
			young += got;
			int hunters = TakeFromRest(t, RZ_ETribePart.HUNTERS, size - got);
			if (young + hunters < 2)
			{
				// Not enough lying up without bodies: give them back
				GiveToRest(t, young, hunters);
				break;
			}

			RZ_TribePart party = AddPart(t, RZ_ETribePart.DAY, young + hunters, young);
			party.m_vRest = t.m_vLair;
			party.m_vPos = t.m_vLair;
			party.m_bOut = true;
			party.m_bHop = true;
			array<vector> walk = {};
			DayWalk(t, hungry, walk);
			party.SetPath(walk);
			party.m_fLegLeft = Math.RandomFloat(100, 200);
			party.m_fSpeed = 0.7;
			want -= young + hunters;
			groups++;
		}

		if (groups > 0)
			RZ_Log.Info(string.Format("Tribes: %1 has %2 groups out by day%3.", t.m_sPlace, groups, HungryNote(hungry)));
	}

	//------------------------------------------------------------------------------------------------
	protected static string HungryNote(bool hungry)
	{
		if (hungry)
			return " (hungry: as far as the fields at the towns' edges)";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Takes up to n members of a kind from parts lying up at the lair with no bodies.
	protected int TakeFromRest(RZ_Tribe t, RZ_ETribePart kind, int n)
	{
		int got = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (got >= n)
				break;

			if (p.m_eKind != kind || p.HasBody() || p.m_bOut || p.m_iCount <= 0)
				continue;

			int take = Math.Min(n - got, p.m_iCount);
			if (kind == RZ_ETribePart.HUNTERS)
				take = Math.Min(take, p.m_iCount - p.m_iYoung);

			p.m_iCount = p.m_iCount - take;
			got += take;
		}

		return got;
	}

	//------------------------------------------------------------------------------------------------
	//! Back into the resting ring (into parts with no bodies, else new ones).
	void GiveToRest(RZ_Tribe t, int young, int hunters)
	{
		if (young > 0)
		{
			RZ_TribePart y = RestingPart(t, RZ_ETribePart.YOUNG);
			if (y)
				y.m_iCount = y.m_iCount + young;
			else
				AddPart(t, RZ_ETribePart.YOUNG, young, 0);
		}

		if (hunters > 0)
		{
			RZ_TribePart h = RestingPart(t, RZ_ETribePart.HUNTERS);
			if (h && h.m_iCount + hunters <= 5)
				h.m_iCount = h.m_iCount + hunters;
			else
				AddPart(t, RZ_ETribePart.HUNTERS, hunters, 0);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_TribePart RestingPart(RZ_Tribe t, RZ_ETribePart kind)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind == kind && !p.HasBody() && !p.m_bOut)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A day walk: 3-5 points in the range along forest edges and field margins, out of the towns,
	//! no man's land and the line's margin, round its fear places, back to the lair. A hungry
	//! tribe's goes out to the fields at the nearest town's edge (outside its radius); in rain it
	//! keeps to the woods. By temperament: a territorial tribe's walks its range's edge, a cautious
	//! one's keeps to the woods, a nomad's walks ahead on the next leg of its loop.
	protected void DayWalk(RZ_Tribe t, bool hungry, array<vector> walk)
	{
		RZ_Land land = m_GM.GetLand();
		RZ_Environment env = m_GM.GetEnvironment();
		bool rain = env && env.IsRaining();
		bool woods = rain || (m_Settings.m_bLifeCycle && t.m_eTemper == RZ_ETemperament.CAUTIOUS);
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		vector centre = t.RangeCentre();
		float lo = 0.3;
		float hi = 0.9;
		if (m_Settings.m_bLifeCycle && t.m_eTemper == RZ_ETemperament.TERRITORIAL)
		{
			lo = 0.75;
			hi = 1;
		}

		if (m_Settings.m_bLifeCycle && t.m_eTemper == RZ_ETemperament.NOMADIC && t.m_aLoop.Count() > 1)
		{
			vector nextLair = t.m_aLoop[(t.m_iLoopAt + 1) % t.m_aLoop.Count()];
			centre = (t.m_vLair + nextLair) * 0.5;
		}

		// Furious: its walkers walk the capture road toward the lab, as far as half again its range
		array<vector> capture = {};
		if (m_People.IsFurious(t) && FuriousTrail(t, capture))
		{
			foreach (vector cp : capture)
			{
				if (RZ_Util.DistXZ(cp, t.m_vLair) > t.m_fRadius * 1.5)
					break;

				walk.Insert(cp);
			}

			walk.Insert(t.m_vLair);
			return;
		}

		m_ScoreFor = t;
		walk.Insert(t.m_vLair);
		int points = Math.RandomIntInclusive(3, 5);
		vector last = t.m_vLair;
		for (int i = 0; i < points; i++)
		{
			vector best = last;
			float bestScore = -100;
			for (int tries = 0; tries < 6; tries++)
			{
				vector cand = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(centre, t.m_fRadius * lo, t.m_fRadius * hi));
				float score = PointScore(land, cand);
				if (woods && land.CellAt(cand) >= 0 && land.GetTreeClass(land.CellAt(cand)) == RZ_Land.TREES_NONE)
					score -= 3;

				if (perimeter && perimeter.IsActive() && perimeter.IsNearLineWithin(cand, 200))
					score = -100;

				if (score > bestScore)
				{
					bestScore = score;
					best = cand;
				}
			}

			if (bestScore > -50)
			{
				walk.Insert(best);
				last = best;
			}
		}

		m_ScoreFor = null;
		vector field;
		if (hungry && TownEdgeField(t, field))
			walk.Insert(field);

		walk.Insert(t.m_vLair);
		m_People.KeepOffTable(walk);
	}

	//------------------------------------------------------------------------------------------------
	//! A field at a town's edge for a hungry day party: a field square outside the town's radius and
	//! outside no man's land, within 2.5 km of the lair.
	protected bool TownEdgeField(RZ_Tribe t, out vector field)
	{
		RZ_Land land = m_GM.GetLand();
		vector found;
		for (int tries = 0; tries < 4; tries++)
		{
			float minDist = 200 + tries * 400;
			if (!land.NearestCellWith(t.m_vLair, RZ_Land.BIT_FIELD, minDist, 2500, found))
				return false;

			int town = m_GM.FindNearestSettlement(found);
			RZ_Settlement s = m_GM.GetSettlement(town);
			if (s && RZ_Util.DistXZ(s.m_vCenter, found) < s.m_fRadius + 30)
				continue;

			if (land.InNoMansLand(found))
				continue;

			field = RZ_Util.GroundPos(found);
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 18:00: the day party walks back to the lair for the call.
	protected void DayBack(RZ_Tribe t)
	{
		t.m_bDayBack = true;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind == RZ_ETribePart.DAY && p.m_iTarget < 0)
				PathHome(t, p);
		}
	}

	// ================================================================ Dusk
	//------------------------------------------------------------------------------------------------
	//! Dusk (18:30): the day party back in the ring, the young who joined (the taken, the ones who
	//! ran), the count for the dawn.
	protected void OnDusk()
	{
		foreach (RZ_Tribe t : m_aTribes)
		{
			MergeDayParties(t);
			if (t.m_iPendingYoung > 0)
			{
				GiveToRest(t, t.m_iPendingYoung, 0);
				RZ_Log.Info(string.Format("Tribes: %1 gains %2 young at dusk (the taken and the ones who ran).", t.m_sPlace, t.m_iPendingYoung));
				t.m_iPendingYoung = 0;
			}

			EmptyNight(t);
			t.m_iAtDusk = t.Members();
			t.m_iLostTonight = 0;
			t.m_aDeadTonight.Clear();
			t.m_bTookTonight = false;
			t.m_bHunted = false;
			t.m_bDawnCalled = false;
			t.m_aBlocked.Clear();
			t.m_aDeafTo.Clear();
			t.m_fSetOutHour = m_Rest.SetOutHour(t);
		}

		m_Life.OnDusk();
	}

	//------------------------------------------------------------------------------------------------
	//! Day-party groups home with no bodies go back into the ring; the others stay a group of
	//! hunters (their young with them) for the night.
	protected void MergeDayParties(RZ_Tribe t)
	{
		for (int i = t.m_aParts.Count() - 1; i >= 0; i--)
		{
			RZ_TribePart p = t.m_aParts[i];
			if (p.m_eKind != RZ_ETribePart.DAY)
				continue;

			p.m_bPack = false;
			p.m_iTarget = -1;
			if (!p.HasBody() && RZ_Util.DistXZ(p.m_vPos, t.m_vLair) < 150)
			{
				int young = p.m_iYoung;
				int hunters = p.m_iCount - p.m_iYoung;
				t.m_aParts.Remove(i);
				GiveToRest(t, young, hunters);
				continue;
			}

			p.m_eKind = RZ_ETribePart.HUNTERS;
			p.m_bHop = false;
			p.m_fSpeed = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The call (18:45; docs/INFECTED.md, The call): one line to each player at most, from the
	//! nearest tribe within earshot, saying how it is. None during post-feed sleep: silence is the
	//! sign. Then it may hunt again.
	protected void Call(RZ_Tribe t, array<IEntity> players, array<int> ids)
	{
		t.m_bCalled = true;
		bool silent = t.m_bFed;
		t.m_bFed = false;
		m_Rest.OnCall(t, silent);
		float reach = m_Rest.CallReach(t);
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			if (m_mCalledTo.Contains(id))
				continue;

			// A call from a ridge carries further, relayed ridge to ridge, to whoever has no nearer one
			RZ_Tribe nearest = NearestTribe(p.GetOrigin(), CALL_RANGE);
			bool relayed = false;
			if (!nearest && reach > CALL_RANGE && NearestTribe(p.GetOrigin(), reach) == t)
			{
				nearest = t;
				relayed = true;
			}

			if (nearest != t)
				continue;

			m_mCalledTo.Set(id, true);
			string line = CallLine(t, silent);
			if (relayed && !silent)
				line = "Far off, carried from ridge to ridge. " + line;

			if (!silent)
				line += m_Rest.SkillLine(t, id, p.GetOrigin());

			m_GM.NotifyPlayer(id, "At dusk", line);
		}

		// The call as a sound at the lair (the great one's has its own, made by RZ_TribeGreat)
		if (!silent && !t.m_bGreat)
			m_Props.CallSound(t.m_vLair, false);

		if (!silent)
			RZ_Log.Info(string.Format("Tribes: %1 calls at dusk (%2 members, hunger %3 %4, %5).", t.m_sPlace, t.Members(), RZ_Util.RoundToInt(t.m_fHunger), RZ_Tribe.BandName(t.Band()), t.m_sPattern));
	}

	//------------------------------------------------------------------------------------------------
	//! What a player hears at dusk (never "alpha", "tribe" or "crawler"; always the place).
	string CallLine(RZ_Tribe t, bool silent)
	{
		string place = t.m_sPlace;
		string answers = Answers(t.Members());
		string last = m_People.LastWeekCall(t);
		if (!last.IsEmpty())
			return last;

		if (silent)
			return string.Format("Nothing from %1 tonight. Whatever lies up there ate well last night.", place);

		if (t.IsWeakened())
			return string.Format("A short call from %1, and only a few voices answering.", place);

		if (!t.m_bHasAlpha)
			return string.Format("Many voices from %1, and none of them leading.", place);

		string more = m_Borders.AnswerLine(t) + m_Life.DriftLine(t);
		if (RZ_SeasonClock.DaysSince(t.m_fLedSince) < 1)
			return string.Format("A voice nobody knows, calling from %1. %2 answer it.", place, answers) + more;

		if (t.m_bRestless)
			return string.Format("Two voices calling from %1 tonight, one over the other, and %2 answering.", place, answers) + more;

		if (t.Stage() == 2)
			return string.Format("A ragged call from %1, an old voice. %2 answer it.", place, answers) + more;

		more += m_Rest.ShortHanded(t);
		if (t.Band() >= 2)
			return string.Format("A long call from %1, and %2 answering. They'll be out early tonight.", place, answers) + more;

		return string.Format("A call from %1 at dusk, %2, and %3 answering.", place, t.m_sPattern, answers) + more;
	}

	//------------------------------------------------------------------------------------------------
	static string Answers(int n)
	{
		if (n < 6)
			return "a handful";

		if (n < 12)
			return "about ten";

		if (n < 18)
			return "a dozen or more";

		return "twenty or more";
	}

	//------------------------------------------------------------------------------------------------
	//! The set-out (19:30-20:30): tonight's trails (one; two when hungry, one of them testing the
	//! wire of the nearest lived-in place), hunters in file along them, the young ahead, the core
	//! behind (a bold alpha in front). A wounded alpha stays home with the guard; a cautious one
	//! stays home unless hungry; nobody goes out in a storm, during the vigil, or while it feeds.
	protected void SetOut(RZ_Tribe t)
	{
		// The Clash at the lair holds the set-out until it's settled
		if (t.m_bClash)
			return;

		t.m_bSetOut = true;
		t.m_aTrailsTonight.Clear();
		bool shortNight = t.m_bShortNight;
		t.m_bShortNight = false;
		if (IsStorm() || t.m_bVigil || t.m_aTrails.IsEmpty())
			return;

		t.m_bHunted = true;
		int band = t.Band();
		int trails = 1;
		if (band >= 2 && !t.IsWeakDays() && !shortNight)
			trails = 2;

		array<ref array<vector>> tonight = {};
		int first = m_Ways.PickTrail(t);
		if (first < 0)
			first = Math.RandomInt(0, t.m_aTrails.Count());

		// Clear the Trail: a dropped trail isn't walked for 3 counted nights
		if (first == t.m_iTrailOff && m_iCountedNights < t.m_iTrailOffUntil && t.m_aTrails.Count() > 1)
			first = (first + 1) % t.m_aTrails.Count();

		// Furious (its leader caught alive): every trail runs along the capture road toward the lab,
		// stopping at the lab's no man's land plus P, whatever its hunger
		array<vector> furious = {};
		if (m_People.IsFurious(t) && FuriousTrail(t, furious))
		{
			tonight.Insert(furious);
			trails = 1;
		}
		else
		{
			tonight.Insert(t.m_aTrails[first]);
			t.m_aTrailsTonight.Insert(first);
		}

		// A frenzy night: three trails, and the wire whatever its hunger; a clan's first trail runs
		// to one ground, the wire nearest the clan's middle
		bool frenzy = t.m_fFrenzy > 0 && furious.IsEmpty();
		if (frenzy)
			trails = 3;

		vector clanGround;
		if (frenzy && m_Rest.ClanGround(t, clanGround))
		{
			array<vector> toClan = {};
			BuildTrail(t.m_vLair, clanGround, toClan);
			tonight[0] = toClan;
			t.m_aTrailsTonight.Clear();
		}

		bool noWire = t.m_iNoWireNight == m_iCountedNights;
		for (int extra = 1; extra < trails; extra++)
		{
			array<vector> wire = {};
			if (extra == 1 && !noWire && t.m_eTemper != RZ_ETemperament.TERRITORIAL && WireTrail(t, wire))
			{
				tonight.Insert(wire);
			}
			else if (t.m_aTrails.Count() > extra)
			{
				int second = (first + extra) % t.m_aTrails.Count();
				if (second == t.m_iTrailOff && m_iCountedNights < t.m_iTrailOffUntil)
					continue;

				tonight.Insert(t.m_aTrails[second]);
				t.m_aTrailsTonight.Insert(second);
			}
		}

		bool coreOut = t.m_fAlphaHealth >= 0.5 && (t.m_eTemper != RZ_ETemperament.CAUTIOUS || band >= 2) && !(t.Stage() == 2 && Math.RandomFloat01() < 0.5);
		int n = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind == RZ_ETribePart.SCOUT || p.m_eKind == RZ_ETribePart.DAY || p.m_iTarget >= 0 || p.m_bGathering)
				continue;

			if (p.IsCore() && !coreOut)
				continue;

			int trailIdx = n % tonight.Count();
			array<vector> trail = tonight[trailIdx];
			if (p.m_eKind == RZ_ETribePart.YOUNG || p.IsCore())
			{
				trail = tonight[0];
				trailIdx = 0;
			}
			else
			{
				n++;
			}

			p.m_iTrail = -1;
			if (trailIdx < t.m_aTrailsTonight.Count())
				p.m_iTrail = t.m_aTrailsTonight[trailIdx];

			p.m_bWaits = false;
			p.SetPath(trail);
			p.m_bOut = true;
			p.m_bHome = false;
			p.m_bHop = false;
			p.m_fSpeed = 0;
			p.m_fHoldUntil = m_fClock;
			if (p.IsCore() && t.m_eTemper != RZ_ETemperament.BOLD)
				p.m_fHoldUntil = m_fClock + 40;		// 100-150 m behind the hunters at 3.5 m/s

			if (p.m_eKind == RZ_ETribePart.HUNTERS)
				p.m_fHoldUntil = m_fClock + 10;
		}

		Habits(t);
		m_Ways.OnSetOut(t);
		m_Rest.DuskPosts(t, tonight[0]);
		RZ_Log.Info(string.Format("Tribes: %1 sets out on %2 trails (core out %3).", t.m_sPlace, tonight.Count(), coreOut));
	}

	//------------------------------------------------------------------------------------------------
	//! Tonight's habits (docs/INFECTED.md, Habits), at 50 or more: a hunter group waits at the
	//! crossing from dusk; the young wait at the back door's wire (and a runner's tribe's young at the
	//! nearest house's wire, its echo); a hunter group walks the road habit's road.
	protected void Habits(RZ_Tribe t)
	{
		if (!m_Settings.m_bLifeCycle)
			return;

		RZ_TribeHabit crossing = t.StrongHabit(RZ_EHabit.CROSSING);
		if (crossing)
			SendToWait(t, RZ_ETribePart.HUNTERS, crossing.m_vPos);

		RZ_TribeHabit road = t.StrongHabit(RZ_EHabit.ROAD);
		if (road && Math.AbsFloat(Hour() - road.m_fHour) < 2)
			SendToWait(t, RZ_ETribePart.HUNTERS, road.m_vPos);

		RZ_TribeHabit back = t.StrongHabit(RZ_EHabit.BACKDOOR);
		if (back)
		{
			SendToWait(t, RZ_ETribePart.YOUNG, back.m_vPos);
			return;
		}

		// A runner's young wait at the nearest house's wire at dusk (its echo), and so, some
		// evenings, do the walk-outs a pack was fed (the dark corners: only ever gossip)
		array<vector> wire = {};
		bool walkouts = m_People.WalkoutsAtWire(t);
		if ((t.Echo(RZ_EAlphaKind.RUNNER) || walkouts) && WireTrail(t, wire))
			SendToWait(t, RZ_ETribePart.YOUNG, wire[wire.Count() - 1]);
	}

	//------------------------------------------------------------------------------------------------
	//! A free group of a kind walks to pos and waits there until it's sent home.
	protected void SendToWait(RZ_Tribe t, RZ_ETribePart kind, vector pos)
	{
		RZ_Land land = m_GM.GetLand();
		vector at = pos;
		if (land.InNoMansLand(at))
		{
			vector edge;
			if (!land.NearestNoMansLand(t.m_vLair, edge))
				return;

			vector back = t.m_vLair - edge;
			back[1] = 0;
			if (back.Length() < 1)
				return;

			back.Normalize();
			at = RZ_Util.GroundPos(edge + back * RZ_InfectedBodies.P);
		}

		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != kind || p.m_iTarget >= 0 || p.m_iCount <= 0 || p.m_bWaits)
				continue;

			array<vector> path = {p.m_vPos, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(at, 10, 40))};
			BuildTrail(p.m_vPos, path[1], path);
			p.SetPath(path);
			p.m_bOut = true;
			p.m_bHome = false;
			p.m_bWaits = true;
			p.m_iTrail = -1;
			p.m_fHoldUntil = m_fClock;
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A Furious tribe's trail: from the lair toward the lab, ending at the lab's no man's land plus P.
	protected bool FuriousTrail(RZ_Tribe t, array<vector> trail)
	{
		if (!m_GM.HasLab())
			return false;

		vector lab = m_GM.GetLabPosition();
		vector dir = RZ_TribePeople.Dir(lab, t.m_vLair);
		vector end = RZ_Util.GroundPos(lab + dir * (300 + RZ_InfectedBodies.P));
		if (RZ_Util.DistXZ(end, t.m_vLair) < 200)
			return false;

		BuildTrail(t.m_vLair, end, trail);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Draws a tribe to a spot (the call box, the final stand): at night its hunters and young walk
	//! there and wait, never the leader or the guard; by day only its day party, within 1.5 km. The
	//! number of parts sent.
	int DrawTo(RZ_Tribe t, vector at, bool night)
	{
		int n = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_iCount <= 0 || p.m_iTarget >= 0 || p.IsCore() || p.m_eKind == RZ_ETribePart.SCOUT)
				continue;

			if (night && p.m_eKind != RZ_ETribePart.HUNTERS && p.m_eKind != RZ_ETribePart.YOUNG && p.m_eKind != RZ_ETribePart.DAY)
				continue;

			if (!night && (p.m_eKind != RZ_ETribePart.DAY || RZ_Util.DistXZ(p.m_vPos, at) > 1500))
				continue;

			array<vector> path = {p.m_vPos, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(at, 10, 40))};
			BuildTrail(p.m_vPos, path[1], path);
			p.SetPath(path);
			p.m_bOut = true;
			p.m_bHome = false;
			p.m_bHop = false;
			p.m_bWaits = true;
			p.m_iTrail = -1;
			p.m_fHoldUntil = m_fClock;
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Testing the wire (a hungry night): a trail toward the nearest lived-in place, ending at its no
	//! man's land plus P. They come up to the wire and go home; they never go in.
	protected bool WireTrail(RZ_Tribe t, array<vector> trail)
	{
		vector end;
		if (!WireEnd(t.m_vLair, end) || RZ_Util.DistXZ(end, t.m_vLair) > GROUND_RANGE * 1.5)
			return false;

		BuildTrail(t.m_vLair, end, trail);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The wire of the lived-in place nearest from: its no man's land's edge plus P, on from's side.
	bool WireEnd(vector from, out vector end)
	{
		RZ_Land land = m_GM.GetLand();
		vector edge;
		if (!land || !land.NearestNoMansLand(from, edge))
			return false;

		vector dir = from - edge;
		dir[1] = 0;
		if (dir.Length() < 1)
			return false;

		dir.Normalize();
		end = RZ_Util.GroundPos(edge + dir * RZ_InfectedBodies.P);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Whether t's trails have fear places to go round (its own or its clan's).
	bool WaysFear(RZ_Tribe t)
	{
		return !t.m_aFear.IsEmpty() || t.m_iClan > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The Gathering (RZ_TribeGreat): one of t's hunter groups walks along the great one's way, by its
	//! lair, to the wire, and waits there until it's sent home at dawn. False if it had none free.
	bool GatherTo(RZ_Tribe t, RZ_Tribe great, vector wire)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != RZ_ETribePart.HUNTERS || p.m_iCount <= 0 || p.m_iTarget >= 0 || p.m_bGathering)
				continue;

			array<vector> path = {};
			if (t != great)
				BuildTrail(p.m_vPos, great.m_vLair, path);
			else
				path.Insert(p.m_vPos);

			array<vector> leg = {};
			BuildTrail(great.m_vLair, wire, leg);
			foreach (int i, vector v : leg)
			{
				if (i > 0 || path.IsEmpty())
					path.Insert(v);
			}

			p.SetPath(path);
			p.m_bGathering = true;
			p.m_bOut = true;
			p.m_bHome = false;
			p.m_bHop = false;
			p.m_bWaits = true;
			p.m_iTrail = -1;
			p.m_fHoldUntil = m_fClock;
			return true;
		}

		return false;
	}

	// ================================================================ Night and dawn
	//------------------------------------------------------------------------------------------------
	protected void OnNight()
	{
		m_fNightOnline = 0;
		m_Life.RoversAtPhase(RZ_ETribePhase.NIGHT);

		// A bright moon on a clear night: every pack's range x1.25, out until 05:30
		bool moon = RZ_TribeSky.BrightMoon(m_GM, m_Settings.m_fBrightMoon);
		foreach (RZ_Tribe t : m_aTribes)
		{
			t.m_bMoon = moon;
		}

		if (moon && !m_aTribes.IsEmpty())
		{
			m_bMoonTell = true;
			RZ_Log.Info(string.Format("Tribes: a bright moon tonight (phase %1): ranges x1.25, out until 05:30.", Math.Round(RZ_TribeSky.MoonPhase() * 100) / 100));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A bright moon: a line to each player within a call's reach of a pack, once a night.
	protected void TellMoon(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			RZ_Tribe t = NearestTribe(p.GetOrigin(), CALL_RANGE);
			if (t)
				m_GM.NotifyPlayer(ids[i], "Nightfall", string.Format("A full moon over %1, and not a cloud. They'll come further tonight, and they'll stay out late.", t.m_sPlace));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! 05:00: the night ends. It counts if someone was online Counted night minutes of it: then
	//! hunger rises Hunger a counted night (3; 2 more in Decline, 2 more restless), or the tribe fed
	//! and sleeps through the next day.
	protected void CountNight()
	{
		bool counted = m_fNightOnline >= Math.Max(1, m_Settings.m_fCountedNightMinutes) * 60;
		m_fNightOnline = 0;

		// A frenzy lasts the night, counted or not; its trails go round fear places again
		foreach (RZ_Tribe f : m_aTribes)
		{
			bool was = f.m_fFrenzy > 0;
			f.m_fFrenzy = 0;
			if (was && WaysFear(f))
				BuildTrails(f);
		}

		if (!counted)
			return;

		m_iCountedNights++;
		foreach (RZ_Tribe t : m_aTribes)
		{
			if (t.m_bTookTonight)
			{
				t.m_bFed = true;
				continue;
			}

			float rise = Math.Max(0, m_Settings.m_fHungerPerNight);
			if (t.Stage() == 2)
				rise += 2;

			// Restless: hungrier
			if (t.m_bRestless)
				rise += 2;

			// The fat moat and the Long Table's neighbours gain less; nothing after the cure (the last week)
			rise = m_People.NightHunger(t, rise);
			t.m_fHunger = Math.Clamp(t.m_fHunger + rise, 0, 100);
		}

		m_Ways.OnCountedNight();
		m_Rest.OnCountedNight();
		m_Life.Starvation();
		RZ_Log.Info(string.Format("Tribes: a counted night (%1 so far).", m_iCountedNights));
	}

	//------------------------------------------------------------------------------------------------
	//! Dawn (05:00): the walk home, slow and spread over the hour; hunts end; the vigil ends and one
	//! of the guard takes up the call (the full succession is planned).
	protected void OnDawn()
	{
		array<RZ_Tribe> list = {};
		foreach (RZ_Tribe each : m_aTribes)
		{
			list.Insert(each);
		}

		foreach (RZ_Tribe t : list)
		{
			// Under a bright moon the hunt goes on until 05:30
			if (!t.m_bMoon || t.m_bVigil)
				m_Bodies.EndHunt(t, false);

			t.m_fFeedUntil = -1;
			t.m_iDayPacks = 0;
			if (t.m_bVigil)
				EndVigil(t);

			if (m_aTribes.Find(t) < 0)
				continue;

			// A bad night leaves stragglers out, to come home late
			m_Rest.Dawn(t);

			// Echoes: a soldier's tribe comes home at 05:30, a farmer's at first light by the fields;
			// the column walks toward the line first
			m_Ways.ColumnDawn(t);
			if (t.Echo(RZ_EAlphaKind.SOLDIER) || t.Echo(RZ_EAlphaKind.FARMER) || t.m_bMoon)
			{
				t.m_bLateHome = true;
				continue;
			}

			SendHome(t, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every part not at the lair walks home (the core only, with coreOnly).
	void SendHome(RZ_Tribe t, bool coreOnly)
	{
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (!p.m_bOut || p.m_bHome)
				continue;

			if (coreOnly && !p.IsCore())
				continue;

			// A straggler after a bad night comes home late, on its own (RZ_TribeRest)
			if (m_Rest.IsStraggler(p))
				continue;

			PathHome(t, p);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A part's path back to its spot at the lair.
	void PathHome(RZ_Tribe t, RZ_TribePart p)
	{
		array<vector> home = {p.m_vPos, p.m_vRest};
		p.SetPath(home);
		p.m_iStep = 1;
		p.m_vTarget = p.m_vRest;
		p.m_bHome = true;
		p.m_bHop = false;
		p.m_bGathering = false;
		p.m_iTarget = -1;
		p.m_fHoldUntil = m_fClock + Math.RandomFloat(0, 600);
	}

	//------------------------------------------------------------------------------------------------
	//! The dawn count (06:00-07:00): the call once more at the lair, and the answers say how many came
	//! back. Players within 2 km hear it.
	protected void DawnCount(RZ_Tribe t, array<IEntity> players, array<int> ids)
	{
		t.m_bDawnCalled = true;
		if (!t.m_bHunted || !t.m_bHasAlpha)
			return;

		string line = string.Format("A call from %1 at first light. The same answers as at dusk.", t.m_sPlace);
		if (t.m_iLostTonight > 0)
			line = string.Format("A call from %1 at first light, and fewer answers than at dusk.", t.m_sPlace);

		Tell(t.m_vLair, DAWN_RANGE, "First light", line);
		RZ_Log.Info(string.Format("Tribes: %1 dawn count: %2 of %3 came home.", t.m_sPlace, t.Members(), t.m_iAtDusk));
	}

	//------------------------------------------------------------------------------------------------
	//! The vigil ends at dawn: one of the guard takes up the call (Rising), else the strongest hunter.
	//! Simplified: the guard fight, the splinters and the weak days are planned (phase 3).
	protected void EndVigil(RZ_Tribe t)
	{
		t.m_bVigil = false;
		m_Rest.VigilEnds(t);
		if (t.m_bHasAlpha)
			return;

		// The life cycle: the guard fight, the splinters, the weak days (RZ_TribeLife)
		if (m_Settings.m_bLifeCycle)
		{
			m_Life.Succession(t);
			return;
		}

		RZ_TribePart from = RestingPart(t, RZ_ETribePart.GUARD);
		if (!from)
			from = RestingPart(t, RZ_ETribePart.HUNTERS);

		if (!from || from.m_iCount <= 0)
		{
			RZ_Log.Info(string.Format("Tribes: %1 has nobody left to call.", t.m_sPlace));
			return;
		}

		from.m_iCount = from.m_iCount - 1;
		if (from.m_iYoung > from.m_iCount)
			from.m_iYoung = from.m_iCount;

		t.m_bHasAlpha = true;
		t.m_fLedSince = RZ_SeasonClock.Days();
		t.m_fAlphaHealth = 1;
		t.m_iAlphaTaken = 0;
		t.m_sAlphaWho = "one of its own guard";
		AddPart(t, RZ_ETribePart.ALPHA, 1, 0);
		RZ_Log.Info(string.Format("Tribes: %1 has a new voice at first light.", t.m_sPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! The alpha fell: the vigil (docs/INFECTED.md, The vigil). No hunt that night; the tribe holds
	//! where it fell (at the lair if that was outside the range) until dawn; bodies near players
	//! stand round it 30 real minutes at most.
	void StartVigil(RZ_Tribe t, vector at)
	{
		// The great one: its whole clan holds the vigil, and the clan breaks up (RZ_TribeGreat)
		if (t.m_bGreat)
			m_Great.OnGreatDown(t);

		// A named alpha's history in the headlines, always with the place
		if (t.m_bHasAlpha && !t.m_sAlphaName.IsEmpty() && m_Settings.m_bLifeCycle)
		{
			string last = "";
			if (!t.m_aHistory.IsEmpty())
				last = string.Format(" %1 %2.", t.m_sAlphaName, t.m_aHistory[0]);

			m_Ways.Headline(t, string.Format("The calling at %1 stopped tonight. %2 led them, and doesn't any more.%3", t.m_sPlace, t.m_sAlphaName, last));
		}

		t.m_bHasAlpha = false;
		t.m_bClash = false;
		t.m_bClashDue = false;
		t.m_bVigil = true;
		t.m_vVigilAt = at;
		if (RZ_Util.DistXZ(at, t.m_vLair) > t.m_fRadius)
			t.m_vVigilAt = t.m_vLair;

		t.m_fVigilBodiesUntil = RZ_BaconBody.Now() + 1800;
		m_Bodies.EndHunt(t, false);
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind == RZ_ETribePart.SCOUT)
				continue;

			p.m_iTarget = -1;
			array<vector> path = {p.m_vPos, RZ_Util.GroundPos(RZ_Util.RandomPointInRing(t.m_vVigilAt, 10, 30))};
			p.SetPath(path);
			p.m_iStep = 1;
			p.m_vTarget = path[1];
			p.m_bOut = true;
			p.m_bHome = false;
			p.m_fHoldUntil = m_fClock;
		}

		Tell(at, LONG_CALL_RANGE, "Quiet", string.Format("The calling from %1 stops. Then nothing at all.", t.m_sPlace));
		RZ_Log.Info(string.Format("Tribes: %1 lost its leader; the vigil at %2 until dawn.", t.m_sPlace, RZ_SettlementScanner.GridReference(at)));
	}

	// ================================================================ The data tick
	//------------------------------------------------------------------------------------------------
	//! Once a played minute: parts walk their paths (hop and hold by day), hear remembered noises,
	//! take wanderers as data at night, leave tracks and stir towns; the alpha heals at the lair; the
	//! ones who ran are banked; Hunters read tracks.
	protected void DataTick(float dt, array<int> ids, array<IEntity> players)
	{
		BankTheOnesWhoRan();
		float fade = Math.Pow(2.71828, -dt / 36000);
		m_fTribeSamples = m_fTribeSamples * fade;
		m_fAllSamples = m_fAllSamples * fade;

		for (int ti = m_aTribes.Count() - 1; ti >= 0; ti--)
		{
			RZ_Tribe t = m_aTribes[ti];
			for (int i = t.m_aParts.Count() - 1; i >= 0; i--)
			{
				RZ_TribePart p = t.m_aParts[i];
				if (p.m_iCount <= 0 && !p.HasBody())
				{
					t.m_aParts.Remove(i);
					continue;
				}

				StepPart(t, p, dt, players);
			}

			if (t.Members() <= 0)
			{
				RZ_Log.Info(string.Format("Tribes: %1 is gone; its slot is free.", t.m_sPlace));
				if (m_Settings.m_bLifeCycle)
					m_Life.RemoveTribe(t, true);
				else
					m_aTribes.Remove(ti);

				continue;
			}

			// The alpha heals 0.1 an hour played while it lies up at the lair (infected never heal by themselves)
			RZ_TribePart alpha = t.FindPart(RZ_ETribePart.ALPHA);
			if (alpha && !alpha.m_bOut && t.m_fAlphaHealth < 1 && !alpha.HasBody())
				t.m_fAlphaHealth = Math.Min(1, t.m_fAlphaHealth + 0.1 * dt / 3600);

			PruneTracks(t);
			t.CountRoles();
		}

		ReadTracks(ids, players);
		m_Rest.SeePrey(players);
		m_Rest.TrucksAtCrossings();
		if (m_ePhase == RZ_ETribePhase.DAY)
		{
			RZ_Environment env = m_GM.GetEnvironment();
			bool fog = env && env.IsFoggy();
			foreach (RZ_Tribe pt : m_aTribes)
			{
				m_Rest.Posts(pt, fog);
			}
		}

		int day = RZ_SeasonClock.Day();
		m_iLastSeasonDay = day;
		m_Life.Tick(dt, ids, players);
		m_Borders.NightTick();
	}

	//------------------------------------------------------------------------------------------------
	//! One part, one minute: its noise, its hold, its path (patrolling a trail, hopping a day walk,
	//! going home), then moving as data (bodies write their own position back).
	void StepPart(RZ_Tribe t, RZ_TribePart p, float dt, array<IEntity> players)
	{
		if (p.m_iTarget >= 0)
			return;

		Hear(t, p);
		Take(t, p, players);
		LeaveTrack(t, p);
		StirTown(t, p);
		if (m_fClock < p.m_fNoiseUntil)
		{
			p.m_vTarget = p.m_vNoise;
			p.m_bHasTarget = true;
			p.MoveAsData(dt);
			return;
		}

		if (m_fClock < p.m_fHoldUntil || p.m_aPath.IsEmpty())
		{
			p.m_bHasTarget = false;
			if (p.m_aPath.IsEmpty() && !p.m_bOut && !p.HasBody())
				p.m_vPos = p.m_vRest;

			return;
		}

		// Weakened: hunts as data only, and keeps 600 m from players
		if (t.IsWeakened() && p.m_bOut && !p.m_bHome && RZ_Util.NearestDistance(p.m_vPos, players) < 600)
		{
			p.m_bHasTarget = false;
			return;
		}

		p.m_vTarget = p.m_aPath[p.m_iStep];
		p.m_bHasTarget = true;
		float reach = 20;
		if (p.HasBody())
			reach = 30;

		if (RZ_Util.DistXZ(p.m_vPos, p.m_vTarget) <= reach)
			NextStep(t, p);

		p.MoveAsData(dt);
		if (p.m_bHop)
		{
			// Bodies write their own position back, so the leg counts what it really walked
			p.m_fLegLeft -= RZ_Util.DistXZ(p.m_vPos, p.m_vLastStep);
			p.m_vLastStep = p.m_vPos;
			if (p.m_fLegLeft <= 0)
			{
				// Hop and hold: a leg of 100-200 m, then 4-6 minutes still
				p.m_fLegLeft = Math.RandomFloat(100, 200);
				p.m_fHoldUntil = m_fClock + Math.RandomFloat(240, 360);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At a path point: the next one; at the end of a trail 10 minutes at the hunting ground, then
	//! back; at the lair end 5 minutes, then out again; home: arrived.
	protected void NextStep(RZ_Tribe t, RZ_TribePart p)
	{
		int next = p.m_iStep + p.m_iDir;
		if (next >= 0 && next < p.m_aPath.Count())
		{
			p.m_iStep = next;
			p.m_vTarget = p.m_aPath[next];
			return;
		}

		// Waiting at a crossing, a wire or a border until it's sent home
		if (p.m_bWaits && !p.m_bHome)
		{
			p.m_fHoldUntil = m_fClock + 120;
			return;
		}

		if (p.m_bHome || p.m_bPack)
		{
			p.m_aPath.Clear();
			p.m_bOut = false;
			p.m_bHome = false;
			p.m_bHasTarget = false;
			return;
		}

		if (p.m_bHop)
		{
			// A day walk loops until it's called back
			p.m_iStep = 0;
			p.m_fHoldUntil = m_fClock + Math.RandomFloat(240, 360);
			return;
		}

		p.m_iDir = -p.m_iDir;
		p.m_iStep = Math.ClampInt(p.m_iStep + p.m_iDir, 0, p.m_aPath.Count() - 1);
		if (p.m_iDir < 0)
			p.m_fHoldUntil = m_fClock + 600;
		else
			p.m_fHoldUntil = m_fClock + 300;
	}

	//------------------------------------------------------------------------------------------------
	//! Hearing as data (rule 7): a part out on its trail at night (by day only the day party; never
	//! the resting ring or the core) turns to a remembered noise within 700 m (the young 400; a
	//! cautious tribe ignores noises beyond 400) and waits there 3 minutes, once per noise.
	protected void Hear(RZ_Tribe t, RZ_TribePart p)
	{
		bool night = m_ePhase == RZ_ETribePhase.NIGHT;
		bool may = (night && p.m_bOut && p.IsHunting()) || (!night && p.m_eKind == RZ_ETribePart.DAY && p.m_bOut);
		if (!may || p.m_bHome)
			return;

		float radius = 700;
		if (p.m_eKind == RZ_ETribePart.YOUNG || t.m_eTemper == RZ_ETemperament.CAUTIOUS)
			radius = 400;

		radius = radius * m_Ways.HearingMult(t, p);

		int id;
		vector at;
		if (!m_GM.GetHeat().NewestNoise(p.m_iHeard, p.m_vPos, radius, 300, id, at))
			return;

		p.m_iHeard = id;
		p.m_vNoise = RZ_Util.GroundPos(at);
		p.m_fNoiseUntil = m_fClock + RZ_Util.DistXZ(p.m_vPos, at) / RZ_InfectedBodies.DATA_SPEED + 180;
	}

	//------------------------------------------------------------------------------------------------
	//! Takings as data (rule 7), at night with no player within 1 km: a wanderer near a hunting
	//! part's trail is taken one time in ten, at most one a tribe every 3 counted nights. Trucks
	//! taken as data: RZ_TribeLife.TryTruck, and where a trail crosses a road (RZ_TribeRest).
	protected void Take(RZ_Tribe t, RZ_TribePart p, array<IEntity> players)
	{
		if (m_ePhase != RZ_ETribePhase.NIGHT || !p.m_bOut || !p.IsHunting() || p.HasBody() || t.IsRover())
			return;

		if (RZ_Util.NearestDistance(p.m_vPos, players) < 1000)
			return;

		// Supply trucks as data (the truck's own failure path)
		if (m_Life.TryTruck(t, p))
			return;

		// A wanderer a tribe every 3 counted nights (decision 15)
		if (m_iCountedNights - t.m_iLastWandererNight < 3 || Math.RandomFloat01() > 0.1)
			return;

		RZ_Wanderers wanderers = m_GM.GetWanderers();
		if (!wanderers || !wanderers.TakeNear(p.m_vPos, 150))
			return;

		t.m_iLastWandererNight = m_iCountedNights;
		// Traces' V5: a runner taken within 3 km of the lab or a Halcyon field station leaves his case
		// in the ditch (World/Traces/RZ_Scenes.c)
		RZ_Traces traces = m_GM.GetTraces();
		RZ_Territory territory = m_GM.GetTerritory();
		bool nearLab = m_GM.HasLab() && RZ_Util.DistXZ(p.m_vPos, m_GM.GetLabPosition()) <= 3000;
		if (!nearLab && territory)
			nearLab = territory.FindHolding(RZ_EFaction.INSTITUTE, p.m_vPos, 3000) != null;

		if (traces && traces.IsOn() && nearLab)
			traces.GetScenes().DroppedRun(p.m_vPos, "a pack took a runner");
	}

	//------------------------------------------------------------------------------------------------
	//! A hunting part passing through a town at night stirs it (+30, once a night).
	protected void StirTown(RZ_Tribe t, RZ_TribePart p)
	{
		if (m_ePhase != RZ_ETribePhase.NIGHT || !p.m_bOut || p.m_bStirred)
			return;

		int idx = m_GM.FindNearestSettlement(p.m_vPos);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s || RZ_Util.DistXZ(s.m_vCenter, p.m_vPos) > s.m_fRadius + 50)
			return;

		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (crawlers && crawlers.IsEnabled())
			crawlers.AddStir(p.m_vPos, 30);

		p.m_bStirred = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The ones who ran (rule 5): each town's rise in infection is banked to the nearest tribe whose
	//! range comes within 1.5 km of it; every 4 points is one young at the next dusk, 2 a season day
	//! at most, 8 points banked at most; halved from the 75% serum, none after the cure.
	protected void BankTheOnesWhoRan()
	{
		array<ref RZ_Settlement> towns = m_GM.GetSettlements();
		bool fresh = m_aLastInfection.Count() != towns.Count();
		if (fresh)
		{
			m_aLastInfection.Clear();
			foreach (RZ_Settlement s0 : towns)
			{
				m_aLastInfection.Insert(s0.m_fInfection);
			}

			return;
		}

		RZ_CureResearch cure = m_GM.GetCure();
		bool cured = cure && cure.IsCured();
		float mult = 1;
		if (cure && cure.m_fProgress >= 75)
			mult = 0.5;

		int today = RZ_SeasonClock.Day();
		foreach (int i, RZ_Settlement s : towns)
		{
			float rise = s.m_fInfection - m_aLastInfection[i];
			m_aLastInfection[i] = s.m_fInfection;
			if (rise <= 0 || cured)
				continue;

			RZ_Tribe best;
			float bestEdge = 1500;
			foreach (RZ_Tribe t : m_aTribes)
			{
				float edge = RZ_Util.DistXZ(t.RangeCentre(), s.m_vCenter) - t.m_fRadius;
				if (edge <= bestEdge)
				{
					bestEdge = edge;
					best = t;
				}
			}

			if (!best)
				continue;

			if (best.m_iRanDay != today)
			{
				best.m_iRanDay = today;
				best.m_iRanToday = 0;
			}

			best.m_fBank = Math.Min(8, best.m_fBank + rise * mult);
			while (best.m_fBank >= 4 && best.m_iRanToday < 2)
			{
				best.m_fBank -= 4;
				best.m_iRanToday++;
				best.m_iPendingYoung++;
				m_Ways.OnJoined(best, false);
				RZ_Log.Info(string.Format("Tribes: someone from %1 ran, and joins %2 at dusk.", s.m_sName, best.m_sPlace));
			}
		}
	}

	// ================================================================ The taken
	//------------------------------------------------------------------------------------------------
	//! A player died (RZ_FactionPerks): if a tribe's body hit them in the last 60 s, they were taken by
	//! that tribe (in the open; inside a town they're a crawler, and the town stirs +30).
	void OnPlayerDied(int playerId, vector at)
	{
		if (!IsEnabled() || m_aTribes.IsEmpty())
			return;

		int role;
		int tribeId;
		int town;
		if (!m_GM.GetBiteDetector().GetLastHit(playerId, 60, role, tribeId, town))
			return;

		RZ_Tribe t = Find(tribeId);
		if (t && t.IsRover())
			t = null;

		if (!t && !InTown(at))
			t = TribeForOpen(at);

		if (!t)
		{
			if (!InTown(at))
				m_Life.OnOpenDeath(at, Who(playerId));

			return;
		}

		if (InTown(at))
		{
			RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
			if (crawlers && crawlers.IsEnabled())
				crawlers.AddStir(at, 30);

			return;
		}

		TakePlayer(t, playerId, at);
	}

	//------------------------------------------------------------------------------------------------
	//! A player turned (RZ_GameModeComponent.TurnPlayer): in the open within 1.5 x a range radius of a
	//! tribe, that tribe takes them.
	void OnTurned(int playerId, vector at)
	{
		if (!IsEnabled() || InTown(at))
			return;

		RZ_Tribe t = TribeForOpen(at);
		if (t)
			TakePlayer(t, playerId, at);
		else
			m_Life.OnOpenDeath(at, Who(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer died to the infected or turned (RZ_Wanderers.Die): in the open near a tribe's
	//! range, that tribe took them.
	void OnWandererDied(vector at, string who, string how, int killer)
	{
		if (!IsEnabled() || killer > 0 || InTown(at))
			return;

		if (!how.Contains("infected") && how != "turned")
			return;

		RZ_Tribe t = TribeForOpen(at);
		if (t)
		{
			Taken(t, at, who, true);
			m_People.OnNamedTaken(t, who, at, false);
		}
		else
		{
			m_Life.OnOpenDeath(at, who);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An outpost's or a safehouse's leader bitten off-screen (RZ_Leaders): in the open near a pack's
	//! range, the pack took them (the taken, rule 4); in a town, they're the town's.
	void OnLeaderBitten(vector at, string name)
	{
		if (!IsEnabled() || m_aTribes.IsEmpty() || InTown(at))
			return;

		RZ_Tribe t = TribeForOpen(at);
		if (!t)
			return;

		Taken(t, at, name, true);
		m_People.OnNamedTaken(t, name, at, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Inside a settlement's radius + 100 m.
	protected bool InTown(vector at)
	{
		int idx = m_GM.FindNearestSettlement(at);
		RZ_Settlement s = m_GM.GetSettlement(idx);
		return s && RZ_Util.DistXZ(s.m_vCenter, at) <= s.m_fRadius + 100;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest tribe whose lair is within 1.5 x its range radius of at.
	protected RZ_Tribe TribeForOpen(vector at)
	{
		RZ_Tribe best;
		float bestDist = 100000;
		foreach (RZ_Tribe t : m_aTribes)
		{
			float d = RZ_Util.DistXZ(t.RangeCentre(), at);
			if (d <= t.m_fRadius * 1.5 && d < bestDist)
			{
				bestDist = d;
				best = t;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A player's character taken: a young at the next dusk if Your dead run allows it (a life of 10
	//! minutes or more, once per player a season day, 3 players' young per tribe at most).
	protected void TakePlayer(RZ_Tribe t, int playerId, vector at)
	{
		bool young = m_Settings.m_eYourDeadRun != RZ_EDeadRun.OFF;
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		if (profile && profile.m_fLifeSeconds < 600)
			young = false;

		int day = RZ_SeasonClock.Day();
		int last;
		if (m_mTakenDay.Find(playerId, last) && last == day)
			young = false;

		if (t.m_iPlayerYoung >= 3)
			young = false;

		if (young)
		{
			m_mTakenDay.Set(playerId, day);
			t.m_iPlayerYoung++;
		}

		// A Soldier's or a Police Officer's character makes the young's clothes military
		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		if (young && bg)
			m_Ways.OnJoined(t, bg.m_sId == "soldier" || bg.m_sId == "police");

		Taken(t, at, Who(playerId), young);
		m_People.OnPlayerTaken(t, playerId, at, Described(playerId), young);
	}

	//------------------------------------------------------------------------------------------------
	//! How a player's character is named in public text: their name with Your dead run on Named,
	//! else a description from their background ("someone who'd been a hunter").
	protected string Who(int playerId)
	{
		return m_People.PublicName(playerId, RZ_Util.PlayerName(playerId), Described(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! A player's character described: "someone who'd been a hunter".
	protected string Described(int playerId)
	{
		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		string what = "a traveller";
		if (bg && !bg.m_sName.IsEmpty())
		{
			what = bg.m_sName;
			what.ToLower();
			what = "a " + what;
		}

		return "someone who'd been " + what;
	}

	//------------------------------------------------------------------------------------------------
	//! Someone taken (rule 4): -25 hunger, a young at the next dusk (when young), the alpha's count,
	//! the kill site (crawlers come out to it the next night) where the tribe feeds in order.
	void Taken(RZ_Tribe t, vector at, string who, bool young)
	{
		// After the cure nobody new turns; from the 75% serum each one taken comes back half the time
		RZ_CureResearch cure = m_GM.GetCure();
		if (young && m_People.IsOn() && cure && (cure.IsCured() || (cure.m_fProgress >= 75 && Math.RandomFloat01() < 0.5)))
			young = false;

		t.m_fHunger = Math.Max(0, t.m_fHunger - 25);
		if (young)
			t.m_iPendingYoung++;

		if (t.m_bHasAlpha)
			t.m_iAlphaTaken++;

		t.AddTaken(who);
		m_Ways.OnTaken(t, at, who);
		m_Rest.OnTaken(t, at);
		string low = who;
		low.ToLower();
		if (young && (low.Contains("soldier") || low.Contains("patrol") || low.Contains("deserter")))
			m_Ways.OnJoined(t, true);

		if (RZ_TribeWays.IsAName(who))
			t.AddHistory("took " + who);

		m_People.OnTaken(t, who);
		if (m_ePhase != RZ_ETribePhase.DAY)
		{
			t.m_bTookTonight = true;
			t.m_vFeedAt = RZ_Util.GroundPos(at);
			t.m_fFeedUntil = m_fClock + 1200;
			m_Bodies.Feed(t);
		}

		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (crawlers && crawlers.IsEnabled())
			crawlers.AddKillSite(at, who);

		RZ_Log.Info(string.Format("Tribes: %1 took %2 at %3 (hunger %4, young %5).", t.m_sPlace, who, RZ_SettlementScanner.GridReference(at), RZ_Util.RoundToInt(t.m_fHunger), young));
	}

	// ================================================================ Hooks
	//------------------------------------------------------------------------------------------------
	//! A building burned or a molotov landed (RZ_Traps): burned land, and a lair disturbed or burned.
	//! playerId <= 0: an event's fire.
	void OnFire(vector pos, int playerId)
	{
		if (!IsEnabled())
			return;

		RZ_TribeContract.OnFire(m_GM, pos, playerId);
		m_Life.OnFire(pos, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Shells landed (RZ_WorldWriters.Shell): a lair within 200 m is disturbed (+50, by Bastion).
	void OnShell(vector pos)
	{
		if (IsEnabled())
			m_Life.OnShell(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! A player fired (the game mode): the shots habit remembers it.
	void OnShot(vector pos)
	{
		if (IsEnabled())
			m_Ways.OnShot(pos);
	}

	// ================================================================ Samples
	//------------------------------------------------------------------------------------------------
	//! A tribe body died (the spawner's per-member tracking). Hunt losses count; the alpha's death
	//! starts the vigil. Samples (docs/INFECTED.md, Samples) only if a player was within 150 m: the
	//! alpha by stage (Rising none, Prime 2, Decline 3; 3 times a season per tribe), a guard 1 under a
	//! Prime or older alpha (one a tribe every 2 played hours). Silent kills give nothing.
	void OnMemberKilled(RZ_TrackedZombie z, vector at, bool silent)
	{
		RZ_Tribe t = Find(z.m_iTribe);
		if (!t)
			return;

		RZ_TribePart p = RZ_TribePart.Cast(z.m_Part);
		if (p && p.m_iYoung > p.m_iCount)
			p.m_iYoung = p.m_iCount;

		RZ_TribeContract.OnKill(m_GM, t, z.m_eRole, p, at, silent);
		if (t.IsRover())
		{
			m_Life.OnRoverKilled(t, z.m_eRole, at, silent);
			return;
		}

		if (m_ePhase != RZ_ETribePhase.DAY)
		{
			t.m_iLostTonight++;
			if (!silent)
				m_Ways.OnDiedAtNight(t, at);
		}

		m_Bodies.OnLoss(t, p);
		if (!silent)
			m_Rest.OnDied(t, at);

		if (!silent && (z.m_eRole == RZ_EInfectedRole.YOUNG || (p && p.m_eKind == RZ_ETribePart.YOUNG)))
			m_People.OnYoungDown(t, at, RZ_TribeContract.WantedYoung(m_GM, t, at));

		if (!silent && m_Settings.m_bLifeCycle)
		{
			m_Ways.OnKilledNear(t, z.m_eRole, at);
			if (z.m_eRole == RZ_EInfectedRole.YOUNG || (p && p.m_eKind == RZ_ETribePart.YOUNG))
				m_Ways.OnYoungKilled(t, at);

			if (z.m_eRole == RZ_EInfectedRole.GUARD && RZ_Util.DistXZ(at, t.m_vLair) <= 150)
				m_Life.Disturb(t, 15, -1);
		}

		// The Clash: the old one shot, the challenger takes over with no vigil
		if (m_Life.OnClashKill(t, z.m_eRole))
			return;

		// Only the tribe's own leader starts the vigil (a deposed one's body walks with the lone one)
		if (z.m_eRole == RZ_EInfectedRole.ALPHA && p && t.FindPart(RZ_ETribePart.ALPHA) == p)
		{
			int stage = t.Stage();
			if (!silent)
			{
				m_People.OnLeaderKilled(t, at);
				RZ_RepVoice.PackLeaderDown(m_GM, at);
			}

			StartVigil(t, at);
			int n = 0;
			if (!silent && PlayerWithin(at, 150) && !Cured() && t.m_iAlphaSamples < 3 && stage > 0)
			{
				n = 2;
				if (stage == 2)
					n = 3;

				t.m_iAlphaSamples++;
				OnSample(n, true);
			}

			// Its samples and its body are kept through the vigil (RZ_TribeRest)
			m_Rest.VigilKeep(t, z, at, n);
			if (n > 0)
				m_People.OnSampleNear(t, at);

			return;
		}

		if (silent || z.m_eRole != RZ_EInfectedRole.GUARD || !t.m_bHasAlpha || t.Stage() < 1 || Cured())
			return;

		float played = m_fClock / 60;
		if (played - t.m_fGuardSampleAt < 120 || !PlayerWithin(at, 150) || !GuardSampleRoom(played))
			return;

		t.m_fGuardSampleAt = played;
		m_aGuardSampleAt.Insert(played);
		m_fGuardSamples += 1;
		m_GM.SpawnSamples(at, 1);
		OnSample(1, true);
		m_People.OnSampleNear(t, at);
	}

	//------------------------------------------------------------------------------------------------
	protected bool Cured()
	{
		return m_GM.GetCure() && m_GM.GetCure().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	static bool PlayerWithin(vector at, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return RZ_Util.NearestDistance(at, players) <= radius;
	}

	//------------------------------------------------------------------------------------------------
	//! Samples dropped (tribe: by a tribe body), for the town specials' scaling.
	void OnSample(int n, bool tribe)
	{
		m_fAllSamples += n;
		if (tribe)
		{
			m_fTribeSamples += n;
			m_fPackSamples += n;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At most 4 guard samples a played hour on the whole server.
	protected bool GuardSampleRoom(float played)
	{
		for (int i = m_aGuardSampleAt.Count() - 1; i >= 0; i--)
		{
			if (played - m_aGuardSampleAt[i] >= 60)
				m_aGuardSampleAt.Remove(i);
		}

		return m_aGuardSampleAt.Count() < 4;
	}

	//------------------------------------------------------------------------------------------------
	//! Town specials scaled by the tribes' share of samples over the last 10 played hours, never
	//! below x0.6, so specials per player-hour stay near the season pacing's assumptions.
	float TownSpecialMult()
	{
		if (m_fAllSamples < 1)
			return 1;

		return Math.Clamp(1 - m_fTribeSamples / m_fAllSamples, 0.6, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A part's bodies went back to data (the spawner): the alpha's health is kept as data.
	void OnPartEnds(RZ_TribePart p, RZ_TrackedZombie z)
	{
		if (!p || p.m_eKind != RZ_ETribePart.ALPHA || !z.m_Entity)
			return;

		RZ_Tribe t = Find(p.m_iTribe);
		if (!t)
			return;

		float hp = m_Bodies.HealthOf(RZ_Tribe.FirstMember(z.m_Entity));
		if (hp >= 0)
			t.m_fAlphaHealth = hp;
	}

	//------------------------------------------------------------------------------------------------
	//! An alpha wounded in the last 10 minutes isn't removed until every player is 1.5 km away.
	bool KeepBody(RZ_TrackedZombie z, float nearest)
	{
		if (z.m_eRole != RZ_EInfectedRole.ALPHA || nearest >= 1500)
			return false;

		RZ_Tribe t = Find(z.m_iTribe);
		return t && m_fClock - t.m_fAlphaHurtAt < 600;
	}

	// ================================================================ Tracks
	//------------------------------------------------------------------------------------------------
	//! A part out and moving leaves a track every 150 m or so: how many, which way, whether the big
	//! one was with them, how worn the ground is.
	protected void LeaveTrack(RZ_Tribe t, RZ_TribePart p)
	{
		if (!p.m_bOut || !p.m_bHasTarget || p.m_eKind == RZ_ETribePart.SCOUT)
			return;

		int wear = 1;
		foreach (RZ_Track known : t.m_aTracks)
		{
			if (RZ_Util.DistXZ(known.m_vPos, p.m_vPos) < 80)
			{
				if (m_fClock - known.m_fAt < 1800)
					return;

				wear = Math.Max(wear, known.m_iWear + 1);
			}
		}

		RZ_Track tr = new RZ_Track();
		tr.m_vPos = p.m_vPos;
		tr.m_fAt = m_fClock;
		tr.m_iCount = p.m_iCount;
		vector h = p.m_vTarget - p.m_vPos;
		h[1] = 0;
		if (h.Length() > 1)
			h.Normalize();

		tr.m_vHeading = h;
		tr.m_bBig = p.m_eKind == RZ_ETribePart.ALPHA;
		tr.m_bYoung = p.m_eKind == RZ_ETribePart.YOUNG || p.m_iYoung > 0;
		tr.m_bNight = m_ePhase == RZ_ETribePhase.NIGHT;
		tr.m_iWear = Math.Min(10, wear);
		if (p.m_iTrail >= 0 && m_Settings.m_bLifeCycle)
			tr.m_iWear = RZ_TribeWays.WearOf(t, p.m_iTrail);
		t.m_aTracks.Insert(tr);
		while (t.m_aTracks.Count() > 40)
		{
			t.m_aTracks.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void PruneTracks(RZ_Tribe t)
	{
		// In snow (the Winter rule set) a track keeps twice as long, and always reads fresh
		float keep = TRACK_KEEP;
		if (RZ_TribeLife.IsWinter())
			keep = TRACK_KEEP * 2;

		for (int i = t.m_aTracks.Count() - 1; i >= 0; i--)
		{
			if (m_fClock - t.m_aTracks[i].m_fAt > keep)
				t.m_aTracks.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A Hunter within 40 m of a track reads it (once every 3 minutes): "Nine or ten, heading north,
	//! last night. A big one with them. They come this way most nights." Near a lair by day: what
	//! lies up there.
	protected void ReadTracks(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			RZ_Background bg = m_GM.GetPlayerBackground(id);
			if (!bg || bg.m_sId != "hunter")
				continue;

			float told;
			if (m_mTrackToldAt.Find(id, told) && m_fClock - told < 180)
				continue;

			string line = TrackLine(p.GetOrigin());
			if (line.IsEmpty())
				continue;

			m_mTrackToldAt.Set(id, m_fClock);
			m_GM.NotifyPlayer(id, "Tracks", line);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string TrackLine(vector at)
	{
		string fear = m_Ways.FearLine(at);
		if (!fear.IsEmpty())
			return fear;

		RZ_Track best;
		float bestDist = TRACK_READ;
		foreach (RZ_Tribe t : m_aTribes)
		{
			if (m_ePhase == RZ_ETribePhase.DAY && RZ_Util.DistXZ(t.m_vLair, at) < 60)
				return "No birds in this wood, and the ground's laid flat under the trees. Something lies up here by day.";

			foreach (RZ_Track tr : t.m_aTracks)
			{
				float d = RZ_Util.DistXZ(tr.m_vPos, at);
				if (d < bestDist)
				{
					bestDist = d;
					best = tr;
				}
			}
		}

		if (!best)
			return "";

		int lo = Math.Max(1, best.m_iCount - 1);
		string count = string.Format("%1 or %2", Capitalize(NumberWord(lo)), NumberWord(lo + 1));
		if (best.m_iCount <= 1)
			count = "One";

		string when = "within the hour";
		float age = m_fClock - best.m_fAt;
		if (best.m_bNight && m_ePhase != RZ_ETribePhase.NIGHT)
			when = "last night";
		else if (age > 10800)
			when = "earlier today";
		else if (age > 3600)
			when = "a few hours ago";

		string line = string.Format("%1, heading %2, %3.", count, CompassWord(vector.Zero, best.m_vHeading), when);
		if (RZ_TribeLife.IsWinter())
			line += " Sharp as the day they were made, in this snow.";
		if (best.m_bBig)
			line += " A big one with them.";

		if (best.m_bYoung)
			line += " Young ones all over the place.";

		if (m_Settings.m_bLifeCycle && best.m_bNight)
			line += RZ_TribeWays.WearLine(best.m_iWear);
		else if (best.m_iWear >= 3)
			line += " They come this way most nights.";

		return line;
	}

	//------------------------------------------------------------------------------------------------
	static string NumberWord(int n)
	{
		array<string> words = {"none", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve"};
		if (n >= 0 && n < words.Count())
			return words[n];

		return "a dozen";
	}

	//------------------------------------------------------------------------------------------------
	static string Capitalize(string text)
	{
		return RZ_Util.Capitalize(text);
	}

	// ================================================================ Lines and lookups
	//------------------------------------------------------------------------------------------------
	//! The tribe whose lair is nearest pos, within range.
	RZ_Tribe NearestTribe(vector pos, float range)
	{
		RZ_Tribe best;
		float bestDist = range;
		foreach (RZ_Tribe t : m_aTribes)
		{
			float d = RZ_Util.DistXZ(t.m_vLair, pos);
			if (d <= bestDist)
			{
				bestDist = d;
				best = t;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A line to every living player within radius of pos.
	void Tell(vector pos, float radius, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The prefab list for a part's bodies, with the fallbacks (docs/INFECTED.md, Roles): the alpha
	//! and guard from the Special prefabs, hunters from Common, scouts and young from hunters.
	array<ResourceName> PrefabsFor(RZ_ETribePart kind, RZ_ZombieSettings zombies)
	{
		switch (kind)
		{
			case RZ_ETribePart.ALPHA:
			{
				if (m_Settings.m_aAlphaPrefabs && !m_Settings.m_aAlphaPrefabs.IsEmpty())
					return m_Settings.m_aAlphaPrefabs;

				return zombies.m_aSpecialPrefabs;
			}
			case RZ_ETribePart.GUARD:
			{
				if (m_Settings.m_aGuardPrefabs && !m_Settings.m_aGuardPrefabs.IsEmpty())
					return m_Settings.m_aGuardPrefabs;

				return zombies.m_aSpecialPrefabs;
			}
			case RZ_ETribePart.SCOUT:
			{
				if (m_Settings.m_aScoutPrefabs && !m_Settings.m_aScoutPrefabs.IsEmpty())
					return m_Settings.m_aScoutPrefabs;

				break;
			}
			case RZ_ETribePart.YOUNG:
			{
				if (m_Settings.m_aYoungPrefabs && !m_Settings.m_aYoungPrefabs.IsEmpty())
					return m_Settings.m_aYoungPrefabs;

				break;
			}
		}

		if (m_Settings.m_aHunterPrefabs && !m_Settings.m_aHunterPrefabs.IsEmpty())
			return m_Settings.m_aHunterPrefabs;

		return zombies.m_aCommonPrefabs;
	}

	//------------------------------------------------------------------------------------------------
	//! The young's prefabs (a day party's first members).
	array<ResourceName> YoungPrefabs(RZ_ZombieSettings zombies)
	{
		return PrefabsFor(RZ_ETribePart.YOUNG, zombies);
	}

	//------------------------------------------------------------------------------------------------
	//! A part's prefabs with its tribe's look (docs/INFECTED.md, Looks): hunters, scouts and young
	//! mostly in its look's clothes, its soldiers in military ones. The alpha and guard as before.
	array<ResourceName> PrefabsForTribe(RZ_Tribe t, RZ_ETribePart kind, RZ_ZombieSettings zombies)
	{
		array<ResourceName> base = PrefabsFor(kind, zombies);
		if (!m_Settings.m_bLifeCycle || kind == RZ_ETribePart.ALPHA || kind == RZ_ETribePart.GUARD)
			return base;

		return m_Ways.MixFor(t, base);
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	RZ_TribesSave Save()
	{
		RZ_TribesSave data = new RZ_TribesSave();
		data.seeded = m_bSeeded;
		data.nextId = m_iNextId;
		data.nightOnline = m_fNightOnline;
		data.tribeSamples = m_fTribeSamples;
		data.allSamples = m_fAllSamples;
		data.countedNights = m_iCountedNights;
		data.life = m_Life.Save();
		data.people = m_People.Save();
		data.great = m_Great.Save();
		data.restPatrolNight = m_Rest.GetPatrolNight();
		m_Borders.Save(data.borders);
		foreach (RZ_Tribe r : m_Life.GetRovers())
		{
			data.rovers.Insert(SaveOne(r));
		}

		foreach (float f : m_aLastInfection)
		{
			data.lastInfection.Insert(f);
		}

		foreach (RZ_Tribe t : m_aTribes)
		{
			data.tribes.Insert(SaveOne(t));
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_TribeSave SaveOne(RZ_Tribe t)
	{
		t.CountRoles();
		RZ_TribeSave s = new RZ_TribeSave();
		s.id = t.m_iId;
		s.seed = t.m_iSeed;
		s.place = t.m_sPlace;
		s.lairX = t.m_vLair[0];
		s.lairZ = t.m_vLair[2];
		s.lairKind = t.m_sLairKind;
		s.radius = t.m_fRadius;
		s.temper = t.m_eTemper;
		s.pattern = t.m_sPattern;
		s.column = t.m_bColumn;
		s.hunger = t.m_fHunger;
		s.bank = t.m_fBank;
		s.pendingYoung = t.m_iPendingYoung;
		s.ranDay = t.m_iRanDay;
		s.ranToday = t.m_iRanToday;
		s.playerYoung = t.m_iPlayerYoung;
		foreach (string who : t.m_aTaken)
		{
			s.taken.Insert(who);
		}

		s.fed = t.m_bFed;
		s.alphaSamples = t.m_iAlphaSamples;
		s.founded = t.m_fFounded;
		s.hasAlpha = t.m_bHasAlpha;
		s.alphaWho = t.m_sAlphaWho;
		s.ledSince = t.m_fLedSince;
		s.alphaHealth = t.m_fAlphaHealth;
		s.alphaTaken = t.m_iAlphaTaken;
		s.guards = t.m_iGuards;
		s.hunters = t.m_iHunters;
		s.scouts = t.m_iScouts;
		s.young = t.m_iYoung;
		RZ_TribeWays.SaveTribe(t, s);
		RZ_TribePeople.SaveTribe(t, s);
		s.great = t.m_bGreat;
		foreach (int pi, vector pa : t.m_aPreyAt)
		{
			s.preyX.Insert(pa[0]);
			s.preyZ.Insert(pa[2]);
			s.preyHits.Insert(t.m_aPreyHits[pi]);
		}

		foreach (vector pg : t.m_aPreyGrounds)
		{
			s.preyGroundX.Insert(pg[0]);
			s.preyGroundZ.Insert(pg[2]);
		}

		s.takenAliveAt = t.m_fTakenAliveAt;
		s.voicedLeader = t.m_fVoicedLeader;
		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! From state.json; the tribes themselves are rebuilt once the land grid is ready (Seed). A save
	//! from before the life cycle reads with its defaults.
	void Restore(RZ_TribesSave data)
	{
		m_Restored = data;
		if (!data)
			return;

		m_iNextId = Math.Max(1, data.nextId);
		m_iCountedNights = data.countedNights;
		m_Life.Restore(data.life);
		m_People.Restore(data.people);
		m_Great.Restore(data.great);
		m_Rest.SetPatrolNight(data.restPatrolNight);
		m_Borders.Restore(data.borders);
		m_fNightOnline = data.nightOnline;
		m_fTribeSamples = data.tribeSamples;
		m_fAllSamples = data.allSamples;
		m_aLastInfection.Clear();
		if (data.lastInfection)
		{
			foreach (float f : data.lastInfection)
			{
				m_aLastInfection.Insert(f);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RestoreTribes()
	{
		if (!m_Restored.tribes)
			return;

		foreach (RZ_TribeSave s : m_Restored.tribes)
		{
			RZ_Tribe t = RestoreOne(s);
			BuildPlaces(t);
			BuildParts(t);
			m_aTribes.Insert(t);
		}

		if (!m_Restored.rovers)
			return;

		foreach (RZ_TribeSave rs : m_Restored.rovers)
		{
			RZ_Tribe r = RestoreOne(rs);
			if (!r.IsRover())
				continue;

			RZ_ETribePart kind = RZ_ETribePart.HUNTERS;
			int count = r.m_iHunters + r.m_iYoung;
			if (r.m_eKind == RZ_ETribeKind.LONE)
			{
				kind = RZ_ETribePart.ALPHA;
				count = 1;
			}

			if (count <= 0)
				continue;

			RZ_TribePart p = AddPart(r, kind, count, r.m_iYoung);
			p.m_vPos = r.m_vLair;
			p.m_vRest = r.m_vLair;
			m_Life.GetRovers().Insert(r);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Tribe RestoreOne(RZ_TribeSave s)
	{
		RZ_Tribe t = new RZ_Tribe();
		t.m_iId = s.id;
		t.m_iSeed = s.seed;
		t.m_sPlace = s.place;
		t.m_vLair = RZ_Util.GroundPos(Vector(s.lairX, 0, s.lairZ));
		t.m_sLairKind = s.lairKind;
		t.m_fRadius = s.radius;
		t.m_eTemper = Math.ClampInt(s.temper, 0, 4);
		t.m_sPattern = s.pattern;
		t.m_bColumn = s.column;
		t.m_fHunger = s.hunger;
		t.m_fBank = s.bank;
		t.m_iPendingYoung = s.pendingYoung;
		t.m_iRanDay = s.ranDay;
		t.m_iRanToday = s.ranToday;
		t.m_iPlayerYoung = s.playerYoung;
		if (s.taken)
		{
			foreach (string who : s.taken)
			{
				t.m_aTaken.Insert(who);
			}
		}

		t.m_bFed = s.fed;
		t.m_iAlphaSamples = s.alphaSamples;
		t.m_fFounded = s.founded;
		t.m_bHasAlpha = s.hasAlpha;
		t.m_sAlphaWho = s.alphaWho;
		t.m_fLedSince = s.ledSince;
		t.m_fAlphaHealth = Math.Clamp(s.alphaHealth, 0.05, 1);
		t.m_iAlphaTaken = s.alphaTaken;
		t.m_iGuards = s.guards;
		t.m_iHunters = s.hunters;
		t.m_iScouts = s.scouts;
		t.m_iYoung = s.young;
		if (t.m_iId >= m_iNextId)
			m_iNextId = t.m_iId + 1;

		RZ_TribeWays.RestoreTribe(t, s);
		RZ_TribePeople.RestoreTribe(t, s);
		t.m_bGreat = s.great && t.m_bHasAlpha;
		if (s.preyX && s.preyZ && s.preyHits)
		{
			for (int pi = 0; pi < s.preyX.Count() && pi < s.preyZ.Count() && pi < s.preyHits.Count(); pi++)
			{
				t.m_aPreyAt.Insert(Vector(s.preyX[pi], 0, s.preyZ[pi]));
				t.m_aPreyHits.Insert(s.preyHits[pi]);
				t.m_aPreyNight.Insert(-1);
			}
		}

		if (s.preyGroundX && s.preyGroundZ)
		{
			for (int gi = 0; gi < s.preyGroundX.Count() && gi < s.preyGroundZ.Count(); gi++)
			{
				t.m_aPreyGrounds.Insert(RZ_Util.GroundPos(Vector(s.preyGroundX[gi], 0, s.preyGroundZ[gi])));
			}
		}

		t.m_fTakenAliveAt = s.takenAliveAt;
		t.m_fVoicedLeader = s.voicedLeader;
		if (t.m_sAlphaName.IsEmpty() && t.m_bHasAlpha)
			m_Ways.NameAlpha(t);

		return t;
	}

	// ================================================================ Admin visibility
	//------------------------------------------------------------------------------------------------
	protected void LogTribes(string how)
	{
		RZ_Land land = m_GM.GetLand();
		RZ_Log.Info(string.Format("Tribes: %1 tribes %2 (%3 slots).", m_aTribes.Count(), how, land.Slots()));
		foreach (RZ_Tribe t : m_aTribes)
		{
			RZ_Log.Info("Tribes: " + Describe(t));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "1 the woods north of Levie: 13 (alpha Rising 1.00, guard 2, hunters 8, scouts 1, young 2), hunger 30 fed, bold, range 800 m, lair 053-112"
	string Describe(RZ_Tribe t)
	{
		t.CountRoles();
		string alpha = "no leader";
		if (t.m_bHasAlpha)
			alpha = string.Format("leader %1 %2", RZ_Tribe.StageName(t.Stage()), Math.Round(t.m_fAlphaHealth * 100) / 100);

		string text = string.Format("%1 %2: %3 (%4, guard %5, hunters %6, scouts %7, young %8)", t.m_iId, t.m_sPlace, t.Members(), alpha, t.m_iGuards, t.m_iHunters, t.m_iScouts, t.m_iYoung);
		text += string.Format(", hunger %1 %2, %3, range %4 m, lair %5", RZ_Util.RoundToInt(t.m_fHunger), RZ_Tribe.BandName(t.Band()), typename.EnumToString(RZ_ETemperament, t.m_eTemper), RZ_Util.RoundToInt(t.m_fRadius), RZ_SettlementScanner.GridReference(t.m_vLair));
		int dayGroups = 0;
		int dayBodies = 0;
		foreach (RZ_TribePart p : t.m_aParts)
		{
			if (p.m_eKind != RZ_ETribePart.DAY)
				continue;

			dayGroups++;
			if (p.HasBody())
				dayBodies += p.m_iCount;
		}

		string hunting = "no";
		if (t.m_Hunt)
			hunting = "player " + t.m_Hunt.m_iTarget.ToString();
		else if (t.m_bHunted)
			hunting = "on its trails";

		text += string.Format(", hunting %1, day party %2 groups (%3 bodies), fed %4, vigil %5, samples %6/3, pending young %7", hunting, dayGroups, dayBodies, t.m_bFed, t.m_bVigil, t.m_iAlphaSamples, t.m_iPendingYoung);
		if (!m_Settings.m_bLifeCycle)
			return text;

		text += string.Format(" | %1 (%2), %3, look %4 (%5%% soldiers), disturb %6, restless %7, clan %8, weak days %9",
			t.m_sAlphaName, RZ_TribeWays.AlphaKindName(t.m_eAlphaKind), RZ_Tribe.StageName(t.Stage()), typename.EnumToString(RZ_ELook, t.m_eLook), RZ_Util.RoundToInt(t.m_fMilitaryShare * 100), RZ_Util.RoundToInt(t.m_fDisturb), t.m_bRestless, t.m_iClan, t.IsWeakDays());
		text += string.Format(", fear %1, habits %2, marks %3, bonds %4, drift %5 m, history: %6", t.m_aFear.Count(), t.m_aHabits.Count(), t.m_aMarks.Count(), t.m_iBonds, RZ_Util.RoundToInt(t.m_vDrift.Length()), RZ_TribeWays.HistoryText(t));
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! One line per tribe for the status log.
	void StatusLines()
	{
		if (!IsEnabled())
			return;

		foreach (RZ_Tribe t : m_aTribes)
		{
			RZ_Log.Info("Status | tribe " + Describe(t));
		}

		RZ_Log.Info(string.Format("Status | tribes: phase %1, counted nights %2, town specials x%3 | %4", typename.EnumToString(RZ_ETribePhase, m_ePhase), m_iCountedNights, Math.Round(TownSpecialMult() * 100) / 100, m_Bodies.StatusText()));
		float ph = Math.Max(0.01, m_fPlayerHours);
		RZ_Log.Info(string.Format("Status | samples a player-hour: packs %1 (guards %2), over %3 player-hours", Math.Round(m_fPackSamples / ph * 100) / 100, Math.Round(m_fGuardSamples / ph * 100) / 100, Math.Round(m_fPlayerHours * 10) / 10));
		if (m_Settings.m_bLifeCycle)
			RZ_Log.Info(string.Format("Status | tribes' life: %1 | %2", m_Life.StatusText(), m_Borders.StatusText()));

		if (m_People.IsOn())
			RZ_Log.Info("Status | " + m_People.StatusText());

		RZ_Log.Info(string.Format("Status | the great one: %1 | %2", m_Great.StatusText(), m_Props.StatusText()));
		RZ_Log.Info("Status | tribes' " + m_Rest.StatusText());
	}

	//------------------------------------------------------------------------------------------------
	//! /tribes from an admin: no word dumps everything; "stir" stirs the nearest town (+60), "site"
	//! leaves a kill site at the admin's feet, "hunt" sends the nearest tribe's hunt for the admin
	//! (any time of day), "call" makes every tribe call now.
	void Command(int playerId, string text)
	{
		if (!RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Not allowed", "That command is for server admins.");
			return;
		}

		string word = text;
		word.ToLower();
		word = word.Trim();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (me && crawlers && crawlers.IsEnabled() && word == "stir")
		{
			int idx = crawlers.AddStir(me.GetOrigin(), 60);
			RZ_Settlement s = m_GM.GetSettlement(idx);
			if (s)
				m_GM.NotifyPlayer(playerId, "Infected", string.Format("%1 is stirred (%2).", s.m_sName, RZ_Util.RoundToInt(s.m_fStir)));
			else
				m_GM.NotifyPlayer(playerId, "Infected", "No town within 1 km of you to stir.");

			return;
		}

		if (me && crawlers && crawlers.IsEnabled() && word == "site")
		{
			crawlers.AddKillSite(me.GetOrigin(), "a test by an admin");
			m_GM.NotifyPlayer(playerId, "Infected", "A kill site where you stand. The nearest town's slow ones come out to it at nightfall.");
			return;
		}

		if (me && word == "hunt")
		{
			RZ_Tribe t = NearestTribe(me.GetOrigin(), 5000);
			if (t && m_Bodies.StartHunt(t, playerId, true))
				m_GM.NotifyPlayer(playerId, "Infected", string.Format("Something from %1 is coming for you.", t.m_sPlace));
			else
				m_GM.NotifyPlayer(playerId, "Infected", "Nothing within 5 km can come for you now.");

			return;
		}

		if (word == "call")
		{
			array<int> ids = {};
			array<IEntity> players = {};
			RZ_Util.GetAlivePlayers(ids, players);
			m_mCalledTo.Clear();
			foreach (RZ_Tribe ct : m_aTribes)
			{
				Call(ct, players, ids);
			}

			return;
		}

		Dump(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! /tribes (admins): everything the infected are doing, written to the server log, and a short
	//! notice to the admin. The status log prints the summary every few minutes anyway, so nothing
	//! here needs typing (docs/UI.md, Controllers).
	void Dump(int playerId)
	{
		if (playerId > 0 && !RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Not allowed", "That command is for server admins.");
			return;
		}

		RZ_Log.Info("Tribes dump | " + m_GM.GetSpawner().StatusText() + " | " + ActiveAIText() + " | " + LandText());
		RZ_Land land = m_GM.GetLand();
		if (IsEnabled() && land && land.IsReady())
			land.LogSlots();

		RZ_Crawlers crawlers = m_GM.GetSpawner().GetCrawlers();
		if (crawlers)
			crawlers.Dump();

		m_GM.GetSpawner().DumpTracked();
		m_GM.GetBiteDetector().DumpHits();
		foreach (RZ_Tribe rv : m_Life.GetRovers())
		{
			RZ_Log.Info(string.Format("Tribes dump | rover %1 %2 from %3: %4 members, until season day %5", rv.m_iId, typename.EnumToString(RZ_ETribeKind, rv.m_eKind), rv.m_sPlace, rv.Members(), Math.Round(rv.m_fRoverUntil * 10) / 10));
		}

		RZ_Log.Info("Tribes dump | " + m_Life.StatusText() + " | " + m_Borders.StatusText());
		RZ_Log.Info("Tribes dump | " + m_People.StatusText());
		RZ_Log.Info(string.Format("Tribes dump | the great one: %1 | %2", m_Great.StatusText(), m_Props.StatusText()));
		foreach (string beat : m_People.GetBeats())
		{
			RZ_Log.Info("Tribes dump | for the epilogue: " + beat);
		}
		foreach (RZ_Tribe t : m_aTribes)
		{
			RZ_Log.Info("Tribes dump | tribe " + Describe(t));
			foreach (RZ_TribePart p : t.m_aParts)
			{
				RZ_Log.Info(string.Format("Tribes dump |   %1 %2 out %3 home %4 target %5 path %6/%7", typename.EnumToString(RZ_ETribePart, p.m_eKind), p.Describe(), p.m_bOut, p.m_bHome, p.m_iTarget, p.m_iStep, p.m_aPath.Count()));
			}
		}

		if (RZ_Behaviour.Get())
			RZ_Log.Info("Tribes dump | " + RZ_Behaviour.Get().StatusText());

		if (playerId > 0)
			m_GM.NotifyPlayer(playerId, "Infected", "Written to the server log: towns, packs, their groups, every tracked unit and the budget.");
	}
}
