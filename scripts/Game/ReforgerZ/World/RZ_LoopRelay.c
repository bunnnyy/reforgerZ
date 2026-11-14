//! The loop's relay and Bastion's last call (docs/DESIGN.md, "The loop's relay and Bastion's last
//! call"), at the masts (World/RZ_Masts.c).
//!
//! The relay: one mast carries the government broadcast's loop. With every mast up it can be
//! reached: cut it and "Aid is on its way" goes silent for the whole server for a night, or swap the
//! tape for one of four short tapes for an evening. Once in a long while (Relay cooldown), and every
//! faction says something about it. The broadcast's own words (the season mood, RZ_Broadcast) stay
//! the source of truth: the relay only silences or overrides the loop for its time.
//!
//! Bastion's last call: at the tallest mast, up and open or Bastion's, someone Known with Bastion
//! holds the power while the radio officer calls Command. He calls; the band hisses; nobody answers
//! (the Zone is sealed). Once a real day for the whole server. The climb's noise brings what's below.
//! Server side.

//------------------------------------------------------------------------------------------------
class RZ_RelaySave
{
	int relay = -1;			// the mast that carries the loop
	int tallest = -1;		// the mast Bastion calls from
	int silentUntil;		// Unix seconds: the loop is silent until then
	int tapeUntil;			// Unix seconds: the swapped tape plays until then
	int tape = -1;			// which tape (RZ_LoopRelay.TapeLine)
	int nextOpen;			// Unix seconds: the relay can be reached again from then
	int lastCallDay;		// the real day Bastion last called from the tallest mast
	ref array<string> callPaid = {};	// "<identity>|<day>": the call's Fame, once a real day per player
}

//------------------------------------------------------------------------------------------------
class RZ_LoopRelay
{
	protected static const string CUT = "relay.cut";
	protected static const string TAPE = "relay.tape.";
	protected static const string CALL = "mast.call";
	protected static const int TAPES = 4;
	protected static const float CALL_HEAT = 10;
	protected static const float CREW_REACH = 4000;	// a Red Hand holding this close to the tallest mast sends a crew for the call
	protected static const int CREW_STAYS = 180;		// seconds the crew hangs about after the call ends
	protected static const float TAPE_SECONDS = 12;

	protected static RZ_LoopRelay s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected RZ_MastSettings m_Settings;
	protected RZ_Masts m_Masts;
	protected ref RZ_RelaySave m_State = new RZ_RelaySave();
	protected int m_iCaller;			// the player holding the power for the call (0 none)
	protected ref RZ_UnitWatch m_CallCrew = new RZ_UnitWatch();	// the Red Hand's crew come for the set while the call is held
	protected int m_iCrewUntil;			// unix time the crew goes home (a while after the call)
	static bool s_bTestCrewAnywhere;	// the runtime test only: a crew comes whatever holdings are near
	protected int m_iCutter;			// the player whose relay hold is open (0 none)
	protected bool m_bTapeAired;		// the swapped tape aired once at once (the loop then carries it)

	//------------------------------------------------------------------------------------------------
	void RZ_LoopRelay(RZ_GameModeComponent gm, RZ_MastSettings settings, RZ_Masts masts)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Masts = masts;
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_LoopRelay Get()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		return m_Settings && m_Settings.m_bRelay && m_Masts && m_Masts.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	int RelayIndex() { return m_State.relay; }
	int TallestIndex() { return m_State.tallest; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_RelaySave saved)
	{
		if (!saved)
			return;

		m_State = saved;
		if (!m_State.callPaid)
			m_State.callPaid = {};
	}

	//------------------------------------------------------------------------------------------------
	RZ_RelaySave Save()
	{
		return m_State;
	}

