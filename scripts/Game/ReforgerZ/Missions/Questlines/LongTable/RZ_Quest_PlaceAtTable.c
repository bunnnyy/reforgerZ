//! C2. A Place at the Table (the Long Table; docs/QUESTLINES.md, docs/MISSIONS.md "Questlines").
//! The darkest line in the mod, and personal: the host makes his offer to one character, on the
//! third meal of a life (World/RZ_LongTable.c, MakeOffer), and only that character can do its
//! stages. A player is never the one taken, and nothing is described: it's all said sideways.
//!
//! 1. The dead: stand where someone died to the infected in the last hour (a wanderer the house
//!    heard of, a truck crew, or else a traveller the mission lays in a ditch), then tell the host
//!    (place.dead, yours). A bowl and true road news: where the next truck is going.
//! 2. The dying (after a wait): a bitten wanderer on the road. Tell them where the farm is (they
//!    never arrive; place.dying, yours), send them the wrong way (the usual bitten wanderer, and the
//!    host is cooler with you), or give them your antidote.
//! 3. A stray (after a wait): an exile the Red Hand keeps track of. Bring him to the farm (an
//!    escort; place.stray, yours), walk him to a safehouse or a Bastion seat instead
//!    (place.undercover), or refuse the host at the gate: the guest rule holds in daylight now.
//! 4. Grace (after a wait, in the evening): sit at the long table and say a name (place.family),
//!    get up and go with the proof (place.proof: C1 at "Who to tell"), or fight, at the table.
//!
//! Endings: Family, Undercover, Walked away; and the house broken at the table (C1's "by your own
//! hand", reached here). ★ Police: walking away from the host at stage 1 takes the proof with you.
class RZ_Quest_PlaceAtTable : RZ_Questline
{
	static const int S_DEAD = 0;
	static const int S_DYING = 1;
	static const int S_STRAY = 2;
	static const int S_GRACE = 3;

	static const int OUT_TOLD = 10;			// the dead: told the host
	static const int OUT_WALKED = 11;		// walked away from him
	static const int OUT_SENT = 12;			// the dying: told him where the farm is
	static const int OUT_LIED = 13;			// sent him the wrong way
	static const int OUT_SAVED = 14;		// gave him an antidote
	static const int OUT_BROUGHT = 15;		// the stray: brought to the farm
	static const int OUT_UNDERCOVER = 16;	// walked somewhere safe instead
	static const int OUT_REFUSED = 17;		// refused the host
	static const int OUT_SAT = 18;			// grace: sat and said the name
	static const int OUT_LEFT = 19;			// got up and went, with the proof
	static const int OUT_FOUGHT = 20;		// fought them at the table, and the house is finished

	static const int E_FAMILY = 0;
	static const int E_UNDERCOVER = 1;
	static const int E_WALKED_AWAY = 2;
	static const int E_BY_HAND = 3;

	//------------------------------------------------------------------------------------------------
	override void Define()
	{
		Info(RZ_LongTable.C2_KEY, "A Place at the Table", -1);
		Slot(RZ_LongTable.SLOT);

		RZ_QuestStage dead = MissionStage(S_DEAD, "The dead", "{host} says the dead don't need what they have. Find where someone has died to the infected, before anyone else does, stand there, then go back and tell him where.");
		dead.GiveUp(4, "{fizzle}");
		dead.Visits(2);

		RZ_QuestStage dying = MissionStage(S_DYING, "The dying", "{host} says there's one on the road who won't make it. Bitten. \"Tell them where we are. We'll look after them.\"");
		dying.Wait(20, 6);
		dying.GiveUp(6, "{fizzle}");
		dying.Visits(2);

		RZ_QuestStage stray = MissionStage(S_STRAY, "A stray", "{host} wants a living one now: {stray}, an exile the Red Hand keeps track of, sleeping rough near {stray_town}. \"Bring him to supper.\" Or don't.");
		stray.Wait(20, 6);
		stray.GiveUp(6, "{fizzle}");
		stray.Visits(2);

		RZ_QuestStage grace = MissionStage(S_GRACE, "Grace", "{mother} has set a place for you at the long table, at the head of it. \"Grace tonight is yours. Say a name.\"");
		grace.Wait(20, 6);
		grace.GiveUp(8, "{fizzle}");
		grace.Visits(2);
	}

	// ================================================================ The house and the guest
	//------------------------------------------------------------------------------------------------
	protected static RZ_LongTable Table(RZ_GameModeComponent gm)
	{
		return gm.GetLongTable();
	}

	//------------------------------------------------------------------------------------------------
	//! The character the offer was made to, if they're online in the same life. -1 otherwise.
	static int Guest(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int playerId = gm.GetQuestlines().OnlineId(run.Text("ident"));
		if (playerId <= 0 || gm.GetProfile(playerId).m_iCharacter != run.Flag("life"))
			return -1;

		return playerId;
	}

