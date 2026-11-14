//! The sorting duties, played in the world (docs/ERRANDS.md, "The sorting duties"): the dead laid
//! out, the people waiting at the gate, the sample trays on the bench, the crates in the store.
//! Each subject is a thing standing or lying at the place, each person in their own clothes. You go
//! to it and use your hands: its one prompt (its name, RZ_WorkSites.Gather) lists three places on it
//! to look (a hold each, and fewer looks than there are places, so you choose) and taking it up, under
//! the rules, the looks left and what you've seen of it; a tell is free when you walk up. Then you
//! carry it to one of three named places close by (the gate, the wash, the holding room), each a
//! thing that reads as what it is, and leave it there: that's the verdict. The line at a gate stands
//! at the place's real way in. Every prompt is a work site (Missions/Blocks/RZ_WorkSites.c): held,
//! animated, the world showing what you did. The tracker says where the three places are while
//! you're leading one; the clipboard at the place is there to read but never needed.
//! The server holds the truth and judges: right is full, too careful is half, too careless is
//! nothing, and a bad one let through is the dangerous miss. How many are bad follows the world
//! (the sickest town near, the outbreak's stage). Owned by the errand dealer. Server only.

//------------------------------------------------------------------------------------------------
class RZ_DutySubject
{
	string m_sTitle;				// "Tag 114", "A courier", "Tray 15"
	string m_sTell;					// what's free to see
	int m_iTruth;					// RZ_DutyGame.CLEAN, ODD or BAD
	int m_iBadSign;					// which of the first two signs shows it (a bad one)
	ref array<bool> m_aLooked = {false, false, false};
	int m_iVerdict = -1;			// RZ_DutyGame.PASS, CHECK or STOP; -1 none yet
	int m_iResult;					// RZ_DutyGame.RESULT_*, once judged
	vector m_vPos;
	float m_fYaw;
	bool m_bTold;					// its tell has been said
	bool m_bSigns;					// its looking places are open (the player is beside it)
	bool m_bTaken;					// in the player's hands, or carried off already
	ResourceName m_sProp;			// what stands for it: a person in their own clothes
}

//------------------------------------------------------------------------------------------------
//! One player's sorting for one duty.
class RZ_DutyGame
{
	// The skins
	static const int KIND_DEAD = 1;
	static const int KIND_LINE = 2;
	static const int KIND_BENCH = 3;
	static const int KIND_STOCK = 4;

	// What a subject is
	static const int CLEAN = 0;
	static const int ODD = 1;		// the third sign: papers that don't match, a fever, a label or a chit that's wrong
	static const int BAD = 2;		// one of the first two signs: a bite, clouded eyes, a broken seal, a short count

	// The verdicts, by how careful they are (the same numbers as what they're right for)
	static const int PASS = 0;
	static const int CHECK = 1;
	static const int STOP = 2;

	static const int RESULT_NONE = 0;
	static const int RESULT_RIGHT = 1;
	static const int RESULT_HALF = 2;		// too careful
	static const int RESULT_WRONG = 3;		// too careless, and it was only odd
	static const int RESULT_DANGER = 4;		// too careless, and it was bad

	static const int SIGN_HIDDEN = 0;
	static const int SIGN_CLEAN = 1;
	static const int SIGN_ODD = 2;
	static const int SIGN_BAD = 3;

	int m_iPlayer;
	int m_iErrand;
	int m_iKind;
	int m_iFaction = -1;
	string m_sTitle;				// the duty's title
	string m_sSub;					// where
	string m_sNote;					// one line about this run ("It's raining: the queue won't wait. One look fewer.")
	int m_iLooks;
	int m_iCarried = -1;			// the subject in the player's hands, -1 none
	bool m_bLaid;					// its things are out at the place
	bool m_bJudged;
	bool m_bDanger;					// a bad one went through
	bool m_bPaidFor;				// its dangerous miss has been answered for
	float m_fScore;					// 0..1
	int m_iRight;
	vector m_vPlace;
	vector m_vAxis;					// the row's direction
	ref array<vector> m_aDest = {};	// where each verdict's spot starts (PASS, CHECK, STOP)
	ref array<int> m_aDestUsed = {0, 0, 0};
	ref array<string> m_aKeys = {};	// every work site it has opened
	ref array<ref RZ_DutySubject> m_aSubjects = {};
	bool m_bAtGate;					// the line stands at the place's way in (m_vGate, m_vInside)
	vector m_vGate;
	vector m_vInside;
	ref array<IEntity> m_aMarks = {};	// the three places' things (a barrier, a shower, a tent), while it's laid out
	string m_sNow;					// the tracker's line under the step: where to take them, or the rules

	//------------------------------------------------------------------------------------------------
	//! The three places' names: PASS, CHECK, STOP.
	static string DestName(int kind, int verdict)
	{
		array<string> names = {"The place", "The place", "The place"};
		switch (kind)
		{
			case KIND_DEAD: names = {"The graves", "The holding table", "The pyre"}; break;
			case KIND_LINE: names = {"The gate", "The wash", "The holding room"}; break;
			case KIND_BENCH: names = {"The rack", "The bath", "The bin"}; break;
			case KIND_STOCK: names = {"The counted stack", "The write-off pile", "The query table"}; break;
		}

		return names[verdict];
	}

