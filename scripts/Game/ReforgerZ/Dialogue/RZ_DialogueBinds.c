//! Bindings, what you told someone, and the world's own tests of a lie (docs/DIALOGUE.md, "The text
//! format" and "Lies you're told"). Server only.
//!
//! - `bind <name> = <picker>`: a place picked once when the topic opens and kept for the session:
//!   `town:nearest`, `town:thick_within(<m>)` (the nearest town thick with them, 60 or more), `house:nearest`,
//!   `house:nearest_short` (the nearest safehouse that ran short). `{bind:<name>}` (or `{town:@<name>}`)
//!   says it; `@<name>` in a key binds the key to it, as `@here` binds it to the speaker.
//! - `{claim:<key>}`: what you told this speaker for that claim, in your own words.
//! - `watch:<key>@<name>:<infection>`: a world predicate. The truth of a lie about a bound town is
//!   learned the moment you walk into that town while its infection is at least that (the truth key is
//!   `<key>@<bound place>`). Kept for 3 real hours; not saved.

//------------------------------------------------------------------------------------------------
class RZ_DWatch
{
	int m_iPlayer;
	string m_sIdent;
	string m_sKey;
	int m_iTown;
	int m_iInfection;
	int m_iUntil;
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueBinds
{
	protected static const int WATCH_SECONDS = 10800;
	protected static const int WATCH_MAX = 64;
	protected static ref array<ref RZ_DWatch> s_aWatches = {};

	//------------------------------------------------------------------------------------------------
	//! A topic opens: its bindings are picked (and any from the last topic are forgotten).
	static void Pick(RZ_DCtx ctx, RZ_DConv c)
	{
		ctx.m_mBind.Clear();
		foreach (int i, string name : c.m_aBindNames)
		{
			string value = Picked(ctx, c.m_aBindPickers[i]);
			if (!value.IsEmpty())
				ctx.m_mBind.Set(name, value);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "town:N" or "house:N", "" when nothing fits.
	protected static string Picked(RZ_DCtx ctx, string picker)
	{
		RZ_GameModeComponent gm = ctx.m_GM;
		vector at = ctx.Pos();
		if (picker == "town:nearest")
		{
			if (ctx.m_iTown < 0)
				return "";

			return "town:" + ctx.m_iTown.ToString();
		}

		if (picker.StartsWith("town:thick_within("))
		{
			float within = picker.Substring(18, picker.Length() - 19).ToFloat();
			int best = -1;
			float bestDist = within;
			foreach (int i, RZ_Settlement s : gm.GetSettlements())
			{
				float d = RZ_Util.DistXZ(s.m_vCenter, at);
				if (s.m_fInfection >= 60 && d <= bestDist)
				{
					bestDist = d;
					best = i;
				}
			}

			if (best < 0)
				return "";

			return "town:" + best.ToString();
		}

		RZ_Safehouses houses = gm.GetSafehouses();
		if (!houses)
			return "";

		if (picker == "house:nearest")
		{
			int near = houses.FindNearest(at, 3000);
			if (near < 0)
				return "";

			return "house:" + near.ToString();
		}

		if (picker == "house:nearest_short")
		{
			int shortest = -1;
			float shortDist = 3000;
			for (int h = 0; h < 64; h++)
			{
				RZ_Safehouse house = houses.Get(h);
				if (!house)
					break;

				float hd = RZ_Util.DistXZ(house.m_vPos, at);
				if (house.m_bShort && hd <= shortDist)
				{
					shortDist = hd;
					shortest = h;
				}
			}

			if (shortest < 0)
				return "";

			return "house:" + shortest.ToString();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! A bound value's name for players.
	protected static string NameOf(RZ_GameModeComponent gm, string value)
	{
		int colon = value.IndexOf(":");
		if (colon <= 0)
			return "somewhere near";

		int idx = value.Substring(colon + 1, value.Length() - colon - 1).ToInt();
		if (value.StartsWith("town:"))
		{
			RZ_Settlement s = gm.GetSettlement(idx);
			if (s)
				return s.m_sName;
		}
		else if (gm.GetSafehouses())
		{
			RZ_Safehouse h = gm.GetSafehouses().Get(idx);
			if (h)
				return h.m_sName;
		}

		return "somewhere near";
	}

	//------------------------------------------------------------------------------------------------
	//! {bind:<name>}, {town:@<name>}, {claim:<key>}.
	static string Fill(RZ_DCtx ctx, string text)
	{
		string s = text;
		int guard = 0;
		int at = FirstToken(s);
		while (at >= 0 && guard < 8)
		{
			guard++;
			int close = s.IndexOfFrom(at, "}");
			if (close < 0)
				break;

			string token = s.Substring(at, close - at + 1);
			string inner = token.Substring(1, token.Length() - 2);
			string value = "somewhere near";
			if (inner.StartsWith("bind:") || inner.StartsWith("town:@"))
			{
				string name = inner.Substring(inner.IndexOf(":") + 1, inner.Length() - inner.IndexOf(":") - 1);
				name.Replace("@", "");
				string bound;
				if (ctx.m_mBind.Find(name, bound))
					value = NameOf(ctx.m_GM, bound);
			}
			else
			{
				value = ClaimText(ctx, inner.Substring(6, inner.Length() - 6));
			}

			s.Replace(token, value);
			at = FirstToken(s);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	protected static int FirstToken(string s)
	{
		int best = -1;
		array<string> heads = {"{bind:", "{town:@", "{claim:"};
		foreach (string head : heads)
		{
			int i = s.IndexOf(head);
			if (i >= 0 && (best < 0 || i < best))
				best = i;
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! What this player told this speaker for a claim, in their own words ("what you said" if none).
	protected static string ClaimText(RZ_DCtx ctx, string key)
	{
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		string said = "";
		if (trees)
			said = trees.GetMemory().FindAfter(ctx.m_iPlayer, RZ_DialogueMemory.NOTES, ctx.m_sKey + "|said." + key + "|");

		if (said.IsEmpty())
			return "what you said";

		return said;
	}

	// ================================================================ World predicates
	//------------------------------------------------------------------------------------------------
	//! watch:<key>@<name>:<infection>
	static void Watch(RZ_TalkSession s, string arg)
	{
		RZ_DCtx ctx = s.m_Tree.m_Ctx;
		int colon = arg.LastIndexOf(":");
		int amp = arg.IndexOf("@");
		if (colon <= 0 || amp <= 0 || amp > colon)
			return;

		string name = arg.Substring(amp + 1, colon - amp - 1);
		string bound;
		if (!ctx.m_mBind.Find(name, bound) || !bound.StartsWith("town:"))
			return;

		RZ_DWatch w = new RZ_DWatch();
		w.m_iPlayer = s.m_iPlayer;
		w.m_sIdent = ctx.m_GM.GetProfile(s.m_iPlayer).m_sIdentity;
		w.m_sKey = RZ_DialogueTrees.Scoped(ctx, arg.Substring(0, colon));
		w.m_iTown = bound.Substring(5, bound.Length() - 5).ToInt();
		w.m_iInfection = arg.Substring(colon + 1, arg.Length() - colon - 1).ToInt();
		w.m_iUntil = RZ_SeasonClock.Now() + WATCH_SECONDS;
		if (s_aWatches.Count() >= WATCH_MAX)
			s_aWatches.Remove(0);

		s_aWatches.Insert(w);
	}

	//------------------------------------------------------------------------------------------------
	//! On the dialogue tick: a watched town walked into while it holds teaches the truth.
	static void Tick(RZ_GameModeComponent gm)
	{
		if (s_aWatches.IsEmpty())
			return;

		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		int now = RZ_SeasonClock.Now();
		for (int i = s_aWatches.Count() - 1; i >= 0; i--)
		{
			RZ_DWatch w = s_aWatches[i];
			if (now > w.m_iUntil)
			{
				s_aWatches.Remove(i);
				continue;
			}

			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(w.m_iPlayer);
			RZ_Settlement town = gm.GetSettlement(w.m_iTown);
			if (!me || !town || gm.GetProfile(w.m_iPlayer).m_sIdentity != w.m_sIdent)
				continue;

			if (RZ_Util.DistXZ(me.GetOrigin(), town.m_vCenter) > town.m_fRadius || town.m_fInfection < w.m_iInfection)
				continue;

			trees.GetMemory().Learn(w.m_iPlayer, w.m_sKey);
			gm.NotifyPlayer(w.m_iPlayer, town.m_sName, "It's thick with them. Whoever told you otherwise was wrong, or lying.");
			RZ_Log.Info(string.Format("Dialogue: %1 saw for themselves: %2", RZ_Util.PlayerName(w.m_iPlayer), w.m_sKey));
			s_aWatches.Remove(i);
		}
	}
}