	//------------------------------------------------------------------------------------------------
	//! The offer is over: the house is gone or found out, or the guest's life ended.
	protected static bool Over(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || t.IsExposed())
			return true;

		RZ_PlayerProfile profile = gm.FindProfileByIdentity(run.Text("ident"));
		return !profile || profile.m_iCharacter != run.Flag("life");
	}

	//------------------------------------------------------------------------------------------------
	protected void RefreshTexts(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		if (!t)
			return;

		if (t.Exists())
		{
			run.SetText("town", t.GetTownName());
			run.SetText("family", t.GetSurname());
			run.SetPlace(t.GetHousePos(), t.FarmTitle());
		}

		string mother = t.NameOf(RZ_ETableRole.MOTHER);
		if (mother.IsEmpty())
			mother = "the father";

		run.SetText("mother", mother);
		run.SetText("host", t.GreeterName());
		run.SetText("fizzle", FizzleText(gm, run));
	}

	//------------------------------------------------------------------------------------------------
	protected string FizzleText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		string town = run.Text("town");
		if (!t || !t.Exists())
			return string.Format("There's nobody at the farm past %1 any more. The place they set for you is gone with them.", town);

		if (t.IsExposed())
			return string.Format("Everyone knows about the farm past %1 now. Nobody meets anyone on the road there any more.", town);

		RZ_PlayerProfile profile = gm.FindProfileByIdentity(run.Text("ident"));
		if (!profile || profile.m_iCharacter != run.Flag("life"))
			return string.Format("The farm past %1 set a place for you a few nights running. Then they stopped.", town);

		return string.Format("%1 stopped meeting you on the road. There's still a place set at the farm past %2, for somebody.", run.Text("host"), town);
	}

	// ================================================================ The questline
	//------------------------------------------------------------------------------------------------
	//! Only when the host makes his offer (RZ_LongTable.MakeOffer): to someone who has eaten with
	//! them twice this life and come back, who isn't Trusted with the Haven.
	override bool CanStart(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RZ_LongTable t = Table(gm);
		if (!t || t.GetState() != RZ_ETableState.ACTIVE)
			return false;

		if (!RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !RZ_PuzzleMission.HasPrefab(gm, RZ_EPuzzleProp.CIVILIAN))
			return false;

		string ident = t.GetOfferIdent();
		int playerId = gm.GetQuestlines().OnlineId(ident);
		if (playerId <= 0 || t.Meals(playerId) < 2)
			return false;

		if (RZ_Reputation.GetAccessRank(gm.GetTier(playerId, RZ_EFaction.SURVIVORS)) >= 2)
			return false;

		if (t.Remembers(playerId, RZ_LongTable.MEM_FAMILY | RZ_LongTable.MEM_WALKED | RZ_LongTable.MEM_CROSSED))
			return false;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		run.SetText("ident", ident);
		run.SetFlag("life", gm.GetProfile(playerId).m_iCharacter);
		run.SetText("guest", RZ_Names.Public(playerId));
		run.SetText("stray", RZ_LongTable.ManName(rng));
		run.SetText("stray_town", t.GetTownName());
		run.SetText("name", t.ShortLost());
		run.SetText("name_from", t.GetLostFrom());
		RefreshTexts(gm, run);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBegin(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		int playerId = Guest(gm, run);
		run.Did(gm, playerId, RZ_EQuestApproach.NONE, "was made an offer at the gate");
		run.Note("{host} made you an offer at the gate. The dead don't need what they have. Tell the family where the dead are, before anyone else finds them, and the family will feed you and tell you what the roads are doing.");
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageBegin(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RefreshTexts(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	//! The guest online in the same life, the house open and not found out. Grace is in the evening.
	override bool IsReady(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		RefreshTexts(gm, run);
		if (Over(gm, run))
			return true;

		if (Guest(gm, run) <= 0)
			return false;

		if (stage == S_GRACE)
		{
			float hour = RZ_GameTime.GetHour();
			if (hour >= 0 && !RZ_GameTime.IsWithin(hour, 17, 5))
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override RZ_Mission MakeMission(RZ_GameModeComponent gm, RZ_QuestRun run, int stage)
	{
		if (Over(gm, run))
			return new RZ_QM_TableGone();

		switch (stage)
		{
			case S_DEAD: return new RZ_QM_PlaceDead();
			case S_DYING: return new RZ_QM_PlaceDying();
			case S_STRAY: return new RZ_QM_PlaceStray();
			case S_GRACE: return new RZ_QM_PlaceGrace();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override int Next(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, int outcome)
	{
		RefreshTexts(gm, run);
		if (outcome == OUT_FOUGHT)
			return Finish(run, E_BY_HAND, "You turned the long table over. The {family} farm is finished, by your own hand.");

		if (Over(gm, run))
			return Fizzle(run, "{fizzle}");

		if (outcome == OUT_WALKED || outcome == OUT_REFUSED || outcome == OUT_LEFT)
			return Finish(run, E_WALKED_AWAY, "Walked away. {host} won't meet you on the road again.");

		switch (stage)
		{
			case S_DEAD:
			{
				if (outcome == OUT_TOLD || outcome == OUTCOME_DONE)
				{
					run.Note("You told {host} where the dead were, before anyone else found them. There was a bowl, and the road news, and none of it was a lie.");
					return S_DYING;
				}

				run.Note("Nobody found the dead in time. {host} is patient.");
				return S_DEAD;
			}
			case S_DYING:
			{
				if (outcome == OUT_SENT || outcome == OUTCOME_DONE)
					run.Note("You told the bitten wanderer where the farm is, and were thanked for it. They never arrived anywhere.");
				else if (outcome == OUT_LIED)
					run.Note("You sent the bitten wanderer the wrong way. {host} knows. He's a little cooler with you now.");
				else if (outcome == OUT_SAVED)
					run.Note("You gave the bitten wanderer your antidote, and they walked on. {host} didn't say anything about it at all.");
				else
					run.Note("Nobody went to the bitten wanderer on the road. {host} says it doesn't matter. It was a kindness, that's all.");

				if (outcome != OUT_SENT && outcome != OUTCOME_DONE)
					run.SetFlag("cool", 1);

				return S_STRAY;
			}
			case S_STRAY:
			{
				if (outcome == OUT_BROUGHT || outcome == OUTCOME_DONE)
				{
					run.SetText("name", run.Text("stray"));
					run.SetText("name_from", run.Text("stray_town"));
					run.Note("You walked {stray} to the farm. The door shut behind him. Somebody inside was singing.");
					return S_GRACE;
				}

				if (outcome == OUT_UNDERCOVER)
					return Finish(run, E_UNDERCOVER, "Undercover. {stray} is safe, and you know everything you need to about the {family} farm. Take it to someone.");

				run.Note("{stray} wasn't brought anywhere. {host} will ask again.");
				return S_STRAY;
			}
			case S_GRACE:
			{
				if (outcome == OUT_SAT || outcome == OUTCOME_DONE)
					return Finish(run, E_FAMILY, "Family. You said the name at grace, and the {family} farm keeps a bowl for you now.");

				run.Note("The place at the head of the table stayed empty tonight.");
				return S_GRACE;
			}
		}

		return Fizzle(run, "{fizzle}");
	}

	//------------------------------------------------------------------------------------------------
	//! Through the house's memory (World/RZ_LongTable.c) and season-wide facts (place.*).
	override void OnEnding(RZ_GameModeComponent gm, RZ_QuestRun run, int ending)
	{
		RZ_LongTable t = Table(gm);
		RZ_Questlines q = gm.GetQuestlines();
		int playerId = Guest(gm, run);
		switch (ending)
		{
			case E_FAMILY:
			{
				if (t && playerId > 0)
					t.Remember(playerId, RZ_LongTable.MEM_FAMILY);

				q.SetWorldFlag("place.family", 1);
				return;
			}
			case E_UNDERCOVER:
			{
				if (t && playerId > 0)
					t.Remember(playerId, RZ_LongTable.MEM_UNDERCOVER | RZ_LongTable.MEM_CROSSED | RZ_LongTable.MEM_KNOWS);

				q.SetWorldFlag("place.undercover", 1);
				if (t)
					t.TryBeginC1();

				return;
			}
			case E_WALKED_AWAY:
			{
				if (t && playerId > 0)
					t.Remember(playerId, RZ_LongTable.MEM_WALKED);

				// Got up and went with the proof (or a Police eye took it at any stage): C1 at "Who to tell"
				if (run.Flag("proof") == 1)
				{
					q.SetWorldFlag("place.proof", 1);
					if (t && playerId > 0)
						t.Remember(playerId, RZ_LongTable.MEM_KNOWS);

					if (t)
						t.TryBeginC1();
				}

				return;
			}
			case E_BY_HAND:
			{
				q.SetWorldFlag("place.by_hand", 1);
				q.SetWorldFlag("table.by_hand", 1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override string JournalText(RZ_GameModeComponent gm, RZ_QuestRun run)
	{
		RefreshTexts(gm, run);
		if (Over(gm, run))
			return run.Text("fizzle");

		return super.JournalText(gm, run);
	}
}

//------------------------------------------------------------------------------------------------
//! Shared by A Place at the Table's stages: in person, for the guest the offer was made to and
//! nobody else. ★ Police: walking away takes the proof with you.
class RZ_QM_PlaceBase : RZ_QM_TableBase
{
	//------------------------------------------------------------------------------------------------
	override bool IsEligible(RZ_GameModeComponent gm, int playerId)
	{
		if (!super.IsEligible(gm, playerId))
			return false;

		RZ_PlayerProfile profile = gm.GetProfile(playerId);
		return profile.m_sIdentity == QText(gm, "ident") && profile.m_iCharacter == QFlag(gm, "life");
	}

	//------------------------------------------------------------------------------------------------
	protected int GuestId(RZ_GameModeComponent gm)
	{
		RZ_QuestRun run = Run(gm);
		if (!run)
			return -1;

		return RZ_Quest_PlaceAtTable.Guest(gm, run);
	}

	//------------------------------------------------------------------------------------------------
	protected void OfferGuest(RZ_GameModeComponent gm)
	{
		OfferTo(gm, GuestId(gm));
	}

	//------------------------------------------------------------------------------------------------
	//! Walking away from him: the end of the offer. ★ Police: with the proof.
	protected void WalkAway(RZ_GameModeComponent gm, int playerId)
	{
		if (RZ_QuestApproach.HasBackground(gm, playerId, "police"))
		{
			SetQFlag(gm, "proof", 1);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You walk away from the {family} farm. You were a policeman once: you've seen enough here to say it to anyone, and be believed."));
		}
		else
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You walk away from {host}. He doesn't call after you. He never will again."));
		}

		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_WALKED, "walked away from the offer", false);
	}

	//------------------------------------------------------------------------------------------------
	//! The host at the farm gate (this mission's person): "Talk to" and the stage's choices.
	protected IEntity SpawnHostAtGate(RZ_GameModeComponent gm, int objIdx, int talkClue, string choices)
	{
		RZ_LongTable t = Table(gm);
		int role = RZ_ETableRole.MOTHER;
		RZ_TableMember greeter = t.Greeter();
		if (greeter)
			role = greeter.m_eRole;

		return SpawnMember(gm, role, LandPointNear(t.GetHousePos(), 40, 60), objIdx, talkClue, choices);
	}
}

//------------------------------------------------------------------------------------------------
//! C2 stage 1, The dead: where someone died to the infected in the last hour (a wanderer, a truck
//! crew), or a traveller in a ditch this mission lays out, with the infected still about. Stand
//! there 10 s (or look at them), then tell the host at the gate. Or walk away.
class RZ_QM_PlaceDead : RZ_QM_PlaceBase
{
	protected vector m_vDead;
	protected string m_sWho;
	protected bool m_bMade;			// a traveller this mission laid out (not a real death)
	protected bool m_bSeen;
	protected float m_fDwelt;
	protected int m_iBodyClue = -1;
	protected int m_iHostTalk = -1;

	protected static const int OBJ_HOST = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
		m_Req.Bonus("police");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !HoldFarm(gm))
			return false;

		vector house = t.GetHousePos();
		vector at;
		string who;
		if (t.FindRecentDeath(3600, at, who) && RZ_Util.DistXZ(at, house) <= 4000)
		{
			m_vDead = at;
			m_sWho = who;
		}
		else
		{
			m_bMade = true;
			m_vDead = LandPointNear(house, 900, 2000);
			m_sWho = "a man nobody knew";
		}

		m_iBodyClue = AddClue("In the ditch", "A man nobody knew, on his back in the ditch with his pack still on. The infected got there first, and moved on. Nobody else has found him yet.");
		m_iHostTalk = AddClue(QText(gm, "host"), QFill(gm, "{host} is at the gate, the way someone always is. \"Well?\""));
		m_vPos = m_vDead;
		m_iSettlement = gm.FindNearestSettlement(m_vDead);
		m_sTitle = "The dead";
		m_sBrief = QFill(gm, string.Format("{host} says the dead don't need what they have. Someone died to the infected near %1 not long ago. Stand where they fell, before anyone else finds them. Then go back to the farm gate and tell him.", PlaceName(gm, m_vDead)));
		ObjThen(QFill(gm, string.Format("Stand where someone fell near %1.", PlaceName(gm, m_vDead))), QFill(gm, "Tell {host} at the farm gate."));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected string PlaceName(RZ_GameModeComponent gm, vector pos)
	{
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		if (m_bMade)
		{
			SpawnClue(gm, RZ_EPuzzleProp.BODY, m_vDead, m_iBodyClue, "Look at them");
			SpawnGuards(gm, m_vDead, 2, 15, 40);
		}

		return SpawnHostAtGate(gm, OBJ_HOST, m_iHostTalk, "Tell him where the dead are|Walk away") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferGuest(gm);
		CheckFamilyBodies(gm);
		if (m_bSeen || m_bResolved)
			return;

		int guest = GuestId(gm);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != guest || RZ_Util.DistXZ(p.GetOrigin(), m_vDead) > 20)
				continue;

			m_fDwelt += dt;
			if (m_fDwelt >= 10)
				Seen(gm, guest);
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx == m_iBodyClue && playerId == GuestId(gm))
			Seen(gm, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Seen first: the host at the farm gate is next (ObjThen).
	override protected bool ObjFirstDone()
	{
		return m_bSeen;
	}

	//------------------------------------------------------------------------------------------------
	protected void Seen(RZ_GameModeComponent gm, int playerId)
	{
		if (m_bSeen)
			return;

		m_bSeen = true;
		Remember(gm, playerId, RZ_EQuestApproach.TALK, "found the dead first");
		Table(gm).Notice(playerId);
		RZ_LongTable t = Table(gm);
		m_vPos = LandPointNear(t.GetHousePos(), 40, 60);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "Nobody else has been here yet. {host} will want to know. He's at the farm gate."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_HOST || m_bResolved || playerId != GuestId(gm))
			return;

		if (option == 1)
		{
			WalkAway(gm, playerId);
			return;
		}

		if (option != 0)
			return;

		if (!m_bSeen)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} smiles. \"You haven't been yet. Go and look first. Before anyone else does.\""));
			return;
		}

		RZ_LongTable t = Table(gm);
		t.Remember(playerId, RZ_LongTable.MEM_DEAD);
		t.Feed(15);
		t.RecordMeal(playerId);
		string news = t.TruckNews();
		if (news.IsEmpty())
			news = t.RoadNews(source.GetOrigin());

		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} listens with head on one side. \"Before anyone else found them?\" A nod. \"Good. Nothing's wasted.\" There's a bowl kept back for you at the long table, and the road news, which is true: " + news));
		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_TOLD, "told the host where the dead were");
	}

	//------------------------------------------------------------------------------------------------
	//! A supply truck's crew killed with the infected about is "the dead" too.
	override void OnTruckEvent(RZ_GameModeComponent gm, int faction, int what, vector pos, int driver)
	{
		if (what != RZ_SupplyLines.TRUCK_DESTROYED && what != RZ_SupplyLines.TRUCK_ROBBED)
			return;

		if (!gm.GetSpawner().IsZombieNear(pos, 80))
			return;

		Table(gm).NoteDeath(pos, string.Format("%1's truck crew", RZ_Factions.GetName(faction)));
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The dead: you told him where they were.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The dead: somebody else got there first, or nobody went.";
	}
}

