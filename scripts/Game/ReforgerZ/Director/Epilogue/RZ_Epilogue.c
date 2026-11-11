//! The epilogue week's controller (docs/EPILOGUE.md): after the cure the world stays open for
//! Season > Epilogue hours (168 by default) and tells the season. Saved with the season.
//!
//! - At the cure: the hall of fame freezes, the record's snapshot is taken from the owning systems,
//!   the beats are chosen (RZ_EpiBeats), the closing work is listed (RZ_EpiClosing), and everyone
//!   online gets the cure line and their own first line. Players offline get theirs at their next
//!   spawn.
//! - Three phases on real time (each a share of the epilogue's length, x Epilogue > Test scale):
//!   A the news (to the first dusk with a player online, 1 hour to L/28), B the days after (the
//!   evolving step at a third and two thirds), C processing (the last L/7, never more than Season >
//!   Warn hours; earlier on an empty server).
//! - The carriers ask it for their lines: leaders (the talk screen's "About the season", "Where
//!   should I look?" and the stronghold grey choices, or RZ_LeaderAction without the screen),
//!   residents, wanderers, the open channels and the reel, boards, readables, the news, /season.
//! - The dose outlets (RZ_EpiOutlets), the places and scenes (RZ_EpiSpots).
//! - No cure: the Unsigned last day (IsLastDay) builds the record without a route and nothing else.
//! Nothing physical is saved: marks, scenes and props come back when a player is near.
class RZ_Epilogue
{
	static const int TAG = -16;		// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id: an epilogue place

	protected static const int HOUR = 3600;
	protected static const float CHECK_SECONDS = 10;
	protected static const int NAMED_KEEP = 40;

	protected static RZ_Epilogue s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_EpilogueSettings m_Settings;
	protected ref RZ_EpilogueSave m_S = new RZ_EpilogueSave();
	protected ref RZ_EpiBeats m_Beats;
	protected ref RZ_EpiOutlets m_Outlets;
	protected ref RZ_EpiSpots m_Spots;
	protected ref RZ_EpiClosing m_Closing;

	protected int m_iBuildStep = -1;		// ticks of the build after the cure (-1: none)
	protected float m_fCheck;
	protected int m_iLastStep = -1;
	protected bool m_bPlaced;				// the marks went up this run (they aren't saved)
	protected ref array<int> m_aNewsTold = {};	// players who had the week's news this session
	protected ref array<int> m_aWalking = {};	// players walking out (their "death" isn't one)
	protected float m_fReel;
	protected int m_iReelNext;
	protected ref array<string> m_aReelHeard = {};	// "player|entry" this session
	protected ref map<int, int> m_mReelCount = new map<int, int>();

