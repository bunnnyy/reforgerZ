//! The rest of the epilogue week (docs/EPILOGUE.md): what the first pass left out, in one place.
//! - The newcomer's scenario, The empty kitchen: a new face's first spawn in the week gets Petr's
//!   note (the queue, the key), which deals The Key Where It Was (rest.kitchen).
//! - Holdouts as people: Carry the Dose's holdout stands at their door while the box is on its way.
//! - Standing versions of the codas and the supper reading, until the wipe: the List and its clerk
//!   at the chair's house, the roster board and its operator, the tags on the nail AT THE BARRIER,
//!   the case on the lab's table, the lorry with the book and the lad who reads you your line; the
//!   supper reading as a scene at the chair's table 18:00-20:00 (and a few Last Entries read there).
//! - Scenes that move: people join the queue from the houses, the queue shuffles up at each change
//!   of shift in processing, the lab gate's group stands by route, a scene slides 30 m along the
//!   verge when a car stands in it; the Long Table's family in the queue, and the boy talked out.
//! - Places and people: residents' lines by route (and the losing routes' with dignity), the cook's,
//!   the sentry's and the toll man's own lines; a wanderer who remembers you says your line first;
//!   the Preacher reads a town's dead at dusk and rings his bells.
//! - Leaders' topics one by one (the cure, this place, me, the dead) coloured by traits, and "What
//!   you didn't tell me" from the truth deck.
//! - The closing work's rest: the routes' closing lines on the codas, keepsakes from the named
//!   young and the lost trucks, the fence buying back, the factions' jobs on the ridges, Take Back a
//!   Town's kitchen and its candle cap, the Long Table at the queue, the Walk Out's order by the
//!   terms and its fallback without a checkpoint, the last five backgrounds' goals.
//! - The calendar: how many evenings phase B holds at the commit.
//! Server only. Everything reads the record and the world; nothing here spawns outside the spots.

//------------------------------------------------------------------------------------------------
class RZ_EpiRestSave
{
	ref array<string> kitchen = {};			// identities who read the kitchen note
	ref array<string> holdouts = {};		// "errand id|name|x|z"
	ref array<int> holdoutUnix = {};		// the last time each was asked for (the errand still stands)
	int evenings = -1;						// evenings phase B holds, worked out at the commit
	int joined;								// people who joined the queue from the houses
	int joinUnix;
	int shuffles;							// changes of shift the queue has moved up in processing
	int shiftKey = -1;						// day * 3 + shift of the last move
	int preacherDay = -1;
	int answerDay = -1;						// the Haven channel's answer to the broadcast
	ref array<string> codaLines = {};		// codas whose route line was aired
	ref array<string> topics = {};			// "ident|leader|topic": each topic once a real day
	ref array<int> topicUnix = {};
	ref array<string> truthsTold = {};		// identities told a truth ("What you didn't tell me")
	ref array<string> ridgeJobs = {};		// "ident|tribe|faction"
	ref array<string> ridgeNames = {};		// up to three for the hall line
	ref array<string> kitchens = {};		// "ident|house": Take Back a Town's kitchen once more
	ref array<string> takeNames = {};		// up to three for the hall line
	ref array<string> boyOut = {};			// who talked the boy out of the queue
	int tableTold;							// 0 nobody, 1 the corporal, 2 the chair
	ref array<string> bought = {};			// keepsakes bought back from the fence ("ident|name")
	bool keepsakes;							// the extra keepsakes were added
	int stallUnix;							// the Unsigned last day's long-stall line
	ref array<string> settled = {};			// "ident|faction": Settle Accounts at a leader, once a faction
	ref array<string> epithets = {};		// "ident|at the barrier": the codas' epithets on the season block
}

//------------------------------------------------------------------------------------------------
class RZ_EpiRest
{
	protected static const int HOUR = 3600;
	protected static const int DAY = 86400;
	protected static const int MAX_CANDLE_TOWNS = 8;
	protected static const float SLIDE = 30;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Epilogue m_Epi;

	//------------------------------------------------------------------------------------------------
	void RZ_EpiRest(RZ_GameModeComponent gm, RZ_Epilogue epi)
	{
		m_GM = gm;
		m_Epi = epi;
	}

	//------------------------------------------------------------------------------------------------
	RZ_EpiRestSave R()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (!s.rest)
			s.rest = new RZ_EpiRestSave();

		return s.rest;
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	void Tick(float step, array<int> ids, array<IEntity> players)
	{
		if (!m_Epi.IsBuilt() || m_Epi.Data().lastDay)
			return;

		if (R().evenings < 0)
			R().evenings = Evenings();

		Keepsakes();
		QueueJoins();
		QueueShift(players);
		Preacher(players);
		CodaLines();
		BroadcastAnswer();
	}

	//------------------------------------------------------------------------------------------------
	//! How many evenings phase B holds: its played hours (RoomHours) over the real length of a game
	//! day at the time acceleration of the commit. Worked out once at the commit.
	protected int Evenings()
	{
		float playedHours = m_Epi.RoomHours();
		float dayHours = Math.Max(0.1, RZ_SeasonClock.DayMinutes() / 60);
		int evenings = Math.Max(1, RZ_Util.RoundToInt(Math.Floor(playedHours / dayHours)));
		RZ_Log.Info(string.Format("Epilogue: phase B holds about %1 evenings.", evenings));
		return evenings;
	}

	//------------------------------------------------------------------------------------------------
	//! Fewer evenings than codas: the second airings may share an evening.
	bool TightEvenings()
	{
		return R().evenings >= 0 && R().evenings < 4;
	}

	// ================================================================ The newcomer
	//------------------------------------------------------------------------------------------------
	//! A new face's first spawn in the week: the empty kitchen. Petr's note on the table points to
	//! the queue, and the key is where it was (The Key Where It Was). The first leader's lines are
	//! the season topic, as for anyone.
	void OnSpawn(int playerId, IEntity ent)
	{
		if (!ent || !m_Epi.IsBuilt() || m_Epi.Data().lastDay || m_Epi.Phase() >= 3)
			return;

		string ident = Ident(playerId);
		if (ident.IsEmpty() || m_Epi.Data().playerIdents.Contains(ident) || R().kitchen.Contains(ident))
			return;

		R().kitchen.Insert(ident);
		m_GM.NotifyPlayer(playerId, "The empty kitchen", "A note on the kitchen table, weighed down with a mug: \"Gone to the queue at the north road. If you're new, go there too. Petr, the key's where it was. Bring it, would you?\"");
	}

	//------------------------------------------------------------------------------------------------
	bool ReadKitchen(string ident)
	{
		return R().kitchen.Contains(ident);
	}

	// ================================================================ Holdouts
	//------------------------------------------------------------------------------------------------
	//! Carry the Dose's holdout, asked for while the box is on its way (the card's Still).
	void Holdout(int errandId, string name, vector at)
	{
		string head = errandId.ToString() + "|";
		foreach (int i, string h : R().holdouts)
		{
			if (h.StartsWith(head))
			{
				R().holdoutUnix[i] = m_Epi.Now();
				return;
			}
		}

		R().holdouts.Insert(string.Format("%1|%2|%3|%4", errandId, name, at[0], at[2]));
		R().holdoutUnix.Insert(m_Epi.Now());
		m_Epi.GetSpots().Dirty();
	}

	//------------------------------------------------------------------------------------------------
	void HoldoutDone(int errandId)
	{
		string head = errandId.ToString() + "|";
		for (int i = R().holdouts.Count() - 1; i >= 0; i--)
		{
			if (R().holdouts[i].StartsWith(head))
			{
				R().holdouts.Remove(i);
				R().holdoutUnix.Remove(i);
			}
		}

		m_Epi.GetSpots().Dirty();
	}

	// ================================================================ Spots
	//------------------------------------------------------------------------------------------------
	//! The places this file adds to the week's spots, and its changes to the built ones.
	void Spots(notnull array<ref RZ_EpiSpot> result)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (s.lastDay || m_Epi.Phase() <= 0)
			return;

