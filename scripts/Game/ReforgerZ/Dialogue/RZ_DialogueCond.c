//! Who is talking to whom, and the one evaluator for conversation conditions (docs/DIALOGUE.md,
//! "Conditions"). Every key reads a system that's already there; keys from designs that aren't
//! built yet read false. Server only.

//------------------------------------------------------------------------------------------------
//! One conversation's people: the player, and the speaker as the adapter describes them.
class RZ_DCtx
{
	RZ_GameModeComponent m_GM;
	int m_iPlayer;
	IEntity m_User;					// weak
	IEntity m_Body;					// weak: the speaker's body
	ref array<string> m_aKinds = {};	// speaker selectors ("seat:haven/council_head", "leader:haven", "leader")
	string m_sKey;					// memory key ("leader:12:0", "wanderer:3:1", "house:2", "named:41:1", "post:2:7", "table:host")
	int m_iMetBefore = -1;			// `met` as it was when this conversation opened (1 or 0), -1 not taken yet
	int m_iFaction = -1;			// the speaker's faction, -1 none
	string m_sName;					// "Ivan Calder"
	string m_sShort;				// "Calder", "Old Marek"
	string m_sRole;					// "council head", "toll man", "trader"
	string m_sPlace;				// the town or holding
	RZ_Leader m_Leader;				// weak
	RZ_Wanderer m_Wanderer;			// weak
	int m_iHouse = -1;				// a safehouse index
	int m_iPost = -1;				// a road post index
	int m_iTableRole = -1;			// RZ_ETableRole
	bool m_bStronghold;				// a stronghold leader, or inside a stronghold or the lab: no threats
	int m_iTown = -1;				// the nearest settlement
	string m_sScene;				// the open topic's scene record key ("" none)
	RZ_StrangerVisit m_Stranger;	// weak: a stranger (errands), for {their:...} and stranger:<kind>
	bool m_bRadio;					// the open channel: no body, nothing personal on air
	string m_sRun;					// a mission's people: "<mission id>" (the `run` scene scope)
	ref map<string, string> m_mBind = new map<string, string>();	// bindings picked when a topic opens: name -> "town:N" | "house:N"
	string m_sLiarOn;				// A good liar (Backgrounds/RZ_Habits.c): the one persuade this conversation made easier

