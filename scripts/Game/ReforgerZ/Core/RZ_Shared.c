//! The shared engine (docs/DESIGN.md, "The shared engine"): the pieces every later system uses,
//! built once and owned here, so no system keeps its own copy.
//!
//! - Notice lanes and the interruption budget (UI/RZ_NoticeLanes.c)
//! - The callback queue (Core/RZ_Callbacks.c)
//! - The marks spawner (World/RZ_Marks.c)
//! - The season budget and its lanes, and the set-piece calendar (Director/RZ_MajorBudget.c)
//! - The season scope of the deeds (Core/RZ_Deeds.c)
//! - Work sites: steps done with your hands at a visible thing (Missions/Blocks/RZ_WorkSites.c)
//! Elsewhere, and also shared: the checks and ways (Missions/RZ_Checks.c, RZ_Ways.c), world memory
//! (in the world facts, World/RZ_WorldMemory.c), the clocks (Core/RZ_Clocks.c, RZ_GameTime), and
//! the standing calls with flags and Forgive() on RZ_GameModeComponent.
//!
//! The one body budget (a mission's people, wanderers, and later scene people and askers, errand
//! scenes, strangers, epilogue scenes and post speakers) is the infected's agent budget
//! (Zombies/RZ_InfectedBodies.c, RZ_BodyBudget). Every system that spawns such a body asks
//! CanSpawnBody() below first and enters it with AddBody().
//! Server only.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Shared.
[BaseContainerProps()]
class RZ_SharedSettings
{
	// Notice lanes and the interruption budget
	[Attribute("1", desc: "Notice lanes: a ceiling on what speaks to a player unasked. Contacts (leaders' pings, the Long Table's host on the road, and later radio offers, wanderers and errands) at most one per Contact gap; barks one a minute and one per speaker per 10 minutes; approach lines once per place per session. Critical notices and talk are never held. The budget only stops: it never starts anything (docs/DESIGN.md, Notice lanes)")]
	bool m_bLanes;

	[Attribute("4", UIWidgets.Slider, "Contact gap: real minutes between unasked contacts for one player", "0 30 0.5")]
	float m_fContactGapMinutes;

	[Attribute("60", UIWidgets.Slider, "Bark gap: real seconds between barks for one player", "0 600 5")]
	float m_fBarkGapSeconds;

	[Attribute("10", UIWidgets.Slider, "Bark speaker gap: real minutes before the same speaker barks at the same player again", "0 60 1")]
	float m_fBarkSpeakerMinutes;

	// The callback queue
	[Attribute("1", desc: "Callbacks: a line that comes back later (2-6 hours of that character's play), near its subject, words only; at death a waiting one becomes an unsigned note on the nearest safehouse board (docs/DESIGN.md, Callbacks)")]
	bool m_bCallbacks;

	[Attribute("10", UIWidgets.Slider, "Callbacks waiting per player", "1 30 1")]
	int m_iCallbacksPerPlayer;

	[Attribute("100", UIWidgets.Slider, "Callbacks waiting server-wide that belong to nobody (the system pool)", "0 300 10")]
	int m_iCallbackPool;

	[Attribute("1000", UIWidgets.Slider, "Callback radius (m): the character must be this close to its subject", "100 3000 50")]
	float m_fCallbackRadius;

	[Attribute("1", UIWidgets.Slider, "Hours of a character's play between two callbacks", "0 6 0.25")]
	float m_fCallbackGapHours;

	[Attribute("3", UIWidgets.Slider, "Season days a waiting callback lasts", "0.5 10 0.5")]
	float m_fCallbackDays;

	[Attribute("24", UIWidgets.Slider, "Real hours a waiting callback lasts in the epilogue", "1 168 1")]
	float m_fEpilogueCallbackHours;

	// The marks spawner
	[Attribute("1", desc: "Marks: one spawner for every prop that marks the world (season marks, epilogue props, signs, mission props). Spawned only within the spawn radius of a player, never saved; with no prefab or no room, a mark is text only (docs/DESIGN.md, Marks)")]
	bool m_bMarks;

	[Attribute("150", UIWidgets.Slider, "Marks spawn within this many metres of a player", "50 400 10")]
	float m_fMarkSpawnRadius;

	[Attribute("200", UIWidgets.Slider, "...and are removed beyond this many metres (after Mark minutes up)", "60 600 10")]
	float m_fMarkDespawnRadius;

	[Attribute("10", UIWidgets.Slider, "Real minutes a spawned mark stays up at least", "0 60 1")]
	float m_fMarkMinutes;

