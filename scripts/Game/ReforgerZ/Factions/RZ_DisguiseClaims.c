//! Borrowed kit's rest (docs/REPUTATION.md, "Borrowed kit"): what a claim says and what the look adds
//! after disguise phases 1 to 3.
//! - Cover details spoken as choices (`cover:<key>` in conversations: name, dead, crew, notebook,
//!   paper): one you have counts at the next look with that faction (10 minutes), naming the very
//!   detail; one you don't have is a lie, caught at once while you're in their kit. One claim line
//!   per speaker per life (`claimed`).
//! - The look's last minuses: something of theirs from a mission against them on you (-1); after the
//!   90% lock, every look by a faction whose route you didn't choose (-1), until the cure.
//! - The stand's spare rule: a fight zone that spares friends also spares a player locked on that
//!   faction's route who wears its colours.
//! - The HUD's look-coming line (`RZ_Hud_Look`, owner-only through RZ_UIFeed, sent only when it
//!   changes), and the standing page's kit line per faction.
//! - The Haven's gossip about impostors, sideways and about food.
//! - The majors' tricks, as hooks: Await Relief's roster read in Bastion kit at night, Proof's
//!   records room in a Halcyon coat at night.
//! - The last week: a dead man's kit carried home is taken in, signed-for kit handed back at its
//!   buyer's quartermaster, the factions' lines from the season's counts and the hall's two lines.
//! Server only.

//------------------------------------------------------------------------------------------------
class RZ_DisguiseClaims
{
	static const string KEY_NAME = "name";			// a leader of theirs you've met this life
	static const string KEY_DEAD = "dead";			// a dead man's name heard on Bastion's nightly call
	static const string KEY_CREW = "crew";			// your crew name (the Red Hand)
	static const string KEY_NOTEBOOK = "notebook";	// a dead researcher's notebook number (Halcyon)
	static const string KEY_PAPER = "paper";		// a mission's paper (SetPaper)

	static const string FLAG_WEEK = "disguise.week_told";	// the last week's lines were told
	static const string FLAG_ROSTER = "disguise.roster_name";	// the dead man's name signed on Bastion's roster (its nail id + 1)

	protected static const float SPOKEN_SECONDS = 600;	// a spoken detail counts at looks this long
	protected static const float HUD_RADIUS = 200;		// m from where the look was shown: the line clears beyond
	protected static const float HOME_RADIUS = 40;		// m from his post: a dead man's kit is taken in
	protected static const float QM_RADIUS = 8;			// m from the buyer's quartermaster: signed kit handed back
	protected static const float GOSSIP_GAP = 21600;	// s between two of the Haven's impostor stories

	protected RZ_GameModeComponent m_GM;
	protected RZ_Disguise m_Disguise;
	protected ref map<string, string> m_mSpoken = new map<string, string>();	// "ident|f" -> key spoken
	protected ref map<string, float> m_mSpokenAt = new map<string, float>();
	protected ref map<string, string> m_mPaper = new map<string, string>();	// "ident|f" -> a mission's paper
	protected ref array<string> m_aClaimed = {};		// "ident|life|speaker": a claim line spoken
	protected ref map<int, string> m_mHud = new map<int, string>();			// playerId -> look line shown
	protected ref map<int, vector> m_mHudAt = new map<int, vector>();
	protected float m_fClock;
	protected float m_fGossipAt = -100000;
	protected float m_fWeekTimer;

