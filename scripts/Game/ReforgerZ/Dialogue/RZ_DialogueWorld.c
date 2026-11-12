//! Conditions and tokens that read the season's story, the infected and the radio (docs/DIALOGUE.md,
//! "Conditions"): who holds the cure, how a major ended and who decided it, how a voter leans, a
//! questline's flags, the packs near the speaker, the bite you declared, the secure net you cracked,
//! kit that remembers whose it was, the cover detail you can say. Every key reads a system that's
//! already there and reads false when that system is off. Server only.
class RZ_DialogueWorld
{
	//------------------------------------------------------------------------------------------------
	//! One of this file's keys. `known` is false for a key this file doesn't own.
	static bool Term(RZ_DCtx ctx, RZ_DTerm t, out bool known)
	{
		known = true;
		RZ_GameModeComponent gm = ctx.m_GM;
		int pid = ctx.m_iPlayer;
		RZ_PlayerProfile profile = gm.GetProfile(pid);
		switch (t.m_sKey)
		{
			case "cure_held": return CureHeld(gm, t.m_sArg);
			case "ending": return Ending(t.m_sArg, t.m_sVal);
			case "was": return Was(profile.m_sIdentity, t.m_sArg);
			case "lean": return Lean(ctx, t.m_sArg);
			case "q": return QuestFlag(gm, t);
			case "tribe": return PackNear(ctx) != null;
			case "alpha":
			{
				RZ_Tribe pack = PackNear(ctx);
				return pack && pack.m_bHasAlpha;
			}
			case "declared": return profile.m_bBiteDeclared;
			case "symptoms": return profile.m_iInfectionStage >= RZ_EInfectionStage.WEAKNESS;
			case "group": return GroupNear(ctx);
			case "secure": return Secure(pid, t.m_sArg);
			case "power": return ctx.m_bRadio && RZ_PowerGrid.IsPoweredAt(ctx.Pos());
			case "cover": return CoverIs(ctx, t.m_sArg);
			case "distress": return RZ_DialogueNets.DistressHouse(ctx) >= 0;
			case "claimed": return Claimed(ctx);
			case "made_right": return RZ_DialogueMakeRight.WasMadeRight(ctx);
			case "scene_lied":
			{
				RZ_WorldFacts facts = gm.GetFacts();
				return facts && facts.Flag("scene_lied." + t.m_sArg) > 0;
			}
			case "talk":
			{
				// A scene's own world flag (set:<key>=N)
				RZ_WorldFacts flags = gm.GetFacts();
				if (!flags)
					return false;

				int v = flags.Flag("talk." + t.m_sArg);
				if (t.m_sOp.IsEmpty())
					return v != 0;

				return Cmp(v, t.m_sOp, t.m_iVal);
			}
		}

		known = false;
		return false;
	}

