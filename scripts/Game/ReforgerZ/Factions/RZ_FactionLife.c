//! Factions feel alive (docs/DESIGN.md, "Faction life"; docs/REPUTATION.md,
//! "Morale, needs, truces and feuds"). Everything is capped, background and cheap:
//!
//! - Needs: what each faction wants most right now, from the world (lost trucks, deaths,
//!   lost towns, hungry safehouses, research). Leaders say it; it leans contract weights (+20% at most).
//! - Morale 5..95 per faction: falls with losses, recovers toward 60 on its own. Low morale
//!   makes its people jumpier (retaliation), its shops dearer and the Haven's rescued less
//!   likely to stay; high morale the other way. All small.
//! - Radio chatter on the factions' open channels with real information (truck departures and
//!   routes, shift times, toll hours, samples wanted). Radio Operators hear rival open channels;
//!   secure channels (GetSecureTraffic) are cracked by Radio Operators (RZ_SecureRadio).
//! - Life events in the headlines: funerals, promotions, recruiting, a researcher quitting.
//! - Temporary truces and feuds (at most two at once, 45-90 minutes, always end) that change
//!   rival Infamy and who takes towns from whom.
//! - Big moments are remembered ("not since the night the lab nearly fell") and come back
//!   in leaders' and wanderers' talk.
//! - Rumours about other factions: true, biased or false, more often false about rivals.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Faction life.
[BaseContainerProps()]
class RZ_FactionLifeSettings
{
	[Attribute("1", desc: "Faction life on: needs, morale, radio chatter, life events, truces and feuds, remembered moments, rumours")]
	bool m_bEnabled;

	[Attribute("10", desc: "Real minutes between faction chatter on the open channels (players with a radio). 0 = off")]
	float m_fChatterMinutes;

	[Attribute("35", desc: "Average real minutes between faction life events in the headlines. 0 = off")]
	float m_fLifeEventMinutes;

	[Attribute("1", desc: "Temporary truces and feuds between factions (they change rival Infamy and territory takes, and always end)")]
	bool m_bTrucesAndFeuds;

	[Attribute("0.35", desc: "Chance a rumour about a rival is biased or false (about anyone else: 0.1)")]
	float m_fFalseRumourChance;
}

//------------------------------------------------------------------------------------------------
class RZ_FactionLifeSave
{
	float clock;
	float relCooldown;
	ref array<float> morale = {};
	ref array<int> relA = {};
	ref array<int> relB = {};
	ref array<int> relKind = {};
	ref array<float> relUntil = {};
	ref array<string> moments = {};
}

//------------------------------------------------------------------------------------------------
class RZ_FactionLife
{
	static const int REL_TRUCE = 0;
	static const int REL_FEUD = 1;

	protected static const float MORALE_MIN = 5;
	protected static const float MORALE_MAX = 95;
	protected static const float MORALE_REST = 60;
	protected static const float MORALE_INTERVAL = 600;
	protected static const float NEED_INTERVAL = 300;
	protected static const float REL_INTERVAL = 1800;
	protected static const float REL_COOLDOWN = 1800;
	protected static const int REL_MAX = 2;
	protected static const int MOMENTS_KEPT = 8;
	protected static const float HORDE_MOMENT_GAP = 7200;
	protected static const float SECURE_RADIUS = 400;
	protected static const float MULT_MIN = 0.85;
	protected static const float MULT_MAX = 1.2;

	protected RZ_GameModeComponent m_GM;
	protected RZ_FactionLifeSettings m_Settings;
	protected ref RZ_FactionLifeSave m_Restored;
	protected ref array<float> m_aMorale = {60, 60, 60, 60};
	protected ref array<int> m_aNeed = {0, 0, 0, 0};
	protected ref array<float> m_aRecentTrucks = {0, 0, 0, 0};		// lost trucks, decaying by one an hour
	protected ref array<float> m_aRecentDeaths = {0, 0, 0, 0};
	protected ref array<float> m_aRecentLosses = {0, 0, 0, 0};		// outposts lost
	protected ref array<int> m_aRelA = {};
	protected ref array<int> m_aRelB = {};
	protected ref array<int> m_aRelKind = {};
	protected ref array<float> m_aRelUntil = {};
	protected ref array<string> m_aMoments = {};
	protected ref array<string> m_aPending = {};		// life events waiting their turn (funerals)
	protected ref array<float> m_aPendingAt = {};
	protected float m_fClock;
	protected float m_fRelCooldown;
	protected float m_fMoraleTimer;
	protected float m_fNeedTimer;
	protected float m_fRelTimer;
	protected float m_fNextEvent;
	protected float m_fChatterTimer;
	protected int m_iChatterStep;
	protected float m_fLastHordeMoment = -100000;
	protected bool m_bActive;

