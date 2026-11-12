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
	static const int CREW_SMOKE = 7;	// threw a smoke grenade (RZ_RepThrows, the throw hook)

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
		if (manner == "hostile" && IsOwed(gm, playerId, faction))
			return "owed";

		if (manner != "hostile" || !leader || !gm.GetLeaders())
			return manner;

		if (leader.HasTrait(RZ_ELeaderTrait.PRAGMATIST))
			return "wary";

		if (leader.m_eLoyalty != RZ_ELoyalty.LOYAL && gm.GetLeaders().Attitude(leader, playerId) >= 0)
			return "wary";

		return manner;
	}

	//------------------------------------------------------------------------------------------------
	//! Hunted or worse with `faction`, and still owed a hearing (the parley, RZ_Parley.IsOwed).
	static bool IsOwed(RZ_GameModeComponent gm, int playerId, int faction)
	{
		RZ_Checkpoints posts = gm.GetCheckpoints();
		return RZ_Factions.IsValid(faction) && posts && posts.GetParley() && posts.GetParley().IsOwed(playerId, faction);
	}

	// ================================================================ The word for you
	//------------------------------------------------------------------------------------------------
	//! What `faction` calls this player to their face this life (docs/REPUTATION.md, "The word").
	//! A set word (the majors, questlines) wins unless they're hostile. The Red Hand uses your crew
	//! name once you have one ("new face" until then). The owed get their own word; after the 90%
	//! lock and until the cure, a locked player is called by the lock's word instead.
	static string Word(RZ_GameModeComponent gm, int playerId, int faction)
	{
		if (!RZ_Factions.IsValid(faction))
			return "you";

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		p.EnsureArrays();
		string manner = Manner(p.GetFame(faction), p.GetInfamy(faction));
		if (manner == "hostile" && IsOwed(gm, playerId, faction))
			return OwedWord(faction);

		string locked = LockOverlay(gm, playerId, faction);
		if (manner != "hostile" && !locked.IsEmpty())
			return locked;

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
	//! The word for the owed (Hunted, with a hearing still owed).
	static string OwedWord(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "put out, prayed for";
			case RZ_EFaction.MILITARY: return "on the list, with a note";
			case RZ_EFaction.BANDITS: return "in the book, both pages";
		}

		return "excluded, with thanks";
	}

	//------------------------------------------------------------------------------------------------
	//! After the 90% lock and until the cure (docs/FINALE.md, "The lock"): what `faction` calls a
	//! locked player. With them, on a rival route, or on the Independent route. "" not locked.
	static string LockOverlay(RZ_GameModeComponent gm, int playerId, int faction)
	{
		RZ_CureFinale fin = gm.GetFinale();
		if (!fin || (gm.GetCure() && gm.GetCure().IsCured()))
			return "";

		int r = fin.RouteOfPlayer(playerId);
		if (r == RZ_CureFinale.NONE)
			return "";

		int rel = 1;
		if (r == faction)
			rel = 0;
		else if (r == RZ_CureFinale.INDEP)
			rel = 2;

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS:
			{
				array<string> haven = {"with the house", "the other lot's", "nobody's"};
				return haven[rel];
			}
			case RZ_EFaction.MILITARY:
			{
				array<string> bastion = {"attached", "hostile element", "unaffiliated"};
				return bastion[rel];
			}
			case RZ_EFaction.BANDITS:
			{
				array<string> redhand = {"in on it", "the other crew", "freelance"};
				return redhand[rel];
			}
		}

		array<string> halcyon = {"signed on", "a confound", "an uncontrolled variable"};
		return halcyon[rel];
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
	//! A wanderer's greeting with Trees on: what they remember of you this life first (saved, helped,
	//! wronged), else the manner of your Haven standing; the Deserter reads Bastion's, reversed (who
	//! Bastion hunts, he warms to). "" with Trees off or no line: the built greeting stays.
	static string WandererGreeting(RZ_GameModeComponent gm, int playerId, RZ_Wanderer w, vector pos)
	{
		if (!TreesOn() || !w || w.m_eRole == RZ_EWandererRole.KID)
			return "";

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		string manner = "";
		if (w.MemoryLife(p.m_sIdentity) == p.m_iLives)
		{
			int mem = w.GetMemory(p.m_sIdentity);
			if ((mem & RZ_Wanderers.MEM_SAVED) != 0)
				manner = "saved";
			else if ((mem & RZ_Wanderers.MEM_WRONGED) != 0)
				manner = "wronged";
			else if ((mem & RZ_Wanderers.MEM_HELPED) != 0)
				manner = "helped";
		}

		string kind = "wanderer";
		int faction = RZ_EFaction.SURVIVORS;
		if (manner.IsEmpty() && w.m_eRole == RZ_EWandererRole.DESERTER)
		{
			kind = "wanderer_deserter";
			faction = RZ_EFaction.MILITARY;
			manner = Manner(p.GetFame(faction), p.GetInfamy(faction));
			if (manner == "warm" || manner == "devoted" || manner == "fond")
				manner = "warm";
			else if (manner != "hostile")
				manner = "unknown";
		}
		else if (manner.IsEmpty())
		{
			manner = Manner(p.GetFame(faction), p.GetInfamy(faction));
		}

		return RZ_VoiceBank.PickFor(gm, kind, faction, manner, null, playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! A pack's leader laid down (RZ_Tribes): the nearest player within 150 m gets the crews' Bailiff,
	//! and each faction's own words for it as a deed, unless a bigger one stands ("laid the big
	//! one down at its lair", "put the heavy down", "evicted the boss", "brought in the index").
	static void PackLeaderDown(RZ_GameModeComponent gm, vector at)
	{
		int best = -1;
		float bestDist = 150;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), at);
			if (d <= bestDist)
			{
				bestDist = d;
				best = ids[i];
			}
		}

		if (best <= 0)
			return;

		CrewEvent(gm, best, CREW_BAILIFF);
		RZ_PlayerProfile profile = gm.GetProfile(best);
		profile.EnsureArrays();
		array<string> words = {"laid the big one down at its lair", "put the heavy down", "evicted the boss up the hill", "brought in the index"};
		for (int f = 0; f < RZ_Factions.COUNT && f < words.Count(); f++)
		{
			RZ_Deeds.NoteBest(profile, f, 1, words[f]);
		}

		RZ_Log.Info(string.Format("Reputation: %1 laid a pack's leader down", RZ_Util.PlayerName(best)));
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

		// Step 1: a major's or a questline's ending this player decided, at a seat of the faction it was about
		string crossing = CrossingLine(gm, ident, l, seat);
		if (!crossing.IsEmpty())
			return crossing;

		// The epilogue week: the first visit each real day, the ending in their voice and where you stood
		string week = EpilogueOpener(gm, playerId, l, seat);
		if (!week.IsEmpty())
			return week;

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

		// After the lock (and the unsided while routes are open): the first visit each real day
		string route = RouteLine(gm, playerId, l, seat);
		if (!route.IsEmpty())
			return FillFor(gm, playerId, l.m_eFaction, route);

		string manner = VoiceManner(gm, playerId, l.m_eFaction, l);
		// The epilogue week, after the day's plain opener: now and then a joke for the ending in the
		// faction's voice (the bank's `week` lines; mood, grief and joke memory apply)
		if (manner != "hostile" && manner != "owed" && gm.GetFacts() && gm.GetFacts().IsEpilogue())
		{
			string weekJoke = RZ_VoiceBank.PickFor(gm, "week", l.m_eFaction, "", l, playerId, l.m_vPos);
			if (!weekJoke.IsEmpty())
				return weekJoke;
		}

		if (manner != "hostile" && manner != "owed")
		{
			// Kin of a fallen Haven Champion, at the Haven's seats, once a life per leader
			string kinKey = seat + "|kin";
			if (p.m_bFallenKin && l.m_eFaction == RZ_EFaction.SURVIVORS && trees && !trees.GetMemory().Has(playerId, RZ_DialogueMemory.NOTES, kinKey))
			{
				trees.GetMemory().Add(playerId, RZ_DialogueMemory.NOTES, kinKey);
				return "\"Family. Sit by me. We read the name every supper, you know. We haven't stopped.\"";
			}

			string aside = WhoYouWorkFor(gm, playerId, l);
			if (!aside.IsEmpty())
				return aside;

			// One time in three, the best thing they know you did
			string best = p.m_aBestDeed[l.m_eFaction];
			if (!best.IsEmpty() && Math.RandomInt(0, 3) == 0)
				return string.Format("\"You're the one who %1.\"", best);
		}

		// One time in four, a tally standing in for the worst deed: how many of theirs you've put down
		string tally = KillsLine(p, l.m_eFaction);
		if (!tally.IsEmpty() && Math.RandomInt(0, 4) == 0)
			return tally;

		string line = RZ_VoiceBank.PickFor(gm, "opener", l.m_eFaction, manner, l, playerId, l.m_vPos);
		if (!line.IsEmpty())
			return FillFor(gm, playerId, l.m_eFaction, line);

		RZ_Rng rng = new RZ_Rng(Math.RandomInt(1, 2000000000));
		return RZ_LeaderVoice.Greeting(rng, l.m_eFaction);
	}

	//------------------------------------------------------------------------------------------------
	//! The epilogue week's opener, built from parts so every ending has plenty of plain lines: what the
	//! ending is to this faction, then where you stood (on the route that won, on another, on none),
	//! then the place's own beat when there is one. Plain, never a joke; the first visit each real day.
	protected static string EpilogueOpener(RZ_GameModeComponent gm, int playerId, RZ_Leader l, string seat)
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!epi || !epi.IsBuilt() || !trees)
			return "";

		string key = seat + "|week";
		int at = trees.GetMemory().TopicAt(playerId, key);
		if (at > 0 && RZ_SeasonClock.Now() - at < RZ_DialogueMemory.DAY)
			return "";

		trees.GetMemory().NoteTopic(playerId, key);
		int ending = EndingIndex(epi);
		string text = EndingClause(l.m_eFaction, ending);
		RZ_CureFinale fin = gm.GetFinale();
		int mine = RZ_CureFinale.NONE;
		if (fin)
			mine = fin.RouteOfPlayer(playerId);

		int stood = 2;
		if (mine != RZ_CureFinale.NONE && mine == epi.Route())
			stood = 0;
		else if (mine != RZ_CureFinale.NONE)
			stood = 1;

		text += " " + YouClause(l.m_eFaction, stood);
		RZ_EpiBeats beats = epi.GetBeats();
		if (beats)
		{
			string place = beats.PlaceLine(l.m_vPos, 400);
			if (!place.IsEmpty() && Math.RandomInt(0, 2) == 0)
				text += "\n" + place;
		}

		return "\"" + text + "\"";
	}

	//------------------------------------------------------------------------------------------------
	//! 0-3 a faction's route, 4 the Independent, 5 the slow cure, 6 handed over, 7 no cure.
	protected static int EndingIndex(RZ_Epilogue epi)
	{
		if (epi.Data().lastDay)
			return 7;

		if (epi.Data().slow)
			return 5;

		int r = epi.Route();
		if (r >= 0 && r <= 4)
			return r;

		return 6;
	}

	//------------------------------------------------------------------------------------------------
	protected static string EndingClause(int faction, int ending)
	{
		array<string> haven = {"It went hand to hand, like we said. The houses are still arguing about who first.",
			"Bastion has it in a crate. We're on their list, every one of us.",
			"The Red Hand has it, and a price. We pay in tins.",
			"The lab published it. Every name on the paper. I read it twice.",
			"Somebody walked it out to every house. No crate, no list.",
			"The lab finished on its own. Nobody fought for it, and everybody's claiming a share.",
			"Nobody held the room, so it went where the Fame was. We take what comes.",
			"No cure. We read the names tonight, and every night after."};
		array<string> bastion = {"The Haven issues it by hand. No register. We keep one anyway.",
			"Issued at the north road and the posts, one name and one signature at a time.",
			"Sold, at a price. Logged as theft. The log doesn't help.",
			"Published, with every name on it but ours. We have a copy.",
			"Carried out by persons unknown, to everyone. Unregistered.",
			"The lab finished unassisted. Every faction claims it. The log records all the claims.",
			"Allocated by the season's standings. Not an order. A result.",
			"No cure. The roll is called at the shift change, as before."};
		array<string> redhand = {"The Haven's giving it away. Bad for business. Good for everyone else, I hear.",
			"Bastion's got it locked up. Locks are a kind of price list.",
			"Ours. Priced fair. Fair to us.",
			"The beaks printed it. You can't sell a recipe everyone's read.",
			"Somebody gave it to everyone for nothing. Robbed on behalf of the whole Zone.",
			"The lab did it on its own. Nobody fought, so nobody gets paid. I hate it.",
			"Nobody held the room. It went by the scores. The book noticed.",
			"No cure. The book's closed. Nobody's paying anyone."};
		array<string> halcyon = {"The Haven distributes by hand. Unmeasured. Effective by every measure we have.",
			"Bastion holds the stock and issues it alphabetically. We are under H.",
			"Sold. The dose is ours, the price is theirs. Both are recorded.",
			"Published. Every author named. The method is on the lab door.",
			"Distributed without control or record. The outcome is good. We are revising.",
			"Completed in the lab without a finisher. A clean result. A lonely one.",
			"Allocated by season standing, with no controller. Not the protocol. Acceptable.",
			"No cure. The record ends here. Someone should write that down, and I will."};
		ending = Math.ClampInt(ending, 0, 7);
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return haven[ending];
			case RZ_EFaction.MILITARY: return bastion[ending];
			case RZ_EFaction.BANDITS: return redhand[ending];
		}

		return halcyon[ending];
	}

	//------------------------------------------------------------------------------------------------
	//! Where you stood: 0 on the route that holds it, 1 on another, 2 on none.
	protected static string YouClause(int faction, int stood)
	{
		array<string> haven = {"You stood with it. Sit. There's soup.", "You picked someone else's way. You're still fed here.", "You never picked a side. That's allowed. Eat."};
		array<string> bastion = {"You signed the roster and held to it.", "You signed for someone else. It's on file.", "You signed nothing. Also logged."};
		array<string> redhand = {"You backed the winner. The book likes that.", "You backed the other horse. No hard feelings. A few.", "You didn't bet. Sensible. Boring."};
		array<string> halcyon = {"You chose this. Your name is in the acknowledgements.", "You chose another route. That is also data.", "You chose nothing. We have a column for that."};
		stood = Math.ClampInt(stood, 0, 2);
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return haven[stood];
			case RZ_EFaction.MILITARY: return bastion[stood];
			case RZ_EFaction.BANDITS: return redhand[stood];
		}

		return halcyon[stood];
	}

	//------------------------------------------------------------------------------------------------
	//! The crossing opener: once a life per leader, a leader of the faction a major's ending was about
	//! says this player decided it ("You're the one who gave Bastion the book."). "" if none.
	protected static string CrossingLine(RZ_GameModeComponent gm, string ident, RZ_Leader l, string seat)
	{
		RZ_Majors majors = RZ_Majors.Get();
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		int playerId = gm.FindPlayerByIdentity(ident);
		if (!majors || !trees || playerId <= 0)
			return "";

		string once = seat + "|crossing";
		if (trees.GetMemory().Has(playerId, RZ_DialogueMemory.NOTES, once))
			return "";

		foreach (RZ_StoryRecord r : majors.GetStories())
		{
			if (r.ending < 0 || r.faction != l.m_eFaction || !r.deciders.Contains(ident))
				continue;

			string ending = RZ_Majors.EndingName(r.major, r.ending);
			if (ending.IsEmpty())
				continue;

			trees.GetMemory().Add(playerId, RZ_DialogueMemory.NOTES, once);
			return CrossingWords(l.m_eFaction, ending);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static string CrossingWords(int faction, string ending)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("\"You're the one who was there for %1. The house remembers who stood up.\"", ending);
			case RZ_EFaction.MILITARY: return string.Format("\"You were at %1. It's in the log, with your name beside it.\"", ending);
			case RZ_EFaction.BANDITS: return string.Format("\"%1. That was you. The book's got a line about it, in red.\"", ending);
		}

		return string.Format("\"You were one of the people who decided %1. Noted, in the margin.\"", ending);
	}

	//------------------------------------------------------------------------------------------------
	//! "You've put 4 of ours down." in their voice, when the tally is 3 or more and no worst deed says it.
	protected static string KillsLine(RZ_PlayerProfile p, int faction)
	{
		int kills = p.m_aKills[faction];
		if (kills < 3 || !RZ_Deeds.WorstText(p, faction).IsEmpty())
			return "";

		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("\"You've put %1 of ours in the ground. I count. Somebody has to.\"", kills);
			case RZ_EFaction.MILITARY: return string.Format("\"%1 of ours, by your hand. It's in the log. So are you.\"", kills);
			case RZ_EFaction.BANDITS: return string.Format("\"%1 of the crew. The book has a page just for you. It's a long page.\"", kills);
		}

		return string.Format("\"%1 of our people. We recorded each one. We record everything.\"", kills);
	}

	//------------------------------------------------------------------------------------------------
	//! A route line (the voice bank's `route` kind) the first visit to this leader each real day,
	//! once routes are open or someone has locked; "" otherwise.
	protected static string RouteLine(RZ_GameModeComponent gm, int playerId, RZ_Leader l, string seat)
	{
		RZ_CureFinale fin = gm.GetFinale();
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!fin || !trees || (!fin.IsOpen() && fin.RouteOfPlayer(playerId) == RZ_CureFinale.NONE))
			return "";

		RZ_DialogueMemory mem = trees.GetMemory();
		string key = seat + "|route";
		int at = mem.TopicAt(playerId, key);
		if (at > 0 && RZ_SeasonClock.Now() - at < RZ_DialogueMemory.DAY)
			return "";

		string line = RZ_VoiceBank.PickFor(gm, "route", l.m_eFaction, "", l, playerId, l.m_vPos);
		if (!line.IsEmpty())
			mem.NoteTopic(playerId, key);

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! The change notice in the faction's voice (the voice bank's `change` lines, by manner), then
	//! the rule in brackets: "(Hunted: their people shoot you. They still owe you a hearing at a
	//! gate.)". "" with Trees off or no line: the plain notice stays.
	static string ChangeLine(RZ_GameModeComponent gm, int playerId, int faction)
	{
		if (!TreesOn() || !RZ_Factions.IsValid(faction))
			return "";

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		int fame = p.GetFame(faction);
		int infamy = p.GetInfamy(faction);
		string manner = VoiceManner(gm, playerId, faction, null);
		string line = RZ_VoiceBank.PickFor(gm, "change", faction, manner, null, playerId, vector.Zero);
		if (line.IsEmpty())
			return "";

		line = FillFor(gm, playerId, faction, line);
		RZ_ETier tier = gm.GetTier(playerId, faction);
		string rule = string.Format("(%1)", RZ_RepNames.NameWithRule(fame, infamy));
		if (RZ_Reputation.IsHunted(tier) && IsOwed(gm, playerId, faction))
			rule = "(Hunted: their people shoot you. They still owe you a hearing at a gate.)";
		else if (RZ_Reputation.IsHunted(tier))
			rule = "(Hunted: their people shoot you.)";

		return line + " " + rule;
	}

	//------------------------------------------------------------------------------------------------
	//! One time in four: who you work for, when it's another faction and they know it (the faction
	//! you've given the most Fame this life, 15 or more).
	protected static string WhoYouWorkFor(RZ_GameModeComponent gm, int playerId, RZ_Leader l)
	{
		if (Math.RandomInt(0, 4) != 0)
			return "";

		RZ_PlayerProfile p = gm.GetProfile(playerId);
		int best = -1;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (best < 0 || p.GetFame(f) > p.GetFame(best))
				best = f;
		}

		if (best < 0 || best == l.m_eFaction || p.GetFame(best) < 15)
			return "";

		string who = RZ_Factions.GetName(best);
		switch (l.m_eFaction)
		{
			case RZ_EFaction.SURVIVORS: return string.Format("\"You run for %1 these days, I hear. Eat first. Then tell me about it.\"", who);
			case RZ_EFaction.MILITARY: return string.Format("\"You run for %1. Noted. Arms out.\"", who);
			case RZ_EFaction.BANDITS: return string.Format("\"Running for %1 now? Whatever they pay you, it's in our book too.\"", who);
		}

		return string.Format("\"You work for %1, mostly. We've adjusted for it.\"", who);
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
