//! The radio beyond the open channel (docs/DIALOGUE.md, "The open channel"): the secure nets, the
//! distress calls, and a house radio with power. Server only.
//!
//! - **Secure nets.** A Radio Operator listening to a cracked net (RZ_SecureRadio) who calls on a
//!   radio can talk on that net too: Bastion's command net, the Red Hand boss's channel, Halcyon's
//!   lab channel (`for radio:bastion`, `radio:redhand`, `radio:halcyon`, `radio:secure`). Nothing on
//!   a secure net is public: an intruder's words go to the net, not to the air.
//! - **Distress calls.** Now and then someone near a safehouse is on the open channel asking for
//!   help (`distress`): the nearest Haven house within 2 km has nobody at its gate, and an hour in
//!   two carries a call (the season's seed and the real hour). Answering sends them to that house's
//!   gate (`call:distress`), where errands deal its gate card (Who's at the Gate) under errands' own
//!   rules and pay. Once a real hour per player.
//! - **A house radio** can call the open channel only with power (RZ_PowerGrid).
class RZ_DialogueNets
{
	protected static const float DISTRESS_RANGE = 2000;
	protected static const int DISTRESS_GAP = 3600;

	//------------------------------------------------------------------------------------------------
	//! The secure net's kinds for the radio adapter: radio:secure and radio:<faction>.
	static void SecureKinds(int playerId, notnull array<string> kinds)
	{
		int f = RZ_DialogueWorld.SecureNet(playerId);
		if (f < 0 || !RZ_DialogueSettings.Get().m_bRadioCalls)
			return;

		kinds.Insert("radio:" + RZ_DialogueDB.FactionWord(f));
		kinds.Insert("radio:secure");
	}

	//------------------------------------------------------------------------------------------------
	//! The safehouse a distress call is about (-1: no call on the air for this player now).
	static int DistressHouse(RZ_DCtx ctx)
	{
		if (!ctx.m_bRadio || !RZ_DialogueSettings.Get().m_bRadioCalls)
			return -1;

		RZ_GameModeComponent gm = ctx.m_GM;
		RZ_Safehouses houses = gm.GetSafehouses();
		RZ_Errands errands = gm.GetErrands();
		if (!houses || !errands || !errands.GetStrangers())
			return -1;

		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		int answered = trees.GetMemory().TopicAt(ctx.m_iPlayer, "radio:open|distress");
		if (answered > 0 && RZ_SeasonClock.Now() - answered < DISTRESS_GAP)
			return -1;

		int house = houses.FindNearest(ctx.Pos(), DISTRESS_RANGE);
		if (house < 0 || errands.GetStrangers().AtGate(house) > 0)
			return -1;

		// An hour in two carries a call, the same for everyone listening
		int seed = 0;
		if (RZ_WorldFacts.Get())
			seed = RZ_WorldFacts.Get().GetSeed();

		int hour = RZ_SeasonClock.Now() / 3600;
		if (Math.AbsInt(seed + hour * 7919 + house * 31) % 2 != 0)
			return -1;

		return house;
	}

	//------------------------------------------------------------------------------------------------
	//! call:distress: the caller is sent to the house's gate (errands deal the rest).
	static void Answer(RZ_TalkSession s)
	{
		RZ_DCtx ctx = s.m_Tree.m_Ctx;
		int house = DistressHouse(ctx);
		RZ_GameModeComponent gm = ctx.m_GM;
		if (house < 0)
		{
			s.m_aNew.Insert("Static. Whoever it was has stopped calling.");
			return;
		}

		bool bitten = Math.AbsInt(RZ_SeasonClock.Now() / 3600 + house) % 4 == 0;
		gm.GetErrands().GetStrangers().SendToGate(house, bitten);
		RZ_DialogueTrees.Get().GetMemory().NoteTopic(ctx.m_iPlayer, "radio:open|distress");
		s.m_aNew.Insert(string.Format("(They'll make for the gate at %1. Someone there will need a hand with them.)", gm.GetSafehouses().Get(house).m_sName));
		RZ_Log.Info(string.Format("Dialogue: distress call answered by %1, sent to house %2 (bitten %3)", RZ_Util.PlayerName(ctx.m_iPlayer), house, bitten));
	}

	//------------------------------------------------------------------------------------------------
	//! {distress}: the house the caller is near, by its town ("" none).
	static string DistressPlace(RZ_DCtx ctx)
	{
		int house = DistressHouse(ctx);
		if (house < 0)
			return "somewhere close";

		return ctx.m_GM.GetSafehouses().Get(house).m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! call:net_word: Bastion's net gave you the word at the wire (+1 at Bastion's looks today).
	static void NetWord(RZ_TalkSession s)
	{
		RZ_GameModeComponent gm = s.m_Tree.m_Ctx.m_GM;
		RZ_Hostility hostility = gm.GetHostility();
		if (!hostility || !hostility.GetDisguise().IsOn())
			return;

		hostility.GetDisguise().LearnWord(s.m_iPlayer);
		RZ_DialogueTrees.Get().GetMemory().Learn(s.m_iPlayer, "bastion.word");
		s.m_aNew.Insert(string.Format("(The word at the wire today: %1.)", hostility.GetDisguise().WordToday()));
	}

	//------------------------------------------------------------------------------------------------
	//! call:net_lost: they caught an intruder on the net and changed codes (the listening ends).
	static void NetLost(RZ_TalkSession s)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (hub && hub.GetRadio())
			hub.GetRadio().OnPlayerSpawned(s.m_iPlayer);

		s.m_aNew.Insert("(Codes changed. The net's gone back to noise.)");
	}

	//------------------------------------------------------------------------------------------------
	//! A radio that isn't an item you carry (a house set) calls only with power.
	static bool CanCall(IEntity radio)
	{
		if (!radio)
			return false;

		if (InventoryItemComponent.Cast(radio.FindComponent(InventoryItemComponent)))
			return true;

		return RZ_PowerGrid.IsPoweredAt(radio.GetOrigin());
	}
}