		HoldoutSpots(result);
		StandingSpots(result);
		SupperSpot(result);
		RidgeSpots(result);
		KitchenSpots(result);
		BarrierFallback(result);
		int lasting = 0;
		int evening = 0;
		foreach (RZ_EpiSpot spot : result)
		{
			if (spot.key == "queue")
				QueueScene(spot);
			else if (spot.key == "labgate")
				GateScene(spot);
			else if (spot.key == "crossroads")
				spot.Verb("Buy a keepsake back (10 Favor)", "rest:buyback");
			else if (spot.key.StartsWith("house."))
				HouseExtras(spot);
			else if (spot.key == "table")
				CrateNames(spot);

			if (!spot.scene)
				continue;

			Clear(spot);
			// Scenes with bodies: Lasting (the queue, the lab gate, the barrier) and Evening (supper)
			if (spot.key == "stand.supper")
			{
				evening++;
				if (evening > m_Epi.GetSettings().m_iScenesEvening)
					spot.person = false;
			}
			else
			{
				lasting++;
				if (lasting > m_Epi.GetSettings().m_iScenesLasting)
					spot.person = false;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The open crate: the names of whoever reached it first, chalked on the lid.
	protected void CrateNames(RZ_EpiSpot table)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (s.crate.IsEmpty())
			return;

		array<string> names = {};
		foreach (string ident : s.crate)
		{
			int i = s.playerIdents.Find(ident);
			string shown = "";
			if (i >= 0)
				shown = RZ_EpiNames.Show(ident, s.playerNames[i]);

			if (!shown.IsEmpty() && names.Count() < 10)
				names.Insert(shown);
		}

		if (!names.IsEmpty())
			table.talk = table.talk + "\nChalked on the lid, first come: " + Joined(names) + ".";
	}

	//------------------------------------------------------------------------------------------------
	//! The prop for a beat's mark by its kind (each slot optional), else `fallback`.
	ResourceName PrefabFor(RZ_EpiBeat b, ResourceName fallback)
	{
		RZ_EpilogueSettings cfg = m_Epi.GetSettings();
		ResourceName pick = "";
		if (b.key.StartsWith("grave.") || b.key.StartsWith("leaders."))
			pick = cfg.m_sGravePrefab;
		else if (b.key.StartsWith("wanted."))
			pick = cfg.m_sPosterPrefab;
		else if (b.key.StartsWith("wreck."))
			pick = cfg.m_sWreckPrefab;
		else if (b.key == "names")
			pick = cfg.m_sNamesBoardPrefab;
		else if (b.key == "crossroads")
			pick = cfg.m_sPriceBoardPrefab;
		else if (b.key == "roster")
			pick = cfg.m_sStencilPrefab;
		else if (b.key == "station" || b.key.StartsWith("post.") || b.key.StartsWith("faction."))
			pick = cfg.m_sNoticePrefab;
		else if (b.key == "queue")
			pick = cfg.m_sTrestlePrefab;
		else if (b.key == "table")
			pick = cfg.m_sHandcartPrefab;
		else if (b.key.StartsWith("since.") && b.key.Contains("home"))
			pick = cfg.m_sRibbonPrefab;
		else if (b.key.StartsWith("house.") && b.more.Contains("Bunting"))
			pick = cfg.m_sBuntingPrefab;
		else if (b.key.StartsWith("town."))
		{
			if (b.more.Contains("sheet"))
				pick = cfg.m_sSheetPrefab;
			else if (b.more.Contains("stencil"))
				pick = cfg.m_sStencilPrefab;
			else if (b.more.Contains("Tape"))
				pick = cfg.m_sTapePrefab;
		}
		else if (b.key.StartsWith("story.") && b.texts.Count() > 0 && b.texts[0].Contains("crate"))
		{
			pick = cfg.m_sCratePrefab;
		}

		// One kit with Traces: its own piece when this slot is empty
		if (pick.IsEmpty())
			pick = RZ_Traces.EpilogueKit(b.key, b.more);

		if (pick.IsEmpty())
			return fallback;

		return pick;
	}

	//------------------------------------------------------------------------------------------------
	//! A notable death in the week (2 hours or more): a new grave by the road, taking the place of
	//! the least notable one when the graves are full.
	void OnWeekDeath(string ident, string name, string role, vector at, float lifeSeconds)
	{
		if (!m_Epi.IsBuilt() || m_Epi.Data().lastDay || ident.IsEmpty())
			return;

		RZ_EpiBeat weakest;
		int graves = 0;
		foreach (RZ_EpiBeat b : m_Epi.Data().beats)
		{
			if (!b.key.StartsWith("grave.") || b.dropped)
				continue;

			graves++;
			if (!weakest || b.weight < weakest.weight)
				weakest = b;
		}

		if (graves >= 6 && weakest)
			weakest.dropped = true;

		string upper = role;
		upper.ToUpper();
		m_Epi.GetBeats().AddSince("grave.week." + ident, m_Epi.PlaceWords(at), RZ_Mission.LandPointNear(at, 5, 15), string.Format("Fresh earth, two fence palings and a name in biro. %1. THIS WEEK.", upper), "There's a fresh grave by the road.", ident, name);
	}

	//------------------------------------------------------------------------------------------------
	//! The Unsigned last day: the broadcast's long-stall line every two real hours in place of the
	//! wipe's warnings ("" when it isn't due).
	string LongStall()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (!s.lastDay)
			return "";

		int now = m_Epi.Now();
		if (now - R().stallUnix < RZ_Util.RoundToInt(2 * HOUR * m_Epi.Scale()))
			return "";

		R().stallUnix = now;
		return string.Format("Day %1. The perimeter will be maintained. This message will repeat.", RZ_WorldFacts.Get().Day());
	}