	//------------------------------------------------------------------------------------------------
	void RZ_DisguiseClaims(RZ_GameModeComponent gm, RZ_Disguise disguise)
	{
		m_GM = gm;
		m_Disguise = disguise;
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Key(string ident, int f)
	{
		return ident + "|" + f.ToString();
	}

	// ================================================================ Cover details spoken
	//------------------------------------------------------------------------------------------------
	//! The cover detail this player could say to `f` for `key`, as the look writes it ("Name: Kral",
	//! "Crew name: Magpie", "Notebook 31", a paper), "" if they have none.
	string CoverText(int playerId, int f, string key)
	{
		switch (key)
		{
			case KEY_NAME:
			{
				RZ_Leaders leaders = m_GM.GetLeaders();
				if (!leaders)
					return "";

				array<RZ_Leader> met = {};
				leaders.ListMet(playerId, met);
				foreach (RZ_Leader l : met)
				{
					if (l.m_eFaction == f)
						return "Name: " + l.m_sShort;
				}

				return "";
			}
			case KEY_DEAD:
			{
				string dead = m_Disguise.GetWatch().HeardName(playerId);
				if (f != RZ_EFaction.MILITARY || dead.IsEmpty())
					return "";

				return "Name: " + RZ_DisguiseMarks.Surname(dead);
			}
			case KEY_CREW:
			{
				string crew = m_GM.GetProfile(playerId).m_sCrewName;
				if (f != RZ_EFaction.BANDITS || crew.IsEmpty())
					return "";

				return "Crew name: " + crew;
			}
			case KEY_NOTEBOOK:
			{
				int n = m_Disguise.NotebookOf(playerId);
				if (f != RZ_EFaction.INSTITUTE || n <= 0)
					return "";

				return "Notebook " + n.ToString();
			}
			case KEY_PAPER:
			{
				string paper;
				m_mPaper.Find(Key(Ident(playerId), f), paper);
				return paper;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The detail as spoken in a line: "Kral", "Magpie", "31", the paper's own words.
	string SpokenWords(int playerId, int f, string key)
	{
		string text = CoverText(playerId, f, key);
		int colon = text.IndexOf(": ");
		if (colon >= 0)
			return text.Substring(colon + 2, text.Length() - colon - 2);

		if (text.IndexOf("Notebook ") == 0)
			return text.Substring(9, text.Length() - 9);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A mission stage hands this player a paper for `f`'s look (the cover detail a stage provides).
	void SetPaper(int playerId, int f, string paper)
	{
		if (RZ_Factions.IsValid(f))
			m_mPaper.Set(Key(Ident(playerId), f), paper);
	}

	//------------------------------------------------------------------------------------------------
	//! cover:<key> said to `f`'s speaker: a detail you have counts at the next look (10 minutes) and
	//! names that very detail; one you don't have is a lie, caught at once while you're in their kit.
	//! Either way it's this speaker's one claim line this life. True: it was true.
	bool Say(int playerId, int f, string key, string speaker, IEntity user)
	{
		MarkClaimed(playerId, speaker);
		string text = CoverText(playerId, f, key);
		if (!text.IsEmpty())
		{
			string k = Key(Ident(playerId), f);
			m_mSpoken.Set(k, key);
			m_mSpokenAt.Set(k, m_fClock);
			RZ_Log.Info(string.Format("Disguise: %1 gave %2 a cover detail (%3)", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), key));
			return true;
		}

		RZ_Log.Info(string.Format("Disguise: %1 gave %2 a cover detail they don't have (%3): a lie", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), key));
		if (user && m_Disguise.InTheirKit(playerId, f))
			m_Disguise.Caught(playerId, f, user, LieLine(f, key));

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string LieLine(int f, string key)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "Never heard of him. Hands on the barrier.";
			case RZ_EFaction.BANDITS: return "Nobody calls you that. Nobody ever has.";
			case RZ_EFaction.INSTITUTE: return "There's no notebook with that number. Security, please.";
		}

		return "No. Try that on someone else.";
	}

	//------------------------------------------------------------------------------------------------
	//! The detail spoken to `f` in the last 10 minutes, as the look writes it, "" none.
	string SpokenDetail(int playerId, int f)
	{
		string k = Key(Ident(playerId), f);
		string key;
		float at;
		if (!m_mSpoken.Find(k, key) || !m_mSpokenAt.Find(k, at) || m_fClock - at > SPOKEN_SECONDS)
			return "";

		return CoverText(playerId, f, key);
	}

	//------------------------------------------------------------------------------------------------
	//! A claim line already spoken to this speaker in this life.
	bool ClaimedTo(int playerId, string speaker)
	{
		return m_aClaimed.Contains(ClaimKey(playerId, speaker));
	}

	//------------------------------------------------------------------------------------------------
	void MarkClaimed(int playerId, string speaker)
	{
		string k = ClaimKey(playerId, speaker);
		if (!speaker.IsEmpty() && !m_aClaimed.Contains(k))
			m_aClaimed.Insert(k);

		while (m_aClaimed.Count() > 400)
		{
			m_aClaimed.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string ClaimKey(int playerId, string speaker)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		return string.Format("%1|%2|%3", p.m_sIdentity, p.m_iLives, speaker);
	}

	// ================================================================ The look's last minuses
	//------------------------------------------------------------------------------------------------
	//! Added to the look (RZ_Disguise.MoreTags): something of theirs from a mission against them on
	//! you, and after the 90% lock a faction whose route you didn't choose. The score change.
	int MoreTags(int playerId, int f, IEntity me, out string tags)
	{
		tags = "";
		int score = 0;
		if (CarriesTheirs(f, me))
		{
			score--;
			tags += " [Something of theirs on you -1]";
		}

		if (OtherRoute(playerId, f))
		{
			score--;
			tags += " [Sworn to another side -1]";
		}

		return score;
	}

	//------------------------------------------------------------------------------------------------
	//! A mission item from a job against `f` (its rival) is in this character's inventory.
	protected bool CarriesTheirs(int f, IEntity me)
	{
		RZ_MissionManager mm = m_GM.GetMissions();
		if (!me || !mm)
			return false;

		array<IEntity> items = {};
		RZ_MissionItemHelper.FindItems(me, items);
		foreach (IEntity item : items)
		{
			RZ_MissionItemComponent comp = RZ_MissionItemComponent.Cast(item.FindComponent(RZ_MissionItemComponent));
			if (!comp)
				continue;

			RZ_Mission m = mm.FindById(comp.GetMissionId());
			if (m && m.m_bHasRival && m.m_eRival == f && (!m.m_bHasGiver || m.m_eGiver != f))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Locked on a route at the 90% lock, and `f` isn't that route's faction, until the cure.
	bool OtherRoute(int playerId, int f)
	{
		RZ_CureFinale fin = m_GM.GetFinale();
		if (!fin || !RZ_CureFinale.IsOn() || Cured())
			return false;

		int r = fin.RouteOfPlayer(playerId);
		if (r < 0)
			return false;

		return RZ_CureFinale.RouteFaction(r) != f;
	}

	//------------------------------------------------------------------------------------------------
	//! The stand's spare rule (RZ_Hostility's fight zones that spare friends): a player locked on
	//! `f`'s route, wearing `f`'s colours, is spared like a friend.
	bool SpareInColours(int playerId, int f)
	{
		RZ_CureFinale fin = m_GM.GetFinale();
		if (!fin || !RZ_CureFinale.IsOn() || !RZ_Factions.IsValid(f))
			return false;

		int r = fin.RouteOfPlayer(playerId);
		return r >= 0 && RZ_CureFinale.RouteFaction(r) == f && m_Disguise.KitLevel(playerId, f) > 0;
	}

	//------------------------------------------------------------------------------------------------
	protected bool Cured()
	{
		return m_GM.GetCure() && m_GM.GetCure().IsCured();
	}

	// ================================================================ The HUD and the standing page
	//------------------------------------------------------------------------------------------------
	//! The look coming (RZ_Disguise.ShowComingNow): the HUD's line, "PASS AS BASTION: 2 OF 2", held
	//! until the player is 200 m from here, out of the kit, passed or caught.
	void ShowLook(int playerId, int f, int score, int difficulty, bool instant)
	{
		string side = RZ_Factions.GetName(f);
		side.ToUpper();
		string line = string.Format("PASS AS %1: %2 OF %3", side, Math.Max(0, score), difficulty);
		if (instant)
			line = string.Format("PASS AS %1: SEEN THROUGH", side);

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (me)
			m_mHudAt.Set(playerId, me.GetOrigin());

		SendHud(playerId, line);
	}

	//------------------------------------------------------------------------------------------------
	//! The look is over (passed, caught, walked off): the HUD's line goes.
	void ClearLook(int playerId)
	{
		SendHud(playerId, "");
		m_mHudAt.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void SendHud(int playerId, string line)
	{
		string had;
		m_mHud.Find(playerId, had);
		if (had == line)
			return;

		m_mHud.Set(playerId, line);
		RZ_UIFeed feed = m_GM.GetUIFeed();
		if (feed)
			feed.PushLook(playerId, line);
	}

	//------------------------------------------------------------------------------------------------
	//! The standing page's kit line for `f` ("" for the Haven and when disguises are off): your face,
	//! catches this life, blown time left, their tells, what their enemies do to their colours.
	string StandingLine(int playerId, int f)
	{
		if (!m_Disguise.IsOn() || f == RZ_EFaction.SURVIVORS || !RZ_Factions.IsValid(f))
			return "";

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		string face = "They don't know your face.";
		if (m_Disguise.KnowsFace(playerId, f))
			face = "They know your face.";

		string text = face;
		int catches = p.m_aCatches[f];
		if (catches > 0)
			text += string.Format(" Caught %1 of 3 this life.", catches);

		if (m_Disguise.IsBlown(playerId, f))
		{
			int left = RZ_Util.RoundToInt((p.m_aBlownUntil[f] - p.m_fLifeSeconds) / 60);
			text += string.Format(" Blown for %1 more minutes.", Math.Max(1, left));
		}

		return text + " " + Tells(f) + " " + Enemies(f);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Tells(int f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "Their tells: another side's colours, a weapon that isn't a service one.";
			case RZ_EFaction.BANDITS: return "Their tells: another side's colours, an armband they gave someone else.";
			case RZ_EFaction.INSTITUTE: return "Their tells: another side's colours, any long gun, a bite that shows.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string Enemies(int f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "In their green, the Red Hand's shops turn you away and a feuding crew opens up.";
			case RZ_EFaction.BANDITS: return "In their red, Bastion's shops and the Haven's people turn you away.";
			case RZ_EFaction.INSTITUTE: return "In their coat, the Red Hand's toll doubles.";
		}

		return "";
	}

	// ================================================================ Word gets round: the Haven
	//------------------------------------------------------------------------------------------------
	//! A catch (RZ_Disguise.CaughtAt): the Haven tells the story sideways, about food, once in six
	//! hours of play, with the nearest town.
	void OnCaught(int playerId, int f, IEntity me)
	{
		ClearLook(playerId);
		if (!me || m_fClock - m_fGossipAt < GOSSIP_GAP || !RZ_WorldFacts.Get())
			return;

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(me.GetOrigin()));
		if (!s)
			return;

		string line = "";
		switch (f)
		{
			case RZ_EFaction.MILITARY: line = "Somebody came to %1 in a soldier's coat asking for bread. We gave him bread. The coat didn't fit."; break;
			case RZ_EFaction.BANDITS: line = "A lad came to %1 with a red rag round his arm asking for soup. We gave him soup. He kept the rag on while he ate it."; break;
			case RZ_EFaction.INSTITUTE: line = "Somebody in one of those white coats came to %1 asking for tea. We gave her tea. She held the cup like she'd never had to wash one."; break;
		}

		if (line.IsEmpty())
			return;

		m_fGossipAt = m_fClock;
		RZ_WorldFacts.Get().AddRumour(string.Format(line, s.m_sName), 12);
	}

	// ================================================================ The majors' tricks (hooks)
	//------------------------------------------------------------------------------------------------
	//! At night (20:30-05:00, or fog): the majors' tricks run only then.
	static bool Night()
	{
		float h = RZ_GameTime.GetHour();
		return h >= 20.5 || (h >= 0 && h < 5);
	}

	//------------------------------------------------------------------------------------------------
	//! Await Relief's Act IV, the roster read in Bastion kit at night: look 1 is the gate sentry
	//! (difficulty 2), look 2 the duty board (3), with a name off the nail as the cover detail (the
	//! dead man's name heard on the nightly call). Passing the board signs the roster as him, for the
	//! muster's callback (MusterCallback). False: not night, or caught (the catch is the built one).
	bool RosterTrick(int playerId, int look, string place)
	{
		if (!Night() || !m_Disguise.InTheirKit(playerId, RZ_EFaction.MILITARY))
			return false;

		int difficulty = 2;
		if (look >= 2)
			difficulty = 3;

		string paper = "";
		string dead = m_Disguise.GetWatch().HeardName(playerId);
		if (!dead.IsEmpty())
			paper = "Name: " + RZ_DisguiseMarks.Surname(dead);

		bool pass = m_Disguise.Look(playerId, RZ_EFaction.MILITARY, difficulty, place, paper, 600);
		if (pass && look >= 2 && !dead.IsEmpty() && RZ_WorldFacts.Get())
		{
			int nail = m_Disguise.GetMarks().NailIdOf(RZ_EFaction.MILITARY, dead);
			if (nail >= 0)
				RZ_WorldFacts.Get().SetFlag(FLAG_ROSTER, nail + 1);
		}

		return pass;
	}

	//------------------------------------------------------------------------------------------------
	//! The muster's callback for a roster signed as a dead man: "And whoever signed the roster as
	//! Kral: he's still dead." "" when nobody did.
	string MusterCallback()
	{
		if (!RZ_WorldFacts.Get())
			return "";

		int id = RZ_WorldFacts.Get().Flag(FLAG_ROSTER, 0) - 1;
		RZ_NailSave nail = m_Disguise.GetMarks().FindNail(id);
		if (!nail)
			return "";

		return string.Format("And whoever signed the roster as %1: he's still dead.", RZ_DisguiseMarks.Surname(nail.name));
	}

	//------------------------------------------------------------------------------------------------
	//! Proof's records room in a Halcyon coat at night: the duty officer's look at difficulty 3, a
	//! real notebook number as the cover detail when you carry one. False: not night, or caught.
	bool RecordsTrick(int playerId, string place)
	{
		if (!Night() || !m_Disguise.InTheirKit(playerId, RZ_EFaction.INSTITUTE))
			return false;

		return m_Disguise.Look(playerId, RZ_EFaction.INSTITUTE, 3, place, "", 600);
	}

	// ================================================================ The tick and the last week
	//------------------------------------------------------------------------------------------------
	//! On the disguise tick: the HUD's line clears out of the kit or 200 m on; after the cure, the
	//! last week's pieces (on the kit read).
	void Tick(float dt, array<int> ids, array<IEntity> players, bool kitRead)
	{
		m_fClock += dt;
		if (!kitRead)
			return;

		foreach (int i, IEntity p : players)
		{
			int pid = ids[i];
			vector at;
			if (m_mHudAt.Find(pid, at) && (m_Disguise.Wearing(pid) < 0 || RZ_Util.DistXZ(at, p.GetOrigin()) > HUD_RADIUS))
				ClearLook(pid);
		}

		if (!Cured())
			return;

		WeekLines();
		foreach (int j, IEntity q : players)
		{
			Homecoming(ids[j], q);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The last week: a dead man's kit carried to within 40 m of his post is taken in (his name read
	//! at the roll, into the record, or in the book as "sent off"); kit off the turned taken back at a
	//! Bastion holding; signed-for kit worn by someone else handed back at its buyer's quartermaster
	//! (no catch, the buyer told).
	protected void Homecoming(int playerId, IEntity me)
	{
		if (!m_Disguise.MarksOn())
			return;

		RZ_DisguiseMarks marks = m_Disguise.GetMarks();
		RZ_Territory territory = m_GM.GetTerritory();
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			array<IEntity> pieces = {};
			m_Disguise.WornPieces(me, f, pieces);
			foreach (IEntity piece : pieces)
			{
				RZ_KitMark m = marks.Find(piece);
				if (!m)
					continue;

				if (m.m_iKind == RZ_DisguiseMarks.DEAD && territory)
				{
					RZ_NailSave nail = marks.FindNail(m.m_iNail);
					if (!nail || nail.holding < 0)
						continue;

					RZ_Holding h = territory.GetHolding(nail.holding);
					if (!h || RZ_Util.DistXZ(h.m_vPos, me.GetOrigin()) > HOME_RADIUS)
						continue;

					marks.TakeHome(piece, nail);
					m_GM.Say(playerId, RZ_Factions.GetName(f), f, HomeLine(f, nail.name));
					continue;
				}

				if (m.m_iKind == RZ_DisguiseMarks.TURNED && f == RZ_EFaction.MILITARY && territory)
				{
					RZ_Holding post = territory.FindHolding(RZ_EFaction.MILITARY, me.GetOrigin(), HOME_RADIUS);
					if (!post)
						continue;

					marks.TakeHome(piece, null);
					m_GM.Say(playerId, RZ_Factions.GetName(f), f, "Off one of ours, then. He's home. Mark him.");
					continue;
				}

				if (m.m_iKind == RZ_DisguiseMarks.SIGNED && m.m_sOwner != Ident(playerId))
				{
					vector qm;
					if (!m_GM.GetPerks() || !m_GM.GetPerks().GetQuartermasterPos(f, qm) || RZ_Util.DistXZ(qm, me.GetOrigin()) > QM_RADIUS)
						continue;

					string owner = m.m_sOwner;
					marks.TakeHome(piece, null);
					m_GM.Say(playerId, RZ_Factions.GetName(f), f, "Back on the shelf. No questions this week.");
					int buyerId = OnlineId(owner);
					if (buyerId > 0)
						m_GM.Say(buyerId, RZ_Factions.GetName(f), f, "Your jacket came home. Clean, mostly.");
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static string HomeLine(int f, string name)
	{
		string surname = RZ_DisguiseMarks.Surname(name);
		switch (f)
		{
			case RZ_EFaction.MILITARY: return string.Format("\"He's home. Mark him.\" %1's tag goes on the nail, and they read his name at the roll.", surname);
			case RZ_EFaction.BANDITS: return string.Format("They write %1's line in the book: sent off.", surname);
			case RZ_EFaction.INSTITUTE: return string.Format("They read %1's notebook into the record, and add him to the authors.", surname);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Once, at the first kit read after the cure: each faction's line from the season's counts, and
	//! the hall's two lines ("Caught in someone else's kit", a count with no names, and "Never caught",
	//! the most looks passed without a catch, an honour line: frozen, filtered, anonymous kept so).
	protected void WeekLines()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (!facts || facts.Flag(FLAG_WEEK, 0) == 1)
			return;

		facts.SetFlag(FLAG_WEEK, 1);
		RZ_DisguiseSave d = m_Disguise.GetMarks().Data();
		RZ_FactionLife life = m_GM.GetFactionLife();
		int bastion = 0;
		int caughtAll = 0;
		if (d.catches)
		{
			foreach (int fi, int c : d.catches)
			{
				caughtAll += c;
				if (fi == RZ_EFaction.MILITARY)
					bastion = c;
			}
		}

		string best = BestDressed(d);
		if (life)
		{
			if (bastion > 0)
			{
				string jackets = RZ_WorldFacts.NumberWords(bastion);
				jackets = RZ_Tribes.Capitalize(jackets);
				life.OpenChannelLine(RZ_EFaction.MILITARY, string.Format("%1 jackets on the wire this season. None of them fit anyone.", jackets));
			}

			if (!best.IsEmpty())
				life.OpenChannelLine(RZ_EFaction.BANDITS, string.Format("Best-dressed of the season: %1. Drinks are on Bastion.", best));

			if (d.coatsOut > 0)
				life.OpenChannelLine(RZ_EFaction.INSTITUTE, string.Format("Coats returned: %1 of %2. The rest are listed as authors.", d.coatsBack, d.coatsOut));

			life.OpenChannelLine(RZ_EFaction.SURVIVORS, "The ribbon only ever went on the people it was given to. Mostly.");
		}

		RZ_HallOfFame hall = m_GM.GetHall();
		if (hall)
		{
			hall.AddWeekLine(string.Format("Caught in someone else's kit: %1 times.", caughtAll));
			if (!best.IsEmpty())
				hall.AddWeekLine("Never caught: " + best + ".");
		}

		RZ_Log.Info(string.Format("Disguise: the last week's lines (%1 caught this season, best-dressed %2)", caughtAll, best));
	}

	//------------------------------------------------------------------------------------------------
	//! The player online with this identity, 0 none.
	protected int OnlineId(string ident)
	{
		if (ident.IsEmpty())
			return 0;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			RZ_PlayerProfile p = m_GM.FindProfile(id);
			if (p && p.m_sIdentity == ident)
				return id;
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	//! "Mara, through 9 looks in other people's kit": the most looks passed by someone never caught
	//! this season (at least 3), "somebody" when anonymous or filtered, "" none.
	protected string BestDressed(RZ_DisguiseSave d)
	{
		if (!d.lookIdents || !d.lookCounts)
			return "";

		string ident = "";
		int most = 2;
		foreach (int i, string who : d.lookIdents)
		{
			if (i >= d.lookCounts.Count() || d.lookCounts[i] <= most || (d.caughtIdents && d.caughtIdents.Contains(who)))
				continue;

			most = d.lookCounts[i];
			ident = who;
		}

		if (ident.IsEmpty())
			return "";

		string name = "somebody";
		RZ_HallOfFame hall = m_GM.GetHall();
		if (hall && !hall.IsAnonymous(ident))
		{
			string shown = hall.NameOf(ident);
			int online = OnlineId(ident);
			if (shown.IsEmpty() && online > 0)
				shown = RZ_Util.PlayerName(online);

			if (!shown.IsEmpty() && RZ_EpiNames.Filtered(shown) == shown)
				name = shown;
		}

		return string.Format("%1, through %2 looks in other people's kit and never caught", name, most);
	}
}