	[Attribute("40", UIWidgets.Slider, "Season marks at once (the town log's marks, memorials)", "0 100 1")]
	int m_iSeasonMarks;

	[Attribute("100", UIWidgets.Slider, "Epilogue marks at once", "0 200 5")]
	int m_iEpilogueMarks;

	[Attribute("30", UIWidgets.Slider, "Prop marks at once (missions' props and chalk, the ambient notice prop, the Long Table's tells)", "0 100 1")]
	int m_iPropMarks;

	[Attribute("30", UIWidgets.Slider, "Sign marks at once (the tribes' signs, their own pool)", "0 100 1")]
	int m_iSignMarks;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Season mark prop (optional): the small memorial the town log's marks use (a cross, a cairn, flowers on a post). Empty = the marks are text only, said to whoever passes (docs/DESIGN.md, World memory)", "et")]
	ResourceName m_sSeasonMarkPrefab;

	[Attribute("6", UIWidgets.Slider, "On the verge: least metres off a road's centre line", "2 20 0.5")]
	float m_fVergeMin;

	[Attribute("10", UIWidgets.Slider, "On the verge: most metres off a road's centre line", "2 30 0.5")]
	float m_fVergeMax;

	// The season budget and its lanes
	[Attribute("20", UIWidgets.Slider, "Season budget, majors lane: morale per faction a season (the epilogue's 5 is inside it)", "0 100 1")]
	int m_iMajorMorale;

	[Attribute("8", UIWidgets.Slider, "Season budget, majors lane: strength per faction a season", "0 50 1")]
	int m_iMajorStrength;

	[Attribute("15", UIWidgets.Slider, "Season budget, majors lane: morale per major a season", "0 100 1")]
	int m_iPerMajorMorale;

	[Attribute("5", UIWidgets.Slider, "Season budget, majors lane: strength per major a season", "0 50 1")]
	int m_iPerMajorStrength;

	[Attribute("10", UIWidgets.Slider, "Season budget, majors lane: morale in one call", "0 50 1")]
	int m_iPerCallMorale;

	[Attribute("5", UIWidgets.Slider, "Season budget, majors lane: strength in one call", "0 50 1")]
	int m_iPerCallStrength;

	[Attribute("10", UIWidgets.Slider, "Season budget, contracts and errands lane: morale per faction a season day", "0 50 1")]
	int m_iContractMorale;

	[Attribute("4", UIWidgets.Slider, "Season budget, contracts and errands lane: strength per faction a season day", "0 20 1")]
	int m_iContractStrength;

	[Attribute("30", UIWidgets.Slider, "Season budget: contracts never take a leader's loyalty below this", "0 100 1")]
	int m_iContractLoyaltyFloor;

	[Attribute("10", UIWidgets.Slider, "Season budget, finale lane: morale per faction", "0 50 1")]
	int m_iFinaleMorale;

	[Attribute("5", UIWidgets.Slider, "Season budget, finale lane: strength per faction", "0 20 1")]
	int m_iFinaleStrength;

	[Attribute("5", UIWidgets.Slider, "Season budget, epilogue lane: morale per faction (inside the majors lane)", "0 20 1")]
	int m_iEpilogueMorale;

	// The set-piece calendar
	[Attribute("600", UIWidgets.Slider, "Set-piece calendar: errands deal nothing within this many metres of a set piece on its evening", "0 2000 50")]
	float m_fSetPieceQuietRadius;

	[Attribute("2", UIWidgets.Slider, "Set-piece calendar: evenings a set piece waits for a free one before it runs at the next free hour", "0 5 1")]
	int m_iSetPieceWaits;

	// Work sites
	[Attribute(desc: "Work sites: a step you do with your hands is a thing at its place with one hold prompt, and the world shows the result (docs/DESIGN.md, Work site)")]
	ref RZ_WorkSiteSettings m_WorkSites;

	//------------------------------------------------------------------------------------------------
	RZ_WorkSiteSettings WorkSites()
	{
		if (!m_WorkSites)
			m_WorkSites = RZ_WorkSiteSettings.Cast(RZ_Util.NewWithDefaults("RZ_WorkSiteSettings"));

		return m_WorkSites;
	}

	protected static ref RZ_SharedSettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	//! The game mode's settings, else the defaults (always non-null).
	static RZ_SharedSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetSharedSettings())
			return gm.GetSharedSettings();

		if (!s_Defaults)
			s_Defaults = RZ_SharedSettings.Cast(RZ_Util.NewWithDefaults("RZ_SharedSettings"));

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
//! The shared engine's saved part (state.json, wiped with the season).
class RZ_SharedSave
{
	ref RZ_CallbackSave callbacks;
	ref RZ_MajorBudgetSave budget;
	ref RZ_CalendarSave calendar;
	ref RZ_SeasonDeedsSave deeds;
}