	//------------------------------------------------------------------------------------------------
	//! After the masts are found: which carries the loop (the one nearest the radio station, else the
	//! first) and which is tallest (the highest ground). A saved pick stands while it's in range.
	void Setup()
	{
		int count = m_Masts.Count();
		if (count <= 0)
			return;

		if (m_State.relay < 0 || m_State.relay >= count)
		{
			vector station = vector.Zero;
			bool hasStation = false;
			if (m_GM.GetRadioStation())
			{
				station = m_GM.GetRadioStation().GetPosition();
				hasStation = station != vector.Zero;
			}

			m_State.relay = 0;
			float best = float.MAX;
			for (int i = 0; i < count && hasStation; i++)
			{
				float d = RZ_Util.DistXZ(m_Masts.GetMast(i).m_vPos, station);
				if (d < best)
				{
					best = d;
					m_State.relay = i;
				}
			}
		}

		if (m_State.tallest < 0 || m_State.tallest >= count)
		{
			m_State.tallest = 0;
			float high = -100000;
			for (int k = 0; k < count; k++)
			{
				float h = m_Masts.GetMast(k).m_vPos[1];
				if (h > high)
				{
					high = h;
					m_State.tallest = k;
				}
			}
		}

		RZ_Log.Info(string.Format("Masts: the loop's relay is %1; Bastion calls from %2.", m_Masts.GetMast(m_State.relay).m_sName, m_Masts.GetMast(m_State.tallest).m_sName));
	}

	// ================================================================ The loop (RZ_Broadcast)
	//------------------------------------------------------------------------------------------------
	//! The loop is silent now (the relay cut).
	static bool LoopSilent()
	{
		RZ_LoopRelay r = Get();
		return r && r.IsOn() && RZ_Guidance.UnixNow() < r.m_State.silentUntil;
	}

	//------------------------------------------------------------------------------------------------
	//! The swapped tape's words while it plays, else "" (the loop's own words).
	static string LoopOverride()
	{
		RZ_LoopRelay r = Get();
		if (!r || !r.IsOn() || r.m_State.tape < 0 || RZ_Guidance.UnixNow() >= r.m_State.tapeUntil)
			return "";

		return TapeLine(r.m_State.tape);
	}