//------------------------------------------------------------------------------------------------
//! C2 stage 2, The dying: a wanderer from the roster (held by this mission, so they're nowhere
//! else), bitten, on the road 1-2 km from the farm; or a stranger if nobody is free. Tell him
//! where the farm is (he never arrives), send him the wrong way (back on the roads as the usual
//! bitten wanderer), or give him your antidote.
class RZ_QM_PlaceDying : RZ_QM_PlaceBase
{
	protected int m_iWanderer = -1;
	protected string m_sDying;
	protected vector m_vRoad;
	protected int m_iTalk = -1;
	protected bool m_bSettled;

	protected static const int OBJ_DYING = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(2400);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen())
			return false;

		m_vRoad = LandPointNear(t.GetHousePos(), 1000, 2000);
		RZ_Wanderers wanderers = gm.GetWanderers();
		int roles = RZ_Wanderers.AllRoles() - RZ_Wanderers.RoleBit(RZ_EWandererRole.KID);
		if (wanderers)
			m_iWanderer = wanderers.FindForMission(roles);

		if (m_iWanderer > 0)
		{
			m_sDying = wanderers.GetTitle(m_iWanderer);
			wanderers.Hold(m_iWanderer, m_iId);
		}
		else
		{
			m_sDying = RZ_LongTable.ManName(m_Rng) + ", a man on the road alone";
		}

		m_iTalk = AddClue(m_sDying, string.Format("%1 is sitting on a milestone with a dressing on one arm, already dark through. \"Don't. I know.\" A look up the road, and down it. \"Is there anywhere near? Anywhere with a fire?\"", m_sDying));
		m_vPos = m_vRoad;
		m_iSettlement = gm.FindNearestSettlement(m_vRoad);
		m_sTitle = "The dying";
		m_sBrief = QFill(gm, string.Format("{host} says %1 is on the road and won't make it. Bitten. \"Tell them where we are. We'll look after them.\"", m_sDying));
		m_sObjective = QFill(gm, string.Format("Find %1, bitten, on the road, and decide whether to send %1 to {host}.", m_sDying));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		return SpawnPerson(gm, m_vRoad, OBJ_DYING, m_iTalk, "Talk to " + m_sDying, "Tell them where the farm is|Send them the wrong way|Give them your antidote") != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferGuest(gm);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_DYING || m_bResolved)
			return;

		if (playerId != GuestId(gm))
		{
			gm.NotifyPlayer(playerId, m_sDying, "\"I'm waiting for someone.\" They don't say who.");
			return;
		}

		RZ_LongTable t = Table(gm);
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (option == 0)
		{
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "A thank you. An actual thank you. \"A farm? With a fire?\" Up off the milestone, and walking, toward {town}."));
			m_bSettled = true;
			int role = -1;
			if (wanderers && m_iWanderer > 0)
			{
				role = wanderers.GetRole(m_iWanderer);
				wanderers.TakeOffRoad(m_iWanderer);
			}

			t.Remember(playerId, RZ_LongTable.MEM_DYING);
			t.Taken(m_sDying, PlaceName(gm, m_vRoad), t.FarmTitle(), m_vRoad, role,
				string.Format("%1 was bitten on the road near %2, and never arrived anywhere.", RZ_Util.Capitalize(m_sDying), PlaceName(gm, m_vRoad)));
			Resolve(gm, playerId, RZ_EQuestApproach.BETRAY, RZ_Quest_PlaceAtTable.OUT_SENT, "told the dying wanderer where the farm was");
			return;
		}

		if (option == 1)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You point the other way, toward a safehouse. A nod, and they go, slowly. Somewhere behind you, you're sure, the host already knows.");
			Release(gm, true);
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_LIED, "sent the dying wanderer the wrong way");
			return;
		}

		if (option != 2)
			return;

		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!user || !RZ_AntidoteHelper.TakeOne(user))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "You've no antidote to give. A look at your empty hands, and a nod.");
			return;
		}

		if (wanderers && m_iWanderer > 0)
			wanderers.RememberPlayer(m_iWanderer, playerId, RZ_Wanderers.MEM_SAVED);

		Release(gm, false);
		RZ_QuestRun run = Run(gm);
		if (run)
			run.Reward(gm, playerId, RZ_EFaction.SURVIVORS, 8, 0, "The needle goes in. A long minute, and then the shaking stops. They walk on, and the Haven hears who saved them.");

		Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_SAVED, "gave the dying wanderer an antidote");
	}

	//------------------------------------------------------------------------------------------------
	protected string PlaceName(RZ_GameModeComponent gm, vector pos)
	{
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Back on the road where he sat; bitten for real unless someone gave him a dose.
	protected void Release(RZ_GameModeComponent gm, bool bitten)
	{
		if (m_bSettled)
			return;

		m_bSettled = true;
		RZ_Wanderers wanderers = gm.GetWanderers();
		if (!wanderers || m_iWanderer <= 0)
			return;

		wanderers.Release(m_iWanderer, m_vRoad);
		if (bitten)
			wanderers.BiteOnRoad(m_iWanderer);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		// Nobody came: he goes on, bitten, like anyone bitten on the road
		Release(gm, true);
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "The dying: it's done, one way or the other.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "The dying: nobody went to him. He walked on alone.";
	}
}

