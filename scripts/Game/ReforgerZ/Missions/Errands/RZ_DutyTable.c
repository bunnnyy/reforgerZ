//! The table (docs/ERRANDS.md, "The table"): the small sorting game three duties share, in three
//! skins: the dead on the slab, the line at the gate, the sample trays on the bench. A few subjects,
//! each with three signs face down and one tell that's free. A few looks for the whole table, fewer
//! than there are signs. One of three verdicts each. The server holds the truth and judges:
//! right is full, too careful is half, too careless is nothing, and a bad one let through is the
//! dangerous miss. How many are bad follows the world (the town's infection, the outbreak's stage).
//! The screen is RZ_DutyMenu (UI/RZ_DutyMenu.c); every move comes back here and is checked.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_DutySubject
{
	string m_sTitle;				// "TAG 114", "A crew lad", "TRAY 7-C"
	string m_sTell;					// what's free to see
	int m_iTruth;					// RZ_DutyTable.CLEAN, ODD or BAD
	int m_iBadSign;					// which of the first two signs shows it (a bad one)
	ref array<bool> m_aLooked = {false, false, false};
	int m_iVerdict = -1;			// RZ_DutyTable.PASS, CHECK or STOP; -1 none yet
	int m_iResult;					// RZ_DutyTable.RESULT_*, once judged
}

//------------------------------------------------------------------------------------------------
//! One player's table for one duty.
class RZ_DutyTable
{
	// The skins
	static const int KIND_DEAD = 1;
	static const int KIND_LINE = 2;
	static const int KIND_BENCH = 3;

	// What a subject is
	static const int CLEAN = 0;
	static const int ODD = 1;		// the third sign: papers that don't match, a fever, a label in two hands
	static const int BAD = 2;		// one of the first two signs: a bite, clouded eyes, a broken seal

	// The verdicts, by how careful they are (the same numbers as what they're right for)
	static const int PASS = 0;
	static const int CHECK = 1;
	static const int STOP = 2;

	static const int RESULT_NONE = 0;
	static const int RESULT_RIGHT = 1;
	static const int RESULT_HALF = 2;		// too careful
	static const int RESULT_WRONG = 3;		// too careless, and it was only odd
	static const int RESULT_DANGER = 4;		// too careless, and it was bad

	// A sign's look on the screen
	static const int SIGN_HIDDEN = 0;
	static const int SIGN_CLEAN = 1;
	static const int SIGN_ODD = 2;
	static const int SIGN_BAD = 3;

	int m_iPlayer;
	int m_iErrand;
	int m_iKind;
	int m_iFaction = -1;
	int m_iSerial;					// the open screen's serial, 0 while it's shut
	string m_sTitle;				// the duty's title
	string m_sSub;					// where
	string m_sNote;					// one line about this run ("It's raining: the queue won't wait. One look fewer.")
	int m_iLooks;
	bool m_bJudged;
	bool m_bDanger;					// a bad one went through
	bool m_bPaidFor;				// its dangerous miss has been answered for
	float m_fScore;					// 0..1
	int m_iRight;
	bool m_bAway = true;			// off the spot since it was last shut (no work sites: it opens by standing there)
	ref array<ref RZ_DutySubject> m_aSubjects = {};