	//------------------------------------------------------------------------------------------------
	//! The four tapes, read on the government's band in a voice that isn't the government's.
	static string TapeLine(int tape)
	{
		switch (tape)
		{
			case 0: return "This isn't the government. Nobody's coming. Look after the ones next to you.";
			case 1: return "This isn't the government. The lab's still working. Hold on a while yet.";
			case 2: return "This isn't the government. Stay off the roads after dark. They hear you before you hear them.";
			case 3: return "This isn't the government. We're still here. Say it to someone tonight.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The row's words for a tape.
	static string TapeLabel(int tape)
	{
		switch (tape)
		{
			case 0: return "Swap the tape: \"Nobody's coming\"";
			case 1: return "Swap the tape: \"The lab's still working\"";
			case 2: return "Swap the tape: \"Stay off the roads after dark\"";
			case 3: return "Swap the tape: \"We're still here\"";
		}

		return "";
	}

	// ================================================================ The menu (RZ_Masts.Rows)
	//------------------------------------------------------------------------------------------------
	//! Every mast is up, so the relay's loop can be reached.
	bool AllUp()
	{
		return m_Masts.Count() > 0 && m_Masts.UpCount() >= m_Masts.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Rows at mast `m` for `playerId`: the relay's at the relay, the call's at the tallest.
	void Rows(RZ_Mast m, int playerId, notnull array<string> acts, notnull array<string> labels, notnull array<string> locks)
	{
		if (!IsOn() || !m.m_bUp)
			return;

		if (m.m_iId == m_State.relay)
		{
			string why = "";
			if (!AllUp())
				why = string.Format("The loop's relay won't take a hand until every mast is up (%1 of %2).", m_Masts.UpCount(), m_Masts.Count());
			else if (RZ_Guidance.UnixNow() < m_State.nextOpen)
				why = "Someone's been at the relay. The box is soldered shut again, for now.";
			else if (m_iCutter > 0 && m_iCutter != playerId)
				why = "Someone's at the relay already.";

			acts.Insert("relay:cut");
			labels.Insert("Cut the loop");
			locks.Insert(why);
			for (int t = 0; t < TAPES; t++)
			{
				acts.Insert("relay:tape:" + t.ToString());
				labels.Insert(TapeLabel(t));
				locks.Insert(why);
			}
		}

		if (m.m_iId == m_State.tallest && (m.m_eOwner == RZ_EMastOwner.OPEN || m.m_eOwner == RZ_EMastOwner.BASTION))
		{
			string callWhy = "";
			if (playerId > 0 && !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN))
				callWhy = "Bastion don't ask strangers to hold their set. Be Known with them first.";
			else if (m_State.lastCallDay == RealDay())
				callWhy = "Bastion have called Command from here today already. Tomorrow.";
			else if (m_iCaller > 0 && m_iCaller != playerId)
				callWhy = "Someone's holding the power for them already.";

			acts.Insert("relay:call");
			labels.Insert("Hold the power for Bastion's call");
			locks.Insert(callWhy);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A relay or call row picked. True if it was one.
	bool Act(RZ_Mast m, string act, int playerId)
	{
		if (act.IndexOf("relay:") != 0)
			return false;

		if (!IsOn() || !m.m_bUp)
			return true;

		if (act == "relay:cut")
		{
			OpenRelay(m, playerId, CUT, "Cut the loop", CutSeconds());
			return true;
		}

		if (act == "relay:call")
		{
			OpenCall(m, playerId);
			return true;
		}

		if (act.IndexOf("relay:tape:") == 0)
		{
			int tape = act.Substring(11, act.Length() - 11).ToInt();
			if (tape >= 0 && tape < TAPES)
				OpenRelay(m, playerId, TAPE + tape.ToString(), "Swap the tape", TAPE_SECONDS);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected float CutSeconds()
	{
		RZ_Substations subs = RZ_Substations.Get();
		if (subs && subs.GetSettings())
			return subs.GetSettings().m_fCutSeconds;

		return 8;
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenRelay(RZ_Mast m, int playerId, string key, string label, float hold)
	{
		if (m.m_iId != m_State.relay || !AllUp() || RZ_Guidance.UnixNow() < m_State.nextOpen)
			return;

		m_iCutter = playerId;
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites || !RZ_WorkSites.Available())
		{
			Done(m, key, playerId);
			return;
		}

		sites.Close(key);
		sites.Open(key, RZ_EWork.HANDS, label, hold, m_Masts.CabinetPos(m), playerId);
		sites.SetAtPiece(key, m_Masts.CabinetPos(m), 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenCall(RZ_Mast m, int playerId)
	{
		if (m.m_iId != m_State.tallest || m_State.lastCallDay == RealDay())
			return;

		if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN))
			return;

		m_iCaller = playerId;
		string channel = RZ_FactionVoice.ChannelTitle(RZ_EFaction.MILITARY);
		m_GM.Say(playerId, channel, RZ_EFaction.MILITARY, "Radio officer. Keep your hand on that breaker and don't let it drop. I'm calling Command. Same as every night.");

		// The set's hum and the generator are heard; what lives below comes
		if (m_GM.GetHeat())
			m_GM.GetHeat().AddHeat(m.m_vPos, CALL_HEAT);

		m_Masts.WakeBelow(m);
		SendCrew(m, playerId);
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites || !RZ_WorkSites.Available())
		{
			Done(m, CALL, playerId);
			return;
		}

		sites.Close(CALL);
		sites.Open(CALL, RZ_EWork.HANDS, "Hold the power for Bastion's call", m_Settings.m_fCallSeconds, m_Masts.CabinetPos(m), playerId);
		sites.SetAtPiece(CALL, m_Masts.CabinetPos(m), 0);
		RZ_Log.Info(string.Format("Masts: %1 holds the power at %2 for Bastion's call.", RZ_Names.Public(playerId), m.m_sName));
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand at the call: with one of its holdings within CREW_REACH of the tallest mast, a crew
	//! comes up the hill for the set while the power's held (Missions > Red Hand group), spawned 180 to
	//! 260 m out (a player is there: the one holding the power) and sent to the cabinet, set to fight.
	//! They go home CREW_STAYS seconds after the call ends, or when they're all down.
	protected bool SendCrew(RZ_Mast m, int playerId)
	{
		if (m_CallCrew.IsSpawned() || !m_GM.GetTerritory() || m_GM.GetRedHandGroupPrefab().IsEmpty())
			return false;

		bool near = s_bTestCrewAnywhere;
		foreach (RZ_Holding h : m_GM.GetTerritory().GetHoldings())
		{
			if (h && h.m_eFaction == RZ_EFaction.BANDITS && RZ_Util.DistXZ(h.m_vPos, m.m_vPos) <= CREW_REACH)
			{
				near = true;
				break;
			}
		}

		if (!near)
			return false;

		vector from = RZ_Mission.LandPointNear(m.m_vPos, 180, 260);
		if (!m_CallCrew.Spawn(m_GM.GetRedHandGroupPrefab(), from))
			return false;

		if (m_GM.GetSpawner())
			RZ_Util.SendGroupTo(m_CallCrew.m_Unit, m_Masts.CabinetPos(m), m_GM.GetSpawner().GetMoveWaypointPrefab());

		RZ_Hostility.MarkFight(RZ_EFaction.BANDITS, m_CallCrew.m_Unit);
		m_iCrewUntil = 0;
		m_GM.Say(playerId, RZ_FactionVoice.ChannelTitle(RZ_EFaction.MILITARY), RZ_EFaction.MILITARY, "Engines on the hill road. That's the Red Hand: they want the set, or they want us off it. Keep the power on.");
		RZ_Log.Info(string.Format("Masts: a Red Hand crew comes up %1 for the set while Bastion calls (from %2).", m.m_sName, RZ_SettlementScanner.GridReference(from)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The crew goes home a while after the call (or when it's all down).
	protected void TickCrew()
	{
		if (!m_CallCrew.IsSpawned())
			return;

		if (m_CallCrew.IsDead(1))
		{
			m_CallCrew.Delete();
			return;
		}

		if (m_iCaller > 0)
			return;

		int now = RZ_Guidance.UnixNow();
		if (m_iCrewUntil == 0)
			m_iCrewUntil = now + CREW_STAYS;

		if (now >= m_iCrewUntil)
		{
			m_CallCrew.Delete();
			RZ_Log.Info("Masts: the Red Hand's crew goes home from the call.");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: a crew sent for the call at `m` now (the holdings' rule kept). True if it came.
	bool TestCrew(RZ_Mast m, int playerId)
	{
		return SendCrew(m, playerId);
	}

	//------------------------------------------------------------------------------------------------
	bool HasCrew()
	{
		return m_CallCrew.IsSpawned() && m_CallCrew.m_Unit != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: the crew sent home now.
	void TestCrewHome()
	{
		m_CallCrew.Delete();
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Masts.TickHolds: a relay or call hold at `m` finished.
	void TickHolds(RZ_Mast m)
	{
		TickCrew();
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return;

		array<string> keys = {CUT, CALL};
		for (int t = 0; t < TAPES; t++)
		{
			keys.Insert(TAPE + t.ToString());
		}

		foreach (string key : keys)
		{
			if (!sites.IsDone(key))
				continue;

			int by = sites.DoneBy(key);
			sites.Close(key);
			if (by > 0)
				Done(m, key, by);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A hold done (or no holds on this server).
	protected void Done(RZ_Mast m, string key, int playerId)
	{
		if (key == CALL)
		{
			m_iCaller = 0;
			FinishCall(m, playerId);
			return;
		}

		m_iCutter = 0;
		if (key == CUT)
		{
			CutLoop(playerId);
			return;
		}

		if (key.IndexOf(TAPE) == 0)
			SwapTape(playerId, key.Substring(TAPE.Length(), key.Length() - TAPE.Length()).ToInt());
	}

	// ================================================================ The relay
	//------------------------------------------------------------------------------------------------
	//! Cut the loop: silent for a night, server-wide. True if it was cut.
	bool CutLoop(int playerId)
	{
		if (!IsOn() || RZ_Guidance.UnixNow() < m_State.nextOpen)
			return false;

		int now = RZ_Guidance.UnixNow();
		m_State.silentUntil = now + RZ_Util.RoundToInt(m_Settings.m_fSilenceHours * 3600);
		m_State.tape = -1;
		m_State.tapeUntil = 0;
		m_State.nextOpen = now + RZ_Util.RoundToInt(m_Settings.m_fRelayCooldownHours * 3600);
		Flag("relay.cut");

		if (m_GM.GetHeat())
			m_GM.GetHeat().AddHeat(m_Masts.GetMast(m_State.relay).m_vPos, 8);

		// Who did it is answered for: Bastion's Command loop cut, the Haven glad of the quiet
		if (m_Settings.m_iRelayInfamy > 0)
			m_GM.AddInfamy(playerId, RZ_EFaction.MILITARY, m_Settings.m_iRelayInfamy, "cut the government's loop");

		if (m_Settings.m_iRelayFame > 0)
			m_GM.AddFame(playerId, RZ_EFaction.SURVIVORS, m_Settings.m_iRelayFame, false, 0, "stopped the loop's lie for a night");

		React(true);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline("The government's loop went quiet tonight. Somebody cut the relay.", false);

		RZ_Log.Info(string.Format("Masts: %1 cut the loop's relay; silent for %2 h.", RZ_Names.Public(playerId), m_Settings.m_fSilenceHours));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Swap the tape: it airs at once to everyone, then the loop carries it for an evening. True if
	//! it was swapped.
	bool SwapTape(int playerId, int tape)
	{
		if (!IsOn() || tape < 0 || tape >= TAPES || RZ_Guidance.UnixNow() < m_State.nextOpen)
			return false;

		int now = RZ_Guidance.UnixNow();
		m_State.tape = tape;
		m_State.tapeUntil = now + RZ_Util.RoundToInt(m_Settings.m_fTapeHours * 3600);
		m_State.silentUntil = 0;
		m_State.nextOpen = now + RZ_Util.RoundToInt(m_Settings.m_fRelayCooldownHours * 3600);
		Flag("relay.tape");

		if (m_Settings.m_iTapeInfamy > 0)
			m_GM.AddInfamy(playerId, RZ_EFaction.MILITARY, m_Settings.m_iTapeInfamy, "put words on the government's band");

		// It airs now, once, to everyone: the loop then carries it until the evening's out
		m_GM.NotifyAll(RZ_Broadcast.TITLE, TapeLine(tape));
		React(false);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline("Somebody put their own tape in the government's loop tonight.", false);

		RZ_Log.Info(string.Format("Masts: %1 swapped the loop's tape (%2) for %3 h.", RZ_Names.Public(playerId), tape, m_Settings.m_fTapeHours));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every faction says something on its open channel, and its morale moves a little.
	protected void React(bool cut)
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (!life)
			return;

		if (cut)
		{
			life.OpenChannelLine(RZ_EFaction.SURVIVORS, "The loop's stopped. First quiet night in weeks. Whoever did it: thank you. Now listen to each other instead.");
			life.OpenChannelLine(RZ_EFaction.MILITARY, "All stations. The Command loop is down. That is not an order to stand down. Nobody stands down.");
			life.OpenChannelLine(RZ_EFaction.BANDITS, "Somebody shut the government up. Drinks are on the book. Not really.");
			life.OpenChannelLine(RZ_EFaction.INSTITUTE, "Halcyon, all staff. The outside loop is down. Record the time. They'll think we've stopped transmitting.");
			MoveMorale(RZ_EFaction.SURVIVORS, 2);
			MoveMorale(RZ_EFaction.MILITARY, -3);
			MoveMorale(RZ_EFaction.BANDITS, 2);
			MoveMorale(RZ_EFaction.INSTITUTE, -1);
			return;
		}

		life.OpenChannelLine(RZ_EFaction.SURVIVORS, "Did you hear the band? That wasn't them. That was one of us. Keep it on tonight.");
		life.OpenChannelLine(RZ_EFaction.MILITARY, "All stations. Unauthorised traffic on the government band. Disregard it. Do not repeat it.");
		life.OpenChannelLine(RZ_EFaction.BANDITS, "Somebody's on the big band with their own tape. Bold. We'd have charged for the slot.");
		life.OpenChannelLine(RZ_EFaction.INSTITUTE, "Halcyon, all staff. Someone is using the government band. Note the content. Do not comment on it.");
		MoveMorale(RZ_EFaction.SURVIVORS, 1);
		MoveMorale(RZ_EFaction.MILITARY, -2);
		MoveMorale(RZ_EFaction.BANDITS, 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveMorale(int f, float delta)
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.AddMorale(f, delta);
	}

	//------------------------------------------------------------------------------------------------
	protected void Flag(string key)
	{
		RZ_WorldFacts facts = m_GM.GetFacts();
		if (facts)
			facts.SetFlag(key, facts.Flag(key) + 1);
	}

	// ================================================================ Bastion's last call
	//------------------------------------------------------------------------------------------------
	//! The power held to the end: he calls, the band hisses, nobody answers. True if the call ran.
	bool FinishCall(RZ_Mast m, int playerId)
	{
		int today = RealDay();
		if (m_State.lastCallDay == today)
			return false;

		m_State.lastCallDay = today;
		string channel = RZ_FactionVoice.ChannelTitle(RZ_EFaction.MILITARY);
		m_GM.Say(playerId, channel, RZ_EFaction.MILITARY, "Command, Command, this is Task Force Bastion. We are holding. Requesting relief. Over.");
		m_GM.Say(playerId, channel, RZ_EFaction.MILITARY, "Nothing but the hiss, for a long time. Bastion out. Thanks for holding it. Same time tomorrow, if you're about.");

		// Await Relief (docs/QUESTLINES.md): the nights Bastion called, a world fact the major's lines can
		// read; while it runs, the officer says it goes in the log for the CO
		Flag("bastion.last_call");
		RZ_Questlines quests = m_GM.GetQuestlines();
		if (quests && quests.IsRunning("orders"))
			m_GM.Say(playerId, channel, RZ_EFaction.MILITARY, "Log it for the CO. Another night, no reply. He'll want the count.");
		MoveMorale(RZ_EFaction.MILITARY, -1);

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p && !p.m_sIdentity.IsEmpty() && m_Settings.m_iCallFame > 0)
		{
			string key = p.m_sIdentity + "|" + today.ToString();
			if (!m_State.callPaid.Contains(key))
			{
				m_State.callPaid.Insert(key);
				m_GM.AddFame(playerId, RZ_EFaction.MILITARY, m_Settings.m_iCallFame, false, 0, "held the power while we called Command");
			}
		}

		RZ_Log.Info(string.Format("Masts: Bastion called Command from %1 (%2 held the power). No answer.", m.m_sName, RZ_Names.Public(playerId)));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static int RealDay()
	{
		return RZ_Guidance.UnixNow() / 86400;
	}

	// ================================================================ The test (RZT_Relay.c)
	//------------------------------------------------------------------------------------------------
	//! Test only: the relay's cooldown cleared, so it can be reached again now.
	void TestOpen()
	{
		m_State.nextOpen = 0;
		m_State.lastCallDay = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: the silence and the tape run out now.
	void TestExpire()
	{
		m_State.silentUntil = 0;
		m_State.tapeUntil = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: the call run for a player at the tallest mast.
	bool TestCall(int playerId)
	{
		RZ_Mast m = m_Masts.GetMast(m_State.tallest);
		if (!m)
			return false;

		return FinishCall(m, playerId);
	}
}