//------------------------------------------------------------------------------------------------
//! C2 stage 3, A stray: an exile sleeping rough 1.5-3 km from the farm, away from the safehouses
//! (an escort that follows you once asked). Brought within 40 m of the farm: betrayed. Within
//! 60 m of a safehouse, or 80 m of a Bastion holding or seat: safe, and you're undercover. Or
//! refuse the host at the farm gate.
class RZ_QM_PlaceStray : RZ_QM_PlaceBase
{
	protected ref RZ_Escort m_Escort = new RZ_Escort();
	protected vector m_vHouse;
	protected vector m_vBarn;
	protected int m_iStrayTalk = -1;
	protected int m_iHostTalk = -1;
	protected bool m_bFollowing;

	protected static const int OBJ_STRAY = 0;
	protected static const int OBJ_HOST = 1;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !RZ_Escort.HasRoom(gm) || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		m_vBarn = LandPointNear(m_vHouse, 1500, 3000);
		RZ_Safehouses houses = gm.GetSafehouses();
		int guard = 0;
		while (houses && houses.FindNearest(m_vBarn, 600) >= 0 && guard < 8)
		{
			guard++;
			m_vBarn = LandPointNear(m_vHouse, 1500, 3000);
		}

		string town = PlaceName(gm, m_vBarn);
		SetQText(gm, "stray_town", town);
		string stray = QText(gm, "stray");
		m_iStrayTalk = AddClue(stray, string.Format("%1 is sleeping in a field shelter with his boots on. He wakes all at once, hands up. \"I'm not taking anything.\" The Haven put him out for stealing from the stores. The Red Hand knows where he sleeps. Everybody does.", stray));
		m_iHostTalk = AddClue(QText(gm, "host"), QFill(gm, "{host} is at the gate. \"Bring him to supper. He'll be glad of it.\""));
		m_Escort.m_sName = stray;
		m_Escort.m_sPrefab = RZ_Escort.PrefabFor(gm, 0);
		m_Escort.SetTalk(m_iId, OBJ_STRAY, "Come with me", m_iStrayTalk, "Talk to " + stray);
		if (!m_Escort.Begin(gm, m_vBarn))
			return false;

