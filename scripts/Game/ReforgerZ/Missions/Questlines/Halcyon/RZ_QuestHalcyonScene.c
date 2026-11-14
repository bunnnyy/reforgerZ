//! Shared by Halcyon's three questlines (The Three Weeks, The Other Two, Authors; docs/MISSIONS.md,
//! "Questlines"): a stage mission whose people and props stand at several places, each put down
//! only when a player comes within m_fSpotRadius of it and gone with the stage; a story item (a
//! letter, a notebook) one player carries as data; and the checks every stage needs: is the person
//! the story stored still the same person in the same seat, and may this player act for the story.

//------------------------------------------------------------------------------------------------
//! One place in a scene: a person (choices and a "Talk to"), a clue or a prop.
class RZ_HalcyonSpot
{
	vector m_vPos;
	RZ_EPuzzleProp m_eKind;
	int m_iObj = -1;		// puzzle object index for choices and uses, -1 = a clue only
	int m_iClue = -1;
	string m_sRead;
	string m_sUse;
	string m_sChoices;
	string m_sName;			// "the duty officer at the lab", for the where text ("" = not listed)
	bool m_bOn = true;		// off = not put down (or taken away again)
	bool m_bUp;
	IEntity m_Ent;
}

//------------------------------------------------------------------------------------------------
class RZ_QM_HalcyonScene : RZ_QuestMission
{
	protected ref array<ref RZ_HalcyonSpot> m_aSpots = {};
	protected float m_fSpotRadius = 300;

	// The story item, carried as data by one player (a letter, a notebook). It goes back where it
	// came from if they're gone (dead or off the server) for a minute.
	protected int m_iHolder = -1;
	protected float m_fHolderGone;
	protected string m_sItemName = "it";

	protected static const float HOLDER_GRACE = 60;