	// ================================================================ The season's story
	//------------------------------------------------------------------------------------------------
	//! cure_held:<who> after the cure: haven, bastion, redhand, halcyon, everyone (the Independent),
	//! nobody (handed over), slow (the lab on its own), any.
	static bool CureHeld(RZ_GameModeComponent gm, string who)
	{
		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (!epi || !epi.IsBuilt() || epi.Data().lastDay)
			return false;

		int route = epi.Route();
		switch (who)
		{
			case "any": return true;
			case "haven": return route == RZ_CureFinale.HAVEN;
			case "bastion": return route == RZ_CureFinale.BASTION;
			case "redhand": return route == RZ_CureFinale.REDHAND;
			case "halcyon": return route == RZ_CureFinale.HALCYON;
			case "everyone": return route == RZ_CureFinale.INDEP;
			case "indep": return route == RZ_CureFinale.INDEP;
			case "slow": return epi.Data().slow;
			case "nobody": return route < 0 && !epi.Data().slow;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A major's story record, or null.
	protected static RZ_StoryRecord Story(string major)
	{
		RZ_Majors majors = RZ_Majors.Get();
		if (!majors)
			return null;

		foreach (RZ_StoryRecord r : majors.GetStories())
		{
			if (r.major == major && r.ending >= 0)
				return r;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! A major's ending as a condition word: "Every Name" -> every_name, "The Closed Book" -> closed_book.
	static string EndingWord(string major, int ending)
	{
		string name = RZ_Majors.EndingName(major, ending);
		name.ToLower();
		if (name.StartsWith("the "))
			name = name.Substring(4, name.Length() - 4);

		name.Replace(" ", "_");
		name.Replace(":", "");
		return name;
	}

	//------------------------------------------------------------------------------------------------
	//! ending:<major> (it has ended), ending:<major>=<word> (it ended that way).
	static bool Ending(string major, string word)
	{
		RZ_StoryRecord r = Story(major);
		if (!r)
			return false;

		if (word.IsEmpty())
			return true;

		return EndingWord(major, r.ending) == word;
	}

	//------------------------------------------------------------------------------------------------
	//! was:<major>: this player was one of those who decided it (the majors' decider list).
	static bool Was(string ident, string major)
	{
		RZ_StoryRecord r = Story(major);
		return r && !ident.IsEmpty() && r.deciders.Contains(ident);
	}

	//------------------------------------------------------------------------------------------------
	//! The first name the record keeps for who decided a major ("someone" if none), names policy applied.
	static string Decider(string major)
	{
		RZ_StoryRecord r = Story(major);
		if (!r || r.deciders.IsEmpty())
			return "someone";

		RZ_Epilogue epi = RZ_Epilogue.Get();
		if (epi)
		{
			string known = epi.KnownName(r.deciders[0]);
			if (!known.IsEmpty())
				return known;
		}

		return "someone";
	}

	//------------------------------------------------------------------------------------------------
	//! lean:<major>.<vote>.<word>: the speaker's own voter at that sitting leans to that option.
	static bool Lean(RZ_DCtx ctx, string key)
	{
		RZ_QuestVote vote;
		int option = VoteOption(ctx.m_GM, key, vote);
		if (!vote || option < 0 || !ctx.m_Leader)
			return false;

		for (int i = 0; i < vote.Count(); i++)
		{
			if (vote.VoterLeader(i) == ctx.m_Leader.m_iId)
				return vote.Lean(i) == option;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A vote and the option a word names on it ("every_name.first.pot"), -1 none.
	protected static int VoteOption(RZ_GameModeComponent gm, string key, out RZ_QuestVote vote)
	{
		vote = null;
		array<string> parts = {};
		key.Split(".", parts, true);
		if (parts.Count() < 3 || !gm.GetQuestlines())
			return -1;

		RZ_QuestRun run = gm.GetQuestlines().FindRunByKey(parts[0]);
		if (!run)
			return -1;

		vote = new RZ_QuestVote(run, parts[1]);
		if (vote.Count() <= 0)
		{
			vote = null;
			return -1;
		}

		for (int o = 0; o < RZ_QuestVote.MAX_OPTIONS; o++)
		{
			if (run.Text("vote." + parts[1] + ".w" + o.ToString()) == parts[2])
				return o;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! {lean}: the speaker's lean at the sitting a conversation names in its key ("every_name.first"),
	//! as the vote's own line ("Mara Novak leans to the shared pot."), "" when they don't vote.
	static string LeanLine(RZ_DCtx ctx, string key)
	{
		array<string> parts = {};
		key.Split(".", parts, true);
		if (parts.Count() < 2 || !ctx.m_Leader || !ctx.m_GM.GetQuestlines())
			return "";

		RZ_QuestRun run = ctx.m_GM.GetQuestlines().FindRunByKey(parts[0]);
		if (!run)
			return "";

		RZ_QuestVote vote = new RZ_QuestVote(run, parts[1]);
		for (int i = 0; i < vote.Count(); i++)
		{
			if (vote.VoterLeader(i) != ctx.m_Leader.m_iId)
				continue;

			int lean = vote.Lean(i);
			if (lean < 0)
				return "\"I haven't made my mind up. Nobody believes that, but it's true.\"";

			string what = run.Text("vote." + parts[1] + ".w" + lean.ToString());
			what.Replace("_", " ");
			return string.Format("\"My vote's on the %1. Tell me why it shouldn't be.\"", what);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! q:<questline>.<flag>[op N]: a running questline's own flag (q:every_name.act=1).
	static bool QuestFlag(RZ_GameModeComponent gm, RZ_DTerm t)
	{
		int dot = t.m_sArg.IndexOf(".");
		if (dot <= 0 || !gm.GetQuestlines())
			return false;

		RZ_QuestRun run = gm.GetQuestlines().FindRunByKey(t.m_sArg.Substring(0, dot));
		if (!run)
			return false;

		int v = run.Flag(t.m_sArg.Substring(dot + 1, t.m_sArg.Length() - dot - 1));
		if (t.m_sOp.IsEmpty())
			return v != 0;

		return RZ_DialogueWorld.Cmp(v, t.m_sOp, t.m_iVal);
	}

	//------------------------------------------------------------------------------------------------
	static bool Cmp(int a, string op, int b)
	{
		switch (op)
		{
			case "=": return a == b;
			case "!=": return a != b;
			case ">=": return a >= b;
			case "<=": return a <= b;
			case ">": return a > b;
			case "<": return a < b;
		}

		return a != 0;
	}

	// ================================================================ The infected, the room, the radio
	//------------------------------------------------------------------------------------------------
	//! The pack whose range takes in the speaker's place (3 km at most), null if none. Rovers don't count.
	static RZ_Tribe PackNear(RZ_DCtx ctx)
	{
		RZ_Tribes tribes = ctx.m_GM.GetTribes();
		if (!tribes || !tribes.GetPeople() || !tribes.GetPeople().IsOn())
			return null;

		RZ_Tribe t = tribes.NearestTribe(ctx.Pos(), 3000);
		if (!t || t.IsRover() || RZ_Util.DistXZ(t.m_vLair, ctx.Pos()) > t.m_fRadius + 1000)
			return null;

		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! Another player of your vanilla group within 150 m.
	static bool GroupNear(RZ_DCtx ctx)
	{
		if (!ctx.m_User)
			return false;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		foreach (int id : ids)
		{
			if (id == ctx.m_iPlayer || !RZ_Util.SameGroup(ctx.m_iPlayer, id))
				continue;

			IEntity e = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
			if (e && RZ_Util.DistXZ(e.GetOrigin(), ctx.m_User.GetOrigin()) <= 150)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! secure (any) or secure:bastion|redhand|halcyon: listening to that cracked secure net now.
	static int SecureNet(int playerId)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.GetRadio())
			return -1;

		float left;
		return hub.GetRadio().GetListening(playerId, left);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Secure(int playerId, string which)
	{
		int f = SecureNet(playerId);
		if (f < 0)
			return false;

		if (which.IsEmpty())
			return true;

		return f == RZ_DialogueDB.FactionKey(which);
	}

	//------------------------------------------------------------------------------------------------
	//! The best cover detail you could say to the speaker's faction ("" none): the word at the wire,
	//! a crew name, a notebook number, a real name (RZ_Disguise.CoverDetail).
	static string Cover(RZ_DCtx ctx)
	{
		RZ_Hostility hostility = ctx.m_GM.GetHostility();
		if (!hostility || !hostility.GetDisguise().IsOn() || !RZ_Factions.IsValid(ctx.m_iFaction))
			return "";

		return hostility.GetDisguise().CoverDetail(ctx.m_iPlayer, ctx.m_iFaction, "");
	}

	//------------------------------------------------------------------------------------------------
	//! cover (any), cover:word|crew|notebook|name: the best cover detail you can say is of that kind.
	protected static bool CoverIs(RZ_DCtx ctx, string kind)
	{
		string cover = Cover(ctx);
		if (cover.IsEmpty())
			return false;

		switch (kind)
		{
			case "word": return cover.StartsWith("Word: ");
			case "crew": return cover.StartsWith("Crew name: ");
			case "notebook": return cover.StartsWith("Notebook ");
			case "name": return cover.StartsWith("Name: ");
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A speaker a running story holds: a quest topic of theirs shows now, so nothing else may start
	//! or offer work through them this turn.
	static bool Claimed(RZ_DCtx ctx)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		if (!trees)
			return false;

		array<RZ_DConv> all = {};
		trees.GetDB().ForSpeaker(ctx.m_aKinds, all);
		foreach (RZ_DConv c : all)
		{
			if (c.m_iPrio != 0 || c.IsGreet() || c.IsBark() || (c.m_iFlags & RZ_DConv.FLAG_ALWAYS) != 0)
				continue;

			if (c.m_When && !c.m_When.IsEmpty() && c.m_When.Find("claimed") == null && RZ_DialogueCond.Holds(ctx, c.m_When))
				return true;
		}

		return false;
	}

	// ================================================================ Kit that remembers
	//------------------------------------------------------------------------------------------------
	//! What the kit itself says, for the speaker's faction (RZ_DisguiseMarks): issued, signed_other,
	//! dead_mans, off_turned; "" for clean kit, none, or marks off. `mark` gets the record.
	static string KitWord(RZ_DCtx ctx, out RZ_KitMark mark)
	{
		mark = null;
		RZ_Hostility hostility = ctx.m_GM.GetHostility();
		if (!hostility || !hostility.GetDisguise().IsOn() || !RZ_Factions.IsValid(ctx.m_iFaction))
			return "";

		RZ_DisguiseMarks marks = hostility.GetDisguise().GetMarks();
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(ctx.m_iPlayer);
		if (!marks || !me)
			return "";

		switch (marks.KindOf(me, ctx.m_iPlayer, ctx.m_iFaction, mark))
		{
			case RZ_DisguiseMarks.KIT_ISSUED: return "issued";
			case RZ_DisguiseMarks.KIT_SIGNED_OTHER: return "signed_other";
			case RZ_DisguiseMarks.KIT_DEAD: return "dead_mans";
			case RZ_DisguiseMarks.KIT_TURNED: return "off_turned";
		}

		return "";
	}

	// ================================================================ Tokens
	//------------------------------------------------------------------------------------------------
	//! This file's tokens: {resident}, {decider:<major>}, {lean:<major>.<vote>}, {kit_owner},
	//! {dead_name}, {cover}, {pack}, {pack_leader}, {tier:<f>}, {rep:<f>}, {word:<f>}.
	static string Fill(RZ_DCtx ctx, string text)
	{
		string s = text;
		if (s.Contains("{resident}"))
			s.Replace("{resident}", Resident(ctx));

		if (s.Contains("{kit_owner}") || s.Contains("{dead_name}"))
		{
			RZ_KitMark mark;
			KitWord(ctx, mark);
			string who = "someone";
			if (mark && !mark.m_sName.IsEmpty())
				who = mark.m_sName;

			s.Replace("{kit_owner}", who);
			s.Replace("{dead_name}", who);
		}

		if (s.Contains("{cover}"))
		{
			string cover = Cover(ctx);
			int colon = cover.IndexOf(": ");
			if (colon >= 0)
				cover = cover.Substring(colon + 2, cover.Length() - colon - 2);

			if (cover.IsEmpty())
				cover = "nobody you'd know";

			s.Replace("{cover}", cover);
		}

		if (s.Contains("{distress}"))
			s.Replace("{distress}", RZ_DialogueNets.DistressPlace(ctx));

		if (s.Contains("{pack"))
		{
			RZ_Tribe pack = PackNear(ctx);
			int f = ctx.m_iFaction;
			if (!RZ_Factions.IsValid(f))
				f = RZ_EFaction.SURVIVORS;

			string packName = "them up the hill";
			string leader = "the big one";
			if (pack)
			{
				packName = ctx.m_GM.GetTribes().GetPeople().NameAt(pack, f);
				if (pack.m_bHasAlpha)
					leader = ctx.m_GM.GetTribes().GetPeople().LeaderWord(pack, f);
			}

			s.Replace("{pack_leader}", leader);
			s.Replace("{pack}", packName);
		}

		if (s.Contains("{decider:") || s.Contains("{lean:") || s.Contains("{tier:") || s.Contains("{rep:") || s.Contains("{word:"))
			s = Kinds(ctx, s);

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse's seeded resident surname (never a name from LORE), for "Mrs {resident}".
	static string Resident(RZ_DCtx ctx)
	{
		int seed = 17;
		if (RZ_WorldFacts.Get())
			seed = RZ_WorldFacts.Get().GetSeed();

		int house = Math.Max(0, ctx.m_iHouse);
		RZ_Rng rng = new RZ_Rng(seed + house * 977 + 31);
		return RZ_NoteText.Surname(rng);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Kinds(RZ_DCtx ctx, string text)
	{
		string s = text;
		RZ_PlayerProfile profile = ctx.m_GM.GetProfile(ctx.m_iPlayer);
		array<string> words = {"haven", "bastion", "redhand", "halcyon"};
		foreach (string w : words)
		{
			int f = RZ_DialogueDB.FactionKey(w);
			if (s.Contains("{tier:" + w + "}"))
				s.Replace("{tier:" + w + "}", RZ_Reputation.GetTierName(ctx.m_GM.GetTier(ctx.m_iPlayer, f)));

			if (s.Contains("{rep:" + w + "}"))
				s.Replace("{rep:" + w + "}", RZ_RepNames.GetName(RZ_RepNames.GetRep(profile.GetFame(f), profile.GetInfamy(f))));

			if (s.Contains("{word:" + w + "}"))
				s.Replace("{word:" + w + "}", RZ_RepVoice.FillFor(ctx.m_GM, ctx.m_iPlayer, f, "{word}"));
		}

		array<string> majors = {"every_name", "orders", "book", "proof"};
		foreach (string m : majors)
		{
			if (s.Contains("{decider:" + m + "}"))
				s.Replace("{decider:" + m + "}", Decider(m));
		}

		// {lean:every_name.first}: up to the closing brace
		int at = s.IndexOf("{lean:");
		int guard = 0;
		while (at >= 0 && guard < 4)
		{
			guard++;
			int close = s.IndexOfFrom(at, "}");
			if (close < 0)
				break;

			string token = s.Substring(at, close - at + 1);
			string key = token.Substring(6, token.Length() - 7);
			s.Replace(token, LeanLine(ctx, key));
			at = s.IndexOf("{lean:");
		}

		return s;
	}
}
