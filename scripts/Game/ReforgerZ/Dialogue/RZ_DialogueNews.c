//! What's going on, for conversations (docs/DIALOGUE.md, "Conditions" and "The text format"): the
//! news a speaker can know, read from the world facts and the world memory, never stored here.
//! Server only.
//!
//! - `news:<kind>` (a condition) holds when the picker of the same name finds something; `bind <name>
//!   = news:<kind>` (a picker) names it in a line through `{bind:<name>}`. Kinds: taken, taken_by,
//!   taken_from (the newest outpost that changed hands, who took it, who lost it), won and lost (the
//!   speaker's faction's newest), won_by(<faction>) and lost_by(<faction>), burned, emptied, rescue
//!   (a town's log, two season days), overrun (the nearest town gone to the infected), shelled, hungry
//!   (the nearest house on half portions), truck (the newest truck that didn't arrive), leader_dead and
//!   leader_dead_at (who and where), death, bite and horde (near the speaker, a season day).
//! - `supply:running|road|short|lost`: the speaker faction's trucks run; one is on the road; one of its
//!   places is short; one of its trucks was lost in the last season day. Pickers `supply:short` and
//!   `supply:truck` name the place.
//! - `town:held_by(<faction>)` and `town:quiet_within(<m>)` (pickers): the nearest town that faction
//!   holds; the nearest quiet town that isn't the speaker's.
//! - `persona:1..3`: which of three ways this one person talks (fixed by their memory key), so two
//!   quartermasters or two lookouts don't sound alike. `rain`, `fog`, `edge` (the speaker is within
//!   1500 m of the Perimeter's line), `stand` (the lab's final stand), `major:<key>[op act]`,
//!   `holds:<faction>[op N]`.
//! - What you did with a faction, so a line can say it (`{best}`, `{worst}`, `{jobs}`, `{kills}`):
//!   `best:<faction>` and `worst:<faction>` (a deed to name), `jobs:<faction>>=N`, `kills:<faction>>=N`.
class RZ_DialogueNews
{
	protected static const float NEAR = 3000;		// m: news "near" the speaker
	protected static const float RECENT_DAYS = 2;	// season days a town's log counts as news
	protected static const float SIGNAL_DAYS = 1;	// season days a signal counts as news

