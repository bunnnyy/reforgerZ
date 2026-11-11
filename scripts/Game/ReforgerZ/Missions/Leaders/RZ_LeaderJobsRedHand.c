//! Red Hand leader jobs (docs/MISSIONS.md, "Leader missions"): from crew bosses who run their
//! own crew (Independent) or are working against the boss (Rogue).

//------------------------------------------------------------------------------------------------
//! Independent. Hold this crew's crossroads outside the camp for five minutes, waves and all.
class RZ_Job_OurCrossroads : RZ_LeaderHold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HARD, 6, 20);
		m_fHoldNeeded = 300;
		m_fHoldRadius = 30;
		m_fWaveInterval = 70;
		m_iMaxWaves = 3;
		m_fWaveTimer = 40;
		m_fParticipationRadius = 50;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_iSettlement = gm.FindNearestSettlement(l.m_vPos);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius + 50, s.m_fRadius + 250);
		m_sTitle = "Our Crossroads";
		m_sBrief = string.Format("The crossroads outside %1 is ours. Not the boss's. Ours. Somebody's been parking on it. Stand there for five minutes and let the dead and the living both see who owns it.",
			s.m_sName);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. A crew member ran off with the boss's watch and the crew boss's boots. Take
//! it back (loud), talk it back (60%, always for a Talker boss), or let him keep it (fails).
class RZ_Job_HonourAmongThieves : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;
	protected string m_sThief;
	protected bool m_bTalker;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_TALK, 6, 25);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_bTalker = l.HasTrait(RZ_ELeaderTrait.TALKER);
		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 800, 3000, 10, 85);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.3, s.m_fRadius * 0.9);
		m_sThief = RZ_RedHandEdges.DebtorName(m_Rng);
		m_iTalk = AddClue(m_sThief, string.Format("\"Did %1 send you? I figured. Look, the boss owed me three months. I took three months' worth.\" He's wearing very good boots.", l.m_sShort));
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", "Take it back|Talk it back|Let him keep it", m_iTalk, "Talk to " + m_sThief);

		m_sTitle = "Honour Among Thieves";
		m_sBrief = string.Format("%1 was one of mine. Took the boss's watch and my good boots, and walked. He's in %2. I don't need him dead. I need the boots.", m_sThief, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		ChosenBy(playerId);
		if (option == 0)
		{
			gm.GetHeat().AddHeat(m_vPos, 15);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("It isn't pretty and it isn't quiet. %1 ends up on the floor in his socks. You walk out with a watch and a pair of boots.", m_sThief));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			if (m_bTalker || Math.RandomFloat01() < 0.6)
			{
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("You talk about what he owes, who he owes it to, and who'd be sent next time. %1 sighs and unlaces the boots.", m_sThief));
				m_eState = RZ_EMissionState.SUCCEEDED;
				return;
			}

			gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 nods along, says he'll think about it, and goes out the back window while you're still talking.", m_sThief));
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (option == 2)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You tell him to keep them. He'll need them where he's going. The crew boss won't see it that way.");
			m_eState = RZ_EMissionState.FAILED;
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. This crew's own fight pit behind the garages: four waves, bets paid in Favor.
class RZ_Job_HouseRules : RZ_LeaderHold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_HARD, 4, 35);
		m_fHoldNeeded = 200;
		m_fHoldRadius = 25;
		m_fWaveInterval = 45;
		m_iMaxWaves = 4;
		m_fWaveDistance = 150;
		m_fWaveTimer = 40;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_vPos = LandPointNear(l.m_vPos, 60, 150);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "House Rules";
		m_sBrief = "The boss runs the big pit. This one's ours: our rules, our bets, our cut. Get in the ring behind the garages and stay standing. Win and I'll pay out of the pot.";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnWave(RZ_GameModeComponent gm, int wave)
	{
		NotifyParticipants(gm, m_sTitle, string.Format("Round %1. Somebody's put money on you. Somebody else has put more against.", wave));
	}
}

//------------------------------------------------------------------------------------------------
//! Independent. Collect a debt: talk him round (60% pay), threaten him (full pay; if he's a
//! Haven wanderer, Haven Infamy and he remembers), or pay it off yourself (15 Red Hand Favor).
//! Uses a real Trader, Scavenger or Courier wanderer when one is free.
class RZ_Job_DebtCollection : RZ_LeaderPuzzle
{
	protected int m_iTalk = -1;
	protected int m_iWanderer = -1;
	protected string m_sDebtor;

