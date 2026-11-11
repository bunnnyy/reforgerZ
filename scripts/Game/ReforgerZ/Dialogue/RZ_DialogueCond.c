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
	int m_iFaction = -1;			// the speaker's faction, -1 none
	string m_sName;					// "Ines Calder"
	string m_sShort;				// "Calder", "Old Marta"
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
				return mem.Knows(pid, t.m_sArg);
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
				return facts && facts.Flag("tally." + t.m_sArg + "." + profile.m_sIdentity) > 0;
			case "mood":
				return facts && RZ_SeasonMood.Name(facts.SeasonMood()) == t.m_sArg;
			case "met":
				return MemoryBit(ctx, 1);
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
				return RZ_CureFinale.RouteTerm(t.m_sArg, pid);
			case "finale":
				return RZ_CureFinale.FinaleTerm(t.m_sArg, pid, ctx.m_Wanderer);
		}

		// Keys of designs not built yet (tribes, the majors, disguise, errands): quiet
		return false;
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

		s = RZ_RepVoice.FillFor(ctx.m_GM, ctx.m_iPlayer, ctx.m_iFaction, s);
		if (ctx.m_GM.GetFacts())
			s = ctx.m_GM.GetFacts().Fill(s);

		return s;
	}
}