	//------------------------------------------------------------------------------------------------
	void RZ_FactionLife(RZ_GameModeComponent gm, RZ_FactionLifeSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive() { return m_bActive; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_FactionLifeSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_FactionLifeSave Save()
	{
		RZ_FactionLifeSave data = new RZ_FactionLifeSave();
		data.clock = m_fClock;
		data.relCooldown = m_fRelCooldown;
		foreach (float morale : m_aMorale)
		{
			data.morale.Insert(morale);
		}

		foreach (int i, int a : m_aRelA)
		{
			data.relA.Insert(a);
			data.relB.Insert(m_aRelB[i]);
			data.relKind.Insert(m_aRelKind[i]);
			data.relUntil.Insert(m_aRelUntil[i]);
		}

		foreach (string moment : m_aMoments)
		{
			data.moments.Insert(moment);
		}

		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!m_Settings.m_bEnabled)
		{
			RZ_Log.Info("Faction life is off.");
			return;
		}

		m_bActive = true;
		if (m_Restored)
		{
			m_fClock = m_Restored.clock;
			m_fRelCooldown = m_Restored.relCooldown;
			if (m_Restored.morale)
			{
				for (int f = 0; f < m_Restored.morale.Count() && f < RZ_Factions.COUNT; f++)
				{
					m_aMorale[f] = Math.Clamp(m_Restored.morale[f], MORALE_MIN, MORALE_MAX);
				}
			}

			if (m_Restored.relA && m_Restored.relB && m_Restored.relKind && m_Restored.relUntil)
			{
				for (int r = 0; r < m_Restored.relA.Count() && r < m_Restored.relB.Count() && r < m_Restored.relKind.Count() && r < m_Restored.relUntil.Count(); r++)
				{
					m_aRelA.Insert(m_Restored.relA[r]);
					m_aRelB.Insert(m_Restored.relB[r]);
					m_aRelKind.Insert(m_Restored.relKind[r]);
					m_aRelUntil.Insert(m_Restored.relUntil[r]);
				}
			}

			if (m_Restored.moments)
			{
				foreach (string phrase : m_Restored.moments)
				{
					m_aMoments.Insert(phrase);
				}
			}
		}

		m_Restored = null;
		ScheduleEvent();
		UpdateNeeds();
		RZ_Log.Info(string.Format("Faction life: morale %1/%2/%3/%4, %5 remembered moments.",
			RZ_Util.RoundToInt(m_aMorale[0]), RZ_Util.RoundToInt(m_aMorale[1]), RZ_Util.RoundToInt(m_aMorale[2]), RZ_Util.RoundToInt(m_aMorale[3]), m_aMoments.Count()));
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		float decay = dt / 3600;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			m_aRecentTrucks[f] = Math.Max(0, m_aRecentTrucks[f] - decay);
			m_aRecentDeaths[f] = Math.Max(0, m_aRecentDeaths[f] - decay);
			m_aRecentLosses[f] = Math.Max(0, m_aRecentLosses[f] - decay * 0.5);
		}

		m_fMoraleTimer += dt;
		if (m_fMoraleTimer >= MORALE_INTERVAL)
		{
			m_fMoraleTimer = 0;
			RecoverMorale();
		}

		m_fNeedTimer += dt;
		if (m_fNeedTimer >= NEED_INTERVAL)
		{
			m_fNeedTimer = 0;
			UpdateNeeds();
		}

		ExpireRelations();
		if (ids.IsEmpty())
			return;

		m_fRelTimer += dt;
		if (m_fRelTimer >= REL_INTERVAL)
		{
			m_fRelTimer = 0;
			MaybeStartRelation();
		}

		if (m_Settings.m_fLifeEventMinutes > 0 && m_fClock >= m_fNextEvent)
		{
			ScheduleEvent();
			LifeEvent();
		}

		if (m_Settings.m_fChatterMinutes > 0)
		{
			// Working radios while the power grid runs: they talk more (RZ_PowerGrid)
			float chatterMult = 1;
			if (m_GM.GetPowerGrid())
				chatterMult = m_GM.GetPowerGrid().ChatterMult();

			m_fChatterTimer += dt;
			if (m_fChatterTimer >= m_Settings.m_fChatterMinutes * 60 * chatterMult)
			{
				m_fChatterTimer = 0;
				Chatter(ids, players);
			}
		}
	}

	// ================================================================ Morale
	//------------------------------------------------------------------------------------------------
	float GetMorale(int f)
	{
		if (!RZ_Factions.IsValid(f))
			return MORALE_REST;

		return m_aMorale[f];
	}

	//------------------------------------------------------------------------------------------------
	void AddMorale(int f, float delta)
	{
		if (!m_bActive || !RZ_Factions.IsValid(f))
			return;

		m_aMorale[f] = Math.Clamp(m_aMorale[f] + delta, MORALE_MIN, MORALE_MAX);
	}

