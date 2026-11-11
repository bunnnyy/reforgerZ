//! Reputation's voice (docs/REPUTATION.md, "What people call you"): what each faction calls you to
//! your face, the Red Hand's crew names, the whole picture, what a leader makes of you in their
//! faction's voice, and the one opening line a leader greets you with. A name opens nothing: every
//! rule still reads the tier. Server only.
//!
//! The openings, the verdicts and the picture are said only while Dialogue > Trees is on (the talk
//! screen as built uses the built lines). The word and crew names are kept either way.

//------------------------------------------------------------------------------------------------
//! The whole picture, first match wins (docs/REPUTATION.md, "The whole picture").
enum RZ_EPictureKind
{
	WANTED,			// Hunted by two or more
	TORN,			// Hunted by one, Known or better with another
	OUTLAW,			// Hunted by one, Known or better with none
	MIXED,			// no Hunted; any mixed cell
	SIDED,			// Known or better with one or more, Watched by one or more
	FRIEND_OF_ALL,	// Known or better with two or more, Watched by none
	ONE_SIDE,		// Known or better with exactly one, Watched by none
	WATCHED,		// Watched by one or more, Known by none
	UNKNOWN			// Stranger with all four
}

//------------------------------------------------------------------------------------------------
class RZ_RepVoice
{
	static const int CREW_LUCKY = 0;	// survived the cut dose
	static const int CREW_SNACK = 1;	// bitten twice in one life
	static const int CREW_LEGS = 2;		// ran a Bastion checkpoint and reached a Red Hand roadblock the same day
	static const int CREW_SKINT = 3;	// paid a toll with exactly the Favor you had
	static const int CREW_POCKETS = 4;	// three things taken at one Bastion search
	static const int CREW_BAILIFF = 5;	// laid down a pack's leader (with the tribes)
	static const int CREW_SORRY = 6;	// killed a crewman, then did a Red Hand job the same day
	static const int CREW_SMOKE = 7;	// threw a signal smoke (planned: needs a throw hook)