	//------------------------------------------------------------------------------------------------
	//! A place's name inside a sentence ("the wash").
	static string DestLower(int kind, int verdict)
	{
		string name = DestName(kind, verdict);
		return "t" + name.Substring(1, name.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! What stands at a place so it reads as what it is (vanilla props, each measured in
	//! tools/camp_kit/footprints.json and known to MarkBox): PASS, CHECK, STOP.
	static ResourceName DestMark(int kind, int verdict)
	{
		array<ResourceName> marks = {"", "", ""};
		switch (kind)
		{
			case KIND_DEAD: marks = {"{EAD2277399B3C658}Prefabs/Props/Military/Graves/Grave_USSR_02.et", "{7E4A4D680FAD3461}Prefabs/Props/Medical/TableMedical_01/TableMedical_01.et", "{13FD3EC5D111AD6D}Prefabs/Props/Forest/WoodPile/WoodPile_01_A.et"}; break;
			case KIND_LINE: marks = {"{64C9C1D1C2C1B38D}Prefabs/Props/Infrastructure/Roadblocks/RoadBlock_01.et", "{DC6B28A772BCC731}Prefabs/Structures/Recreation/Beach/ShowerBeach_01.et", "{908E3B090C2438E9}Prefabs/Props/Recreation/Camp/TentSmall_03/TentSmall_03_white.et"}; break;
			case KIND_BENCH: marks = {"{0089B23327C9B8CA}Prefabs/Props/Furniture/TableLong_01/TableLong_01.et", "{6FD13E995D293853}Prefabs/Props/Furniture/SinkOld_01.et", "{DF3EF41CE4D441DE}Prefabs/Props/Industrial/BarrelMetal_01_rust.et"}; break;
			case KIND_STOCK: marks = {"{1112756E8A13445B}Prefabs/Props/Industrial/Pallets/Pallet_01_Pile.et", "{B3283803059E2750}Prefabs/Props/Crates/CrateWooden_01_plain.et", "{0089B23327C9B8CA}Prefabs/Props/Furniture/TableLong_01/TableLong_01.et"}; break;
		}

		return marks[verdict];
	}

	//------------------------------------------------------------------------------------------------
	//! A place's thing's measured box in its own space (tools/camp_kit/footprints.json: w, d, h, ox, oz).
	//! False for one never measured: it isn't stood.
	static bool MarkBox(ResourceName prefab, out vector fmin, out vector fmax)
	{
		array<float> m = {};
		if (prefab.Contains("Grave_USSR_02.et"))
			m = {1.046, 2.367, 1.227, 0, 0};
		else if (prefab.Contains("TableMedical_01.et"))
			m = {1.139, 0.577, 1.249, 0.024, 0.042};
		else if (prefab.Contains("WoodPile_01_A.et"))
			m = {1.92, 6.6, 1.61, -0.008, -0.011};
		else if (prefab.Contains("RoadBlock_01.et"))
			m = {2.214, 0.522, 1.169, 0, 0};
		else if (prefab.Contains("ShowerBeach_01.et"))
			m = {2.201, 2.199, 3.102, 0, 0};
		else if (prefab.Contains("TentSmall_03_white.et"))
			m = {3.367, 6.217, 1.913, -0.03, 0};
		else if (prefab.Contains("TableLong_01.et"))
			m = {2.406, 1.002, 0.903, 0, 0};
		else if (prefab.Contains("SinkOld_01.et"))
			m = {0.845, 1.452, 1.004, 0, 0};
		else if (prefab.Contains("BarrelMetal_01_rust.et"))
			m = {0.759, 0.762, 1.1, 0, 0};
		else if (prefab.Contains("Pallet_01_Pile.et"))
			m = {0.994, 1.447, 0.979, 0.007, -0.009};
		else if (prefab.Contains("CrateWooden_01_plain.et"))
			m = {0.879, 1.455, 0.834, 0, 0};
		else
			return false;

		fmin = Vector(m[3] - m[0] * 0.5, 0, m[4] - m[1] * 0.5);
		fmax = Vector(m[3] + m[0] * 0.5, m[2], m[4] + m[1] * 0.5);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The rules, in one paragraph: which sign means which place.
	static string Rules(int kind)
	{
		array<string> lines = {};
		Legend(kind, lines);
		string text = "";
		foreach (string line : lines)
		{
			if (!text.IsEmpty())
				text += " ";

			text += line;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A subject's menu: its tell, what you've seen of it, the looks left, and the rules.
	string SubjectText(RZ_DutySubject s)
	{
		string text = s.m_sTell + ".";
		for (int sign = 0; sign < 3; sign++)
		{
			int look = SignLook(s, sign);
			if (look != SIGN_HIDDEN)
				text += " " + SignSeen(m_iKind, sign, look) + ".";
		}

		text += string.Format("\n\n%1 looks left.\n\n%2", m_iLooks, Rules(m_iKind));
		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! The three places to look, as prompts.
	static string SignPrompt(int kind, int sign)
	{
		switch (kind)
		{
			case KIND_DEAD:
			{
				if (sign == 0)
					return "Look at the arms";

				if (sign == 1)
					return "Look at the neck";

				return "Check the papers against the tag";
			}

			case KIND_LINE:
			{
				if (sign == 0)
					return "Have them roll their sleeves";

				if (sign == 1)
					return "Look in their eyes";

				return "Feel their brow";
			}

			case KIND_BENCH:
			{
				if (sign == 0)
					return "Check the seal";

				if (sign == 1)
					return "Hold it to the lamp";

				return "Read the label against the log";
			}

			case KIND_STOCK:
			{
				if (sign == 0)
					return "Count what's in it";

				if (sign == 1)
					return "Check the seal";

				return "Read the chit against the book";
			}
		}

		return "Look";
	}

	//------------------------------------------------------------------------------------------------
	//! What a look found, in a few words.
	static string SignSeen(int kind, int sign, int look)
	{
		bool bad = look == SIGN_BAD;
		bool odd = look == SIGN_ODD;
		switch (kind)
		{
			case KIND_DEAD:
			{
				if (sign == 2 && odd)
					return "The papers give another name";

				if (sign == 2)
					return "The papers match the tag";

				if (bad && sign == 0)
					return "A bite on the arm, wrapped in cloth";

				if (bad)
					return "A bite on the neck, under the collar";

				if (sign == 0)
					return "Clean arms";

				return "A clean neck";
			}

			case KIND_LINE:
			{
				if (sign == 2 && odd)
					return "A burning brow";

				if (sign == 2)
					return "A cool brow";

				if (bad && sign == 0)
					return "A bite under the sleeve";

				if (bad)
					return "Clouded eyes";

				if (sign == 0)
					return "Clean arms";

				return "Clear eyes";
			}

			case KIND_BENCH:
			{
				if (sign == 2 && odd)
					return "The label doesn't match the log";

				if (sign == 2)
					return "The label matches the log";

				if (bad && sign == 0)
					return "The seal is broken";

				if (bad)
					return "The colour has turned";

				if (sign == 0)
					return "The seal is intact";

				return "The colour is clear";
			}

			case KIND_STOCK:
			{
				if (sign == 2 && odd)
					return "The chit is for another crate";

				if (sign == 2)
					return "The chit matches the book";

				if (bad && sign == 0)
					return "The count is short";

				if (bad)
					return "The seal was cut and pressed back";

				if (sign == 0)
					return "The count is all there";

				return "The seal is whole";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Taking a subject up, as a prompt.
	static string TakePrompt(int kind)
	{
		switch (kind)
		{
			case KIND_DEAD: return "Lift onto the stretcher";
			case KIND_LINE: return "Take them by the arm";
			case KIND_BENCH: return "Pick up the tray";
			case KIND_STOCK: return "Pick up the crate";
		}

		return "Take";
	}

	//------------------------------------------------------------------------------------------------
	//! Where a verdict is given, as a prompt, with the place's name: "Leave them here: the wash".
	static string DestPrompt(int kind, int verdict)
	{
		string verb = "Leave them here";
		if (kind == KIND_DEAD && verdict == PASS)
			verb = "Bury them here";
		else if (kind == KIND_DEAD)
			verb = "Lay them here";
		else if (kind == KIND_BENCH || kind == KIND_STOCK)
			verb = "Set it down here";

		return verb + ": " + DestLower(kind, verdict);
	}


	//------------------------------------------------------------------------------------------------
	//! Where a verdict put it, for the line that says so and the clipboard.
	static string DestSaid(int kind, int verdict)
	{
		switch (kind)
		{
			case KIND_DEAD:
			{
				if (verdict == PASS)
					return "buried";

				if (verdict == CHECK)
					return "held";

				return "on the pyre";
			}

			case KIND_LINE:
			{
				if (verdict == PASS)
					return "let in";

				if (verdict == CHECK)
					return "sent to wash";

				return "held in the room";
			}

			case KIND_BENCH:
			{
				if (verdict == PASS)
					return "filed";

				if (verdict == CHECK)
					return "to run again";

				return "destroyed";
			}

			case KIND_STOCK:
			{
				if (verdict == PASS)
					return "counted in";

				if (verdict == CHECK)
					return "written off";

				return "queried";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The work a verdict's spot is (its looks and its hold): a grave is dug, a pyre is a pile.
	static RZ_EWork DestWork(int kind, int verdict)
	{
		if (kind == KIND_DEAD)
		{
			if (verdict == PASS)
				return RZ_EWork.GRAVE;

			if (verdict == CHECK)
				return RZ_EWork.CARE;

			return RZ_EWork.BURN;
		}

		if (kind == KIND_BENCH && verdict == STOP)
			return RZ_EWork.BURN;

		if ((kind == KIND_BENCH || kind == KIND_STOCK) && verdict == PASS)
			return RZ_EWork.BENCH;

		return RZ_EWork.HANDS;
	}

	//------------------------------------------------------------------------------------------------
	//! The goal, in three short lines: which sign means which verdict.
	static void Legend(int kind, notnull array<string> lines)
	{
		switch (kind)
		{
			case KIND_DEAD:
			{
				lines.Insert("A bite goes on the pyre.");
				lines.Insert("Papers that don't match go on the holding table.");
				lines.Insert("All clear goes in a grave.");
				break;
			}

			case KIND_LINE:
			{
				lines.Insert("A bite or clouded eyes goes to the holding room.");
				lines.Insert("Only a fever goes to the wash.");
				lines.Insert("All clear goes through the gate.");
				break;
			}

			case KIND_BENCH:
			{
				lines.Insert("A broken seal or a turned colour goes in the bin.");
				lines.Insert("A label that doesn't match goes in the bath, to run again.");
				lines.Insert("All clear goes on the rack.");
				break;
			}

			case KIND_STOCK:
			{
				lines.Insert("Short, or the seal cut, goes on the query table, and someone answers for it.");
				lines.Insert("Only the chit wrong is written off.");
				lines.Insert("All there is stacked with the counted.");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What a sign shows once looked at.
	int SignLook(RZ_DutySubject s, int sign)
	{
		if (!s.m_aLooked[sign])
			return SIGN_HIDDEN;

		if (sign == 2)
		{
			if (s.m_iTruth == ODD)
				return SIGN_ODD;

			return SIGN_CLEAN;
		}

		if (s.m_iTruth == BAD && s.m_iBadSign == sign)
			return SIGN_BAD;

		return SIGN_CLEAN;
	}

	//------------------------------------------------------------------------------------------------
	bool AllGiven()
	{
		foreach (RZ_DutySubject s : m_aSubjects)
		{
			if (s.m_iVerdict < 0)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Judge it as it stands (one with no verdict gets the careful one). Right is 1, too careful a
	//! half, too careless nothing.
	void Judge()
	{
		float sum = 0;
		m_iRight = 0;
		m_bDanger = false;
		foreach (RZ_DutySubject s : m_aSubjects)
		{
			if (s.m_iVerdict < 0)
				s.m_iVerdict = STOP;

			if (s.m_iVerdict == s.m_iTruth)
			{
				s.m_iResult = RESULT_RIGHT;
				sum += 1;
				m_iRight++;
			}
			else if (s.m_iVerdict > s.m_iTruth)
			{
				s.m_iResult = RESULT_HALF;
				sum += 0.5;
			}
			else if (s.m_iTruth == BAD)
			{
				s.m_iResult = RESULT_DANGER;
				m_bDanger = true;
			}
			else
			{
				s.m_iResult = RESULT_WRONG;
			}
		}

		m_fScore = 0;
		if (!m_aSubjects.IsEmpty())
			m_fScore = sum / m_aSubjects.Count();

		m_bJudged = true;
	}

	//------------------------------------------------------------------------------------------------
	//! How it went, in a line.
	string Summary()
	{
		int half = 0;
		int wrong = 0;
		int danger = 0;
		foreach (RZ_DutySubject s : m_aSubjects)
		{
			if (s.m_iResult == RESULT_HALF)
				half++;
			else if (s.m_iResult == RESULT_WRONG)
				wrong++;
			else if (s.m_iResult == RESULT_DANGER)
				danger++;
		}

		string text = string.Format("That's all of them. %1 of %2 right.", m_iRight, m_aSubjects.Count());
		if (half > 0)
			text += string.Format(" %1 more careful than it needed.", half);

		if (wrong > 0)
			text += string.Format(" %1 missed.", wrong);

		if (danger > 0)
			text += " " + DangerLine(danger);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected string DangerLine(int count)
	{
		switch (m_iKind)
		{
			case KIND_DEAD: return string.Format("%1 bitten, and not burned. It will get up.", count);
			case KIND_LINE: return string.Format("%1 let through with the sickness. It will come out.", count);
			case KIND_BENCH: return string.Format("%1 turned sample kept. The count will be wrong.", count);
			case KIND_STOCK: return string.Format("%1 theft passed as sound. Someone got away with it.", count);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The clipboard at the place: the goal, the looks left, and each subject with what's been seen.
	string Sheet()
	{
		array<string> legend = {};
		Legend(m_iKind, legend);
		string text = "";
		foreach (string line : legend)
		{
			text += line + "\n";
		}

		text += string.Format("\n%1 looks left.\n", m_iLooks);
		foreach (RZ_DutySubject s : m_aSubjects)
		{
			text += string.Format("\n%1. %2.\n", s.m_sTitle, s.m_sTell);
			string seen = "";
			for (int sign = 0; sign < 3; sign++)
			{
				int look = SignLook(s, sign);
				if (look == SIGN_HIDDEN)
					continue;

				seen += SignSeen(m_iKind, sign, look) + ". ";
			}

			if (seen.IsEmpty())
				seen = "Not looked at. ";

			if (s.m_iVerdict >= 0)
				seen += RZ_Util.Capitalize(DestSaid(m_iKind, s.m_iVerdict)) + ".";

			text += seen.Trim() + "\n";
		}

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! Every sorting duty under way: dealt for a duty's errand, laid out at its place as work sites,
//! judged when the last subject is put down, read by the duty when it's handed in.
class RZ_DutyGames
{
	protected static const float SPACING = 2.4;			// between subjects in the row
	protected static const float SIGN_STEP = 0.7;		// between a subject's three places to look
	protected static const float NEAR = 5;				// a subject's looking places open this close
	protected static const float FAR = 9;				// and close again this far off
	protected static const float TELL_RANGE = 4;
	protected static const float TELL_TRUE = 0.75;		// how often a tell points the right way

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Dealer;
	protected ref array<ref RZ_DutyGame> m_aGames = {};

	//------------------------------------------------------------------------------------------------
	void RZ_DutyGames(RZ_GameModeComponent gm, RZ_Errands dealer)
	{
		m_GM = gm;
		m_Dealer = dealer;
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyGame Find(int errandId)
	{
		foreach (RZ_DutyGame g : m_aGames)
		{
			if (g.m_iErrand == errandId)
				return g;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsJudged(int errandId)
	{
		RZ_DutyGame g = Find(errandId);
		return g && g.m_bJudged;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand is over: what's still to do is put away (what was done stands until nobody's near,
	//! the work sites' rule).
	void Drop(int errandId)
	{
		for (int i = m_aGames.Count() - 1; i >= 0; i--)
		{
			if (m_aGames[i].m_iErrand != errandId)
				continue;

			CloseAll(m_aGames[i]);
			m_aGames.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseAll(RZ_DutyGame g)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (sites)
		{
			foreach (string key : g.m_aKeys)
			{
				sites.Close(key);
			}

			foreach (int i, RZ_DutySubject s : g.m_aSubjects)
			{
				sites.Scatter(ChoiceKey(g, i));
			}
		}

		g.m_aKeys.Clear();
		foreach (IEntity mark : g.m_aMarks)
		{
			if (mark)
				RZ_Util.DeleteUnit(mark);
		}

		g.m_aMarks.Clear();
		g.m_sNow = "";
	}

	//------------------------------------------------------------------------------------------------
	//! The tracker's line under the step (RZ_Errands.JournalText): where to take the one you're
	//! leading, else the rules and the looks left. "" when there's no sorting for it.
	string NowLine(int errandId)
	{
		RZ_DutyGame g = Find(errandId);
		if (!g || g.m_bJudged)
			return "";

		return g.m_sNow;
	}

	// ================================================================ Dealing
	//------------------------------------------------------------------------------------------------
	//! A sorting for this duty's errand at its place, drawn from the world: 3 subjects (4 from the
	//! Collapse on), more of them bad the sicker the town they come from and the later the outbreak.
	//! `fromTown` is the town a risky tell names.
	RZ_DutyGame Deal(RZ_Errand e, int kind, string title, string where, string fromTown, float infection, int stage, bool rain)
	{
		RZ_DutyGame g = Find(e.m_iId);
		if (g)
			return g;

		g = new RZ_DutyGame();
		g.m_iPlayer = e.m_iAsker;
		g.m_iErrand = e.m_iId;
		g.m_iKind = kind;
		g.m_iFaction = e.m_iFaction;
		g.m_sTitle = title;
		g.m_sSub = where;
		g.m_vPlace = e.m_vPlace;
		int count = 3;
		if (stage >= RZ_EOutbreakStage.COLLAPSE)
			count = 4;

		float bad = Math.Clamp(0.15 + infection / 300 + stage * 0.04, 0.15, 0.5);
		float odd = 0.22;
		bool any = false;
		array<string> used = {};
		for (int i = 0; i < count; i++)
		{
			RZ_DutySubject s = new RZ_DutySubject();
			float roll = Math.RandomFloat01();
			if (roll < bad)
				s.m_iTruth = RZ_DutyGame.BAD;
			else if (roll < bad + odd)
				s.m_iTruth = RZ_DutyGame.ODD;

			if (s.m_iTruth != RZ_DutyGame.CLEAN)
				any = true;

			g.m_aSubjects.Insert(s);
		}

		// Never a row with nothing in it to find
		if (!any)
		{
			RZ_DutySubject one = g.m_aSubjects[Math.RandomInt(0, count)];
			one.m_iTruth = RZ_DutyGame.ODD;
			if (Math.RandomFloat01() < bad / (bad + odd))
				one.m_iTruth = RZ_DutyGame.BAD;
		}

		foreach (int idx, RZ_DutySubject made : g.m_aSubjects)
		{
			made.m_iBadSign = Math.RandomInt(0, 2);
			made.m_sTitle = SubjectTitle(kind, idx, used);
			made.m_sProp = SubjectProp(kind, RZ_Looks.SeedOf(e.m_iId, idx));
			// The tell points the right way three times in four
			int points = made.m_iTruth;
			if (Math.RandomFloat01() > TELL_TRUE)
				points = (made.m_iTruth + 1 + Math.RandomInt(0, 2)) % 3;

			made.m_sTell = Tell(kind, points, fromTown, used);
		}

		g.m_iLooks = count + 2;
		if (rain && kind == RZ_DutyGame.KIND_LINE)
		{
			g.m_iLooks = g.m_iLooks - 1;
			g.m_sNote = "It's raining and the queue won't stand for it: one look fewer.";
		}
		else if (infection >= 50 && !fromTown.IsEmpty() && kind != RZ_DutyGame.KIND_STOCK)
		{
			g.m_sNote = string.Format("%1 is sick. Expect more of them bad.", fromTown);
		}

		// The line at a gate stands at the place's real way in, and the step points there
		if (kind == RZ_DutyGame.KIND_LINE)
		{
			vector gate;
			vector inside;
			string how = RZ_DutyScenes.Entrance(e.m_vPlace, gate, inside);
			if (RZ_Util.DistXZ(gate, inside) > 1)
			{
				g.m_bAtGate = true;
				g.m_vGate = gate;
				g.m_vInside = inside;
				g.m_vPlace = gate;
				e.m_vPlace = gate;
			}

			RZ_Log.Info(string.Format("Duty sorting: the line for errand %1 stands at %2 (%3).", e.m_iId, how, RZ_SettlementScanner.GridReference(g.m_vPlace)));
		}

		g.m_sNow = string.Format("%1 %2 looks.", RZ_DutyGame.Rules(kind), g.m_iLooks);
		m_aGames.Insert(g);
		RZ_Log.Info(string.Format("Duty sorting: %1 for %2, %3 subjects, %4 looks (errand %5).", KindName(kind), RZ_Util.PlayerName(e.m_iAsker), count, g.m_iLooks, e.m_iId));
		return g;
	}

	//------------------------------------------------------------------------------------------------
	static string KindName(int kind)
	{
		switch (kind)
		{
			case RZ_DutyGame.KIND_DEAD: return "the dead";
			case RZ_DutyGame.KIND_LINE: return "the line";
			case RZ_DutyGame.KIND_BENCH: return "the bench";
			case RZ_DutyGame.KIND_STOCK: return "the stock";
		}

		return "sorting";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Unused(notnull array<string> pool, notnull array<string> used)
	{
		array<string> free = {};
		foreach (string p : pool)
		{
			if (!used.Contains(p))
				free.Insert(p);
		}

		if (free.IsEmpty())
			free.Copy(pool);

		string pick = free[Math.RandomInt(0, free.Count())];
		used.Insert(pick);
		return pick;
	}

	//------------------------------------------------------------------------------------------------
	protected static string SubjectTitle(int kind, int index, notnull array<string> used)
	{
		switch (kind)
		{
			case RZ_DutyGame.KIND_DEAD: return string.Format("Tag %1", 101 + index * 7 + Math.RandomInt(0, 6));
			case RZ_DutyGame.KIND_BENCH: return string.Format("Tray %1", 11 + index * 4 + Math.RandomInt(0, 3));
			case RZ_DutyGame.KIND_STOCK: return string.Format("Crate %1", 21 + index * 5 + Math.RandomInt(0, 4));
		}

		array<string> who = {"A man, forties", "Two brothers", "A courier", "A lad with a pack", "An old man", "A father and child", "A farmhand", "A man in a coat"};
		return Unused(who, used);
	}

	//------------------------------------------------------------------------------------------------
	//! The free tell: calm, odd or risky, in the skin's words. A risky one names the sick town.
	protected static string Tell(int kind, int points, string town, notnull array<string> used)
	{
		array<string> pool = {};
		string from = "the road";
		if (!town.IsEmpty())
			from = town;

		switch (kind)
		{
			case RZ_DutyGame.KIND_DEAD:
			{
				if (points == RZ_DutyGame.BAD)
				{
					pool.Insert("Found in the street at " + from);
					pool.Insert("Brought in off the road, alone");
					pool.Insert("Sleeve torn, cloth wrapped round it");
				}
				else if (points == RZ_DutyGame.ODD)
				{
					pool.Insert("The tag is written twice");
					pool.Insert("Nobody has come for them");
					pool.Insert("Two names in the pocket");
				}
				else
				{
					pool.Insert("Died in bed, family there");
					pool.Insert("Old age, the tag says");
					pool.Insert("A fall. Seen by three people");
				}

				break;
			}

			case RZ_DutyGame.KIND_LINE:
			{
				if (points == RZ_DutyGame.BAD)
				{
					pool.Insert("Walked in from " + from);
					pool.Insert("Keeps a sleeve pulled down");
					pool.Insert("Won't look at the lamp");
				}
				else if (points == RZ_DutyGame.ODD)
				{
					pool.Insert("Sweating. Says it's the walk");
					pool.Insert("Shivering in a thick coat");
					pool.Insert("Coughs, then smiles about it");
				}
				else
				{
					pool.Insert("Known here. Back from the well");
					pool.Insert("Calm. Offers both hands");
					pool.Insert("Sent ahead by someone inside");
				}

				break;
			}

			case RZ_DutyGame.KIND_BENCH:
			{
				if (points == RZ_DutyGame.BAD)
				{
					pool.Insert("Came in warm from " + from);
					pool.Insert("Two days in a pack, no ice");
					pool.Insert("The box was dropped");
				}
				else if (points == RZ_DutyGame.ODD)
				{
					pool.Insert("Label in two hands");
					pool.Insert("Logged at night, no initials");
					pool.Insert("The number was overwritten");
				}
				else
				{
					pool.Insert("Drawn here this morning");
					pool.Insert("Logged and countersigned");
					pool.Insert("Cold chain unbroken");
				}

				break;
			}

			case RZ_DutyGame.KIND_STOCK:
			{
				if (points == RZ_DutyGame.BAD)
				{
					pool.Insert("Lighter than the one beside it");
					pool.Insert("Moved since the last count");
					pool.Insert("Fresh scratches round the lid");
				}
				else if (points == RZ_DutyGame.ODD)
				{
					pool.Insert("The chit is in pencil");
					pool.Insert("Two numbers chalked on it");
					pool.Insert("Booked in on a Sunday");
				}
				else
				{
					pool.Insert("Dust on the lid, undisturbed");
					pool.Insert("Came off the last truck, signed");
					pool.Insert("Stacked where the book says");
				}

				break;
			}
		}

		if (pool.IsEmpty())
			return "";

		return Unused(pool, used);
	}

	// ================================================================ In the world
	//------------------------------------------------------------------------------------------------
	//! What stands for a subject while it waits (Missions > Errands > Props; empty: the next best).
	//! People in the line each wear their own clothes, picked by `seed` from Duty person looks, else
	//! from vanilla's civilians and the Duty person (so no two side by side are the same).
	protected ResourceName SubjectProp(int kind, int seed)
	{
		RZ_ErrandProps props = RZ_ErrandSettings.Get().Props();
		switch (kind)
		{
			case RZ_DutyGame.KIND_DEAD:
			{
				if (!props.m_sDutyBody.IsEmpty())
					return props.m_sDutyBody;

				return props.m_sEffects;
			}

			case RZ_DutyGame.KIND_LINE:
			{
				ResourceName person = props.m_sDutyPerson;
				if (person.IsEmpty())
					person = m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.CIVILIAN);

				array<ResourceName> looks = {};
				if (props.m_aDutyPersonLooks)
					looks.Copy(props.m_aDutyPersonLooks);

				if (looks.IsEmpty())
				{
					looks.Insert("{8C7093AF368F496A}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_1.et");
					looks.Insert("{1FFE2B88BEF51840}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_Turtleneck_2.et");
					looks.Insert("{408B8BD5E3F09FF3}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_3.et");
					if (!person.IsEmpty())
						looks.Insert(person);
				}

				return RZ_Looks.PickFrom(person, looks, seed);
			}

			case RZ_DutyGame.KIND_BENCH:
			{
				if (!props.m_sDutyTray.IsEmpty())
					return props.m_sDutyTray;

				return props.m_sCrate;
			}
		}

		return props.m_sCrate;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Key(RZ_DutyGame g, string part)
	{
		return string.Format("errand.%1.%2", g.m_iErrand, part);
	}

	//------------------------------------------------------------------------------------------------
	//! A subject's one prompt (its looks and taking it up).
	protected static string ChoiceKey(RZ_DutyGame g, int subject)
	{
		return Key(g, string.Format("s%1.who", subject));
	}

	//------------------------------------------------------------------------------------------------
	protected void Open(RZ_DutyGame g, string part, RZ_EWork work, string label, float hold, vector pos, ResourceName before = "", float yaw = -1)
	{
		string key = Key(g, part);
		RZ_WorkSites.Get().Open(key, work, label, hold, pos, g.m_iPlayer, before, "", yaw);
		if (!g.m_aKeys.Contains(key))
			g.m_aKeys.Insert(key);
	}

	//------------------------------------------------------------------------------------------------
	//! The row laid out at the place: each subject where it waits (its prop, and its one prompt),
	//! the three places to take them, each with its thing standing (LayDests), and the clipboard.
	//! At a gate the line queues on the road outside, facing in.
	protected void LayOut(RZ_DutyGame g)
	{
		g.m_bLaid = true;
		float yaw = Math.RandomFloat(0, 360);
		float rad = yaw * Math.DEG2RAD;
		g.m_vAxis = Vector(Math.Cos(rad), 0, -Math.Sin(rad));
		int count = g.m_aSubjects.Count();
		if (g.m_bAtGate)
		{
			vector outward = RZ_DutyScenes.Toward(g.m_vInside, g.m_vGate, 1) - g.m_vInside;
			outward[1] = 0;
			if (outward.Length() > 0.1)
				g.m_vAxis = outward.Normalized();

			yaw = RZ_DutyRound.Bearing(g.m_vGate, g.m_vInside);
		}

		foreach (int i, RZ_DutySubject s : g.m_aSubjects)
		{
			if (g.m_bAtGate)
				s.m_vPos = RZ_Util.GroundPos(g.m_vGate + g.m_vAxis * (3 + i * SPACING));
			else
				s.m_vPos = g.m_vPlace + g.m_vAxis * ((i - (count - 1) * 0.5) * SPACING);

			s.m_fYaw = yaw;
			Open(g, string.Format("s%1.take", i), RZ_EWork.TAKE, RZ_DutyGame.TakePrompt(g.m_iKind), 3, s.m_vPos, s.m_sProp, yaw);
		}

		LayDests(g);
		Open(g, "clip", RZ_EWork.WRITE, "Read the clipboard", 1, RZ_WorkSites.Spot(g.m_vPlace, 2, 4));
	}

	//------------------------------------------------------------------------------------------------
	//! The three places, within 10 m of the line, each with its thing standing (DestMark) where its
	//! measured box is clear, level and apart from the others; one that won't fit is dropped, never
	//! squeezed, and its place is still there to leave them at. At a gate: the barrier beside the
	//! road at the gate, the wash and the holding room just inside, either side.
	protected void LayDests(RZ_DutyGame g)
	{
		array<vector> taken = {};
		vector side = Vector(-g.m_vAxis[2], 0, g.m_vAxis[0]);
		for (int v = 0; v < 3; v++)
		{
			ResourceName mark = RZ_DutyGame.DestMark(g.m_iKind, v);
			vector fmin;
			vector fmax;
			bool measured = !mark.IsEmpty() && RZ_DutyGame.MarkBox(mark, fmin, fmax);
			vector spot = vector.Zero;
			float facing = 0;
			bool fits = false;
			for (int tries = 0; tries < 10 && !fits; tries++)
			{
				vector at;
				int round = tries / 3;
				float back = tries - round * 3;
				float across = 1;
				if (v == RZ_DutyGame.STOP)
					across = -1;

				if (g.m_bAtGate && v == RZ_DutyGame.PASS)
					at = g.m_vGate + side * (3.5 + tries * 0.8);
				else if (g.m_bAtGate)
					at = g.m_vInside + side * ((5 + tries * 0.8) * across) - g.m_vAxis * back;
				else
					at = RZ_WorkSites.Spot(g.m_vPlace, 5, 9);

				at = RZ_Util.GroundPos(at);
				if (FarFromOthers(g, at, taken) == false)
					continue;

				spot = at;
				facing = RZ_DutyRound.Bearing(at, g.m_vPlace);
				if (!measured)
					break;

				array<vector> quad = {};
				RZ_CampLife.Quad(at, facing, fmin, fmax, 0, quad);
				if (RZ_CampLife.Blocked(quad) || !RZ_DutyRound.Level(quad, 0.5) || RZ_DutyRound.Overlaps(at, facing, fmin, fmax, taken))
					continue;

				fits = true;
				RZ_CampLife.Quad(at, facing, fmin, fmax, 0, taken);
			}

			if (spot == vector.Zero)
				spot = RZ_WorkSites.Spot(g.m_vPlace, 5, 9);

			// The place to leave them: in front of its thing, toward the line
			vector leave = spot;
			if (fits)
			{
				IEntity e = RZ_Util.SpawnPrefab(mark, spot, facing);
				if (e)
				{
					RZ_DressingGuard.Strip(e, mark);
					g.m_aMarks.Insert(e);
				}

				leave = RZ_DutyScenes.Toward(spot, g.m_vPlace, fmax[2] + 1.2);
				RZ_Log.Info(string.Format("Duty sorting: %1 for errand %2 stands at %3.", RZ_DutyGame.DestName(g.m_iKind, v), g.m_iErrand, RZ_SettlementScanner.GridReference(spot)));
			}
			else if (measured)
			{
				RZ_Log.Info(string.Format("Duty sorting: %1's thing for errand %2 dropped: no clear, level spot in 10 tries.", RZ_DutyGame.DestName(g.m_iKind, v), g.m_iErrand));
			}

			g.m_aDest.Insert(leave);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At least 4 m from the places already set, and 2.5 m from every subject.
	protected bool FarFromOthers(RZ_DutyGame g, vector at, array<vector> taken)
	{
		foreach (vector have : g.m_aDest)
		{
			if (RZ_Util.DistXZ(have, at) < 4)
				return false;
		}

		foreach (RZ_DutySubject s : g.m_aSubjects)
		{
			if (RZ_Util.DistXZ(s.m_vPos, at) < 2.5)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A verdict's spot, for the one in the player's hands (each beside the last put there, so the
	//! graves make a row). The spots are only there while something is carried.
	protected void OpenDest(RZ_DutyGame g, int verdict)
	{
		int n = g.m_aDestUsed[verdict];
		vector at = g.m_aDest[verdict] + g.m_vAxis * (n * 1.8);
		float hold = 4;
		if (RZ_DutyGame.DestWork(g.m_iKind, verdict) == RZ_EWork.GRAVE)
			hold = 8;

		string part = string.Format("d%1.%2", verdict, n);
		Open(g, part, RZ_DutyGame.DestWork(g.m_iKind, verdict), RZ_DutyGame.DestPrompt(g.m_iKind, verdict), hold, at);
	}

	//------------------------------------------------------------------------------------------------
	protected string DestKey(RZ_DutyGame g, int verdict)
	{
		return Key(g, string.Format("d%1.%2", verdict, g.m_aDestUsed[verdict]));
	}

	//------------------------------------------------------------------------------------------------
	//! A subject's three places to look: in its one prompt's list with taking it up (with the object
	//! menu), else side by side along it while the player stands beside it.
	protected void OpenSigns(RZ_DutyGame g, int i, bool open)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		RZ_DutySubject s = g.m_aSubjects[i];
		bool menu = RZ_ObjectScreen.Available();
		s.m_bSigns = open;
		array<string> keys = {};
		vector side = Vector(-g.m_vAxis[2], 0, g.m_vAxis[0]);
		for (int sign = 0; sign < 3; sign++)
		{
			string part = string.Format("s%1.g%2", i, sign);
			if (!open || s.m_aLooked[sign] || g.m_iLooks <= 0)
			{
				sites.Close(Key(g, part));
				continue;
			}

			vector at = s.m_vPos + side * 0.9 + g.m_vAxis * ((sign - 1) * SIGN_STEP);
			if (menu)
				at = s.m_vPos;

			Open(g, part, RZ_EWork.HANDS, RZ_DutyGame.SignPrompt(g.m_iKind, sign), 2, at);
			keys.Insert(Key(g, part));
		}

		// Its one prompt: its name, listing what's left to look at, then taking it up
		if (s.m_bTaken)
			return;

		keys.Insert(Key(g, string.Format("s%1.take", i)));
		sites.Gather(ChoiceKey(g, i), s.m_sTitle, keys, s.m_vPos, g.SubjectText(s), s.m_fYaw);
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: lay the row out, say a subject's tell as the player walks up, open the places
	//! to look on the one they stand at, and read what their hands have done since the last tick.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (m_aGames.IsEmpty())
			return;

		RZ_WorkSites sites = RZ_WorkSites.Get();
		foreach (RZ_DutyGame g : m_aGames)
		{
			if (g.m_bJudged)
				continue;

			int idx = ids.Find(g.m_iPlayer);
			if (idx < 0)
				continue;

			vector me = players[idx].GetOrigin();
			// No work sites on this server: it's sorted the careful way by standing there
			if (!sites || !RZ_WorkSites.Available())
			{
				if (RZ_Util.DistXZ(me, g.m_vPlace) <= NEAR)
					SelfJudge(g);

				continue;
			}

			if (!g.m_bLaid)
			{
				if (RZ_Util.DistXZ(me, g.m_vPlace) > 60)
					continue;

				LayOut(g);
				array<string> legend = {};
				RZ_DutyGame.Legend(g.m_iKind, legend);
				string goal = string.Format("%1 %2 %3 You have %4 looks.", legend[0], legend[1], legend[2], g.m_iLooks);
				if (!g.m_sNote.IsEmpty())
					goal += " " + g.m_sNote;

				m_GM.NotifyPlayer(g.m_iPlayer, g.m_sTitle, goal);
			}

			Step(g, me);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Step(RZ_DutyGame g, vector me)
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		foreach (int i, RZ_DutySubject s : g.m_aSubjects)
		{
			if (s.m_bTaken)
				continue;

			float d = RZ_Util.DistXZ(me, s.m_vPos);
			if (d <= TELL_RANGE && !s.m_bTold)
			{
				s.m_bTold = true;
				m_GM.NotifyPlayer(g.m_iPlayer, s.m_sTitle, s.m_sTell + ".");
			}

			// With the object menu its looks are always in its list; else they stand while you're beside it
			bool menu = RZ_ObjectScreen.Available();
			if ((d <= NEAR || menu) && !s.m_bSigns && g.m_iLooks > 0)
				OpenSigns(g, i, true);
			else if (d > FAR && s.m_bSigns && !menu)
				OpenSigns(g, i, false);

			sites.SetChoiceText(ChoiceKey(g, i), g.SubjectText(s));

			// A look: the place is turned over for good, and it's one look gone
			for (int sign = 0; sign < 3 && s.m_bSigns; sign++)
			{
				string signKey = Key(g, string.Format("s%1.g%2", i, sign));
				if (s.m_aLooked[sign] || !sites.IsDone(signKey))
					continue;

				sites.Close(signKey);
				s.m_aLooked[sign] = true;
				g.m_iLooks = g.m_iLooks - 1;
				string seen = RZ_DutyGame.SignSeen(g.m_iKind, sign, g.SignLook(s, sign));
				m_GM.NotifyPlayer(g.m_iPlayer, s.m_sTitle, string.Format("%1. %2 looks left.", seen, g.m_iLooks));
				if (g.m_iLooks <= 0)
					OutOfLooks(g);
			}

			// Taken up: one at a time
			string takeKey = Key(g, string.Format("s%1.take", i));
			if (!sites.IsDone(takeKey))
				continue;

			if (g.m_iCarried >= 0)
			{
				m_GM.NotifyPlayer(g.m_iPlayer, g.m_sTitle, "Your hands are full. Put down the one you have first.");
				sites.Close(takeKey);
				Open(g, string.Format("s%1.take", i), RZ_EWork.TAKE, RZ_DutyGame.TakePrompt(g.m_iKind), 3, s.m_vPos, s.m_sProp, s.m_fYaw);
				continue;
			}

			sites.Close(takeKey);
			s.m_bTaken = true;
			OpenSigns(g, i, false);
			sites.Scatter(ChoiceKey(g, i));
			g.m_iCarried = i;
			for (int spot = 0; spot < 3; spot++)
			{
				OpenDest(g, spot);
			}

			m_GM.NotifyPlayer(g.m_iPlayer, s.m_sTitle, string.Format("You have them. Take them to %1, %2 or %3. %4", RZ_DutyGame.DestLower(g.m_iKind, 0), RZ_DutyGame.DestLower(g.m_iKind, 1), RZ_DutyGame.DestLower(g.m_iKind, 2), RZ_DutyGame.Rules(g.m_iKind)));
		}

		Point(g, me);

		// Put down: the spot is the verdict
		for (int v = 0; v < 3 && g.m_iCarried >= 0; v++)
		{
			string destKey = DestKey(g, v);
			if (!sites.IsDone(destKey))
				continue;

			RZ_DutySubject carried = g.m_aSubjects[g.m_iCarried];
			carried.m_iVerdict = v;
			g.m_iCarried = -1;
			// The spot used keeps what was put there; the other two go until the next is carried
			for (int other = 0; other < 3; other++)
			{
				sites.Close(DestKey(g, other));
			}

			g.m_aDestUsed[v] = g.m_aDestUsed[v] + 1;
			m_GM.NotifyPlayer(g.m_iPlayer, carried.m_sTitle, string.Format("%1.", RZ_Util.Capitalize(RZ_DutyGame.DestSaid(g.m_iKind, v))));
			if (g.AllGiven())
				Finish(g);

			return;
		}

		// The clipboard: read as often as you like
		string clip = Key(g, "clip");
		if (sites.IsDone(clip))
		{
			RZ_Reader.Show(m_GM, g.m_iPlayer, g.m_sTitle, "A clipboard on a nail, in pencil", g.Sheet());
			vector at = sites.PosOf(clip);
			sites.Close(clip);
			Open(g, "clip", RZ_EWork.WRITE, "Read the clipboard", 1, at);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The tracker: while you lead one, the step points between the three places and the line under
	//! it names each, with its way and distance; else it points at the line, under the rules.
	protected void Point(RZ_DutyGame g, vector me)
	{
		RZ_Errand e = m_Dealer.FindById(g.m_iErrand);
		if (g.m_iCarried < 0 || g.m_aDest.Count() < 3)
		{
			g.m_sNow = string.Format("%1 %2 looks left.", RZ_DutyGame.Rules(g.m_iKind), g.m_iLooks);
			if (e)
				e.m_vPlace = g.m_vPlace;

			return;
		}

		string line = string.Format("Leading %1.", g.m_aSubjects[g.m_iCarried].m_sTitle);
		vector middle = vector.Zero;
		for (int v = 0; v < 3; v++)
		{
			vector at = g.m_aDest[v];
			middle = middle + at * (1.0 / 3.0);
			line += string.Format(" %1: %2 m %3.", RZ_DutyGame.DestName(g.m_iKind, v), RZ_Util.RoundToInt(RZ_Util.DistXZ(me, at)), Compass(me, at));
		}

		g.m_sNow = line + " " + RZ_DutyGame.Rules(g.m_iKind);
		if (e)
			e.m_vPlace = middle;
	}

	//------------------------------------------------------------------------------------------------
	//! "N", "NE", ... from `from` to `to`.
	protected static string Compass(vector from, vector to)
	{
		float deg = Math.Atan2(to[0] - from[0], to[2] - from[2]) * Math.RAD2DEG;
		if (deg < 0)
			deg += 360;

		array<string> names = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
		return names[RZ_Util.RoundToInt(deg / 45) % 8];
	}

	//------------------------------------------------------------------------------------------------
	protected void OutOfLooks(RZ_DutyGame g)

	{
		foreach (int i, RZ_DutySubject s : g.m_aSubjects)
		{
			if (s.m_bSigns)
				OpenSigns(g, i, false);
		}

		m_GM.NotifyPlayer(g.m_iPlayer, g.m_sTitle, "That's all the looking there's time for. Sort the rest on what you know. If you aren't sure, the careful spot costs you half.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_DutyGame g)
	{
		g.Judge();
		CloseAll(g);
		m_GM.NotifyPlayer(g.m_iPlayer, g.m_sTitle, g.Summary());
		RZ_Log.Info(string.Format("Duty sorting: errand %1 judged for %2: %3 of %4 right, score %5, danger %6.", g.m_iErrand, RZ_Util.PlayerName(g.m_iPlayer), g.m_iRight, g.m_aSubjects.Count(), g.m_fScore, g.m_bDanger));
	}

	//------------------------------------------------------------------------------------------------
	//! No work sites: every one gets the careful verdict (half pay, and nothing dangerous).
	protected void SelfJudge(RZ_DutyGame g)
	{
		foreach (RZ_DutySubject s : g.m_aSubjects)
		{
			s.m_iVerdict = RZ_DutyGame.STOP;
		}

		g.Judge();
		m_GM.NotifyPlayer(g.m_iPlayer, g.m_sTitle, "There's nothing set out to sort them by here, so it was done the careful way: every one of them.");
		RZ_Log.Info(string.Format("Duty sorting: errand %1 judged itself (no work sites), score %2.", g.m_iErrand, g.m_fScore));
	}
}