//------------------------------------------------------------------------------------------------
class RZ_Shared
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_SharedSettings m_Settings;
	protected ref RZ_NoticeLanes m_Lanes;
	protected ref RZ_Callbacks m_Callbacks;
	protected ref RZ_Marks m_Marks;
	protected ref RZ_MajorBudget m_Budget;
	protected ref RZ_SetPieceCalendar m_Calendar;
	protected ref RZ_SeasonDeeds m_SeasonDeeds = new RZ_SeasonDeeds();
	protected ref RZ_WorkSites m_WorkSites;

	//------------------------------------------------------------------------------------------------
	void RZ_Shared(RZ_GameModeComponent gm, RZ_SharedSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Lanes = new RZ_NoticeLanes(settings);
		m_Callbacks = new RZ_Callbacks(gm, settings);
		m_Marks = new RZ_Marks(settings);
		m_Budget = new RZ_MajorBudget(settings);
		m_Calendar = new RZ_SetPieceCalendar(settings);
		m_WorkSites = new RZ_WorkSites(settings);
	}

	//------------------------------------------------------------------------------------------------
	RZ_NoticeLanes GetLanes() { return m_Lanes; }
	RZ_Callbacks GetCallbacks() { return m_Callbacks; }
	RZ_Marks GetMarks() { return m_Marks; }
	RZ_MajorBudget GetBudget() { return m_Budget; }
	RZ_SetPieceCalendar GetCalendar() { return m_Calendar; }
	RZ_SeasonDeeds GetSeasonDeeds() { return m_SeasonDeeds; }
	RZ_WorkSites GetWorkSites() { return m_WorkSites; }

	//------------------------------------------------------------------------------------------------
	//! The shared engine from anywhere on the server, null without a game mode.
	static RZ_Shared Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return null;

		return gm.GetShared();
	}

	//------------------------------------------------------------------------------------------------
	//! The one body budget (docs/INFECTED.md, "The one body budget"): whether a body of this kind may
	//! spawn at pos now. kind: RZ_BodyBudget.STORY, WANDERER, SCENE, ASKER, ERRAND, STRANGER,
	//! EPILOGUE, SPEAKER. True without a spawner.
	bool CanSpawnBody(string kind, vector pos)
	{
		RZ_BodyBudget bodies = GetBodies();
		if (!bodies)
			return true;

		return bodies.CanSpawn(kind, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Enter a body that was just spawned into the one body budget, so it counts until it's gone.
	void AddBody(string kind, IEntity e)
	{
		RZ_BodyBudget bodies = GetBodies();
		if (bodies)
			bodies.Add(kind, e);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_BodyBudget GetBodies()
	{
		if (!m_GM || !m_GM.GetSpawner())
			return null;

		return m_GM.GetSpawner().GetBodies();
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_Callbacks.Tick(dt, ids, players);
		m_Marks.Tick(dt, players);
		m_Budget.Tick();
		m_Calendar.Tick(dt, !ids.IsEmpty());
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: who is working at which work site, and their props near players.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		m_WorkSites.Tick(1, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A new life starts for this player (before the profile starts it): the old character's
	//! waiting callbacks become board notes.
	void OnNewLife(int playerId, RZ_PlayerProfile profile)
	{
		if (profile)
			m_Callbacks.OnLifeEnded(profile.m_sIdentity, profile.m_iLives);
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_Lanes.Forget(playerId);
	}

	//------------------------------------------------------------------------------------------------
	RZ_SharedSave Save()
	{
		RZ_SharedSave data = new RZ_SharedSave();
		data.callbacks = m_Callbacks.Save();
		data.budget = m_Budget.Save();
		data.calendar = m_Calendar.Save();
		data.deeds = m_SeasonDeeds.Save();
		return data;
	}

	//------------------------------------------------------------------------------------------------
	//! Older saves have none: everything starts empty.
	void Restore(RZ_SharedSave data)
	{
		if (!data)
			return;

		m_Callbacks.Restore(data.callbacks);
		m_Budget.Restore(data.budget);
		m_Calendar.Restore(data.calendar);
		m_SeasonDeeds.Restore(data.deeds);
		RZ_Log.Info(string.Format("Shared: %1 callbacks waiting, %2 season records.", m_Callbacks.Count(), m_SeasonDeeds.Count()));
	}
}