	//------------------------------------------------------------------------------------------------
	static bool TreesOn()
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		return trees && trees.IsOn();
	}

	// ================================================================ Manners (for picking lines only)
	//------------------------------------------------------------------------------------------------
	//! The manner of a cell: unknown, warm, devoted, wary, fond, needed, hostile. Lines are written
	//! per faction and manner; no rule ever reads one.
	static string Manner(int fame, int infamy)
	{
		RZ_ERep rep = RZ_RepNames.GetRep(fame, infamy);
		switch (rep)
		{
			case RZ_ERep.STRANGER: return "unknown";
			case RZ_ERep.KNOWN: return "warm";
			case RZ_ERep.TRUSTED: return "warm";
			case RZ_ERep.CHAMPION: return "devoted";
			case RZ_ERep.WATCHED: return "wary";
			case RZ_ERep.MIXED_BAG: return "wary";
			case RZ_ERep.USEFUL_TROUBLE: return "fond";
			case RZ_ERep.FAVOURITE_RASCAL: return "fond";
			case RZ_ERep.WILDCARD: return "needed";
			case RZ_ERep.NECESSARY_EVIL: return "needed";
			case RZ_ERep.WILD_CHILD: return "needed";
		}

		return "hostile";
	}

	//------------------------------------------------------------------------------------------------
	//! The manner as this speaker sees it: a Pragmatist, or an Independent or Rogue leader who
	//! doesn't hold anything against you, deals with people their faction hates (wary, not hostile).
	static string VoiceManner(RZ_GameModeComponent gm, int playerId, int faction, RZ_Leader leader)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!RZ_Factions.IsValid(faction))
			faction = RZ_EFaction.SURVIVORS;

		string manner = Manner(p.GetFame(faction), p.GetInfamy(faction));
		if (manner != "hostile" || !leader || !gm.GetLeaders())
			return manner;

		if (leader.HasTrait(RZ_ELeaderTrait.PRAGMATIST))
			return "wary";

		if (leader.m_eLoyalty != RZ_ELoyalty.LOYAL && gm.GetLeaders().Attitude(leader, playerId) >= 0)
			return "wary";

		return manner;
	}

	// ================================================================ The word for you
	//------------------------------------------------------------------------------------------------
	//! What `faction` calls this player to their face this life (docs/REPUTATION.md, "The word").
	//! A set word (the majors, questlines) wins unless they're hostile. The Red Hand uses your crew
	//! name once you have one ("new face" until then).
	static string Word(RZ_GameModeComponent gm, int playerId, int faction)
	{
		if (!RZ_Factions.IsValid(faction))
			return "you";

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		string manner = Manner(p.GetFame(faction), p.GetInfamy(faction));
		if (manner != "hostile" && !p.m_aSetWord[faction].IsEmpty())
			return p.m_aSetWord[faction];

		RZ_ERep rep = RZ_RepNames.GetRep(p.GetFame(faction), p.GetInfamy(faction));
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (p.m_bFallenKin)
					return "family";

				switch (manner)
				{
					case "unknown": return "love";
					case "warm":
					{
						if (rep == RZ_ERep.TRUSTED)
							return "the one with the ribbon";

						return "a friend of the house";
					}
					case "devoted": return "one of ours";
					case "wary": return "a worry";
					case "fond": return "a worry we feed";
					case "needed": return "a worry we feed";
				}

				return "put out";
			}
			case RZ_EFaction.MILITARY:
			{
				switch (manner)
				{
					case "unknown": return "civilian";
					case "warm":
					{
						if (rep == RZ_ERep.TRUSTED)
							return "auxiliary";

						return "civilian, cleared";
					}
					case "devoted": return "on the roster";
					case "wary": return "uncleared";
					case "fond": return "flagged, useful";
					case "needed": return "flagged";
				}

				return "on the list";
			}
			case RZ_EFaction.BANDITS:
			{
				string crew = CrewName(p);
				switch (manner)
				{
					case "unknown": return "you";
					case "wary": return "you";
					case "hostile": return "that one";
				}

				if (crew.IsEmpty())
					return "new face";

				return crew;
			}
		}

		switch (manner)
		{
			case "unknown": return "visitor";
			case "warm":
			{
				if (rep == RZ_ERep.TRUSTED)
					return "field associate";

				return "courier";
			}
			case "devoted": return "colleague";
			case "wary": return "subject of concern";
			case "fond": return "under review";
			case "needed": return "under review";
		}

		return "excluded from the study";
	}

	//------------------------------------------------------------------------------------------------
	//! A word the majors or questlines give for this life ("Sergeant Novak"). Empty clears it.
	static void SetWord(RZ_GameModeComponent gm, int playerId, int faction, string text)
	{
		if (!RZ_Factions.IsValid(faction))
			return;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		p.m_aSetWord[faction] = text;
	}

	// ================================================================ Crew names
	//------------------------------------------------------------------------------------------------
	static string CrewName(RZ_PlayerProfile p)
	{
		if (!p)
			return "";

		return p.m_sCrewName;
	}

	//------------------------------------------------------------------------------------------------
	static string CrewWord(int which)
	{
		switch (which)
		{
			case CREW_LUCKY: return "Lucky";
			case CREW_SNACK: return "Snack";
			case CREW_LEGS: return "Legs";
			case CREW_SKINT: return "Skint";
			case CREW_POCKETS: return "Pockets";
			case CREW_BAILIFF: return "Bailiff";
			case CREW_SORRY: return "Sorry";
		}

		return "Smoke";
	}

	//------------------------------------------------------------------------------------------------
	//! Something the crews would rather you forgot, but won't. The first one this life while you're
	//! Known or better with the Red Hand becomes your crew name; it's leverage, not an honour.
	static void CrewEvent(RZ_GameModeComponent gm, int playerId, int which)
	{
		if (!gm || playerId <= 0 || !RZ_ReputationSettings.Get().m_bCrewNames)
			return;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!p.m_sCrewName.IsEmpty() || !RZ_Reputation.IsAtLeast(gm.GetTier(playerId, RZ_EFaction.BANDITS), RZ_ETier.KNOWN))
			return;

		p.m_sCrewName = CrewWord(which);
		RZ_Log.Info(string.Format("Reputation: crew name %1 for %2", p.m_sCrewName, RZ_Util.PlayerName(playerId)));
		gm.NotifyPlayer(playerId, "The Red Hand", string.Format("Word on the toll roads: the crews have started calling you %1. It isn't a compliment. It means they've got something on you.", p.m_sCrewName));
	}

	//------------------------------------------------------------------------------------------------
	//! A Bastion checkpoint run today (for Legs).
	static void NoteRan(RZ_GameModeComponent gm, int playerId)
	{
		gm.GetProfile(playerId).m_iRanDay = RZ_SeasonClock.Day();
	}

	//------------------------------------------------------------------------------------------------
	//! At a Red Hand roadblock's barrier: Legs, if a checkpoint was run the same season day.
	static void NoteRoadblock(RZ_GameModeComponent gm, int playerId)
	{
		if (gm.GetProfile(playerId).m_iRanDay == RZ_SeasonClock.Day())
			CrewEvent(gm, playerId, CREW_LEGS);
	}

	//------------------------------------------------------------------------------------------------
	//! A Red Hand person killed (for Sorry, with a Red Hand job the same day).
	static void NoteCrewKill(RZ_GameModeComponent gm, int playerId)
	{
		gm.GetProfile(playerId).m_iCrewKillDay = RZ_SeasonClock.Day();
	}

	//------------------------------------------------------------------------------------------------
	static void NoteRedHandJob(RZ_GameModeComponent gm, int playerId)
	{
		if (gm.GetProfile(playerId).m_iCrewKillDay == RZ_SeasonClock.Day())
			CrewEvent(gm, playerId, CREW_SORRY);
	}

	//------------------------------------------------------------------------------------------------
	//! Another bite this life (for Snack, the second).
	static void NoteBite(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.m_iBites++;
		if (p.m_iBites >= 2)
			CrewEvent(gm, playerId, CREW_SNACK);
	}

	// ================================================================ The whole picture
	//------------------------------------------------------------------------------------------------
	static RZ_EPictureKind Kind(RZ_GameModeComponent gm, int playerId)
	{
		int hunted = 0;
		int known = 0;
		int watched = 0;
		bool mixed = false;
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			RZ_ETier tier = gm.GetTier(playerId, f);
			if (RZ_Reputation.IsHunted(tier))
				hunted++;
			else if (RZ_Reputation.GetAccessRank(tier) >= 1)
				known++;
			else if (RZ_Reputation.IsWatchedOrWorse(tier))
				watched++;

			if (RZ_RepNames.IsMixed(RZ_RepNames.GetRep(p.GetFame(f), p.GetInfamy(f))))
				mixed = true;
		}

		if (hunted >= 2)
			return RZ_EPictureKind.WANTED;
		if (hunted == 1 && known >= 1)
			return RZ_EPictureKind.TORN;
		if (hunted == 1)
			return RZ_EPictureKind.OUTLAW;
		if (mixed)
			return RZ_EPictureKind.MIXED;
		if (known >= 1 && watched >= 1)
			return RZ_EPictureKind.SIDED;
		if (known >= 2)
			return RZ_EPictureKind.FRIEND_OF_ALL;
		if (known == 1)
			return RZ_EPictureKind.ONE_SIDE;
		if (watched >= 1)
			return RZ_EPictureKind.WATCHED;

		return RZ_EPictureKind.UNKNOWN;
	}

	//------------------------------------------------------------------------------------------------
	//! For conditions (picture:mixed).
	static string PictureName(RZ_EPictureKind kind)
	{
		switch (kind)
		{
			case RZ_EPictureKind.WANTED: return "wanted";
			case RZ_EPictureKind.TORN: return "torn";
			case RZ_EPictureKind.OUTLAW: return "outlaw";
			case RZ_EPictureKind.MIXED: return "mixed";
			case RZ_EPictureKind.SIDED: return "sided";
			case RZ_EPictureKind.FRIEND_OF_ALL: return "friend_of_all";
			case RZ_EPictureKind.ONE_SIDE: return "one_side";
			case RZ_EPictureKind.WATCHED: return "watched";
		}

		return "unknown";
	}

	//------------------------------------------------------------------------------------------------
	//! The two factions that know your name best, in a wanderer's voice: "Word is you're the Haven's
	//! Champion and Bastion's Bad News."
	static string Picture(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		RZ_EPictureKind kind = Kind(gm, playerId);
		if (kind == RZ_EPictureKind.UNKNOWN)
			return "\"Nobody's heard of you. Enjoy it.\"";

		int first = -1;
		int second = -1;
		int all = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			int n = RZ_RepNames.Notoriety(p.GetFame(f), p.GetInfamy(f));
			if (n >= 2)
				all++;

			if (first < 0 || Louder(p, f, first))
			{
				second = first;
				first = f;
			}
			else if (second < 0 || Louder(p, f, second))
			{
				second = f;
			}
		}

		if (all >= RZ_Factions.COUNT)
			return "\"Everybody's got an opinion about you. None of them agree.\"";

		string one = string.Format("%1 %2", Possessive(first), RZ_RepNames.Shown(p.GetFame(first), p.GetInfamy(first)));
		if (second < 0 || RZ_RepNames.Notoriety(p.GetFame(second), p.GetInfamy(second)) == 0)
			return string.Format("\"Word is you're %1. That's all anyone says.\"", one);

		string two = string.Format("%1 %2", Possessive(second), RZ_RepNames.Shown(p.GetFame(second), p.GetInfamy(second)));
		if (kind == RZ_EPictureKind.WANTED)
			return string.Format("\"Word is you're %1 and %2. I'd keep off the roads.\"", one, two);

		return string.Format("\"Word is you're %1 and %2. I'll believe the better half.\"", one, two);
	}

	//------------------------------------------------------------------------------------------------
	//! Higher notoriety first; Infamy breaks ties, then Fame.
	protected static bool Louder(RZ_PlayerProfile p, int a, int b)
	{
		int na = RZ_RepNames.Notoriety(p.GetFame(a), p.GetInfamy(a));
		int nb = RZ_RepNames.Notoriety(p.GetFame(b), p.GetInfamy(b));
		if (na != nb)
			return na > nb;

		if (p.GetInfamy(a) != p.GetInfamy(b))
			return p.GetInfamy(a) > p.GetInfamy(b);

		return p.GetFame(a) > p.GetFame(b);
	}

	//------------------------------------------------------------------------------------------------
	static string Possessive(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "the Haven's";
			case RZ_EFaction.MILITARY: return "Bastion's";
			case RZ_EFaction.BANDITS: return "the Red Hand's";
		}

		return "Halcyon's";
	}

	// ================================================================ What they make of you
	//------------------------------------------------------------------------------------------------
	//! "What do you make of me?" in the faction's voice: the name, the best and the worst deed of
	//! this life (or neither), what the leader remembers of this life, then the rule in brackets.
	static void Verdict(RZ_GameModeComponent gm, int playerId, int faction, RZ_Leader leader, notnull array<string> lines)
	{
		if (!RZ_Factions.IsValid(faction))
			return;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		int fame = p.GetFame(faction);
		int infamy = p.GetInfamy(faction);
		string rep = RZ_RepNames.Shown(fame, infamy);
		string best = p.m_aBestDeed[faction];
		string worst = RZ_Deeds.WorstText(p, faction);
		string word = Word(gm, playerId, faction);
		lines.Insert(Template(faction, rep, word, best, worst));

		if (leader && gm.GetLeaders())
		{
			string ident = p.m_sIdentity;
			if (leader.MemoryLife(ident) == p.m_iLives)
			{
				int mem = leader.GetMemory(ident);
				if ((mem & RZ_Leaders.MEM_SAVED) != 0)
					lines.Insert("\"You were there when it counted. I don't forget that.\"");
				else if ((mem & RZ_Leaders.MEM_HELPED) != 0)
					lines.Insert("\"You've done work for us. It was done properly.\"");

				if ((mem & RZ_Leaders.MEM_CROSSED) != 0)
					lines.Insert("\"And I haven't forgotten the other thing.\"");
			}
		}

		lines.Insert(string.Format("(%1: %2. Fame %3, Infamy %4. %5 of %6 left to forgive.)", RZ_Factions.GetName(faction),
			RZ_RepNames.NameWithRule(fame, infamy), fame, infamy, p.ForgiveLeft(faction), RZ_ReputationSettings.Get().m_iForgiveBudget));
	}

	//------------------------------------------------------------------------------------------------
	//! One template per faction, four variants: both deeds, best only, worst only, neither.
	protected static string Template(int faction, string rep, string word, string best, string worst)
	{
		bool b = !best.IsEmpty();
		bool w = !worst.IsEmpty();
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				if (b && w)
					return string.Format("\"People have started calling you %1. You %2. You also %3. I'd rather they didn't have to.\"", rep, best, worst);
				if (b)
					return string.Format("\"People call you %1. You %2. That gets around a house.\"", rep, best);
				if (w)
					return string.Format("\"People call you %1. You %2. That gets around a house too.\"", rep, worst);

				return string.Format("\"People call you %1. Nobody can say why yet.\"", rep);
			}
			case RZ_EFaction.MILITARY:
			{
				if (b && w)
					return string.Format("\"Your file says %1. Commendation: %2. Incident: %3. Both signed for.\"", rep, best, worst);
				if (b)
					return string.Format("\"Your file says %1. Commendation: %2. It went in the log.\"", rep, best);
				if (w)
					return string.Format("\"Your file says %1. Incident: %2. Noted, and initialled.\"", rep, worst);

				return string.Format("\"Your file says %1. The rest of the page is blank.\"", rep);
			}
			case RZ_EFaction.BANDITS:
			{
				if (b && w)
					return string.Format("\"Crews call you %1. The book says %2. Owed: %3. Owing: %4. The book's never wrong, only late.\"", word, rep, best, worst);
				if (b)
					return string.Format("\"Crews call you %1. The book says %2. Owed: %3. We'll get round to paying.\"", word, rep, best);
				if (w)
					return string.Format("\"Crews call you %1. The book says %2. Owing: %3. We'll get round to collecting.\"", word, rep, worst);

				return string.Format("\"The book says %1. Short entry.\"", rep);
			}
		}

		if (b && w)
			return string.Format("\"We have you as %1. It's a category, not a judgement. Contribution: %2. Incident: %3. Both are in the record.\"", rep, best, worst);
		if (b)
			return string.Format("\"We have you as %1. Contribution: %2. It's in the record.\"", rep, best);
		if (w)
			return string.Format("\"We have you as %1. Incident: %2. It's in the record.\"", rep, worst);

		return string.Format("\"We have you as %1. Insufficient data.\"", rep);
	}

	// ================================================================ The opening line
	//------------------------------------------------------------------------------------------------
	//! A leader's one opening line, never a speech. The first that exists: a local deed at this
	//! seat; memory from this life; the name from an earlier life (once per leader per life); the
	//! greeting for your standing in the leader's voice (the voice bank); the built greeting.
	static string Opening(RZ_GameModeComponent gm, int playerId, RZ_Leader l)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		string ident = p.m_sIdentity;
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		string seat = string.Format("leader:%1:%2", l.m_iId, l.m_iGeneration);

		string deed = LocalDeed(playerId, seat);
		if (!deed.IsEmpty())
			return string.Format("\"You're the one who %1.\"", deed);

		int mem = l.GetMemory(ident);
		if (l.MemoryLife(ident) == p.m_iLives)
		{
			if ((mem & RZ_Leaders.MEM_SAVED) != 0)
				return "\"Here's the one I owe. Sit down.\"";
			if ((mem & RZ_Leaders.MEM_CROSSED) != 0)
				return "\"You. I remember.\"";
		}
		else
		{
			int old = l.OldMemory(ident);
			string once = seat + "|oldname";
			if (old != 0 && trees && !trees.GetMemory().Has(playerId, RZ_DialogueMemory.NOTES, once))
			{
				trees.GetMemory().Add(playerId, RZ_DialogueMemory.NOTES, once);
				if ((old & RZ_Leaders.MEM_CROSSED) != 0)
					return string.Format("\"There was a %1 who crossed me once. Different face. Same name. Should I be worried?\"", RZ_Util.PlayerName(playerId));

				return string.Format("\"There was a %1 who did right by us once. Different face. I'll take the name as a start.\"", RZ_Util.PlayerName(playerId));
			}
		}

		string manner = VoiceManner(gm, playerId, l.m_eFaction, l);
		string line = RZ_VoiceBank.PickFor(gm, "opener", l.m_eFaction, manner, l, playerId, l.m_vPos);
		if (!line.IsEmpty())
			return line;

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		return RZ_LeaderVoice.Greeting(rng, l.m_eFaction);
	}

	// ================================================================ Local deeds
	//------------------------------------------------------------------------------------------------
	//! One phrase per player per seat, this life: "You're the one who fixed the pump."
	static void SetLocalDeed(int playerId, string seat, string phrase)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees || phrase.IsEmpty() || seat.IsEmpty())
			return;

		trees.GetMemory().SetDeed(playerId, seat, phrase);
	}

	//------------------------------------------------------------------------------------------------
	static string LocalDeed(int playerId, string seat)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees)
			return "";

		return trees.GetMemory().GetDeed(playerId, seat);
	}

	// ================================================================ Tokens
	//------------------------------------------------------------------------------------------------
	//! A line's per-player tokens, for `faction` (-1: the Haven): {rep}, {word}, {best}, {worst},
	//! {kills}, {jobs}, {picture}, {name}, {crew}.
	static string FillFor(RZ_GameModeComponent gm, int playerId, int faction, string text)
	{
		if (!text.Contains("{"))
			return text;

		int f = faction;
		if (!RZ_Factions.IsValid(f))
			f = RZ_EFaction.SURVIVORS;

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		string s = text;
		s.Replace("{rep}", RZ_RepNames.Shown(p.GetFame(f), p.GetInfamy(f)));
		if (s.Contains("{word}"))
			s.Replace("{word}", Word(gm, playerId, f));

		s.Replace("{best}", p.m_aBestDeed[f]);
		if (s.Contains("{worst}"))
			s.Replace("{worst}", RZ_Deeds.WorstText(p, f));

		s.Replace("{kills}", p.m_aKills[f].ToString());
		s.Replace("{jobs}", p.m_aJobs[f].ToString());
		if (s.Contains("{picture}"))
			s.Replace("{picture}", Picture(gm, playerId));

		s.Replace("{name}", RZ_Util.PlayerName(playerId));
		string crew = CrewName(p);
		if (crew.IsEmpty())
			crew = "new face";

		s.Replace("{crew}", crew);
		return s;
	}
}
