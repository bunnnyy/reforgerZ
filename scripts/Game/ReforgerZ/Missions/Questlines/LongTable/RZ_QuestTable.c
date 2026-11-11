//! Shared by the Long Table's two questlines (RZ_Quest_HouseWentQuiet, RZ_Quest_PlaceAtTable):
//! the stage missions' base, which knows the house (World/RZ_LongTable.c), and the stand-in for
//! a stage whose house is gone. The house's own people stand aside (HoldScene) while a stage
//! mission has the farm; the mission's people are the family, and a death among them is the
//! house's death too (MemberKilled).

//------------------------------------------------------------------------------------------------
class RZ_QM_TableBase : RZ_QuestMission
{
	protected bool m_bHolding;
	protected float m_fOfferTimer = 1000;
	protected ref array<IEntity> m_aFamilyBodies = {};
	protected ref array<int> m_aFamilyRoles = {};
	protected ref array<int> m_aIdle = {};			// persons whose only choice is a greeting

	protected static const float OFFER_SECONDS = 60;

	//------------------------------------------------------------------------------------------------
	protected RZ_LongTable Table(RZ_GameModeComponent gm)
	{
		return gm.GetLongTable();
	}

	//------------------------------------------------------------------------------------------------
	//! Nobody gives this job: offered in person only (OfferTo), no faction pays, anyone may take it.
	protected void ConfigureUnaligned(float seconds)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 0, 0);
		m_bHasGiver = false;
		m_Req.m_bLeaderDecides = true;
		m_bInPersonOnly = true;
		m_fParticipationRadius = 0;
		m_fTimeLeft = seconds;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's contract, offered on the radio like any other (the end of the house, once it's
	//! been told).
	protected void ConfigureFor(RZ_EFaction giver, int fame, int favor, float seconds)
	{
		SetContract(giver, RZ_ETier.STRANGER, fame, favor);
		m_fParticipationRadius = 0;
		m_fTimeLeft = seconds;
	}

	//------------------------------------------------------------------------------------------------
	//! The farm is this mission's while it runs. False if nobody is at the farm.
	protected bool HoldFarm(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsOpen())
			return false;

		t.HoldScene(m_iId);
		m_bHolding = true;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! This stage, in person: its brief and where, once.
	protected void OfferTo(RZ_GameModeComponent gm, int playerId)
	{
		if (playerId <= 0 || m_aOfferedTo.Contains(playerId) || !IsEligible(gm, playerId))
			return;

		m_aOfferedTo.Insert(playerId);
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		vector from = vector.Zero;
		if (ent)
			from = ent.GetOrigin();

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("%1\n(%2)", m_sBrief, GetWhereTextFor(gm, from, playerId)));
	}

	//------------------------------------------------------------------------------------------------
	//! Every minute: offer it to everyone online who took part in the questline, and (noticers)
	//! to everyone who noticed something about the house.
	protected void OfferAround(RZ_GameModeComponent gm, float dt, bool noticers)
	{
		m_fOfferTimer += dt;
		if (m_fOfferTimer < OFFER_SECONDS)
			return;

		m_fOfferTimer = 0;
		RZ_QuestRun run = Run(gm);
		if (run)
		{
			array<string> idents = {};
			run.GetIdents(idents);
			foreach (string ident : idents)
			{
				OfferTo(gm, gm.GetQuestlines().OnlineId(ident));
			}
		}

		RZ_LongTable t = Table(gm);
		if (!noticers || !t)
			return;

		array<int> online = {};
		t.GetNoticers(online);
		foreach (int playerId : online)
		{
			OfferTo(gm, playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One of the family, as this mission's person (Puzzles > Civilian): "Talk to <name>" and up to
	//! four choices (none: a greeting, so the person never shows unlabelled actions). Watched: if a
	//! player kills them, the house loses them.
	protected IEntity SpawnMember(RZ_GameModeComponent gm, int role, vector pos, int objIdx, int talkClue, string choices)
	{
		RZ_LongTable t = Table(gm);
		if (!t || !t.IsAlive(role))
			return null;

		string name = t.NameOf(role);
		if (talkClue < 0)
			talkClue = AddClue(name, string.Format("%1 nods to you, and goes on with what they were doing.", name));

		if (choices.IsEmpty())
		{
			choices = "Wish them good evening";
			if (!m_aIdle.Contains(objIdx))
				m_aIdle.Insert(objIdx);
		}

		IEntity e = SpawnPerson(gm, pos, objIdx, talkClue, "Talk to " + name, choices);
		if (e)
		{
			m_aFamilyBodies.Insert(e);
			m_aFamilyRoles.Insert(role);
		}

		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! People with nothing to choose only greet you back.
	override void HandleChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_aIdle.Contains(objIdx))
		{
			gm.NotifyPlayer(playerId, m_sTitle, "\"Evening.\" A smile, a nod at the house. \"Staying for supper?\"");
			return;
		}

		super.HandleChoice(gm, playerId, objIdx, option, source);
	}

	//------------------------------------------------------------------------------------------------
	//! This mission's people of one role leave the scene (the men go to fight, the boy is taken
	//! away): their bodies go.
	protected void RemoveMembers(int role)
	{
		for (int i = m_aFamilyBodies.Count() - 1; i >= 0; i--)
		{
			if (m_aFamilyRoles[i] != role)
				continue;

			IEntity e = m_aFamilyBodies[i];
			m_aFamilyBodies.Remove(i);
			m_aFamilyRoles.Remove(i);
			if (e)
			{
				m_aProps.RemoveItem(e);
				RZ_Util.DeleteUnit(e);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A member's body here died: the house hears of it (Infamy rules included), and the mission
	//! is told (OnMemberKilled).
	protected void CheckFamilyBodies(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		for (int i = m_aFamilyBodies.Count() - 1; i >= 0; i--)
		{
			IEntity e = m_aFamilyBodies[i];
			if (!e || RZ_Util.IsCharacterAlive(e))
				continue;

			int role = m_aFamilyRoles[i];
			int killer = RZ_Util.GetKillerPlayerId(e);
			m_aFamilyBodies.Remove(i);
			m_aFamilyRoles.Remove(i);
			if (t)
				t.MemberKilled(role, killer, e.GetOrigin());

			OnMemberKilled(gm, role, killer);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMemberKilled(RZ_GameModeComponent gm, int role, int killer)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! A player within radius of pos, -1 if none.
	protected int PlayerNear(array<int> ids, array<IEntity> players, vector pos, float radius)
	{
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				return ids[i];
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! An infected within radius of pos.
	protected bool InfectedNear(RZ_GameModeComponent gm, vector pos, float radius)
	{
		return gm.GetSpawner().IsZombieNear(pos, radius);
	}

	//------------------------------------------------------------------------------------------------
	//! The supper hour (18:30 to 20:30), when the whole house is at the table. False if the clock
	//! isn't known.
	static bool IsSupper()
	{
		float hour = RZ_GameTime.GetHour();
		return hour >= 0 && RZ_GameTime.IsWithin(hour, 18.5, 2);
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		RZ_LongTable t = Table(gm);
		if (m_bHolding && t)
			t.ReleaseScene(m_iId);

		m_bHolding = false;
		super.Cleanup(gm);
	}
}

//------------------------------------------------------------------------------------------------
//! A stage whose house is gone (broken up, moved away for good): it ends at once, and the
//! questline's Next() says so with a line that makes sense.
class RZ_QM_TableGone : RZ_QM_TableBase
{
	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		ConfigureUnaligned(60);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_sTitle = "The farm";
		m_sBrief = "There's nobody at the farm any more.";
		RZ_LongTable t = Table(gm);
		if (t)
			m_vPos = t.GetHousePos();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return "";
	}
}