	//------------------------------------------------------------------------------------------------
	void RZ_Epilogue(RZ_GameModeComponent gm, RZ_EpilogueSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		s_Instance = this;
		m_Beats = new RZ_EpiBeats(gm, this);
		m_Outlets = new RZ_EpiOutlets(gm, this);
		m_Spots = new RZ_EpiSpots(gm, this);
		m_Closing = new RZ_EpiClosing(gm, this);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Epilogue Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOn()
	{
		return s_Instance && s_Instance.m_Settings && s_Instance.m_Settings.m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	RZ_EpilogueSettings GetSettings() { return m_Settings; }
	RZ_EpilogueSave Data() { return m_S; }
	RZ_EpiBeats GetBeats() { return m_Beats; }
	RZ_EpiOutlets GetOutlets() { return m_Outlets; }
	RZ_EpiSpots GetSpots() { return m_Spots; }
	RZ_EpiClosing GetClosing() { return m_Closing; }
	bool IsBuilt() { return IsOn() && m_S.built; }
	int Route() { return m_S.route - 1; }
	int Terms() { return m_S.terms; }
	int Faction() { return m_S.faction; }

	// ================================================================ Clocks
	//------------------------------------------------------------------------------------------------
	//! Epilogue > Test scale, kept sane.
	float Scale()
	{
		if (!m_Settings)
			return 1;

		return Math.Clamp(m_Settings.m_fTestScale, 0.001, 10);
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue's length L in real hours, scaled.
	float Hours()
	{
		RZ_Seasons seasons = m_GM.GetSeasons();
		if (!seasons)
			return 168 * Scale();

		return seasons.EpilogueHours();
	}

	//------------------------------------------------------------------------------------------------
	int Now()
	{
		return RZ_SeasonClock.Now();
	}

	//------------------------------------------------------------------------------------------------
	//! When processing begins (unix), 0 if never (Wipe Off and nobody started it).
	int ProcessingAt()
	{
		if (m_S.cStartUnix > 0)
			return m_S.cStartUnix;

		RZ_Seasons seasons = m_GM.GetSeasons();
		if (!seasons)
			return 0;

		int at = seasons.WipeAt();
		if (at <= 0)
			return 0;

		float len = Math.Min(Hours() / 7, seasons.WarnHours() * Scale());
		return at - RZ_Util.RoundToInt(len * HOUR);
	}

	//------------------------------------------------------------------------------------------------
	//! 0 none, 1 the news, 2 the days after, 3 processing.
	int Phase()
	{
		if (!IsBuilt() || m_S.lastDay)
			return 0;

		if (m_S.cForced || m_S.cStartUnix > 0)
			return 3;

		int c = ProcessingAt();
		if (c > 0 && Now() >= c)
			return 3;

		if (m_S.aEndUnix <= 0)
			return 1;

		return 2;
	}

	//------------------------------------------------------------------------------------------------
	//! The evolving step: 0 (phase A and the start of B), 1 a third into B, 2 two thirds, 3 processing.
	int Step()
	{
		int phase = Phase();
		if (phase >= 3)
			return 3;

		if (phase <= 1 || m_S.aEndUnix <= 0)
			return 0;

		int end = ProcessingAt();
		if (end <= 0)
			end = m_S.commitUnix + RZ_Util.RoundToInt(Hours() * 6 / 7 * HOUR);

		float len = Math.Max(60, end - m_S.aEndUnix);
		float frac = (Now() - m_S.aEndUnix) / len;
		if (frac >= 0.667)
			return 2;

		if (frac >= 0.333)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Real hours of room before processing, times the share of hours with someone online (the
	//! majors' reading of the last 7 real days).
	float RoomHours()
	{
		int c = ProcessingAt();
		if (c <= 0)
			return 9999;

		float left = Math.Max(0, c - Now()) / 3600.0;
		float share = 0.33;
		RZ_Majors majors = RZ_Majors.Get();
		if (majors)
			share = Math.Clamp(majors.HoursPerDay() / 24, 0.05, 1);

		return left * share;
	}

	//------------------------------------------------------------------------------------------------
	//! In the epilogue with the world settling: holdings don't change hands (RZ_WorldFacts).
	bool HoldingsFrozen()
	{
		return IsBuilt() && !m_S.lastDay && m_Settings.m_bWorldSettles;
	}

	//------------------------------------------------------------------------------------------------
	//! The closing work runs: on, and an epilogue of 24 hours or more (before Test scale).
	bool ClosingOn()
	{
		if (!IsBuilt() || m_S.lastDay || !m_Settings.m_bClosingWork)
			return false;

		return Hours() / Scale() >= 24;
	}

	//------------------------------------------------------------------------------------------------
	bool IsStruck(string name)
	{
		return !name.IsEmpty() && m_S.struck.Contains(name);
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_EpilogueSave saved)
	{
		if (!saved)
			return;

		m_S = saved;
		while (m_S.choices.Count() < 4)
		{
			m_S.choices.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_EpilogueSave Save()
	{
		return m_S;
	}

	// ================================================================ The cure and the build
	//------------------------------------------------------------------------------------------------
	//! The cure is out (RZ_GameModeComponent.OnCureCompleted): the build runs over the next ticks.
	void OnCure()
	{
		if (!IsOn())
			return;

		m_S.cureUnix = Now();
		m_S.lastDay = false;
		m_S.built = false;
		m_iBuildStep = 0;
		RZ_Log.Info("Epilogue: the cure is out; the record is taken on the next ticks.");
	}

	//------------------------------------------------------------------------------------------------
	//! A tick of the build: 1 the snapshot and the hall's freeze, 2 the beats, 3 the closing work and
	//! the commit.
	protected void BuildTick()
	{
		m_iBuildStep++;
		if (m_iBuildStep == 1)
		{
			Snapshot();
			FreezeHall();
			return;
		}

		if (m_iBuildStep == 2)
		{
			m_S.beats.Clear();
			m_Beats.Build();
			return;
		}

		m_iBuildStep = -1;
		m_Closing.List();
		Commit();
	}

	//------------------------------------------------------------------------------------------------
	//! The record's snapshot, from every owning system in one tick (docs/EPILOGUE.md).
	protected void Snapshot()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		m_S.cureDay = facts.Day();
		m_S.route = 0;
		m_S.how = RZ_CureFinale.HOW_HANDED;
		m_S.terms = 0;
		m_S.condition = 0;
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && RZ_CureFinale.IsOn() && !m_S.lastDay)
		{
			m_S.route = RZ_CureFinale.RouteFlag(finale.Controller());
			m_S.how = Math.MaxInt(1, finale.How());
			m_S.terms = finale.Terms();
			m_S.condition = finale.Condition();
		}

		m_S.slow = facts.Flag("cure.slow") != 0;
		m_S.faction = m_GM.GetSeasonWinner();
		m_S.fameLeader = m_GM.GetSeasonLeader();
		if (m_S.fameLeader >= 0)
			m_S.fameLeaderFame = m_GM.GetSeasonFame(m_S.fameLeader);

		// Nobody controls: the Fame leader's faction, under the terms its stronghold leader picks
		if (m_S.route == 0 && m_S.faction < 0)
			m_S.faction = m_S.fameLeader;

		if (m_S.lastDay)
			m_S.faction = -1;

		m_S.deciders.Clear();
		RZ_SeasonRecord rec = RZ_Persistence.LoadSeason();
		if (rec && rec.season == m_GM.GetSeasons().GetSeason() && rec.deciders)
		{
			foreach (string d : rec.deciders)
			{
				m_S.deciders.Insert(d);
			}
		}

		m_S.standNames.Clear();
		RZ_HallOfFame hall = m_GM.GetHall();
		if (hall)
			hall.GetStandNames(m_S.standNames);

		m_S.majorLines.Clear();
		if (RZ_Majors.Get())
		{
			foreach (RZ_StoryRecord story : RZ_Majors.Get().GetStories())
			{
				if (!story.line.IsEmpty())
					m_S.majorLines.Insert(story.line);
			}
		}

		// Safehouses, holdings, towns with crawlers, packs still calling
		m_S.housePop.Clear();
		m_S.houseTier.Clear();
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int h = 0; h < houses.Count(); h++)
			{
				RZ_Safehouse house = houses.Get(h);
				m_S.housePop.Insert(house.m_iPopulation);
				m_S.houseTier.Insert(house.GetTier());
			}
		}

		m_S.holdIds.Clear();
		m_S.holdOwner.Clear();
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding hold : territory.GetHoldings())
			{
				m_S.holdIds.Insert(hold.m_iId);
				m_S.holdOwner.Insert(hold.m_eFaction);
			}
		}

		m_S.townIdx.Clear();
		m_S.townCrawlers.Clear();
		m_S.townTaken.Clear();
		m_S.townBy.Clear();
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			int n = TownCrawlers(idx);
			if (n < 6)
				continue;

			m_S.townIdx.Insert(idx);
			m_S.townCrawlers.Insert(n);
			m_S.townTaken.Insert(0);
			m_S.townBy.Insert("");
		}

		m_S.packIds.Clear();
		m_S.packPlaces.Clear();
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes && tribes.IsEnabled())
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				if (!t.m_bHasAlpha)
					continue;

