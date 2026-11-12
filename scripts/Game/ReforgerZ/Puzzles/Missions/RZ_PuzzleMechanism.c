//! Puzzle missions: mechanisms. See docs/MISSIONS.md, "Puzzle missions".

//------------------------------------------------------------------------------------------------
//! #13 Junction Boxes (Bastion): four fuse boxes with two switches each. Every switch
//! flips its own lamp and one or two others (a lights-out puzzle). The start state
//! is generated so the shortest fix takes 3-6 presses. Each box shows its lamps and
//! the wiring labels; an electrician's note narrows it down. All green opens the
//! substation. ★ Mechanic: the note names one more switch.
class RZ_Mission_JunctionBoxes : RZ_PuzzleMission
{
	protected ref array<int> m_aMasks = {};
	protected ref array<vector> m_aBoxes = {};
	protected int m_iState;			// bit set = lamp red
	protected int m_iSolution;		// a shortest set of presses (bit per switch)
	protected int m_iNote = -1;
	protected int m_iFirstBoxClue = -1;
	protected int m_iExtraHint = -1;	// ★ Mechanic: one more switch from the shortest fix

	protected static const int SWITCHES = 8;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.MILITARY, RZ_ETier.STRANGER, 15, 15);
		m_Req.Bonus("mechanic");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 2700;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.FUSEBOX) || !HasPrefab(gm, RZ_EPuzzleProp.CLUE))
			return 0;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	protected static string SwitchName(int k)
	{
		string boxes = "ABCD";
		int n = k % 2 + 1;
		return boxes.Substring(k / 2, 1) + n.ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected static int CountBits(int v)
	{
		int n = 0;
		for (int i = 0; i < SWITCHES; i++)
		{
			if ((v & (1 << i)) != 0)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int Apply(int presses)
	{
		int state = 0;
		for (int i = 0; i < SWITCHES; i++)
		{
			if ((presses & (1 << i)) != 0)
				state = state ^ m_aMasks[i];
		}

		return state;
	}

	//------------------------------------------------------------------------------------------------
	//! Fewest presses that turn `state` all green (brute force over 256 sets). -1 if none.
	protected int ShortestSolution(int state)
	{
		int best = -1;
		int bestCount = SWITCHES + 1;
		for (int presses = 0; presses < (1 << SWITCHES); presses++)
		{
			if (Apply(presses) != state)
				continue;

			int c = CountBits(presses);
			if (c < bestCount)
			{
				bestCount = c;
				best = presses;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		m_iSettlement = PickSettlement(gm, 0, 80);
		RZ_Settlement s = gm.GetSettlement(m_iSettlement);
		if (!s)
			return false;

		m_vPos = s.m_vCenter;
		m_fParticipationRadius = s.m_fRadius + 50;

		bool ok = false;
		for (int attempt = 0; attempt < 50 && !ok; attempt++)
		{
			m_aMasks.Clear();
			for (int k = 0; k < SWITCHES; k++)
			{
				int mask = 1 << k;
				int extra = m_Rng.Int(1, 3);
				for (int e = 0; e < extra; e++)
				{
					int other = m_Rng.Int(0, SWITCHES);
					mask = mask | (1 << other);
				}

				m_aMasks.Insert(mask);
			}

			int presses = 0;
			int wanted = m_Rng.Int(3, 7);
			while (CountBits(presses) < wanted)
			{
				presses = presses | (1 << m_Rng.Int(0, SWITCHES));
			}

			m_iState = Apply(presses);
			if (m_iState == 0)
				continue;

			m_iSolution = ShortestSolution(m_iState);
			int len = CountBits(m_iSolution);
			ok = m_iSolution >= 0 && len >= 3 && len <= 6;
		}

		if (!ok)
			return false;

		float baseAngle = m_Rng.Range(0, 360);
		for (int b = 0; b < 4; b++)
		{
			vector around = RZ_Compass.PointAt(s.m_vCenter, baseAngle + b * 90 + m_Rng.Range(-25, 25), s.m_fRadius * m_Rng.Range(0.35, 0.8));
			vector pos = LandPointNear(around, 0, 20);
			m_aBoxes.Insert(pos);
			int clue = AddClue("Fuse box " + SwitchName(b * 2).Substring(0, 1), "");
			if (b == 0)
				m_iFirstBoxClue = clue;

			SpawnObject(gm, RZ_EPuzzleProp.FUSEBOX, pos, b, string.Format("Flip switch %1|Flip switch %2", SwitchName(b * 2), SwitchName(b * 2 + 1)), "", clue, "Look at the lamps");
		}

		// The note: one switch that's in the shortest fix, one that isn't
		int inFix = PickSwitch(true, -1);
		int notInFix = PickSwitch(false, -1);
		string electrician = RZ_NoteText.FirstName(m_Rng);
		string hint = string.Format("%1 has to be thrown, whatever else you do.", SwitchName(inFix));
		if (notInFix >= 0)
			hint += string.Format(" Leave %1 alone, it only makes things worse.", SwitchName(notInFix));

		m_iNote = AddClue("Electrician's note", string.Format("Taped inside the substation door:\n\"Whoever wired this was drunk. Every switch trips its own lamp and some others, it's written under each one. All eight green and the door unlocks. %1\"\n- %2", hint, electrician));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, LandPointNear(s.m_vCenter, 0, 15), m_iNote, "Read the note");
		m_iExtraHint = PickSwitch(true, inFix);

		m_sTitle = "Junction Boxes";
		m_sBrief = string.Format("Bastion needs the substation in %1 open. Four fuse boxes around town control the lock, and someone wired them in a hurry. Get every lamp green. The electrician left a note at the substation door.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A random switch in (or not in) the shortest fix, other than `except`. -1 if none.
	protected int PickSwitch(bool inSolution, int except)
	{
		array<int> pool = {};
		for (int k = 0; k < SWITCHES; k++)
		{
			bool isIn = (m_iSolution & (1 << k)) != 0;
			if (isIn == inSolution && k != except)
				pool.Insert(k);
		}

		if (pool.IsEmpty())
			return -1;

		return pool[m_Rng.Int(0, pool.Count())];
	}

	//------------------------------------------------------------------------------------------------
	protected string LampText(int k)
	{
		if ((m_iState & (1 << k)) != 0)
			return "red";

		return "green";
	}

	//------------------------------------------------------------------------------------------------
	protected string BoxStatus(int b)
	{
		string text = "";
		for (int v = 0; v < 2; v++)
		{
			int k = b * 2 + v;
			string trips = "";
			for (int j = 0; j < SWITCHES; j++)
			{
				if (j == k || (m_aMasks[k] & (1 << j)) == 0)
					continue;

				if (!trips.IsEmpty())
					trips += ", ";

				trips += SwitchName(j);
			}

			text += string.Format("Lamp %1: %2. Under the switch: \"also trips %3\"\n", SwitchName(k), LampText(k), trips);
		}

		text += string.Format("Main panel: %1 of 8 green.", SWITCHES - CountBits(m_iState));
		return text;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		if (m_iFirstBoxClue >= 0 && idx >= m_iFirstBoxClue && idx < m_iFirstBoxClue + 4)
			return BoxStatus(idx - m_iFirstBoxClue);

		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iNote && HasBonus(gm, playerId) && m_iExtraHint >= 0)
			text += string.Format("\n(You trace the cables yourself: %1 has to be thrown too.)", SwitchName(m_iExtraHint));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (objIdx < 0 || objIdx > 3 || value < 0 || value > 1)
			return;

		int k = objIdx * 2 + value;
		m_iState = m_iState ^ m_aMasks[k];
		gm.GetHeat().AddHeat(m_aBoxes[objIdx], 2);

		if (m_iState == 0)
		{
			NotifyParticipants(gm, m_sTitle, "Every lamp is green. Across town, the substation lock clunks open.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		gm.NotifyPlayer(playerId, "Clunk", "Somewhere a relay knocks.\n" + BoxStatus(objIdx));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vPos);
	}
}

//------------------------------------------------------------------------------------------------
//! #14 The Weighing Vault (Haven): a miller's scale crate opens when what's inside
//! matches a weight given in a riddle (within 0.2 kg). Feedback is "too heavy / too
//! light". With "Scale counts items" on, it counts items instead of weighing them.
//! ★ Mechanic: reads the needle to a tenth of a kilo.
class RZ_Mission_WeighingVault : RZ_PuzzleMission
{
	protected vector m_vScale;
	protected float m_fTarget;
	protected bool m_bCount;

	protected static const float TOLERANCE = 0.2;

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, 15, 20);
		m_Req.Bonus("mechanic");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 3000;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.SCALE) || !HasPrefab(gm, RZ_EPuzzleProp.CLUE))
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
		m_vScale = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.4);
		m_bCount = gm.ScaleCountsItems();
		if (!SpawnObject(gm, RZ_EPuzzleProp.SCALE, m_vScale, 0, "Pull the lever", ""))
			return false;

		string miller = RZ_NoteText.FullName(m_Rng);
		string riddle;
		if (m_bCount)
		{
			int n = m_Rng.Int(3, 10);
			m_fTarget = n;
			riddle = string.Format("My scale is old and honest and it only counts. Feed it %1 things, no more, no fewer, then pull the lever.", RZ_NoteText.NumberPhrase(n));
		}
		else
		{
			array<int> bigs = {12, 10, 9, 8, 7};
			array<int> smalls = {2, 3, 4, 5};
			int big = bigs[m_Rng.Int(0, bigs.Count())];
			int small = smalls[m_Rng.Int(0, smalls.Count())];
			m_fTarget = big - small;
			string thumb = "";
			if (m_Rng.Chance(0.5))
			{
				m_fTarget += 0.5;
				thumb = " Then add half a kilo for the miller's thumb.";
			}

			riddle = string.Format("My scale only opens for an honest weight. Take %1 kilos, give back %2.%3 Put it in the crate and pull the lever.",
				RZ_NoteText.NumberPhrase(big), RZ_NoteText.NumberPhrase(small), thumb);
		}

		int clue = AddClue("The miller's riddle", riddle + "\n" + RZ_NoteText.Signoff(m_Rng, miller));
		SpawnClue(gm, RZ_EPuzzleProp.CLUE, RZ_Util.RandomPointInRing(m_vScale, 2, 5), clue, "Read the riddle");

		m_sTitle = "The Weighing Vault";
		m_sBrief = string.Format("The old miller in %1 kept his savings in a vault that only opens for the right weight on his scale. The Haven could use what's inside. His riddle is pinned up next to it.", s.m_sName);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		float measured;
		if (m_bCount)
			measured = RZ_ScaleHelper.CountItems(source);
		else
			measured = RZ_ScaleHelper.GetContentWeight(source);

		if (Math.AbsFloat(measured - m_fTarget) <= TOLERANCE)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "The needle settles, trembles, and stays. Below the floor something heavy slides back.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		gm.GetHeat().AddHeat(m_vScale, 3);
		string verdict = "The needle swings past the mark. Too heavy.";
		if (measured < m_fTarget)
			verdict = "The needle barely moves. Too light.";

		if (m_bCount)
			verdict = string.Format("The counter clicks to %1. Not what it wants.", RZ_Util.RoundToInt(measured));
		else if (HasBonus(gm, playerId))
			verdict += string.Format(" (You read it at %1 kg.)", FormatKg(measured));

		m_iWrong++;
		if (m_iWrong % 4 == 0)
		{
			LockOut(120);
			verdict += " The lever jams. Give it a couple of minutes.";
		}

		gm.NotifyPlayer(playerId, m_sTitle, verdict);
	}

	//------------------------------------------------------------------------------------------------
	protected static string FormatKg(float kg)
	{
		int tenths = RZ_Util.RoundToInt(kg * 10);
		return string.Format("%1.%2", tenths / 10, tenths % 10);
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		SpawnRewardAt(gm, m_vScale);
	}
}

//! Reads what's inside a crate. Isolated so the weight call is easy to fix.
class RZ_ScaleHelper
{
	//------------------------------------------------------------------------------------------------
	protected static void GetContents(IEntity crate, notnull array<IEntity> items)
	{
		if (!crate)
			return;

		BaseInventoryStorageComponent storage = BaseInventoryStorageComponent.Cast(crate.FindComponent(BaseInventoryStorageComponent));
		if (!storage)
			return;

		storage.GetAll(items);
	}

	//------------------------------------------------------------------------------------------------
	//! Total weight of the items in the crate (not the crate itself), kg.
	static float GetContentWeight(IEntity crate)
	{
		array<IEntity> items = {};
		GetContents(crate, items);
		float total = 0;
		foreach (IEntity item : items)
		{
			if (!item)
				continue;

			InventoryItemComponent iic = InventoryItemComponent.Cast(item.FindComponent(InventoryItemComponent));
			if (iic)
				total += iic.GetTotalWeight(); // VERIFY: GetTotalWeight() (weight plus "additional weight") includes a bag's contents
		}

		return total;
	}

	//------------------------------------------------------------------------------------------------
	static int CountItems(IEntity crate)
	{
		array<IEntity> items = {};
		GetContents(crate, items);
		return items.Count();
	}
}

//------------------------------------------------------------------------------------------------
//! #15 The Last Experiment (Halcyon): a lab terminal's password is a short word.
//! Each missing researcher's belongings, spread over 4-5 settlements, carry one
//! letter. Collect them, unscramble, and type it on the terminal keypad
//! (phone-style letters), or, without a keypad prefab, pick it from four words.
//! Three wrong tries wipe the terminal. ★ Lab Tech: the screen shows the first letter.
class RZ_Mission_LastExperiment : RZ_PuzzleMission
{
	protected string m_sWord;
	protected int m_iRight = -1;		// choice mode: the right option
	protected bool m_bKeypad;
	protected int m_iScreen = -1;
	protected vector m_vTerminal;
	protected ref array<vector> m_aBelongings = {};

	//------------------------------------------------------------------------------------------------
	override void Configure(RZ_GameModeComponent gm)
	{
		SetContract(RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, 25, 25);
		m_Req.Bonus("labtech");
		m_fBonusFameMult = 1.25;
		m_fTimeLeft = 4800;
		m_fParticipationRadius = 0;
	}

	//------------------------------------------------------------------------------------------------
	override float GetWeight(RZ_GameModeComponent gm)
	{
		if (!HasPrefab(gm, RZ_EPuzzleProp.CLUE) || gm.GetSettlements().Count() < 6)
			return 0;

		if (!HasPrefab(gm, RZ_EPuzzleProp.TERMINAL) && !HasPrefab(gm, RZ_EPuzzleProp.OBJECT))
			return 0;

		return 0.8;
	}

	//------------------------------------------------------------------------------------------------
	//! Phone keypad digits for a word: ABC=2 ... WXYZ=9.
	static string ToKeypad(string word)
	{
		array<string> keys = {"ABC", "DEF", "GHI", "JKL", "MNO", "PQRS", "TUV", "WXYZ"};
		string upper = word;
		upper.ToUpper();
		string digits = "";
		int len = upper.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = upper.Substring(i, 1);
			foreach (int k, string group : keys)
			{
				int key = k + 2;
				if (group.Contains(ch))
					digits += key.ToString();
			}
		}

		return digits;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		// No word here is an anagram of another common word: the letters spell exactly one answer
		array<string> words4 = {"FAWN", "DOVE", "HYMN", "MOTH", "WREN", "HUSK"};
		array<string> words5 = {"EMBER", "MERCY", "ROWAN", "LINEN", "ASHES", "BIRCH"};
		array<string> pool = words4;
		if (m_Rng.Chance(0.5))
			pool = words5;

		m_Rng.ShuffleStrings(pool);
		m_sWord = pool[0];
		int letters = m_sWord.Length();

		m_vTerminal = gm.GetLabPosition();
		if (!gm.HasLab())
		{
			m_iSettlement = PickSettlement(gm, 0, 70);
			RZ_Settlement s = gm.GetSettlement(m_iSettlement);
			if (!s)
				return false;

			m_vTerminal = LandPointNear(s.m_vCenter, 0, s.m_fRadius * 0.3);
		}
		else
		{
			m_iSettlement = gm.GetLabSettlement();
		}

		m_vPos = m_vTerminal;
		array<int> used = {m_iSettlement};
		array<int> towns = {};
		CollectSettlements(gm, m_vTerminal, 300, 4000, used, towns);
		if (towns.Count() < letters)
			return false;

		// One researcher per letter, letters handed out in shuffled order
		array<string> names = {};
		RZ_NoteText.DistinctFirstNames(m_Rng, letters, names);
		array<int> order = {};
		for (int i = 0; i < letters; i++)
		{
			order.Insert(i);
		}

		m_Rng.ShuffleInts(order);
		for (int j = 0; j < letters && j < names.Count(); j++)
		{
			RZ_Settlement t = gm.GetSettlement(towns[j]);
			vector pos = LandPointNear(t.m_vCenter, 0, t.m_fRadius * 0.5);
			m_aBelongings.Insert(pos);
			string letter = m_sWord.Substring(order[j], 1);
			int clue = AddClue("Halcyon field notebook", string.Format("%1 %2's notebook. Water-stained readings, then nothing. On the last page one letter, traced over and over until the pen went through: \"%3\".\n%4",
				names[j], RZ_NoteText.Surname(m_Rng), letter, RZ_NoteText.PersonalLine(m_Rng, t.m_sName)));
			SpawnClue(gm, RZ_EPuzzleProp.BODY, pos, clue, "Search the belongings");
		}

		m_bKeypad = HasPrefab(gm, RZ_EPuzzleProp.TERMINAL);
		if (m_bKeypad)
		{
			m_iScreen = AddClue("Terminal", string.Format("PROJECT LAZARUS. LOCKED.\nPassword: %1 letters. Each of the team kept one.\nKeys: 2 ABC  3 DEF  4 GHI  5 JKL  6 MNO  7 PQRS  8 TUV  9 WXYZ. Type it, then Enter.\nAttempts before wipe: 3", letters));
			SpawnObject(gm, RZ_EPuzzleProp.TERMINAL, m_vTerminal, 0, "", "", m_iScreen, "Read the screen");
		}
		else
		{
			array<string> options = {};
			for (int o = 0; o < 4; o++)
			{
				options.Insert(pool[o]);
			}

			m_Rng.ShuffleStrings(options);
			m_iRight = options.Find(m_sWord);
			string labels = string.Format("Type %1|Type %2|Type %3|Type %4", options[0], options[1], options[2], options[3]);
			m_iScreen = AddClue("Terminal", string.Format("PROJECT LAZARUS. LOCKED.\nPassword: %1 letters. Each of the team kept one.\nAttempts before wipe: 3", letters));
			SpawnObject(gm, RZ_EPuzzleProp.OBJECT, m_vTerminal, 0, "", labels, m_iScreen, "Read the screen");
		}

		m_sTitle = "The Last Experiment";
		m_sBrief = string.Format("A Halcyon terminal holds the last results of a team that went into the field and didn't come back. Each of them kept one letter of the password. Find their belongings, %1 of them, in the towns around, and open the terminal.", letters);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override string GetWhereText(RZ_GameModeComponent gm, vector from)
	{
		string where = "";
		foreach (int i, vector p : m_aBelongings)
		{
			if (i > 0)
				where += ", ";

			where += PlaceText(gm, p);
		}

		return super.GetWhereText(gm, from) + "\nLast known positions: " + where;
	}

	//------------------------------------------------------------------------------------------------
	override string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		string text = super.GetClueText(gm, playerId, idx);
		if (idx == m_iScreen && HasBonus(gm, playerId))
			text += string.Format("\n(You know Halcyon's habits: the password always starts with the project lead's letter, %1.)", m_sWord.Substring(0, 1));

		return text;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCodeEntered(RZ_GameModeComponent gm, int playerId, int objIdx, string code, IEntity source)
	{
		Answer(gm, playerId, code == ToKeypad(m_sWord));
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		Answer(gm, playerId, option == m_iRight);
	}

	//------------------------------------------------------------------------------------------------
	protected void Answer(RZ_GameModeComponent gm, int playerId, bool right)
	{
		if (right)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "ACCESS GRANTED. The screen fills with numbers, then a video file named for each of them.");
			m_eState = RZ_EMissionState.SUCCEEDED;
			return;
		}

		m_iWrong++;
		SoundAlarm(gm, m_vTerminal, 15, false);
		if (m_iWrong >= 3)
		{
			gm.NotifyPlayer(playerId, m_sTitle, "WIPING. The drive grinds for a long time. When it stops, there's nothing left.");
			m_eState = RZ_EMissionState.FAILED;
			return;
		}

		LockOut(300);
		gm.NotifyPlayer(playerId, m_sTitle, string.Format("ACCESS DENIED. %1 attempts left. The terminal locks for five minutes and beeps loudly.", 3 - m_iWrong));
	}

	//------------------------------------------------------------------------------------------------
	override void OnSuccess(RZ_GameModeComponent gm)
	{
		gm.GetCure().AddResearch(5, "The terminal of a Halcyon team that never came back from the field is open. Their final results are in the lab's record, under all their names.");
	}
}