		m_vPos = m_vBarn;
		m_iSettlement = gm.FindNearestSettlement(m_vBarn);
		m_sTitle = "A stray";
		m_sBrief = QFill(gm, "{host} wants a living one: {stray}, an exile the Red Hand keeps track of, sleeping rough near {stray_town}. \"Bring him to supper.\" Bring him, walk him somewhere safe instead, or tell {host} no at the gate.");
		m_sObjective = QFill(gm, "Find {stray} sleeping rough near {stray_town} and decide where to take {stray}.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetQText(RZ_GameModeComponent gm, string key, string value)
	{
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetText(key, value);
	}

	//------------------------------------------------------------------------------------------------
	protected string PlaceName(RZ_GameModeComponent gm, vector pos)
	{
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return "grid " + RZ_SettlementScanner.GridReference(pos);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		SpawnHostAtGate(gm, OBJ_HOST, m_iHostTalk, "Tell him no");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_bResolved || option != 0)
			return;

		if (playerId != GuestId(gm))
			return;

		if (objIdx == OBJ_HOST)
		{
			RZ_LongTable t = Table(gm);
			t.Remember(playerId, RZ_LongTable.MEM_CROSSED | RZ_LongTable.MEM_DAYLIGHT);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "{host} hears you out, still smiling. \"That's a shame.\" He doesn't say anything else. You're not a guest here any more, and you know it, by daylight or not."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_REFUSED, "refused the host", false);
			return;
		}

