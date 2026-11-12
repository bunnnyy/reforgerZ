//! The rest of the four majors (docs/QUESTLINES.md, "The majors"; docs/DESIGN.md, "The majors"):
//! the shared engine's last pieces and each act's remaining approaches, in one place.
//! - Channels: OpenChannel (a reading of lines with a gap, the director's two minutes, the chair's
//!   List), SecureLine (a line Radio Operators hear on a faction's secure net for some hours), AtTime
//!   (a one-off line at a set game hour: Bastion's 20:30 last call, the 05:30 walk), Mute (one night
//!   of a faction's open channel).
//! - Profiles: Bastion's rank text ("Sergeant Novak") and the per-life "on the roster" flag.
//! - The codas' timing (the last call at 20:30 as radio, the walk at the next 05:30) and the no-cure
//!   codas: the Haven's Gate at the first wipe warning, Bastion's last call in the last 24 real
//!   hours, the Red Hand's ending at the wipe warning on its current branch.
//! - The optional props: the Red Hand lorry at the crossroads (the send-off and the last line) and
//!   the bonfire (sent off loud), spawned only near players.
//! - More(): each act's remaining approaches as choices on its scene, and their effects at the
//!   major's ending (AtEnding, from RZ_Majors.SeasonStory).
//! Server only. Readings and one-off lines are runtime (a restart drops a reading in progress).

//------------------------------------------------------------------------------------------------
class RZ_MajorReading
{
	int faction;
	ref array<string> lines = {};
	float gap;
	float wait;
}

//------------------------------------------------------------------------------------------------
class RZ_MajorTimed
{
	int faction;
	float hour;
	string text;
	string key;
}

//------------------------------------------------------------------------------------------------
class RZ_MajorRest
{
	protected static ref RZ_MajorRest s_Instance;
	protected static ref array<int> s_aSecureFaction = {};
	protected static ref array<string> s_aSecureText = {};
	protected static ref array<int> s_aSecureUntil = {};

	protected ref array<ref RZ_MajorReading> m_aReadings = {};
	protected ref array<ref RZ_MajorTimed> m_aTimed = {};
	protected IEntity m_Lorry;
	protected IEntity m_Bonfire;
	protected float m_fProps;

	//------------------------------------------------------------------------------------------------
	static RZ_MajorRest Get()
	{
		if (!s_Instance)
			s_Instance = new RZ_MajorRest();

		return s_Instance;
	}

	// ================================================================ Channels
	//------------------------------------------------------------------------------------------------
	//! A reading on a faction's open channel: one line every `gap` seconds of play (two minutes in
	//! all for the director's paper or the chair's List).
	static void OpenChannel(int faction, notnull array<string> lines, float gap)
	{
		RZ_MajorReading r = new RZ_MajorReading();
		r.faction = faction;
		r.gap = Math.Max(5, gap);
		foreach (string l : lines)
		{
			r.lines.Insert(l);
		}

		Get().m_aReadings.Insert(r);
	}

	//------------------------------------------------------------------------------------------------
	//! A line on a faction's secure net for `hours` real hours: Radio Operators hear it among the net's
	//! traffic (RZ_FactionLife.GetSecureTraffic).
	static void SecureLine(int faction, string text, float hours)
	{
		s_aSecureFaction.Insert(faction);
		s_aSecureText.Insert(text);
		s_aSecureUntil.Insert(RZ_SeasonClock.Now() + RZ_Util.RoundToInt(hours * 3600));
	}

