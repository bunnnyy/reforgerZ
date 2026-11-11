//! The people's side of the tribes (docs/INFECTED.md, "People"): each faction's words for them, always
//! with the place; their voice at dusk; marks where a road enters a range; what each character knows
//! (heard of, read, mapped) and the ways to learn it; skills by background; your dead run (the name
//! setting, the opt-out, the name filter, the ID card); the dark corners (the fat moat, the Long
//! Table, walk-outs, good condition); the moat rule on the territory tick; the cure's milestones,
//! the call box, the final stand's waves; frenzies and Furious tribes; and the last week after the
//! cure. Owned by RZ_Tribes. Server only.

//------------------------------------------------------------------------------------------------
//! What a character knows of a tribe (docs/INFECTED.md, Knowing). Per character, lost with the life.
enum RZ_EKnow
{
	NONE,
	HEARD,		// its place and the name heard, a rough area (500 m)
	READ,		// its range, and its mood while you're near
	MAPPED		// lair, leader, size, temperament and habits, as of that day
}

//------------------------------------------------------------------------------------------------
//! One character's knowledge: parallel lists by tribe.
class RZ_TribeKnowing
{
	string m_sIdent;
	int m_iLife;
	ref array<int> m_aTribe = {};
	ref array<int> m_aLevel = {};
	ref array<int> m_aHeard = {};	// bits: the speakers whose word they've heard (RZ_EFaction, and 4 for the Long Table)
	ref array<int> m_aDay = {};		// the season day of the level

	//------------------------------------------------------------------------------------------------
	int IndexOf(int tribe)
	{
		return m_aTribe.Find(tribe);
	}
}

//------------------------------------------------------------------------------------------------
//! The people's side in state.json.
class RZ_TribePeopleSave
{
	ref array<string> knowIdent = {};
	ref array<int> knowLife = {};
	ref array<int> knowOwner = {};	// index into knowIdent for each row below
	ref array<int> knowTribe = {};
	ref array<int> knowLevel = {};
	ref array<int> knowHeard = {};
	ref array<int> knowDay = {};
	ref array<string> hidden = {};
	ref array<string> deliveredIdent = {};
	ref array<int> deliveredDay = {};
	ref array<string> callBoxIdent = {};
	ref array<int> callBoxAt = {};
	ref array<string> runsIdent = {};
	ref array<string> runsText = {};
	ref array<string> beats = {};
	ref array<int> moatHolding = {};
	ref array<int> moatCount = {};
	int nextContact;
	int furiousOrder;
	int cureUnix;
	int lastWeekAt;
	bool lastCall;
	int lastFrenzyDay = -100;
	int lastRedDuskDay = -100;
	int pitStock;
	string pitCrew;
	string pitPrefab;
}

