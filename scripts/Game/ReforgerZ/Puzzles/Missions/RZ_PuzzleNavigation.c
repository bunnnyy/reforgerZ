//! Puzzle missions: navigation. See docs/MISSIONS.md, "Puzzle missions".

//------------------------------------------------------------------------------------------------
//! A stash with no prop of its own (Numbers Station, Hot and Cold, Shot at the Line): a grid
//! reference only narrows it to a 100 m square, so it's found within FIND_RADIUS, and the reward
//! crate is put a few steps in front of the finder, toward the hidden point.
class RZ_HiddenStash
{
	static const float FIND_RADIUS = 45;
	protected static const float FOUND_STEP = 8;

	//------------------------------------------------------------------------------------------------
	//! The centre of the 100 m grid square pos is in (RZ_SettlementScanner.GridReference).
	static vector GridSquareCentre(vector pos)
	{
		vector c = pos;
		c[0] = Math.Floor(pos[0] / 100) * 100 + 50;
		c[2] = Math.Floor(pos[2] / 100) * 100 + 50;
		return RZ_Util.GroundPos(c);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the nearest player finds it: FOUND_STEP in front of them, or the point itself.
	static vector FoundAt(array<IEntity> players, vector target)
	{
		float best = -1;
		vector from = target;
		foreach (IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), target);
			if (best >= 0 && d >= best)
				continue;

			best = d;
			from = p.GetOrigin();
		}

		if (best < 0 || best <= FOUND_STEP)
			return target;

		return RZ_Compass.PointAt(from, RZ_Compass.Bearing(from, target), FOUND_STEP);
	}
}

//------------------------------------------------------------------------------------------------
//! #1 The Numbers Station: a note gives a frequency; only one of three radio masts
//! carries it. The broadcast is a grid reference with every third number a decoy.
//! ★ Radio Operator: hears the broadcast without the note or the mast.
class RZ_Mission_NumbersStation : RZ_PuzzleMission
{
	protected vector m_vStash;
	protected int m_iRightMast;
	protected string m_sBroadcast;
	protected ref array<vector> m_aMasts = {};
	protected ref array<int> m_aToldOps = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3600;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || !HasPrefab(gm, RZ_EPuzzleProp.RADIO_MAST))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 70);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		// The broadcast is a grid square: the stash sits in its middle
		m_vStash = RZ_HiddenStash.GridSquareCentre(LandPointNear(s.m_vCenter, s.m_fRadius + 300, s.m_fRadius + 1500));
		if (RZ_Util.IsWater(m_vStash))
			return false;

		// Three masts on high ground, at different bearings from the town
		array<string> freqs = {};
		float baseAngle = m_Rng.Range(0, 360);
		for (int i = 0; i < 3; i++)
		{
			vector around = RZ_Compass.PointAt(s.m_vCenter, baseAngle + i * 120 + m_Rng.Range(-20, 20), m_Rng.Range(500, 1300));
			vector mast = HighPointNear(around, 0, 250, 12);
			m_aMasts.Insert(mast);

			string f = string.Format("%1.%2", m_Rng.Int(3, 10), RZ_NoteText.Pad(m_Rng.Int(0, 1000), 3));
			while (freqs.Contains(f))
			{
				f = string.Format("%1.%2", m_Rng.Int(3, 10), RZ_NoteText.Pad(m_Rng.Int(0, 1000), 3));
			}

			freqs.Insert(f);
			SpawnObject(gm, RZ_EPuzzleProp.RADIO_MAST, mast, i, string.Format("Tune the receiver to %1 MHz and listen", f), "");
		}

		m_iRightMast = m_Rng.Int(0, 3);

		// Grid "47-112" -> digits 0 4 7 1 1 2, with a decoy after every second real digit
		int gx = Math.Floor(m_vStash[0] / 100);
		int gz = Math.Floor(m_vStash[2] / 100);
		string digits = RZ_NoteText.Pad(gx, 3) + RZ_NoteText.Pad(gz, 3);
		m_sBroadcast = "";
		int spoken = 0;
		for (int d = 0; d < 6; d++)
		{
			m_sBroadcast += RZ_NoteText.DigitWord(digits.Substring(d, 1).ToInt()) + ". ";
			spoken++;
			if (spoken % 2 == 0)
				m_sBroadcast += RZ_NoteText.DigitWord(m_Rng.Int(0, 10)) + ". ";
		}

		string author = RZ_NoteText.FullName(m_Rng);
		int note = AddClue("A radio log", string.Format("%1 MHz. Every night at the hour, a woman reading numbers. Not the quarantine loop, something else.\nI think it's a map grid: three numbers across, three up. Every third number is padding, skip it. I wrote them down twice and the padding never matches.\n%2\n%3",
			freqs[m_iRightMast], RZ_NoteText.PersonalLine(m_Rng, s.m_sName), RZ_NoteText.Signoff(m_Rng, author)));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4), note, "Read the radio log");

		m_sTitle = "The Numbers Station";
		m_sBrief = string.Format("A Haven radio man heard numbers on the air before he went quiet. His log is somewhere in %1. Find the frequency, find the mast that carries it, and work out where the numbers point.", s.m_sName);
		m_sObjective = string.Format("Find the radio man's log in %1 and where the numbers point.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string masts = "";
		foreach (int i, vector p : m_aMasts)
		{
			if (i > 0)
				masts += ", ";

			masts += RZ_SettlementScanner.GridReference(p);
		}

		return super.GetWhereText(gm, from) + "\nRadio masts (grid): " + masts;
	}

	//------------------------------------------------------------------------------------------------
	protected string BroadcastText()
	{
		return "A woman's voice, flat and patient: \"" + m_sBroadcast + "\" Then it starts again.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx == m_iRightMast)
		{
			gm.NotifyPlayer(playerId, "The numbers", BroadcastText());
			return;
		}

		gm.NotifyPlayer(playerId, "Static", "\"This area is under federal quarantine. Remain indoors. Aid is on its way.\" The same loop as everywhere else.");
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (m_aToldOps.Contains(playerId) || !IsInvolved(playerId) || !RZ_Radio.IsRadioOperator(gm, playerId))
				continue;

			if (RZ_Util.DistXZ(p.GetOrigin(), m_vPos) > 2500)
				continue;

			m_aToldOps.Insert(playerId);
			AddParticipant(playerId);
			gm.NotifyPlayer(playerId, "On your set", "You find it yourself, between two dead channels. " + BroadcastText() + " Every third number is padding: the rhythm gives it away.");
		}

		if (ReachedBy(ids, players, m_vStash, RZ_HiddenStash.FIND_RADIUS))
		{
			m_vStash = RZ_HiddenStash.FoundAt(players, m_vStash);
			m_eState = RZ_EMissionState.SUCCEEDED;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vStash);
		NotifyParticipants(gm, m_sTitle, "Under a tarp at the grid: a stash, and a dead radio someone was too scared to keep.");
	}
}