	protected static const int PAY_FAVOR = 15;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.INDEPENDENT, RZ_Leaders.TONE_TALK | RZ_Leaders.TONE_DARK, 6, 25);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		m_iSettlement = PickSettlementAway(gm, l.m_vPos, 600, 3000, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = LandPointNear(s.m_vCenter, s.m_fRadius * 0.2, s.m_fRadius * 0.7);
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (wanderers)
		{
			int mask = RZ_Wanderers.RoleBit(RZ_EWandererRole.TRADER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.SCAVENGER) | RZ_Wanderers.RoleBit(RZ_EWandererRole.COURIER);
			m_iWanderer = wanderers.FindForMission(mask);
		}

		if (m_iWanderer >= 0)
		{
			wanderers.Hold(m_iWanderer, m_iId);
			m_sDebtor = wanderers.GetName(m_iWanderer);
		}
		else
		{
			m_sDebtor = RZ_NoteText.FullName(m_Rng);
		}

		m_iTalk = AddClue(m_sDebtor, "\"Forty tins. I know. I had forty tins, and then I had a daughter with a fever and a doctor who wanted forty tins. You can see how the arithmetic went.\"");
		string labels = string.Format("Talk %1 round|Threaten %1 [Threaten - +3 Infamy]|Pay what %1 owes (%2 Red Hand Favor)", m_sDebtor, PAY_FAVOR);
		SpawnObject(gm, RZ_EPuzzleProp.CIVILIAN, m_vPos, 0, "", labels, m_iTalk, "Talk to " + m_sDebtor);

		m_sTitle = "Debt Collection";
		m_sBrief = string.Format("%1 owes us forty tins and a favour. They're in %2. Talk, threaten, pay it off yourself. I don't care which, as long as the book balances.", m_sDebtor, s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != 0)
			return;

		if (option == 2 && !gm.SpendFavor(playerId, RZ_EFaction.BANDITS, PAY_FAVOR))
		{
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("The debt is %1 Red Hand Favor. You have %2.", PAY_FAVOR, gm.GetProfile(playerId).GetFavor(RZ_EFaction.BANDITS)));
			return;
		}

		ChosenBy(playerId);
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (option == 0)
		{
			m_fRewardScale = 0.6;
			RememberDebtor(wanderers, playerId, RZ_Wanderers.MEM_HELPED);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You agree on half now and half when the fever's gone. %1 shakes your hand like you've done them a kindness. The crew boss pays you like you've done half a job.", m_sDebtor));
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 1)
		{
			// The one Fear check: a wanderer debtor's Infamy is the Haven's; the threat itself costs +3
			int listener = -1;
			if (m_iWanderer >= 0)
				listener = RZ_EFaction.SURVIVORS;

			RZ_ECheck fear = RZ_Checks.Threaten(gm, playerId, "debt." + m_sDebtor, listener, RZ_EFaction.BANDITS, 1, false);
			if (fear == RZ_ECheck.REFUSED)
			{
				gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1 just looks at you. \"Go on, then. I've got nothing left to take.\" That won't do it.", m_sDebtor));
				return;
			}

			RememberDebtor(wanderers, playerId, RZ_Wanderers.MEM_WRONGED);
			string text = string.Format("You explain what happens to people who don't pay. %1 finds the tins. All forty.", m_sDebtor);
			if (m_iWanderer >= 0)
				text += " Every safehouse on their route will hear how you said it.";

			gm.NotifyPlayer(playerId, m_sTitle, text);
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		if (option == 2)
		{
			RememberDebtor(wanderers, playerId, RZ_Wanderers.MEM_HELPED);
			gm.NotifyPlayer(playerId, m_sTitle, string.Format("You square it out of your own pocket. %1 stares at you, then at the door, then at you. The book balances.", m_sDebtor));
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RememberDebtor(RZ_Wanderers wanderers, int playerId, int flag)
	{
		if (wanderers && m_iWanderer >= 0)
			wanderers.RememberPlayer(m_iWanderer, playerId, flag);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (wanderers && m_iWanderer >= 0)
			wanderers.Release(m_iWanderer, m_vPos);

		m_iWanderer = -1;
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Secret medicine to a Haven safehouse: its town's infection -5 and Haven Fame.
//! The boss would call it charity.
class RZ_Job_SoftTouch : RZ_LeaderDelivery
{
	protected int m_iHouse = -1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_HOPEFUL | RZ_Leaders.TONE_CROSS, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.SURVIVORS) || RZ_LeaderJob.SafehouseNear(gm, leader.m_vPos, 300, 6000, -1) < 0)
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		m_iHouse = RZ_LeaderJob.SafehouseNear(gm, l.m_vPos, 300, 6000, -1);
		RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
		if (!h || !SpawnItemAtSeat(gm))
			return false;

		if (!AddDrop(gm, RZ_EFaction.SURVIVORS, LandPointNear(h.m_vPos, 3, 8), "the Haven safehouse in " + h.m_sName))
			return false;

		AddExposeDrop(gm);
		m_sTitle = "The Soft Touch";
		m_sBrief = string.Format("Don't laugh. There's a safehouse in %1 with sick kids and no medicine, and I've got medicine. The boss would call it charity. The boss would call it a lot of things. Get it there, and keep your mouth shut.",
			h.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		RZ_Safehouse h = gm.GetSafehouses().Get(m_iHouse);
		if (h)
			RZ_LeaderJob.TownInfection(gm, h.m_vPos, -5);

		gm.NotifyPlayer(playerId, m_sTitle, "The medic opens the box and goes very quiet. \"Who sent this?\" You tell her nobody did.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.SURVIVORS, 5);
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Carry a word to two other crews (other Red Hand posts if there are any, else
//! camps outside two towns). It never fully works: half the time the boss laughs it off
//! (the crew boss is pulled back toward the line), half the time there's a purge (the crew
//! boss is discovered and everyone who carried the word takes Red Hand Infamy).
class RZ_Job_CoupTalk : RZ_LeaderVisit
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK | RZ_Leaders.TONE_TALK, 0, 25);
		m_bRogueJob = false; // its own ending instead of the usual discovery roll
		m_fVisitRadius = 20;
		m_fVisitSeconds = 20;
		m_sPointName = "Crew";
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		array<string> names = {};
		array<RZ_Leader> crews = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.BANDITS, crews);
		foreach (RZ_Leader c : crews)
		{
			if (c == l || c.IsStronghold() || m_aPoints.Count() >= 2)
				continue;

			AddPoint(LandPointNear(c.m_vPos, 5, 12));
			names.Insert(c.m_sPlace);
		}

		array<int> used = {};
		for (int tries = 0; tries < 6 && m_aPoints.Count() < 2; tries++)
		{
			int town = PickSettlementAway(gm, l.m_vPos, 800, 3000, 15, 90);
			RZ_Settlement s = gm.GetSettlement(town);
			if (!s || used.Contains(town))
				continue;

			used.Insert(town);
			AddPoint(LandPointNear(s.m_vCenter, s.m_fRadius, s.m_fRadius + 60));
			names.Insert(s.m_sName);
		}

		if (m_aPoints.Count() < 2)
			return false;

		m_vPos = m_aPoints[0];
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "Coup Talk";
		m_sBrief = string.Format("The boss is old and slow, and the other crews know it. Carry a word to the crews outside %1 and %2. Just a word. Stay long enough to see who listens.", names[0], names[1]);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return;

		if (Math.RandomFloat01() < 0.5)
		{
			gm.GetLeaders().ShiftLoyalty(l, 15, true);
			NotifyParticipants(gm, m_sTitle, string.Format("The other crews laugh. One of them tells the boss, who laughs too, and sends %1 a bottle with a note: \"Nice try.\" It stings worse than a beating.", l.m_sShort));
			return;
		}

		gm.GetLeaders().Discover(l, m_aParticipants, string.Format("The boss of the Red Hand has heard who's been talking about a new boss. %1's crew in %2 is being purged.", l.m_sName, l.m_sPlace));
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. A map to a rival crew's camp, for Bastion. If there's another Red Hand crew boss
//! (not the boss), Bastion raids them: they're killed and replaced later. Small Bastion Fame.
class RZ_Job_SellOutRivalCrew : RZ_LeaderDelivery
{
	protected int m_iRival = -1;
	protected string m_sRivalPlace;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK | RZ_Leaders.TONE_CROSS, 0, 30);
	}

	//------------------------------------------------------------------------------------------------
	override float GetLeaderWeight(RZ_GameModeComponent gm, RZ_Leader leader)
	{
		if (!CanRun(gm, RZ_EFaction.MILITARY))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupJob(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		vector meet;
		string meetName;
		if (!RZ_LeaderJob.MeetingPoint(gm, RZ_EFaction.MILITARY, l.m_vPos, meet, meetName))
			return false;

		// Another crew boss to sell out, if there is one no mission or story is using (else the raid
		// couldn't touch them); otherwise squatters in a rough town
		array<RZ_Leader> crews = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.BANDITS, crews);
		foreach (RZ_Leader c : crews)
		{
			if (c != l && !c.IsStronghold() && gm.GetFacts().LeaderFree(c.m_iId))
			{
				m_iRival = c.m_iId;
				m_sRivalPlace = c.m_sPlace;
				break;
			}
		}

		if (m_iRival < 0)
		{
			RZ_Settlement s = gm.GetSettlement(PickSettlementAway(gm, l.m_vPos, 800, 4000, 15, 95));
			if (!s)
				return false;

			m_sRivalPlace = s.m_sName;
		}

		if (!SpawnItemAtSeat(gm) || !AddDrop(gm, RZ_EFaction.MILITARY, meet, meetName))
			return false;

		AddExposeDrop(gm);
		m_sTitle = "Sell Out a Rival Crew";
		m_sBrief = string.Format("The crew in %1 has been taking from my roads. Bastion would love to know where they sleep. Here's a map. Walk it to %2. Nobody ever has to know who drew it.",
			m_sRivalPlace, meetName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnJobDelivered(RZ_GameModeComponent gm, int playerId, int dropIndex)
	{
		// The rival may have been taken up by a story since (KillOffscreen refuses then): the raid
		// finds the camp empty instead of a death nobody hears about
		bool missed = m_iRival >= 0 && !gm.GetLeaders().KillOffscreen(m_iRival, string.Format("was killed when Bastion raided the camp at %1", m_sRivalPlace));
		string text = string.Format("A lieutenant studies the map, then you. \"Tonight,\" is all he says. You hear the trucks leave for %1 an hour later.", m_sRivalPlace);
		if (missed)
			text += " They come back before dawn, empty-handed. Whoever slept there had moved on.";

		gm.NotifyPlayer(playerId, m_sTitle, text);
	}

	//------------------------------------------------------------------------------------------------
	override void OnRewardPlayer(RZ_GameModeComponent gm, int playerId)
	{
		if (!m_bExposed)
			RZ_LeaderJob.CrossFame(gm, playerId, RZ_EFaction.MILITARY, 4);
	}
}

//------------------------------------------------------------------------------------------------
//! Rogue. Take the boss's toll on the road out of town for five minutes and keep it for the
//! crew boss. The travellers are Haven people: a little Haven Infamy.
class RZ_Job_SkimTheToll : RZ_LeaderHold
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetLeaderJob(RZ_EFaction.BANDITS, RZ_ELoyalty.ROGUE, RZ_Leaders.TONE_DARK, 0, 30);
		SetRival(RZ_EFaction.SURVIVORS, 3);
		m_fHoldNeeded = 300;
		m_fHoldRadius = 25;
		m_fHeatPerSecond = 0.1;
		m_fParticipationRadius = 40;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		RZ_Leader l = GetLeader(gm);
		if (!l)
			return false;

		int town = PickSettlementAway(gm, l.m_vPos, 800, 3000, 0, 90);
		RZ_Settlement s = gm.GetSettlement(town);
		if (!s)
			return false;

		// On the road out, about a third of the way to the next town
		m_vPos = LandPointNear(l.m_vPos + (s.m_vCenter - l.m_vPos) * 0.35, 0, 40);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "Skim the Toll";
		m_sBrief = string.Format("The boss takes the toll on the road to %1. Today the boss is short-handed and I'm not. Stand on the road for five minutes and collect. What the boss doesn't count, the boss doesn't miss.",
			s.m_sName);
		return true;
	}
}