		if (objIdx != OBJ_STRAY || m_bFollowing)
			return;

		m_bFollowing = true;
		m_Escort.Follow(playerId);
		m_Escort.Go();
		AddParticipant(playerId);
		gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "He gets up and follows you, a few steps behind, glad of anyone. \"Is it far?\" The farm is one way. A safehouse, or a Bastion post, is another."));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferGuest(gm);
		CheckFamilyBodies(gm);
		m_Escort.Update(gm, dt, ids, players);
		if (m_bResolved)
			return;

		if (m_Escort.IsDead())
		{
			NotifyParticipants(gm, m_sTitle, QFill(gm, "{stray} didn't get anywhere."));
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!m_bFollowing)
			return;

		int by = m_Escort.GetFollowed();
		RZ_LongTable t = Table(gm);
		string stray = QText(gm, "stray");

		// To the farm: the door shuts behind him
		if (m_Escort.IsNear(m_vHouse, 40))
		{
			m_Escort.Arrive();
			t.Remember(by, RZ_LongTable.MEM_STRAY);
			t.Taken(stray, QText(gm, "stray_town"), t.FarmTitle(), m_Escort.m_vPos, -1,
				string.Format("Someone the Haven turned out was seen walking toward the farm past %1 with a friend. He wasn't seen again.", t.GetTownName()));
			gm.NotifyPlayer(by, m_sTitle, QFill(gm, "He goes in ahead of you. The door shuts behind him. Somebody inside is singing."));
			Resolve(gm, by, RZ_EQuestApproach.BETRAY, RZ_Quest_PlaceAtTable.OUT_BROUGHT, "brought the stray to supper");
			return;
		}

		// Somewhere safe instead
		RZ_Safehouses houses = gm.GetSafehouses();
		int house = -1;
		if (houses)
			house = houses.FindNearest(m_Escort.m_vPos, 60);

		if (house >= 0)
		{
			m_Escort.Arrive();
			RZ_Questline.ChangePopulation(gm, house, 1);
			gm.NotifyPlayer(by, m_sTitle, QFill(gm, "The safehouse door opens, and stays open. Somebody brings {stray} a blanket and doesn't ask what he did. You know all you need to about the {family} farm now."));
			Undercover(gm, by);
			return;
		}

		if (AtBastion(gm, m_Escort.m_vPos))
		{
			m_Escort.Arrive();
			RZ_Questline.Headline(gm, QFill(gm, "Bastion took in a civilian off the roads near {stray_town}. \"Sheltered,\" the log says, and nothing else."), false);
			gm.NotifyPlayer(by, m_sTitle, QFill(gm, "The corporal at the gate takes {stray}'s name and gives him a blanket. \"Sheltered civilian,\" he writes. You know all you need to about the {family} farm now."));
			Undercover(gm, by);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool AtBastion(RZ_GameModeComponent gm, vector pos)
	{
		RZ_Territory territory = gm.GetTerritory();
		if (territory && territory.IsActive() && territory.FindHolding(RZ_EFaction.MILITARY, pos, 80))
			return true;

		RZ_Leaders leaders = gm.GetLeaders();
		return leaders && leaders.FindNearest(RZ_EFaction.MILITARY, pos, 80);
	}

	//------------------------------------------------------------------------------------------------
	protected void Undercover(RZ_GameModeComponent gm, int playerId)
	{
		Table(gm).Remember(playerId, RZ_LongTable.MEM_UNDERCOVER);
		Resolve(gm, playerId, RZ_EQuestApproach.SIDE, RZ_Quest_PlaceAtTable.OUT_UNDERCOVER, "walked the stray somewhere safe");
	}

	//------------------------------------------------------------------------------------------------
	//! The stray at the barn, then (he walks with you) the farm (a safehouse or a Bastion post
	//! also ends it, as somewhere safe instead).
	override RZ_ObjPlace ObjectivePlace(RZ_GameModeComponent gm, int playerId, vector from)
	{
		if (m_bFollowing && !m_Escort.IsArrived())
			return RZ_ObjPlace.Spot(m_vHouse, 40);

		return super.ObjectivePlace(gm, playerId, from);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		m_Escort.Cleanup();
		super.Cleanup(gm);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "A stray: {stray} got where he was going.");
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return QFill(gm, "A stray: {stray} went nowhere tonight.");
	}
}

