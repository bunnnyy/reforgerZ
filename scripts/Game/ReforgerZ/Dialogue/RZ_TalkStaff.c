//! People whose services are their own actions: a quartermaster, a safehouse's medic
//! (docs/DIALOGUE.md, "The talk screen"). One Talk prompt opens the talk screen, and the person's
//! other ReforgerZ actions are its replies: Trade opens the shop, Ask for whoever's in charge hands
//! the conversation to the seat's leader, the trial dose and the safehouse's services run as built.
//! Without the talk screen the prompt hides and the old actions come back.

//------------------------------------------------------------------------------------------------
//! "Talk to the quartermaster": the one Talk prompt on a person with no puzzle components. Put it
//! in the person's ActionsManagerComponent (the four quartermasters and the safehouse medic have
//! it). While the talk screen is used the person's other ReforgerZ actions hide
//! (RZ_TalkMenu.HidesActionsOn); vanilla's stay.
class RZ_TalkAction : ScriptedUserAction
{
	[Attribute("quartermaster", desc: "Who they are, lower case: the prompt says Talk to the <role>; the talk screen and the nametag name them by it")]
	protected string m_sRole;

	[Attribute("0", UIWidgets.ComboBox, "Who they're with (a quartermaster's own faction wins)", "", ParamEnumArray.FromEnum(RZ_EFaction))]
	protected RZ_EFaction m_eFaction;

	protected static IEntity s_LastOwner;			// weak: the last person asked about (On is asked every frame)
	protected static RZ_TalkAction s_LastAction;

	//------------------------------------------------------------------------------------------------
	//! The Talk action on `owner`, or null (cached for the last person asked about).
	static RZ_TalkAction On(IEntity owner)
	{
		if (!owner)
			return null;

		if (owner == s_LastOwner)
			return s_LastAction;

		s_LastOwner = owner;
		s_LastAction = null;
		ActionsManagerComponent actions = ActionsManagerComponent.Cast(owner.FindComponent(ActionsManagerComponent));
		if (!actions)
			return null;

		array<BaseUserAction> list = {};
		actions.GetActionsList(list);
		foreach (BaseUserAction a : list)
		{
			RZ_TalkAction talk = RZ_TalkAction.Cast(a);
			if (talk)
			{
				s_LastAction = talk;
				break;
			}
		}

		return s_LastAction;
	}

	//------------------------------------------------------------------------------------------------
	string GetRole()
	{
		if (m_sRole.IsEmpty())
			return "quartermaster";

		return m_sRole;
	}