//------------------------------------------------------------------------------------------------
class RZ_TribePeople
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Tribes m_Tribes;
	protected ref map<string, ref RZ_TribeKnowing> m_mKnow = new map<string, ref RZ_TribeKnowing>();
	protected ref array<string> m_aHidden = {};			// identities who opted out of being named
	protected ref map<string, int> m_mDelivered = new map<string, int>();	// Bring Them Home: identity -> season day
	protected ref map<string, int> m_mCallBox = new map<string, int>();		// identity -> real time of their last call box
	protected ref map<string, string> m_mRunsWith = new map<string, string>();	// identity -> "Runs with ... now."
	protected ref array<string> m_aBeats = {};			// what the infected hand the epilogue
	protected ref map<int, int> m_mMoatBlocks = new map<int, int>();	// holding -> takes the moat has held up
	protected ref map<string, int> m_mMarkRead = new map<string, int>();	// "identity:tribe:faction" -> the mark's day read
	protected ref array<string> m_aFilter = {};
	protected int m_iNextContact = 1;
	protected int m_iFuriousOrder;
	protected int m_iCureUnix;
	protected int m_iLastWeekAt;
	protected bool m_bLastCall;
	protected bool m_bLastCallDue;
	protected int m_iLastFrenzyDay = -100;
	protected int m_iLastRedDuskDay = -100;
	protected int m_iPitStock;				// Stock the Pit: young caged for the next Fight Pit
	protected string m_sPitCrew;
	protected ResourceName m_sPitPrefab;
	protected float m_fMarkTimer;
	protected float m_fVoiceDay = -1;
	protected int m_iVoiceDay = -1;
	protected bool m_bDuskDone;
	protected float m_fStandCarry;
	protected int m_iSpawnChecked;
	protected ref array<vector> m_aKeepOff = {};		// HC1's store: the site pack's bodies keep 150 m off
	protected ref array<int> m_aKeepOffMission = {};
	protected ref array<int> m_aKeepOffTribe = {};

	static const float MARK_READ = 25;			// a mark is read from this close
	static const float LAB_DATA = 4000;			// Halcyon reads the ranges within this of the lab
	static const float BASTION_READS = 3000;	// Bastion reads contacts within this of its holdings
	static const int SPEAKER_TABLE = 4;			// the Long Table's bit in RZ_TribeKnowing.m_aHeard
	static const int HOUR = 3600;

	//------------------------------------------------------------------------------------------------
	void RZ_TribePeople(RZ_GameModeComponent gm, RZ_Tribes tribes)
	{
		m_GM = gm;
		m_Tribes = tribes;
		RZ_TribeSettings s = tribes.GetSettings();
		if (s)
			RZ_ItemHelper.Words(s.m_sNameFilter, m_aFilter);
	}

	//------------------------------------------------------------------------------------------------
	bool IsOn()
	{
		RZ_TribeSettings s = m_Tribes.GetSettings();
		return m_Tribes.IsEnabled() && s && s.m_bPeople;
	}

	// ================================================================ The words
	//------------------------------------------------------------------------------------------------
	//! What a speaker calls a tribe (never "alpha", "tribe" or "crawler"): the Haven "the Quarry lot",
	//! Bastion "Contact Papa-two", the Red Hand a crew's nickname, Halcyon "Pack P-02", the Long Table
	//! "the neighbours". faction: RZ_EFaction, or SPEAKER_TABLE.
	string Name(RZ_Tribe t, int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (HavenKnowsLeader(t))
					return string.Format("%1's lot", FirstName(t.m_sAlphaName));

				return string.Format("the %1 lot", RZ_Tribes.Capitalize(KindWord(t)));
			}
			case RZ_EFaction.MILITARY:
			{
				if (t.m_iContact <= 0)
					return "an unlisted contact";

				return "Contact Papa-" + RZ_Tribes.NumberWord(t.m_iContact);
			}
			case RZ_EFaction.BANDITS:
				return CrewName(t);
			case RZ_EFaction.INSTITUTE:
				return "Pack " + PackCode(t);
			case SPEAKER_TABLE:
				return "the neighbours";
		}

		return t.m_sPlace;
	}

	//------------------------------------------------------------------------------------------------
	//! A speaker's name with the place, so four names are one pack: "the Quarry lot, up at the quarry
	//! by Morton", "Contact Papa-two, the quarry by Morton", "the Postmen up the quarry by Morton",
	//! "Pack P-02 (the quarry by Morton)".
	string NameAt(RZ_Tribe t, int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("%1, up at %2", Name(t, faction), t.m_sPlace);
			case RZ_EFaction.MILITARY: return string.Format("%1, %2", Name(t, faction), t.m_sPlace);
			case RZ_EFaction.BANDITS: return string.Format("%1 up %2", Name(t, faction), t.m_sPlace);
			case RZ_EFaction.INSTITUTE: return string.Format("%1 (%2)", Name(t, faction), t.m_sPlace);
		}

		return string.Format("%1, at %2", Name(t, faction), t.m_sPlace);
	}

	//------------------------------------------------------------------------------------------------
	//! A speaker's word for the leader: the Haven's name when it knew them, else "the big one";
	//! Bastion "the heavy"; the Red Hand "the boss"; Halcyon "the index".
	string LeaderWord(RZ_Tribe t, int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (HavenKnowsLeader(t))
					return t.m_sAlphaName;

				return "the big one";
			}
			case RZ_EFaction.MILITARY: return "the heavy";
			case RZ_EFaction.BANDITS: return "the boss";
			case RZ_EFaction.INSTITUTE: return "the index";
		}

		return "the big one";
	}

	//------------------------------------------------------------------------------------------------
	//! The world's pack tokens (RZ_WorldFacts.Fill): {tribe:N} is pack N in the speaker's word with the
	//! place, {alpha:N} its leader in the speaker's word (the Haven's name when it knew them), {young:N}
	//! its newest named young (described where a name isn't allowed), else the speaker's word for the
	//! young. The speaker follows a dot (haven, bastion, redhand, halcyon, table): {tribe:3.bastion};
	//! the Haven's words without one. False if there's no such pack (the token is left alone).
	bool TokenText(string arg, int what, out string value)
	{
		string idText = arg;
		string who = "";
		int dot = arg.IndexOf(".");
		if (dot > 0)
		{
			idText = arg.Substring(0, dot);
			who = arg.Substring(dot + 1, arg.Length() - dot - 1);
		}

		RZ_Tribe t = m_Tribes.Find(idText.ToInt(-1));
		if (!t || t.IsRover())
			return false;

		int f = SpeakerFrom(who);
		switch (what)
		{
			case 0:
			{
				value = NameAt(t, f);
				return true;
			}
			case 1:
			{
				if (!t.m_bHasAlpha)
					return false;

				value = LeaderWord(t, f);
				return true;
			}
			case 2:
			{
				if (t.m_aNamed.IsEmpty())
					value = YoungWord(f);
				else
					value = PublicNameOf(t.m_aNamed[t.m_aNamed.Count() - 1]);

				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A token's speaker: "bastion", "redhand", "halcyon", "table"; anything else is the Haven.
	static int SpeakerFrom(string who)
	{
		switch (who)
		{
			case "bastion": return RZ_EFaction.MILITARY;
			case "redhand": return RZ_EFaction.BANDITS;
			case "halcyon": return RZ_EFaction.INSTITUTE;
			case "table": return SPEAKER_TABLE;
		}

		return RZ_EFaction.SURVIVORS;
	}

	//------------------------------------------------------------------------------------------------
	//! A speaker's word for the young, the towns' slow ones and a lone one (docs/LORE.md).
	static string YoungWord(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "new contacts";
			case RZ_EFaction.BANDITS: return "new hires";
			case RZ_EFaction.INSTITUTE: return "recent conversions";
		}

		return "the new ones";
	}

	//------------------------------------------------------------------------------------------------
	static string SlowWord(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "statics";
			case RZ_EFaction.BANDITS: return "the furniture";
			case RZ_EFaction.INSTITUTE: return "sessile cases";
		}

		return "stay-at-homes";
	}

	//------------------------------------------------------------------------------------------------
	static string LoneWord(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.MILITARY: return "a single";
			case RZ_EFaction.BANDITS: return "a boss who lost";
			case RZ_EFaction.INSTITUTE: return "an isolate";
		}

		return "the one on his own";
	}

	//------------------------------------------------------------------------------------------------
	//! The Haven knows who led it about one time in three: a walk-out, a runner, a farmer, a resident.
	bool HavenKnowsLeader(RZ_Tribe t)
	{
		if (!t.m_bHasAlpha || t.m_sAlphaName.IsEmpty())
			return false;

		if (t.m_eAlphaKind == RZ_EAlphaKind.SOLDIER || t.m_eAlphaKind == RZ_EAlphaKind.COLUMN)
			return false;

		return (t.m_iSeed + t.m_sAlphaName.Length()) % 3 == 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static string FirstName(string full)
	{
		array<string> parts = {};
		full.Split(" ", parts, true);
		if (parts.IsEmpty())
			return full;

		return parts[0];
	}

	//------------------------------------------------------------------------------------------------
	//! "Quarry", "Woods", "Barn": the lair's kind.
	protected static string KindWord(RZ_Tribe t)
	{
		if (t.m_sLairKind.IsEmpty() || t.m_sLairKind == "forest")
			return "woods";

		return t.m_sLairKind;
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand names a crew after something it would rather forget, the way the crews name people.
	string CrewName(RZ_Tribe t)
	{
		if (t.m_bColumn)
			return "the Commuters";

		if (t.Echo(RZ_EAlphaKind.SOLDIER))
			return "the Postmen";

		if (t.Echo(RZ_EAlphaKind.FARMER))
			return "the Farmhands";

		if (t.m_bRestless || t.m_eTemper == RZ_ETemperament.BOLD)
			return "the Choir";

		array<string> names = {"the Lodgers", "the Night Shift", "the Regulars", "the Tenants", "the Day Trippers", "the Committee", "the Late Crowd", "the Congregation"};
		return names[(t.m_iSeed + t.m_iId) % names.Count()];
	}

	//------------------------------------------------------------------------------------------------
	//! "P-02": Halcyon numbers packs in the order they formed.
	static string PackCode(RZ_Tribe t)
	{
		if (t.m_iId < 10)
			return "P-0" + t.m_iId.ToString();

		return "P-" + t.m_iId.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! "The quarry by Morton: the Quarry lot (Haven), Papa-two (Bastion), the Postmen (Red Hand),
	//! P-02 (Halcyon)." Only the names this character has heard.
	string NamesLine(RZ_Tribe t, int heard)
	{
		string text = RZ_Tribes.Capitalize(t.m_sPlace) + ":";
		string sep = " ";
		array<int> order = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE, SPEAKER_TABLE};
		array<string> who = {"Haven", "Bastion", "Red Hand", "Halcyon", "the farm"};
		foreach (int i, int f : order)
		{
			if ((heard & (1 << f)) == 0)
				continue;

			if (f == RZ_EFaction.MILITARY && t.m_iContact <= 0)
				continue;

			text += string.Format("%1%2 (%3)", sep, Name(t, f), who[i]);
			sep = ", ";
		}

		if (sep == " ")
			text += " no name for it yet";

		return text + ".";
	}

	// ================================================================ Knowing
	//------------------------------------------------------------------------------------------------
	protected RZ_TribeKnowing Knowing(int playerId, bool create)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (!p || p.m_sIdentity.IsEmpty())
			return null;

		RZ_TribeKnowing k;
		if (m_mKnow.Find(p.m_sIdentity, k))
		{
			if (k.m_iLife == p.m_iLives)
				return k;

			// A new character knows nothing
			k.m_iLife = p.m_iLives;
			k.m_aTribe.Clear();
			k.m_aLevel.Clear();
			k.m_aHeard.Clear();
			k.m_aDay.Clear();
			return k;
		}

		if (!create)
			return null;

		k = new RZ_TribeKnowing();
		k.m_sIdent = p.m_sIdentity;
		k.m_iLife = p.m_iLives;
		m_mKnow.Set(p.m_sIdentity, k);
		return k;
	}

	//------------------------------------------------------------------------------------------------
	//! What this character knows of the tribe (RZ_EKnow).
	int Level(int playerId, RZ_Tribe t)
	{
		RZ_TribeKnowing k = Knowing(playerId, false);
		if (!k || !t)
			return RZ_EKnow.NONE;

		int i = k.IndexOf(t.m_iId);
		if (i < 0)
			return RZ_EKnow.NONE;

		return k.m_aLevel[i];
	}

	//------------------------------------------------------------------------------------------------
	//! This character learns of a tribe: the level rises (never falls) and the speaker's word is
	//! kept under the place. speaker: RZ_EFaction, SPEAKER_TABLE, or -1 (nobody's word). True if
	//! the level rose.
	bool Learn(int playerId, RZ_Tribe t, int level, int speaker)
	{
		if (!IsOn() || !t || t.IsRover())
			return false;

		RZ_TribeKnowing k = Knowing(playerId, true);
		if (!k)
			return false;

		int i = k.IndexOf(t.m_iId);
		if (i < 0)
		{
			k.m_aTribe.Insert(t.m_iId);
			k.m_aLevel.Insert(RZ_EKnow.NONE);
			k.m_aHeard.Insert(0);
			k.m_aDay.Insert(RZ_SeasonClock.Day());
			i = k.m_aTribe.Count() - 1;
		}

		if (speaker >= 0)
			k.m_aHeard[i] = k.m_aHeard[i] | (1 << speaker);

		if (level <= k.m_aLevel[i])
			return false;

		k.m_aLevel[i] = level;
		k.m_aDay[i] = RZ_SeasonClock.Day();
		if (level == RZ_EKnow.MAPPED)
			m_GM.NotifyPlayer(playerId, "In your notebook", string.Format("%1: where it lies up, and what leads it, as of today.", RZ_Tribes.Capitalize(t.m_sPlace)));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone online within radius of pos learns it (a call heard, a mark read).
	void LearnNear(vector pos, float radius, RZ_Tribe t, int level, int speaker)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				Learn(ids[i], t, level, speaker);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The journal: one row per tribe this character knows of (RZ_UIFeed.BuildInfected).
	void JournalFor(int playerId, vector from, notnull array<int> tribes, notnull array<string> titles, notnull array<string> bys, notnull array<vector> wheres, notnull array<string> briefs)
	{
		RZ_TribeKnowing k = Knowing(playerId, false);
		RZ_PlayerProfile profile = m_GM.GetProfile(playerId);
		string runs;
		if (profile && m_mRunsWith.Find(profile.m_sIdentity, runs))
		{
			tribes.Insert(0);
			titles.Insert("Your last life");
			bys.Insert("");
			wheres.Insert(vector.Zero);
			briefs.Insert(runs);
		}

		if (!k)
			return;

		// The three levels in order: what you've mapped, what you've read, what you've heard of
		for (int want = RZ_EKnow.MAPPED; want >= RZ_EKnow.HEARD; want--)
		{
			foreach (int i, int id : k.m_aTribe)
			{
				RZ_Tribe t = m_Tribes.Find(id);
				if (!t || t.IsRover() || k.m_aLevel[i] != want)
					continue;

				tribes.Insert(id);
				titles.Insert(RZ_Tribes.Capitalize(t.m_sPlace));
				bys.Insert(LevelName(k.m_aLevel[i]));
				wheres.Insert(RoughArea(t, k.m_aLevel[i]));
				briefs.Insert(JournalText(playerId, t, k.m_aLevel[i], k.m_aHeard[i], k.m_aDay[i]));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The map's range rings (RZ_UIFeed.BuildMap, the marker_range icon), one a pack this character
	//! knows of: Heard of, a ring of half a kilometre round its rough area; Read, its range; Mapped, its
	//! range, with where it lies up in the label.
	void MapFor(int playerId, notnull array<vector> wheres, notnull array<float> radii, notnull array<string> labels)
	{
		RZ_TribeKnowing k = Knowing(playerId, false);
		if (!k)
			return;

		foreach (int i, int id : k.m_aTribe)
		{
			RZ_Tribe t = m_Tribes.Find(id);
			int level = k.m_aLevel[i];
			if (!t || t.IsRover() || level == RZ_EKnow.NONE)
				continue;

			string label = string.Format("%1 (%2)", RZ_Tribes.Capitalize(t.m_sPlace), LevelName(level));
			if (level == RZ_EKnow.HEARD)
			{
				wheres.Insert(RoughArea(t, level));
				radii.Insert(500);
			}
			else
			{
				wheres.Insert(t.RangeCentre());
				radii.Insert(t.m_fRadius);
			}

			if (level >= RZ_EKnow.MAPPED)
				label += string.Format(", lies up at %1", RZ_SettlementScanner.GridReference(t.m_vLair));

			labels.Insert(label);
		}
	}

	//------------------------------------------------------------------------------------------------
	static string LevelName(int level)
	{
		switch (level)
		{
			case RZ_EKnow.HEARD: return "Heard of";
			case RZ_EKnow.READ: return "Read";
			case RZ_EKnow.MAPPED: return "Mapped";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Heard of: a 500 m circle round a point near its range; Read: the range's centre; Mapped: the lair.
	vector RoughArea(RZ_Tribe t, int level)
	{
		if (level >= RZ_EKnow.MAPPED)
			return t.m_vLair;

		if (level == RZ_EKnow.READ)
			return t.RangeCentre();

		int deg = t.m_iSeed % 360;
		float a = deg * Math.DEG2RAD;
		return t.RangeCentre() + Vector(Math.Sin(a) * 250, 0, Math.Cos(a) * 250);
	}

	//------------------------------------------------------------------------------------------------
	//! The journal's text for one tribe, by what this character knows and their skills.
	string JournalText(int playerId, RZ_Tribe t, int level, int heard, int day)
	{
		string text = NamesLine(t, heard);
		text += string.Format(" %1, day %2.", LevelName(level), day);
		if (level == RZ_EKnow.HEARD)
			text += " Somewhere about here, give or take half a kilometre.";

		if (level >= RZ_EKnow.READ)
			text += string.Format(" Its ground runs about %1 m round %2.", RZ_Util.RoundToInt(t.m_fRadius / 50) * 50, RZ_SettlementScanner.GridReference(t.RangeCentre()));

		if (level >= RZ_EKnow.MAPPED)
		{
			text += string.Format(" It lies up at %1. %2", RZ_SettlementScanner.GridReference(t.m_vLair), SizeText(t));
			text += " " + TemperText(t);
		}

		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		string bgId = "";
		if (bg)
			bgId = bg.m_sId;

		// Elder: who they were; Ex-Con: which crew is fed
		if (bgId == "elder" && HavenKnowsLeader(t))
			text += string.Format(" The Haven knew who leads it: %1, %2.", t.m_sAlphaName, t.m_sAlphaWho);

		if (bgId == "elder")
		{
			foreach (RZ_NamedYoung y : t.m_aNamed)
			{
				if (!y.m_bPlayer)
					text += string.Format(" %1 runs with it now.", y.m_sName);
			}
		}

		if (bgId == "raider" && t.m_bMoat)
			text += string.Format(" A Red Hand crew keeps %1 fed. Nobody says what with.", CrewName(t));

		// Tags: Lab Techs and Halcyon's Trusted
		if (t.m_iTagDay >= 0 && (bgId == "labtech" || RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.TRUSTED)))
			text += " " + TagText(t);

		return text;
	}

	//------------------------------------------------------------------------------------------------
	protected string SizeText(RZ_Tribe t)
	{
		string lead = "Nothing leads it.";
		if (t.m_bHasAlpha)
			lead = string.Format("Something big leads it, %1 of its time as lead behind it.", RZ_Tribe.StageName(t.Stage()));

		return string.Format("About %1 of them. %2", RZ_Tribes.Answers(t.Members()), lead);
	}

	//------------------------------------------------------------------------------------------------
	protected static string TemperText(RZ_Tribe t)
	{
		switch (t.m_eTemper)
		{
			case RZ_ETemperament.BOLD: return "It comes straight in, and early.";
			case RZ_ETemperament.CAUTIOUS: return "It's careful: keeps to the trees and gives up quickly.";
			case RZ_ETemperament.CUNNING: return "It learns: the same road, the same hour.";
			case RZ_ETemperament.TERRITORIAL: return "It never leaves its own ground.";
			case RZ_ETemperament.NOMADIC: return "It never stays long in one place.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A tag's reading: the leader's rough area (300 m), its stage, lean or fed.
	string TagText(RZ_Tribe t)
	{
		if (!t.m_bHasAlpha)
			return "The tag is still sending, from the ground. Nothing carries it any more.";

		vector at = t.m_vLair;
		RZ_TribePart a = t.FindPart(RZ_ETribePart.ALPHA);
		if (a)
			at = a.m_vPos;

		string fed = "lean";
		if (t.m_fHunger < 40)
			fed = "fed";

		return string.Format("Tag reading: near %1 (300 m), %2, %3.", RZ_SettlementScanner.GridReference(at), RZ_Tribe.StageName(t.Stage()), fed);
	}

	// ================================================================ Ways to learn (services)
	//------------------------------------------------------------------------------------------------
	//! A quartermaster's service (RZ_FactionServices): Ask about the roads (Haven, Known), the contact
	//! board (Bastion, Known), the map of the crews and the tag frequencies (Red Hand), the range map
	//! (Halcyon, from 25%), the call box (Halcyon, Trusted, from 50%). True if handled.
	bool Service(int playerId, RZ_EFactionService service, IEntity user)
	{
		if (!IsOn())
		{
			m_GM.NotifyPlayer(playerId, "Nothing to tell", "\"Nothing out there worth telling you about.\"");
			return true;
		}

		vector from = vector.Zero;
		if (user)
			from = user.GetOrigin();

		switch (service)
		{
			case RZ_EFactionService.ROADS: return AskRoads(playerId, from);
			case RZ_EFactionService.CONTACT_BOARD: return ContactBoard(playerId);
			case RZ_EFactionService.CREW_MAP: return CrewMap(playerId);
			case RZ_EFactionService.FREQUENCIES: return Frequencies(playerId, user);
			case RZ_EFactionService.RANGE_MAP: return RangeMap(playerId);
			case RZ_EFactionService.CALL_BOX: return GiveCallBox(playerId, user);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasTier(int playerId, RZ_EFaction f, RZ_ETier tier, string what)
	{
		RZ_ETier have = m_GM.GetTier(playerId, f);
		if (RZ_Reputation.IsAtLeast(have, tier) && !RZ_Reputation.IsHunted(have))
			return true;

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("%1 is for people who are %2 with them.", what, RZ_Reputation.GetTierName(tier)));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Ask about the roads: the three nearest tribes, the Haven's names for them, and their mood.
	protected bool AskRoads(int playerId, vector from)
	{
		if (!HasTier(playerId, RZ_EFaction.SURVIVORS, RZ_ETier.KNOWN, "Asking about the roads"))
			return true;

		array<RZ_Tribe> near = {};
		Nearest(from, 3, near);
		if (near.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, "The roads", "\"Quiet, round here. Nothing out in the woods we know of.\"");
			return true;
		}

		string text = "";
		foreach (RZ_Tribe t : near)
		{
			Learn(playerId, t, RZ_EKnow.HEARD, RZ_EFaction.SURVIVORS);
			text += string.Format("\"%1: %2\" ", RZ_Tribes.Capitalize(NameAt(t, RZ_EFaction.SURVIVORS)), MoodWords(t));
		}

		string voice = RZ_VoiceBank.PickFor(m_GM, "infected", RZ_EFaction.SURVIVORS, "roads", null, playerId, from, NameAt(near[0], RZ_EFaction.SURVIVORS));
		if (!voice.IsEmpty())
			text += voice;

		m_GM.NotifyPlayer(playerId, "The roads", text.Trim());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! "Quiet; ate last night." The mood a speaker gives, in plain words.
	protected static string MoodWords(RZ_Tribe t)
	{
		if (t.m_bFed)
			return "fed last night, quiet today.";

		switch (t.Band())
		{
			case 0: return "quiet lately.";
			case 1: return "lean. Mind the roads after dark.";
			case 2: return "hungry. Nobody out after seven.";
		}

		return "desperate. They'll come to the wire.";
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's contact board: every tribe Bastion has read, as Read, with its numbers.
	protected bool ContactBoard(int playerId)
	{
		if (!HasTier(playerId, RZ_EFaction.MILITARY, RZ_ETier.KNOWN, "The contact board"))
			return true;

		int n = 0;
		string text = "";
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iContact <= 0)
				continue;

			Learn(playerId, t, RZ_EKnow.READ, RZ_EFaction.MILITARY);
			if (n < 4)
				text += ContactLine(t) + " ";

			n++;
		}

		if (n == 0)
			text = "\"Board's empty. Nothing near our posts we've had to count.\"";

		m_GM.NotifyPlayer(playerId, "The contact board", text.Trim());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion puts a pack on its contact board (numbered as found).
	void AddToBoard(RZ_Tribe t)
	{
		if (t.m_iContact > 0)
			return;

		t.m_iContact = m_iNextContact;
		m_iNextContact++;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's Known online read a pack on the board.
	void BoardToKnown(RZ_Tribe t)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.MILITARY), RZ_ETier.KNOWN))
				Learn(playerId, t, RZ_EKnow.READ, RZ_EFaction.MILITARY);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Contact Papa-two, the quarry by Morton, twenty plus, one heavy. Cull pending."
	string ContactLine(RZ_Tribe t)
	{
		string count = "under ten";
		int m = t.Members();
		if (m >= 20)
			count = "twenty plus";
		else if (m >= 10)
			count = "ten plus";

		string heavy = "no heavy";
		if (t.m_bHasAlpha)
			heavy = "one heavy";

		string pending = "Cull pending.";
		if (RZ_WorldFacts.Get().BastionPresence() < 0.2)
			pending = "No cull. No reply.";

		return string.Format("%1, %2, %3. %4", NameAt(t, RZ_EFaction.MILITARY), count, heavy, pending);
	}

	//------------------------------------------------------------------------------------------------
	//! The map of the crews, from the Red Hand's fence, for Favor: every tribe Read, with its crew name.
	protected bool CrewMap(int playerId)
	{
		RZ_TribeSettings s = m_Tribes.GetSettings();
		if (!HasTier(playerId, RZ_EFaction.BANDITS, RZ_ETier.KNOWN, "The map of the crews"))
			return true;

		if (!PayFavor(playerId, RZ_EFaction.BANDITS, s.m_iCrewMapFavor))
			return true;

		string moat = "";
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			Learn(playerId, t, RZ_EKnow.READ, RZ_EFaction.BANDITS);
			if (t.m_bMoat)
				moat = string.Format(" \"And %1 are ours. Don't ask what they eat.\"", NameAt(t, RZ_EFaction.BANDITS));
		}

		m_GM.NotifyPlayer(playerId, "The map of the crews", "A map with pencil rings on it, a name in each ring. It's in your notebook." + moat);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's tag frequencies, sold by the Red Hand's fence (Trusted, for Favor): every tagged
	//! leader's rough area, and a stolen tag when the server has the tag item.
	protected bool Frequencies(int playerId, IEntity user)
	{
		RZ_TribeSettings s = m_Tribes.GetSettings();
		if (!HasTier(playerId, RZ_EFaction.BANDITS, RZ_ETier.TRUSTED, "The tag frequencies"))
			return true;

		array<RZ_Tribe> tagged = {};
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iTagDay >= 0)
				tagged.Insert(t);
		}

		if (tagged.IsEmpty() && s.m_sTagPrefab.IsEmpty())
		{
			m_GM.NotifyPlayer(playerId, "The fence", "\"Nothing on the beaks' frequencies this week. Come back when they've tagged something.\"");
			return true;
		}

		if (!PayFavor(playerId, RZ_EFaction.BANDITS, s.m_iFrequencyFavor))
			return true;

		string text = "\"Bought off a beak who needed it more than we did.\"";
		foreach (RZ_Tribe t2 : tagged)
		{
			Learn(playerId, t2, RZ_EKnow.READ, RZ_EFaction.BANDITS);
			text += " " + NameAt(t2, RZ_EFaction.BANDITS) + ": " + TagText(t2);
		}

		if (!s.m_sTagPrefab.IsEmpty() && user)
		{
			IEntity tag = RZ_Util.SpawnPrefab(s.m_sTagPrefab, user.GetOrigin() + Vector(0, 0.3, 0));
			if (tag)
				text += " \"And a tag, one of theirs. You didn't get it here.\"";
		}

		m_GM.NotifyPlayer(playerId, "The fence", text);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Halcyon's range map, from 25%: the ranges Halcyon has read or tagged, as Read (not the lairs).
	protected bool RangeMap(int playerId)
	{
		if (!HasTier(playerId, RZ_EFaction.INSTITUTE, RZ_ETier.KNOWN, "The range map"))
			return true;

		RZ_CureResearch cure = m_GM.GetCure();
		if (!cure || cure.m_fProgress < 25)
		{
			m_GM.NotifyPlayer(playerId, "Halcyon", "\"We don't have enough to draw a map yet. When the antiviral's done, perhaps.\"");
			return true;
		}

		int n = GiveRangeMap(playerId);
		if (n == 0)
			m_GM.NotifyPlayer(playerId, "Halcyon", "\"Nothing we've read near enough to map. We're sorry.\"");
		else
			m_GM.NotifyPlayer(playerId, "Halcyon's range map", string.Format("A printout with %1 rings on it. It's in your notebook.", n));

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The ranges Halcyon has read (within 4 km of the lab) or tagged, as Read. The count.
	int GiveRangeMap(int playerId)
	{
		int n = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!HalcyonReads(t))
				continue;

			Learn(playerId, t, RZ_EKnow.READ, RZ_EFaction.INSTITUTE);
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	bool HalcyonReads(RZ_Tribe t)
	{
		return t.m_iTagDay >= 0 || (m_GM.HasLab() && RZ_Util.DistXZ(t.RangeCentre(), m_GM.GetLabPosition()) <= LAB_DATA);
	}

	//------------------------------------------------------------------------------------------------
	protected bool PayFavor(int playerId, RZ_EFaction f, int cost)
	{
		if (cost <= 0)
			return true;

		if (m_GM.SpendFavor(playerId, f, cost))
			return true;

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("That's %1 Favor. You haven't got it.", cost));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The n tribes nearest pos (within 4 km).
	void Nearest(vector pos, int n, notnull array<RZ_Tribe> result)
	{
		array<RZ_Tribe> left = {};
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.RangeCentre(), pos) <= 4000)
				left.Insert(t);
		}

		while (result.Count() < n && !left.IsEmpty())
		{
			int best = 0;
			for (int i = 1; i < left.Count(); i++)
			{
				if (RZ_Util.DistXZ(left[i].RangeCentre(), pos) < RZ_Util.DistXZ(left[best].RangeCentre(), pos))
					best = i;
			}

			result.Insert(left[best]);
			left.Remove(best);
		}
	}

	// ================================================================ The call box (50%)
	//------------------------------------------------------------------------------------------------
	//! The call box (Halcyon's quartermaster, Trusted, from 50%): 2 a real day on the server, one per
	//! player a real day. Needs the call box item (Zombies > Tribes > Call box prefab).
	protected bool GiveCallBox(int playerId, IEntity user)
	{
		RZ_TribeSettings s = m_Tribes.GetSettings();
		if (s.m_sCallBoxPrefab.IsEmpty() || !user)
		{
			m_GM.NotifyPlayer(playerId, "Halcyon", "\"We haven't got one to lend.\"");
			return true;
		}

		if (!HasTier(playerId, RZ_EFaction.INSTITUTE, RZ_ETier.TRUSTED, "The call box"))
			return true;

		RZ_CureResearch cure = m_GM.GetCure();
		if (!cure || cure.m_fProgress < 50)
		{
			m_GM.NotifyPlayer(playerId, "Halcyon", "\"The recordings aren't clean enough yet. After the bite treatment.\"");
			return true;
		}

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		int now = RZ_SeasonClock.Now();
		int last;
		if (p && m_mCallBox.Find(p.m_sIdentity, last) && now - last < 24 * HOUR)
		{
			m_GM.NotifyPlayer(playerId, "Halcyon", "\"One a day. You've had today's.\"");
			return true;
		}

		int today = 0;
		foreach (string ident, int at : m_mCallBox)
		{
			if (now - at < 24 * HOUR)
				today++;
		}

		if (today >= 2)
		{
			m_GM.NotifyPlayer(playerId, "Halcyon", "\"Both boxes are out. Try tomorrow.\"");
			return true;
		}

		IEntity box = RZ_Util.SpawnPrefab(s.m_sCallBoxPrefab, user.GetOrigin() + Vector(0, 0.3, 0));
		if (!box)
			return true;

		if (p)
			m_mCallBox.Set(p.m_sIdentity, now);

		m_GM.NotifyPlayer(playerId, "The call box", "\"A recorded call from the ridges, ninety seconds of it. Not near people, and not near anyone who isn't with you. Please bring it back.\"");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The call box plays (RZ_CallBoxAction): 90 s of a recorded dusk call, as a data noise. At night
	//! the nearest tribe within 1.5 km sends its hunters and young (never the leader or the guard);
	//! by day the crawlers drift to it, and a day party within 1.5 km. Never within 300 m of a lived-in
	//! place, or within 150 m of another player who isn't in your group. True if it played.
	bool PlayCallBox(int playerId, vector at)
	{
		RZ_Land land = m_GM.GetLand();
		if (land && land.InNoMansLand(at))
		{
			m_GM.NotifyPlayer(playerId, "The call box", "Not here. There are people too close.");
			return false;
		}

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (ids[i] != playerId && !RZ_Util.SameGroup(ids[i], playerId) && RZ_Util.DistXZ(p.GetOrigin(), at) <= 150)
			{
				m_GM.NotifyPlayer(playerId, "The call box", "Not with someone else this close.");
				return false;
			}
		}

		m_GM.GetHeat().AddHeat(at, 25);
		m_Tribes.GetProps().CallSound(at, false);
		RZ_Tribe t = m_Tribes.NearestTribe(at, 1500);
		bool night = m_Tribes.GetPhase() != RZ_ETribePhase.DAY;
		int drawn = 0;
		if (t)
			drawn = m_Tribes.DrawTo(t, at, night);

		RZ_Log.Info(string.Format("Tribes: a call box played at %1 (%2 parts drawn).", RZ_SettlementScanner.GridReference(at), drawn));
		m_GM.NotifyPlayer(playerId, "The call box", "The box plays a call off the ridges, long and rising, for a minute and a half. Then it clicks off.");
		return true;
	}

	// ================================================================ Skills
	//------------------------------------------------------------------------------------------------
	//! On spawn: a Soldier has the contact board, a Police Officer has read the column, an Elder
	//! has heard the Haven's names, an Ex-Con the Red Hand's.
	void OnPlayerSpawned(int playerId)
	{
		if (!IsOn())
			return;

		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		if (!bg)
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (bg.m_sId == "soldier" && t.m_iContact > 0)
				Learn(playerId, t, RZ_EKnow.READ, RZ_EFaction.MILITARY);
			else if (bg.m_sId == "police" && t.m_bColumn)
				Learn(playerId, t, RZ_EKnow.READ, -1);
			else if (bg.m_sId == "elder" && HavenKnowsLeader(t))
				Learn(playerId, t, RZ_EKnow.HEARD, RZ_EFaction.SURVIVORS);
			else if (bg.m_sId == "raider")
				Learn(playerId, t, RZ_EKnow.HEARD, RZ_EFaction.BANDITS);
		}

		// Halcyon's range map, from 25%, for its Known
		RZ_CureResearch cure = m_GM.GetCure();
		if (cure && cure.m_fProgress >= 25 && RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.KNOWN))
			GiveRangeMap(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! At dusk, after the calls: whoever heard a call from its range's edge has read it; a Radio
	//! Operator hears the calls pass ridge to ridge.
	void AfterCalls(array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		array<RZ_Tribe> called = {};
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_bCalled && !t.m_bFed && t.m_bHasAlpha)
				called.Insert(t);
		}

		foreach (int i, IEntity p : players)
		{
			vector pos = p.GetOrigin();
			foreach (RZ_Tribe t2 : called)
			{
				float d = RZ_Util.DistXZ(t2.RangeCentre(), pos);
				if (d <= t2.m_fRadius + 300)
					Learn(ids[i], t2, RZ_EKnow.READ, -1);
				else if (d <= RZ_Tribes.CALL_RANGE)
					Learn(ids[i], t2, RZ_EKnow.HEARD, -1);
			}

			RZ_Background bg = m_GM.GetPlayerBackground(ids[i]);
			if (!bg || bg.m_sId != "radioham" || called.Count() < 2)
				continue;

			// The relays: from the westmost call within 5 km to the eastmost, or north to south
			RZ_Tribe a;
			RZ_Tribe b;
			foreach (RZ_Tribe c : called)
			{
				if (RZ_Util.DistXZ(c.RangeCentre(), pos) > 5000)
					continue;

				if (!a || c.RangeCentre()[0] < a.RangeCentre()[0])
					a = c;

				if (!b || c.RangeCentre()[0] > b.RangeCentre()[0])
					b = c;
			}

			if (!a || !b || a == b)
				continue;

			Learn(ids[i], a, RZ_EKnow.READ, -1);
			Learn(ids[i], b, RZ_EKnow.READ, -1);
			m_GM.NotifyPlayer(ids[i], "On the set", string.Format("Calls passing %1, ridge to ridge, from %2 toward %3.", RZ_Tribes.CompassWord(a.RangeCentre(), b.RangeCentre()), a.m_sPlace, b.m_sPlace));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe body dropped samples near a Lab Technician (from 50%): which pack, and how long its
	//! leader has led.
	void OnSampleNear(RZ_Tribe t, vector at)
	{
		RZ_CureResearch cure = m_GM.GetCure();
		if (!IsOn() || !cure || cure.m_fProgress < 50)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			RZ_Background bg = m_GM.GetPlayerBackground(ids[i]);
			if (!bg || bg.m_sId != "labtech" || RZ_Util.DistXZ(p.GetOrigin(), at) > 150)
				continue;

			Learn(ids[i], t, RZ_EKnow.READ, RZ_EFaction.INSTITUTE);
			int days = RZ_Util.RoundToInt(RZ_SeasonClock.DaysSince(t.m_fLedSince));
			m_GM.NotifyPlayer(ids[i], "The sample", string.Format("%1, by the markers. Its index has led for about %2 days.", NameAt(t, RZ_EFaction.INSTITUTE), days));
		}
	}

	// ================================================================ Faction voices and marks
	//------------------------------------------------------------------------------------------------
	//! Once a game evening, at dusk: Bastion reads contacts near its posts onto the board and calls
	//! them; the Haven warns about a hungry pack near its houses; Halcyon reports a tagged index; the
	//! Red Hand talks about its crew. Each to the faction's Known, one line each, with the place.
	void DuskVoices()
	{
		if (!IsOn() || m_bDuskDone)
			return;

		m_bDuskDone = true;
		RZ_Territory territory = m_GM.GetTerritory();
		float presence = RZ_WorldFacts.Get().BastionPresence();
		RZ_Tribe bastionNews;
		RZ_Tribe havenNews;
		RZ_Tribe halcyonNews;
		RZ_Tribe crewNews;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (territory && presence >= 0.2 && t.m_iContact <= 0 && territory.FindHolding(RZ_EFaction.MILITARY, t.RangeCentre(), BASTION_READS) != null)
			{
				t.m_iContact = m_iNextContact;
				m_iNextContact++;
				bastionNews = t;
			}

			if (t.Band() >= 2 && NearHaven(t.RangeCentre(), 3000) && (!havenNews || t.m_fHunger > havenNews.m_fHunger))
				havenNews = t;

			if (t.m_iTagDay >= 0 && t.m_bHasAlpha)
				halcyonNews = t;

			if (t.m_bMoat)
				crewNews = t;
		}

		if (bastionNews)
			TellFaction(RZ_EFaction.MILITARY, "Task Force Bastion", "contact", bastionNews, ContactLine(bastionNews) + " Bastion out.");

		if (havenNews)
			TellFaction(RZ_EFaction.SURVIVORS, "The Haven", "warn", havenNews, string.Format("\"Nobody on the roads by %1 after seven. %2 are out.\"", havenNews.m_sPlace, RZ_Tribes.Capitalize(Name(havenNews, RZ_EFaction.SURVIVORS))));

		if (halcyonNews)
			TellFaction(RZ_EFaction.INSTITUTE, "Halcyon", "index", halcyonNews, string.Format("\"%1: the index is in its %2 as lead. Please don't shoot it.\"", NameAt(halcyonNews, RZ_EFaction.INSTITUTE), WeekWord(halcyonNews)));

		if (crewNews && Math.RandomFloat01() < 0.3)
			TellFaction(RZ_EFaction.BANDITS, "The Red Hand", "crew", crewNews, string.Format("\"%1. Same as us: the boss is whoever's got something on the rest.\"", RZ_Tribes.Capitalize(NameAt(crewNews, RZ_EFaction.BANDITS))));
	}

	//------------------------------------------------------------------------------------------------
	protected static string WeekWord(RZ_Tribe t)
	{
		int weeks = Math.Floor(RZ_SeasonClock.DaysSince(t.m_fLedSince) / 7) + 1;
		switch (weeks)
		{
			case 1: return "first week";
			case 2: return "second week";
			case 3: return "third week";
		}

		return "fourth week or more";
	}

	//------------------------------------------------------------------------------------------------
	//! A faction's line to its Known: the voice bank's line for this kind when it has one (with the
	//! speaker's name and the place as {place}), else the built one. They hear the faction's word.
	void TellFaction(RZ_EFaction f, string title, string manner, RZ_Tribe t, string built)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, f), RZ_ETier.KNOWN))
				continue;

			string text = RZ_VoiceBank.PickFor(m_GM, "infected", f, manner, null, playerId, t.RangeCentre(), NameAt(t, f));
			if (text.IsEmpty())
				text = built;

			text = CapQuoted(text);

			Learn(playerId, t, RZ_EKnow.HEARD, f);
			m_GM.NotifyPlayer(playerId, title, text);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The first letter up, inside an opening quote too.
	static string CapQuoted(string text)
	{
		if (text.Length() < 2 || text.Get(0) != "\"")
			return RZ_Tribes.Capitalize(text);

		return "\"" + RZ_Tribes.Capitalize(text.Substring(1, text.Length() - 1));
	}

	//------------------------------------------------------------------------------------------------
	protected bool NearHaven(vector pos, float radius)
	{
		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return false;

		int idx = houses.FindNearest(pos, radius);
		return idx >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a road enters the range: the road square on the range's edge nearest the nearest town,
	//! else the edge toward that town.
	vector MarkSpot(RZ_Tribe t)
	{
		vector c = t.RangeCentre();
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(c));
		vector toward = c + Vector(0, 0, t.m_fRadius);
		if (s)
			toward = s.m_vCenter;

		RZ_Land land = m_GM.GetLand();
		vector best = c + Dir(c, toward) * t.m_fRadius;
		float bestD = 100000;
		for (int i = 0; i < 16; i++)
		{
			float a = i * 22.5 * Math.DEG2RAD;
			vector p = c + Vector(Math.Sin(a) * t.m_fRadius, 0, Math.Cos(a) * t.m_fRadius);
			if (!land || !land.IsReady())
				break;

			int cell = land.CellAt(p);
			if (cell < 0 || land.GetRoadWidth(cell) <= 0)
				continue;

			float d = RZ_Util.DistXZ(p, toward);
			if (d < bestD)
			{
				bestD = d;
				best = p;
			}
		}

		return RZ_Util.GroundPos(best);
	}

	//------------------------------------------------------------------------------------------------
	//! Each faction writes its mark where a road enters a range it knows (props are planned; as text
	//! today): the Haven near its houses, Bastion on its board, the Red Hand near its camps and its
	//! fed crew, Halcyon on a tag. Rewritten every 3 season days, so an old mark can be wrong.
	protected void WriteMarks()
	{
		int day = RZ_SeasonClock.Day();
		RZ_Territory territory = m_GM.GetTerritory();
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			vector c = t.RangeCentre();
			for (int f = 0; f < 4; f++)
			{
				bool knows = false;
				switch (f)
				{
					case RZ_EFaction.SURVIVORS: knows = NearHaven(c, 3000); break;
					case RZ_EFaction.MILITARY: knows = t.m_iContact > 0; break;
					case RZ_EFaction.BANDITS: knows = t.m_bMoat || (territory && territory.FindHolding(RZ_EFaction.BANDITS, c, 3000) != null); break;
					case RZ_EFaction.INSTITUTE: knows = t.m_iTagDay >= 0; break;
				}

				if (!knows || (!t.m_aMarkText[f].IsEmpty() && day - t.m_aMarkDay[f] < 3))
					continue;

				t.m_aMarkText[f] = MarkText(t, f);
				t.m_aMarkDay[f] = day;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Haven chalk: QUARRY LOT. NOT AFTER 7." / "a Bastion stencil: P-2 · QUARRY · 20+ · HVY · 1930" /
	//! "a red hand with a line through it, and POSTMEN" / "a Halcyon plate with the plague doctor on
	//! it: P-02 · I d19".
	string MarkText(RZ_Tribe t, int f)
	{
		string kind = KindWord(t);
		kind.ToUpper();
		switch (f)
		{
			case RZ_EFaction.SURVIVORS:
			{
				string lot = Name(t, f);
				lot.Replace("the ", "");
				lot.ToUpper();
				return string.Format("Haven chalk on a post: %1. NOT AFTER 7.", lot);
			}
			case RZ_EFaction.MILITARY:
			{
				string count = "-10";
				if (t.Members() >= 20)
					count = "20+";
				else if (t.Members() >= 10)
					count = "10+";

				string hvy = "";
				if (t.m_bHasAlpha)
					hvy = " · HVY";

				return string.Format("A Bastion stencil: P-%1 · %2 · %3%4 · %5", t.m_iContact, kind, count, hvy, RZ_Util.RoundToInt(t.m_fSetOutHour * 100));
			}
			case RZ_EFaction.BANDITS:
			{
				string crew = CrewName(t);
				crew.Replace("the ", "");
				crew.ToUpper();
				return string.Format("A red hand with a line through it, and %1.", crew);
			}
			case RZ_EFaction.INSTITUTE:
				return string.Format("A Halcyon plate with the plague doctor stencilled on it: %1 · I d%2", PackCode(t), RZ_Util.RoundToInt(RZ_SeasonClock.DaysSince(t.m_fLedSince)));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A player within 25 m of a mark reads it (once a version): who wrote it and when, and the
	//! range is Read.
	protected void ReadMarks(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			bool any = false;
			foreach (string m : t.m_aMarkText)
			{
				if (!m.IsEmpty())
					any = true;
			}

			if (!any)
				continue;

			vector spot = MarkSpot(t);
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), spot) > MARK_READ)
					continue;

				RZ_PlayerProfile prof = m_GM.GetProfile(ids[i]);
				if (!prof)
					continue;

				for (int f = 0; f < 4; f++)
				{
					if (t.m_aMarkText[f].IsEmpty())
						continue;

					string key = string.Format("%1:%2:%3", prof.m_sIdentity, t.m_iId, f);
					int read;
					if (m_mMarkRead.Find(key, read) && read == t.m_aMarkDay[f])
						continue;

					m_mMarkRead.Set(key, t.m_aMarkDay[f]);
					Learn(ids[i], t, RZ_EKnow.READ, f);
					m_GM.NotifyPlayer(ids[i], "A mark", string.Format("%1 Written on day %2.", t.m_aMarkText[f], t.m_aMarkDay[f]));
				}
			}
		}
	}

	// ================================================================ Your dead run
	//------------------------------------------------------------------------------------------------
	//! A player's character taken (RZ_Tribes.TakePlayer): a named young, a line in their next life's
	//! journal, and their group told. publicText: the description, or the name where it's allowed.
	void OnPlayerTaken(RZ_Tribe t, int playerId, vector at, string description, bool young)
	{
		if (!young || !IsOn() || Cured())
			return;

		RZ_PlayerProfile prof = m_GM.GetProfile(playerId);
		RZ_Background bg = m_GM.GetPlayerBackground(playerId);
		RZ_NamedYoung y = new RZ_NamedYoung();
		y.m_sName = RZ_Util.PlayerName(playerId);
		y.m_sPublic = PublicName(playerId, y.m_sName, description);
		y.m_iPlayerId = playerId;
		y.m_bPlayer = true;
		if (prof)
			y.m_sIdent = prof.m_sIdentity;

		if (bg)
			y.m_sWhat = bg.m_sName;

		FillDeath(y, at);
		AddNamed(t, y);
		if (prof)
			m_mRunsWith.Set(prof.m_sIdentity, string.Format("Your last character, %1, runs with %2 now. Somebody might bring them home.", y.m_sName, NameAt(t, RZ_EFaction.SURVIVORS)));

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int other : all)
		{
			if (other != playerId && RZ_Util.SameGroup(other, playerId))
				m_GM.NotifyPlayer(other, "Your group", string.Format("%1 didn't come back. They run with %2 now.", y.m_sName, NameAt(t, RZ_EFaction.SURVIVORS)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Someone else taken with a name (a wanderer, a walk-out): a named young.
	void OnNamedTaken(RZ_Tribe t, string who, vector at, bool walkout)
	{
		if (!RZ_TribeWays.IsAName(who))
			return;

		RZ_NamedYoung y = new RZ_NamedYoung();
		y.m_sName = who;
		y.m_sPublic = who;
		y.m_bWalkout = walkout;
		FillDeath(y, at);
		AddNamed(t, y);
		if (walkout)
			t.m_iWalkouts++;
	}

	//------------------------------------------------------------------------------------------------
	protected void FillDeath(RZ_NamedYoung y, vector at)
	{
		y.m_iDay = RZ_SeasonClock.Day();
		y.m_fDays = RZ_SeasonClock.Days();
		y.m_vAt = at;
		y.m_sWhere = "out on the roads";
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
		if (s)
			y.m_sWhere = "near " + s.m_sName;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			RZ_PlayerProfile prof = m_GM.GetProfile(ids[i]);
			if (prof && RZ_Util.DistXZ(p.GetOrigin(), at) <= 300)
				y.m_aNear.Insert(prof.m_sIdentity);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! At most 3 players' characters and 6 named young in all a tribe; the oldest goes first.
	protected void AddNamed(RZ_Tribe t, RZ_NamedYoung y)
	{
		t.m_aNamed.Insert(y);
		int players = 0;
		for (int i = t.m_aNamed.Count() - 1; i >= 0; i--)
		{
			if (!t.m_aNamed[i].m_bPlayer)
				continue;

			players++;
			if (players > 3)
				t.m_aNamed.Remove(i);
		}

		while (t.m_aNamed.Count() > 6)
		{
			t.m_aNamed.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player's character in public text: the name only with Your dead run on Named, the player
	//! not opted out, and the name past the name filter; else the description.
	string PublicName(int playerId, string name, string description)
	{
		RZ_TribeSettings s = m_Tribes.GetSettings();
		if (s.m_eYourDeadRun != RZ_EDeadRun.NAMED || name.IsEmpty())
			return description;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p && m_aHidden.Contains(p.m_sIdentity))
			return description;

		string low = name;
		low.ToLower();
		foreach (string word : m_aFilter)
		{
			if (!word.IsEmpty() && low.Contains(word))
				return description;
		}

		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! The opt-out (the notebook item's action, or /mydead): hidden keeps your name out of public
	//! text even on a Named server. Toggles when hidden is -1. Tells the player.
	void SetHidden(int playerId, int hidden)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (!p)
			return;

		bool now = m_aHidden.Contains(p.m_sIdentity);
		bool want = !now;
		if (hidden >= 0)
			want = hidden > 0;

		if (want && !now)
			m_aHidden.Insert(p.m_sIdentity);
		else if (!want && now)
			m_aHidden.RemoveItem(p.m_sIdentity);

		RZ_TribeSettings s = m_Tribes.GetSettings();
		string server = "";
		if (s.m_eYourDeadRun != RZ_EDeadRun.NAMED)
			server = " (This server never uses names in public anyway.)";

		if (want)
			m_GM.NotifyPlayer(playerId, "Your dead", "If you die out there, people will be told what you looked like, never your name." + server);
		else
			m_GM.NotifyPlayer(playerId, "Your dead", "If you die out there, people may hear your name, where this server allows it." + server);
	}

	//------------------------------------------------------------------------------------------------
	//! A young of the tribe put down near a player: if it was someone named, the ID card (the ID card
	//! prefab when set, else a line): the name, what they did, the day and place they died. Bring
	//! Them Home's target is always the one it wants. The named young, or null.
	RZ_NamedYoung OnYoungDown(RZ_Tribe t, vector at, RZ_NamedYoung wanted)
	{
		if (t.m_aNamed.IsEmpty() || !RZ_Tribes.PlayerWithin(at, 100))
			return null;

		RZ_NamedYoung y = wanted;
		if (!y)
		{
			// The named are among the young: one of them about as often as they're a share of it
			float share = t.m_aNamed.Count();
			share = share / Math.Max(1, t.Count(RZ_ETribePart.YOUNG) + 1);
			if (Math.RandomFloat01() > share)
				return null;

			y = t.m_aNamed.GetRandomElement();
		}

		t.m_aNamed.RemoveItem(y);
		if (y.m_bPlayer && t.m_iPlayerYoung > 0)
			t.m_iPlayerYoung--;

		string card = IdCardText(y);
		RZ_TribeSettings s = m_Tribes.GetSettings();
		if (!s.m_sIdCardPrefab.IsEmpty() && !wanted)
		{
			IEntity item = RZ_Util.SpawnPrefab(s.m_sIdCardPrefab, RZ_Util.GroundPos(at) + Vector(0, 0.2, 0));
			if (item)
				RZ_GameModeComponent.TrackLooseItem(item);
		}

		m_Tribes.Tell(at, 100, "An ID card", card);
		AddBeat(string.Format("Brought home: %1 was put down %2 and laid to rest.", PublicNameOf(y), y.m_sWhere));
		return y;
	}

	//------------------------------------------------------------------------------------------------
	string IdCardText(RZ_NamedYoung y)
	{
		string what = y.m_sWhat;
		if (what.IsEmpty())
			what = "no trade on it";

		return string.Format("In a pocket, an ID card: %1. %2. Died on day %3, %4.", y.m_sName, what, y.m_iDay, y.m_sWhere);
	}

	//------------------------------------------------------------------------------------------------
	string PublicNameOf(RZ_NamedYoung y)
	{
		if (!y.m_bPlayer)
			return y.m_sName;

		return PublicName(y.m_iPlayerId, y.m_sName, y.m_sPublic);
	}

	//------------------------------------------------------------------------------------------------
	//! Bring Them Home's exclusions: never for someone in your group, someone who died within 300 m
	//! of you, someone dead under a season day, or after your delivery today.
	bool CanBringHome(int playerId, RZ_NamedYoung y)
	{
		if (!y)
			return false;

		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (!p)
			return false;

		if (y.m_bPlayer && (y.m_iPlayerId == playerId || RZ_Util.SameGroup(y.m_iPlayerId, playerId) || y.m_sIdent == p.m_sIdentity))
			return false;

		if (y.m_aNear.Contains(p.m_sIdentity) || RZ_SeasonClock.DaysSince(y.m_fDays) < 1)
			return false;

		int day;
		return !m_mDelivered.Find(p.m_sIdentity, day) || day != RZ_SeasonClock.Day();
	}

	//------------------------------------------------------------------------------------------------
	void Delivered(int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (p)
			m_mDelivered.Set(p.m_sIdentity, RZ_SeasonClock.Day());
	}

	//------------------------------------------------------------------------------------------------
	//! Wanderers' road news, Haven-flavoured, always with the place; a named young among the news.
	string RoadNews(vector from)
	{
		if (!IsOn())
			return "";

		array<RZ_Tribe> near = {};
		Nearest(from, 1, near);
		if (near.IsEmpty())
			return "";

		RZ_Tribe t = near[0];
		foreach (RZ_NamedYoung y : t.m_aNamed)
		{
			if (Math.RandomFloat01() < 0.4)
				return string.Format("\"%1 runs with %2 now. Somebody saw them at dusk.\"", RZ_Tribes.Capitalize(PublicNameOf(y)), NameAt(t, RZ_EFaction.SURVIVORS));
		}

		if (t.Band() >= 2)
			return string.Format("\"Nobody on the roads by %1 after seven. %2 are out.\"", t.m_sPlace, RZ_Tribes.Capitalize(Name(t, RZ_EFaction.SURVIVORS)));

		return string.Format("\"%1 call at dusk up at %2. Keep off their side of the valley after dark.\"", RZ_Tribes.Capitalize(Name(t, RZ_EFaction.SURVIVORS)), t.m_sPlace);
	}

	//------------------------------------------------------------------------------------------------
	//! Road news heard: the nearest tribe is Heard of, in the Haven's word.
	void OnRoadNews(int playerId, vector from)
	{
		array<RZ_Tribe> near = {};
		Nearest(from, 1, near);
		if (!near.IsEmpty())
			Learn(playerId, near[0], RZ_EKnow.HEARD, RZ_EFaction.SURVIVORS);
	}

	//------------------------------------------------------------------------------------------------
	//! A straight way from a to b crosses the ground of a hungry pack (hunger 60 or more).
	bool CrossesHungry(vector a, vector b)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.Band() >= 2 && Between(t.RangeCentre(), a, b, t.m_fRadius))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Inside some pack's ground.
	bool InRange(vector pos)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.RangeCentre(), pos) <= t.m_fRadius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Within `radius` of a pack's lair.
	bool NearLair(vector pos, float radius)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.m_vLair, pos) <= radius)
				return true;
		}

		return false;
	}

	// ================================================================ The changed missions
	//------------------------------------------------------------------------------------------------
	//! A mission's horde from a pack instead (Crashed Helicopter, Zombie Bait): the pack whose ground
	//! takes in `at` (or comes within `reach` of it) sends its hunters and young at night, its walkers
	//! by day. False with no pack, or none of it free: the mission spawns its own horde then.
	bool PackComes(vector at, float reach)
	{
		if (!IsOn())
			return false;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.RangeCentre(), at) > t.m_fRadius + reach || t.IsWeakened())
				continue;

			bool night = m_Tribes.GetPhase() != RZ_ETribePhase.DAY;
			if (m_Tribes.DrawTo(t, at, night) > 0)
			{
				RZ_Log.Info(string.Format("Tribes: %1 comes to a mission's noise at %2.", t.m_sPlace, RZ_SettlementScanner.GridReference(at)));
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Siren Test: the crawlers converge; packs come only with a bells habit (within 3 km).
	void BellsCome(vector at)
	{
		if (!IsOn())
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.StrongHabit(RZ_EHabit.BELLS) && RZ_Util.DistXZ(t.RangeCentre(), at) <= 3000)
				m_Tribes.DrawTo(t, at, m_Tribes.GetPhase() != RZ_ETribePhase.DAY);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Mass Grave: the nearest pack within reach eats (hunger falls).
	void FeedNearest(vector at, float reach, float by)
	{
		if (!IsOn())
			return;

		RZ_Tribe t = m_Tribes.NearestTribe(at, reach);
		if (t)
			t.m_fHunger = Math.Max(0, t.m_fHunger - by);
	}

	//------------------------------------------------------------------------------------------------
	//! Walk Him Out's "walk on": the nearest pack within 2 km takes him, a named young (a walk-out),
	//! instead of +5 infection. False with none near.
	bool WalksOn(vector at, string who)
	{
		if (!IsOn())
			return false;

		RZ_Tribe t = m_Tribes.NearestTribe(at, 2000);
		if (!t)
			return false;

		m_Tribes.Taken(t, at, who, true);
		OnNamedTaken(t, who, at, true);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Recon: Bastion's word for the hungriest pack, with its place, or "".
	string Hungriest()
	{
		if (!IsOn())
			return "";

		RZ_Tribe best;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!best || t.m_fHunger > best.m_fHunger)
				best = t;
		}

		if (!best || best.Band() < 1)
			return "";

		AddToBoard(best);
		return string.Format("Your reports point to %1: hungry, and gathering. Expect it to come out early.", NameAt(best, RZ_EFaction.MILITARY));
	}

	// ================================================================ Questline hooks
	//------------------------------------------------------------------------------------------------
	//! HC1 The Three Weeks' site stages: the pack whose ground takes in the site keeps 150 m off the
	//! store while the mission runs (its data spots are pushed out, so its bodies appear outside).
	//! The pack, for the caller to claim, or null.
	static RZ_Tribe HoldSite(RZ_GameModeComponent gm, vector site, int missionId)
	{
		RZ_Tribes tribes = gm.GetTribes();
		if (!tribes || !tribes.GetPeople().IsOn())
			return null;

		RZ_Tribe best;
		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (RZ_Util.DistXZ(t.RangeCentre(), site) <= t.m_fRadius + 300 && (!best || RZ_Util.DistXZ(t.RangeCentre(), site) < RZ_Util.DistXZ(best.RangeCentre(), site)))
				best = t;
		}

		if (!best)
			return null;

		RZ_TribePeople people = tribes.GetPeople();
		people.m_aKeepOff.Insert(site);
		people.m_aKeepOffMission.Insert(missionId);
		people.m_aKeepOffTribe.Insert(best.m_iId);
		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Every 5 s: the held packs' parts with no bodies keep 150 m off their store; a keep-off whose
	//! mission has ended goes.
	protected void KeepOff()
	{
		RZ_MissionManager mm = m_GM.GetMissions();
		for (int i = m_aKeepOff.Count() - 1; i >= 0; i--)
		{
			RZ_Mission m;
			if (mm)
				m = mm.FindById(m_aKeepOffMission[i]);

			RZ_Tribe t = m_Tribes.Find(m_aKeepOffTribe[i]);
			if (!m || m.m_eState != RZ_EMissionState.ACTIVE || !t)
			{
				m_aKeepOff.Remove(i);
				m_aKeepOffMission.Remove(i);
				m_aKeepOffTribe.Remove(i);
				continue;
			}

			foreach (RZ_TribePart p : t.m_aParts)
			{
				if (p.m_Body || RZ_Util.DistXZ(p.m_vPos, m_aKeepOff[i]) >= 150)
					continue;

				p.m_vPos = RZ_Util.GroundPos(m_aKeepOff[i] + Dir(m_aKeepOff[i], p.m_vPos) * 160);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! H3 The Edge of Town ends and the walks stop: the pack that learned that edge (the nearest
	//! within 2 km of the walker's house) goes hungry, +20, and tests the house's wire for 2 season days.
	void EdgeStops(vector house)
	{
		if (!IsOn())
			return;

		RZ_Tribe t = m_Tribes.NearestTribe(house, 2000);
		if (!t)
			return;

		t.m_fHunger = Math.Min(100, t.m_fHunger + 20);
		t.m_fHungryUntil = RZ_SeasonClock.Days() + 2;
		RZ_Log.Info(string.Format("Tribes: the walks at the edge of town stopped; %1 goes hungry (%2).", t.m_sPlace, RZ_Util.RoundToInt(t.m_fHunger)));
	}

	//------------------------------------------------------------------------------------------------
	//! A pack fed walk-outs: some evenings its young stand at the nearest house's wire at dusk, and
	//! the Haven only gossips about it ("Somebody saw ... at the wire last night"). True if they go.
	bool WalkoutsAtWire(RZ_Tribe t)
	{
		if (!IsOn() || t.m_iWalkouts <= 0 || Math.RandomFloat01() > 0.3)
			return false;

		string who = "";
		foreach (RZ_NamedYoung y : t.m_aNamed)
		{
			if (y.m_bWalkout)
				who = y.m_sName;
		}

		if (!who.IsEmpty())
			TellFaction(RZ_EFaction.SURVIVORS, "Gossip", "gossip", t, string.Format("\"Somebody saw %1 at the wire last night. Nobody went out.\"", who));

		return true;
	}

	// ================================================================ Dark corners, the moat
	//------------------------------------------------------------------------------------------------
	//! A counted night's hunger for one tribe, after the life cycle's rise: the Red Hand's fed tribe
	//! and a tribe whose range comes within 1 km of the Long Table's farm gain only 3, and the fed one
	//! never goes above 40 (rule 19). After the cure, the last week's clock runs hunger instead.
	float NightHunger(RZ_Tribe t, float rise)
	{
		if (Cured())
			return 0;

		if (t.m_bMoat)
		{
			if (t.m_fHunger >= 40)
				return 0;

			return Math.Min(Math.Min(rise, 3), 40 - t.m_fHunger);
		}

		if (NearTable(t, 1000))
			return Math.Min(rise, 3);

		return rise;
	}

	//------------------------------------------------------------------------------------------------
	bool NearTable(RZ_Tribe t, float margin)
	{
		RZ_LongTable table = m_GM.GetLongTable();
		if (!table || !table.Exists())
			return false;

		return RZ_Util.DistXZ(t.RangeCentre(), table.GetHousePos()) <= t.m_fRadius + margin;
	}

	//------------------------------------------------------------------------------------------------
	//! A trail or a walk turns back 1 km short of the Long Table's farm: the points past that go.
	void TableTurnsBack(array<vector> path)
	{
		RZ_LongTable table = m_GM.GetLongTable();
		if (!table || !table.Exists() || !path)
			return;

		vector farm = table.GetHousePos();
		for (int i = 1; i < path.Count(); i++)
		{
			if (RZ_Util.DistXZ(path[i], farm) >= 1000)
				continue;

			while (path.Count() > i)
			{
				path.Remove(path.Count() - 1);
			}

			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A day walk keeps 1 km off the Long Table's farm: the points nearer go.
	void KeepOffTable(array<vector> path)
	{
		RZ_LongTable table = m_GM.GetLongTable();
		if (!table || !table.Exists() || !path)
			return;

		vector farm = table.GetHousePos();
		for (int i = path.Count() - 2; i >= 1; i--)
		{
			if (RZ_Util.DistXZ(path[i], farm) < 1000)
				path.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The fat moat (rule 19), once a season day: a Red Hand camp within 3 km feeds the one tribe
	//! that lies between it and Bastion's nearest holding. It gains a member a season day up to 22,
	//! stays settled and doesn't split.
	protected void Moat()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory)
			return;

		RZ_Tribe best;
		float bestD = 100000;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			t.m_bMoat = false;
			vector c = t.RangeCentre();
			RZ_Holding camp = territory.FindHolding(RZ_EFaction.BANDITS, c, 3000);
			if (!camp)
				continue;

			RZ_Holding post = territory.FindHolding(RZ_EFaction.MILITARY, camp.m_vPos, 100000);
			if (!post || !Between(c, camp.m_vPos, post.m_vPos, t.m_fRadius + 500))
				continue;

			float d = RZ_Util.DistXZ(c, camp.m_vPos);
			if (d < bestD)
			{
				bestD = d;
				best = t;
			}
		}

		if (!best)
			return;

		best.m_bMoat = true;
		best.m_bRestless = false;
		best.m_bSplitDue = false;
		if (best.Members() < 22 && !Cured())
			m_Tribes.GetLife().Join(best, 0, 1, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! The flat direction from a to b (north when they're the same spot).
	static vector Dir(vector a, vector b)
	{
		vector d = b - a;
		d[1] = 0;
		float len = d.Length();
		if (len < 1)
			return Vector(0, 0, 1);

		return d * (1 / len);
	}

	//------------------------------------------------------------------------------------------------
	//! pos is near the line from a to b, between them (within `width`).
	static bool Between(vector pos, vector a, vector b, float width)
	{
		vector ab = b - a;
		ab[1] = 0;
		float len = ab.Length();
		if (len < 1)
			return false;

		vector dir = ab * (1 / len);
		vector ap = pos - a;
		ap[1] = 0;
		float along = vector.Dot(ap, dir);
		if (along < 0 || along > len)
			return false;

		vector closest = a + dir * along;
		return RZ_Util.DistXZ(closest, pos) <= width;
	}

	//------------------------------------------------------------------------------------------------
	//! The moat rule (RZ_Territory.TryTake): a take whose straight line from the taker's nearest
	//! holding crosses the range of a tribe whose leader is in its prime or older waits. The third
	//! time it's blocked, it goes through: "they found a way round". True if it waits.
	bool BlocksTake(int holdingId, vector from, vector to)
	{
		if (!IsOn())
			return false;

		RZ_Tribe blocker;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_bHasAlpha && t.Stage() >= 1 && Between(t.RangeCentre(), from, to, t.m_fRadius))
			{
				blocker = t;
				break;
			}
		}

		if (!blocker)
			return false;

		int n;
		m_mMoatBlocks.Find(holdingId, n);
		n++;
		if (n >= 3)
		{
			m_mMoatBlocks.Remove(holdingId);
			RZ_Log.Info(string.Format("Tribes: a take held up by %1 three times goes through (they found a way round).", blocker.m_sPlace));
			return false;
		}

		m_mMoatBlocks.Set(holdingId, n);
		RZ_Log.Info(string.Format("Tribes: a take waits: its road crosses %1 (%2 of 3).", blocker.m_sPlace, n));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand scores a camp town higher when its fed tribe lies between it and Bastion.
	float CampScore(vector town)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!IsOn() || !territory)
			return 1;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (!t.m_bMoat)
				continue;

			RZ_Holding post = territory.FindHolding(RZ_EFaction.MILITARY, town, 100000);
			if (post && Between(t.RangeCentre(), town, post.m_vPos, t.m_fRadius))
				return 1.3;
		}

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's toll at night on a road one of its crews hunts at that hour: one tin more.
	//! The line, or "" (flavour on the existing toll).
	string NightRate(vector at)
	{
		if (!IsOn() || m_Tribes.GetPhase() != RZ_ETribePhase.NIGHT)
			return "";

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			RZ_TribeHabit road = t.StrongHabit(RZ_EHabit.ROAD);
			if (road && RZ_Util.DistXZ(road.m_vPos, at) <= 1500)
				return string.Format("\"Night rate. %1 are out.\"", RZ_Tribes.Capitalize(Name(t, RZ_EFaction.BANDITS)));
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe took someone: Halcyon's sorry line when it's a tagged index ("good condition").
	void OnTaken(RZ_Tribe t, string who)
	{
		if (!IsOn() || t.m_iTagDay < 0 || !t.m_bHasAlpha || !RZ_TribeWays.IsAName(who) || Math.RandomFloat01() > 0.5)
			return;

		TellFaction(RZ_EFaction.INSTITUTE, "Halcyon", "sorry", t, string.Format("\"%1 took %2. The index is in good condition. We're sorry.\"", PackCode(t), who));
	}

	//------------------------------------------------------------------------------------------------
	//! A tribe's leader killed: a tagged one in its prime or older costs its killer 2 Halcyon Infamy
	//! ("a year of data").
	void OnLeaderKilled(RZ_Tribe t, vector at)
	{
		if (!IsOn() || t.m_iTagDay < 0 || t.Stage() < 1)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int nearest = -1;
		float best = 150;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d < best)
			{
				best = d;
				nearest = ids[i];
			}
		}

		t.m_iTagDay = -1;
		if (nearest > 0)
			m_GM.AddInfamy(nearest, RZ_EFaction.INSTITUTE, 2, "a year of data");

		TellFaction(RZ_EFaction.INSTITUTE, "Halcyon", "lost", t, string.Format("\"%1's tag has stopped moving. That was a year of data.\"", PackCode(t)));
	}

	//------------------------------------------------------------------------------------------------
	//! Plant a Tag's action: the tribe's leader carries Halcyon's tag now.
	void Tag(RZ_Tribe t, int playerId)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		t.m_iTagDay = RZ_SeasonClock.Day();
		if (p)
			t.m_sTaggedBy = p.m_sIdentity;

		Learn(playerId, t, RZ_EKnow.MAPPED, RZ_EFaction.INSTITUTE);
		WriteMarks();
	}

	// ================================================================ The changed missions
	//------------------------------------------------------------------------------------------------
	//! A pack's own bodies for a mission's crowd (Lead Them Away, Blow the Bridge): its hunters' and its
	//! young's prefabs, in its look. Empty if it has none.
	array<ResourceName> PackHordePrefabs(RZ_Tribe t)
	{
		array<ResourceName> list = {};
		if (!t)
			return list;

		RZ_ZombieSettings zs = m_GM.GetSpawner().GetSettings();
		foreach (ResourceName h : m_Tribes.PrefabsForTribe(t, RZ_ETribePart.HUNTERS, zs))
		{
			list.Insert(h);
		}

		foreach (ResourceName y : m_Tribes.PrefabsForTribe(t, RZ_ETribePart.YOUNG, zs))
		{
			list.Insert(y);
		}

		return list;
	}

	//------------------------------------------------------------------------------------------------
	//! One of a pack's young, as a prefab (Missing Patrol, the Military Remnant Camp, Live Capture,
	//! Stock the Pit). "" if it has none.
	ResourceName YoungPrefab(RZ_Tribe t)
	{
		if (!t)
			return "";

		array<ResourceName> list = m_Tribes.PrefabsForTribe(t, RZ_ETribePart.YOUNG, m_GM.GetSpawner().GetSettings());
		if (!list || list.IsEmpty())
			return "";

		return list.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! One of a pack's guard, as a prefab (Live Capture). "" if it has none.
	ResourceName GuardPrefab(RZ_Tribe t)
	{
		if (!t)
			return "";

		array<ResourceName> list = m_Tribes.PrefabsForTribe(t, RZ_ETribePart.GUARD, m_GM.GetSpawner().GetSettings());
		if (!list || list.IsEmpty())
			return "";

		return list.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest military pack (soldiers' clothes, or a third of it were soldiers) within radius of
	//! pos, with young to spare, or null (Missing Patrol, the Military Remnant Camp).
	RZ_Tribe NearMilitary(vector pos, float radius)
	{
		if (!IsOn())
			return null;

		RZ_Tribe best;
		float bestD = radius;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_eLook != RZ_ELook.MILITARY && t.m_fMilitaryShare < 0.33)
				continue;

			if (t.Count(RZ_ETribePart.YOUNG) <= 0)
				continue;

			float d = RZ_Util.DistXZ(t.RangeCentre(), pos);
			if (d <= bestD)
			{
				bestD = d;
				best = t;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Stock the Pit: two of a pack's young caged for the next Fight Pit: the crews' name for the pack
	//! they ran with, and their prefab.
	void StockThePit(string crew, ResourceName prefab)
	{
		m_iPitStock = 2;
		m_sPitCrew = crew;
		m_sPitPrefab = prefab;
	}

	//------------------------------------------------------------------------------------------------
	//! The next Fight Pit takes the stock: true and the crew and prefab if there is some.
	bool TakePitStock(out string crew, out ResourceName prefab)
	{
		if (m_iPitStock <= 0)
			return false;

		crew = m_sPitCrew;
		prefab = m_sPitPrefab;
		m_iPitStock = 0;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool HasPitStock()
	{
		return m_iPitStock > 0;
	}

	// ================================================================ Frenzies and Furious
	//------------------------------------------------------------------------------------------------
	//! A frenzy night (Every Ridge Calls, Red Dusk): one at a time, never in the first 2 season days,
	//! one every 3 at most. False if one can't start now.
	bool CanFrenzy()
	{
		int day = RZ_SeasonClock.Day();
		return IsOn() && day > 2 && day - m_iLastFrenzyDay >= 3;
	}

	//------------------------------------------------------------------------------------------------
	//! Every tribe within `radius` of pos (everywhere when radius <= 0) frenzies tonight: its hunting
	//! range x mult, and it hunts three trails. The number of tribes.
	int Frenzy(vector pos, float radius, float mult, bool redDusk)
	{
		m_iLastFrenzyDay = RZ_SeasonClock.Day();
		if (redDusk)
			m_iLastRedDuskDay = m_iLastFrenzyDay;

		int n = 0;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (radius > 0 && RZ_Util.DistXZ(t.RangeCentre(), pos) > radius)
				continue;

			t.m_fFrenzy = mult;
			n++;
			// A frenzy ignores fear places: tonight's trails go straight through them
			if (m_Tribes.WaysFear(t))
				m_Tribes.BuildTrails(t);
		}

		RZ_Log.Info(string.Format("Tribes: a frenzy night (%1 tribes, range x%2).", n, mult));
		// The great one's clan gathers on a frenzy night
		m_Tribes.GetGreat().OnFrenzy();
		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Red Dusk: at most once every 7 season days, from Collapse.
	bool CanRedDusk()
	{
		return CanFrenzy() && RZ_SeasonClock.Day() - m_iLastRedDuskDay >= 7;
	}

	//------------------------------------------------------------------------------------------------
	//! Its leader was caught alive (Capture the Index): Furious for 2 counted nights.
	void MakeFurious(RZ_Tribe t)
	{
		m_iFuriousOrder++;
		t.m_iFuriousOrder = m_iFuriousOrder;
		t.m_iFuriousUntil = m_Tribes.CountedNights() + 2;
		TellFaction(RZ_EFaction.SURVIVORS, "The Haven", "furious", t, string.Format("\"Whatever they took from %1, %2 want it back.\"", t.m_sPlace, Name(t, RZ_EFaction.SURVIVORS)));
		AddBeat(string.Format("Taken alive for the lab: the index of %1.", NameAt(t, RZ_EFaction.INSTITUTE)));
	}

	//------------------------------------------------------------------------------------------------
	bool IsFurious(RZ_Tribe t)
	{
		return t.m_iFuriousUntil >= 0 && m_Tribes.CountedNights() < t.m_iFuriousUntil;
	}

	//------------------------------------------------------------------------------------------------
	//! The most recently Furious tribe still Furious (the last wave of the stand), or null.
	RZ_Tribe LastFurious()
	{
		RZ_Tribe best;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (IsFurious(t) && (!best || t.m_iFuriousOrder > best.m_iFuriousOrder))
				best = t;
		}

		return best;
	}

	// ================================================================ The cure and the stand
	//------------------------------------------------------------------------------------------------
	//! A breakthrough (RZ_CureResearch): 25% the range map for Halcyon's Known.
	void OnMilestone(int milestone)
	{
		if (!IsOn() || milestone != 1)
			return;

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE), RZ_ETier.KNOWN) && GiveRangeMap(playerId) > 0)
				m_GM.NotifyPlayer(playerId, "Halcyon", "\"We've drawn up what we know of the packs' ground. It's in your notebook.\"");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One of the final stand's hordes (RZ_CureResearch): from the crawlers of a town within 3 km of
	//! the lab, or the nearest tribe's hunters, kept between 70% and 130% of the built waves by how
	//! infected the lab's neighbours are. The last wave brings the most recently Furious tribe. The
	//! number of hordes it sent (0: the built wave runs instead).
	int StandWave(vector lab, int hordes, bool last)
	{
		if (!IsOn())
			return 0;

		RZ_ZombieSpawner spawner = m_GM.GetSpawner();
		array<int> towns = {};
		float inf = 0;
		foreach (int idx, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (RZ_Util.DistXZ(s.m_vCenter, lab) <= 3000 && idx != m_GM.GetLabSettlement() && s.m_fInfection > 0)
			{
				towns.Insert(idx);
				inf += s.m_fInfection;
			}
		}

		RZ_Tribe t = m_Tribes.NearestTribe(lab, 3000);
		if (towns.IsEmpty() && !t)
			return 0;

		float avg = 0;
		if (!towns.IsEmpty())
			avg = inf / towns.Count();

		float scale = Math.Clamp(0.7 + 0.6 * avg / 100, 0.7, 1.3);
		m_fStandCarry += hordes * scale;
		int n = Math.Floor(m_fStandCarry);
		m_fStandCarry -= n;
		n = Math.Max(1, n);
		for (int h = 0; h < n; h++)
		{
			vector from;
			if (t && (towns.IsEmpty() || h % 2 == 1))
			{
				from = RZ_Util.GroundPos(t.RangeCentre() + Dir(t.RangeCentre(), lab) * t.m_fRadius);
				m_Tribes.DrawTo(t, lab, true);
			}
			else
			{
				RZ_Settlement s2 = m_GM.GetSettlement(towns.GetRandomElement());
				from = s2.m_vCenter;
			}

			// Out of sight, 350-450 m out, on the line from where they come
			vector at = RZ_Util.GroundPos(lab + Dir(lab, from) * (350 + 50 * h));
			spawner.SpawnHordeAt(at, lab);
		}

		if (last)
		{
			RZ_Tribe furious = LastFurious();
			if (furious)
			{
				m_Tribes.DrawTo(furious, lab, true);
				spawner.SpawnHordeAt(RZ_Util.GroundPos(lab + Dir(lab, furious.RangeCentre()) * 400), lab);

				m_GM.NotifyAll("The last wave", string.Format("%1 came down from %2 with the last of them.", RZ_Tribes.Capitalize(Name(furious, RZ_EFaction.SURVIVORS)), furious.m_sPlace));
			}
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The stand was lost: the tribes that came go home, and the lab's nearest tribe is Hungry for a
	//! season day.
	void OnStandLost(vector lab)
	{
		if (!IsOn())
			return;

		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			m_Tribes.SendHome(t, false);
		}

		RZ_Tribe near = m_Tribes.NearestTribe(lab, 5000);
		if (!near)
			return;

		near.m_fHunger = Math.Max(near.m_fHunger, 60);
		near.m_fHungryUntil = RZ_SeasonClock.Days() + 1;
	}

	// ================================================================ The last week
	//------------------------------------------------------------------------------------------------
	protected bool Cured()
	{
		return m_GM.GetCure() && m_GM.GetCure().IsCured();
	}

	//------------------------------------------------------------------------------------------------
	//! After the cure the tribes run down on real hours (docs/INFECTED.md, "The last week"): hunger
	//! +15 every 12 real hours, and a starving tribe loses a fifth (at least 1) every 12 real hours,
	//! lying down where they are. No break-ups and no lone ones: it keeps its lair and its leader.
	protected void LastWeek()
	{
		int now = RZ_SeasonClock.Now();
		if (m_iCureUnix <= 0)
		{
			m_iCureUnix = now;
			m_iLastWeekAt = now;
			foreach (RZ_Tribe still : m_Tribes.GetTribeList())
			{
				if (still.m_bHasAlpha)
					AddBeat(string.Format("Still calling: %1, with %2 answering at the cure.", NameAt(still, RZ_EFaction.SURVIVORS), RZ_Tribes.Answers(still.Members())));
			}

			RZ_Log.Info("Tribes: the cure is out. Nobody new turns, and the packs run down on real hours.");
			return;
		}

		if (now - m_iLastWeekAt < 12 * HOUR)
			return;

		m_iLastWeekAt = now;
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_fHunger < 100)
			{
				t.m_fHunger = Math.Min(100, t.m_fHunger + 15);
				continue;
			}

			// The young and the hunters go first, the guard last
			int want = Math.Max(1, RZ_Util.RoundToInt(t.Members() * 0.2));
			int gone = m_Tribes.GetLife().TakeMembers(t, RZ_ETribePart.YOUNG, want);
			if (gone < want)
				gone += m_Tribes.GetLife().TakeMembers(t, RZ_ETribePart.HUNTERS, want - gone);

			if (gone < want)
				gone += m_Tribes.GetLife().TakeMembers(t, RZ_ETribePart.SCOUT, want - gone);

			if (gone < want && t.Count(RZ_ETribePart.GUARD) > 0 && t.Members() > 2)
				gone += m_Tribes.GetLife().TakeMembers(t, RZ_ETribePart.GUARD, 1);

			RZ_Log.Info(string.Format("Tribes: %1 is starving in the last week: %2 lay down.", t.m_sPlace, gone));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The call after the cure: thinner every evening. "" if it's not the last week.
	string LastWeekCall(RZ_Tribe t)
	{
		if (!Cured())
			return "";

		if (m_bLastCallDue)
			return string.Format("A call from %1, long and slow. Nothing answers it.", t.m_sPlace);

		return string.Format("%1 still calls. There's hardly anyone left to come.", RZ_Tribes.Capitalize(t.m_sPlace));
	}

	//------------------------------------------------------------------------------------------------
	//! At dusk: is this the last dusk before the wipe falls due? Then every leader calls once and
	//! nothing answers, and at dawn it lies down at its lair.
	protected void CheckLastCall()
	{
		if (!Cured() || m_bLastCall)
			return;

		RZ_Seasons seasons = m_GM.GetSeasons();
		if (!seasons)
			return;

		int wipe = seasons.WipeAt();
		float day = Math.Max(2, m_GM.GetDayLengthHours()) * HOUR;
		if (wipe <= 0 || wipe - RZ_SeasonClock.Now() > day)
			return;

		m_bLastCallDue = true;
		m_bLastCall = true;
		RZ_Log.Info("Tribes: the last call. Every leader still living calls once, and nothing answers.");
	}

	//------------------------------------------------------------------------------------------------
	//! At dawn after the last call: the leaders lie down at their lairs.
	protected void AfterLastCall()
	{
		if (!m_bLastCallDue)
			return;

		m_bLastCallDue = false;
		array<RZ_Tribe> list = {};
		foreach (RZ_Tribe each : m_Tribes.GetTribeList())
		{
			list.Insert(each);
		}

		foreach (RZ_Tribe t : list)
		{
			m_Tribes.Tell(t.m_vLair, 2000, "First light", string.Format("Nothing moves at %1 this morning, and nothing will.", t.m_sPlace));
			m_Tribes.GetLife().RemoveTribe(t, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A beat handed to the epilogue (docs/INFECTED.md, "The last week"), shown by /tribes; the
	//! epilogue's closing work reads them when it's built.
	void AddBeat(string beat)
	{
		m_aBeats.Insert(beat);
		while (m_aBeats.Count() > 40)
		{
			m_aBeats.Remove(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	array<string> GetBeats()
	{
		return m_aBeats;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	//! From RZ_Tribes.Tick: marks every 5 s, the dusk voices once a dusk, the moat once a season
	//! day, the last week's clock.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!IsOn())
			return;

		// The last call is decided as dusk begins, before the calls at 18:45
		RZ_ETribePhase phase = m_Tribes.GetPhase();
		if (phase == RZ_ETribePhase.DUSK)
			CheckLastCall();

		if (phase == RZ_ETribePhase.DUSK && RZ_GameTime.GetHour() >= 18.8)
			DuskVoices();

		if (phase != RZ_ETribePhase.DUSK)
			m_bDuskDone = false;

		if (phase == RZ_ETribePhase.DAY)
			AfterLastCall();

		m_fMarkTimer += dt;
		if (m_fMarkTimer >= 5)
		{
			m_fMarkTimer = 0;
			ReadMarks(ids, players);
			LairsSeen(ids, players);
			KeepOff();
		}

		int day = RZ_SeasonClock.Day();
		if (day != m_iVoiceDay)
		{
			m_iVoiceDay = day;
			Moat();
			WriteMarks();
			foreach (RZ_Tribe t : m_Tribes.GetTribeList())
			{
				if (t.m_fHungryUntil > 0 && RZ_SeasonClock.Days() < t.m_fHungryUntil)
					t.m_fHunger = Math.Max(t.m_fHunger, 60);
			}
		}

		if (Cured())
			LastWeek();
	}

	//------------------------------------------------------------------------------------------------
	//! Seeing a lair by day (within 150 m) or a leader's body (within 80 m) maps the tribe.
	protected void LairsSeen(array<int> ids, array<IEntity> players)
	{
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			IEntity alpha = t.AlphaBody();
			foreach (int i, IEntity p : players)
			{
				vector pos = p.GetOrigin();
				bool lair = m_Tribes.GetPhase() == RZ_ETribePhase.DAY && RZ_Util.DistXZ(pos, t.m_vLair) <= 150;
				bool leader = alpha && RZ_Util.DistXZ(pos, alpha.GetOrigin()) <= 80;
				if (lair || leader)
					Learn(ids[i], t, RZ_EKnow.MAPPED, -1);
			}
		}
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	static void SaveTribe(RZ_Tribe t, RZ_TribeSave s)
	{
		s.contact = t.m_iContact;
		s.tagDay = t.m_iTagDay;
		s.taggedBy = t.m_sTaggedBy;
		s.furiousUntil = t.m_iFuriousUntil;
		s.furiousOrder = t.m_iFuriousOrder;
		s.moat = t.m_bMoat;
		s.hungryUntil = t.m_fHungryUntil;
		s.trailOff = t.m_iTrailOff;
		s.trailOffUntil = t.m_iTrailOffUntil;
		s.walkouts = t.m_iWalkouts;
		foreach (int i, string m : t.m_aMarkText)
		{
			s.markText.Insert(m);
			s.markDay.Insert(t.m_aMarkDay[i]);
		}

		foreach (RZ_NamedYoung y : t.m_aNamed)
		{
			s.namedName.Insert(y.m_sName);
			s.namedPublic.Insert(y.m_sPublic);
			s.namedIdent.Insert(y.m_sIdent);
			s.namedWhat.Insert(y.m_sWhat);
			s.namedDay.Insert(y.m_iDay);
			s.namedDays.Insert(y.m_fDays);
			s.namedWhere.Insert(y.m_sWhere);
			s.namedX.Insert(y.m_vAt[0]);
			s.namedZ.Insert(y.m_vAt[2]);
			s.namedWalkout.Insert(y.m_bWalkout);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A save from before phase 4 reads with its defaults.
	static void RestoreTribe(RZ_Tribe t, RZ_TribeSave s)
	{
		t.m_iContact = s.contact;
		t.m_iTagDay = s.tagDay;
		t.m_sTaggedBy = s.taggedBy;
		t.m_iFuriousUntil = s.furiousUntil;
		t.m_iFuriousOrder = s.furiousOrder;
		t.m_bMoat = s.moat;
		t.m_fHungryUntil = s.hungryUntil;
		t.m_iTrailOff = s.trailOff;
		t.m_iTrailOffUntil = s.trailOffUntil;
		t.m_iWalkouts = s.walkouts;
		if (s.markText && s.markDay)
		{
			for (int i = 0; i < Math.Min(4, Math.Min(s.markText.Count(), s.markDay.Count())); i++)
			{
				t.m_aMarkText[i] = s.markText[i];
				t.m_aMarkDay[i] = s.markDay[i];
			}
		}

		if (!s.namedName || !s.namedWalkout)
			return;

		for (int j = 0; j < s.namedName.Count() && j < s.namedWalkout.Count(); j++)
		{
			RZ_NamedYoung y = new RZ_NamedYoung();
			y.m_sName = s.namedName[j];
			y.m_sPublic = s.namedPublic[j];
			y.m_sIdent = s.namedIdent[j];
			y.m_bPlayer = !y.m_sIdent.IsEmpty();
			y.m_iPlayerId = -1;
			y.m_sWhat = s.namedWhat[j];
			y.m_iDay = s.namedDay[j];
			y.m_fDays = s.namedDays[j];
			y.m_sWhere = s.namedWhere[j];
			y.m_vAt = Vector(s.namedX[j], 0, s.namedZ[j]);
			y.m_bWalkout = s.namedWalkout[j];
			t.m_aNamed.Insert(y);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_TribePeopleSave Save()
	{
		RZ_TribePeopleSave d = new RZ_TribePeopleSave();
		foreach (string ident, RZ_TribeKnowing k : m_mKnow)
		{
			int owner = d.knowIdent.Count();
			d.knowIdent.Insert(ident);
			d.knowLife.Insert(k.m_iLife);
			foreach (int i, int tribe : k.m_aTribe)
			{
				d.knowOwner.Insert(owner);
				d.knowTribe.Insert(tribe);
				d.knowLevel.Insert(k.m_aLevel[i]);
				d.knowHeard.Insert(k.m_aHeard[i]);
				d.knowDay.Insert(k.m_aDay[i]);
			}
		}

		foreach (string h : m_aHidden)
		{
			d.hidden.Insert(h);
		}

		foreach (string di, int dd : m_mDelivered)
		{
			d.deliveredIdent.Insert(di);
			d.deliveredDay.Insert(dd);
		}

		foreach (string ci, int ca : m_mCallBox)
		{
			d.callBoxIdent.Insert(ci);
			d.callBoxAt.Insert(ca);
		}

		foreach (string ri, string rt : m_mRunsWith)
		{
			d.runsIdent.Insert(ri);
			d.runsText.Insert(rt);
		}

		foreach (string b : m_aBeats)
		{
			d.beats.Insert(b);
		}

		foreach (int mh, int mc : m_mMoatBlocks)
		{
			d.moatHolding.Insert(mh);
			d.moatCount.Insert(mc);
		}

		d.nextContact = m_iNextContact;
		d.furiousOrder = m_iFuriousOrder;
		d.cureUnix = m_iCureUnix;
		d.lastWeekAt = m_iLastWeekAt;
		d.lastCall = m_bLastCall;
		d.lastFrenzyDay = m_iLastFrenzyDay;
		d.lastRedDuskDay = m_iLastRedDuskDay;
		d.pitStock = m_iPitStock;
		d.pitCrew = m_sPitCrew;
		d.pitPrefab = m_sPitPrefab;
		return d;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_TribePeopleSave d)
	{
		if (!d)
			return;

		m_iNextContact = Math.Max(1, d.nextContact);
		m_iFuriousOrder = d.furiousOrder;
		m_iCureUnix = d.cureUnix;
		m_iLastWeekAt = d.lastWeekAt;
		m_bLastCall = d.lastCall;
		m_iLastFrenzyDay = d.lastFrenzyDay;
		m_iLastRedDuskDay = d.lastRedDuskDay;
		m_iPitStock = d.pitStock;
		m_sPitCrew = d.pitCrew;
		m_sPitPrefab = d.pitPrefab;
		if (d.knowIdent && d.knowOwner)
		{
			array<ref RZ_TribeKnowing> owners = {};
			foreach (int i, string ident : d.knowIdent)
			{
				RZ_TribeKnowing k = new RZ_TribeKnowing();
				k.m_sIdent = ident;
				k.m_iLife = d.knowLife[i];
				m_mKnow.Set(ident, k);
				owners.Insert(k);
			}

			foreach (int j, int owner : d.knowOwner)
			{
				if (owner < 0 || owner >= owners.Count())
					continue;

				owners[owner].m_aTribe.Insert(d.knowTribe[j]);
				owners[owner].m_aLevel.Insert(d.knowLevel[j]);
				owners[owner].m_aHeard.Insert(d.knowHeard[j]);
				owners[owner].m_aDay.Insert(d.knowDay[j]);
			}
		}

		if (d.hidden)
		{
			foreach (string h : d.hidden)
			{
				m_aHidden.Insert(h);
			}
		}

		if (d.deliveredIdent)
		{
			foreach (int a, string di : d.deliveredIdent)
			{
				m_mDelivered.Set(di, d.deliveredDay[a]);
			}
		}

		if (d.callBoxIdent)
		{
			foreach (int b, string ci : d.callBoxIdent)
			{
				m_mCallBox.Set(ci, d.callBoxAt[b]);
			}
		}

		if (d.runsIdent)
		{
			foreach (int c, string ri : d.runsIdent)
			{
				m_mRunsWith.Set(ri, d.runsText[c]);
			}
		}

		if (d.beats)
		{
			foreach (string beat : d.beats)
			{
				m_aBeats.Insert(beat);
			}
		}

		if (d.moatHolding)
		{
			foreach (int e, int mh : d.moatHolding)
			{
				m_mMoatBlocks.Set(mh, d.moatCount[e]);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! For the status log: "people: 12 characters know of packs, 2 on Bastion's board, 1 tagged, moat
	//! at the quarry by Morton, 3 named young, frenzy none, Furious none, 4 beats".
	string StatusText()
	{
		int board = 0;
		int tagged = 0;
		int named = 0;
		string moat = "none";
		string frenzy = "none";
		string furious = "none";
		foreach (RZ_Tribe t : m_Tribes.GetTribeList())
		{
			if (t.m_iContact > 0)
				board++;

			if (t.m_iTagDay >= 0)
				tagged++;

			named += t.m_aNamed.Count();
			if (t.m_bMoat)
				moat = t.m_sPlace;

			if (t.m_fFrenzy > 0)
				frenzy = t.m_sPlace;

			if (IsFurious(t))
				furious = t.m_sPlace;
		}

		return string.Format("the packs' people: %1 characters know of packs, %2 on Bastion's board, %3 tagged, moat at %4, %5 named young, frenzy %6, Furious %7, %8 epilogue beats", m_mKnow.Count(), board, tagged, moat, named, frenzy, furious, m_aBeats.Count());
	}
}