//------------------------------------------------------------------------------------------------
//! C2 stage 4, Grace: the father at the long table. Sit and say the name (family), get up and go
//! with the proof, or fight, at the table (the fight prefab; the house is finished if you win).
class RZ_QM_PlaceGrace : RZ_QM_PlaceBase
{
	protected vector m_vHouse;
	protected int m_iTalk = -1;
	protected bool m_bFighting;

	protected static const int OBJ_MOTHER = 0;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(3600);
		m_Req.Bonus("police");
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen() || !t.IsAlive(RZ_ETableRole.MOTHER) || !HoldFarm(gm))
			return false;

		m_vHouse = t.GetHousePos();
		string name = QText(gm, "name");
		if (name.IsEmpty())
			name = "the ones who came before";

		SetText(gm, "name", name);
		m_iTalk = AddClue(QText(gm, "mother"), QFill(gm, "{mother} has set your place at the head of the table, where {host} usually sits. Everyone is waiting. Nobody eats. \"Grace tonight is yours,\" he says. \"Say a name.\" He doesn't need to tell you which."));
		m_Quiet.Watch(m_vHouse, 100);
		m_vPos = m_vHouse;
		m_iSettlement = t.GetSettlement();
		m_sTitle = "Grace";
		m_sBrief = QFill(gm, "{mother} has set a place for you at the long table, at the head of it. \"Grace tonight is yours. Say a name.\"");
		m_sObjective = QFill(gm, "Say grace at {mother}'s long table at the farm past {town}.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetText(RZ_GameModeComponent gm, string key, string value)
	{
		RZ_QuestRun run = Run(gm);
		if (run)
			run.SetText(key, value);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SpawnScene(RZ_GameModeComponent gm)
	{
		string choices = "Sit, and say the name|Get up and go";
		if (!Table(gm).FightPrefab().IsEmpty())
			choices += "|Turn the table over";

		return SpawnMember(gm, RZ_ETableRole.MOTHER, LandPointNear(m_vHouse, 2, 4), OBJ_MOTHER, m_iTalk, choices) != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (objIdx != OBJ_MOTHER || m_bResolved || m_bFighting || playerId != GuestId(gm))
			return;

		RZ_LongTable t = Table(gm);
		if (option == 0)
		{
			t.RecordMeal(playerId);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You say it. \"For what we are about to receive. For {name}, who came from {name_from}.\" Everyone says amen. The boy says it last. {mother} squeezes your hand across the corner of the table. \"Family,\" he says, and passes you the bread."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_SAT, "sat at the head of the table and said the name");
			return;
		}

		if (option == 1)
		{
			SetQFlag(gm, "proof", 1);
			t.Remember(playerId, RZ_LongTable.MEM_CROSSED);
			gm.NotifyPlayer(playerId, m_sTitle, QFill(gm, "You get up. Nobody stops you. At the door {host} says, quite gently, \"You'll be back,\" and you know that you won't be. You know what this house is, and you can say it to anyone."));
			Resolve(gm, playerId, RZ_EQuestApproach.TALK, RZ_Quest_PlaceAtTable.OUT_LEFT, "got up from the table and went, with the proof", false);
			return;
		}

		if (option != 2)
			return;

		array<int> targets = {playerId};
		t.Sanction(playerId);
		if (!t.StartFight(targets, m_vHouse, false))
			return;

		m_bFighting = true;
		RemoveMembers(RZ_ETableRole.MOTHER);
		Remember(gm, playerId, RZ_EQuestApproach.FORCE, "turned the table over");
		gm.NotifyPlayer(playerId, m_sTitle, "The bowls go over, and the bread, and the long table. Somebody puts the lamp out.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		OfferGuest(gm);
		CheckFamilyBodies(gm);
		if (m_bResolved || !m_bFighting)
			return;

		RZ_LongTable t = Table(gm);
		if (t.IsFighting())
			return;

		m_bFighting = false;
		int guest = GuestId(gm);
		if (t.FightResult() == 1 || !t.Exists())
		{
			if (t.Exists())
				t.BreakUp(RZ_ETableEnd.BY_HAND, t.IsAlive(RZ_ETableRole.BOY), false, QFill(gm, "The lamp at the {family} farm past {town} didn't come on last night. Nobody has been up to see why."));

			Resolve(gm, guest, RZ_EQuestApproach.FORCE, RZ_Quest_PlaceAtTable.OUT_FOUGHT, "finished the house at its own table");
			return;
		}

		// Driven off: out of the door with what you know
		SetQFlag(gm, "proof", 1);
		Resolve(gm, guest, RZ_EQuestApproach.FORCE, RZ_Quest_PlaceAtTable.OUT_LEFT, "was driven from the table", false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return "Grace: it's said.";
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "Grace: the place at the head of the table stayed empty.";
	}
}