	//------------------------------------------------------------------------------------------------
	//! Who they're with: a quartermaster's own faction, else the attribute.
	RZ_EFaction GetFaction()
	{
		RZ_QuartermasterComponent qm = RZ_QuartermasterComponent.Cast(GetOwner().FindComponent(RZ_QuartermasterComponent));
		if (qm)
			return qm.GetFaction();

		return m_eFaction;
	}

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Talk to the " + GetRole();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Only while this machine uses the talk screen; without it, the person's old actions.
	override bool CanBeShownScript(IEntity user)
	{
		return RZ_TalkMenu.Available() && RZ_Dialogue.IsPerson(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_QuartermasterHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0 || !gm.GetDialogue())
			return;

		gm.GetDialogue().Open(playerId, pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
//! The talk screen's adapter for someone with an RZ_TalkAction. The replies are their own actions
//! in the order the prefab lists them: Trade (the shop screen, while it's used), Ask for whoever's
//! in charge (RZ_LeaderAction Talk), the trial dose (greyed with what it needs when it doesn't
//! apply), and every grouped action that shows for you (the safehouse's services). A leader's own
//! asks (work, news, standing, the epilogue's) are the leader's replies, not these. The shop's Buy,
//! Requisition and service actions stay the shop screen's: without it they're world prompts.
class RZ_TalkStaff : RZ_TalkSpeaker
{
	static const int TRADE = 60;
	static const int IN_CHARGE = 61;
	static const int ACTION_BASE = 62;				// 62-87: the person's own actions, as last listed
	protected static const int ACTION_MAX = 26;		// below RZ_Dialogue.PAGE_BACK (89)

	protected IEntity m_Owner;						// weak
	protected RZ_TalkAction m_Talk;
	protected ref array<BaseUserAction> m_aShown = {};	// what ACTION_BASE + i runs

	//------------------------------------------------------------------------------------------------
	void RZ_TalkStaff(IEntity owner, RZ_TalkAction talk)
	{
		m_Owner = owner;
		m_Talk = talk;
	}

	//------------------------------------------------------------------------------------------------
	//! "The quartermaster"; the sub-line "The Haven - quartermaster - Morton".
	override void Setup(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId)
	{
		if (!m_Talk || !m_Owner)
			return;

		string role = m_Talk.GetRole();
		RZ_EFaction faction = m_Talk.GetFaction();
		s.m_sName = "The " + role;
		s.m_sHeader = s.m_sName;
		s.m_iFaction = faction;
		s.m_sSub = RZ_Factions.GetName(faction) + " - " + role;
		RZ_Settlement town = gm.GetSettlement(gm.FindNearestSettlement(m_Owner.GetOrigin()));
		if (town)
			s.m_sSub += " - " + town.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! Their one line when you walk up, in their faction's voice.
	override void Greet(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user)
	{
		if (!m_Talk || !m_Owner)
			return;

		bool counter = m_Owner.FindComponent(RZ_QuartermasterComponent) != null;
		s.m_aNew.Insert(Greeting(m_Talk.GetFaction(), counter));
	}

	//------------------------------------------------------------------------------------------------
	//! A quartermaster's greeting by faction; anyone else's (the safehouse medic) is the Haven's.
	static string Greeting(RZ_EFaction faction, bool counter)
	{
		if (!counter)
			return "\"Sit where I can see you. Anything bleeding? Anything bitten? Tell me the truth, it's quicker.\"";

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "\"Stores are open, such as they are. Tins, dressings, the odd rifle. What do you need, love?\"";
			case RZ_EFaction.MILITARY: return "\"Stores. State what you need. Everything is signed for, including you.\"";
			case RZ_EFaction.BANDITS: return "\"A customer. Everything's for sale, and everything's counted. Including the questions.\"";
			case RZ_EFaction.INSTITUTE: return "\"Supplies are issued against Favor. I log every item, and every name.\"";
		}

		return "\"Yes?\"";
	}

	//------------------------------------------------------------------------------------------------
	override void Replies(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, IEntity user, notnull array<ref RZ_TalkReply> replies)
	{
		m_aShown.Clear();
		if (!m_Owner)
			return;

		ActionsManagerComponent actions = ActionsManagerComponent.Cast(m_Owner.FindComponent(ActionsManagerComponent));
		if (!actions)
			return;

		bool shop = RZ_ShopMenu.Available() && m_Owner.FindComponent(RZ_QuartermasterComponent) != null;
		array<BaseUserAction> list = {};
		actions.GetActionsList(list);
		foreach (BaseUserAction a : list)
		{
			if (RZ_ShopOpenAction.Cast(a))
			{
				if (shop)
					replies.Insert(Reply(TRADE, "Trade"));

				continue;
			}

			RZ_LeaderAction leader = RZ_LeaderAction.Cast(a);
			if (leader)
			{
				if (leader.GetKind() == RZ_ELeaderAction.TALK)
					replies.Insert(Reply(IN_CHARGE, "Ask for whoever's in charge"));

				continue;
			}

			if (m_aShown.Count() >= ACTION_MAX)
				continue;

			if (RZ_TrialDoseAction.Cast(a))
			{
				RZ_TalkReply dose = Reply(ACTION_BASE + m_aShown.Count(), "Ask for the trial dose");
				string need = "";
				if (gm.GetInfection())
					need = gm.GetInfection().TrialDoseNeed(playerId);

				if (!need.IsEmpty())
				{
					dose.m_bEnabled = false;
					dose.m_sNeed = need;
				}

				m_aShown.Insert(a);
				replies.Insert(dose);
				continue;
			}

			RZ_GroupedAction grouped = RZ_GroupedAction.Cast(a);
			string label;
			if (!grouped || !grouped.Visible(user) || !grouped.Label(label) || label.IsEmpty())
				continue;

			m_aShown.Insert(a);
			replies.Insert(Reply(ACTION_BASE + m_aShown.Count() - 1, label));
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Pick(RZ_GameModeComponent gm, RZ_TalkSession s, int playerId, int replyId, IEntity user)
	{
		if (!m_Owner)
			return;

		if (replyId == TRADE)
		{
			RZ_ShopScreen.Open(gm, playerId, m_Owner, user);
			return;
		}

		if (replyId == IN_CHARGE)
		{
			// The seat's leader takes the conversation over; nobody there, or busy: they say so
			if (gm.GetDialogue() && gm.GetDialogue().ToSeat(s, m_Owner, user))
				return;

			if (gm.GetLeaders())
				gm.GetLeaders().HandleAction(playerId, RZ_ELeaderAction.TALK, m_Owner, user);

			return;
		}

		int i = replyId - ACTION_BASE;
		if (i < 0 || i >= m_aShown.Count())
			return;

		BaseUserAction a = m_aShown[i];
		if (RZ_TrialDoseAction.Cast(a))
		{
			if (gm.GetInfection())
				gm.GetInfection().TrialDose(playerId);

			return;
		}

		RZ_GroupedAction grouped = RZ_GroupedAction.Cast(a);
		if (grouped)
			grouped.Run(m_Owner, user);
	}
}