	//------------------------------------------------------------------------------------------------
	//! A place in the scene. kind CIVILIAN or OBJECT for something with choices (objIdx >= 0),
	//! CLUE or BODY for something to read. Returns the spot's index.
	protected int AddSpot(vector pos, RZ_EPuzzleProp kind, int objIdx, int clueIdx, string readLabel, string choices, string name, string useLabels = "")
	{
		RZ_HalcyonSpot s = new RZ_HalcyonSpot();
		s.m_vPos = pos;
		s.m_eKind = kind;
		s.m_iObj = objIdx;
		s.m_iClue = clueIdx;
		s.m_sRead = readLabel;
		s.m_sChoices = choices;
		s.m_sUse = useLabels;
		s.m_sName = name;
		m_aSpots.Insert(s);
		return m_aSpots.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_HalcyonSpot GetSpot(int idx)
	{
		if (idx < 0 || idx >= m_aSpots.Count())
			return null;

		return m_aSpots[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! New choices (and uses) on a spot, now if it's already standing.
	protected void SetSpotLabels(int idx, string choices, string useLabels = "")
	{
		RZ_HalcyonSpot s = GetSpot(idx);
		if (!s)
			return;

		s.m_sChoices = choices;
		s.m_sUse = useLabels;
		if (s.m_bUp && s.m_iObj >= 0)
			SetObjectLabels(s.m_iObj, useLabels, choices);
	}

	//------------------------------------------------------------------------------------------------
	//! Off: taken away (and never put down until it's on again).
	protected void SetSpotOn(int idx, bool on)
	{
		RZ_HalcyonSpot s = GetSpot(idx);
		if (!s || s.m_bOn == on)
			return;

		s.m_bOn = on;
		if (on || !s.m_bUp)
			return;

		RZ_Leaving.Remove(s.m_Ent);
		s.m_Ent = null;
		s.m_bUp = false;
	}

	//------------------------------------------------------------------------------------------------
	protected vector SpotPos(int idx)
	{
		RZ_HalcyonSpot s = GetSpot(idx);
		if (!s)
			return m_vPos;

		return s.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! Puts down the spots players have come near. Subclasses do their part in OnScene.
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (RZ_HalcyonSpot s : m_aSpots)
		{
			if (!s.m_bOn || s.m_bUp || RZ_Util.NearestDistance(s.m_vPos, players) > m_fSpotRadius)
				continue;

			s.m_bUp = true;
			if (s.m_iObj >= 0)
				s.m_Ent = SpawnObject(gm, s.m_eKind, s.m_vPos, s.m_iObj, s.m_sUse, s.m_sChoices, s.m_iClue, s.m_sRead);
			else if (s.m_iClue >= 0)
				s.m_Ent = SpawnClue(gm, s.m_eKind, s.m_vPos, s.m_iClue, s.m_sRead);

			OnSpotUp(gm, s);
		}

		TrackHolder(gm, dt, ids);
		if (m_eState == RZ_EMissionState.ACTIVE && !m_bResolved)
			OnScene(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! A spot was just put down (guards, a line of text).
	protected void OnSpotUp(RZ_GameModeComponent gm, RZ_HalcyonSpot s)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Once per tick while the stage is open.
	protected void OnScene(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
	}

	// ================================================================ The story item
	//------------------------------------------------------------------------------------------------
	protected bool IsHolder(int playerId)
	{
		return playerId > 0 && playerId == m_iHolder;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasHolder()
	{
		return m_iHolder > 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetHolder(int playerId)
	{
		m_iHolder = playerId;
		m_fHolderGone = 0;
		AddParticipant(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void TrackHolder(RZ_GameModeComponent gm, float dt, array<int> ids)
	{
		if (m_iHolder <= 0 || m_bResolved)
			return;

		if (ids.Contains(m_iHolder))
		{
			m_fHolderGone = 0;
			return;
		}

		m_fHolderGone += dt;
		if (m_fHolderGone < HOLDER_GRACE)
			return;

		m_iHolder = -1;
		OnHolderLost(gm);
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever carried the story item is gone: it went back where it came from.
	protected void OnHolderLost(RZ_GameModeComponent gm)
	{
		NotifyInterested(gm, m_sTitle, string.Format("Whoever was carrying %1 is gone. It found its way back to where it came from.", m_sItemName));
	}

	// ================================================================ Who may act
	//------------------------------------------------------------------------------------------------
	//! Someone the contract was for, or someone who has already been part of the story.
	protected bool MayAct(RZ_GameModeComponent gm, int playerId, bool tell = true)
	{
		if (IsEligible(gm, playerId))
			return true;

		RZ_QuestRun run = Run(gm);
		if (run && run.PlayerTookPart(gm, playerId))
			return true;

		if (tell)
			gm.NotifyPlayer(playerId, m_sTitle, "I don't know you. This isn't your business.");

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The player's access rank with a faction (Stranger 0, Known 1, Trusted 2, Champion 3).
	static int Rank(RZ_GameModeComponent gm, int playerId, int faction)
	{
		if (playerId <= 0 || !RZ_Factions.IsValid(faction))
			return 0;

		return RZ_Reputation.GetAccessRank(gm.GetTier(playerId, faction));
	}

	//------------------------------------------------------------------------------------------------
	static IEntity PlayerEntity(int playerId)
	{
		if (playerId <= 0)
			return null;

		return GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
	}

	// ================================================================ People the story stored
	//------------------------------------------------------------------------------------------------
	//! The leader stored as flag `key` (id) and text `key` (name): only if they're still in their
	//! seat and still the same person (a successor keeps the seat's id, not the name). Else null.
	static RZ_Leader SameLeader(RZ_GameModeComponent gm, RZ_QuestRun run, string key)
	{
		if (!gm || !run || !gm.GetLeaders())
			return null;

		RZ_Leader l = gm.GetLeaders().Get(run.Flag(key, -1));
		if (!l || !l.IsPresent() || l.m_sName != run.Text(key))
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's stronghold leader, if it has one in its seat.
	static RZ_Leader Top(RZ_GameModeComponent gm, int faction)
	{
		if (!gm || !gm.GetLeaders() || !RZ_Factions.IsValid(faction))
			return null;

		RZ_Leader l = gm.GetLeaders().GetStronghold(faction);
		if (!l || !l.IsPresent())
			return null;

		return l;
	}

	//------------------------------------------------------------------------------------------------
	//! "the director" when there's no name to give.
	static string TopName(RZ_GameModeComponent gm, int faction, string fallback)
	{
		RZ_Leader l = Top(gm, faction);
		if (!l)
			return fallback;

		return l.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	//! A Halcyon field lead to build a story on: Independent or Rogue ones first (allowLoyal: any).
	static RZ_Leader PickFieldLead(RZ_GameModeComponent gm, bool allowLoyal)
	{
		if (!gm.GetLeaders())
			return null;

		array<RZ_Leader> all = {};
		gm.GetLeaders().GetFactionLeaders(RZ_EFaction.INSTITUTE, all);
		array<RZ_Leader> best = {};
		array<RZ_Leader> loyal = {};
		foreach (RZ_Leader l : all)
		{
			if (l.IsStronghold() || l.m_iHeldBy >= 0)
				continue;

			if (l.m_eLoyalty == RZ_ELoyalty.LOYAL)
				loyal.Insert(l);
			else
				best.Insert(l);
		}

		if (!best.IsEmpty())
			return best.GetRandomElement();

		if (allowLoyal && !loyal.IsEmpty())
			return loyal.GetRandomElement();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Keeps a leader on the run: flag `key` = id, text `key` = name, text `key`_place = seat.
	static void StoreLeader(RZ_QuestRun run, string key, RZ_Leader l)
	{
		run.SetFlag(key, l.m_iId);
		run.SetText(key, l.m_sName);
		run.SetText(key + "_short", l.m_sShort);
		run.SetText(key + "_place", l.m_sPlace);
	}

	//------------------------------------------------------------------------------------------------
	//! A position kept on the run as two int flags.
	static void StorePos(RZ_QuestRun run, string key, vector pos)
	{
		run.SetFlag(key + "_x", RZ_Util.RoundToInt(pos[0]));
		run.SetFlag(key + "_z", RZ_Util.RoundToInt(pos[2]));
	}

	//------------------------------------------------------------------------------------------------
	static vector LoadPos(RZ_QuestRun run, string key)
	{
		return RZ_Util.GroundPos(Vector(run.Flag(key + "_x"), 0, run.Flag(key + "_z")));
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse that still has people in it.
	static bool HouseLives(RZ_GameModeComponent gm, int idx)
	{
		RZ_Safehouse h = RZ_Questline.GetHouse(gm, idx);
		return h && h.m_iPopulation > 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a stand-in for a seat's people stands: a few metres from the seat, never behind a lock.
	static vector NearSeat(vector seat)
	{
		return RZ_Mission.LandPointNear(seat, 5, 10);
	}

	// ================================================================ Text and cleanup
	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string text = "";
		foreach (RZ_HalcyonSpot s : m_aSpots)
		{
			if (!s.m_bOn || s.m_sName.IsEmpty())
				continue;

			if (!text.IsEmpty())
				text += "\n";

			text += string.Format("%1: grid %2 (%3)", RZ_Util.Capitalize(s.m_sName), RZ_SettlementScanner.GridReference(s.m_vPos), RZ_Util.DirectionText(from, s.m_vPos));
		}

		if (text.IsEmpty())
			return super.GetWhereText(gm, from);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (RZ_HalcyonSpot s : m_aSpots)
		{
			if (s.m_bOn)
				result.Insert(s.m_vPos);
		}
	}
}