	//------------------------------------------------------------------------------------------------
	vector Pos()
	{
		if (m_Body)
			return m_Body.GetOrigin();

		if (m_User)
			return m_User.GetOrigin();

		return vector.Zero;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueCond
{
	//------------------------------------------------------------------------------------------------
	static bool Holds(RZ_DCtx ctx, RZ_DCond cond)
	{
		if (!cond)
			return true;

		foreach (RZ_DTerm t : cond.m_aTerms)
		{
			bool v = Term(ctx, t);
			if (t.m_bNeg)
				v = !v;

			if (!v)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Cmp(int a, string op, int b)
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

	//------------------------------------------------------------------------------------------------
	//! A tier's name in conditions ("trusted", "kill_on_sight"), -1 unknown.
	static int TierKey(string key)
	{
		switch (key)
		{
			case "stranger": return RZ_ETier.STRANGER;
			case "known": return RZ_ETier.KNOWN;
			case "trusted": return RZ_ETier.TRUSTED;
			case "champion": return RZ_ETier.CHAMPION;
			case "watched": return RZ_ETier.WATCHED;
			case "hunted": return RZ_ETier.HUNTED;
			case "kill_on_sight": return RZ_ETier.KILL_ON_SIGHT;
			case "uneasy": return RZ_ETier.UNEASY;
			case "tolerated": return RZ_ETier.TOLERATED;
			case "respected": return RZ_ETier.RESPECTED;
			case "wildcard": return RZ_ETier.WILDCARD;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Access rank for comparisons: Hunted -2, Kill on Sight -3, Watched -1, else as built.
	protected static int Rank(RZ_ETier tier)
	{
		if (tier == RZ_ETier.KILL_ON_SIGHT)
			return -3;

		if (tier == RZ_ETier.HUNTED)
			return -2;

		if (tier == RZ_ETier.WATCHED)
			return -1;

		return RZ_Reputation.GetAccessRank(tier);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Term(RZ_DCtx ctx, RZ_DTerm t)
	{
		RZ_GameModeComponent gm = ctx.m_GM;
		int pid = ctx.m_iPlayer;
		RZ_PlayerProfile profile = gm.GetProfile(pid);
		RZ_WorldFacts facts = gm.GetFacts();
		RZ_DialogueMemory mem = RZ_DialogueTrees.Get().GetMemory();
		string key = t.m_sKey;

		switch (key)
		{
			case "bg":
				return t.m_aArgs.Find(profile.m_sBackgroundId) >= 0;
			case "tier":
			{
				if (!RZ_Factions.IsValid(t.m_iFaction))
					return false;

				RZ_ETier tier = gm.GetTier(pid, t.m_iFaction);
				int want = TierKey(t.m_sVal);
				if (t.m_sOp.IsEmpty() || want < 0)
					return true;

				if (t.m_sOp == "=")
					return tier == want;

				if (t.m_sOp == "!=")
					return tier != want;

				return Cmp(Rank(tier), t.m_sOp, Rank(want));
			}
			case "fame":
				return RZ_Factions.IsValid(t.m_iFaction) && Cmp(profile.GetFame(t.m_iFaction), OpOr(t), t.m_iVal);
			case "infamy":
				return RZ_Factions.IsValid(t.m_iFaction) && Cmp(profile.GetInfamy(t.m_iFaction), OpOr(t), t.m_iVal);
			case "rep":
			{
				if (!RZ_Factions.IsValid(t.m_iFaction))
					return false;

				string name = RZ_RepNames.GetName(RZ_RepNames.GetRep(profile.GetFame(t.m_iFaction), profile.GetInfamy(t.m_iFaction)));
				name.ToLower();
				name.Replace(" ", "_");
				name.Replace("-", "_");
				return name == t.m_sVal;
			}
			case "hunted":
				return RZ_Factions.IsValid(t.m_iFaction) && RZ_Reputation.IsHunted(gm.GetTier(pid, t.m_iFaction));
			case "exiled":
				return gm.GetSafehouses() && gm.GetSafehouses().IsExiled(pid);
			case "kin":
				return profile.m_bFallenKin;
			case "flag":
			{
				if (!facts)
					return false;

				return Cmp(facts.Flag(t.m_sArg), OpOr(t), t.m_iVal);
			}
			case "cflag":
				return Cmp(mem.CharFlag(pid, t.m_sArg), OpOr(t), t.m_iVal);
			case "nmem":
				return mem.Has(pid, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|" + t.m_sArg);
			case "regard":
				return Cmp(mem.Regard(pid, ctx.m_sKey), t.m_sOp, t.m_iVal);
			case "face":
				return mem.HasFace(pid, t.m_iFaction);
			case "seen":
				return mem.Has(pid, RZ_DialogueMemory.SEEN, t.m_sArg);
			case "claim":
				return mem.Has(pid, RZ_DialogueMemory.CLAIMS, ctx.m_sKey + "|" + t.m_sArg);
			case "knows":
				return mem.Knows(pid, RZ_DialogueTrees.Scoped(ctx, t.m_sArg));
			case "item":
			{
				int star = t.m_sArg.IndexOf("*");
				int need = 1;
				if (star > 0)
					need = t.m_sArg.Substring(star + 1, t.m_sArg.Length() - star - 1).ToInt();

				return ctx.m_User && RZ_HavenQuest.CountFood(gm, ctx.m_User) >= need;
			}
			case "armed":
				return ctx.m_User && RZ_Util.IsArmed(ctx.m_User);
			case "bite_shown":
				return profile.m_fWoundLeft > 0 || profile.m_fInfectionLeft >= 0;
			case "stage":
				return profile.m_iInfectionStage >= StageKey(t.m_sArg);
			case "night":
				return facts && facts.IsNight();
			case "dusk":
				return facts && facts.IsDusk();
			case "day":
				return Cmp(RZ_SeasonClock.Day(), t.m_sOp, t.m_iVal);
			case "hour":
				return Cmp(RZ_Util.RoundToInt(Math.Floor(RZ_GameTime.GetHour())), t.m_sOp, t.m_iVal);
			case "outbreak":
				return facts && facts.Stage() >= OutbreakKey(t.m_sArg);
			case "cure":
				return facts && Cmp(RZ_Util.RoundToInt(facts.CurePercent()), t.m_sOp, t.m_iVal);
			case "epilogue":
				return facts && facts.IsEpilogue();
			case "trait":
				return ctx.m_Leader && ctx.m_Leader.HasTrait(TraitKey(t.m_sArg));
			case "loyalty":
				return ctx.m_Leader && RZ_LeaderVoice.LoyaltyName(ctx.m_Leader.m_eLoyalty) == Capital(t.m_sArg);
			case "attitude":
				return Cmp(RZ_DialogueTrees.Get().Attitude(ctx), t.m_sOp, t.m_iVal);
			case "house":
				return HouseTerm(ctx, t.m_sArg);
			case "town":
				return TownTerm(ctx, t);
			case "others":
				return Cmp(OthersNear(ctx), OpOr(t), t.m_iVal);
			case "decided":
			{
				if (!facts || ctx.m_sScene.IsEmpty())
					return false;

				if (t.m_sArg.IsEmpty())
					return facts.Flag("scene." + ctx.m_sScene) > 0;

				return facts.Flag("scene." + ctx.m_sScene + "." + t.m_sArg) > 0;
			}
			case "swayed":
			{
				// A major's sitting (every_name.first): its own one sway per player; else a tally's
				int vote = RZ_DialogueVotes.Swayed(gm, pid, t.m_sArg);
				if (vote >= 0)
					return vote > 0;

				return facts && facts.Flag("tally." + t.m_sArg + "." + profile.m_sIdentity) > 0;
			}
			case "mood":
				return facts && RZ_SeasonMood.Name(facts.SeasonMood()) == t.m_sArg;
			case "met":
			{
				// As it was when the conversation opened: a first meeting stays one to its end
				if (ctx.m_iMetBefore >= 0)
					return ctx.m_iMetBefore == 1;

				return Met(ctx);
			}
			case "helped":
				return MemoryBit(ctx, 2);
			case "saved":
				return MemoryBit(ctx, 4);
			case "crossed":
				return MemoryBit(ctx, 8);
			case "crew":
				return !RZ_RepVoice.CrewName(profile).IsEmpty();
			case "role":
			{
				string role = ctx.m_sRole;
				role.Replace(" ", "_");
				return role == t.m_sArg;
			}
			case "need":
				return NeedTerm(ctx, t.m_sArg);
			case "wbitten":
				return ctx.m_Wanderer && ctx.m_Wanderer.IsBitten();
			case "kidlost":
				return ctx.m_Wanderer && ctx.m_Wanderer.m_eRole == RZ_EWandererRole.KID && !ctx.m_Wanderer.m_bSettled && ctx.m_Wanderer.m_iEscortPlayer < 0;
			case "table":
				return TableTerm(ctx, t.m_sArg);
			case "picture":
				return RZ_RepVoice.PictureName(RZ_RepVoice.Kind(gm, pid)) == t.m_sArg;
			case "stronghold":
				return ctx.m_Leader && ctx.m_Leader.IsStronghold();
			case "route":
				return RouteTerm(ctx, t.m_sArg);
			case "finale":
				return RZ_CureFinale.FinaleTerm(t.m_sArg, pid, ctx.m_Wanderer);
			case "lockdays":
			{
				RZ_RepSeason rec = RZ_RepSeason.Get();
				return rec && Cmp(rec.LockDays(profile.m_sIdentity), OpOr(t), t.m_iVal);
			}
			case "heard_lie":
				return mem.HeardLie(pid, RZ_DialogueTrees.Scoped(ctx, t.m_sArg));
			case "promise":
				return mem.PromiseState(pid, RZ_DialogueTrees.Scoped(ctx, t.m_sArg)) == RZ_DialogueMemory.OPEN;
			case "kept":
				return mem.PromiseState(pid, RZ_DialogueTrees.Scoped(ctx, t.m_sArg)) == RZ_DialogueMemory.KEPT;
			case "broke":
				return mem.PromiseState(pid, RZ_DialogueTrees.Scoped(ctx, t.m_sArg)) == RZ_DialogueMemory.BROKEN;
			case "due":
				return mem.Due(pid, RZ_DialogueTrees.Scoped(ctx, t.m_sArg));
			case "kit":
				return KitTerm(ctx, t.m_sArg);
			case "cover":
			{
				// Borrowed kit: you have this cover detail to give the speaker's faction (RZ_DisguiseClaims)
				RZ_Disguise covers = Disguise(gm);
				return covers && covers.IsOn() && RZ_Factions.IsValid(ctx.m_iFaction) && !covers.GetClaims().CoverText(pid, ctx.m_iFaction, t.m_sArg).IsEmpty();
			}
			case "claimed":
			{
				// One claim line per speaker per life (RZ_DisguiseClaims)
				RZ_Disguise claims = Disguise(gm);
				return claims && claims.GetClaims().ClaimedTo(pid, ctx.m_sKey);
			}
			case "passing":
			{
				RZ_Disguise passing = Disguise(gm);
				return passing && RZ_Factions.IsValid(ctx.m_iFaction) && passing.IsPassing(pid, ctx.m_iFaction);
			}
			case "caught":
			{
				RZ_Disguise blown = Disguise(gm);
				return blown && RZ_Factions.IsValid(ctx.m_iFaction) && blown.IsBlown(pid, ctx.m_iFaction);
			}
			case "sitting":
				return RZ_DialogueVotes.Sitting(gm, t.m_sArg);
			case "tabled":
				return RZ_DialogueVotes.Tabled(gm, t.m_sArg);
			case "title":
			{
				RZ_RepSeason titles = RZ_RepSeason.Get();
				if (!titles || !RZ_Factions.IsValid(t.m_iFaction))
					return false;

				string title = titles.TitleFor(profile.m_sIdentity, t.m_iFaction);
				title.ToLower();
				title.Replace(" ", "_");
				title.Replace("-", "_");
				if (t.m_sOp.IsEmpty())
					return !title.IsEmpty();

				return title == t.m_sVal;
			}
			case "seasoned":
			{
				RZ_RepSeason seasoned = RZ_RepSeason.Get();
				return seasoned && RZ_Factions.IsValid(t.m_iFaction) && seasoned.KnownByName(profile.m_sIdentity, t.m_iFaction, profile.m_iLives);
			}
			case "owed":
			{
				RZ_Checkpoints posts = gm.GetCheckpoints();
				return RZ_Factions.IsValid(t.m_iFaction) && posts && posts.GetParley() && posts.GetParley().IsOwed(pid, t.m_iFaction);
			}
			case "setword":
			{
				profile.EnsureArrays();
				return RZ_Factions.IsValid(t.m_iFaction) && !profile.m_aSetWord[t.m_iFaction].IsEmpty();
			}
			case "radio":
				return ctx.m_bRadio;
			case "truce":
				return RelationTerm(ctx, t, true);
			case "feud":
				return RelationTerm(ctx, t, false);
		}

		// The season's story, the infected, the radio, kit marks (RZ_DialogueWorld)
		bool known;
		bool v = RZ_DialogueWorld.Term(ctx, t, known);
		if (known)
			return v;

		// What's going on: the news, the trucks, the weather, the majors' acts (RZ_DialogueNews)
		bool news;
		bool w = RZ_DialogueNews.Term(ctx, t, news);
		return news && w;
	}

	//------------------------------------------------------------------------------------------------
	protected static string OpOr(RZ_DTerm t)
	{
		if (t.m_sOp.IsEmpty())
			return ">";

		return t.m_sOp;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Capital(string s)
	{
		if (s.IsEmpty())
			return s;

		string first = s.Substring(0, 1);
		first.ToUpper();
		return first + s.Substring(1, s.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	static int TraitKey(string key)
	{
		switch (key)
		{
			case "greedy": return RZ_ELeaderTrait.GREEDY;
			case "paranoid": return RZ_ELeaderTrait.PARANOID;
			case "merciful": return RZ_ELeaderTrait.MERCIFUL;
			case "ruthless": return RZ_ELeaderTrait.RUTHLESS;
			case "idealist": return RZ_ELeaderTrait.IDEALIST;
			case "pragmatist": return RZ_ELeaderTrait.PRAGMATIST;
			case "veteran": return RZ_ELeaderTrait.VETERAN;
			case "talker": return RZ_ELeaderTrait.TALKER;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static int StageKey(string key)
	{
		string k = key;
		k.Replace("+", "");
		switch (k)
		{
			case "fever": return RZ_EInfectionStage.FEVER;
			case "weakness": return RZ_EInfectionStage.WEAKNESS;
			case "final": return RZ_EInfectionStage.FINAL;
		}

		return 99;
	}

	//------------------------------------------------------------------------------------------------
	protected static int OutbreakKey(string key)
	{
		switch (key)
		{
			case "early": return RZ_EOutbreakStage.EARLY_OUTBREAK;
			case "collapse": return RZ_EOutbreakStage.COLLAPSE;
			case "overrun": return RZ_EOutbreakStage.OVERRUN;
			case "aftermath": return RZ_EOutbreakStage.AFTERMATH;
		}

		return 99;
	}

	//------------------------------------------------------------------------------------------------
	//! You've talked to this speaker before this life: a leader's or wanderer's built memory, or the
	//! talk memory's note `met` that every conversation leaves (RZ_DialogueTrees.MarkMet), so `met`
	//! works for every speaker (occupants, residents, named people, strangers, posts, the table).
	static bool Met(RZ_DCtx ctx)
	{
		if (MemoryBit(ctx, 1))
			return true;

		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		return trees && !ctx.m_sKey.IsEmpty() && trees.GetMemory().Has(ctx.m_iPlayer, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|met");
	}

	//------------------------------------------------------------------------------------------------
	//! met 1, helped 2, saved 4, crossed 8: the built memory bits of the leader or wanderer, from
	//! this life only (earlier lives are voiced once as the name: RZ_RepVoice).
	protected static bool MemoryBit(RZ_DCtx ctx, int which)
	{
		string ident = ctx.m_GM.GetProfile(ctx.m_iPlayer).m_sIdentity;
		int life = ctx.m_GM.GetProfile(ctx.m_iPlayer).m_iLives;
		if (ctx.m_Leader)
		{
			if (ctx.m_Leader.MemoryLife(ident) != life)
				return false;

			int lmem = ctx.m_Leader.GetMemory(ident);
			switch (which)
			{
				case 1: return (lmem & RZ_Leaders.MEM_MET) != 0;
				case 2: return (lmem & RZ_Leaders.MEM_HELPED) != 0;
				case 4: return (lmem & RZ_Leaders.MEM_SAVED) != 0;
				case 8: return (lmem & RZ_Leaders.MEM_CROSSED) != 0;
			}

			return false;
		}

		if (ctx.m_Wanderer)
		{
			if (ctx.m_Wanderer.MemoryLife(ident) != life)
				return false;

			int wmem = ctx.m_Wanderer.GetMemory(ident);
			switch (which)
			{
				case 1: return wmem != 0;
				case 2: return (wmem & RZ_Wanderers.MEM_HELPED) != 0;
				case 4: return (wmem & RZ_Wanderers.MEM_SAVED) != 0;
				case 8: return (wmem & RZ_Wanderers.MEM_WRONGED) != 0;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! house:short (the last meal ran short), house:fed (food for two meals or more), house:died
	//! (someone died there lately), house:medic, house:empty.
	protected static bool HouseTerm(RZ_DCtx ctx, string what)
	{
		RZ_WorldFacts facts = ctx.m_GM.GetFacts();
		if (!facts || ctx.m_iHouse < 0)
			return false;

		RZ_HouseFacts h = facts.House(ctx.m_iHouse);
		if (!h.m_bValid)
			return false;

		switch (what)
		{
			case "short": return h.m_bShort;
			case "fed": return !h.m_bShort && h.m_iFood >= 2 * Math.Max(1, h.m_iPeople / 4);
			case "died": return h.m_iDied > 0;
			case "medic": return h.m_bHasMedic;
			case "empty": return h.m_bEmpty;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! town:held (somebody holds the nearest town), town:held=redhand, town:infection>=60.
	protected static bool TownTerm(RZ_DCtx ctx, RZ_DTerm t)
	{
		RZ_WorldFacts facts = ctx.m_GM.GetFacts();
		if (!facts || ctx.m_iTown < 0)
			return false;

		if (t.m_sArg == "held")
		{
			RZ_TownOwner owner = facts.TownOwner(ctx.m_iTown);
			if (t.m_sOp.IsEmpty())
				return owner.IsHeld();

			return owner.IsHeldBy(RZ_DialogueDB.FactionKey(t.m_sVal));
		}

		if (t.m_sArg == "infection")
		{
			RZ_Settlement s = ctx.m_GM.GetSettlement(ctx.m_iTown);
			return s && Cmp(RZ_Util.RoundToInt(s.m_fInfection), t.m_sOp, t.m_iVal);
		}

		// The town's state (town:safe, infested, overrun) and its town memory (looted, burned, barricaded, powered)
		RZ_Settlement here = ctx.m_GM.GetSettlement(ctx.m_iTown);
		RZ_TownMemory towns = ctx.m_GM.GetTownMemory();
		switch (t.m_sArg)
		{
			case "safe": return here && here.GetState() == RZ_ESettlementState.SAFE;
			case "infested": return here && here.GetState() >= RZ_ESettlementState.INFESTED;
			case "overrun": return here && here.GetState() == RZ_ESettlementState.OVERRUN;
			case "looted": return towns && towns.IsEnabled() && towns.HasFlag(ctx.m_iTown, RZ_TownMemory.FLAG_LOOTED);
			case "burned": return towns && towns.IsEnabled() && towns.HasFlag(ctx.m_iTown, RZ_TownMemory.FLAG_BURNED);
			case "barricaded": return towns && towns.IsEnabled() && towns.HasFlag(ctx.m_iTown, RZ_TownMemory.FLAG_BARRICADED);
			case "powered": return towns && towns.IsEnabled() && towns.HasFlag(ctx.m_iTown, RZ_TownMemory.FLAG_POWERED);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! need:food, need:supplies, need:security, need:samples, need:people, need:revenge: what the
	//! speaker's faction needs now (faction life).
	protected static bool NeedTerm(RZ_DCtx ctx, string what)
	{
		RZ_FactionLife life = ctx.m_GM.GetFactionLife();
		if (!life || !RZ_Factions.IsValid(ctx.m_iFaction))
			return false;

		int need = life.GetNeed(ctx.m_iFaction);
		switch (what)
		{
			case "food": return need == RZ_ENeed.FOOD;
			case "supplies": return need == RZ_ENeed.SUPPLIES;
			case "security": return need == RZ_ENeed.SECURITY;
			case "samples": return need == RZ_ENeed.SAMPLES;
			case "people": return need == RZ_ENeed.PEOPLE;
			case "revenge": return need == RZ_ENeed.REVENGE;
			case "any": return need != RZ_ENeed.NONE;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! table:guest, table:family, table:crossed: the Long Table's three words, as built.
	protected static bool TableTerm(RZ_DCtx ctx, string what)
	{
		RZ_LongTable table = ctx.m_GM.GetLongTable();
		if (!table)
			return false;

		switch (what)
		{
			case "guest": return table.Remembers(ctx.m_iPlayer, RZ_LongTable.MEM_GUEST);
			case "family": return table.Remembers(ctx.m_iPlayer, RZ_LongTable.MEM_FAMILY);
			case "crossed": return table.Remembers(ctx.m_iPlayer, RZ_LongTable.MEM_CROSSED);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! truce:<f>, feud:<f>: the speaker's faction and the one named, now (faction life).
	protected static bool RelationTerm(RZ_DCtx ctx, RZ_DTerm t, bool truce)
	{
		RZ_FactionLife life = ctx.m_GM.GetFactionLife();
		if (!life || !RZ_Factions.IsValid(ctx.m_iFaction) || !RZ_Factions.IsValid(t.m_iFaction))
			return false;

		if (truce)
			return life.HasTruce(ctx.m_iFaction, t.m_iFaction);

		return life.HasFeud(ctx.m_iFaction, t.m_iFaction);
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_Disguise Disguise(RZ_GameModeComponent gm)
	{
		RZ_Hostility hostility = gm.GetHostility();
		if (!hostility)
			return null;

		return hostility.GetDisguise();
	}

	//------------------------------------------------------------------------------------------------
	//! {cover:name}, {cover:dead}, {cover:crew}, {cover:notebook}, {cover:paper}: the cover detail as
	//! you'd say it to the speaker's faction ("Kral", "Magpie", "31"), "" if you have none.
	static string CoverWords(RZ_DCtx ctx, string text)
	{
		RZ_Disguise d = Disguise(ctx.m_GM);
		array<string> keys = {RZ_DisguiseClaims.KEY_NAME, RZ_DisguiseClaims.KEY_DEAD, RZ_DisguiseClaims.KEY_CREW, RZ_DisguiseClaims.KEY_NOTEBOOK, RZ_DisguiseClaims.KEY_PAPER};
		string s = text;
		foreach (string key : keys)
		{
			string words = "";
			if (d && RZ_Factions.IsValid(ctx.m_iFaction))
				words = d.GetClaims().SpokenWords(ctx.m_iPlayer, ctx.m_iFaction, key);

			s.Replace("{cover:" + key + "}", words);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! kit:own (the speaker faction's kit, worn as your own colours), kit:in_kit (their kit, below
	//! Known with them), kit:full (their full kit), kit:other (another faction's colours), kit:any.
	protected static bool KitTerm(RZ_DCtx ctx, string what)
	{
		RZ_Disguise d = Disguise(ctx.m_GM);
		if (!d || !d.IsOn())
			return false;

		int pid = ctx.m_iPlayer;
		int f = ctx.m_iFaction;
		if (what == "any")
			return d.Wearing(pid) >= 0;

		if (what == "other")
		{
			for (int o = 0; o < RZ_Factions.COUNT; o++)
			{
				if (o != f && d.KitLevel(pid, o) > 0)
					return true;
			}

			return false;
		}

		if (!RZ_Factions.IsValid(f))
			return false;

		switch (what)
		{
			case "own": return d.KitLevel(pid, f) > 0 && d.IsOwnColours(pid, f);
			case "in_kit": return d.InTheirKit(pid, f);
			case "full": return d.KitLevel(pid, f) >= 2;
		}

		// What the kit remembers (RZ_DisguiseMarks): issued, signed_other, dead_mans, off_turned
		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
		if (!me || !d.MarksOn() || d.KitLevel(pid, f) <= 0)
			return false;

		RZ_KitMark mark;
		int kind = d.GetMarks().KindOf(me, pid, f, mark);
		switch (what)
		{
			case "issued": return kind == RZ_DisguiseMarks.KIT_ISSUED;
			case "signed_other": return kind == RZ_DisguiseMarks.KIT_SIGNED_OTHER;
			case "dead_mans": return kind == RZ_DisguiseMarks.KIT_DEAD;
			case "off_turned": return kind == RZ_DisguiseMarks.KIT_TURNED;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The player's route at the 90% lock (docs/FINALE.md): none, any, in (the speaker's faction's
	//! route), out (another faction's route), independent, unsided (no route while the routes are
	//! open), won and lost (after the cure, the route that controls it or not), or a route by name.
	protected static bool RouteTerm(RZ_DCtx ctx, string what)
	{
		RZ_CureFinale fin = ctx.m_GM.GetFinale();
		int r = RZ_CureFinale.NONE;
		if (fin && RZ_CureFinale.IsOn())
			r = fin.RouteOfPlayer(ctx.m_iPlayer);

		bool cured = ctx.m_GM.GetCure() && ctx.m_GM.GetCure().IsCured();
		switch (what)
		{
			case "none": return r == RZ_CureFinale.NONE;
			case "any": return r != RZ_CureFinale.NONE;
			// The lock ends at the cure: in, out and independent are the locked weeks only
			case "in": return !cured && r != RZ_CureFinale.NONE && r != RZ_CureFinale.INDEP && r == ctx.m_iFaction;
			case "out": return !cured && r != RZ_CureFinale.NONE && r != RZ_CureFinale.INDEP && r != ctx.m_iFaction;
			case "independent": return !cured && r == RZ_CureFinale.INDEP;
			case "unsided": return r == RZ_CureFinale.NONE && fin && fin.IsOpen();
			case "won": return cured && fin && r != RZ_CureFinale.NONE && fin.Controller() == r;
			case "lost": return cured && fin && r != RZ_CureFinale.NONE && fin.Controller() != r;
			case "haven": return r == RZ_CureFinale.HAVEN;
			case "bastion": return r == RZ_CureFinale.BASTION;
			case "redhand": return r == RZ_CureFinale.REDHAND;
			case "halcyon": return r == RZ_CureFinale.HALCYON;
			case "indep": return r == RZ_CureFinale.INDEP;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Other players within 8 m.
	static int OthersNear(RZ_DCtx ctx)
	{
		if (!ctx.m_User)
			return 0;

		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);
		int count = 0;
		foreach (int id : ids)
		{
			if (id == ctx.m_iPlayer)
				continue;

			IEntity e = GetGame().GetPlayerManager().GetPlayerControlledEntity(id);
			if (e && RZ_Util.DistXZ(e.GetOrigin(), ctx.m_User.GetOrigin()) <= 8)
				count++;
		}

		return count;
	}
}

//------------------------------------------------------------------------------------------------
//! A conversation line's tokens, then the world's (RZ_WorldFacts.Fill) and the player's
//! (RZ_RepVoice.FillFor).
class RZ_DialogueText
{
	//------------------------------------------------------------------------------------------------
	static string Fill(RZ_DCtx ctx, string text)
	{
		if (!text.Contains("{"))
			return text;

		string s = text;
		s.Replace("{npc}", ctx.m_sName);
		s.Replace("{npc_short}", ctx.m_sShort);
		s.Replace("{role}", ctx.m_sRole);
		s.Replace("{place}", ctx.m_sPlace);
		s.Replace("{here}", ctx.m_sPlace);
		s.Replace("{player}", RZ_Util.PlayerName(ctx.m_iPlayer));
		s.Replace("{faction}", RZ_Factions.GetName(ctx.m_iFaction));
		if (ctx.m_Stranger && s.Contains("{their:"))
			s = TheirText(ctx.m_Stranger, s);

		if (s.Contains("{title:"))
			s = Titles(ctx, s);

		if (s.Contains("{wire:"))
			s = RZ_DialogueWire.Fill(ctx.m_GM, s);

		if (s.Contains("{cover:"))
			s = RZ_DialogueCond.CoverWords(ctx, s);
		if (s.Contains("{toll}"))
			s.Replace("{toll}", ctx.m_GM.GetCheckpoints().TollFor(ctx.m_iPlayer).ToString());

		if (s.Contains("{house}") && ctx.m_iHouse >= 0 && ctx.m_GM.GetFacts())
			s.Replace("{house}", ctx.m_GM.GetFacts().House(ctx.m_iHouse).m_sTown);

		if (s.Contains("{boss}"))
		{
			RZ_Leader boss;
			if (ctx.m_GM.GetLeaders() && RZ_Factions.IsValid(ctx.m_iFaction))
				boss = ctx.m_GM.GetLeaders().GetStronghold(ctx.m_iFaction);

			if (boss)
				s.Replace("{boss}", boss.m_sShort);
			else
				s.Replace("{boss}", "the boss");
		}

		s = RZ_DialogueWorld.Fill(ctx, s);
		s = RZ_RepVoice.FillFor(ctx.m_GM, ctx.m_iPlayer, ctx.m_iFaction, s);
		if (ctx.m_GM.GetFacts())
			s = ctx.m_GM.GetFacts().Fill(s);

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! {their:town}: a stranger's own words (the errand routine's texts: town, house, name...).
	protected static string TheirText(RZ_StrangerVisit v, string text)
	{
		string s = text;
		foreach (string key, string value : v.m_mText)
		{
			s.Replace("{their:" + key + "}", value);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! {title:bastion}: the name this player's best life earned with that faction this season.
	protected static string Titles(RZ_DCtx ctx, string text)
	{
		RZ_RepSeason rec = RZ_RepSeason.Get();
		string ident = ctx.m_GM.GetProfile(ctx.m_iPlayer).m_sIdentity;
		string s = text;
		array<string> words = {"haven", "bastion", "redhand", "halcyon"};
		foreach (string w : words)
		{
			string token = "{title:" + w + "}";
			if (!s.Contains(token))
				continue;

			string title = "";
			if (rec)
				title = rec.TitleFor(ident, RZ_DialogueDB.FactionKey(w));

			if (title.IsEmpty())
				title = "a stranger";

			s.Replace(token, title);
		}

		return s;
	}
}
