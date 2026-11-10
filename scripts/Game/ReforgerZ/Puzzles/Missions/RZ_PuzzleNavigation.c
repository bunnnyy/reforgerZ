//! Puzzle missions: navigation. See docs/MISSIONS.md, "Puzzle missions".

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
		m_vStash = LandPointNear(s.m_vCenter, s.m_fRadius + 300, s.m_fRadius + 1500);

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

		if (ReachedBy(ids, players, m_vStash, 10))
			m_eState = RZ_EMissionState.SUCCEEDED;
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
		m_vPos = m_vTarget;

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

//------------------------------------------------------------------------------------------------
//! #3 Paces from the Tower: "From the church, face the water tower, turn right,
//! 400 paces." Built from real landmarks in the town (or a fallback prop).
//! ★ Hunter: the note's paces also come in metres.
class RZ_Mission_PacesFromTheTower : RZ_PuzzleMission
{
	protected vector m_vTarget;
	protected vector m_vBody;
	protected int m_iNote = -1;
	protected float m_fMetres;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 12, 12);
		m_Req.Bonus("hunter");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2700;
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
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		array<string> kinds = {"church", "watertower", "tower", "school", "castle", "lighthouse", "station", "monument", "fuel", "well"};
		array<ref RZ_Landmark> found = {};
		RZ_LandmarkFinder.Find(s.m_vCenter, s.m_fRadius * 1.2, kinds, found);

		// Two landmarks 60-450 m apart; one may be the fallback prop
		RZ_Landmark from;
		RZ_Landmark face;
		foreach (RZ_Landmark a : found)
		{
			foreach (RZ_Landmark b : found)
			{
				float d = RZ_Util.DistXZ(a.m_vPos, b.m_vPos);
				if (a != b && a.m_sName != b.m_sName && d >= 60 && d <= 450)
				{
					from = a;
					face = b;
					break;
				}
			}

			if (from)
				break;
		}

		if (!from)
		{
			if (found.IsEmpty())
				return false;

			face = found[0];
			from = FindOrPlaceLandmark(gm, s.m_vCenter, 0, kinds, LandPointNear(face.m_vPos, 120, 250));
			if (!from)
				return false;
		}

		// Face the second landmark, turn right (or left if right ends in water), walk N paces
		string turn = "right";
		bool ok = false;
		for (int i = 0; i < 6; i++)
		{
			int paces = m_Rng.Int(15, 55) * 10;
			float heading = RZ_Compass.Bearing(from.m_vPos, face.m_vPos) + 90;
			turn = "right";
			if (i % 2 == 1)
			{
				heading -= 180;
				turn = "left";
			}

			m_fMetres = RZ_Compass.PacesToMetres(paces);
			m_vTarget = RZ_Compass.PointAt(from.m_vPos, heading, m_fMetres);
			if (!RZ_Util.IsWater(m_vTarget))
			{
				ok = true;
				SetupNote(gm, s, from, face, turn, paces);
				break;
			}
		}

		if (!ok)
			return false;

		m_sTitle = "Paces from the Tower";
		m_sBrief = string.Format("A man was found dead in %1 with directions in his pocket. Whatever he hid, it's still out there. His body is near grid %2.", s.m_sName, RZ_SettlementScanner.GridReference(m_vBody));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupNote(RZ_GameModeComponent gm, RZ_Settlement s, RZ_Landmark from, RZ_Landmark face, string turn, int paces)
	{
		string name = RZ_NoteText.FirstName(m_Rng);
		m_iNote = AddClue("Directions", string.Format("In pencil, folded small:\n\"From %1, face %2, turn %3, %4 paces. Under the flat stone. For Ana, not for anyone else.\"\n%5\n%6",
			from.m_sName, face.m_sName, turn, paces, RZ_NoteText.PersonalLine(m_Rng, s.m_sName), RZ_NoteText.Signoff(m_Rng, name)));

		m_vBody = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.5);
		SpawnClue(gm, RZ_EPuzzleProp.BODY, m_vBody, m_iNote, "Search his pockets");
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iNote && HasBonus(gm, playerId))
			text += string.Format("\n(Your stride says that's about %1 metres.)", RZ_Util.RoundToInt(m_fMetres));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		if (ReachedBy(ids, players, m_vTarget, 12))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vTarget);
		NotifyParticipants(gm, m_sTitle, "A flat stone, and under it a tin box wrapped in a coat. Ana never came for it.");
	}
}

//------------------------------------------------------------------------------------------------
//! #4 Hot and Cold: a survivor's beacon in the countryside. Carry a radio and you
//! hear it get louder. ★ Radio Operator: a numeric signal strength.
class RZ_Mission_HotAndCold : RZ_PuzzleMission
{
	protected vector m_vBeacon;
	protected ref map<int, float> m_mLastHint = new map<int, float>();
	protected ref map<int, int> m_mLastLevel = new map<int, int>();

	protected static const float HINT_GAP = 15;
	protected static const float HEAR_RANGE = 2000;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.STRANGER, 12, 15);
		m_Req.Bonus("radioham");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2700;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 100);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_vBeacon = LandPointNear(s.m_vCenter, s.m_fRadius + 300, s.m_fRadius + 900);
		m_sTitle = "Hot and Cold";
		m_sBrief = string.Format("A survivor's emergency beacon is chirping somewhere in the fields around %1. Carry a radio and you'll hear it: the louder it gets, the closer you are. It's small. Get within a few steps.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 = nothing, 1 faint, 2 clear, 3 loud, 4 very loud
	protected static int Level(float d)
	{
		if (d < 60)
			return 4;
		if (d < 200)
			return 3;
		if (d < 500)
			return 2;
		if (d < 1200)
			return 1;
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static string LevelText(int level)
	{
		switch (level)
		{
			case 4: return "The chirp is right on top of you. Look down.";
			case 3: return "Loud and steady. Close now.";
			case 2: return "Clear. You're heading the right way, or near it.";
			case 1: return "Faint, under the static.";
		}

		return "Nothing but static.";
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUpdate(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			if (!IsInvolved(playerId))
				continue;

			float d = RZ_Util.DistXZ(p.GetOrigin(), m_vBeacon);
			if (d > HEAR_RANGE)
				continue;

			float last;
			if (m_mLastHint.Find(playerId, last) && m_fClock - last < HINT_GAP)
				continue;

			if (!RZ_Radio.HasRadio(gm, playerId, p))
				continue;

			m_mLastHint.Set(playerId, m_fClock);
			AddParticipant(playerId);
			int level = Level(d);
			if (RZ_Radio.IsRadioOperator(gm, playerId))
			{
				int strength = Math.Clamp(RZ_Util.RoundToInt(100 - d / 20), 0, 100);
				gm.NotifyPlayer(playerId, "Beacon", string.Format("Signal strength %1 percent.", strength));
				continue;
			}

			int previous = -1;
			m_mLastLevel.Find(playerId, previous);
			if (previous == level && level < 3)
				continue;

			m_mLastLevel.Set(playerId, level);
			gm.NotifyPlayer(playerId, "Beacon", LevelText(level));
		}

		if (ReachedBy(ids, players, m_vBeacon, 10))
			m_eState = RZ_EMissionState.SUCCEEDED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vBeacon);
		NotifyParticipants(gm, m_sTitle, "A beacon taped to a fence post, and a bag beside it. Whoever set it up didn't come back to switch it off.");
	}
}
