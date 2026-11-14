//! The grid's two jobs (docs/MISSIONS.md, "The grid's jobs"; docs/DESIGN.md, "The grid, region by
//! region"): the Red Hand's Lights Out, cutting a rival's substation breaker or a rival's mast for pay
//! through the usual caps; and Lights Back, the hurt faction's ask to put a cut substation back (a
//! fuse fitted, the breaker closed). A cut mast's put-back stays its holder's ask on the channel
//! (World/RZ_Masts.c), so there's no job for it here.

//------------------------------------------------------------------------------------------------
//! Lights Out: the book wants a rival's town dark. Cut the breaker at its substation (or the cable at
//! a mast a rival holds) and walk away. The job's pay replaces the book's tip for a cut
//! (RZ_Substations.SetContracted); the Infamy the cut costs is the cut's own.
class RZ_Mission_LightsOut : RZ_Mission
{
	protected int m_iSub = -1;
	protected int m_iMast = -1;
	protected int m_iVictim = -1;
	protected string m_sKey;
	protected string m_sWhere;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.BANDITS, RZ_ETier.KNOWN, 8, 20);
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A lit substation whose towns a rival holds, or a mast a rival holds. Fills the target; false
	//! when there's none.
	protected bool Pick(RZ_GameModeComponent gm)
	{
		m_iSub = -1;
		m_iMast = -1;
		RZ_Substations subs = RZ_Substations.Get();
		if (subs && subs.IsOn())
		{
			array<int> lit = {};
			for (int i = 0; i < subs.Count(); i++)
			{
				RZ_Substation s = subs.GetSub(i);
				if (s && s.m_bOn && subs.HurtFaction(i) >= 0 && !subs.IsContracted("sub:" + i.ToString()))
					lit.Insert(i);
			}

			if (!lit.IsEmpty())
			{
				m_iSub = lit.GetRandomElement();
				m_iVictim = subs.HurtFaction(m_iSub);
				return true;
			}
		}

		RZ_Masts masts = RZ_Masts.Get();
		if (!masts || !masts.IsOn())
			return false;

		array<int> held = {};
		for (int j = 0; j < masts.Count(); j++)
		{
			RZ_Mast m = masts.GetMast(j);
			if (!m || !m.m_bUp)
				continue;

			int f = RZ_Masts.FactionOf(m.m_eOwner);
			if (f >= 0 && f != RZ_EFaction.BANDITS && (!subs || !subs.IsContracted("mast:" + j.ToString())))
				held.Insert(j);
		}

		if (held.IsEmpty())
			return false;

		m_iMast = held.GetRandomElement();
		m_iVictim = RZ_Masts.FactionOf(masts.GetMast(m_iMast).m_eOwner);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!Pick(gm))
			return 0;

		return 0.6;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		if (!Pick(gm))
			return false;

		RZ_Substations subs = RZ_Substations.Get();
		string them = RZ_Factions.GetNameInText(m_iVictim);
		m_sTitle = "Lights Out";
		if (m_iSub >= 0)
		{
			RZ_Substation s = subs.GetSub(m_iSub);
			m_sKey = "sub:" + m_iSub.ToString();
			m_vPos = s.m_vControl;
			m_sWhere = subs.TownNames(s);
			m_sBrief = string.Format("The book wants %1 dark tonight. %2 keep the lights on there from %3. Cut the breaker and walk away. Nobody needs to see your face.", m_sWhere, them, s.m_sName);
			m_sObjective = string.Format("Cut the breaker at %1.", s.m_sName);
		}
		else
		{
			RZ_Mast m = RZ_Masts.Get().GetMast(m_iMast);
			m_sKey = "mast:" + m_iMast.ToString();
			m_vPos = RZ_Masts.Get().CabinetPos(m);
			m_sWhere = m.m_sShort;
			m_sBrief = string.Format("%1 talk to the whole valley off %2. The book's tired of listening. Cut the cable at its foot.", them, m.m_sShort);
			m_sObjective = string.Format("Cut the cable at %1.", m.m_sShort);
		}

		// A cut costs the cut's own Infamy with them; the job only says who it's against
		SetRival(m_iVictim, 0);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		if (subs)
			subs.SetContracted(m_sKey, true);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 80, m_aParticipants);
		int by = 0;
		if (m_iSub >= 0)
		{
			RZ_Substation s = RZ_Substations.Get().GetSub(m_iSub);
			if (!s || s.m_bOn)
				return;

			by = s.m_iCutBy;
		}
		else
		{
			RZ_Mast m = RZ_Masts.Get().GetMast(m_iMast);
			if (!m || m.m_bUp)
				return;

			by = m.m_iCutBy;
		}

		// Dark: whoever cut it did the job (anyone else's cut still does it, and pays them)
		if (by > 0 && !m_aParticipants.Contains(by))
			m_aParticipants.Insert(by);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		super.Cleanup(gm);
		RZ_Substations subs = RZ_Substations.Get();
		if (subs && !m_sKey.IsEmpty())
			subs.SetContracted(m_sKey, false);
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("Lights Out: %1 went dark. The book's paid.", m_sWhere);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Lights Out: %1 still has its lights. The book's found someone else.", m_sWhere);
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: one tick of the job's own check, with nobody near.
	void TestTick(RZ_GameModeComponent gm)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		OnUpdate(gm, 1, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: the target this job would take (a substation's index, or -1-mast for a mast).
	int TestTarget()
	{
		if (m_iSub >= 0)
			return m_iSub;

		return -1 - m_iMast;
	}
}

//------------------------------------------------------------------------------------------------
//! Lights Back: a substation whose towns a faction holds was cut on purpose. That faction asks for
//! someone to put it back: a fuse fitted and the breaker closed (World/RZ_Substations.c).
class RZ_Mission_LightsBack : RZ_Mission
{
	protected int m_iSub = -1;
	protected string m_sWhere;

	//------------------------------------------------------------------------------------------------
	//! A cut substation (not yet back) whose towns a faction (not the Red Hand) holds; -1 none.
	protected static int PickCut(out int giver)
	{
		giver = -1;
		RZ_Substations subs = RZ_Substations.Get();
		if (!subs || !subs.IsOn())
			return -1;

		for (int i = 0; i < subs.Count(); i++)
		{
			RZ_Substation s = subs.GetSub(i);
			if (!s || s.m_bOn || !s.m_bCut)
				continue;

			int f = subs.HurtFaction(i);
			if (f >= 0)
			{
				giver = f;
				return i;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		int giver;
		PickCut(giver);
		if (giver < 0)
			giver = RZ_EFaction.SURVIVORS;

		SetContract(giver, RZ_ETier.STRANGER, 8, 10);
		m_fTimeLeft = 5400;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		int giver;
		if (PickCut(giver) < 0 || giver != m_eGiver)
			return 0;

		return 1.2;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		int giver;
		m_iSub = PickCut(giver);
		if (m_iSub < 0 || giver != m_eGiver)
			return false;

		RZ_Substations subs = RZ_Substations.Get();
		RZ_Substation s = subs.GetSub(m_iSub);
		if (!ClaimSubject(gm, "substation:" + m_iSub.ToString()))
			return false;

		m_vPos = s.m_vControl;
		m_sWhere = subs.TownNames(s);
		m_iSettlement = gm.FindNearestSettlement(m_vPos);
		m_sTitle = "Lights Back";
		m_sBrief = string.Format("Somebody cut the breaker at %1 and took the fuse with them. %2 have been dark since. Take a fuse out there and put the lights back.", s.m_sName, m_sWhere);
		m_sObjective = string.Format("Fit a fuse at %1 and close the breaker.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Two steps at one cabinet: the tracker's line names the step you're on.
	override string ObjectiveText(RZ_GameModeComponent gm)
	{
		RZ_Substation s = RZ_Substations.Get().GetSub(m_iSub);
		if (!s)
			return super.ObjectiveText(gm);

		if (!s.m_bFused)
			return string.Format("Fit a fuse at %1.", s.m_sName);

		return string.Format("Close the breaker at %1.", s.m_sName);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		RZ_ContractHelp.AddNear(ids, players, m_vPos, 80, m_aParticipants);
		RZ_Substation s = RZ_Substations.Get().GetSub(m_iSub);
		if (!s)
		{
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		if (!s.m_bOn)
			return;

		if (s.m_iOnBy > 0 && !m_aParticipants.Contains(s.m_iOnBy))
			m_aParticipants.Insert(s.m_iOnBy);

		m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override string GetSuccessText(RZ_GameModeComponent gm)
	{
		return string.Format("Lights Back: %1 have their lights again.", m_sWhere);
	}

	//------------------------------------------------------------------------------------------------
	override string GetFailText(RZ_GameModeComponent gm)
	{
		return string.Format("Lights Back: %1 are still dark. Nobody went out to the breaker.", m_sWhere);
	}

	//------------------------------------------------------------------------------------------------
	//! Test only: one tick of the job's own check, with nobody near.
	void TestTick(RZ_GameModeComponent gm)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		OnUpdate(gm, 1, ids, players);
	}
}