//------------------------------------------------------------------------------------------------
//! #2 Two Bearings: two pieces of a helicopter carry scratched bearings. Draw both
//! lines on the map; where they cross is where the pilot went. ★ Hunter/Soldier: more Fame.
class RZ_Mission_TwoBearings : RZ_PuzzleMission
{
	protected vector m_vTarget;
	protected ref array<vector> m_aPieces = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("hunter");
		m_Req.Bonus("soldier");
		m_fBonusFameMult = 1.5;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 90);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vTarget = LandPointNear(s.m_vCenter, s.m_fRadius + 200, s.m_fRadius + 1200);
		// Never the answer: the journal and map show the town; the pieces' grids are the clue
		m_vPos = s.m_vCenter;

		// Bearing from the target out to each piece; the lines cross at 55-125 degrees
		float out1 = m_Rng.Range(0, 360);
		float side = 1;
		if (m_Rng.Chance(0.5))
			side = -1;

		float out2 = out1 + side * m_Rng.Range(55, 125);
		vector p1 = RZ_Compass.PointAt(m_vTarget, out1, m_Rng.Range(350, 800));
		vector p2 = RZ_Compass.PointAt(m_vTarget, out2, m_Rng.Range(350, 800));
		if (RZ_Util.IsWater(p1) || RZ_Util.IsWater(p2))
			return false;

		m_aPieces.Insert(p1);
		m_aPieces.Insert(p2);

		string pilot = RZ_NoteText.FullName(m_Rng);
		int c1 = AddClue("Wreckage", string.Format("A torn door panel. Scratched into the paint with a knife:\n\"%1 alive. Went %2 toward the smoke. Follow.\"", pilot, RZ_Compass.BearingText(RZ_Compass.Bearing(p1, m_vTarget))));
		int c2 = AddClue("Wreckage", string.Format("The tail boom. Written in grease pencil, shaky:\n\"Co-pilot. Can't walk. Saw smoke at %1. If that's him, tell him I waited. %2\"", RZ_Compass.BearingText(RZ_Compass.Bearing(p2, m_vTarget)), RZ_NoteText.Day(m_Rng.Int(3, 11))));
		PlacePiece(gm, p1, c1);
		PlacePiece(gm, p2, c2);

		m_sTitle = "Two Bearings";
		m_sBrief = string.Format("A Bastion helicopter came down in pieces near %1. The pilot walked away from it. Two pieces of the wreck have bearings scratched on them. Draw both lines on your map: he's where they cross.", s.m_sName);
		m_sObjective = string.Format("Find the pilot from the wreck near %1 where the two bearings cross.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The helicopter wreck prefab if it's readable, otherwise a note on the ground.
	protected void PlacePiece(RZ_GameModeComponent gm, vector pos, int clueIdx)
	{
		IEntity wreck = RZ_Util.SpawnPrefab(gm.GetHeliWreckPrefab(), RZ_Util.GroundPos(pos), m_Rng.Range(0, 360));
		TrackProp(wreck);
		if (TagClue(wreck, clueIdx, "Look at the scratches"))
			return;

		SpawnClue(gm, RZ_EPuzzleProp.CLUE, RZ_Util.RandomPointInRing(pos, 2, 4), clueIdx, "Look at the scratches");
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		return GetLocationName(gm) + "\nWreck pieces (grid): " + RZ_SettlementScanner.GridReference(m_aPieces[0]) + ", " + RZ_SettlementScanner.GridReference(m_aPieces[1]);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (ReachedBy(ids, players, m_vTarget, 25))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vTarget);
		NotifyParticipants(gm, m_sTitle, "You find him sitting against a tree, flight helmet in his lap. He didn't make it. His survival kit did.");
	}
}