	//------------------------------------------------------------------------------------------------
	//! The three signs' names.
	static void SignNames(int kind, notnull array<string> names)
	{
		switch (kind)
		{
			case KIND_DEAD: names.Insert("ARMS"); names.Insert("NECK"); names.Insert("PAPERS"); break;
			case KIND_LINE: names.Insert("ARMS"); names.Insert("EYES"); names.Insert("BROW"); break;
			case KIND_BENCH: names.Insert("SEAL"); names.Insert("COLOUR"); names.Insert("LABEL"); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What a turned sign reads.
	static string SignWord(int kind, int sign, int look)
	{
		if (look == SIGN_HIDDEN)
			return "?";

		switch (kind)
		{
			case KIND_DEAD:
			{
				if (look == SIGN_BAD)
					return "BITE";

				if (look == SIGN_ODD)
					return "NO MATCH";

				if (sign == 2)
					return "MATCH";

				return "CLEAN";
			}

			case KIND_LINE:
			{
				if (look == SIGN_BAD && sign == 1)
					return "CLOUDED";

				if (look == SIGN_BAD)
					return "BITE";

				if (look == SIGN_ODD)
					return "FEVER";

				if (sign == 1)
					return "CLEAR";

				if (sign == 2)
					return "COOL";

				return "CLEAN";
			}

			case KIND_BENCH:
			{
				if (look == SIGN_BAD && sign == 0)
					return "BROKEN";

				if (look == SIGN_BAD)
					return "TURNED";

				if (look == SIGN_ODD)
					return "NO MATCH";

				if (sign == 0)
					return "INTACT";

				if (sign == 1)
					return "CLEAR";

				return "MATCH";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The verdicts' labels: PASS, CHECK, STOP.
	static void VerdictNames(int kind, notnull array<string> names)
	{
		switch (kind)
		{
			case KIND_DEAD: names.Insert("BURY"); names.Insert("HOLD"); names.Insert("BURN"); break;
			case KIND_LINE: names.Insert("LET IN"); names.Insert("WASH"); names.Insert("HOLD"); break;
			case KIND_BENCH: names.Insert("FILE"); names.Insert("RE-RUN"); names.Insert("DESTROY"); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The goal, in three short lines: which sign means which verdict.
	static void Legend(int kind, notnull array<string> lines)
	{
		switch (kind)
		{
			case KIND_DEAD:
			{
				lines.Insert("A BITE: BURN");
				lines.Insert("PAPERS DON'T MATCH: HOLD");
				lines.Insert("ALL CLEAR: BURY");
				break;
			}

			case KIND_LINE:
			{
				lines.Insert("A BITE OR CLOUDED EYES: HOLD");
				lines.Insert("ONLY A FEVER: WASH");
				lines.Insert("ALL CLEAR: LET IN");
				break;
			}

			case KIND_BENCH:
			{
				lines.Insert("BROKEN SEAL OR TURNED: DESTROY");
				lines.Insert("LABEL DOESN'T MATCH: RE-RUN");
				lines.Insert("ALL CLEAR: FILE");
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

			// Judged: everything is turned over
			s.m_aLooked[0] = true;
			s.m_aLooked[1] = true;
			s.m_aLooked[2] = true;
		}

		m_fScore = 0;
		if (!m_aSubjects.IsEmpty())
			m_fScore = sum / m_aSubjects.Count();

		m_bJudged = true;
	}

	//------------------------------------------------------------------------------------------------
	//! How it went, in a line for the screen and the giver.
	string Summary()
	{
		array<string> verdicts = {};
		VerdictNames(m_iKind, verdicts);
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

		string text = string.Format("%1 of %2 right.", m_iRight, m_aSubjects.Count());
		if (half > 0)
			text += string.Format(" %1 too careful (half).", half);

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
		}

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! Every table on the server: dealt for a duty's errand, opened at its work site, judged, read by
//! the duty when it's handed in. Owned by the errand dealer.
class RZ_DutyTables
{
	static const int MOVE_DONE = 90;		// hand the table in
	static const int MOVE_FAILED = 99;		// the client couldn't open the screen

	protected static const float SPOT_RANGE = 4;		// no work sites: the table opens this near its place
	protected static const float AWAY_RANGE = 8;		// and again only after stepping this far off
	protected static const float TELL_TRUE = 0.75;		// how often a tell points the right way

	protected RZ_GameModeComponent m_GM;
	protected RZ_Errands m_Dealer;
	protected ref array<ref RZ_DutyTable> m_aTables = {};
	protected int m_iNextSerial = 1;
	protected bool m_bClientFailed;			// a client couldn't open it: tables judge themselves, carefully

	//------------------------------------------------------------------------------------------------
	void RZ_DutyTables(RZ_GameModeComponent gm, RZ_Errands dealer)
	{
		m_GM = gm;
		m_Dealer = dealer;
	}

	//------------------------------------------------------------------------------------------------
	//! The screen is used: UI > Duty card layout set (the client checks the same) and no client failed
	//! to open it. Otherwise a table judges itself with the careful verdict (half), and the duty is its hold.
	bool IsUsed()
	{
		return !m_bClientFailed && !RZ_UISettings.Get().m_sDutyCardLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	RZ_DutyTable Find(int errandId)
	{
		foreach (RZ_DutyTable t : m_aTables)
		{
			if (t.m_iErrand == errandId)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DutyTable OpenFor(int playerId)
	{
		foreach (RZ_DutyTable t : m_aTables)
		{
			if (t.m_iPlayer == playerId && t.m_iSerial > 0)
				return t;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsJudged(int errandId)
	{
		RZ_DutyTable t = Find(errandId);
		return t && t.m_bJudged;
	}

	//------------------------------------------------------------------------------------------------
	//! The errand is over: its table goes (an open screen closes).
	void Drop(int errandId)
	{
		for (int i = m_aTables.Count() - 1; i >= 0; i--)
		{
			RZ_DutyTable t = m_aTables[i];
			if (t.m_iErrand != errandId)
				continue;

			if (t.m_iSerial > 0)
				Shut(t, "gone");

			m_aTables.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Forget(int playerId)
	{
		foreach (RZ_DutyTable t : m_aTables)
		{
			if (t.m_iPlayer == playerId)
				t.m_iSerial = 0;
		}
	}

	// ================================================================ Dealing
	//------------------------------------------------------------------------------------------------
	//! A table for this duty's errand, drawn from the world: 3 to 5 subjects, more of them bad the
	//! sicker the town they come from and the later the outbreak. `fromTown` is the town a risky tell
	//! names.
	RZ_DutyTable Deal(RZ_Errand e, int kind, string title, string where, string fromTown, float infection, int stage, bool rain)
	{
		RZ_DutyTable t = Find(e.m_iId);
		if (t)
			return t;

		t = new RZ_DutyTable();
		t.m_iPlayer = e.m_iAsker;
		t.m_iErrand = e.m_iId;
		t.m_iKind = kind;
		t.m_iFaction = e.m_iFaction;
		t.m_sTitle = title;
		t.m_sSub = where;
		int count = 3 + Math.RandomInt(0, 2);
		if (stage >= RZ_EOutbreakStage.COLLAPSE)
			count++;

		count = Math.ClampInt(count, 3, 5);
		float bad = Math.Clamp(0.15 + infection / 300 + stage * 0.04, 0.15, 0.5);
		float odd = 0.22;
		bool any = false;
		array<string> used = {};
		for (int i = 0; i < count; i++)
		{
			RZ_DutySubject s = new RZ_DutySubject();
			float roll = Math.RandomFloat01();
			if (roll < bad)
				s.m_iTruth = RZ_DutyTable.BAD;
			else if (roll < bad + odd)
				s.m_iTruth = RZ_DutyTable.ODD;

			if (s.m_iTruth != RZ_DutyTable.CLEAN)
				any = true;

			t.m_aSubjects.Insert(s);
		}

		// Never a table with nothing on it to find
		if (!any)
		{
			RZ_DutySubject one = t.m_aSubjects[Math.RandomInt(0, count)];
			one.m_iTruth = RZ_DutyTable.ODD;
			if (Math.RandomFloat01() < bad / (bad + odd))
				one.m_iTruth = RZ_DutyTable.BAD;
		}

		foreach (int idx, RZ_DutySubject made : t.m_aSubjects)
		{
			made.m_iBadSign = Math.RandomInt(0, 2);
			made.m_sTitle = SubjectTitle(kind, idx, used);
			// The tell points the right way three times in four
			int points = made.m_iTruth;
			if (Math.RandomFloat01() > TELL_TRUE)
				points = (made.m_iTruth + 1 + Math.RandomInt(0, 2)) % 3;

			made.m_sTell = Tell(kind, points, fromTown, used);
		}

		t.m_iLooks = count + 2;
		if (rain && kind == RZ_DutyTable.KIND_LINE)
		{
			t.m_iLooks = t.m_iLooks - 1;
			t.m_sNote = "It's raining and the queue won't stand for it: one look fewer.";
		}
		else if (infection >= 50 && !fromTown.IsEmpty())
		{
			t.m_sNote = string.Format("%1 is sick. Expect more of them bad.", fromTown);
		}

		m_aTables.Insert(t);
		RZ_Log.Info(string.Format("Duty table: %1 for %2, %3 subjects, %4 looks (errand %5).", KindName(kind), RZ_Util.PlayerName(e.m_iAsker), count, t.m_iLooks, e.m_iId));
		return t;
	}

	//------------------------------------------------------------------------------------------------
	static string KindName(int kind)
	{
		switch (kind)
		{
			case RZ_DutyTable.KIND_DEAD: return "the dead";
			case RZ_DutyTable.KIND_LINE: return "the line";
			case RZ_DutyTable.KIND_BENCH: return "the bench";
		}

		return "table";
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
			case RZ_DutyTable.KIND_DEAD: return string.Format("TAG %1", 101 + index * 7 + Math.RandomInt(0, 6));
			case RZ_DutyTable.KIND_BENCH: return string.Format("TRAY %1", 11 + index * 4 + Math.RandomInt(0, 3));
		}

		array<string> who = {"A woman, forties", "Two brothers", "A courier", "A lad with a pack", "An old man", "A mother and child", "A farmhand", "A man in a coat"};
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
			case RZ_DutyTable.KIND_DEAD:
			{
				if (points == RZ_DutyTable.BAD)
				{
					pool.Insert("Found in the street at " + from);
					pool.Insert("Brought in off the road, alone");
					pool.Insert("Sleeve torn, cloth wrapped round it");
				}
				else if (points == RZ_DutyTable.ODD)
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

			case RZ_DutyTable.KIND_LINE:
			{
				if (points == RZ_DutyTable.BAD)
				{
					pool.Insert("Walked in from " + from);
					pool.Insert("Keeps a sleeve pulled down");
					pool.Insert("Won't look at the lamp");
				}
				else if (points == RZ_DutyTable.ODD)
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

			case RZ_DutyTable.KIND_BENCH:
			{
				if (points == RZ_DutyTable.BAD)
				{
					pool.Insert("Came in warm from " + from);
					pool.Insert("Two days in a pack, no ice");
					pool.Insert("The box was dropped");
				}
				else if (points == RZ_DutyTable.ODD)
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
		}

		if (pool.IsEmpty())
			return "";

		return Unused(pool, used);
	}

	// ================================================================ The screen
	//------------------------------------------------------------------------------------------------
	//! Once a second: a table whose work site's hold is done (no work sites: whose player stands at
	//! its place) opens; with no screen here it judges itself, carefully.
	void SecondTick(array<int> ids, array<IEntity> players)
	{
		if (m_aTables.IsEmpty())
			return;

		RZ_WorkSites sites = RZ_WorkSites.Get();
		foreach (RZ_DutyTable t : m_aTables)
		{
			if (t.m_bJudged || t.m_iSerial > 0)
				continue;

			int idx = ids.Find(t.m_iPlayer);
			RZ_Errand e = m_Dealer.FindById(t.m_iErrand);
			if (idx < 0 || !e)
				continue;

			string key = RZ_Errands.WorkKey(e);
			bool ready;
			if (sites && sites.Has(key))
			{
				ready = sites.IsDone(key);
			}
			else
			{
				float d = RZ_Util.DistXZ(players[idx].GetOrigin(), e.m_vPlace);
				if (d > AWAY_RANGE)
					t.m_bAway = true;

				ready = t.m_bAway && d <= SPOT_RANGE;
			}

			if (!ready)
				continue;

			if (!IsUsed() || !Open(t, players[idx]))
				SelfJudge(t, "There's no table to sort them at here, so it was done the careful way: every one of them.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool Open(RZ_DutyTable t, IEntity user)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(t.m_iPlayer);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!ui || !guard)
			return false;

		// Another screen is up, or the server is at its cap: it waits, and opens when it can
		if (guard.IsAnyOpen(t.m_iPlayer) || !guard.Open(t.m_iPlayer, RZ_ScreenGuard.KIND_DUTY, null, t.m_sTitle, t.m_iFaction))
			return true;

		t.m_iSerial = m_iNextSerial;
		m_iNextSerial++;
		t.m_bAway = false;
		Send(t, t.m_sNote);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The screen guard or the player closed it: an unjudged table can be come back to (its work
	//! site is put back, to hold again).
	void OnScreenGone(int playerId)
	{
		RZ_DutyTable t = OpenFor(playerId);
		if (!t)
			return;

		t.m_iSerial = 0;
		if (t.m_bJudged)
			return;

		RZ_Errand e = m_Dealer.FindById(t.m_iErrand);
		if (e)
			m_Dealer.ReopenWork(e);

		m_GM.NotifyPlayer(playerId, t.m_sTitle, "Left as it is. Go back to it to finish: what you turned over stays turned.");
	}

	//------------------------------------------------------------------------------------------------
	protected void Shut(RZ_DutyTable t, string reason)
	{
		t.m_iSerial = 0;
		if (m_GM.GetScreenGuard() && m_GM.GetScreenGuard().IsOpen(t.m_iPlayer, RZ_ScreenGuard.KIND_DUTY))
			m_GM.GetScreenGuard().Close(t.m_iPlayer);

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(t.m_iPlayer);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_DUTY, "X|" + reason);
	}

	//------------------------------------------------------------------------------------------------
	//! No screen: every one gets the careful verdict (half pay, and nothing dangerous).
	protected void SelfJudge(RZ_DutyTable t, string why)
	{
		foreach (RZ_DutySubject s : t.m_aSubjects)
		{
			s.m_iVerdict = RZ_DutyTable.STOP;
		}

		t.Judge();
		m_GM.NotifyPlayer(t.m_iPlayer, t.m_sTitle, why);
		RZ_Log.Info(string.Format("Duty table: errand %1 judged itself (no screen), score %2.", t.m_iErrand, t.m_fScore));
	}

	//------------------------------------------------------------------------------------------------
	//! A move from the screen (RZ_PlayerUIComponent.RpcAsk_DutyMove): subject x 10 + slot (0-2 a look
	//! at that sign, 3-5 a verdict), MOVE_DONE, or MOVE_FAILED. Checked here: the screen is open on
	//! this serial and the table isn't judged.
	void Move(int playerId, int serial, int move)
	{
		RZ_DutyTable t = OpenFor(playerId);
		if (!t || t.m_iSerial != serial)
			return;

		if (move == MOVE_FAILED)
		{
			m_bClientFailed = true;
			t.m_iSerial = 0;
			if (m_GM.GetScreenGuard())
				m_GM.GetScreenGuard().Close(playerId);

			RZ_Log.Warn("Duty table: a client couldn't open it (no RZ_DutyMenu preset or layout?). Tables judge themselves from now on.");
			if (!t.m_bJudged)
				SelfJudge(t, "There's no table to sort them at here, so it was done the careful way: every one of them.");

			return;
		}

		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_DUTY))
		{
			t.m_iSerial = 0;
			return;
		}

		guard.Touch(playerId);
		if (t.m_bJudged)
			return;

		string status = "";
		if (move == MOVE_DONE)
		{
			if (!t.AllGiven())
			{
				Send(t, "Every one of them needs a verdict first.");
				return;
			}

			t.Judge();
			RZ_Log.Info(string.Format("Duty table: errand %1 judged for %2: %3 of %4 right, score %5, danger %6.", t.m_iErrand, RZ_Util.PlayerName(playerId), t.m_iRight, t.m_aSubjects.Count(), t.m_fScore, t.m_bDanger));
			Send(t, t.Summary());
			return;
		}

		int subject = move / 10;
		int slot = move % 10;
		if (move < 0 || subject >= t.m_aSubjects.Count() || slot > 5)
			return;

		RZ_DutySubject s = t.m_aSubjects[subject];
		if (slot <= 2)
		{
			if (s.m_aLooked[slot])
				return;

			if (t.m_iLooks <= 0)
			{
				Send(t, "No looks left. Give the rest a verdict on what you know.");
				return;
			}

			s.m_aLooked[slot] = true;
			t.m_iLooks = t.m_iLooks - 1;
		}
		else
		{
			s.m_iVerdict = slot - 3;
		}

		Send(t, status);
	}

	//------------------------------------------------------------------------------------------------
	//! The table as it stands. T|serial|kind|title|where|looks|judged|status; L|line (three);
	//! V|pass|check|stop; S|index|title|tell|verdict|result, then name|word|look for each sign.
	protected void Send(RZ_DutyTable t, string status)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(t.m_iPlayer);
		if (!ui || t.m_iSerial <= 0)
			return;

		int judged = 0;
		if (t.m_bJudged)
			judged = 1;

		string payload = string.Format("T|%1|%2|%3|%4|%5|%6|%7", t.m_iSerial, t.m_iKind, RZ_UIText.Clean(t.m_sTitle, 60), RZ_UIText.Clean(t.m_sSub, 80), t.m_iLooks, judged, RZ_UIText.Clean(status, 200));
		array<string> legend = {};
		RZ_DutyTable.Legend(t.m_iKind, legend);
		foreach (string line : legend)
		{
			payload += "\nL|" + line;
		}

		array<string> verdicts = {};
		RZ_DutyTable.VerdictNames(t.m_iKind, verdicts);
		payload += string.Format("\nV|%1|%2|%3", verdicts[0], verdicts[1], verdicts[2]);
		array<string> names = {};
		RZ_DutyTable.SignNames(t.m_iKind, names);
		foreach (int i, RZ_DutySubject s : t.m_aSubjects)
		{
			payload += string.Format("\nS|%1|%2|%3|%4|%5", i, RZ_UIText.Clean(s.m_sTitle, 30), RZ_UIText.Clean(s.m_sTell, 60), s.m_iVerdict, s.m_iResult);
			for (int sign = 0; sign < 3; sign++)
			{
				int look = t.SignLook(s, sign);
				payload += string.Format("|%1|%2|%3", names[sign], RZ_DutyTable.SignWord(t.m_iKind, sign, look), look);
			}
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_DUTY, payload);
	}
}