	//------------------------------------------------------------------------------------------------
	//! Every 10 minutes, 2 points back toward 60.
	protected void RecoverMorale()
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (m_aMorale[f] < MORALE_REST - 1)
				AddMorale(f, 2);
			else if (m_aMorale[f] > MORALE_REST + 1)
				AddMorale(f, -2);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Retaliation radius and time for this faction's people: x1.25 at low morale, x0.85 at high.
	float JumpinessMult(int f)
	{
		if (!m_bActive)
			return 1;

		float m = GetMorale(f);
		if (m < 30)
			return 1.25;
		if (m > 75)
			return 0.85;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Shop prices: x1.1 at low morale, x0.95 at high.
	float PriceMult(int f)
	{
		if (!m_bActive)
			return 1;

		float m = GetMorale(f);
		if (m < 30)
			return 1.1;
		if (m > 75)
			return 0.95;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The chance a rescued survivor walks off instead of joining a safehouse: x2.5 when the
	//! Haven's morale is low, halved when it's high.
	float HavenWalkChance(float base)
	{
		if (!m_bActive)
			return base;

		float m = GetMorale(RZ_EFaction.SURVIVORS);
		if (m < 30)
			return Math.Min(0.5, base * 2.5);
		if (m > 75)
			return base * 0.5;

		return base;
	}

	// ================================================================ Needs
	//------------------------------------------------------------------------------------------------
	int GetNeed(int f)
	{
		if (!m_bActive || !RZ_Factions.IsValid(f))
			return RZ_ENeed.NONE;

		return m_aNeed[f];
	}

	//------------------------------------------------------------------------------------------------
	//! From the world, every 5 minutes.
	protected void UpdateNeeds()
	{
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			m_aNeed[f] = ComputeNeed(f);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int ComputeNeed(int f)
	{
		bool feud = false;
		foreach (int i, int a : m_aRelA)
		{
			if (m_aRelKind[i] == REL_FEUD && (a == f || m_aRelB[i] == f))
				feud = true;
		}

		if (f == RZ_EFaction.SURVIVORS)
		{
			RZ_Safehouses houses = m_GM.GetSafehouses();
			if (houses)
			{
				int people = 0;
				for (int h = 0; h < houses.Count(); h++)
				{
					if (houses.Get(h).m_bShort)
						return RZ_ENeed.FOOD;

					people += houses.Get(h).m_iPopulation;
				}

				if (m_aRecentTrucks[f] >= 1)
					return RZ_ENeed.FOOD;

				if (m_aRecentDeaths[f] >= 1)
					return RZ_ENeed.SECURITY;

				if (houses.Count() > 0 && people < houses.Count() * 6)
					return RZ_ENeed.PEOPLE;
			}

			return RZ_ENeed.NONE;
		}

		if (f == RZ_EFaction.INSTITUTE)
		{
			if (m_aRecentDeaths[f] >= 1 || m_aRecentLosses[f] >= 1)
				return RZ_ENeed.SECURITY;

			if (m_GM.HasLab() && !m_GM.GetCure().IsCured())
				return RZ_ENeed.SAMPLES;

			return RZ_ENeed.NONE;
		}

		if (f == RZ_EFaction.BANDITS && (feud || m_aRecentLosses[f] >= 1))
			return RZ_ENeed.REVENGE;

		if (m_aRecentTrucks[f] >= 1)
			return RZ_ENeed.SUPPLIES;

		if (m_aRecentDeaths[f] >= 1 || m_aRecentLosses[f] >= 1)
			return RZ_ENeed.SECURITY;

		if (feud)
			return RZ_ENeed.REVENGE;

		return RZ_ENeed.NONE;
	}

	//------------------------------------------------------------------------------------------------
	//! Contract weights (RZ_MissionManager.CreateRandom): work that answers the giver's need
	//! x1.2; a demoralised faction puts out a little less work (x0.9). Within 0.85-1.2.
	float ContractWeightMult(RZ_Mission m)
	{
		if (!m_bActive || !m || !m.m_bContract || !m.m_bHasGiver)
			return 1;

		float mult = 1;
		if (MatchesNeed(m, GetNeed(m.m_eGiver)))
			mult = mult * 1.2;

		if (GetMorale(m.m_eGiver) < 30)
			mult = mult * 0.9;

		return Math.Clamp(mult, MULT_MIN, MULT_MAX);
	}

	//------------------------------------------------------------------------------------------------
	protected bool MatchesNeed(RZ_Mission m, int need)
	{
		switch (need)
		{
			case RZ_ENeed.FOOD: return RZ_Mission_Delivery.Cast(m) != null;
			case RZ_ENeed.SUPPLIES: return RZ_Mission_Delivery.Cast(m) != null;
			case RZ_ENeed.SECURITY: return RZ_Mission_Hold.Cast(m) != null || RZ_Mission_KillTagged.Cast(m) != null || m.m_bHasRival;
			case RZ_ENeed.REVENGE: return m.m_bHasRival && m.m_iRivalInfamy > 0;
			case RZ_ENeed.SAMPLES: return m.m_eGiver == RZ_EFaction.INSTITUTE;
		}

		if (need == RZ_ENeed.PEOPLE)
		{
			string typeName = m.Type().ToString();
			return typeName.Contains("Rescue") || typeName.Contains("Survivor") || typeName.Contains("Escort") || typeName.Contains("Kid");
		}

		return false;
	}

	// ================================================================ World hooks
	//------------------------------------------------------------------------------------------------
	void OnTruckDeparted(int f, string from, string dest)
	{
	}

	//------------------------------------------------------------------------------------------------
	void OnTruckArrived(int f, string place)
	{
		RZ_EpiRecord.Note("truck.ok", f);
		AddMorale(f, 4);
	}

	//------------------------------------------------------------------------------------------------
	//! why: destroyed, robbed or late. Crews that died get a funeral; a truck lost near a rival's
	//! holding may start a feud; a burning convoy is remembered.
	void OnTruckLost(int f, string place, string driver, string why, vector at)
	{
		RZ_EpiRecord.Note("truck.lost", f);
		if (!m_bActive)
			return;

		m_aRecentTrucks[f] = m_aRecentTrucks[f] + 1;
		AddMorale(f, -10);
		if (why == "late")
			return;

		RZ_Territory territory = m_GM.GetTerritory();
		string home = place;
		if (territory && territory.GetStronghold(f))
			home = territory.GetStronghold(f).m_sName;

		QueueEvent(RZ_FactionVoice.Funeral(f, driver, home), Math.RandomFloat(600, 1200));
		if (why == "destroyed")
			RecordMoment("the convoy that burned on the road to " + place);

		if (!territory)
			return;

		RZ_Holding near = territory.FindHolding(-1, at, 1500);
		if (near && near.m_eFaction != f && Math.RandomFloat01() < 0.5)
			StartRelation(f, near.m_eFaction, REL_FEUD);
	}

	//------------------------------------------------------------------------------------------------
	void OnTruckStolen(int owner, int receiver)
	{
		if (m_bActive && Math.RandomFloat01() < 0.6)
			StartRelation(owner, receiver, REL_FEUD);
	}

	//------------------------------------------------------------------------------------------------
	void OnLeaderDied(int f, string name, string place, int killer)
	{
		RZ_EpiRecord.Note("leader.lost", f, name);
		if (!m_bActive)
			return;

		m_aRecentDeaths[f] = m_aRecentDeaths[f] + 1;
		AddMorale(f, -8);
		QueueEvent(RZ_FactionVoice.Funeral(f, name, place), Math.RandomFloat(600, 1200));
		if (killer > 0)
			RecordMoment(string.Format("the day %1 was shot in %2", name, place));
	}

	//------------------------------------------------------------------------------------------------
	void OnOutpostLost(int f, string place, int by)
	{
		RZ_EpiRecord.Note("post.lost", f, place);
		RZ_EpiRecord.Note("post.won", by, place);
		if (!m_bActive)
			return;

		m_aRecentLosses[f] = m_aRecentLosses[f] + 1;
		AddMorale(f, -10);
		RecordMoment(string.Format("the day %1 lost %2", RZ_Factions.GetShortName(f), place));
		if (Math.RandomFloat01() < 0.6)
			StartRelation(f, by, REL_FEUD);
	}

	//------------------------------------------------------------------------------------------------
	void OnOutpostGained(int f, string place)
	{
		AddMorale(f, 5);
	}

	//------------------------------------------------------------------------------------------------
	void OnContractFinished(RZ_Mission m, bool success)
	{
		if (!m_bActive || !m || !m.m_bContract || !m.m_bHasGiver)
			return;

		// The contracts and errands lane of the season budget (Director/RZ_MajorBudget.c)
		float delta = -1;
		if (success)
			delta = 1;

		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		if (budget)
			delta = budget.TakeMorale(RZ_EBudgetLane.CONTRACTS, m.m_eGiver, delta, "", m.m_sTitle);

		if (delta != 0)
			AddMorale(m.m_eGiver, delta);
	}

	//------------------------------------------------------------------------------------------------
	void OnStageChanged(RZ_EOutbreakStage stage)
	{
		switch (stage)
		{
			case RZ_EOutbreakStage.COLLAPSE: RecordMoment("the week the towns started falling"); break;
			case RZ_EOutbreakStage.OVERRUN: RecordMoment("the day the hordes took the roads"); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A horde sent at a town (at most one remembered every two hours).
	void OnHorde(string place)
	{
		if (!m_bActive || place.IsEmpty() || m_fClock - m_fLastHordeMoment < HORDE_MOMENT_GAP)
			return;

		m_fLastHordeMoment = m_fClock;
		RecordMoment("the horde at " + place);
	}

	//------------------------------------------------------------------------------------------------
	void OnFinalStandLost()
	{
		RecordMoment("the night the lab nearly fell");
		AddMorale(RZ_EFaction.INSTITUTE, -15);
	}

	//------------------------------------------------------------------------------------------------
	void OnBreakthrough(int milestone)
	{
		AddMorale(RZ_EFaction.INSTITUTE, 10);
		if (milestone == 2)
			RecordMoment("the day the lab got the antidote right");
	}

	//------------------------------------------------------------------------------------------------
	void OnCureDone()
	{
		RecordMoment("the day the cure went out");
	}

	// ================================================================ Remembered moments
	//------------------------------------------------------------------------------------------------
	//! A big server moment, as a phrase that fits "Not since %1." Kept (8) and saved.
	void RecordMoment(string phrase)
	{
		if (!m_bActive || phrase.IsEmpty() || m_aMoments.Contains(phrase))
			return;

		m_aMoments.Insert(phrase);
		while (m_aMoments.Count() > MOMENTS_KEPT)
		{
			m_aMoments.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A line that refers to one of the last few moments. speaker -1 = a wanderer. "" if none.
	string MomentReference(int speaker)
	{
		if (!m_bActive || m_aMoments.IsEmpty())
			return "";

		int from = Math.Max(0, m_aMoments.Count() - 4);
		string phrase = m_aMoments[Math.RandomInt(from, m_aMoments.Count())];
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		return RZ_FactionVoice.MomentLine(rng, speaker, phrase);
	}

	// ================================================================ Talk
	//------------------------------------------------------------------------------------------------
	//! Added when you talk to a leader or a named person: what the faction needs (most of the
	//! time) and now and then a big moment they remember.
	string TalkExtra(int f, vector pos)
	{
		if (!m_bActive)
			return "";

		string text = "";
		string need = RZ_FactionVoice.NeedLine(f, GetNeed(f));
		if (!need.IsEmpty() && Math.RandomFloat01() < 0.7)
			text = need;

		if (Math.RandomFloat01() < 0.25)
		{
			string moment = MomentReference(f);
			if (!moment.IsEmpty())
			{
				if (!text.IsEmpty())
					text += " ";

				text += moment;
			}
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Leaders' "Ask for news": the latest quiet headline and a rumour (true or not).
	void AddNewsLines(int speaker, vector from, notnull array<string> lines)
	{
		if (!m_bActive)
			return;

		RZ_Leaders leaders = m_GM.GetLeaders();
		if (leaders && !leaders.GetHeadlines().IsEmpty())
		{
			array<string> headlines = leaders.GetHeadlines();
			lines.Insert("\"Have you heard? " + headlines[headlines.Count() - 1] + "\"");
		}

		string rumour = Rumour(speaker, m_Settings.m_fFalseRumourChance);
		if (!rumour.IsEmpty())
			lines.Insert(rumour);

		// The running story arc, if there is one
		if (m_GM.GetStoryArcs())
		{
			string arc = m_GM.GetStoryArcs().Rumour();
			if (!arc.IsEmpty() && Math.RandomFloat01() < 0.5)
				lines.Insert(arc);
		}

		// ...and the running questlines (Director/RZ_Questlines.c)
		if (m_GM.GetQuestlines() && Math.RandomFloat01() < 0.5)
		{
			string quest = m_GM.GetQuestlines().Rumour();
			if (!quest.IsEmpty())
				lines.Insert(quest);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! For a wanderer's talk: a rumour (less often false) or a remembered moment. "" if none.
	string WandererLine()
	{
		if (!m_bActive)
			return "";

		if (Math.RandomFloat01() < 0.35)
		{
			string moment = MomentReference(-1);
			if (!moment.IsEmpty())
				return moment;
		}

		// The running story arc, now and then, and Haven gossip about hungry safehouses
		if (m_GM.GetStoryArcs() && Math.RandomFloat01() < 0.3)
		{
			string arc = m_GM.GetStoryArcs().Rumour();
			if (!arc.IsEmpty())
				return arc;
		}

		if (m_GM.GetQuestlines() && Math.RandomFloat01() < 0.3)
		{
			string quest = m_GM.GetQuestlines().Rumour();
			if (!quest.IsEmpty())
				return quest;
		}

		if (m_GM.GetHeadlines() && Math.RandomFloat01() < 0.15)
		{
			string haven = m_GM.GetHeadlines().HavenFlavour();
			if (!haven.IsEmpty())
				return "\"" + haven + "\"";
		}

		// A dead character's journal, found on the road (town memory)
		if (m_GM.GetTownMemory() && Math.RandomFloat01() < 0.15)
		{
			string journal = m_GM.GetTownMemory().JournalRumour();
			if (!journal.IsEmpty())
				return journal;
		}

		return Rumour(-1, m_Settings.m_fFalseRumourChance * 0.5);
	}

	// ================================================================ Rumours
	//------------------------------------------------------------------------------------------------
	protected bool AreRivals(int a, int b)
	{
		if (a < 0 || b < 0 || a == b)
			return false;

		if (RZ_Reputation.GetRivalShare(a, b) > 0 || RZ_Reputation.GetRivalShare(b, a) > 0)
			return true;

		// Bastion and Halcyon distrust each other without hurting each other's Fame
		return (a == RZ_EFaction.MILITARY && b == RZ_EFaction.INSTITUTE) || (a == RZ_EFaction.INSTITUTE && b == RZ_EFaction.MILITARY);
	}

	//------------------------------------------------------------------------------------------------
	//! One piece of gossip about another faction, told by `speaker` (-1 a wanderer): its
	//! strength, a town it holds, its need, its truck, or a smear. About a rival it's biased or
	//! false `lieChance` of the time; about anyone else 10% of the time.
	string Rumour(int speaker, float lieChance)
	{
		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		array<int> subjects = {};
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (f == speaker)
				continue;

			subjects.Insert(f);
			if (AreRivals(speaker, f))
				subjects.Insert(f); // rivals are talked about twice as often
		}

		if (subjects.IsEmpty())
			return "";

		int subject = subjects[rng.Int(0, subjects.Count())];
		float chance = 0.1;
		if (AreRivals(speaker, subject))
			chance = Math.Clamp(lieChance, 0, 0.9);

		bool lie = rng.Chance(chance);
		RZ_Territory territory = m_GM.GetTerritory();
		RZ_SupplyLines supply = m_GM.GetSupply();

		array<int> kinds = {0, 1, 2, 3, 4};
		rng.ShuffleInts(kinds);
		foreach (int kind : kinds)
		{
			if (kind == 0 && territory && territory.IsActive())
			{
				bool strong = territory.GetStrength(subject) >= 55;
				if (lie)
					strong = !strong;

				if (strong)
					return RZ_FactionVoice.RumourStrong(subject);

				return RZ_FactionVoice.RumourWeak(subject);
			}

			if (kind == 1 && territory && territory.IsActive())
			{
				array<RZ_Holding> posts = {};
				territory.GetOutposts(subject, posts);
				if (posts.IsEmpty())
					continue;

				RZ_Holding post = posts[rng.Int(0, posts.Count())];
				if (lie)
					return RZ_FactionVoice.RumourLost(subject, post.m_sName);

				return RZ_FactionVoice.RumourHolds(subject, post.m_sName);
			}

			if (kind == 2)
			{
				int need = GetNeed(subject);
				if (lie || need == RZ_ENeed.NONE)
					need = rng.Int(1, 7);

				string needLine = RZ_FactionVoice.RumourNeed(subject, need);
				if (!needLine.IsEmpty())
					return needLine;
			}

			if (kind == 3 && supply)
			{
				string dest = supply.TruckDestination(subject);
				if (dest.IsEmpty())
					continue;

				if (lie)
				{
					RZ_Settlement s = m_GM.GetSettlement(rng.Int(0, m_GM.GetSettlements().Count()));
					if (s && s.m_sName != dest)
						dest = s.m_sName;
				}

				return RZ_FactionVoice.RumourTruck(subject, dest);
			}

			if (kind == 4 && (lie || (AreRivals(speaker, subject) && rng.Chance(0.3))))
				return RZ_FactionVoice.RumourSmear(rng, subject);
		}

		return "";
	}

	// ================================================================ Truces and feuds
	//------------------------------------------------------------------------------------------------
	protected int FindRelation(int a, int b)
	{
		foreach (int i, int ra : m_aRelA)
		{
			int rb = m_aRelB[i];
			if ((ra == a && rb == b) || (ra == b && rb == a))
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The truces and feuds running now, with real minutes left (the standing screen).
	int GetRelations(notnull array<int> a, notnull array<int> b, notnull array<int> kinds, notnull array<float> minutesLeft)
	{
		foreach (int i, int fa : m_aRelA)
		{
			a.Insert(fa);
			b.Insert(m_aRelB[i]);
			kinds.Insert(m_aRelKind[i]);
			minutesLeft.Insert(Math.Max(0, (m_aRelUntil[i] - m_fClock) / 60));
		}

		return m_aRelA.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool HasTruce(int a, int b)
	{
		int i = FindRelation(a, b);
		return i >= 0 && m_aRelKind[i] == REL_TRUCE;
	}

	//------------------------------------------------------------------------------------------------
	bool HasFeud(int a, int b)
	{
		int i = FindRelation(a, b);
		return i >= 0 && m_aRelKind[i] == REL_FEUD;
	}

	//------------------------------------------------------------------------------------------------
	//! Rival Infamy between two factions: halved during a truce, x1.5 during a feud.
	float RivalMult(int a, int b)
	{
		if (!m_bActive)
			return 1;

		if (HasTruce(a, b))
			return 0.5;

		if (HasFeud(a, b))
			return 1.5;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A truce or feud between two factions for 45-90 minutes. Capped: two at once, one per
	//! pair, and a pause after one ends. A feud never starts during their truce.
	void StartRelation(int a, int b, int kind)
	{
		if (!m_bActive || !m_Settings.m_bTrucesAndFeuds || a == b || !RZ_Factions.IsValid(a) || !RZ_Factions.IsValid(b))
			return;

		if (m_aRelA.Count() >= REL_MAX || FindRelation(a, b) >= 0 || m_fClock < m_fRelCooldown)
			return;

		m_aRelA.Insert(a);
		m_aRelB.Insert(b);
		m_aRelKind.Insert(kind);
		m_aRelUntil.Insert(m_fClock + Math.RandomFloat(45, 90) * 60);

		string text = RZ_FactionVoice.FeudStart(a, b);
		if (kind == REL_TRUCE)
			text = RZ_FactionVoice.TruceStart(a, b);

		Headline(text);
	}

	//------------------------------------------------------------------------------------------------
	protected void ExpireRelations()
	{
		for (int i = m_aRelA.Count() - 1; i >= 0; i--)
		{
			if (m_fClock < m_aRelUntil[i])
				continue;

			int a = m_aRelA[i];
			int b = m_aRelB[i];
			string text = RZ_FactionVoice.FeudEnd(a, b);
			if (m_aRelKind[i] == REL_TRUCE)
				text = RZ_FactionVoice.TruceEnd(a, b);

			m_aRelA.Remove(i);
			m_aRelB.Remove(i);
			m_aRelKind.Remove(i);
			m_aRelUntil.Remove(i);
			m_fRelCooldown = m_fClock + REL_COOLDOWN;
			Headline(text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every 30 minutes: maybe a truce between two rivals with low morale or a shared threat
	//! (a bad outbreak), or a feud between two that have been hurting each other.
	protected void MaybeStartRelation()
	{
		if (!m_Settings.m_bTrucesAndFeuds || m_aRelA.Count() >= REL_MAX || Math.RandomFloat01() >= 0.3)
			return;

		array<int> pairA = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS};
		array<int> pairB = {RZ_EFaction.BANDITS, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE, RZ_EFaction.INSTITUTE};
		bool threat = m_GM.GetDirector().GetAverageInfection() >= 50 || m_GM.GetDirector().GetStage() >= RZ_EOutbreakStage.COLLAPSE;

		array<int> truces = {};
		array<int> feuds = {};
		foreach (int i, int a : pairA)
		{
			int b = pairB[i];
			if (FindRelation(a, b) >= 0)
				continue;

			if (threat || (m_aMorale[a] < 45 && m_aMorale[b] < 45))
				truces.Insert(i);

			if (m_aRecentLosses[a] + m_aRecentTrucks[a] + m_aRecentLosses[b] + m_aRecentTrucks[b] >= 1)
				feuds.Insert(i);
		}

		int pick = -1;
		int kind = REL_TRUCE;
		if (!feuds.IsEmpty() && (truces.IsEmpty() || Math.RandomFloat01() < 0.5))
		{
			pick = feuds.GetRandomElement();
			kind = REL_FEUD;
		}
		else if (!truces.IsEmpty())
		{
			pick = truces.GetRandomElement();
		}

		if (pick >= 0)
			StartRelation(pairA[pick], pairB[pick], kind);
	}

	// ================================================================ Life events
	//------------------------------------------------------------------------------------------------
	protected void ScheduleEvent()
	{
		float mean = Math.Max(5, m_Settings.m_fLifeEventMinutes) * 60;
		m_fNextEvent = m_fClock + Math.RandomFloat(mean * 0.7, mean * 1.3);
	}

	//------------------------------------------------------------------------------------------------
	protected void QueueEvent(string text, float delay)
	{
		if (!m_bActive || text.IsEmpty() || m_aPending.Count() >= 4)
			return;

		m_aPending.Insert(text);
		m_aPendingAt.Insert(m_fClock + delay);
	}

	//------------------------------------------------------------------------------------------------
	//! One headline at most: a funeral that's due, else a promotion, recruiting or a researcher
	//! walking out, if the faction's state fits.
	protected void LifeEvent()
	{
		for (int i = 0; i < m_aPending.Count(); i++)
		{
			if (m_fClock < m_aPendingAt[i])
				continue;

			string due = m_aPending[i];
			m_aPending.Remove(i);
			m_aPendingAt.Remove(i);
			Headline(due);
			return;
		}

		RZ_Territory territory = m_GM.GetTerritory();
		array<string> options = {};

		if (territory && territory.IsActive() && m_aMorale[RZ_EFaction.MILITARY] >= 45 && Math.RandomFloat01() < 0.5)
		{
			string oldName;
			string newName;
			string where;
			if (territory.PromoteSomeone(oldName, newName, where))
				options.Insert(RZ_FactionVoice.Promotion(oldName, newName, where));
		}

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (!territory || !territory.IsActive() || territory.GetStrength(f) < 58)
				continue;

			array<RZ_Holding> posts = {};
			territory.GetOutposts(f, posts);
			if (!posts.IsEmpty())
				options.Insert(RZ_FactionVoice.Recruiting(f, posts.GetRandomElement().m_sName));
		}

		if (m_aMorale[RZ_EFaction.INSTITUTE] < 40 && m_GM.HasLab())
		{
			RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
			string place = "the lab";
			if (territory && territory.GetStronghold(RZ_EFaction.INSTITUTE))
				place = territory.GetStronghold(RZ_EFaction.INSTITUTE).m_sName;

			options.Insert(RZ_FactionVoice.Quitting("Dr " + RZ_NoteText.FullName(rng), place));
		}

		if (!options.IsEmpty())
			Headline(options.GetRandomElement());
	}

	//------------------------------------------------------------------------------------------------
	protected void Headline(string text)
	{
		if (text.IsEmpty())
			return;

		RZ_Log.Info("Faction life: " + text);
		if (m_GM.GetLeaders())
			m_GM.GetLeaders().AddHeadline(text, false);
	}

	// ================================================================ Radio chatter
	//------------------------------------------------------------------------------------------------
	//! One faction per round (Bastion, the Red Hand, the Haven, Halcyon), with something true to
	//! say. Its open channel reaches players with a radio: the Haven's everyone, the others
	//! people Known with them, and Radio Operators, who tune rival open channels too.
	protected void Chatter(array<int> ids, array<IEntity> players)
	{
		array<int> order = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.SURVIVORS, RZ_EFaction.INSTITUTE};
		for (int n = 0; n < order.Count(); n++)
		{
			int f = order[(m_iChatterStep + n) % order.Count()];
			string line = BuildChatter(f);
			if (line.IsEmpty())
				continue;

			m_iChatterStep = m_iChatterStep + n + 1;
			SendChatter(f, line, ids, players);
			return;
		}

		m_iChatterStep++;
	}

	//------------------------------------------------------------------------------------------------
	protected void SendChatter(int f, string line, array<int> ids, array<IEntity> players)
	{
		// A one-night mute of a faction's open channel (RZ_MajorRest.Mute)
		if (RZ_MajorRest.Muted(f))
			return;

		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			bool operatorHears = RZ_Radio.IsRadioOperator(m_GM, playerId);
			// A radio next to a running generator (world interactions) pulls in every open channel
			bool boosted = false;
			RZ_Interactions hub = RZ_Interactions.Get();
			if (hub && hub.GetTraps().IsRadioBoosted(p.GetOrigin()))
				boosted = true;

			if (f != RZ_EFaction.SURVIVORS && !operatorHears && !boosted && !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, f), RZ_ETier.KNOWN))
				continue;

			if (!RZ_Radio.HasRadio(m_GM, playerId, p))
				continue;

			m_GM.NotifyPlayer(playerId, RZ_FactionVoice.ChannelTitle(f), line);

			// A Radio Operator near a faction's holding notices its secure traffic (cracked in RZ_SecureRadio)
			if (operatorHears && !GetSecureTraffic(f).IsEmpty() && m_GM.GetTerritory())
			{
				RZ_Holding h = m_GM.GetTerritory().FindHolding(f, p.GetOrigin(), SECURE_RADIUS);
				if (h)
					RZ_Guidance.Report(playerId, "channel");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Something true on a faction's open channel, "" if it has nothing to say.
	protected string BuildChatter(int f)
	{
		// The epilogue week: the faction's open channel talks about the season half the time
		if (RZ_Epilogue.Get())
		{
			string week = RZ_Epilogue.Get().ChatterLine(f);
			if (!week.IsEmpty())
				return week;
		}

		RZ_Territory territory = m_GM.GetTerritory();
		RZ_SupplyLines supply = m_GM.GetSupply();
		array<string> options = {};

		if (supply)
		{
			float secs = supply.SecondsToDeparture(f);
			RZ_Holding top;
			if (territory)
				top = territory.GetStronghold(f);

			int minutes = RZ_Util.RoundToInt(Math.Max(60, secs) / 60);
			if (secs >= 0 && secs <= 1200 && top)
				options.Insert(RZ_FactionVoice.ChatterTruck(f, top.m_sName, minutes));

			string dest = supply.TruckDestination(f);
			if (!dest.IsEmpty())
				options.Insert(RZ_FactionVoice.ChatterTruckOnRoad(f, dest));

			string shortPlace = supply.ShortPlace(f);
			if (!shortPlace.IsEmpty())
				options.Insert(RZ_FactionVoice.ChatterShort(f, shortPlace));

			// The Red Hand watches Bastion's trucks
			if (f == RZ_EFaction.BANDITS)
			{
				string bastionDest = supply.TruckDestination(RZ_EFaction.MILITARY);
				if (!bastionDest.IsEmpty())
					options.Insert(RZ_FactionVoice.RumourTruck(RZ_EFaction.MILITARY, bastionDest));
			}
		}

		if (territory && territory.IsActive())
		{
			array<RZ_Holding> posts = {};
			territory.GetOutposts(f, posts);
			if (!posts.IsEmpty())
			{
				RZ_Holding post = posts.GetRandomElement();
				string clock = territory.NextRoutineClock();
				if (!clock.IsEmpty())
					options.Insert(RZ_FactionVoice.ChatterShift(f, post.m_sName, clock));

				if (f == RZ_EFaction.BANDITS)
					options.Insert(RZ_FactionVoice.ChatterToll(post.m_sName, RZ_GameTime.IsNight()));
			}
		}

		if (f == RZ_EFaction.INSTITUTE && m_GM.HasLab() && !m_GM.GetCure().IsCured())
		{
			RZ_Settlement lab = m_GM.GetSettlement(m_GM.GetLabSettlement());
			string labPlace = "the lab";
			if (lab)
				labPlace = lab.m_sName;

			options.Insert(RZ_FactionVoice.ChatterSamples(RZ_Util.RoundToInt(m_GM.GetCure().m_fProgress), labPlace));
		}

		if (options.IsEmpty())
			return "";

		// A tail from the voice bank's open-channel pool (one pick for everyone who hears it)
		string line = options.GetRandomElement();
		string tail = RZ_VoiceBank.PickShared(m_GM, "radio_tail", f);
		if (!tail.IsEmpty())
			line += " " + tail;

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! One line on faction f's open channel now, to everyone who hears it (a caught impostor, never
	//! named; the parley's word). Whoever hears that channel hears it, as chatter.
	void OpenChannelLine(int f, string line)
	{
		if (line.IsEmpty())
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		SendChatter(f, line, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A line on faction f's open channel right now, "" if it has nothing to say (house radios).
	string ChatterFor(int f)
	{
		if (!m_bActive)
			return "";

		return BuildChatter(f);
	}

	//------------------------------------------------------------------------------------------------
	//! What a faction says where it thinks nobody can hear: Bastion's command net, Halcyon's lab
	//! channel, the Red Hand boss's frequency. Cracked by Radio Operators (RZ_SecureRadio).
	//! Patrol and shift times, truck departures and routes, samples wanted. "" if nothing.
	string GetSecureTraffic(int f)
	{
		if (!m_bActive || f == RZ_EFaction.SURVIVORS)
			return "";

		array<string> options = {};
		RZ_SupplyLines supply = m_GM.GetSupply();
		RZ_Territory territory = m_GM.GetTerritory();
		if (m_GM.GetErrands())
			m_GM.GetErrands().GetFaults().SecureLines(f, options);

		// The majors' secure lines for Radio Operators (RZ_MajorRest.SecureLine)
		RZ_MajorRest.SecureLines(f, options);

		if (supply)
		{
			string dest = supply.TruckDestination(f);
			if (!dest.IsEmpty())
				options.Insert(SecureTruck(f, dest));

			float secs = supply.SecondsToDeparture(f);
			if (secs >= 0)
				options.Insert(SecureDeparture(f, RZ_Util.RoundToInt(Math.Max(60, secs) / 60)));

			string shortPlace = supply.ShortPlace(f);
			if (!shortPlace.IsEmpty())
				options.Insert(SecureShort(f, shortPlace));
		}

		if (territory && territory.IsActive())
		{
			array<RZ_Holding> posts = {};
			territory.GetOutposts(f, posts);
			string clock = territory.NextRoutineClock();
			if (!posts.IsEmpty() && !clock.IsEmpty())
				options.Insert(SecurePatrol(f, posts.GetRandomElement().m_sName, clock));
		}

		if (f == RZ_EFaction.INSTITUTE && m_GM.HasLab() && !m_GM.GetCure().IsCured())
			options.Insert(SecureSamples());

		if (f == RZ_EFaction.BANDITS && supply)
		{
			string bastionDest = supply.TruckDestination(RZ_EFaction.MILITARY);
			if (!bastionDest.IsEmpty())
				options.Insert(string.Format("\"Bastion's truck is on the %1 road. Nobody touches it till it's past the bridge. Then it's ours.\"", bastionDest));
		}

		if (options.IsEmpty())
			return "";

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	protected string SecureTruck(int f, string dest)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("\"Vehicle to %1, Command. Crew of two. If contact, hold position. We'll come and get you.\"", dest);
			case RZ_EFaction.BANDITS: return string.Format("\"Boss, the truck's going to %1. Two lads in it. If anyone stops it, I want names.\"", dest);
			case RZ_EFaction.INSTITUTE: return string.Format("\"Courier vehicle to %1. Cold boxes on board. Do not open them for anyone.\"", dest);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string SecureDeparture(int f, int minutes)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("\"Logistics, Command. Next run departs in %1 minutes. Route to follow. Do not broadcast it in the clear.\"", minutes);
			case RZ_EFaction.BANDITS: return string.Format("\"Truck goes out in %1 minutes. Keep the road clear and your mouths shut.\"", minutes);
			case RZ_EFaction.INSTITUTE: return string.Format("\"The next transfer leaves in %1 minutes. Samples first, people second. I'm sorry, but that's the order.\"", minutes);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string SecureShort(int f, string place)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("\"%1 is dry. Requisitions suspended until resupply. Do not tell the civilians.\"", place);
			case RZ_EFaction.BANDITS: return string.Format("\"%1's empty. Double the prices till the truck comes. Nobody's going anywhere else.\"", place);
			case RZ_EFaction.INSTITUTE: return string.Format("\"%1 is out of stock. We are rationing reagents. Note it in the log.\"", place);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected string SecurePatrol(int f, string place, string clock)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("\"All stations: %1 rotates the watch at %2. Two on the gate, the rest stood down. Ten minutes where nobody's looking.\"", place, clock);
			case RZ_EFaction.BANDITS: return string.Format("\"%1 changes over at %2. The new lot are always late. Don't be.\"", place, clock);
			case RZ_EFaction.INSTITUTE: return string.Format("\"The station at %1 rotates at %2. The guards don't check bags at changeover.\"", place, clock);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! How many samples the next breakthrough needs, roughly.
	protected string SecureSamples()
	{
		RZ_CureResearch cure = m_GM.GetCure();
		if (cure.m_iMilestone >= 4)
			return "\"Director, lab. Every result is in. Now we have to hold this building long enough to finish. Pray they don't hear us.\"";

		// Samples won't help at the finale's gate, and a full cold store turns them away
		if (cure.IsAtFinaleGate())
			return "\"Director, lab. Ninety percent, and that's where the samples stop helping. The last step isn't in the cold store. Don't put that in the log.\"";

		if (cure.IsLabFull())
			return string.Format("\"Director, lab. We are %1 percent there and the cold store is full. Hold the samples until we've worked through it. Don't put that in the log.\"", RZ_Util.RoundToInt(cure.m_fProgress));

		// What the next result still needs beyond what already waits in the cold store
		float next = (cure.m_iMilestone + 1) * 25;
		float per = Math.Max(0.01, cure.SampleWorth(0, m_GM.GetResearchPerSample()));
		float missing = next - cure.m_fProgress - cure.GetBacklog();
		if (missing <= 0)
			return string.Format("\"Director, lab. We are %1 percent there, and everything for the next result is in the cold store. It's hours now, not samples. Don't put that in the log.\"", RZ_Util.RoundToInt(cure.m_fProgress));

		int needed = Math.Max(1, RZ_Util.RoundToInt(Math.Ceil(missing / per)));
		return string.Format("\"Director, lab. We are %1 percent there. Roughly %2 more samples for the next result. Whatever it costs. Don't put that in the log.\"", RZ_Util.RoundToInt(cure.m_fProgress), needed);
	}
}
