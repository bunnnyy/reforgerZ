//! The closing work (docs/EPILOGUE.md, Finishing everything): every story the season opened can
//! end inside the week, live if someone plays it, as data if nobody can. Nothing here pays
//! anything that carries into next season except hall-of-fame lines.
//!
//! Items that can close on their own (Loose Ends, the majors' codas, the Last Broadcast, the Long
//! Table's week, the ridges) are read against live signals: the room left before processing x the
//! share of hours with someone online, against what they need; a last call in the giver's voice,
//! a grace of one session of play, then the default. Processing is the hard stop. Bring Them Home,
//! Settle Accounts, the Walk Out and the Last Entry stay open to the wipe.

//------------------------------------------------------------------------------------------------
//! Finds a vehicle near a point (the Mechanic's car for the queue). VERIFY: Vehicle entities are
//! found by a dynamic sphere query.
class RZ_EpiCarQuery
{
	bool m_bFound;

	//------------------------------------------------------------------------------------------------
	bool OnEntity(IEntity e)
	{
		if (Vehicle.Cast(e))
		{
			m_bFound = true;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Near(vector pos, float radius)
	{
		RZ_EpiCarQuery q = new RZ_EpiCarQuery();
		GetGame().GetWorld().QueryEntitiesBySphere(pos, radius, q.OnEntity, null, EQueryEntitiesFlags.DYNAMIC);
		return q.m_bFound;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_EpiClosing
{
	protected static const int HOUR = 3600;
	protected static const int PAID_DELIVERIES = 3;
	protected static const float HOLD_SECONDS = 5;
	protected static const float DECIDE_RADIUS = 30;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;

	// The grey choice on the table (runtime: a restart drops an undecided one)
	protected int m_iPending = -1;
	protected bool m_bPendingFirst;
	protected float m_fPendingAt;
	protected int m_iPendingBy = -1;
	protected vector m_vPendingAt;
	protected ref array<string> m_aObjected = {};
	protected float m_fClock;
	protected int m_iLastWalkHour = -1;
	protected float m_fAirClock;
	protected bool m_bAiredDusk;

	//------------------------------------------------------------------------------------------------
	void RZ_EpiClosing(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpilogueSave S()
	{
		return m_Epi.Data();
	}

	// ================================================================ Counters (saved)
	//------------------------------------------------------------------------------------------------
	int Count(string key)
	{
		int i = S().deedKeys.Find(key);
		if (i < 0)
			return 0;

		return S().deedCounts[i];
	}

	//------------------------------------------------------------------------------------------------
	int Bump(string key, int by = 1)
	{
		int i = S().deedKeys.Find(key);
		if (i < 0)
		{
			S().deedKeys.Insert(key);
			S().deedCounts.Insert(by);
			return by;
		}

		S().deedCounts[i] = S().deedCounts[i] + by;
		return S().deedCounts[i];
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	protected int Day()
	{
		return RZ_WorldFacts.Get().Day();
	}

	// ================================================================ The items
	//------------------------------------------------------------------------------------------------
	//! At the commit: every open questline run (Loose Ends), the majors' codas still to run, the Last
	//! Broadcast, the Long Table's week, the ridges.
	void List()
	{
		S().items.Clear();
		if (!m_Epi.ClosingOn())
			return;

		RZ_Questlines q = m_GM.GetQuestlines();
		if (q)
		{
			foreach (RZ_QuestRun run : q.GetRuns())
			{
				if (run.m_bDone || !run.m_Line || run.m_Line.IsCureFinale())
					continue;

				if (run.m_Line.IsMajor())
				{
					if (RZ_MajorHelp.World(m_GM, run.m_sKey + ".coda") == 0)
						AddItem("coda." + run.m_sKey, run.m_Line.m_sTitle, run.m_Line.m_iFaction, PlaceOf(run), 1, "", "");

					continue;
				}

				if (run.m_sKey == "table" || run.m_sKey == "place")
					continue;

				string giver;
				string call;
				LooseCall(run.m_sKey, giver, call);
				AddItem("loose." + run.m_sKey, run.m_Line.m_sTitle, run.m_Line.m_iFaction, PlaceOf(run), 1, giver, call);
			}
		}

		AddItem("broadcast", "The Last Broadcast", -1, StationPos(), 2, "The station", "\"The station goes out at processing, with or without your names.\"");

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts.Flag("table.exists") != 0 && facts.Flag("table.exposed") == 0 && facts.Flag("table.burnt") == 0)
			AddItem("table", "", -1, vector.Zero, 0, "", "");

		if (!S().packIds.IsEmpty())
			AddItem("ridges", "Quiet the Ridges", -1, vector.Zero, 0, "", "");
	}

	//------------------------------------------------------------------------------------------------
	protected void AddItem(string key, string title, int faction, vector at, float need, string giver, string call)
	{
		RZ_EpiItem it = new RZ_EpiItem();
		it.key = key;
		it.title = title;
		it.faction = faction;
		it.x = at[0];
		it.z = at[2];
		it.need = need;
		it.giver = giver;
		it.lastCall = call;
		S().items.Insert(it);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_EpiItem Item(string key)
	{
		foreach (RZ_EpiItem it : S().items)
		{
			if (it.key == key)
				return it;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	int OpenCount()
	{
		int n = 0;
		foreach (RZ_EpiItem it : S().items)
		{
			if (it.state < 2)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a run's story is: its place, else its faction's stronghold seat.
	protected vector PlaceOf(RZ_QuestRun run)
	{
		if (run.m_bHasPlace)
			return run.m_vPlace;

		RZ_Leader top = RZ_MajorHelp.Top(m_GM, run.m_Line.m_iFaction);
		if (top)
			return RZ_MajorHelp.NearSeat(top.m_vPos);

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	protected vector StationPos()
	{
		RZ_RadioStation station = m_GM.GetRadioStation();
		if (!station || !station.IsActive())
			return vector.Zero;

		return station.GetPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! Every closing check (about every 10 s with players online).
	void Tick(float step, array<int> ids, array<IEntity> players)
	{
		m_fClock += step;
		Decide();
		Squeeze(step);
		Towns();
		Ridges();
		Wanted();
		TableWeek(players);
		QueueWalks();
		Air(step);
	}

	//------------------------------------------------------------------------------------------------
	//! The squeeze: room left against what the items still need; a last call, a grace, the default.
	protected void Squeeze(float step)
	{
		if (!m_Epi.ClosingOn())
			return;

		int phase = m_Epi.Phase();
		float room = m_Epi.RoomHours();
		float scale = m_Epi.Scale();
		int now = RZ_SeasonClock.Now();
		foreach (RZ_EpiItem it : S().items)
		{
			if (it.state >= 2)
				continue;

			if (CheckLive(it))
				continue;

			// The ridges close with the packs (the tribes' last dusk), the Long Table on its own week
			if (it.key == "ridges" || it.key == "table")
				continue;

			if (phase >= 3)
			{
				CloseDefault(it);
				continue;
			}

			if (now - it.touchUnix < 2 * HOUR * scale)
				continue;

			float others = 0;
			foreach (RZ_EpiItem o : S().items)
			{
				if (o != it && o.state < 2 && o.faction == it.faction)
					others += o.need;
			}

			if (it.state == 0 && room < (it.need + others) * scale)
			{
				it.state = 1;
				it.grace = 0;
				if (!it.lastCall.IsEmpty())
					RZ_Questline.Headline(m_GM, string.Format("%1: %2", it.giver, it.lastCall), true);

				RZ_Log.Info("Epilogue: last call " + it.key);
				continue;
			}

			if (it.state == 1)
			{
				it.grace += step;
				if (it.grace >= HOUR * scale)
					CloseDefault(it);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! An item whose story ended some other way (the coda ran, the questline ended): closed live.
	protected bool CheckLive(RZ_EpiItem it)
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (it.key.StartsWith("coda."))
		{
			string major = it.key.Substring(5, it.key.Length() - 5);
			if (RZ_MajorHelp.World(m_GM, major + ".coda") != 0 || !q || !q.IsRunning(major))
			{
				Closed(it, 2, "");
				return true;
			}

			return false;
		}

		if (it.key.StartsWith("loose."))
		{
			string key = it.key.Substring(6, it.key.Length() - 6);
			if (!q || !q.IsRunning(key))
			{
				Closed(it, 2, "");
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void Closed(RZ_EpiItem it, int state, string line)
	{
		if (it.state >= 2)
			return;

		it.state = state;
		it.endLine = line;
		string how = "live";
		if (state == 3)
			how = "default";

		RZ_Log.Info(string.Format("Epilogue: closed %1 (%2)", it.key, how));
		if (it.key.StartsWith("loose.") && !it.title.IsEmpty() && !line.IsEmpty())
			m_Epi.AddHallLine(string.Format("%1: %2", it.title, line));

		m_Epi.GetSpots().Dirty();
	}

	//------------------------------------------------------------------------------------------------
	//! The default ending, as data: no pay, its line said plainly.
	protected void CloseDefault(RZ_EpiItem it)
	{
		if (it.key.StartsWith("loose."))
		{
			string key = it.key.Substring(6, it.key.Length() - 6);
			array<string> labels = {};
			array<int> endings = {};
			array<string> lines = {};
			int defEnding;
			string defLine;
			string stage;
			RZ_QuestRun run = m_GM.GetQuestlines().FindRunByKey(key);
			if (LooseDef(key, labels, endings, lines, defEnding, defLine, stage))
				EndRun(run, defEnding, defLine);
			else
				EndRun(run, -1, string.Format("%1 ended with the season, unfinished.", it.title));

			if (!defLine.IsEmpty())
				RZ_Questline.Headline(m_GM, defLine, false);

			Closed(it, 3, defLine);
			return;
		}

		if (it.key == "broadcast")
		{
			CloseBroadcast();
			return;
		}

		Closed(it, 3, "");
	}

	//------------------------------------------------------------------------------------------------
	//! Admin and test: close one item as data now.
	bool CloseNow(string key)
	{
		RZ_EpiItem it = Item(key);
		if (!it || it.state >= 2)
			return false;

		if (key == "table")
		{
			S().tableState = 2;
			Closed(it, 3, "");
			return true;
		}

		if (key == "ridges")
		{
			Closed(it, 3, "");
			return true;
		}

		CloseDefault(it);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A run ends on `ending` (-1: it fizzles with `line`): the questline's own ending and its world
	//! effect, as built.
	protected void EndRun(RZ_QuestRun run, int ending, string line)
	{
		RZ_Questlines q = m_GM.GetQuestlines();
		if (!run || run.m_bDone || !q)
			return;

		run.m_iEnding = ending;
		run.m_sEndText = run.Fill(line);
		q.JumpTo(run, RZ_Questline.END);
	}

	// ================================================================ Loose Ends
	//------------------------------------------------------------------------------------------------
	//! The last call in the questline's own person's voice.
	protected void LooseCall(string key, out string giver, out string call)
	{
		giver = "Word from the houses";
		call = "\"It ends this week, one way or another. Better if someone's there.\"";
		switch (key)
		{
			case "door": giver = "The council head"; call = "\"The crew's man waits for the last envelope. Tomorrow, with or without you.\""; break;
			case "walk": giver = "The walker"; call = "\"There's one more who needs walking. I'll do it myself if nobody comes.\""; break;
			case "haven_short_weight": giver = "The head"; call = "\"The stores get counted tomorrow at supper, with or without you.\""; break;
			case "census": giver = "The CO"; call = "\"The count goes on the board as last reported, unless somebody brings a better one.\""; break;
			case "pilot": giver = "The pilot"; call = "\"I'm flying at dawn, with or without anyone.\""; break;
			case "cargo": giver = "The driver"; call = "\"The van goes tomorrow. I'd rather it went somewhere else.\""; break;
			case "well": giver = "The village"; call = "\"The crew wants its last payment. We'll find the tins if nobody stands with us.\""; break;
			case "other_two": giver = "The director"; call = "\"His coat is still on the hook. We'll label it.\""; break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A questline's closing stage (docs/QUESTLINES.md, Loose Ends): its ways, each ending on one of
	//! its built endings, and its default. False for a questline with no table row.
	protected bool LooseDef(string key, notnull array<string> labels, notnull array<int> endings, notnull array<string> lines, out int defEnding, out string defLine, out string stage)
	{
		defEnding = -1;
		defLine = "";
		stage = "";
		switch (key)
		{
			case "door":
			{
				stage = "The Last Envelope";
				labels.Insert("Carry the last envelope"); endings.Insert(RZ_Quest_PriceOfTheDoor.E_TINS); lines.Insert("Paid in tins. The last envelope went to the crew, carried by someone who knew what was in it.");
				labels.Insert("Bring the exile home first"); endings.Insert(RZ_Quest_PriceOfTheDoor.E_DEBT); lines.Insert("The debt is yours. The exile came home, and the head paid the crew in tins instead.");
				labels.Insert("Tell the chair"); endings.Insert(RZ_Quest_PriceOfTheDoor.E_STANDS); lines.Insert("Stands alone. The arrangement ended, with the house's line in the book.");
				defEnding = RZ_Quest_PriceOfTheDoor.E_TINS;
				defLine = "The head sent a resident with the last envelope. Nobody asked who carried it.";
				return true;
			}
			case "walk":
			{
				stage = "The Last Walk";
				labels.Insert("Get them to a dose"); endings.Insert(RZ_Quest_EdgeOfTown.E_LAB_ROAD); lines.Insert("The lab road. The last one who declared a bite went to a dose, and came back.");
				labels.Insert("Let the walker walk them"); endings.Insert(RZ_Quest_EdgeOfTown.E_WALKS); lines.Insert("The walks stay. One more, at the edge of town, the week the cure came.");
				labels.Insert("Sit with them, let the nurse decide"); endings.Insert(RZ_Quest_EdgeOfTown.E_LAB_ROAD); lines.Insert("The lab road. The nurse decided, and somebody sat with them while she did.");
				defEnding = RZ_Quest_EdgeOfTown.E_WALKS;
				defLine = "The walker walked one more and read twelve names at supper. The nearest dose was four kilometres away.";
				return true;
			}
			case "haven_short_weight":
			{
				stage = "Count the Stores";
				labels.Insert("Count it honestly"); endings.Insert(RZ_Quest_ShortWeight.E_FULL); lines.Insert("Full Weight. The stores were counted before the house packed for the queue.");
				labels.Insert("Cover for him"); endings.Insert(RZ_Quest_ShortWeight.E_SHORT); lines.Insert("Short Weight. The books stayed a little light, all the way to the queue.");
				labels.Insert("Let the head find it at supper"); endings.Insert(RZ_Quest_ShortWeight.E_FULL); lines.Insert("Full Weight. The head found it at supper. The storekeeper eats last.");
				defEnding = RZ_Quest_ShortWeight.E_FULL;
				defLine = "The head found the short weight at supper. The storekeeper eats last.";
				return true;
			}
			case "census":
			{
				stage = "The Last Count";
				labels.Insert("Bring the real count"); endings.Insert(RZ_Quest_Census.E_COUNTED); lines.Insert("Counted. The real number went on the roster board before the last call.");
				labels.Insert("Forge it"); endings.Insert(RZ_Quest_Census.E_FORGED); lines.Insert("Forged. The board says a number. It isn't the number.");
				labels.Insert("Burn the cards"); endings.Insert(RZ_Quest_Census.E_REFUSED); lines.Insert("Refused. The cards went in the stove, the Haven's way.");
				defEnding = RZ_Quest_Census.E_COUNTED;
				defLine = "The count stays as last reported. A clerk files it unsigned.";
				return true;
			}
			case "pilot":
			{
				stage = "The Last Flight";
				labels.Insert("Fly with him, bring him back"); endings.Insert(RZ_Quest_LastPilot.E_STAYS); lines.Insert("He stays. One last flight round the Zone at dawn, inside the line, and his name for the roll.");
				labels.Insert("Talk him into walking instead"); endings.Insert(RZ_Quest_LastPilot.E_GROUNDED); lines.Insert("Grounded. He walks with the column at 05:30 instead.");
				labels.Insert("Sell the aircraft for parts"); endings.Insert(RZ_Quest_LastPilot.E_BURNT); lines.Insert("Burnt for the Red Hand. What the crew couldn't sell, it burned.");
				defEnding = RZ_Quest_LastPilot.E_GROUNDED;
				defLine = "He didn't fly. The aircraft sits on the pad with its doors open.";
				return true;
			}
			case "cargo":
			{
				stage = "The Last Load";
				labels.Insert("Drive the van to the queue"); endings.Insert(RZ_Quest_Cargo.E_TOLD); lines.Insert("Told. The van went to the queue with whoever was in the back.");
				labels.Insert("Hand the van to the crew"); endings.Insert(RZ_Quest_Cargo.E_CUT); lines.Insert("The cut. The last load went where the crew said.");
				labels.Insert("Follow it"); endings.Insert(RZ_Quest_Cargo.E_TOLD); lines.Insert("Told. Somebody followed the last van, and wrote down where it stopped.");
				defEnding = RZ_Quest_Cargo.E_CUT;
				defLine = "The van went, and came back empty. The driver walked to the queue alone.";
				return true;
			}
			case "well":
			{
				stage = "The Last Toll";
				labels.Insert("Pay it yourself (15 Red Hand Favor)"); endings.Insert(RZ_Quest_WellStory.E_WORKED); lines.Insert("The story worked. Somebody paid the last toll for the village.");
				labels.Insert("Stand with the village"); endings.Insert(RZ_Quest_WellStory.E_STOOD); lines.Insert("Stood up. The village walked out with its last tins.");
				labels.Insert("Tell the story one last time"); endings.Insert(RZ_Quest_WellStory.E_FAKED); lines.Insert("Faked. One more telling, and nobody paid.");
				defEnding = RZ_Quest_WellStory.E_WORKED;
				defLine = "The village paid in its last tins and walked out lighter.";
				return true;
			}
			case "other_two":
			{
				stage = "His Coat";
				labels.Insert("Carry the coat to his sister"); endings.Insert(RZ_Quest_OtherTwo.E_HOME); lines.Insert("Brought home. His sister has the coat.");
				labels.Insert("Take his number to her board"); endings.Insert(RZ_Quest_OtherTwo.E_NAMED); lines.Insert("Named. His number went up on her house's board, with his name over it.");
				labels.Insert("Leave it on the hook"); endings.Insert(RZ_Quest_OtherTwo.E_NUMBERS); lines.Insert("The work goes on. The coat stays on the hook at the lab.");
				defEnding = RZ_Quest_OtherTwo.E_NUMBERS;
				defLine = "The coat stays on the hook. Nobody came for it. The lab labelled it.";
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A way of a closing stage: the questline ends on that built ending; the player is paid half
	//! a stage (Fame 3, Favor 5 with the questline's faction), once.
	protected void LooseWay(int playerId, string key, int way)
	{
		RZ_EpiItem it = Item("loose." + key);
		RZ_QuestRun run = m_GM.GetQuestlines().FindRunByKey(key);
		if (!it || it.state >= 2 || !run)
			return;

		array<string> labels = {};
		array<int> endings = {};
		array<string> lines = {};
		int defEnding;
		string defLine;
		string stage;
		if (!LooseDef(key, labels, endings, lines, defEnding, defLine, stage) || way < 0 || way >= endings.Count())
			return;

		if (key == "well" && way == 0 && !m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, 15))
		{
			m_GM.NotifyPlayer(playerId, stage, "\"One tin. Fifteen Favor. You haven't got it.\"");
			return;
		}

		run.Did(m_GM, playerId, RZ_EQuestApproach.TALK, labels[way]);
		run.Reward(m_GM, playerId, run.m_Line.m_iFaction, 3, 5, lines[way]);
		vector at = Vector(it.x, 0, it.z);
		EndRun(run, endings[way], lines[way]);
		Closed(it, 2, lines[way]);
		m_Epi.GetBeats().AddSince("loose." + key, stage, at, lines[way], string.Format("%1 ended this week.", run.m_Line.m_sTitle));
		m_Epi.Deed(playerId, "loose");
	}

	// ================================================================ Spots
	//------------------------------------------------------------------------------------------------
	//! The closing work's places for RZ_EpiSpots: keepsakes lying out, Loose Ends' people, the
	//! station's tally, the farm's basket.
	void Spots(notnull array<ref RZ_EpiSpot> result)
	{
		RZ_EpilogueSave s = S();
		if (!m_Epi.ClosingOn())
			return;

		foreach (int i, RZ_EpiName n : s.names)
		{
			if (n.state != 0)
				continue;

			RZ_EpiSpot keep = new RZ_EpiSpot();
			keep.key = "keep." + n.name + "." + n.day.ToString();
			keep.pos = Vector(n.x, 0, n.z);
			keep.title = "Something lying here";
			keep.talk = string.Format("%1: %2, %3.", RZ_Util.Capitalize(n.what), n.name, n.role);
			keep.Verb("Take " + n.what, "take:" + i.ToString());
			result.Insert(keep);
		}

		foreach (RZ_EpiItem it : s.items)
		{
			if (it.state >= 2 || !it.key.StartsWith("loose.") || (it.x == 0 && it.z == 0))
				continue;

			string key = it.key.Substring(6, it.key.Length() - 6);
			array<string> labels = {};
			array<int> endings = {};
			array<string> lines = {};
			int defEnding;
			string defLine;
			string stage;
			if (!LooseDef(key, labels, endings, lines, defEnding, defLine, stage))
				continue;

			RZ_EpiSpot loose = new RZ_EpiSpot();
			loose.key = it.key;
			loose.person = true;
			loose.pos = RZ_Mission.LandPointNear(Vector(it.x, 0, it.z), 3, 6);
			loose.title = string.Format("%1: %2", it.title, stage);
			loose.talk = it.lastCall;
			foreach (int w, string label : labels)
			{
				loose.Verb(label, string.Format("loose:%1:%2", key, w));
			}

			result.Insert(loose);
		}

		RZ_EpiItem cast = Item("broadcast");
		vector station = StationPos();
		if (cast && cast.state < 2 && station != vector.Zero && m_Epi.Phase() < 3)
		{
			RZ_EpiSpot radio = new RZ_EpiSpot();
			radio.key = "station";
			radio.pos = RZ_Mission.LandPointNear(station, 4, 7);
			radio.title = "The radio station";
			radio.talk = BroadcastTally();
			radio.Verb("Say: read the names", "say:0");
			if (BroadcastAllowed(1))
				radio.Verb("Say: read the three weeks", "say:1");

			if (BroadcastAllowed(2))
				radio.Verb("Say: tell them the truth", "say:2");

			radio.Verb("Say: keep it plain", "say:3");
			result.Insert(radio);
		}

		RZ_LongTable table = m_GM.GetLongTable();
		if (s.tableState == 1 && table && table.Exists())
		{
			RZ_EpiSpot basket = new RZ_EpiSpot();
			basket.key = "basket";
			basket.pos = RZ_Mission.LandPointNear(table.GetHousePos(), 8, 12);
			basket.title = "A handcart";
			basket.talk = "A handcart in the yard, half loaded. Covered baskets.";
			basket.Verb("Look at the basket", "basket");
			result.Insert(basket);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A verb on a spot (RZ_EpiSpots.HandleChoice).
	void Verb(int playerId, string verb, RZ_EpiSpot spot, IEntity user)
	{
		if (verb.StartsWith("take:"))
		{
			Take(playerId, verb.Substring(5, verb.Length() - 5).ToInt(), user);
			return;
		}

		if (verb.StartsWith("loose:"))
		{
			array<string> parts = {};
			verb.Split(":", parts, true);
			if (parts.Count() >= 3)
				LooseWay(playerId, parts[1], parts[2].ToInt());

			return;
		}

		if (verb.StartsWith("say:"))
		{
			Say(playerId, verb.Substring(4, verb.Length() - 4).ToInt());
			return;
		}

		switch (verb)
		{
			case "cup": Cup(playerId, user, spot); break;
			case "down": PutDown(playerId, spot); break;
			case "car": Car(playerId, spot); break;
			case "stash": Stash(playerId, spot); break;
			case "front": Front(playerId, spot); break;
			case "line": WriteLine(playerId, ""); break;
			case "walk": WalkOut(playerId, user); break;
			case "home": Home(playerId, spot); break;
			case "read": ReadNames(playerId, spot); break;
			case "book": PayBook(playerId, spot); break;
			case "basket": Basket(playerId); break;
			default: m_Epi.GetRest().Verb(playerId, verb, spot, user); break;
		}
	}

	// ================================================================ The queue
	//------------------------------------------------------------------------------------------------
	//! Bring a Cup: one food item for the queue, once a real day (x Test scale); a food hand-in's
	//! small pay with the Haven.
	protected void Cup(int playerId, IEntity user, RZ_EpiSpot spot)
	{
		string ident = Ident(playerId);
		int day = RZ_SeasonClock.Now() / Math.Max(60, RZ_Util.RoundToInt(86400 * m_Epi.Scale()));
		string key = string.Format("cup|%1|%2", ident, day);
		if (Count(key) > 0)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"You brought one already today. Sit down a minute.\"");
			return;
		}

		array<string> words = {};
		if (m_GM.GetSafehouses())
			words.InsertAll(m_GM.GetSafehouses().GetFoodKeywords());

		if (RZ_FoodHelper.TakeFood(user, words, 1) <= 0)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"A cup of what? Bring something to put in it.\"");
			return;
		}

		Bump(key);
		m_GM.AddFame(playerId, RZ_EFaction.SURVIVORS, 1);
		m_GM.NotifyPlayer(playerId, spot.title, "It goes down the queue, a mouthful each. Somebody says thank you without looking up.");
		m_Epi.Deed(playerId, "cup");
	}

	//------------------------------------------------------------------------------------------------
	//! A character alive long enough to walk out: 2 hours played this life (x Test scale), and the
	//! identity hasn't walked out this season.
	protected string CanWalk(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p.m_fLifeSeconds < 7200 * m_Epi.Scale())
			return "\"You've not been here long enough to be on a list. Two hours, then come back.\"";

		foreach (string w : S().walked)
		{
			if (w.StartsWith(p.m_sIdentity + "|"))
				return "\"You've been through once. Nobody comes back.\"";
		}

		// The order by the route's terms (processing's first day)
		string order = m_Epi.GetRest().WalkOrder(playerId);
		if (!order.IsEmpty())
			return order;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Put your name down for the walk (phase B): read at the wipe if you're not there.
	protected void PutDown(int playerId, RZ_EpiSpot spot)
	{
		string why = CanWalk(playerId);
		if (!why.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, spot.title, why);
			return;
		}

		string ident = Ident(playerId);
		foreach (string d : S().walkDown)
		{
			if (d.StartsWith(ident + "|"))
			{
				m_GM.NotifyPlayer(playerId, spot.title, "\"You're on it already. Third from the bottom.\"");
				return;
			}
		}

		S().walkDown.Insert(ident + "|" + RZ_EpiNames.Now(playerId));
		m_GM.NotifyPlayer(playerId, spot.title, "Your name goes on the list, in pencil. If you're not here when it moves, someone will answer for you.");
		m_Epi.Deed(playerId, "down");
	}

	//------------------------------------------------------------------------------------------------
	//! One for the queue (the Mechanic's last goal): a car left within 25 m of the queue.
	protected void Car(int playerId, RZ_EpiSpot spot)
	{
		if (!RZ_EpiCarQuery.Near(spot.pos, 25))
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"What car?\"");
			return;
		}

		string key = "car|" + Ident(playerId);
		if (Count(key) > 0)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"We've got yours. It's a good one.\"");
			return;
		}

		Bump(key);
		m_GM.NotifyPlayer(playerId, spot.title, "\"For the queue?\" Someone chalks FOR THE QUEUE on the bonnet. Three people sit in it out of the wind.");
		m_Epi.Deed(playerId, "car");
	}

	//------------------------------------------------------------------------------------------------
	//! The stash (the Ex-Con's last goal): whatever you brought, left for whoever needs it.
	protected void Stash(int playerId, RZ_EpiSpot spot)
	{
		m_GM.NotifyPlayer(playerId, spot.title, "You leave it on the verge by the queue. By the time you look back, it's gone, and nobody's looking at you.");
		m_Epi.Deed(playerId, "stash");
	}

	//------------------------------------------------------------------------------------------------
	//! A place near the front (the Red Hand's queue in processing): 15 Favor, which buys a place.
	protected void Front(int playerId, RZ_EpiSpot spot)
	{
		if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, 15))
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"Fifteen. The queue's free. The front isn't.\"");
			return;
		}

		m_GM.NotifyPlayer(playerId, spot.title, "The lad moves your bag up three places. Nobody argues. Nobody looks at the lad either.");
	}

	//------------------------------------------------------------------------------------------------
	//! The queue's table line, with what changed: the names put down, the walks, the farm's people.
	string QueueTableLine()
	{
		string line = "";
		int down = S().walkDown.Count();
		if (down > 0)
			line += string.Format(" %1 names put down for the walk.", down);

		if (Count("walks") > 0)
			line += " The queue moved this morning. It hasn't since.";

		if (S().tableState == 2)
			line += " At the back, people with covered baskets.";

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! In processing, at each change of shift (05:30 and 17:30): the queue walks up to the barrier,
	//! one name at a time, and new people join at the back from the houses.
	protected void QueueWalks()
	{
		if (m_Epi.Phase() < 3)
			return;

		float h = RZ_GameTime.GetHour();
		if (h < 0)
			return;

		int shift = -1;
		if (h >= 5.5 && h < 6.5)
			shift = 0;
		else if (h >= 17.5 && h < 18.5)
			shift = 1;

		if (shift < 0)
		{
			m_iLastWalkHour = -1;
			return;
		}

		if (m_iLastWalkHour == shift)
			return;

		m_iLastWalkHour = shift;
		Bump("walks");
		RZ_Safehouses houses = m_GM.GetSafehouses();
		for (int i = 0; houses && i < houses.Count(); i++)
		{
			if (Math.RandomFloat01() < 0.5)
				Bump("gone|" + i.ToString());
		}

		m_GM.NotifyAll("The north road", "The queue walks up to the barrier. At the arm, one name at a time: \"Name?\" Then the next. New people join at the back.");
		RZ_Log.Info(string.Format("Epilogue: walked out (%1 today)", Count("walks")));
	}

	//------------------------------------------------------------------------------------------------
	int GoneHome(int house)
	{
		return Count("gone|" + house.ToString());
	}

	// ================================================================ The Walk Out
	//------------------------------------------------------------------------------------------------
	//! The Perimeter's turn-back at the north road in processing (World/RZ_Perimeter.c): an unarmed
	//! person on the road at the barrier hears "Name?" instead. True: not turned back.
	bool LetThrough(int playerId, IEntity player, vector pos)
	{
		if (!m_Epi.ClosingOn() || m_Epi.Phase() < 3 || !m_Epi.GetSettings().m_bWalkOut || !player)
			return false;

		vector gate = RZ_WorldFacts.Get().CanonPos("day2.checkpoint");
		if (gate == vector.Zero || RZ_Util.DistXZ(gate, pos) > 90 || RZ_Util.IsArmed(player))
			return false;

		string key = string.Format("name?|%1", playerId);
		if (Count(key) == 0)
		{
			Bump(key);
			m_GM.Say(playerId, "The barrier", -1, "A speaker on a post, and nobody in sight past it. \"Name?\"");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Give your name at the barrier: the character is retired. No death, no grave, no journal; the
	//! inventory goes with it. Back to the deploy menu.
	protected void WalkOut(int playerId, IEntity user)
	{
		if (!user || m_Epi.Phase() < 3)
			return;

		if (RZ_Util.IsArmed(user))
		{
			m_GM.NotifyPlayer(playerId, "The barrier", "\"Weapons stay this side.\" Nobody takes it from you. Nobody lifts the arm either.");
			return;
		}

		string why = CanWalk(playerId);
		if (!why.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, "The barrier", why);
			return;
		}

		if (m_GM.GetSpawner() && m_GM.GetSpawner().IsZombieNear(user.GetOrigin(), 40))
		{
			m_GM.NotifyPlayer(playerId, "The barrier", "Not with them this close. The speaker says nothing.");
			return;
		}

		string ident = Ident(playerId);
		string name = RZ_EpiNames.Now(playerId);
		if (name.IsEmpty() || name == "someone")
			name = "a survivor";

		S().walked.Insert(string.Format("%1|%2|%3", ident, name, Day()));
		for (int i = S().walkDown.Count() - 1; i >= 0; i--)
		{
			if (S().walkDown[i].StartsWith(ident + "|"))
				S().walkDown.Remove(i);
		}

		foreach (int w, string want : S().wantIdents)
		{
			if (want == ident && S().wantState[w] == 0)
				S().wantState[w] = 4;
		}

		m_Epi.Deed(playerId, "walk");
		m_GM.NotifyPlayer(playerId, "The barrier", string.Format("\"Name?\"\n\"%1.\"\nThe arm lifts a hand's width. You go through. Nobody tells you what's on the other side.", name));
		RZ_Log.Info(string.Format("Epilogue: walked out (%1 in all)", S().walked.Count()));

		// VERIFY: a character ended this way returns its player to the deploy menu with no death
		// screen; its body and inventory are removed a moment later
		m_Epi.MarkWalking(playerId);
		RZ_Util.KillCharacter(user);
		GetGame().GetCallqueue().CallLater(RemoveBody, 2000, false, user, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveBody(IEntity body, int playerId)
	{
		if (body)
			SCR_EntityHelper.DeleteEntityAndChildren(body);
	}

	// ================================================================ Bring Them Home
	//------------------------------------------------------------------------------------------------
	//! Take a keepsake (a mission item in all but name: it's carried as data, never used or traded).
	protected void Take(int playerId, int idx, IEntity user)
	{
		if (idx < 0 || idx >= S().names.Count())
			return;

		RZ_EpiName n = S().names[idx];
		if (n.state != 0)
			return;

		string ident = Ident(playerId);
		if (!n.killedBy.IsEmpty() && n.killedBy == ident)
		{
			m_GM.NotifyPlayer(playerId, "Bring Them Home", "\"You'd know. You were there.\" You leave it where it is.");
			int f = n.faction;
			if (!RZ_Factions.IsValid(f))
				f = RZ_EFaction.SURVIVORS;

			m_GM.AddInfamy(playerId, f, 3);
			return;
		}

		n.state = 1;
		n.carrier = ident;
		m_GM.NotifyPlayer(playerId, "Bring Them Home", string.Format("You take %1: %2. %3", n.what, n.name, HomeWords(n)));
		m_Epi.Deed(playerId, "take");
	}

	//------------------------------------------------------------------------------------------------
	protected string HomeWords(RZ_EpiName n)
	{
		switch (n.faction)
		{
			case RZ_EFaction.MILITARY: return "It goes on the nail behind the radio at any Bastion post.";
			case RZ_EFaction.BANDITS: return "It goes in the book, at Rudi's crossroads.";
			case RZ_EFaction.INSTITUTE: return "It goes into the record, at the lab gate.";
		}

		return "It goes home to a Haven house.";
	}

	//------------------------------------------------------------------------------------------------
	//! Is someone (or `ident`, "" any) carrying a name home to `faction`'s place?
	bool CarryingFor(int playerId, int faction)
	{
		foreach (RZ_EpiName n : S().names)
		{
			if (n.state == 1 && HomeFaction(n) == faction)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static int HomeFaction(RZ_EpiName n)
	{
		if (n.faction == RZ_EFaction.MILITARY || n.faction == RZ_EFaction.BANDITS || n.faction == RZ_EFaction.INSTITUTE)
			return n.faction;

		return RZ_EFaction.SURVIVORS;
	}

	//------------------------------------------------------------------------------------------------
	//! Hand a carried name in at its home: the nail, the book, the lab, a house. Errands' small pay
	//! (Fame 1, Favor 5 with the house's faction), 3 paid a player a real day; your own or your
	//! group's dead for nothing.
	protected void Home(int playerId, RZ_EpiSpot spot)
	{
		int here = RZ_EFaction.SURVIVORS;
		if (spot.key.StartsWith("post."))
			here = RZ_EFaction.MILITARY;
		else if (spot.key == "crossroads")
			here = RZ_EFaction.BANDITS;
		else if (spot.key == "labgate")
			here = RZ_EFaction.INSTITUTE;

		string ident = Ident(playerId);
		RZ_EpiName n;
		foreach (RZ_EpiName c : S().names)
		{
			if (c.state == 1 && c.carrier == ident && HomeFaction(c) == here)
			{
				n = c;
				break;
			}
		}

		if (!n)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "You've nothing of theirs that belongs here.");
			return;
		}

		string place = spot.title;
		place.Replace("Safehouse: ", "");
		n.state = 2;
		n.home = place;
		n.by = RZ_EpiNames.Now(playerId);
		m_Epi.Deed(playerId, "home");
		if (here == RZ_EFaction.INSTITUTE)
			m_Epi.Deed(playerId, "record");

		string said = "The head reads it at supper, and pins it under the house's names.";
		if (here == RZ_EFaction.MILITARY)
			said = "The sergeant hangs it on the nail and reads the name on the next watch.";
		else if (here == RZ_EFaction.BANDITS)
			said = "The boss writes sent off against the line. What he owed stays in the book.";
		else if (here == RZ_EFaction.INSTITUTE)
			said = "The notebook is read into the record, every page.";

		bool own = n.ident == ident || (!n.ident.IsEmpty() && SameGroup(playerId, n.ident));
		if (own)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "You bring your own name home. The clerk writes it in without asking.");
		}
		else
		{
			int today = RZ_SeasonClock.Now() / Math.Max(60, RZ_Util.RoundToInt(86400 * m_Epi.Scale()));
			string key = string.Format("pay|%1|%2", ident, today);
			if (Count(key) < PAID_DELIVERIES)
			{
				Bump(key);
				m_GM.AddFame(playerId, here, 1);
				m_GM.AddFavor(playerId, here, 5);
				said += " (+1 Fame, +5 Favor)";
			}

			m_GM.NotifyPlayer(playerId, spot.title, string.Format("%1 is home. %2", n.name, said));
		}

		m_Epi.GetBeats().AddSince("home." + n.name, place, spot.pos, string.Format("A name brought home this week: %1.", n.name), string.Format("Somebody brought a name home to %1.", place));
		RZ_Log.Info(string.Format("Epilogue: brought home %1 to %2.", n.name, place));
	}

	//------------------------------------------------------------------------------------------------
	protected bool SameGroup(int playerId, string ident)
	{
		int other = 0;
		if (m_GM.GetQuestlines())
			other = m_GM.GetQuestlines().OnlineId(ident);

		return other > 0 && RZ_Util.SameGroup(playerId, other);
	}

	//------------------------------------------------------------------------------------------------
	int WallCount()
	{
		int n = 0;
		foreach (RZ_EpiName c : S().names)
		{
			if (c.state == 2)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The Names board: the names brought home (with where), in processing those nobody carried
	//! ("NOT BROUGHT HOME"), and the last entries (the newest 12). full: for reading it in person.
	string WallText(bool full)
	{
		string home = "";
		string notHome = "";
		int count = 0;
		foreach (RZ_EpiName n : S().names)
		{
			string name = n.name;
			name.ToUpper();
			if (n.state == 2 && count < 30)
			{
				string to = n.home;
				to.ToUpper();
				home += string.Format("%1. %2. ", name, to);
				count++;
			}
			else if (n.state == 3)
			{
				notHome += name + ". ";
			}
		}

		string text = "Boards at the bend, with names on them.";
		if (!home.IsEmpty())
			text += " " + home;

		if (!notHome.IsEmpty())
			text += " In a second column, NOT BROUGHT HOME: " + notHome;

		if (!full)
			return text.Trim();

		int shown = 0;
		int total = S().entries.Count();
		for (int i = total - 1; i >= 0 && shown < 12; i--)
		{
			RZ_EpiEntry e = S().entries[i];
			text += string.Format("\n%1 (%2)", e.text, RZ_EpiNames.Show(e.ident, e.name));
			shown++;
		}

		if (total > shown)
			text += string.Format("\nand %1 more", total - shown);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A carrier died: the keepsake lies where they fell. A name put down comes off.
	void OnDied(int victim, int killer, vector at)
	{
		string ident = Ident(victim);
		foreach (RZ_EpiName n : S().names)
		{
			if (n.state == 1 && n.carrier == ident)
			{
				n.state = 0;
				n.carrier = "";
				n.x = at[0];
				n.z = at[2];
				n.lies = m_Epi.PlaceWords(at);
			}
		}

		for (int i = S().walkDown.Count() - 1; i >= 0; i--)
		{
			if (S().walkDown[i].StartsWith(ident + "|"))
				S().walkDown.Remove(i);
		}

		// A wanted poster changes: paid by a player, or the infected got there first
		foreach (int w, string want : S().wantIdents)
		{
			if (want != ident || S().wantState[w] != 0)
				continue;

			if (killer > 0 && killer != victim)
				S().wantState[w] = 2;
			else
				S().wantState[w] = 3;
		}
	}

	// ================================================================ Houses
	//------------------------------------------------------------------------------------------------
	//! Read the names at supper (the deck's Read Them Out): once per house and identity.
	protected void ReadNames(int playerId, RZ_EpiSpot spot)
	{
		string key = string.Format("read|%1|%2", Ident(playerId), spot.key);
		if (Count(key) > 0)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "You read them last night. They'll read them again tonight without you.");
			return;
		}

		Bump(key);
		string place = spot.title;
		place.Replace("Safehouse: ", "");
		string names = HomedAt(place);
		string text = "You read the house's names at supper, then the ones we don't say.";
		if (!names.IsEmpty())
			text = string.Format("You read the house's names at supper: %1. Then the ones we don't say.", names);

		m_GM.NotifyPlayer(playerId, spot.title, text);
		m_Epi.Deed(playerId, "read");
	}

	//------------------------------------------------------------------------------------------------
	protected string HomedAt(string place)
	{
		string names = "";
		foreach (RZ_EpiName n : S().names)
		{
			if (n.state != 2 || n.home != place)
				continue;

			if (!names.IsEmpty())
				names += ", ";

			names += n.name;
		}

		return names;
	}

	//------------------------------------------------------------------------------------------------
	//! A leader's "The dead": the nearest house's names this season.
	string DeadAt(vector pos)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return "";

		RZ_Safehouse h = houses.Get(houses.FindNearest(pos, 400));
		if (!h)
			return "";

		string names = HomedAt(h.m_sName);
		if (h.m_iSeasonDied <= 0 && names.IsEmpty())
			return "";

		if (names.IsEmpty())
			return string.Format("\"%1 of ours this season. And the ones we don't say.\"", h.m_iSeasonDied);

		return string.Format("\"%1. And the ones we don't say.\"", names);
	}

	//------------------------------------------------------------------------------------------------
	//! Notes pinned to a house's board in the week.
	void BoardNotes(int house, notnull array<string> authors, notnull array<string> texts)
	{
		RZ_Safehouse h = m_GM.GetSafehouses().Get(house);
		if (!h)
			return;

		string names = HomedAt(h.m_sName);
		if (!names.IsEmpty())
		{
			authors.Insert("Pinned by the head");
			texts.Insert("Brought home this week: " + names + ".");
		}

		if (S().choices[0] == 2)
		{
			authors.Insert("Pinned by the head");
			texts.Insert("If we put you out, come and eat.");
		}

		if (m_Epi.Phase() >= 3)
		{
			authors.Insert("Pinned by the head");
			texts.Insert("Last notes here. They'll go with whoever goes last.");
		}
	}

	// ================================================================ Settle Accounts
	//------------------------------------------------------------------------------------------------
	//! Pay into the book at the crossroads: 15 Red Hand Favor, your line marked paid (read aloud as
	//! paid at the last line), and 5 Red Hand Infamy worked off through the forgiveness budget.
	protected void PayBook(int playerId, RZ_EpiSpot spot)
	{
		string ident = Ident(playerId);
		if (S().bookPaid.Contains(ident))
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"Your line says paid already. Don't make me write it twice.\"");
			return;
		}

		if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, 15))
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"Fifteen. The book takes Favor, not promises.\"");
			return;
		}

		S().bookPaid.Insert(ident);
		S().owed.RemoveItem(ident);
		m_GM.Forgive(playerId, RZ_EFaction.BANDITS, 5, "paid the book");
		m_GM.NotifyPlayer(playerId, spot.title, "The boss licks a pencil and writes PAID beside your line. \"Square. For now.\"");
		m_Epi.Deed(playerId, "book");
	}

	//------------------------------------------------------------------------------------------------
	//! Wanted posters change when the life leaves Kill on Sight (settled).
	protected void Wanted()
	{
		RZ_EpilogueSave s = S();
		foreach (int i, string ident : s.wantIdents)
		{
			if (s.wantState[i] != 0)
				continue;

			RZ_PlayerProfile p = m_GM.FindProfileByIdentity(ident);
			if (p && p.GetTier(s.wantFaction[i]) != RZ_ETier.KILL_ON_SIGHT)
				s.wantState[i] = 1;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Square with a faction: Infamy under 15 with it and no unpaid line in the book.
	protected void SquareCheck()
	{
		array<RZ_PlayerProfile> profiles = {};
		m_GM.GetAllProfiles(profiles);
		foreach (RZ_PlayerProfile p : profiles)
		{
			if (p.m_sIdentity.IsEmpty() || S().owed.Contains(p.m_sIdentity) || S().playerIdents.Find(p.m_sIdentity) < 0)
				continue;

			bool square = true;
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (f < p.m_aInfamy.Count() && p.m_aInfamy[f] >= 15)
					square = false;
			}

			if (square && !S().square.Contains(p.m_sIdentity))
				S().square.Insert(p.m_sIdentity);
			else if (!square)
				S().square.RemoveItem(p.m_sIdentity);
		}
	}

	// ================================================================ Take Back a Town
	//------------------------------------------------------------------------------------------------
	//! Infected killed in a town in the week: whoever was close takes part in taking it back.
	void OnTownKills(int settlement, vector pos)
	{
		int i = S().townIdx.Find(settlement);
		if (i < 0 || S().townTaken[i] > 0)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int k, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) > 200)
				continue;

			string name = RZ_EpiNames.Now(ids[k]);
			string by = S().townBy[i];
			if (!by.Contains(name) && by.Length() < 60)
			{
				if (!by.IsEmpty())
					by += ", ";

				S().townBy[i] = by + name;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A town with 6 or more crawlers at the cure and none left now is taken back: a candle, the
	//! town log's best slot, its houses told.
	protected void Towns()
	{
		RZ_EpilogueSave s = S();
		foreach (int i, int idx : s.townIdx)
		{
			if (s.townTaken[i] > 0 || m_Epi.TownCrawlers(idx) > 0)
				continue;

			// A candle only while fewer than 8 towns carry marks
			if (!m_Epi.GetRest().CandleRoom())
				return;

			s.townTaken[i] = Math.Max(1, Day());
			RZ_Settlement st = m_GM.GetSettlement(idx);
			if (!st)
				continue;

			vector at = RZ_Util.GroundPos(st.m_vCenter + Vector(-st.m_fRadius * 0.6, 0, 0));
			m_Epi.GetBeats().AddSince("town." + idx.ToString(), st.m_sName, at, string.Format("A candle in a jar on the doorstep of the first house on the road into %1.", st.m_sName), string.Format("Somebody's lit a candle in %1.", st.m_sName));
			RZ_WorldFacts.Get().Memory().LogTown(idx, true, "taken back", 3, string.Format("%1 was taken back after the cure. A candle on the step.", st.m_sName));
			m_GM.NotifyAll("Word from the houses", string.Format("Nothing moves in %1 any more. Somebody's lit a candle on a doorstep there.", st.m_sName));
			RZ_Log.Info(string.Format("Epilogue: %1 taken back.", st.m_sName));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The ridges: every pack leader that called at the cure has gone quiet.
	protected void Ridges()
	{
		RZ_EpiItem it = Item("ridges");
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!it || it.state >= 2 || !tribes)
			return;

		int calling = 0;
		foreach (int i, int id : S().packIds)
		{
			RZ_Tribe t = tribes.Find(id);
			if (t && t.m_bHasAlpha)
			{
				calling++;
				S().ridgesLast = S().packPlaces[i];
			}
		}

		if (calling > 0)
			return;

		S().ridgesDay = Day();
		m_Epi.AddHallLine(string.Format("The ridges went quiet on Day %1. The last to call was the lot at %2.", S().ridgesDay, S().ridgesLast));
		Closed(it, 2, "");
	}

	// ================================================================ The Long Table's last week
	//------------------------------------------------------------------------------------------------
	//! Never described. Found out (exposed, as built) closes it live; at processing the chimney
	//! stops and they pack; at the next 05:30 (or 6 hours as data) they walk to the queue.
	protected void TableWeek(array<IEntity> players)
	{
		RZ_EpiItem it = Item("table");
		if (!it || it.state >= 2)
			return;

		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts.Flag("table.exposed") != 0 || facts.Flag("table.burnt") != 0)
		{
			S().tableState = 3;
			Closed(it, 2, "");
			return;
		}

		if (S().tableState != 1)
			return;

		float h = RZ_GameTime.GetHour();
		bool dawn = h >= 5.5 && h < 6.5;
		if (dawn || m_fClock - it.grace >= 6 * HOUR * m_Epi.Scale())
		{
			S().tableState = 2;
			Closed(it, 3, "");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Look at the basket: a tell (+15 suspicion, as the guest rule), once per identity.
	protected void Basket(int playerId)
	{
		string key = "basket|" + Ident(playerId);
		m_GM.NotifyPlayer(playerId, "A handcart", "Jars, packed in straw. Each lid has a date on it, in a neat hand.");
		if (Count(key) > 0)
			return;

		Bump(key);
		RZ_MajorHelp.SetWorld(m_GM, "table.suspicion_add", RZ_MajorHelp.World(m_GM, "table.suspicion_add") + 15);
	}

	// ================================================================ The Last Broadcast
	//------------------------------------------------------------------------------------------------
	protected bool BroadcastAllowed(int version)
	{
		if (version == 1)
			return RZ_MajorHelp.World(m_GM, "weeks.public") != 0;

		if (version == 2)
			return RZ_MajorHelp.World(m_GM, "majors.pages_open") >= 2;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One say per player who brought something this week (any closing deed); a Radio Operator's
	//! counts twice.
	protected void Say(int playerId, int version)
	{
		RZ_EpiItem it = Item("broadcast");
		if (!it || it.state >= 2 || !BroadcastAllowed(version))
			return;

		string ident = Ident(playerId);
		if (Count("deeds|" + ident) <= 0 && !RZ_Radio.IsRadioOperator(m_GM, playerId))
		{
			m_GM.NotifyPlayer(playerId, "The radio station", "\"Bring something first. A name, a page, anything. Then you get a say.\"");
			return;
		}

		int i = S().sayIdent.Find(ident);
		if (i >= 0)
		{
			S().sayVersion[i] = version;
		}
		else
		{
			S().sayIdent.Insert(ident);
			S().sayVersion.Insert(version);
		}

		it.touchUnix = RZ_SeasonClock.Now();
		m_GM.NotifyPlayer(playerId, "The radio station", string.Format("Your say goes down on the sheet: %1.\n%2", RZ_EpiText.BroadcastName(version), BroadcastTally()));
		m_Epi.Deed(playerId, "say");
	}

	//------------------------------------------------------------------------------------------------
	protected string BroadcastTally()
	{
		array<int> votes = {0, 0, 0, 0};
		foreach (int i, string ident : S().sayIdent)
		{
			int weight = 1;
			int pid = 0;
			if (m_GM.GetQuestlines())
				pid = m_GM.GetQuestlines().OnlineId(ident);

			if (pid > 0 && RZ_Radio.IsRadioOperator(m_GM, pid))
				weight = 2;

			int v = S().sayVersion[i];
			if (v >= 0 && v < 4)
				votes[v] = votes[v] + weight;
		}

		return string.Format("The sheet on the desk: the names %1, the three weeks %2, the truth %3, plain %4. It goes out at processing.", votes[0], votes[1], votes[2], votes[3]);
	}

	//------------------------------------------------------------------------------------------------
	//! The tally closes before processing: the most says (ties and nobody: the plain line).
	protected void CloseBroadcast()
	{
		RZ_EpiItem it = Item("broadcast");
		array<int> votes = {0, 0, 0, 0};
		foreach (int i, string ident : S().sayIdent)
		{
			int v = S().sayVersion[i];
			if (v >= 0 && v < 4)
				votes[v] = votes[v] + 1;
		}

		int best = 3;
		int most = 0;
		bool tie = false;
		for (int k = 0; k < 4; k++)
		{
			if (votes[k] > most)
			{
				most = votes[k];
				best = k;
				tie = false;
			}
			else if (votes[k] == most && most > 0)
			{
				tie = true;
			}
		}

		if (tie || most == 0)
			best = 3;

		S().broadcast = best;
		switch (best)
		{
			case 0: Lane(RZ_EFaction.SURVIVORS, 2); break;
			case 1: Lane(RZ_EFaction.INSTITUTE, -2); Lane(RZ_EFaction.SURVIVORS, 1); break;
			case 2:
			{
				for (int f = 0; f < RZ_Factions.COUNT; f++)
				{
					Lane(f, -1);
				}

				Lane(RZ_EFaction.BANDITS, -1);
				break;
			}
		}

		m_Epi.AddHallLine(string.Format("The last broadcast: %1.", RZ_EpiText.BroadcastName(best)));
		if (it)
		{
			int state = 3;
			if (most > 0)
				state = 2;

			Closed(it, state, RZ_EpiText.BroadcastName(best));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Morale from the epilogue's lane (+-5 per faction for the week, inside the majors' lane).
	protected void Lane(int faction, float delta)
	{
		RZ_MajorBudget budget = RZ_MajorBudget.Get();
		float allowed = delta;
		if (budget)
			allowed = budget.TakeMorale(RZ_EBudgetLane.EPILOGUE, faction, delta, "", "epilogue");

		if (allowed != 0)
			RZ_Questline.NudgeMorale(m_GM, faction, allowed);
	}

	//------------------------------------------------------------------------------------------------
	//! In processing: the broadcast on every open channel on the hour (x Test scale), and in full on
	//! the last dusk.
	protected void Air(float step)
	{
		if (m_Epi.Phase() < 3 || S().broadcast < 0)
			return;

		m_fAirClock += step;
		string names = "";
		int n = 0;
		foreach (RZ_EpiName c : S().names)
		{
			if (c.state == 2 && n < 8)
			{
				if (!names.IsEmpty())
					names += ", ";

				names += c.name;
				n++;
			}
		}

		string text = RZ_EpiText.BroadcastText(S().broadcast, names);
		if (m_fAirClock >= HOUR * m_Epi.Scale())
		{
			m_fAirClock = 0;
			m_GM.NotifyAll("Radio: the station", text);
		}

		if (!m_bAiredDusk && RZ_WorldFacts.Get().IsDusk())
		{
			m_bAiredDusk = true;
			m_GM.NotifyAll("Radio: the station", text + " Then a long hiss. Nothing.");
		}
	}

	// ================================================================ The grey choices
	//------------------------------------------------------------------------------------------------
	//! Which stronghold question this leader asks (0 the chair, 1 the CO, 2 the boss, 3 the
	//! director), -1 none.
	int ChoiceOf(RZ_Leader l)
	{
		if (!l || !l.IsStronghold())
			return -1;

		switch (l.m_eFaction)
		{
			case RZ_EFaction.SURVIVORS: return 0;
			case RZ_EFaction.MILITARY: return 1;
			case RZ_EFaction.BANDITS: return 2;
			case RZ_EFaction.INSTITUTE: return 3;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The CO asks once the last call has run; the boss only under the boss's price; the director
	//! only with something taken alive in isolation.
	bool ChoiceOpen(int which)
	{
		if (which == 1)
			return RZ_MajorHelp.World(m_GM, "orders.coda") != 0 || !m_GM.GetQuestlines() || !m_GM.GetQuestlines().IsRunning("orders");

		if (which == 2)
			return m_Epi.Route() == RZ_CureFinale.REDHAND && S().terms == 1;

		if (which == 3)
		{
			RZ_Tribes tribes = m_GM.GetTribes();
			if (!tribes)
				return false;

			foreach (string beat : tribes.GetPeople().GetBeats())
			{
				if (beat.StartsWith("Taken alive"))
					return true;
			}

			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! An answer to a grey choice: the decide hold (5 s with one objection per player), then the
	//! choice is the season's.
	void ChoicePick(RZ_Leader l, int playerId, bool first)
	{
		int which = ChoiceOf(l);
		if (which < 0)
			return;

		if (S().choices[which] != 0)
		{
			m_GM.NotifyPlayer(playerId, l.GetHeader(), "\"Already decided.\"");
			return;
		}

		if (!ChoiceOpen(which))
		{
			string none = "\"Nothing to decide. For once.\"";
			if (which == 2)
				none = "\"Already decided. Not by me.\"";

			m_GM.NotifyPlayer(playerId, l.GetHeader(), none);
			return;
		}

		// Dropping the price takes the Red Hand's trust
		if (which == 2 && !first && !RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.TRUSTED))
		{
			m_GM.NotifyPlayer(playerId, l.GetHeader(), "\"You don't get a say in the price. Trusted people do.\"");
			return;
		}

		string ident = Ident(playerId);
		if (m_iPending == which)
		{
			if (m_bPendingFirst == first)
			{
				m_GM.NotifyPlayer(playerId, l.GetHeader(), "\"That's what's on the table already.\"");
				return;
			}

			if (m_aObjected.Contains(ident))
			{
				m_GM.NotifyPlayer(playerId, l.GetHeader(), "\"You've objected once. Let somebody else.\"");
				return;
			}

			m_aObjected.Insert(ident);
			m_iPending = -1;
			TellNear(l.m_vPos, l.GetHeader(), "An objection. Nothing's decided.");
			return;
		}

		m_iPending = which;
		m_bPendingFirst = first;
		m_fPendingAt = m_fClock;
		m_iPendingBy = playerId;
		m_vPendingAt = l.m_vPos;
		m_aObjected.Clear();
		TellNear(l.m_vPos, l.GetHeader(), string.Format("%1 %2\nAnyone who objects has a few seconds.", RZ_EpiText.Question(which), RZ_EpiText.Answer(which, first)));
	}

	//------------------------------------------------------------------------------------------------
	protected void TellNear(vector pos, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= DECIDE_RADIUS)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The hold is over: the choice is made, its effects come out of the epilogue's lane, the
	//! chooser gets 2 Fame with that faction, and everyone hears what was decided.
	protected void Decide()
	{
		if (m_iPending < 0 || m_fClock - m_fPendingAt < HOLD_SECONDS)
			return;

		int which = m_iPending;
		bool first = m_bPendingFirst;
		m_iPending = -1;
		S().choices[which] = 2;
		if (first)
			S().choices[which] = 1;

		if (!first)
		{
			switch (which)
			{
				case 0: Lane(RZ_EFaction.SURVIVORS, -3); break;
				case 1: Lane(RZ_EFaction.MILITARY, -4); break;
				case 2: Lane(RZ_EFaction.BANDITS, -3); Lane(RZ_EFaction.SURVIVORS, 2); break;
				case 3: Lane(RZ_EFaction.INSTITUTE, -2); break;
			}
		}

		int f = RZ_EFaction.SURVIVORS;
		if (which == 1)
			f = RZ_EFaction.MILITARY;
		else if (which == 2)
			f = RZ_EFaction.BANDITS;
		else if (which == 3)
			f = RZ_EFaction.INSTITUTE;

		if (m_iPendingBy > 0)
		{
			m_GM.AddFame(m_iPendingBy, f, 2);
			m_Epi.Deed(m_iPendingBy, "choice");
		}

		m_GM.NotifyAll(RZ_Factions.GetName(f), RZ_EpiText.Decided(which, first));
		RZ_Log.Info(string.Format("Epilogue: grey choice %1 decided (%2).", which, first));
		m_Epi.GetBeats().Refresh();
	}

	// ================================================================ The Last Entry
	//------------------------------------------------------------------------------------------------
	//! Write your line on the Names board: one per identity, rewritable to the wipe. Without text
	//! (a pad), a line built from your name and the day; with text (/lastentry on a keyboard), up to
	//! 80 characters through the note filter.
	void WriteLine(int playerId, string raw)
	{
		if (!m_Epi.ClosingOn())
			return;

		string ident = Ident(playerId);
		string name = RZ_EpiNames.Now(playerId);
		string text = raw.Trim();
		if (text.IsEmpty())
		{
			array<string> lines = {"Still here.", "Gone home.", "Thanks for the soup.", "Look after the dog.", "Tell them we tried.", "Left the light on."};
			int n = Bump("linepick|" + ident);
			text = string.Format("Day %1. %2", Day(), lines[(n - 1) % lines.Count()]);
		}
		else
		{
			text = RZ_EpiNames.Filtered(text);
			if (text.Length() > 80)
				text = text.Substring(0, 80);
		}

		RZ_EpiEntry e;
		foreach (RZ_EpiEntry old : S().entries)
		{
			if (old.ident == ident)
				e = old;
		}

		if (!e)
		{
			e = new RZ_EpiEntry();
			e.ident = ident;
			S().entries.Insert(e);
			m_Epi.Deed(playerId, "line");
		}

		e.name = name;
		e.text = text;
		e.unix = RZ_SeasonClock.Now();
		m_GM.NotifyPlayer(playerId, "The Names board", string.Format("You write it on the board, in the column for the living: \"%1\"", text));
		RZ_Log.Info(string.Format("Epilogue: a last entry by %1: %2", name, text));
	}

	//------------------------------------------------------------------------------------------------
	//! /epilogue lastnotes <name>: an admin archives a line for the season block (5 at most, with
	//! Epilogue > Archive last entries on).
	string ArchiveEntry(string who)
	{
		if (!m_Epi.GetSettings().m_bArchiveLastEntries)
			return "Epilogue > Archive last entries is off.";

		if (Count("archived") >= 5)
			return "Five are archived already.";

		foreach (RZ_EpiEntry e : S().entries)
		{
			if (e.name != who)
				continue;

			Bump("archived");
			m_Epi.AddHallLine(string.Format("On the Names board: \"%1\" (%2)", e.text, RZ_EpiNames.Show(e.ident, e.name)));
			return "Archived.";
		}

		return "No line by " + who;
	}

	//------------------------------------------------------------------------------------------------
	//! An admin strike: a last entry or a wall name by that name is gone, not starred out.
	void Strike(string what)
	{
		for (int i = S().entries.Count() - 1; i >= 0; i--)
		{
			if (S().entries[i].name == what)
				S().entries.Remove(i);
		}
	}

	// ================================================================ Personal Goals and deeds
	//------------------------------------------------------------------------------------------------
	//! A closing deed: counted for Something to do, and each background's last goal checked.
	void Deed(int playerId, string kind)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		string ident = p.m_sIdentity;
		Bump("deeds|" + ident);
		Bump(string.Format("deed|%1|%2", ident, kind));
		string bg = p.m_sBackgroundId;
		if (RZ_EpiText.LastGoal(bg).IsEmpty() || S().goals.Contains(ident + "|" + bg) || p.m_bObjDone)
			return;

		bool done = false;
		switch (bg)
		{
			case "nurse": done = (kind == "cup" && Count(string.Format("deed|%1|cup", ident)) >= 2) || kind == "witness" || kind == "sit"; break;
			case "mechanic": done = kind == "car"; break;
			case "hunter": done = kind == "home" || kind == "cup"; break;
			case "police": done = kind == "home"; break;
			case "excon": done = kind == "stash"; break;
			case "labtech": done = kind == "record" || kind == "say"; break;
			case "radioham": done = kind == "say"; break;
			case "unemployed": done = Count("deeds|" + ident) >= 2; break;
			case "soldier": done = kind == "ridge" || kind == "home"; break;
			case "doctor": done = (kind == "witness" && Count(string.Format("deed|%1|witness", ident)) >= 2) || kind == "sit"; break;
			case "raider": done = kind == "book" || kind == "buyback"; break;
			case "elder": done = kind == "read"; break;
			case "drifter": done = kind == "down" || kind == "kitchen"; break;
		}

		if (!done)
			return;

		S().goals.Insert(ident + "|" + bg);
		p.m_bObjDone = true;
		p.m_iLegacy++;
		m_GM.AddFame(playerId, RZ_EFaction.SURVIVORS, 10);
		m_GM.OnLegacyChanged(playerId);
		m_GM.NotifyPlayer(playerId, "Personal goal complete", string.Format("%1\nFinished what you came for. Legacy: %2", RZ_EpiText.LastGoal(bg), p.m_iLegacy));
	}

	//------------------------------------------------------------------------------------------------
	//! A dose at an outlet: anyone else within 25 m was there for it (the Nurse's last round).
	void OnDose(int playerId, vector at)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && RZ_Util.DistXZ(p.GetOrigin(), at) <= 25)
				Deed(ids[i], "witness");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The first spawn of a life in the week: the background's last goal, told.
	void OnSpawn(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		string goal = RZ_EpiText.LastGoal(p.m_sBackgroundId);
		if (goal.IsEmpty() || !m_Epi.ClosingOn() || p.m_iObjType != RZ_EObjective.LAST_GOAL)
			return;

		m_GM.NotifyPlayer(playerId, "One last thing", goal);
	}

	//------------------------------------------------------------------------------------------------
	//! Does a background still have its last goal to finish this week (RZ_Objectives.Assign)?
	bool WantsLastGoal(RZ_PlayerProfile p, string bg)
	{
		if (!m_Epi.ClosingOn() || RZ_EpiText.LastGoal(bg).IsEmpty())
			return false;

		return !S().goals.Contains(p.m_sIdentity + "|" + bg);
	}

	//------------------------------------------------------------------------------------------------
	//! The Ex-Con's stash verb shows while anyone online has that last goal open.
	bool HasStash()
	{
		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			RZ_PlayerProfile p = m_GM.FindProfile(id);
			if (p && p.m_sBackgroundId == "excon" && p.m_iObjType == RZ_EObjective.LAST_GOAL && !p.m_bObjDone)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The car verb shows while a vehicle stands at the queue.
	bool CanLeaveCar()
	{
		vector q = m_Epi.GetSpots().QueuePos();
		return q != vector.Zero && RZ_EpiCarQuery.Near(q, 25);
	}

	// ================================================================ Processing and the wipe
	//------------------------------------------------------------------------------------------------
	//! Processing begins: every item still open closes as data; the squares are checked; the farm
	//! packs.
	void OnProcessing()
	{
		foreach (RZ_EpiItem it : S().items)
		{
			if (it.state >= 2 || it.key == "ridges")
				continue;

			if (it.key == "table")
			{
				S().tableState = 1;
				it.grace = m_fClock;
				continue;
			}

			CloseDefault(it);
		}

		SquareCheck();
		m_Epi.GetSpots().Dirty();
	}

	//------------------------------------------------------------------------------------------------
	//! The wipe came due: the names put down are read at the barrier, nobody carried is NOT BROUGHT
	//! HOME, the squares again, and the week's lines for the season block.
	void OnWipe()
	{
		RZ_EpilogueSave s = S();
		foreach (string d : s.walkDown)
		{
			array<string> parts = {};
			d.Split("|", parts, false);
			if (parts.Count() < 2)
				continue;

			s.walked.Insert(string.Format("%1|%2|%3", parts[0], parts[1], Day()));
			m_GM.NotifyAll("The north road", string.Format("\"Name?\"\n\"%1.\" Somebody in the queue answers for them.", parts[1]));
		}

		s.walkDown.Clear();
		foreach (RZ_EpiName n : s.names)
		{
			if (n.state == 0 || n.state == 1)
				n.state = 3;
		}

		SquareCheck();
		Ridges();
		HallLines();
	}

	//------------------------------------------------------------------------------------------------
	protected void HallLines()
	{
		RZ_EpilogueSave s = S();
		int home = WallCount();
		if (home > 0)
		{
			string first = "";
			foreach (RZ_EpiName n : s.names)
			{
				if (n.state == 2)
				{
					first = n.name;
					break;
				}
			}

			m_Epi.AddHallLine(string.Format("Brought home: %1 names. The first on the wall: %2.", home, first));
		}

		int taken = 0;
		string towns = "";
		foreach (int i, int idx : s.townIdx)
		{
			if (s.townTaken[i] <= 0)
				continue;

			taken++;
			RZ_Settlement st = m_GM.GetSettlement(idx);
			if (st && taken <= 3)
			{
				if (!towns.IsEmpty())
					towns += ", ";

				towns += st.m_sName;
			}
		}

		if (taken > 0)
			m_Epi.AddHallLine(string.Format("Towns taken back after the cure: %1 (%2).", taken, towns));

		if (!s.walked.IsEmpty())
		{
			string walked = "";
			int shown = 0;
			foreach (string w : s.walked)
			{
				array<string> parts = {};
				w.Split("|", parts, false);
				if (parts.Count() < 3 || shown >= 12)
					continue;

				if (!walked.IsEmpty())
					walked += ", ";

				walked += string.Format("%1, Day %2", m_Epi.GetRest().Named(parts[0], RZ_EpiNames.Show(parts[0], parts[1])), parts[2]);
				shown++;
			}

			if (s.walked.Count() > shown)
				walked += string.Format(", and %1 more", s.walked.Count() - shown);

			m_Epi.AddHallLine("Walked out: " + walked + ".");
		}

		if (!s.square.IsEmpty())
		{
			string names = "";
			int n2 = 0;
			foreach (string ident : s.square)
			{
				int pi = s.playerIdents.Find(ident);
				if (pi < 0 || n2 >= 8)
					continue;

				if (!names.IsEmpty())
					names += ", ";

				names += m_Epi.GetRest().Named(ident, RZ_EpiNames.Show(ident, s.playerNames[pi]));
				n2++;
			}

			if (!names.IsEmpty())
				m_Epi.AddHallLine("Left the Zone square with all four: " + names + ".");
		}

		if (!s.goals.IsEmpty())
			m_Epi.AddHallLine(string.Format("%1 finished what they came for.", s.goals.Count()));

		m_Epi.GetRest().HallLines();
	}

	// ================================================================ The journal
	//------------------------------------------------------------------------------------------------
	//! "Unfinished": the closing work open to this player, with its last call.
	string UnfinishedText(int playerId)
	{
		string text = "";
		foreach (RZ_EpiItem it : S().items)
		{
			if (it.state >= 2 || it.title.IsEmpty())
				continue;

			string where = "";
			if (it.x != 0 || it.z != 0)
				where = " (" + m_Epi.PlaceWords(Vector(it.x, 0, it.z)) + ")";

			text += it.title + where;
			if (it.state == 1)
				text += ": last call given";

			text += "\n";
		}

		string ident = Ident(playerId);
		foreach (RZ_EpiName n : S().names)
		{
			if (n.state == 1 && n.carrier == ident)
				text += string.Format("Carrying %1 of %2. %3\n", n.what, n.name, HomeWords(n));
		}

		int lying = 0;
		foreach (RZ_EpiName m : S().names)
		{
			if (m.state == 0)
				lying++;
		}

		if (lying > 0)
			text += string.Format("%1 names still to bring home.\n", lying);

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p.m_iObjType == RZ_EObjective.LAST_GOAL && !p.m_bObjDone)
			text += RZ_EpiText.LastGoal(p.m_sBackgroundId) + "\n";

		return text;
	}
}