	//------------------------------------------------------------------------------------------------
	static void SecureLines(int faction, notnull array<string> options)
	{
		int now = RZ_SeasonClock.Now();
		for (int i = s_aSecureText.Count() - 1; i >= 0; i--)
		{
			if (s_aSecureUntil[i] < now)
			{
				s_aSecureFaction.Remove(i);
				s_aSecureText.Remove(i);
				s_aSecureUntil.Remove(i);
				continue;
			}

			if (s_aSecureFaction[i] == faction)
				options.Insert(s_aSecureText[i]);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A one-off line at a set game hour (the next time it comes round), once per key.
	static void AtTime(int faction, float hour, string text, string key)
	{
		if (RZ_WorldFacts.Get().Flag("timed." + key) != 0)
			return;

		foreach (RZ_MajorTimed t : Get().m_aTimed)
		{
			if (t.key == key)
				return;
		}

		RZ_MajorTimed timed = new RZ_MajorTimed();
		timed.faction = faction;
		timed.hour = hour;
		timed.text = text;
		timed.key = key;
		Get().m_aTimed.Insert(timed);
	}

	//------------------------------------------------------------------------------------------------
	//! One night of silence on a faction's open channel (until the next morning).
	static void Mute(int faction)
	{
		RZ_WorldFacts.Get().SetFlag("mute." + faction.ToString(), RZ_WorldFacts.Get().Day() + 1);
	}

	//------------------------------------------------------------------------------------------------
	static bool Muted(int faction)
	{
		int until = RZ_WorldFacts.Get().Flag("mute." + faction.ToString());
		if (until <= 0)
			return false;

		int day = RZ_WorldFacts.Get().Day();
		if (day < until)
			return true;

		return day == until && RZ_GameTime.IsCalibrated() && RZ_GameTime.GetHour() < 6;
	}

	// ================================================================ Profiles
	//------------------------------------------------------------------------------------------------
	//! Put this life on Bastion's roster (Counted, the census, signing on).
	static void OnRoster(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (p && !p.m_sIdentity.IsEmpty())
			RZ_WorldFacts.Get().SetFlag(string.Format("roster.%1.%2", p.m_sIdentity, p.m_iLives), 1);
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOnRoster(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		return p && RZ_WorldFacts.Get().Flag(string.Format("roster.%1.%2", p.m_sIdentity, p.m_iLives)) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's rank text for a life on its roster: by its standing with Bastion; "" off the roster.
	static string RankText(RZ_GameModeComponent gm, int playerId)
	{
		if (!IsOnRoster(gm, playerId))
			return "";

		string rank = "Private";
		RZ_ETier tier = gm.GetTier(playerId, RZ_EFaction.MILITARY);
		if (tier == RZ_ETier.CHAMPION)
			rank = "Sergeant";
		else if (tier == RZ_ETier.TRUSTED)
			rank = "Corporal";

		return string.Format("%1 %2", rank, RZ_Names.Public(playerId));
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! Every majors' tick (10 s) with players online.
	void Tick(float dt, array<int> ids)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		Readings(gm, dt);
		Timed(gm);
		Codas(gm);
		NoCure(gm);
		m_fProps += dt;
		if (m_fProps >= 30)
		{
			m_fProps = 0;
			Props(gm, ids);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Readings(RZ_GameModeComponent gm, float dt)
	{
		RZ_FactionLife life = gm.GetFactionLife();
		if (!life || m_aReadings.IsEmpty())
			return;

		RZ_MajorReading r = m_aReadings[0];
		r.wait -= dt;
		if (r.wait > 0)
			return;

		if (r.lines.IsEmpty())
		{
			m_aReadings.Remove(0);
			return;
		}

		life.OpenChannelLine(r.faction, r.lines[0]);
		r.lines.Remove(0);
		r.wait = r.gap;
	}

	//------------------------------------------------------------------------------------------------
	protected void Timed(RZ_GameModeComponent gm)
	{
		RZ_FactionLife life = gm.GetFactionLife();
		if (!life || !RZ_GameTime.IsCalibrated())
			return;

		float hour = RZ_GameTime.GetHour();
		for (int i = m_aTimed.Count() - 1; i >= 0; i--)
		{
			RZ_MajorTimed t = m_aTimed[i];
			if (hour < t.hour || hour >= t.hour + 0.5)
				continue;

			life.OpenChannelLine(t.faction, t.text);
			RZ_WorldFacts.Get().SetFlag("timed." + t.key, 1);
			m_aTimed.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The codas' timing as radio: Bastion's last call at the next 20:30 and the walk at the 05:30
	//! after it, once the coda has run (they don't take the evening's slot).
	protected void Codas(RZ_GameModeComponent gm)
	{
		if (RZ_MajorHelp.World(gm, "orders.coda") == 0)
			return;

		AtTime(RZ_EFaction.MILITARY, 20.5, "Bastion. Last call of the season. Effective strength as read. Civilians sheltered as read. Bastion out.", "orders.last_call");
		if (RZ_WorldFacts.Get().Flag("timed.orders.last_call") != 0)
			AtTime(RZ_EFaction.MILITARY, 5.5, "Lieutenant Sedlak is walking the box of tags up the north road. Anyone who wants to answer for a name, be at the halt.", "orders.walk");
	}

	//------------------------------------------------------------------------------------------------
	//! No cure: the Haven's Gate at the first wipe warning (held at the stronghold safehouse),
	//! Bastion's last call in the last 24 real hours, the Red Hand's ending at the wipe warning on its
	//! current branch. Each once, as radio.
	protected void NoCure(RZ_GameModeComponent gm)
	{
		RZ_CureResearch cure = gm.GetCure();
		if (!cure || cure.IsCured() || !gm.GetSeasons() || !gm.GetSeasons().IsLastDay())
			return;

		RZ_FactionLife life = gm.GetFactionLife();
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!life)
			return;

		if (facts.Flag("nocure.gate") == 0)
		{
			facts.SetFlag("nocure.gate", 1);
			life.OpenChannelLine(RZ_EFaction.SURVIVORS, "The houses are at the chair's house tonight. No gate to stand at. We'll read the names here instead.");
		}

		if (facts.Flag("nocure.call") == 0 && RZ_Majors.WipeDueInHours() <= 24)
		{
			facts.SetFlag("nocure.call", 1);
			life.OpenChannelLine(RZ_EFaction.MILITARY, "Command, Bastion. Holding at the barrier. Requesting relief. Over.");
		}

		if (facts.Flag("nocure.book") == 0)
		{
			facts.SetFlag("nocure.book", 1);
			string line = "The boss closes the book. Nobody's paid, nobody's square. Same as ever.";
			if (RZ_MajorHelp.World(gm, "book.last") == 4)
				line = "The fire at the crossroads is out. The book went in it. That's the Red Hand's ending, cure or no cure.";

			life.OpenChannelLine(RZ_EFaction.BANDITS, line);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The lorry (the send-off, the last line) and the bonfire (sent off loud, the fire) at Rudi's
	//! crossroads while that stage runs and a player is within 300 m; gone past 500 m or after it.
	protected void Props(RZ_GameModeComponent gm, array<int> ids)
	{
		RZ_MajorSettings settings = gm.GetMajorSettings();
		RZ_Questlines q = gm.GetQuestlines();
		vector cross = RZ_MajorHelp.Crossroads();
		if (!settings || !q || cross == vector.Zero)
			return;

		RZ_QuestRun run = q.FindRunByKey("book");
		int stage = -1;
		if (run && !run.m_bDone)
			stage = run.m_iStage;

		bool near = false;
		foreach (int pid : ids)
		{
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
			if (ent && RZ_Util.DistXZ(ent.GetOrigin(), cross) <= 300)
				near = true;
		}

		bool lorry = near && (stage == RZ_Major_Book.S_SEND || stage == RZ_Major_Book.S_CODA);
		bool fire = near && stage == RZ_Major_Book.S_FIRE;
		m_Lorry = Prop(m_Lorry, lorry, settings.m_sLorryPrefab, RZ_Mission.LandPointNear(cross, 10, 14));
		m_Bonfire = Prop(m_Bonfire, fire, settings.m_sBonfirePrefab, RZ_Mission.LandPointNear(cross, 6, 10));
	}

	//------------------------------------------------------------------------------------------------
	protected static IEntity Prop(IEntity current, bool want, ResourceName prefab, vector at)
	{
		if (!want)
		{
			if (current)
				SCR_EntityHelper.DeleteEntityAndChildren(current);

			return null;
		}

		if (current || prefab.IsEmpty())
			return current;

		return RZ_Util.SpawnPrefab(prefab, at, Math.RandomFloat(0, 360));
	}

	// ================================================================ Ask about...
	//------------------------------------------------------------------------------------------------
	//! "Ask about <major>" (reply 50) on another faction's stronghold leader while that major has a
	//! stage open: the first running major not their own.
	static void AskAbout(RZ_GameModeComponent gm, RZ_Leader l, notnull array<string> labels, notnull array<int> ids)
	{
		RZ_QuestRun run = OtherMajor(gm, l);
		if (!run)
			return;

		labels.Insert("Ask about " + run.m_Line.m_sTitle);
		ids.Insert(50);
	}

	//------------------------------------------------------------------------------------------------
	static bool AskPick(RZ_GameModeComponent gm, RZ_Leader l, int playerId, int replyId)
	{
		if (replyId != 50)
			return false;

		RZ_QuestRun run = OtherMajor(gm, l);
		string text = "\"Whatever that is, it's somebody else's.\"";
		if (run)
			text = string.Format("\"%1? We hear things.\" %2", run.m_Line.m_sTitle, run.m_Line.JournalText(gm, run));

		gm.NotifyPlayer(playerId, l.GetHeader(), text);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_QuestRun OtherMajor(RZ_GameModeComponent gm, RZ_Leader l)
	{
		if (!l || !l.IsStronghold() || !gm.GetQuestlines() || !RZ_Majors.IsOn())
			return null;

		foreach (RZ_QuestRun run : gm.GetQuestlines().GetRuns())
		{
			if (!run.m_bDone && run.m_Line && run.m_Line.IsMajor() && run.m_Line.m_iFaction != l.m_eFaction && run.m_iStage > 0)
				return run;
		}

		return null;
	}

	// ================================================================ The acts' remaining approaches
	//------------------------------------------------------------------------------------------------
	//! More choices on a major's act scene (RZ_QM_MajorScene.Configure, after the major's own).
	void More(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		switch (run.m_sKey)
		{
			case "book": Book(gm, run, stage, scene); break;
			case "orders": Orders(gm, run, stage, scene); break;
			case "every_name": EveryName(gm, run, stage, scene); break;
			case "proof": Proof(gm, run, stage, scene); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The Book: act 1 (stand-ins for the crew bosses, the horns as a heat source), act 4 (slip off
	//! with the real page in the noise), act 5 (carry the bill to the lab: +10 Halcyon Fame; lift the
	//! book at supper: +25 Infamy), act 7 (your line "Paid" when Settle Accounts paid it).
	protected void Book(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector at = scene.m_vPos;
		switch (stage)
		{
			case RZ_Major_Book.S_SEND:
			{
				int chairs = scene.Person(RZ_Mission.LandPointNear(at, 8, 14), "the crew bosses' chairs", "Look at the chairs", "A chair for each crew boss round the fire. Two are empty. Somebody has to sit in them, or the send-off is short.");
				scene.Choice(chairs, "Sit in for a crew boss [Known]").Needs(RZ_EFaction.BANDITS, 1).Sets("standin").Says("You take the chair and the boss's cup. The crew behind it looks at you and decides to allow it.", "sat in for a crew boss");
				scene.Choice(chairs, "Sound the horns for Rudi").Sets("horns").Says("Every horn at the crossroads, all at once. Rudi would have liked it. Whatever's in the woods heard it too.", "sounded the horns for Rudi");
				break;
			}
			case RZ_Major_Book.S_SETTLE:
			{
				int dark = scene.Person(RZ_Mission.LandPointNear(at, 20, 30), "the back of the crowd", "Look at the crowd", "Everyone's facing the boss. Nobody's watching the satchel.");
				scene.Choice(dark, "Slip off with the real page [unseen]").Sneak().Sets("world:book.slipped").Says("The real page goes in your coat while the boss reads the forgery out loud.", "slipped off with the real page");
				break;
			}
			case RZ_Major_Book.S_LAST:
			{
				int bill = scene.Person(RZ_Mission.LandPointNear(at, 6, 10), "the bill, sealed", "Look at the bill", "The bill for the lab: what it owes the Red Hand, line by line. Somebody has to carry it up the hill.");
				scene.Choice(bill, "Carry the bill to the lab").Sets("world:book.bill_carried").Says("Up the hill with the bill in an envelope. The duty officer signs for it without reading it.", "carried the bill to the lab");
				scene.Choice(bill, "Lift the book at supper [unseen]").Sneak().Sets("world:book.lifted").Says("The book, from under the boss's elbow, at supper. Nobody at the table is in disguise. That's the problem.", "lifted the book at supper");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Await Relief: act IV (report the padding, cover for the commander, a dead man's name off the nail,
	//! the fence's rifle count for 20 Red Hand Favor), act V (collect the copies: the clerk's for 30
	//! Favor; the chair's clerk walked to the radio), act VI (answer "Here" for a struck-off man).
	protected void Orders(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector at = scene.m_vPos;
		switch (stage)
		{
			case RZ_Major_Orders.S_COUNT:
			{
				int pad = scene.Person(RZ_Mission.LandPointNear(at, 10, 20), "the duty officer's desk", "Talk to the duty officer", "\"If a post is padding its count, I'd want to know. So would the CO. Or would they.\"");
				scene.Choice(pad, "Report a padded post").Sets("world:orders.reported_pad").Says("The duty officer writes it down and doesn't look up. A sergeant somewhere loses a stripe.", "reported a padded post");
				scene.Choice(pad, "Cover for the commander").Sets("world:orders.covered").Says("\"No padding that I saw.\" The duty officer nods slowly. The number stays as it was.", "covered for a post's commander");
				scene.Choice(pad, "Take a dead man's name off the nail").Trick(RZ_EFaction.MILITARY, 2).Sets("post0").Says("A name off the nail, and a roster you're now on twice. The post's count comes out true.", "counted a post under a dead man's name").Refused("\"That's not your name. I knew him.\"");
				scene.Choice(pad, "Buy the fence's rifle count [20 Favor]").Bribe(RZ_EFaction.BANDITS, 20).Sets("post1").Says("The fence knows how many rifles every post sold. That's the count.", "bought the fence's rifle count");
				break;
			}
			case RZ_Major_Orders.S_MUSTER:
			{
				int copies = scene.Person(RZ_Mission.LandPointNear(at, 10, 20), "the clerk with the carbon copies", "Talk to the clerk", "\"Every post kept a copy of the orders. If you want the call kept, they should all be in one stove.\"");
				scene.Choice(copies, "Buy the clerk's copy [30 Favor]").Bribe(RZ_EFaction.MILITARY, 30).Sets("world:orders.copies").Says("The last carbon, folded small. The stove takes it.", "collected the last copy of the orders");
				scene.Choice(copies, "Walk the chair's clerk to the radio").Sets("world:orders.chair_clerk").Says("After curfew, with a torch, the long way round. The chair's clerk sits by the radio for the call.", "walked the chair's clerk to the radio");
				break;
			}
			case RZ_Major_Orders.S_CALL:
			{
				int roll = scene.Person(RZ_MajorHelp.NorthRoad(gm, 160), "a gap in the roll", "Listen to the roll", "\"Struck off,\" Sedlak reads. Nobody answers. Unless somebody does.");
				scene.Choice(roll, "Answer Here for a struck-off man").Sets("world:orders.here_count").Says("\"Here.\" Sedlak looks up, then writes it down. The count is one more.", "answered Here for a struck-off man");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every Name: act I (the cough a chest cold for a Nurse or Doctor), act II (an Elder hears who each
	//! head would call a mouth; a Radio Operator what each offer is for), act IV (carry 4 food from the
	//! best-fed house; two mouths into another house at night; the stranger to the crew's drop), act V
	//! (gather the others' pages before supper).
	protected void EveryName(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector at = scene.m_vPos;
		switch (stage)
		{
			case RZ_Major_EveryName.S_FAMILY:
			{
				int cough = scene.Person(RZ_Mission.LandPointNear(at, 6, 10), "the coughing child", "Look at the child", "A cough the head says is the fever. It might just be a cough.");
				scene.Choice(cough, "Call it a chest cold").Persuade(RZ_EFaction.SURVIVORS, 1, "nurse,doctor").Decides(RZ_Major_EveryName.OUT_ALL_IN).Says("\"A chest cold. Listen to it.\" The head listens, and lets all four in.", "called the cough a chest cold").Refused("\"You'd know the difference, would you?\"");
				break;
			}
			case RZ_Major_EveryName.S_SITTING1:
			{
				int ears = scene.Person(RZ_Mission.LandPointNear(at, 8, 12), "the back of the hall", "Listen at the back", "Every head has a word for the ones they'd put out. Every offer has a reason nobody says.");
				scene.Choice(ears, "Hear who they'd call a mouth").Persuade(RZ_EFaction.SURVIVORS, 1, "elder").Sets("heard_mouths").Says("You know which heads would say mouth, and which would say neighbour.", "heard who each head would put out").Refused("You'd need to have sat on a council.");
				scene.Choice(ears, "Hear what each offer is for").Persuade(RZ_EFaction.SURVIVORS, 1, "radioham").Sets("heard_offers").Says("Two offers on the table were made on the secure nets first. You know whose.", "heard what each offer is for").Refused("You'd need to have heard the nets.");
				break;
			}
			case RZ_Major_EveryName.S_HUNGRY:
			{
				int cart = scene.Person(RZ_Mission.LandPointNear(at, 15, 30), "the best-fed house's store", "Look at the store", "The best-fed house has four days' food spare. The hungry house has none.");
				scene.Choice(cart, "Carry four food over [4 food]").Trade(RZ_EErrandItem.FOOD, 4).Decides(RZ_Major_EveryName.OUT_FED).Says("Four lots of food across the road in a cart. Nobody stops you.", "carried food from the best-fed house");
				scene.Choice(cart, "Move two mouths at night [unseen]").Sneak().Decides(RZ_Major_EveryName.OUT_MOVED).Says("Two of them sleep somewhere else tonight, and eat there tomorrow.", "moved two mouths to another house at night");
				scene.Choice(cart, "Hand the stranger to the crew").Way(RZ_EQuestApproach.BETRAY).Sets("world:door.exile_sold").Says("The crew's drop takes the stranger and gives you a tin. Nobody at the house asks.", "handed the stranger to the crew's drop");
				break;
			}
			case RZ_Major_EveryName.S_SITTING2:
			{
				int pages = scene.Person(RZ_Mission.LandPointNear(at, 8, 12), "the others' pages", "Look at the pages", "The Red Hand has a page about the houses. Bastion has its Day 3 page. Before supper, somebody could have both.");
				scene.Choice(pages, "Gather the others' pages first").Sets("world:every_name.pages_gathered").Says("Two pages on the table before the soup. The chair reads both.", "gathered the others' pages before supper");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Proof: act I (the Red Hand reads it on its channel for 30 Red Hand Favor), act IV (the director's
	//! two minutes as a reading on the open channel), act VI (the author list read at supper for the
	//! Haven; the radio ham's "Got it.").
	protected void Proof(RZ_GameModeComponent gm, RZ_QuestRun run, int stage, RZ_QM_MajorScene scene)
	{
		vector at = scene.m_vPos;
		switch (stage)
		{
			case RZ_Major_Proof.S_DAYONE:
			{
				int crew = scene.Person(RZ_Mission.LandPointNear(at, 30, 60), "the crew's radio man", "Talk to the radio man", "\"Give me the date and I'll read it on our channel. Thirty, and Halcyon can't take it back.\"");
				scene.Choice(crew, "Have it read on the crews' channel").Bribe(RZ_EFaction.BANDITS, 30).Sets("world:proof.rh_read").Says("The date goes out on the Red Hand's channel, with a laugh after it.", "had the date read on the Red Hand's channel");
				break;
			}
			case RZ_Major_Proof.S_READING:
			{
				int mic = scene.Person(RZ_Mission.LandPointNear(at, 6, 10), "the director's microphone", "Look at the microphone", "The director has two minutes on the open channel, if somebody hands over the microphone.");
				scene.Choice(mic, "Hand the director the microphone").Sets("world:proof.read_air").Says("Two minutes, read slowly, on every channel that's listening.", "gave the director two minutes on air");
				break;
			}
			case RZ_Major_Proof.S_PAPER:
			{
				RZ_Leader chair = RZ_MajorHelp.Top(gm, RZ_EFaction.SURVIVORS);
				if (!chair)
					break;

				int supper = scene.Person(RZ_MajorHelp.NearSeat(chair.m_vPos), "the chair at supper", "Talk to the chair", "\"Read us the author list. All of it. The ones from Day 6 first.\"");
				scene.Choice(supper, "Read the author list at supper").Needs(RZ_EFaction.SURVIVORS, 1).Sets("world:proof.read_supper").Says("The Day 6 names first, then the rest. The table is quiet for a long time.", "read the author list at supper");
				int ham = scene.Person(RZ_MajorHelp.NorthRoad(gm, 200), "the radio ham on the verge", "Talk to the radio ham", "An aerial between two trees and an exercise book open on a knee.");
				scene.Choice(ham, "Read the paper's first page to the ham").Sets("world:proof.ham").Says("\"Got it.\" The ham writes it all down, and reads it back to check.", "read the first page to the radio ham");
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The new approaches' effects at a major's ending (RZ_Majors.SeasonStory).
	static void AtEnding(RZ_GameModeComponent gm, string major, int ending, array<string> deciders)
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (facts.Flag("majors.rest_end." + major) != 0)
			return;

		facts.SetFlag("majors.rest_end." + major, 1);
		if (major == "book")
		{
			if (facts.Flag("book.bill_carried") != 0 && deciders)
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.INSTITUTE, 10, 0);

			if (facts.Flag("book.lifted") != 0 && deciders)
				RZ_MajorHelp.PayIdents(gm, deciders, RZ_EFaction.BANDITS, 0, 25);
		}
		else if (major == "orders")
		{
			if (facts.Flag("orders.here_count") != 0)
				RZ_MajorHelp.SetWorld(gm, "orders.count", RZ_MajorHelp.World(gm, "orders.count") + 1);
		}
		else if (major == "proof")
		{
			if (facts.Flag("proof.read_air") != 0 && ending == 2)
			{
				array<string> lines = {"The director, on every open channel. This is the first page of the paper.", "Three weeks before Day 1, the clinic had the first patient. The lab had the clinic's file.", "We have known since Day 3 what the lab did with them. This is the whole of it."};
				OpenChannel(RZ_EFaction.INSTITUTE, lines, 40);
			}
		}
	}
}