				m_S.packIds.Insert(t.m_iId);
				m_S.packPlaces.Insert(t.m_sPlace);
			}
		}

		// Players: names frozen now; the wanted at the cure (Kill on Sight)
		m_S.playerIdents.Clear();
		m_S.playerNames.Clear();
		m_S.playerLines.Clear();
		m_S.wantIdents.Clear();
		m_S.wantNames.Clear();
		m_S.wantFaction.Clear();
		m_S.wantState.Clear();
		array<RZ_PlayerProfile> profiles = {};
		m_GM.GetAllProfiles(profiles);
		foreach (RZ_PlayerProfile p : profiles)
		{
			if (p.m_sIdentity.IsEmpty())
				continue;

			string name = KnownName(p.m_sIdentity);
			m_S.playerIdents.Insert(p.m_sIdentity);
			m_S.playerNames.Insert(name);
			m_S.playerLines.Insert(BuildLines(p, name));
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (p.GetTier(f) != RZ_ETier.KILL_ON_SIGHT || m_S.wantIdents.Count() >= 8)
					continue;

				m_S.wantIdents.Insert(p.m_sIdentity);
				m_S.wantNames.Insert(name);
				m_S.wantFaction.Insert(f);
				m_S.wantState.Insert(0);
			}
		}

		RZ_Log.Info(string.Format("Epilogue: the record: route %1, how %2, terms %3, faction %4, %5 players, %6 towns with crawlers, %7 packs calling.",
			m_S.route, m_S.how, m_S.terms, m_S.faction, m_S.playerIdents.Count(), m_S.townIdx.Count(), m_S.packIds.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! The name an identity plays under, if it's online now; else the last name the hall knows.
	string KnownName(string ident)
	{
		int pid = 0;
		if (m_GM.GetQuestlines())
			pid = m_GM.GetQuestlines().OnlineId(ident);

		if (pid > 0)
			return RZ_Util.PlayerName(pid);

		RZ_HallOfFame hall = m_GM.GetHall();
		if (hall)
			return hall.NameOf(ident);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A town's crawlers now (the town's count before the ledger, less the ledger); with crawlers
	//! off, its infection read as a rough count.
	int TownCrawlers(int idx)
	{
		RZ_Settlement s = m_GM.GetSettlement(idx);
		if (!s)
			return 0;

		RZ_Crawlers crawlers;
		if (m_GM.GetSpawner())
			crawlers = m_GM.GetSpawner().GetCrawlers();

		if (crawlers && crawlers.IsEnabled())
			return RZ_Util.RoundToInt(Math.Max(0, crawlers.BaseAgents(s) - s.m_fLedger));

		return RZ_Util.RoundToInt(s.m_fInfection / 5);
	}

	//------------------------------------------------------------------------------------------------
	//! Your three lines (docs/EPILOGUE.md, And you), simplified: your route and whether you decided
	//! it, a major you decided, and the deeds that stand out (records, Champion, Wanted).
	protected string BuildLines(RZ_PlayerProfile p, string name)
	{
		array<string> lines = {};
		string ident = p.m_sIdentity;
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && RZ_CureFinale.IsOn() && !m_S.lastDay)
		{
			int r = finale.RouteOf(ident);
			bool decided = finale.GetDeciders().Contains(ident);
			if (decided)
				lines.Insert("Word is you were one of the people who chose how the cure goes out.");
			else if (r == RZ_CureFinale.INDEP)
				lines.Insert("Word is you were on the lab road, for nobody's side but everyone's.");
			else if (r >= 0 && r < RZ_CureFinale.INDEP)
				lines.Insert(string.Format("Word is you were on the lab road for %1.", RZ_Factions.GetNameInText(r)));
		}

		if (RZ_Majors.Get())
		{
			foreach (RZ_StoryRecord story : RZ_Majors.Get().GetStories())
			{
				if (story.deciders.Contains(ident) && lines.Count() < 3)
				{
					lines.Insert(string.Format("%1 remembers you: you had a hand in how its story ended.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(story.faction))));
					break;
				}
			}
		}

		for (int f = 0; f < RZ_Factions.COUNT && lines.Count() < 3; f++)
		{
			RZ_ETier tier = p.GetTier(f);
			if (tier == RZ_ETier.CHAMPION)
				lines.Insert(string.Format("%1 calls you one of its own.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(f))));
			else if (tier == RZ_ETier.KILL_ON_SIGHT)
				lines.Insert(string.Format("%1 has your face on a poster. The cure doesn't cover it.", RZ_Util.Capitalize(RZ_Factions.GetNameInText(f))));
		}

		if (lines.IsEmpty())
			lines.Insert("Word is you kept your head down. Most people did.");

		string text = "";
		foreach (int i, string l : lines)
		{
			if (i > 0)
				text += "\n";

			text += l;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The hall of fame freezes: records stop counting; the week adds only lines.
	protected void FreezeHall()
	{
		RZ_HallOfFame hall = m_GM.GetHall();
		if (hall)
			hall.Freeze();

		m_S.frozen = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The story commits: everyone online hears the cure line and their own first line.
	protected void Commit()
	{
		m_S.built = true;
		m_S.commitUnix = Now();
		m_S.aEndUnix = 0;
		m_S.cStartUnix = 0;
		m_S.cForced = false;
		m_S.step = 0;
		m_S.told.Clear();
		RZ_WorldFacts.Get().SetFlag("story.built", 1);
		m_Outlets.Setup();
		m_bPlaced = false;
		RZ_Log.Info(string.Format("Epilogue: story built (%1 beats, %2 players, %3 open items)", m_S.beats.Count(), m_S.playerIdents.Count(), m_Closing.OpenCount()));

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int playerId : ids)
		{
			TellFirst(playerId);
		}

		m_GM.SaveNow();
	}

	//------------------------------------------------------------------------------------------------
	//! The cure line, the player's own first line, one beat near them and where to look.
	protected void TellFirst(int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		if (ident.IsEmpty() || m_S.told.Contains(ident))
			return;

		m_S.told.Insert(ident);
		string text = RZ_EpiText.CureLine(Route(), m_S.how, m_S.terms, m_S.faction, m_S.slow);
		string own = LineOf(ident, 0);
		if (!own.IsEmpty())
			text += "\n" + own;

		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (ent)
		{
			string near = m_Beats.NearestEcho(ent.GetOrigin(), 3000);
			if (!near.IsEmpty())
				text += "\n" + near;

			string look = m_Beats.WhereToLook(playerId, ent.GetOrigin());
			if (!look.IsEmpty())
				text += "\n" + look;
		}

		m_GM.NotifyPlayer(playerId, "The cure", text);
	}

	//------------------------------------------------------------------------------------------------
	//! One of an identity's lines (0-2), "" if none. New faces get the season in a line.
	string LineOf(string ident, int which)
	{
		int i = m_S.playerIdents.Find(ident);
		if (i < 0)
		{
			if (which == 0)
				return "New face. You missed it. The queue's at the north road.";

			return "";
		}

		array<string> parts = {};
		m_S.playerLines[i].Split("\n", parts, true);
		if (which < 0 || which >= parts.Count())
			return "";

		return parts[which];
	}

	//------------------------------------------------------------------------------------------------
	//! No cure, and the wipe is under Warn hours away (RZ_Seasons.IsLastDay): the record without a
	//! route and its beats; no week, so no closing work, no outlets and no Walk Out.
	protected void BuildLastDay()
	{
		m_S.lastDay = true;
		Snapshot();
		m_S.beats.Clear();
		m_Beats.Build();
		m_S.built = true;
		m_S.commitUnix = Now();
		m_bPlaced = false;
		RZ_Log.Info(string.Format("Epilogue: the Unsigned last day (%1 beats).", m_S.beats.Count()));
		m_GM.NotifyAll(RZ_Broadcast.TITLE, "The perimeter will be maintained indefinitely.");
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		if (m_iBuildStep >= 0)
		{
			BuildTick();
			return;
		}

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		// A restart between the cure and the commit: build again from the live world
		if (!m_S.built && facts.IsCured() && m_S.cureUnix > 0)
		{
			m_iBuildStep = 0;
			return;
		}

		// No cure and the last day has come
		if (!m_S.built && !facts.IsCured() && m_GM.GetSeasons() && m_GM.GetSeasons().IsLastDay())
			BuildLastDay();

		// A cure on the last day: the record is built again, with a route
		if (m_S.built && m_S.lastDay && facts.IsCured())
		{
			OnCure();
			return;
		}

		if (!m_S.built)
			return;

		if (!m_bPlaced)
		{
			m_bPlaced = true;
			m_Beats.PlaceAll();
		}

		m_Spots.Tick(dt, ids, players);
		m_fCheck += dt;
		if (m_fCheck < CHECK_SECONDS * Math.Min(1, Scale() * 10))
			return;

		float step = m_fCheck;
		m_fCheck = 0;
		Phases(ids);
		if (m_S.lastDay)
			return;

		m_Outlets.Tick();
		Reel(step, ids, players);
		if (!ids.IsEmpty())
		{
			m_Closing.Tick(step, ids, players);
			foreach (int playerId : ids)
			{
				TellFirst(playerId);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Phase A ends at the first dusk with someone online (1 hour at least, L/28 at most); processing
	//! begins on its clock, or at the next login after an empty stretch; the evolving step moves.
	protected void Phases(array<int> ids)
	{
		int now = Now();
		bool online = !ids.IsEmpty();
		if (online)
		{
			// Nobody on for 2L/7 in phase B: processing starts at this login
			if (Phase() == 2 && m_Settings.m_bEmptyEndsEarly && m_S.lastOnlineUnix > 0 && now - m_S.lastOnlineUnix >= Hours() * 2 / 7 * HOUR)
			{
				m_S.cForced = true;
				RZ_Log.Info("Epilogue: the server was empty; processing starts now.");
			}

			m_S.lastOnlineUnix = now;
		}

		if (Phase() == 1)
		{
			int since = now - m_S.commitUnix;
			bool dusk = online && RZ_WorldFacts.Get().IsDusk() && since >= HOUR * Scale();
			if (dusk || since >= Hours() / 28 * HOUR)
			{
				m_S.aEndUnix = now;
				RZ_Log.Info("Epilogue: phase B");
				m_Beats.OnFirstDusk();
			}
		}

		if (Phase() == 3 && m_S.cStartUnix <= 0)
		{
			m_S.cStartUnix = now;
			RZ_Log.Info("Epilogue: phase C");
			m_Closing.OnProcessing();
			m_GM.NotifyAll(RZ_Broadcast.TITLE, "Processing at the north road begins. Residents should bring identification.");
		}

		int s = Step();
		if (s != m_iLastStep)
		{
			m_iLastStep = s;
			m_S.step = s;
			m_Beats.Refresh();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Admin and test: move to a phase now (1 A, 2 B, 3 C).
	void ForcePhase(int phase)
	{
		if (!IsBuilt())
			return;

		if (phase <= 1)
		{
			m_S.aEndUnix = 0;
			m_S.cForced = false;
			m_S.cStartUnix = 0;
		}
		else if (phase == 2)
		{
			if (m_S.aEndUnix <= 0)
				m_S.aEndUnix = Now();

			m_S.cForced = false;
			m_S.cStartUnix = 0;
			m_Beats.OnFirstDusk();
		}
		else
		{
			if (m_S.aEndUnix <= 0)
				m_S.aEndUnix = Now();

			m_S.cForced = true;
		}

		array<int> none = {};
		Phases(none);
	}

	// ================================================================ The reel
	//------------------------------------------------------------------------------------------------
	//! A broadcast from the radio station (RZ_RadioStation): before the cure each identity's latest
	//! is kept (12 at most, spread over the season's days); in processing the "Going home" preset
	//! replaces that identity's entry. Broadcasts in the week otherwise go out live, as built.
	void OnBroadcast(int playerId, string text)
	{
		if (!IsOn() || !m_Settings.m_bReel)
			return;

		bool cured = RZ_WorldFacts.Get().IsCured();
		if (cured && Phase() < 3)
			return;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		int day = RZ_WorldFacts.Get().Day();
		int i = m_S.reelIdents.Find(ident);
		if (i >= 0)
		{
			m_S.reelLines[i] = text;
			m_S.reelDays[i] = day;
			return;
		}

		if (m_S.reelIdents.Count() >= 12)
		{
			// Make room on the day with the most entries: one a day first
			int drop = 0;
			int most = 0;
			foreach (int k, int d : m_S.reelDays)
			{
				int same = 0;
				foreach (int d2 : m_S.reelDays)
				{
					if (d2 == d)
						same++;
				}

				if (same > most)
				{
					most = same;
					drop = k;
				}
			}

			m_S.reelIdents.Remove(drop);
			m_S.reelLines.Remove(drop);
			m_S.reelDays.Remove(drop);
		}

		m_S.reelIdents.Insert(ident);
		m_S.reelLines.Insert(text);
		m_S.reelDays.Insert(day);
	}

	//------------------------------------------------------------------------------------------------
	//! In the week: one reel entry every 20 minutes (x Test scale) on the open channels, to players
	//! with a radio; each entry once a player a session, three a session at most; the anonymous are
	//! skipped and the lines pass the note filter.
	protected void Reel(float step, array<int> ids, array<IEntity> players)
	{
		if (!m_Settings.m_bReel || m_S.reelLines.IsEmpty() || ids.IsEmpty())
			return;

		m_fReel += step;
		if (m_fReel < 1200 * Scale())
			return;

		m_fReel = 0;
		int n = m_S.reelLines.Count();
		int idx = -1;
		RZ_HallOfFame hall = m_GM.GetHall();
		for (int k = 0; k < n; k++)
		{
			int c = (m_iReelNext + k) % n;
			if (hall && hall.IsAnonymous(m_S.reelIdents[c]))
				continue;

			idx = c;
			break;
		}

		if (idx < 0)
			return;

		m_iReelNext = idx + 1;
		string line = RZ_EpiNames.Filtered(m_S.reelLines[idx]);
		foreach (int i, IEntity p : players)
		{
			int pid = ids[i];
			string heard = string.Format("%1|%2", pid, idx);
			int count = 0;
			m_mReelCount.Find(pid, count);
			if (count >= 3 || m_aReelHeard.Contains(heard) || !RZ_Radio.HasRadio(m_GM, pid, p))
				continue;

			m_aReelHeard.Insert(heard);
			m_mReelCount.Set(pid, count + 1);
			m_GM.NotifyPlayer(pid, "Radio: the reel", "Recorded before the cure: " + line);
		}
	}

	// ================================================================ Players
	//------------------------------------------------------------------------------------------------
	//! A spawn (RZ_GameModeComponent.HandlePlayerSpawned): the week's news once a session, the
	//! player's own first line if they missed the commit, and the background's last goal.
	void OnSpawn(int playerId, IEntity ent)
	{
		if (!IsBuilt() || !ent)
			return;

		if (m_S.lastDay)
			return;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		if (!m_S.told.Contains(ident))
		{
			TellFirst(playerId);
		}
		else if (!m_aNewsTold.Contains(playerId))
		{
			string text = RZ_EpiText.CureLine(Route(), m_S.how, m_S.terms, m_S.faction, m_S.slow);
			string near = m_Beats.NearestEcho(ent.GetOrigin(), 3000);
			if (!near.IsEmpty())
				text += "\n" + near;

			string look = m_Beats.WhereToLook(playerId, ent.GetOrigin());
			if (!look.IsEmpty())
				text += "\n" + look;

			m_GM.NotifyPlayer(playerId, "Word on the roads", text);
		}

		if (!m_aNewsTold.Contains(playerId))
			m_aNewsTold.Insert(playerId);

		m_Closing.OnSpawn(playerId);
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		m_aNewsTold.RemoveItem(playerId);
		m_mReelCount.Remove(playerId);
		m_aWalking.RemoveItem(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A player died (RZ_FactionPerks.OnPlayerDied): their name comes off the walk list, a wanted
	//! poster changes, a notable life leaves a name to bring home. Never for a walk-out.
	void OnPlayerDied(int victim, int killer, vector at)
	{
		if (!IsOn() || IsWalking(victim))
			return;

		RZ_PlayerProfile p = m_GM.FindProfile(victim);
		if (!p)
			return;

		if (IsBuilt())
			m_Closing.OnDied(victim, killer, at);

		// A notable life (2 hours or more, not by their own hand): a name to bring home all season
		if (killer == victim || p.m_fLifeSeconds < 7200 * Math.Min(1, Scale() * 10))
			return;

		RZ_Background bg = m_GM.GetPlayerBackground(victim);
		string role = "survivor";
		if (bg)
			role = bg.m_sName;

		string killerIdent = "";
		if (killer > 0)
			killerIdent = m_GM.GetProfile(killer).m_sIdentity;

		AddNamed(RZ_Util.PlayerName(victim), "their ID card", -1, at, killerIdent, p.m_sIdentity, role);
	}

	//------------------------------------------------------------------------------------------------
	//! A named person died (a wanderer, a notable player's character): a name to bring home.
	void AddNamed(string name, string what, int faction, vector at, string killerIdent, string ident, string role)
	{
		if (!IsOn() || name.IsEmpty())
			return;

		RZ_EpiName n = new RZ_EpiName();
		n.name = name;
		n.what = what;
		n.faction = faction;
		n.x = at[0];
		n.z = at[2];
		n.lies = PlaceWords(at);
		n.killedBy = killerIdent;
		n.ident = ident;
		n.role = role;
		n.day = RZ_WorldFacts.Get().Day();
		m_S.names.Insert(n);
		while (m_S.names.Count() > NAMED_KEEP)
		{
			// The oldest one still lying out goes; names already home stay on the wall
			int drop = 0;
			foreach (int i, RZ_EpiName old : m_S.names)
			{
				if (old.state == 0)
				{
					drop = i;
					break;
				}
			}

			m_S.names.RemoveOrdered(drop);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "near Morton", or a grid reference.
	string PlaceWords(vector at)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		if (s && RZ_Util.DistXZ(s.m_vCenter, at) <= s.m_fRadius + 800)
			return "near " + s.m_sName;

		return "at grid " + RZ_SettlementScanner.GridReference(at);
	}

	//------------------------------------------------------------------------------------------------
	//! Walking out: the character's end is not a death (no records, no grave, no journal).
	void MarkWalking(int playerId)
	{
		if (!m_aWalking.Contains(playerId))
			m_aWalking.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	bool IsWalking(int playerId)
	{
		return m_aWalking.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! After the walk-out's "death" was skipped.
	void DoneWalking(int playerId)
	{
		m_aWalking.RemoveItem(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Infected killed in a town (RZ_GameModeComponent.OnZombiesKilled): Take Back a Town.
	void OnZombiesKilled(int settlement, vector pos)
	{
		if (ClosingOn())
			m_Closing.OnTownKills(settlement, pos);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerShot(int playerId, vector pos)
	{
		if (IsBuilt() && !m_S.lastDay)
			m_Spots.OnPlayerShot(playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! A closing deed by a player (a delivery, a cup, a name, a line): Personal Goals count them.
	void Deed(int playerId, string kind)
	{
		m_Closing.Deed(playerId, kind);
	}

	//------------------------------------------------------------------------------------------------
	//! A beat a player has met (its place, a leader's topic): the journal lists it.
	void Met(int playerId, string key)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		string k = ident + "|" + key;
		if (!ident.IsEmpty() && !m_S.met.Contains(k))
			m_S.met.Insert(k);
	}

	// ================================================================ The wipe and the hall
	//------------------------------------------------------------------------------------------------
	//! The wipe came due (RZ_Seasons.MarkWipeDue): the voice reads the names put down, the square
	//! check, and the week's lines go into the season block.
	void OnWipeDue()
	{
		if (!IsBuilt() || m_S.lastDay)
		{
			if (IsBuilt())
				AddHallLine("The cure: nobody. Nobody won.");

			return;
		}

		m_Closing.OnWipe();
	}

	//------------------------------------------------------------------------------------------------
	//! A line for this season's block in the hall of fame (by fixed rules; nothing else carries over).
	void AddHallLine(string line)
	{
		if (line.IsEmpty() || m_S.hallLines.Contains(line))
			return;

		m_S.hallLines.Insert(line);
		RZ_HallOfFame hall = m_GM.GetHall();
		if (hall)
			hall.AddWeekLine(line);
	}

	// ================================================================ Carriers
	//------------------------------------------------------------------------------------------------
	//! The talk screen's epilogue replies for a leader (RZ_TalkLeader): "About the season", "Where
	//! should I look?" and a stronghold's grey choice.
	void LeaderReplies(RZ_Leader l, int playerId, notnull array<string> labels, notnull array<int> ids)
	{
		if (!IsBuilt() || !l || m_S.lastDay)
			return;

		labels.Insert("About the season");
		ids.Insert(40);
		labels.Insert("Where should I look?");
		ids.Insert(41);
		int which = m_Closing.ChoiceOf(l);
		if (which < 0 || m_S.choices[which] != 0 || !m_Closing.ChoiceOpen(which))
			return;

		labels.Insert(RZ_EpiText.Answer(which, true));
		ids.Insert(42);
		labels.Insert(RZ_EpiText.Answer(which, false));
		ids.Insert(43);
	}

	//------------------------------------------------------------------------------------------------
	//! A leader reply (40-43) or action (RZ_ELeaderAction.SEASON...): the lines, as a notice.
	void LeaderPick(RZ_Leader l, int playerId, int replyId)
	{
		if (!l)
			return;

		if (!IsBuilt())
		{
			m_GM.NotifyPlayer(playerId, l.GetHeader(), "\"The season? It isn't over yet.\"");
			return;
		}

		if (replyId == 40)
		{
			m_GM.NotifyPlayer(playerId, l.GetHeader(), SeasonTopic(l, playerId));
			return;
		}

		if (replyId == 41)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			string look = "\"Nowhere you haven't been.\"";
			if (ent)
				look = m_Beats.WhereToLook(playerId, ent.GetOrigin());

			if (look.IsEmpty())
				look = "\"Nowhere you haven't been.\"";

			m_GM.NotifyPlayer(playerId, l.GetHeader(), look);
			return;
		}

		m_Closing.ChoicePick(l, playerId, replyId == 42);
	}

	//------------------------------------------------------------------------------------------------
	//! "The cure", "This place", "Me" and "The dead", in one notice.
	protected string SeasonTopic(RZ_Leader l, int playerId)
	{
		string text;
		if (l.IsStronghold())
			text = RZ_EpiText.Verdict(l.m_eFaction, Route(), m_S.terms, m_S.faction);
		else
			text = "\"" + RZ_EpiText.Where(Route(), m_S.terms, m_S.faction) + "\"";

		string place = m_Beats.PlaceLine(l.m_vPos, 400);
		if (!place.IsEmpty())
			text += "\n" + place;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		string me = LineOf(ident, 1);
		if (me.IsEmpty())
			me = LineOf(ident, 0);

		if (!me.IsEmpty())
			text += "\n" + me;

		string dead = m_Closing.DeadAt(l.m_vPos);
		if (!dead.IsEmpty())
			text += "\n" + dead;

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse resident's line in the week ("Ask how they are"), "" before it.
	string ResidentLine(int house)
	{
		if (!IsBuilt() || m_S.lastDay)
			return "";

		RZ_Safehouse h = m_GM.GetSafehouses().Get(house);
		if (!h)
			return "";

		int grew = 0;
		if (house < m_S.houseTier.Count() && h.GetTier() > m_S.houseTier[house])
			grew = h.m_iPopulation;

		return RZ_EpiText.Resident(Step(), h.m_sName, h.m_iSeasonDied, grew);
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer's road news in the week, "" before it.
	string WandererLine(int role, vector pos)
	{
		if (!IsBuilt() || m_S.lastDay)
			return "";

		string where = m_Outlets.NearestText(pos);
		return RZ_EpiText.Road(role, where, m_Beats.NearestEcho(pos, 3000));
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's open channel in the week, "" before it (or now and then, to leave room).
	string ChatterLine(int faction)
	{
		if (!IsBuilt() || m_S.lastDay || Math.RandomFloat01() < 0.5)
			return "";

		return RZ_EpiText.Chatter(faction, Route(), Step());
	}

	//------------------------------------------------------------------------------------------------
	//! Notes pinned to a safehouse board in the week (they don't count toward its 8): the head's
	//! line, the names, a thank-you.
	void BoardNotes(int house, notnull array<string> authors, notnull array<string> texts)
	{
		if (!IsBuilt() || m_S.lastDay)
			return;

		m_Closing.BoardNotes(house, authors, texts);
	}

	//------------------------------------------------------------------------------------------------
	//! A readable found in the week: 3 in 10 are dated in it. "" to keep the built one. Never names
	//! the farm's town unless the house is exposed.
	string DatedReadable(string place, int town)
	{
		if (!IsBuilt() || m_S.lastDay || Math.RandomFloat01() >= 0.3)
			return "";

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts.Flag("table.exists") != 0 && facts.Flag("table.exposed") == 0 && facts.Flag("table.town", -1) == town)
			return "";

		int day = facts.Day();
		array<string> lines = {
			"Gone to the queue at the north road. Took the dog. Petr, the key's where it was.",
			"We heard it on the radio. We're walking to the north road. If you're reading this, so should you.",
			"Gone to the house for the dose. Back for the photos. Don't touch the photos.",
			"The tins are in the oven. The oven doesn't work. That's why they're in it."
		};
		return string.Format("A note on a kitchen table in %1, Day %2: \"%3\"", place, day, lines.GetRandomElement());
	}

	//------------------------------------------------------------------------------------------------
	//! /season and the notebook's action: As the Zone tells it, And you, Unfinished.
	string SeasonText(int playerId)
	{
		if (!IsBuilt())
			return "The season isn't over. Nobody's writing it down yet.";

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		string text = "As the Zone tells it:\n";
		if (m_S.lastDay)
			text += "Nobody won. The perimeter will be maintained.\n";
		else
			text += RZ_EpiText.CureLine(Route(), m_S.how, m_S.terms, m_S.faction, m_S.slow) + "\n";

		text += m_Beats.MetText(ident, Phase() >= 3);
		string you = "";
		for (int i = 0; i < 3; i++)
		{
			string l = LineOf(ident, i);
			if (!l.IsEmpty())
				you += l + "\n";
		}

		if (!you.IsEmpty())
			text += "\nAnd you:\n" + you;

		if (!m_S.lastDay)
		{
			string open = m_Closing.UnfinishedText(playerId);
			if (!open.IsEmpty())
				text += "\nUnfinished:\n" + open;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! /epilogue (admins): preview, phase A|B|C, strike <name or beat>, rebuild, close <item>,
	//! lastnotes <name>, status. Logged.
	void Command(int playerId, string text)
	{
		if (!RZ_NoticeBoard.IsAdmin(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Epilogue", "Only a server admin can do that.");
			return;
		}

		string arg = text.Trim();
		string lower = arg;
		lower.ToLower();
		string result = "";
		if (lower.StartsWith("phase"))
		{
			int phase = 2;
			if (lower.Contains(" a"))
				phase = 1;
			else if (lower.Contains(" c"))
				phase = 3;

			ForcePhase(phase);
			result = string.Format("Phase %1 now.", phase);
		}
		else if (lower.StartsWith("strike"))
		{
			string what = arg.Substring(Math.Min(arg.Length(), 7), Math.Max(0, arg.Length() - 7)).Trim();
			if (!what.IsEmpty() && !m_S.struck.Contains(what))
				m_S.struck.Insert(what);

			m_Beats.Strike(what);
			m_Closing.Strike(what);
			result = "Struck: " + what;
		}
		else if (lower.StartsWith("rebuild") || lower.StartsWith("preview"))
		{
			if (RZ_WorldFacts.Get().IsCured())
			{
				OnCure();
				result = "The record is taken again on the next ticks.";
			}
			else
			{
				m_S.beats.Clear();
				Snapshot();
				m_Beats.Build();
				result = string.Format("Preview: %1 beats would be picked (see the log).", m_S.beats.Count());
				m_S.beats.Clear();
			}
		}
		else if (lower.StartsWith("close"))
		{
			string key = arg.Substring(Math.Min(arg.Length(), 6), Math.Max(0, arg.Length() - 6)).Trim();
			if (m_Closing.CloseNow(key))
				result = "Closed as data: " + key;
			else
				result = "No open item " + key;
		}
		else if (lower.StartsWith("lastnotes"))
		{
			string who = arg.Substring(Math.Min(arg.Length(), 10), Math.Max(0, arg.Length() - 10)).Trim();
			result = m_Closing.ArchiveEntry(who);
		}
		else
		{
			result = StatusText();
		}

		RZ_Log.Info(string.Format("Epilogue: admin %1: %2 (%3)", RZ_Util.PlayerName(playerId), arg, result));
		m_GM.NotifyPlayer(playerId, "Epilogue", result);
	}

	//------------------------------------------------------------------------------------------------
	string StatusText()
	{
		if (!IsBuilt())
			return "Not built: the cure isn't out.";

		return string.Format("Phase %1, step %2, %3 beats, %4 open items, %5 names on the wall, %6 walked out, room %7 h.",
			Phase(), Step(), m_S.beats.Count(), m_Closing.OpenCount(), m_Closing.WallCount(), m_S.walked.Count(), RZ_Util.RoundToInt(RoomHours()));
	}
}