	//------------------------------------------------------------------------------------------------
	//! One of this file's condition keys. `known` is false for a key this file doesn't own.
	static bool Term(RZ_DCtx ctx, RZ_DTerm t, out bool known)
	{
		known = true;
		switch (t.m_sKey)
		{
			case "news": return !Pick(ctx, "news:" + t.m_sArg).IsEmpty();
			case "supply": return Supply(ctx, t.m_sArg);
			case "persona": return Persona(ctx) == t.m_sArg.ToInt();
			case "rain": return RZ_Weather.Rain() >= 0.3;
			case "fog": return RZ_Weather.Fog() >= 0.4;
			case "edge": return NearEdge(ctx);
			case "stand":
			{
				RZ_WorldFacts facts = ctx.m_GM.GetFacts();
				return facts && facts.IsFinalStand();
			}
			case "major": return Major(ctx, t);
			case "holds": return Holds(ctx, t);
			case "best":
			case "worst":
			case "jobs":
			case "kills":
				return Deeds(ctx, t);
		}

		known = false;
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 1, 2 or 3 for this speaker, from their memory key.
	static int Persona(RZ_DCtx ctx)
	{
		return Math.AbsInt(ctx.m_sKey.Hash()) % 3 + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A picker this file owns: "town:N", "house:N", "text:<words>", or "" when nothing fits (or the
	//! picker isn't this file's).
	static string Pick(RZ_DCtx ctx, string picker)
	{
		RZ_GameModeComponent gm = ctx.m_GM;
		RZ_WorldFacts facts = gm.GetFacts();
		if (!facts)
			return "";

		if (picker.StartsWith("supply:"))
			return SupplyPick(ctx, picker.Substring(7, picker.Length() - 7));

		if (picker.StartsWith("town:held_by("))
			return HeldBy(ctx, RZ_DialogueDB.FactionKey(Inner(picker)));

		if (picker.StartsWith("town:quiet_within("))
			return QuietWithin(ctx, Inner(picker).ToFloat());

		if (!picker.StartsWith("news:"))
			return "";

		string kind = picker.Substring(5, picker.Length() - 5);
		RZ_WorldMemory mem = facts.Memory();
		if (kind.StartsWith("won_by("))
			return ReasonTown(Newest(mem, "holding.taken", RZ_DialogueDB.FactionKey(Inner(kind))));

		if (kind.StartsWith("lost_by("))
			return ReasonTown(Newest(mem, "holding.lost", RZ_DialogueDB.FactionKey(Inner(kind))));

		switch (kind)
		{
			case "taken": return ReasonTown(Newest(mem, "holding.taken", -1));
			case "taken_by":
			{
				RZ_Reason took = Newest(mem, "holding.taken", -1);
				if (!took || !RZ_Factions.IsValid(took.faction))
					return "";

				return "text:" + RZ_Factions.GetNameInText(took.faction);
			}
			case "taken_from":
			{
				RZ_Reason newest = Newest(mem, "holding.taken", -1);
				if (!newest)
					return "";

				RZ_Reason lost = mem.FindReason("holding.lost", newest.subject);
				if (!lost || !RZ_Factions.IsValid(lost.faction))
					return "";

				return "text:" + RZ_Factions.GetNameInText(lost.faction);
			}
			case "won": return ReasonTown(Newest(mem, "holding.taken", OwnFaction(ctx)));
			case "lost": return ReasonTown(Newest(mem, "holding.lost", OwnFaction(ctx)));
			case "burned": return LogTown(gm, mem, "burned", false);
			case "emptied": return LogTown(gm, mem, "emptied", false);
			case "rescue": return LogTown(gm, mem, "rescue", true);
			case "overrun": return ReasonTown(Nearest(ctx, mem, "town.infested"));
			case "shelled": return ReasonTown(Newest(mem, "town.shelled", -1));
			case "hungry":
			{
				RZ_Reason hungry = Nearest(ctx, mem, "house.hungry");
				if (!hungry || !hungry.subject.StartsWith("safehouse:"))
					return "";

				return "house:" + hungry.subject.Substring(10, hungry.subject.Length() - 10);
			}
			case "truck": return SignalText(ctx, mem, "truck.", false);
			case "leader_dead": return DeadLeader(mem, true);
			case "leader_dead_at": return DeadLeader(mem, false);
			case "death": return SignalTown(ctx, mem, "death");
			case "bite": return SignalTown(ctx, mem, "bite");
			case "horde": return SignalTown(ctx, mem, "horde");
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! "town:held_by(redhand)" -> "redhand"
	protected static string Inner(string picker)
	{
		int open = picker.IndexOf("(");
		int close = picker.LastIndexOf(")");
		if (open < 0 || close <= open)
			return "";

		return picker.Substring(open + 1, close - open - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected static int OwnFaction(RZ_DCtx ctx)
	{
		if (!RZ_Factions.IsValid(ctx.m_iFaction))
			return -3;		// matches no reason

		return ctx.m_iFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! The newest live reason of `key` (faction -1 any, -3 none, else that faction's).
	protected static RZ_Reason Newest(RZ_WorldMemory mem, string key, int faction)
	{
		array<RZ_Reason> all = {};
		mem.AllReasons(all);
		RZ_Reason best;
		float now = RZ_SeasonClock.Days();
		foreach (RZ_Reason r : all)
		{
			if (r.key != key || r.untilDay < now)
				continue;

			if (faction >= 0 && r.faction != faction)
				continue;

			if (faction == -3)
				continue;

			if (!best || r.madeDay > best.madeDay)
				best = r;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The live reason of `key` nearest the speaker, within NEAR.
	protected static RZ_Reason Nearest(RZ_DCtx ctx, RZ_WorldMemory mem, string key)
	{
		array<RZ_Reason> near = {};
		mem.ReasonsNear(ctx.Pos(), NEAR, near, key);
		RZ_Reason best;
		float bestDist = float.MAX;
		float now = RZ_SeasonClock.Days();
		foreach (RZ_Reason r : near)
		{
			if (r.untilDay < now)
				continue;

			float d = RZ_Util.DistXZ(ctx.Pos(), Vector(r.x, 0, r.z));
			if (d < bestDist)
			{
				bestDist = d;
				best = r;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A reason about a town ("town:N"): that town.
	protected static string ReasonTown(RZ_Reason r)
	{
		if (!r || !r.subject.StartsWith("town:"))
			return "";

		return r.subject;
	}

	//------------------------------------------------------------------------------------------------
	//! The town whose log slot (best or worst) is of `kind` and newest, within RECENT_DAYS.
	protected static string LogTown(RZ_GameModeComponent gm, RZ_WorldMemory mem, string kind, bool best)
	{
		int found = -1;
		float foundDay = -1;
		float now = RZ_SeasonClock.Days();
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			RZ_TownEntry e;
			if (best)
				e = mem.TownBest(i);
			else
				e = mem.TownWorst(i);

			if (!e || e.kind != kind || now - e.day > RECENT_DAYS || e.day <= foundDay)
				continue;

			found = i;
			foundDay = e.day;
		}

		if (found < 0)
			return "";

		return "town:" + found.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The town nearest the newest signal of `kind` near the speaker.
	protected static string SignalTown(RZ_DCtx ctx, RZ_WorldMemory mem, string kind)
	{
		array<RZ_Signal> sigs = {};
		mem.RecentSignals(kind, ctx.Pos(), NEAR, SIGNAL_DAYS, sigs);
		if (sigs.IsEmpty())
			return "";

		int town = ctx.m_GM.FindNearestSettlement(Vector(sigs[0].x, 0, sigs[0].z));
		if (town < 0)
			return "";

		return "town:" + town.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The newest signal whose kind starts with `prefix` (anywhere): its text. Signal texts are
	//! written to be shown (the cause never is).
	protected static string SignalText(RZ_DCtx ctx, RZ_WorldMemory mem, string prefix, bool ownOnly)
	{
		array<RZ_Signal> sigs = {};
		mem.RecentSignals("", ctx.Pos(), float.MAX, SIGNAL_DAYS, sigs);
		foreach (RZ_Signal s : sigs)
		{
			if (!s.kind.StartsWith(prefix) || s.text.IsEmpty())
				continue;

			if (ownOnly && s.kind != prefix + ctx.m_iFaction.ToString())
				continue;

			return "text:" + s.text;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The newest leader death: their name (name) or the place (else), from its reason's own line
	//! ("<name> is dead, in <place>.").
	protected static string DeadLeader(RZ_WorldMemory mem, bool name)
	{
		RZ_Reason r = Newest(mem, "leader.died", -1);
		if (!r)
			return "";

		int cut = r.text.IndexOf(" is dead, in ");
		if (cut <= 0)
			return "";

		if (name)
			return "text:" + r.text.Substring(0, cut);

		string place = r.text.Substring(cut + 13, r.text.Length() - cut - 13);
		place.Replace(".", "");
		return "text:" + place;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest town the faction holds.
	protected static string HeldBy(RZ_DCtx ctx, int faction)
	{
		RZ_WorldFacts facts = ctx.m_GM.GetFacts();
		if (!RZ_Factions.IsValid(faction))
			return "";

		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : ctx.m_GM.GetSettlements())
		{
			if (!facts.TownOwner(i).IsHeldBy(faction))
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, ctx.Pos());
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		if (best < 0)
			return "";

		return "town:" + best.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest town under 20 infection within `within` m, not the speaker's own.
	protected static string QuietWithin(RZ_DCtx ctx, float within)
	{
		int best = -1;
		float bestDist = within;
		foreach (int i, RZ_Settlement s : ctx.m_GM.GetSettlements())
		{
			if (i == ctx.m_iTown || s.m_fInfection >= 20)
				continue;

			float d = RZ_Util.DistXZ(s.m_vCenter, ctx.Pos());
			if (d <= bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		if (best < 0)
			return "";

		return "town:" + best.ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool Supply(RZ_DCtx ctx, string what)
	{
		RZ_SupplyLines lines = ctx.m_GM.GetSupply();
		if (!lines || !RZ_Factions.IsValid(ctx.m_iFaction))
			return false;

		switch (what)
		{
			case "running": return lines.IsFactionRunning(ctx.m_iFaction);
			case "road": return !lines.TruckDestination(ctx.m_iFaction).IsEmpty();
			case "short": return !lines.ShortPlace(ctx.m_iFaction).IsEmpty();
			case "lost":
			{
				RZ_WorldFacts facts = ctx.m_GM.GetFacts();
				return facts && !SignalText(ctx, facts.Memory(), "truck.", true).IsEmpty();
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string SupplyPick(RZ_DCtx ctx, string what)
	{
		RZ_SupplyLines lines = ctx.m_GM.GetSupply();
		if (!lines || !RZ_Factions.IsValid(ctx.m_iFaction))
			return "";

		string place = "";
		if (what == "short")
			place = lines.ShortPlace(ctx.m_iFaction);
		else if (what == "truck")
			place = lines.TruckDestination(ctx.m_iFaction);

		if (place.IsEmpty())
			return "";

		return "text:" + place;
	}

	//------------------------------------------------------------------------------------------------
	//! The speaker stands within 1500 m of the Perimeter's line (the drones, the loudspeakers).
	protected static bool NearEdge(RZ_DCtx ctx)
	{
		RZ_Perimeter line = ctx.m_GM.GetPerimeter();
		return line && line.IsActive() && line.EdgeDistance(ctx.Pos()) <= 1500;
	}

	//------------------------------------------------------------------------------------------------
	//! major:<key> while that major runs; major:<key>>=2 its act (the stage, from 1).
	protected static bool Major(RZ_DCtx ctx, RZ_DTerm t)
	{
		RZ_Questlines q = ctx.m_GM.GetQuestlines();
		if (!q)
			return false;

		RZ_QuestRun run = q.FindRunByKey(t.m_sArg);
		if (!run)
			return false;

		if (t.m_sOp.IsEmpty())
			return true;

		return RZ_DialogueWorld.Cmp(run.m_iStage + 1, t.m_sOp, t.m_iVal);
	}

	//------------------------------------------------------------------------------------------------
	//! best:<f>, worst:<f> (a deed reputation can name), jobs:<f>>=N, kills:<f>>=N (of theirs put down).
	protected static bool Deeds(RZ_DCtx ctx, RZ_DTerm t)
	{
		RZ_PlayerProfile p = ctx.m_GM.GetProfile(ctx.m_iPlayer);
		int f = RZ_DialogueDB.FactionKey(t.m_sArg);
		if (!p || !RZ_Factions.IsValid(f))
			return false;

		p.EnsureArrays();
		if (t.m_sKey == "best")
			return !p.m_aBestDeed[f].IsEmpty();

		if (t.m_sKey == "worst")
			return !RZ_Deeds.WorstText(p, f).IsEmpty();

		int n = p.m_aJobs[f];
		if (t.m_sKey == "kills")
			n = p.m_aKills[f];

		if (t.m_sOp.IsEmpty())
			return n > 0;

		return RZ_DialogueWorld.Cmp(n, t.m_sOp, t.m_iVal);
	}

	//------------------------------------------------------------------------------------------------
	//! holds:<faction>>=N: the holdings that faction has now (strongholds and outposts).
	protected static bool Holds(RZ_DCtx ctx, RZ_DTerm t)
	{
		RZ_Territory land = ctx.m_GM.GetTerritory();
		int f = RZ_DialogueDB.FactionKey(t.m_sArg);
		if (!land || !RZ_Factions.IsValid(f))
			return false;

		int n = 0;
		foreach (RZ_Holding h : land.GetHoldings())
		{
			if (h && h.m_eFaction == f)
				n++;
		}

		if (t.m_sOp.IsEmpty())
			return n > 0;

		return RZ_DialogueWorld.Cmp(n, t.m_sOp, t.m_iVal);
	}
}