	//------------------------------------------------------------------------------------------------
	protected void HoldoutSpots(notnull array<ref RZ_EpiSpot> result)
	{
		int now = m_Epi.Now();
		for (int i = R().holdouts.Count() - 1; i >= 0; i--)
		{
			if (now - R().holdoutUnix[i] > 120)
			{
				R().holdouts.Remove(i);
				R().holdoutUnix.Remove(i);
				continue;
			}

			array<string> parts = {};
			R().holdouts[i].Split("|", parts, false);
			if (parts.Count() < 4)
				continue;

			RZ_EpiSpot h = new RZ_EpiSpot();
			h.key = "holdout." + parts[0];
			h.person = true;
			h.pos = RZ_Mission.LandPointNear(Vector(parts[2].ToFloat(), 0, parts[3].ToFloat()), 2, 4);
			h.title = parts[1];
			h.talk = "In the doorway, in a coat over pyjamas. \"I'm not walking to any queue. If they want me cured, they can come here.\"";
			h.faction = RZ_EFaction.SURVIVORS;
			result.Insert(h);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The codas' standing versions, each from its coda's evening until the wipe.
	protected void StandingSpots(notnull array<ref RZ_EpiSpot> result)
	{
		if (RZ_MajorHelp.World(m_GM, "every_name.coda") != 0)
		{
			int chair = RZ_HavenQuest.ChairHouse(m_GM);
			if (chair >= 0)
			{
				vector house = m_GM.GetSafehouses().Get(chair).m_vPos;
				RZ_EpiSpot list = new RZ_EpiSpot();
				list.key = "stand.list";
				list.person = true;
				list.pos = RZ_Mission.LandPointNear(house, 6, 10);
				list.title = "The List, pinned up";
				list.talk = "The List on the house board, every page, the crossings and all. A clerk sits under it with a stool for whoever wants it read.";
				list.faction = RZ_EFaction.SURVIVORS;
				list.Verb("Ask the clerk to read it", "rest:list");
				result.Insert(list);
			}
		}

		RZ_Leader co = RZ_BastionQuest.Stronghold(m_GM, RZ_EFaction.MILITARY);
		if (co && RZ_MajorHelp.World(m_GM, "orders.coda") != 0)
		{
			RZ_EpiSpot roster = new RZ_EpiSpot();
			roster.key = "stand.roster";
			roster.person = true;
			roster.pos = RZ_Mission.LandPointNear(co.m_vPos, 8, 12);
			roster.title = "The roster board";
			roster.talk = "The last call's transcript on the roster board. The tags are back on the nail, and chalked over them: AT THE BARRIER.";
			roster.faction = RZ_EFaction.MILITARY;
			roster.Verb("Ask the operator to play it back", "rest:call");
			result.Insert(roster);
		}

		if (m_GM.HasLab() && RZ_MajorHelp.World(m_GM, "proof.coda") != 0)
		{
			RZ_EpiSpot paper = new RZ_EpiSpot();
			paper.key = "stand.case";
			paper.pos = RZ_Mission.LandPointNear(m_Epi.GetSpots().LabGatePos(), 4, 6);
			paper.title = "The case on the table";
			paper.talk = "The transport case on a trestle by the lab gate, latched. A notice under it says what's inside.";
			paper.Verb("Read the notice", "rest:case");
			result.Insert(paper);
		}

		vector cross = m_Epi.GetSpots().CrossroadsPos();
		if (cross != vector.Zero && RZ_MajorHelp.World(m_GM, "book.coda") != 0)
		{
			RZ_EpiSpot lorry = new RZ_EpiSpot();
			lorry.key = "stand.lorry";
			lorry.person = true;
			lorry.pos = RZ_Mission.LandPointNear(cross, 10, 14);
			lorry.title = "The lorry at the crossroads";
			lorry.talk = "A lorry across the road with the book nailed open on its tailgate. A lad on a kitchen chair keeps the pages from blowing.";
			lorry.faction = RZ_EFaction.BANDITS;
			lorry.Verb("Ask the lad to read your line", "rest:lorry");
			result.Insert(lorry);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The supper reading as a scene: the chair at the table outside the house, 18:00-20:00.
	protected void SupperSpot(notnull array<ref RZ_EpiSpot> result)
	{
		if (!RZ_GameTime.IsCalibrated() || RZ_MajorHelp.World(m_GM, "every_name.coda") == 0)
			return;

		float hour = RZ_GameTime.GetHour();
		if (hour < 18 || hour >= 20)
			return;

		int chair = RZ_HavenQuest.ChairHouse(m_GM);
		if (chair < 0)
			return;

		RZ_EpiSpot supper = new RZ_EpiSpot();
		supper.key = "stand.supper";
		supper.person = true;
		supper.scene = true;
		supper.people = 2;
		supper.pos = RZ_Mission.LandPointNear(m_GM.GetSafehouses().Get(chair).m_vPos, 10, 14);
		supper.title = "Supper at the long table";
		supper.talk = "Bowls down the table, a lamp, and the chair standing at the end with the names.";
		supper.faction = RZ_EFaction.SURVIVORS;
		supper.Verb("Listen to the names", "rest:supper");
		result.Insert(supper);
	}

	//------------------------------------------------------------------------------------------------
	//! Quiet the Ridges' factions' jobs, at the lairs still calling (four at most).
	protected void RidgeSpots(notnull array<ref RZ_EpiSpot> result)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes || !m_Epi.ClosingOn())
			return;

		int n = 0;
		foreach (int i, int id : m_Epi.Data().packIds)
		{
			RZ_Tribe t = tribes.Find(id);
			if (!t || !t.m_bHasAlpha || n >= 4)
				continue;

			n++;
			RZ_EpiSpot ridge = new RZ_EpiSpot();
			ridge.key = "ridge." + id.ToString();
			ridge.pos = RZ_Mission.LandPointNear(t.m_vLair, 250, 350);
			ridge.title = "Below " + t.m_sPlace;
			ridge.talk = string.Format("A cairn of stones someone started this week, below %1. Still calling up there, at dusk.", t.m_sPlace);
			ridge.Verb("Cull them, for Bastion", "rest:ridge:1:" + id.ToString());
			ridge.Verb("Take the last reading, for Halcyon", "rest:ridge:3:" + id.ToString());
			ridge.Verb("Bet on which calls last (15 Favor)", "rest:ridge:2:" + id.ToString());
			ridge.Verb("Sit and listen to the last call", "rest:ridge:0:" + id.ToString());
			result.Insert(ridge);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Take Back a Town's giver: a resident who'd like to see the kitchen once more, at a house whose
	//! town was taken back (or a town under 8 marks with no crawlers left).
	protected void KitchenSpots(notnull array<ref RZ_EpiSpot> result)
	{
		if (!m_Epi.ClosingOn())
			return;

		RZ_EpilogueSave s = m_Epi.Data();
		foreach (int i, int idx : s.townIdx)
		{
			if (s.townTaken[i] <= 0 || i >= 6)
				continue;

			RZ_Settlement st = m_GM.GetSettlement(idx);
			if (!st)
				continue;

			RZ_EpiSpot k = new RZ_EpiSpot();
			k.key = "kitchen." + idx.ToString();
			k.person = true;
			k.pos = RZ_Mission.LandPointNear(st.m_vCenter, 20, 40);
			k.title = "A resident from " + st.m_sName;
			k.talk = string.Format("\"I'd like to see the kitchen once more. In %1. I won't stay. I just want to see it.\"", st.m_sName);
			k.faction = RZ_EFaction.SURVIVORS;
			k.Verb("Walk them to their kitchen", "rest:kitchen:" + idx.ToString());
			result.Insert(k);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! No checkpoint on the north road: the Walk Out's barrier at the land edge nearest the chair's
	//! house; on an island map (water every way), no Walk Out.
	protected void BarrierFallback(notnull array<ref RZ_EpiSpot> result)
	{
		if (m_Epi.Phase() < 3 || !m_Epi.GetSettings().m_bWalkOut || m_Epi.GetSpots().BarrierPos() != vector.Zero)
			return;

		vector edge = LandEdge();
		if (edge == vector.Zero)
			return;

		RZ_EpiSpot barrier = new RZ_EpiSpot();
		barrier.key = "barrier.edge";
		barrier.person = true;
		barrier.scene = true;
		barrier.pos = edge;
		barrier.title = "The wire at the edge";
		barrier.faction = RZ_EFaction.SURVIVORS;
		barrier.talk = "The wire where the land runs out, and a speaker on a post. \"Name?\"";
		barrier.Verb("Give your name", "walk");
		result.Insert(barrier);
	}

	//------------------------------------------------------------------------------------------------
	//! The land edge nearest the chair's house: eight bearings out in 250 m steps to 12 km; a bearing
	//! that meets water first doesn't count. Zero on an island map or without the chair's house.
	protected vector LandEdge()
	{
		int chair = RZ_HavenQuest.ChairHouse(m_GM);
		if (chair < 0)
			return vector.Zero;

		vector from = m_GM.GetSafehouses().Get(chair).m_vPos;
		vector best = vector.Zero;
		float bestDist = float.MAX;
		for (int b = 0; b < 8; b++)
		{
			float ang = b * Math.PI / 4;
			vector dir = Vector(Math.Cos(ang), 0, Math.Sin(ang));
			vector last = vector.Zero;
			bool wet = false;
			for (int stepNo = 1; stepNo <= 48; stepNo++)
			{
				vector at = RZ_Util.GroundPos(from + dir * (stepNo * 250));
				// VERIFY: past the terrain's edge the ground height reads as 0 (as over the sea)
				if (RZ_Util.IsWater(at))
				{
					wet = true;
					break;
				}

				last = at;
			}

			if (wet || last == vector.Zero)
				continue;

			float d = RZ_Util.DistXZ(last, from);
			if (d < bestDist)
			{
				bestDist = d;
				best = last;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The queue as a moving scene: the joiners, the shuffle, the Long Table's family, each person's
	//! own line.
	protected void QueueScene(RZ_EpiSpot queue)
	{
		int cap = m_Epi.GetSettings().m_iScenePeople;
		queue.people = Math.Min(cap, Math.Max(1, queue.people + R().joined - R().shuffles));
		array<string> said = {
			"\"Second time I've queued on this road. First time, they turned us round.\"",
			"\"I brought the dog. Nobody said no dogs.\"",
			"\"My sister's at the house. She says she'll come when it's shorter. It's never shorter.\"",
			"\"I've got my slip. Day 2. Bus 1. I kept it in my shoe.\""
		};
		int pick = Math.AbsInt(m_Epi.Now() / 600) % said.Count();
		queue.talk = queue.talk + "\n" + said[pick];
		if (m_Epi.Data().tableState == 2)
		{
			queue.people = Math.Min(cap, queue.people + 3);
			queue.talk = queue.talk + "\nA family near the back with a covered basket. The mother says grace over a bowl she doesn't share.";
			if (R().boyOut.IsEmpty())
				queue.Verb("Talk the boy out of the queue", "rest:boy");

			if (R().tableTold == 0)
			{
				queue.Verb("Tell the corporal about the family", "rest:tell:1");
				queue.Verb("Tell the chair about the family", "rest:tell:2");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The lab gate's group, by route.
	protected void GateScene(RZ_EpiSpot gate)
	{
		switch (m_Epi.Route())
		{
			case RZ_CureFinale.HAVEN:
			{
				gate.people = 2;
				gate.talk = gate.talk + "\nTwo Haven medics with a cold box between them, signing for trays.";
				break;
			}
			case RZ_CureFinale.BASTION:
			{
				gate.people = 3;
				gate.talk = gate.talk + "\nA Bastion group by stencilled crates, a clipboard going round.";
				break;
			}
			case RZ_CureFinale.REDHAND:
			{
				gate.people = 2;
				gate.talk = gate.talk + "\nTwo of the crew leaning on the gatepost, counting.";
				break;
			}
			case RZ_CureFinale.HALCYON:
			{
				gate.people = 1;
				gate.talk = gate.talk + "\nA researcher behind a folding table, writing down everyone who asks.";
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HouseExtras(RZ_EpiSpot house)
	{
		house.Verb("Sit with someone who's bitten", "rest:sit");
		int candles = CandleTowns();
		if (candles > 0)
			house.talk = house.talk + string.Format("\nOn the board: CANDLES %1.", candles);
	}

	//------------------------------------------------------------------------------------------------
	//! Scene clearance: a car within 8 m of a scene slides it 30 m along the verge.
	// VERIFY: RZ_EpiCarQuery's entity query finds parked vehicles
	protected void Clear(RZ_EpiSpot spot)
	{
		if (!RZ_EpiCarQuery.Near(spot.pos, 8))
			return;

		spot.pos = RZ_Mission.LandPointNear(spot.pos, SLIDE, SLIDE + 5);
	}

	//------------------------------------------------------------------------------------------------
	//! Towns carrying candles now.
	int CandleTowns()
	{
		int n = 0;
		foreach (int t : m_Epi.Data().townTaken)
		{
			if (t > 0)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A town's candle only while fewer than 8 towns carry marks.
	bool CandleRoom()
	{
		return CandleTowns() < MAX_CANDLE_TOWNS;
	}

	// ================================================================ The verbs
	//------------------------------------------------------------------------------------------------
	void Verb(int playerId, string verb, RZ_EpiSpot spot, IEntity user)
	{
		if (!verb.StartsWith("rest:"))
			return;

		array<string> parts = {};
		verb.Split(":", parts, false);
		if (parts.Count() < 2)
			return;

		string what = parts[1];
		switch (what)
		{
			case "list": ReadList(playerId); break;
			case "call": PlayBack(playerId); break;
			case "case": ReadCase(playerId); break;
			case "lorry": ReadLine(playerId); break;
			case "supper": SupperNames(playerId); break;
			case "buyback": BuyBack(playerId); break;
			case "boy": BoyOut(playerId); break;
			case "sit": SitWith(playerId, spot); break;
			case "tell":
			{
				if (parts.Count() >= 3)
					TellTable(playerId, parts[2].ToInt());

				break;
			}
			case "ridge":
			{
				if (parts.Count() >= 4)
					RidgeJob(playerId, parts[2].ToInt(), parts[3].ToInt());

				break;
			}
			case "kitchen":
			{
				if (parts.Count() >= 3)
					Kitchen(playerId, parts[2].ToInt());

				break;
			}
		}

		m_Epi.GetSpots().Dirty();
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadList(int playerId)
	{
		array<string> names = {};
		foreach (RZ_EpiName n : m_Epi.Data().names)
		{
			if (!n.name.IsEmpty() && !m_Epi.IsStruck(n.name) && names.Count() < 8)
				names.Insert(n.name);
		}

		string text = "The clerk reads it from the top, every crossing, and doesn't skip the ones in pencil.";
		if (!names.IsEmpty())
			text += " At the end, the season's own: " + Joined(names) + ".";

		m_GM.NotifyPlayer(playerId, "The List", text);
		m_Epi.Met(playerId, "stand.list");
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayBack(int playerId)
	{
		string line = "\"Command, Bastion. Holding at the barrier. Requesting relief. Over.\"";
		switch (RZ_MajorHelp.World(m_GM, "orders.stance"))
		{
			case 2: line = "\"All stations, all houses, this is Bastion. Civilians first. Bastion out.\""; break;
			case 3: line = "\"Bastion, Command. Hold the line.\""; break;
			case 4: line = "\"Effective strength as read. Bastion out.\""; break;
		}

		m_GM.NotifyPlayer(playerId, "The roster board", "The operator winds the tape back and plays the last call through the handset: " + line + " Then the hiss, all of it.");
		m_Epi.Met(playerId, "stand.roster");
	}

	//------------------------------------------------------------------------------------------------
	protected void ReadCase(int playerId)
	{
		string text = "\"Contents: the method, the site log, the author list. To be opened on the far side of the barrier.\"";
		switch (RZ_MajorHelp.World(m_GM, "proof.first_page"))
		{
			case 2: text = "\"Contents: the whole of it. First page: three weeks before Day 1.\" Somebody has underlined three weeks."; break;
			case 4: text = "\"Contents: under seal.\" The seal is Bastion's. The latch is Halcyon's."; break;
		}

		m_GM.NotifyPlayer(playerId, "The case on the table", text);
		m_Epi.Met(playerId, "stand.case");
	}

	//------------------------------------------------------------------------------------------------
	//! The lad reads you your line from the book: the epilogue's line about you, else your page.
	protected void ReadLine(int playerId)
	{
		string ident = Ident(playerId);
		string line = m_Epi.LineOf(ident, 0);
		if (line.IsEmpty())
			line = "Nothing under your name. The lad looks twice. \"Clean page. Rare, that.\"";

		m_GM.NotifyPlayer(playerId, "The lorry at the crossroads", "The lad runs a finger down the page and reads it out flat: " + line);
		m_Epi.Met(playerId, "stand.lorry");
	}

	//------------------------------------------------------------------------------------------------
	//! At the table: tonight's names, and in processing a few of the Last Entries.
	protected void SupperNames(int playerId)
	{
		array<string> names = {};
		foreach (RZ_EpiName n : m_Epi.Data().names)
		{
			if (!n.name.IsEmpty() && !m_Epi.IsStruck(n.name) && names.Count() < 6)
				names.Insert(n.name);
		}

		string text = "The chair reads, and the table goes quiet for each one.";
		if (!names.IsEmpty())
			text += " " + Joined(names) + ".";

		string entries = SupperEntries();
		if (!entries.IsEmpty())
			text += "\nThen the clerk reads from the Names board: " + entries;

		m_GM.NotifyPlayer(playerId, "Supper at the long table", text);
		m_Epi.Deed(playerId, "read");
	}

	//------------------------------------------------------------------------------------------------
	//! A few of the Last Entries, read at supper in processing ("" before it).
	string SupperEntries()
	{
		if (m_Epi.Phase() < 3)
			return "";

		string text = "";
		int n = 0;
		foreach (RZ_EpiEntry e : m_Epi.Data().entries)
		{
			if (n >= 3 || m_Epi.IsStruck(e.name))
				continue;

			if (n > 0)
				text += " ";

			text += string.Format("%1: \"%2\"", e.name, e.text);
			n++;
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Buy a keepsake back from the fence: one the season didn't bring home goes back on the road
	//! with you (10 Red Hand Favor). The grey way of Bring Them Home.
	protected void BuyBack(int playerId)
	{
		string ident = Ident(playerId);
		RZ_EpiName pick;
		foreach (RZ_EpiName n : m_Epi.Data().names)
		{
			if (n.state == 3 && n.ident.IsEmpty())
			{
				pick = n;
				break;
			}
		}

		if (!pick)
		{
			m_GM.NotifyPlayer(playerId, "Rudi's crossroads", "\"Nothing of anyone's on the table this week. Ask me tomorrow.\"");
			return;
		}

		if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, 10))
		{
			m_GM.NotifyPlayer(playerId, "Rudi's crossroads", "\"Ten Favor. It's somebody's whole life. Ten.\"");
			return;
		}

		pick.state = 1;
		pick.carrier = ident;
		R().bought.Insert(ident + "|" + pick.name);
		m_GM.NotifyPlayer(playerId, "Rudi's crossroads", string.Format("The fence slides %1 across. \"%2. Off a body on the road. I don't ask.\" Take it home.", pick.what, pick.name));
		m_Epi.Deed(playerId, "buyback");
	}

	//------------------------------------------------------------------------------------------------
	//! The boy talked out of the queue: the Kid on the roads, or a child at the nearest house.
	protected void BoyOut(int playerId)
	{
		if (!R().boyOut.IsEmpty() || m_Epi.Data().tableState != 2)
			return;

		R().boyOut.Insert(Ident(playerId));
		string kid = "";
		vector q = m_Epi.GetSpots().QueuePos();
		if (m_GM.GetWanderers())
			kid = m_GM.GetWanderers().AddWithRole(RZ_EWandererRole.KID, "the boy", q);

		if (kid.IsEmpty() && m_GM.GetSafehouses())
			m_GM.GetSafehouses().AddPopulation(q, 1);

		m_GM.NotifyPlayer(playerId, "The queue", "You talk to him about nothing for a while, and then about the houses. When the queue shuffles up, he doesn't.");
		RZ_WorldFacts.Get().SetFlag("table.boy_out", 1);
		m_Epi.Deed(playerId, "boy");
	}

	//------------------------------------------------------------------------------------------------
	//! At the queue, telling the corporal (Bastion takes them aside) or the chair (the Haven does).
	protected void TellTable(int playerId, int who)
	{
		if (R().tableTold != 0 || m_Epi.Data().tableState != 2)
			return;

		R().tableTold = who;
		RZ_WorldFacts.Get().SetFlag("table.told", who);
		if (who == 1)
			m_GM.NotifyPlayer(playerId, "The queue", "The corporal listens, writes nothing down, and walks the family out of the line to the back of a truck. Nobody in the queue looks round.");
		else
			m_GM.NotifyPlayer(playerId, "The queue", "The chair listens, then goes down the line and stands with the mother a long time. The family stays in the queue, at the very back.");

		m_Epi.Deed(playerId, "table");
	}

	//------------------------------------------------------------------------------------------------
	//! The Nurse's last round in full: sitting with a bitten resident.
	protected void SitWith(int playerId, RZ_EpiSpot spot)
	{
		string key = string.Format("sit|%1|%2", Ident(playerId), RZ_WorldFacts.Get().Day());
		if (m_Epi.GetClosing().Count(key) > 0)
		{
			m_GM.NotifyPlayer(playerId, spot.title, "\"You sat with her this morning. She's asleep now.\"");
			return;
		}

		m_Epi.GetClosing().Bump(key);
		m_GM.NotifyPlayer(playerId, spot.title, "A blanket, a chair by the bed, and an hour of somebody else's stories. The dose comes in the afternoon. You were there when it did.");
		m_Epi.Deed(playerId, "sit");
	}

	//------------------------------------------------------------------------------------------------
	//! Quiet the Ridges, the factions' jobs: 0 sit the last call, 1 Bastion's cull, 2 the Red Hand's
	//! bet (15 Favor), 3 Halcyon's last reading at dusk. Once an identity a pack a job.
	protected void RidgeJob(int playerId, int job, int tribeId)
	{
		string ident = Ident(playerId);
		string key = string.Format("%1|%2|%3", ident, tribeId, job);
		RZ_Tribes tribes = m_GM.GetTribes();
		RZ_Tribe t;
		if (tribes)
			t = tribes.Find(tribeId);

		if (!t || R().ridgeJobs.Contains(key))
			return;

		string place = t.m_sPlace;
		string text;
		if (job == 1)
		{
			if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN))
			{
				m_GM.NotifyPlayer(playerId, "Below " + place, "Bastion's cull is for people Bastion knows.");
				return;
			}

			text = string.Format("You log the contact for the post: %1, still calling. The cull goes on the board for tomorrow.", place);
			m_GM.AddFavor(playerId, RZ_EFaction.MILITARY, 5);
		}
		else if (job == 2)
		{
			if (!m_GM.SpendFavor(playerId, RZ_EFaction.BANDITS, 15))
			{
				m_GM.NotifyPlayer(playerId, "Below " + place, "\"Fifteen on the lot up there. Or nothing.\"");
				return;
			}

			text = string.Format("Your fifteen goes in the book against %1 calling last. The odds are written in pencil.", place);
		}
		else if (job == 3)
		{
			float hour = RZ_GameTime.GetHour();
			if (!RZ_GameTime.IsCalibrated() || hour < 18 || hour > 21.5)
			{
				m_GM.NotifyPlayer(playerId, "Below " + place, "The reading is taken at dusk, when they call.");
				return;
			}

			text = string.Format("You take the reading at dusk below %1 for the paper's appendix. The call, the time, the count.", place);
			m_GM.AddFavor(playerId, RZ_EFaction.INSTITUTE, 5);
		}
		else
		{
			text = string.Format("You sit below %1 until the call comes, and stay until it stops.", place);
		}

		R().ridgeJobs.Insert(key);
		string name = RZ_EpiNames.Now(playerId);
		if (R().ridgeNames.Count() < 3 && !name.IsEmpty() && !R().ridgeNames.Contains(name))
			R().ridgeNames.Insert(name);

		m_GM.NotifyPlayer(playerId, "Below " + place, text);
		m_Epi.Deed(playerId, "ridge");
	}

	//------------------------------------------------------------------------------------------------
	//! Take Back a Town: a resident walked to their kitchen once more (once an identity a town).
	protected void Kitchen(int playerId, int town)
	{
		string key = Ident(playerId) + "|" + town.ToString();
		if (R().kitchens.Contains(key))
			return;

		RZ_Settlement st = m_GM.GetSettlement(town);
		if (!st)
			return;

		R().kitchens.Insert(key);
		string name = RZ_EpiNames.Now(playerId);
		if (R().takeNames.Count() < 3 && !name.IsEmpty() && !R().takeNames.Contains(name))
			R().takeNames.Insert(name);

		m_GM.NotifyPlayer(playerId, st.m_sName, "The kitchen is as it was, under the dust. They stand in it for a while, then wash one cup and leave it upside down on the drainer.");
		m_Epi.Deed(playerId, "kitchen");
	}

	// ================================================================ Scenes that move
	//------------------------------------------------------------------------------------------------
	//! Before processing, one more joins the queue from the houses every two real hours (x scale).
	protected void QueueJoins()
	{
		if (m_Epi.Phase() >= 3)
			return;

		int now = m_Epi.Now();
		int gap = RZ_Util.RoundToInt(2 * HOUR * m_Epi.Scale());
		if (R().joinUnix == 0)
			R().joinUnix = now;

		if (now - R().joinUnix < gap || R().joined >= 6)
			return;

		R().joinUnix = now;
		R().joined = R().joined + 1;
		m_Epi.GetSpots().Dirty();
	}

	//------------------------------------------------------------------------------------------------
	//! In processing, the queue moves up at each change of shift (06:00, 14:00, 22:00): those near
	//! it are told, and it's one shorter.
	protected void QueueShift(array<IEntity> players)
	{
		if (m_Epi.Phase() < 3 || !RZ_GameTime.IsCalibrated())
			return;

		float hour = RZ_GameTime.GetHour();
		int shift = -1;
		if (hour >= 6 && hour < 6.5)
			shift = 0;
		else if (hour >= 14 && hour < 14.5)
			shift = 1;
		else if (hour >= 22 && hour < 22.5)
			shift = 2;

		if (shift < 0)
			return;

		int key = RZ_WorldFacts.Get().Day() * 3 + shift;
		if (key == R().shiftKey)
			return;

		R().shiftKey = key;
		R().shuffles = R().shuffles + 1;
		vector q = m_Epi.GetSpots().QueuePos();
		if (q != vector.Zero)
			TellNear(players, q, 300, "The queue", "The change of shift at the barrier. The queue picks up its bags and shuffles forward twenty metres, one of Bastion's walking beside it.");

		m_Epi.GetSpots().Dirty();
	}

	//------------------------------------------------------------------------------------------------
	//! The Preacher at dusk: the nearest town's recorded dead, and his bells.
	protected void Preacher(array<IEntity> players)
	{
		RZ_Wanderers w = m_GM.GetWanderers();
		if (!w || !RZ_GameTime.IsCalibrated())
			return;

		float hour = RZ_GameTime.GetHour();
		int day = RZ_WorldFacts.Get().Day();
		if (hour < 19 || hour >= 20 || R().preacherDay == day)
			return;

		array<int> found = {};
		w.GetLiving(RZ_Wanderers.RoleBit(RZ_EWandererRole.PREACHER), found);
		if (found.IsEmpty())
			return;

		vector pos;
		if (!w.GetPosition(found[0], pos))
			return;

		R().preacherDay = day;
		int town = m_GM.FindNearestSettlement(pos);
		RZ_Settlement st = m_GM.GetSettlement(town);
		string place = "the road";
		if (st)
			place = st.m_sName;

		string dead = m_Epi.GetClosing().DeadAt(pos);
		string text = string.Format("The Preacher rings his bells at the edge of %1 and reads out the ones the town lost, the names he has.", place);
		if (!dead.IsEmpty())
			text += " " + dead;

		TellNear(players, pos, 400, "At dusk", text);
	}

	// ================================================================ Radio
	//------------------------------------------------------------------------------------------------
	//! Last Rounds: the finale's routes' closing lines carried on the codas, each once, on the
	//! coda's own faction's channel after it ran.
	protected void CodaLines()
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (!life)
			return;

		array<string> keys = {"every_name", "orders", "proof", "book"};
		array<int> factions = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.INSTITUTE, RZ_EFaction.BANDITS};
		foreach (int i, string key : keys)
		{
			if (RZ_MajorHelp.World(m_GM, key + ".coda") == 0 || R().codaLines.Contains(key))
				continue;

			R().codaLines.Insert(key);
			string line = RouteCodaLine(i);
			if (!line.IsEmpty())
				life.OpenChannelLine(factions[i], line);

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The route's line for each coda: the Haven's channel says her name, Bastion's last call reads
	//! the four R numbers, the cook at the lorry, the readout, the chalk on the lab gate.
	protected string RouteCodaLine(int which)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		string unturned = "";
		if (finale && finale.GetPeople())
			unturned = finale.GetPeople().Name("woman");

		switch (which)
		{
			case 0:
			{
				if (unturned.IsEmpty() || RZ_MajorHelp.World(m_GM, "list.buried") != 0)
					return "";

				return string.Format("If you can hear this: her name is %1. She didn't turn. Say it at your table tonight.", unturned);
			}
			case 1: return "R1, R2, R3, R4. Four numbers on the roster nobody ever explained. Read for the record. Bastion out.";
			case 2:
			{
				if (m_Epi.Route() == RZ_CureFinale.HALCYON)
					return "Reference stock: logged. The lab thanks the donors, by number.";

				return "Reference stock: wanted.";
			}
			case 3:
			{
				if (RZ_MajorHelp.World(m_GM, "cook.line_paid") != 0)
					return "The cook's at the lorry with the pan. Her line's paid. Soup. It keeps.";

				return "Soup. It keeps.";
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven channel's answer to the Last Broadcast, the evening after it aired.
	protected void BroadcastAnswer()
	{
		RZ_EpilogueSave s = m_Epi.Data();
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (s.broadcast < 0 || s.airedHour < 0 || R().answerDay >= 0 || !life || !RZ_GameTime.IsCalibrated())
			return;

		float hour = RZ_GameTime.GetHour();
		if (hour < 18 || hour >= 22)
			return;

		R().answerDay = RZ_WorldFacts.Get().Day();
		life.OpenChannelLine(RZ_EFaction.SURVIVORS, "Heard you on the station. Mine's in a car on the north road. Tell whoever's reading the names.");
	}

	// ================================================================ Keepsakes
	//------------------------------------------------------------------------------------------------
	//! Bring Them Home's other keepsakes, once: ID cards off the named young the packs still run with
	//! (never a player's character), and a driver's papers from each of the last four lost trucks.
	protected void Keepsakes()
	{
		if (R().keepsakes || !m_Epi.ClosingOn())
			return;

		R().keepsakes = true;
		RZ_Tribes tribes = m_GM.GetTribes();
		if (tribes)
		{
			foreach (RZ_Tribe t : tribes.GetTribeList())
			{
				foreach (RZ_NamedYoung y : t.m_aNamed)
				{
					if (y.m_bPlayer || !y.m_sIdent.IsEmpty() || y.m_sName.IsEmpty())
						continue;

					m_Epi.AddNamed(y.m_sName, "their ID card", -1, RZ_Mission.LandPointNear(t.m_vLair, 150, 300), "", "", y.m_sWhat);
				}
			}
		}

		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		for (int f = 0; mem && f < RZ_Factions.COUNT; f++)
		{
			array<RZ_Signal> trucks = {};
			mem.RecentSignals("truck." + f.ToString(), vector.Zero, 1000000, 60, trucks);
			int count = 0;
			for (int i = trucks.Count() - 1; i >= 0 && count < 4; i--)
			{
				count++;
				RZ_Signal sig = trucks[i];
				m_Epi.AddNamed(string.Format("the driver for %1", sig.text), "a driver's papers", f, Vector(sig.x, 0, sig.z), "", "", "Driver");
			}
		}
	}

	// ================================================================ People
	//------------------------------------------------------------------------------------------------
	//! A resident's line by the route that controls the cure, the losing routes' with dignity, or ""
	//! (the built line stays). Picked by the house and the day so it doesn't flicker.
	string ResidentLine(int house)
	{
		array<string> lines = {};
		RouteLines(m_Epi.Route(), lines);
		if (lines.IsEmpty())
			return "";

		int pick = Math.AbsInt(house * 7 + RZ_WorldFacts.Get().Day()) % lines.Count();
		return lines[pick];
	}

	//------------------------------------------------------------------------------------------------
	protected void RouteLines(int route, notnull array<string> lines)
	{
		switch (route)
		{
			case RZ_CureFinale.HAVEN:
			{
				lines.Insert("\"She gave three times so we could have this. I say her name at the table now. Everyone does.\"");
				lines.Insert("\"The medics come round with the cold box in the mornings. They ask your name first.\"");
				lines.Insert("\"Our house, then the next house. That's the order. Nobody argued.\"");
				lines.Insert("\"My boy had it this morning. He's asleep. He hasn't slept like that since Day 2.\"");
				lines.Insert("\"The council voted for once and nobody walked out.\"");
				lines.Insert("\"We ran out of bowls, so the soup went round in cups.\"");
				lines.Insert("\"There's a list on the door of who's had it. My name's in pencil. Pencil's fine.\"");
				lines.Insert("\"I was angry all season. I'm too tired for it now.\"");
				break;
			}
			case RZ_CureFinale.BASTION:
			{
				lines.Insert("\"Names on the roster, then the needle. They say it's fair. It's tidy, anyway.\"");
				lines.Insert("\"A soldier wrote my name down three times this week. I was cured on the third.\"");
				lines.Insert("\"Bastion says the column first. My bus was the column. I'm not complaining.\"");
				lines.Insert("\"They stencilled the crates. They stencil everything. Even the cure.\"");
				lines.Insert("\"Curfew's still curfew. The cure didn't change that.\"");
				lines.Insert("\"The lieutenant said sorry when he turned my cousin away. I believed him.\"");
				lines.Insert("\"You queue, you sign, you wait. Like the Day 2 buses, only it works.\"");
				lines.Insert("\"They gave the children theirs first. Whatever else, they did that.\"");
				break;
			}
			case RZ_CureFinale.REDHAND:
			{
				lines.Insert("\"Everyone's got a line in the book now. Mine says two tins. Cheap, for a life.\"");
				lines.Insert("\"The crew's price went down on Tuesday. Nobody knows why. Nobody asks.\"");
				lines.Insert("\"I paid for my mother's. She'll never know what it cost. I'll never tell her.\"");
				lines.Insert("\"Say what you like about the crossroads. The crossroads had it first.\"");
				lines.Insert("\"There's soup at the lorry. The cook won't take money for the soup.\"");
				lines.Insert("\"I owe the Red Hand for the rest of my life. The rest of my life is longer now.\"");
				lines.Insert("\"The book's open on the tailgate. You can read your own page. Most don't.\"");
				lines.Insert("\"They wrote my name in the book in very nice handwriting.\"");
				break;
			}
			case RZ_CureFinale.HALCYON:
			{
				lines.Insert("\"They wrote down my blood type, my weight and my house. Then the dose.\"");
				lines.Insert("\"The researcher thanked me for my participation. I didn't know I'd participated.\"");
				lines.Insert("\"It works. That's what the paper will say. It works, and it was them.\"");
				lines.Insert("\"There's a form for everything at the lab gate. Even for crying.\"");
				lines.Insert("\"They asked if I'd been bitten before Day 1. I said I didn't know there was a before.\"");
				lines.Insert("\"A beak walked me to the table and back. Polite. Very polite.\"");
				lines.Insert("\"The method's published. My cousin at the far house read it out to the medic.\"");
				lines.Insert("\"They gave me a number to keep. I keep it in my shoe.\"");
				break;
			}
			case RZ_CureFinale.INDEP:
			{
				lines.Insert("\"Released to nobody. So it came to us. Somebody chalked it on a crate.\"");
				lines.Insert("\"Eight walked out on Day 6. I knew one. He never said.\"");
				lines.Insert("\"No list, no book, no roster. First come. My feet were first.\"");
				lines.Insert("\"Nobody's cure, they call it. It feels like everybody's.\"");
				lines.Insert("\"The crate was open on the road. We took two and left the rest. Everyone did.\"");
				lines.Insert("\"The factions are sulking. Let them sulk. They're cured too.\"");
				lines.Insert("\"Somebody did this for nothing. I keep wanting to thank them.\"");
				lines.Insert("\"WE TRIED, it says on the lab gate. They did, didn't they.\"");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A losing route's people, carrying on with dignity, for someone of that route ("" otherwise).
	string LostLine(int faction)
	{
		int route = m_Epi.Route();
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale || route < 0 || faction == route || !RZ_Factions.IsValid(faction) || finale.CountLocks(faction) <= 0)
			return "";

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "\"We stood on the lab road. We lost. The houses are still the houses.\"";
			case RZ_EFaction.MILITARY: return "\"We stood on the lab road. We lost. Logged, and the watch goes on.\"";
			case RZ_EFaction.BANDITS: return "\"We stood on the lab road. We lost. The book says we're square.\"";
			case RZ_EFaction.INSTITUTE: return "\"We stood on the lab road. We lost. The work's still the work.\"";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The cook's, the sentry's and the toll man's own lines, by the faction of the one speaking.
	static string OwnLine(int faction, int day)
	{
		array<string> lines = {};
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				lines.Insert("\"I'm the cook. I cooked all season for more people than we had bowls. This week I cook for the queue.\"");
				lines.Insert("\"I've cooked the same soup ninety ways. This week it's the ninety-first.\"");
				break;
			}
			case RZ_EFaction.MILITARY:
			{
				lines.Insert("\"I'm on the gate. I was on the gate on Day 2. Same gate. Different reason to stand at it.\"");
				lines.Insert("\"Sentry. I log who goes in. For the record. Someone should keep a record.\"");
				break;
			}
			case RZ_EFaction.BANDITS:
			{
				lines.Insert("\"I take the toll. One tin, or one dose, whichever's cheaper this week.\"");
				lines.Insert("\"Toll's still a toll. The road didn't stop being a road because the lab finished.\"");
				break;
			}
			case RZ_EFaction.INSTITUTE:
			{
				lines.Insert("\"I keep the gate's book. Name, house, time. For the record.\"");
				lines.Insert("\"We count everything. This week we count the ones who come back to say thank you.\"");
				break;
			}
		}

		if (lines.IsEmpty())
			return "";

		return lines[Math.AbsInt(day) % lines.Count()];
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer who remembers you says your line first ("" if they don't).
	string WandererFirst(RZ_Wanderer w, int playerId)
	{
		if (!w || playerId <= 0)
			return "";

		string ident = Ident(playerId);
		if (!w.Remembers(ident, RZ_Wanderers.MEM_HELPED) && !w.Remembers(ident, RZ_Wanderers.MEM_SAVED))
			return "";

		string line = m_Epi.LineOf(ident, 2);
		if (line.IsEmpty())
			line = m_Epi.LineOf(ident, 0);

		return line;
	}

	// ================================================================ Leaders' topics
	//------------------------------------------------------------------------------------------------
	//! The topics one by one (44 the cure, 45 this place, 46 me, 47 the dead, 48 what you didn't
	//! tell me), each coloured by the leader's traits or loyalty.
	void Topics(RZ_Leader l, int playerId, notnull array<string> labels, notnull array<int> ids)
	{
		labels.Insert("The cure");
		ids.Insert(44);
		labels.Insert("This place");
		ids.Insert(45);
		labels.Insert("Me");
		ids.Insert(46);
		labels.Insert("The dead");
		ids.Insert(47);
		if (l.IsStronghold() && !R().truthsTold.Contains(Ident(playerId)))
		{
			labels.Insert("What you didn't tell me");
			ids.Insert(48);
		}

		if (m_Epi.ClosingOn() && !R().settled.Contains(string.Format("%1|%2", Ident(playerId), l.m_eFaction)))
		{
			labels.Insert("Settle accounts");
			ids.Insert(49);
		}
	}

	//------------------------------------------------------------------------------------------------
	bool PickTopic(RZ_Leader l, int playerId, int replyId)
	{
		if (replyId == 49)
			return Settle(l, playerId);

		if (replyId < 44 || replyId > 48)
			return false;

		string text = "";
		switch (replyId)
		{
			case 44: text = "\"" + RZ_EpiText.Where(m_Epi.Route(), m_Epi.Terms(), m_Epi.Faction()) + "\""; break;
			case 45: text = m_Epi.GetBeats().PlaceLine(l.m_vPos, 400); break;
			case 46: text = (m_Epi.GetRecord().Title(Ident(playerId), l.m_eFaction) + " " + m_Epi.LineOf(Ident(playerId), 1)).Trim(); break;
			case 47: text = m_Epi.GetClosing().DeadAt(l.m_vPos); break;
			case 48: text = Truth(l, playerId); break;
		}

		if (text.IsEmpty())
			text = "\"Nothing I'd say twice.\"";

		string colour = Colour(l, replyId);
		if (!colour.IsEmpty())
			text = colour + "\n" + text;

		string own = OwnLine(l.m_eFaction, RZ_WorldFacts.Get().Day());
		if (replyId == 45 && !own.IsEmpty() && !l.IsStronghold())
			text += "\n" + own;

		string lost = LostLine(l.m_eFaction);
		if (replyId == 44 && !lost.IsEmpty())
			text += "\n" + lost;

		m_GM.NotifyPlayer(playerId, l.GetHeader(), text);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Greedy leader prices it, a Merciful one asks if you've eaten; outpost commanders and council
	//! heads by loyalty.
	protected string Colour(RZ_Leader l, int replyId)
	{
		if (l.m_iTraitA == RZ_ELeaderTrait.GREEDY || l.m_iTraitB == RZ_ELeaderTrait.GREEDY)
			return "\"That one's free. The next one won't be.\"";

		if (l.m_iTraitA == RZ_ELeaderTrait.MERCIFUL || l.m_iTraitB == RZ_ELeaderTrait.MERCIFUL)
			return "\"Have you eaten? Sit. Then ask.\"";

		if (l.IsStronghold())
			return "";

		if (l.m_eLoyalty == RZ_ELoyalty.ROGUE)
			return "\"Don't tell the stronghold I said this.\"";

		if (l.m_eLoyalty != RZ_ELoyalty.LOYAL && replyId == 44)
			return "\"The stronghold has a line on it. Here's mine.\"";

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Settle Accounts at a leader, once a faction in the week: hand back their kit if you're
	//! wearing it, own up to what's owed (up to 10 of their Infamy forgiven), or a plain goodbye.
	protected bool Settle(RZ_Leader l, int playerId)
	{
		int f = l.m_eFaction;
		R().settled.Insert(string.Format("%1|%2", Ident(playerId), f));
		RZ_Hostility hostility = m_GM.GetHostility();
		bool wearing = hostility && hostility.GetDisguise() && hostility.GetDisguise().IsPassing(playerId, f);
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		int owed = p.GetInfamy(f);
		string text;
		if (wearing)
		{
			m_GM.Forgive(playerId, f, Math.Min(5, owed), "handed back borrowed kit");
			text = "\"That's ours you're wearing.\" You take it off and hand it over. \"Kept it clean, at least. Square.\"";
		}
		else if (owed > 0)
		{
			int off = m_GM.Forgive(playerId, f, Math.Min(10, owed), "owned up before the gates opened");
			text = string.Format("You say it plainly, what you did and to whom. %1 listens all the way through. \"Owned up to, then.\" (%2 Infamy off)", l.m_sShort, off);
		}
		else
		{
			text = "\"You don't owe us anything. Odd thing to say this week. Go on.\"";
		}

		m_GM.NotifyPlayer(playerId, l.GetHeader(), text);
		m_Epi.Deed(playerId, "settle");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "What you didn't tell me": the truth deck's season scope. The worst truth still unfound that
	//! this faction's people kept, told once to each player; a Rogue leader lies again.
	protected string Truth(RZ_Leader l, int playerId)
	{
		string ident = Ident(playerId);
		R().truthsTold.Insert(ident);
		RZ_WorldMemory mem = RZ_WorldFacts.Get().Memory();
		if (!mem)
			return "";

		if (l.m_eLoyalty == RZ_ELoyalty.ROGUE)
			return "\"Everything I told you was true.\" It wasn't, and it still isn't.";

		array<RZ_Truth> truths = {};
		mem.AllTruths(truths);
		foreach (RZ_Truth t : truths)
		{
			if (t.value == "as_said" || !t.found.IsEmpty())
				continue;

			mem.FoundTruth(t.key, t.subject, "epilogue");
			return string.Format("\"Since it doesn't matter now: the job about %1 wasn't what we said. It was %2. We knew.\"", Pretty(t.subject), Pretty(t.value));
		}

		return "\"There's nothing left I didn't tell you. That surprises me too.\"";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Pretty(string raw)
	{
		string s = raw;
		s.Replace("_", " ");
		int colon = s.IndexOf(":");
		if (colon >= 0 && colon < s.Length() - 1)
			s = s.Substring(colon + 1, s.Length() - colon - 1);

		return s;
	}

	// ================================================================ The Walk Out's order
	//------------------------------------------------------------------------------------------------
	//! Who walks first, by the route's terms (processing's first day): Bastion's roster (Known with
	//! Bastion), the Haven's houses' order (Known with the Haven), the Red Hand's place near the front
	//! (Known with the Red Hand, or a line paid), the authors (Known with Halcyon); nobody's cure, no
	//! order. From the second day of processing, anyone. "" if this player may walk now.
	string WalkOrder(int playerId)
	{
		RZ_EpilogueSave s = m_Epi.Data();
		if (s.cStartUnix <= 0 || m_Epi.Now() - s.cStartUnix > RZ_Util.RoundToInt(DAY * m_Epi.Scale()))
			return "";

		int route = m_Epi.Route();
		if (route < 0 || route >= RZ_CureFinale.INDEP)
			return "";

		if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, route), RZ_ETier.KNOWN))
			return "";

		if (route == RZ_CureFinale.REDHAND && s.bookPaid.Contains(Ident(playerId)))
			return "";

		switch (route)
		{
			case RZ_CureFinale.BASTION: return "\"Roster first. You're not on it yet. Tomorrow.\"";
			case RZ_CureFinale.HAVEN: return "\"The houses go in their order. Yours is tomorrow.\"";
			case RZ_CureFinale.REDHAND: return "\"The crew's people walk with the next shift. You're after.\"";
			case RZ_CureFinale.HALCYON: return "\"The authors first, then the rest. Tomorrow.\"";
		}

		return "";
	}

	// ================================================================ Lines
	//------------------------------------------------------------------------------------------------
	//! The week's hall lines this file adds (at the wipe): the ridges' and the towns' names.
	void HallLines()
	{
		if (!R().ridgeNames.IsEmpty())
			m_Epi.AddHallLine("Sat the ridges out to the last call: " + Joined(R().ridgeNames) + ".");

		if (!R().takeNames.IsEmpty())
			m_Epi.AddHallLine("Walked people home to their kitchens: " + Joined(R().takeNames) + ".");
	}

	//------------------------------------------------------------------------------------------------
	//! A coda's epithet for an identity ("at the barrier", "on the paper"; RZ_MajorHelp.HallNames).
	void AddEpithet(string ident, string epithet)
	{
		string key = ident + "|";
		foreach (string e : R().epithets)
		{
			if (e.StartsWith(key))
				return;
		}

		R().epithets.Insert(key + epithet);
	}

	//------------------------------------------------------------------------------------------------
	//! A name as the season block shows it, with its coda's epithet: "Novak, at the barrier".
	string Named(string ident, string shown)
	{
		string key = ident + "|";
		foreach (string e : R().epithets)
		{
			if (e.StartsWith(key))
				return shown + ", " + e.Substring(key.Length(), e.Length() - key.Length());
		}

		return shown;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Joined(notnull array<string> names)
	{
		string s = "";
		foreach (int i, string n : names)
		{
			if (i > 0)
				s += ", ";

			s += n;
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected void TellNear(array<IEntity> players, vector pos, float radius, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> all = {};
		RZ_Util.GetAlivePlayers(ids, all);
		foreach (int i, IEntity p : all)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	string StatusText()
	{
		return string.Format("rest: evenings %1, joined %2, shuffles %3, kitchen %4, holdouts %5", R().evenings, R().joined, R().shuffles, R().kitchen.Count(), R().holdouts.Count());
	}
}
