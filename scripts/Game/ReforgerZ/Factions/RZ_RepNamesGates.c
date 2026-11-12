//! Barks at gates and others hear (docs/REPUTATION.md, "What people call you"). Server only.
//!
//! - **A gate bark:** at a holding's gate, a road post or a safehouse door, within 25 m, the place says
//!   one line about what it makes of you, only on a change: your first visit this life, or a new name
//!   or best deed with that faction since your last bark there. From the place, never a body, so it
//!   never gives away a lookout you can't see. Never while you're in someone else's kit, never in the
//!   Hostile manner, and inside the approach lane (once per place per session).
//! - **Others hear:** at notoriety 3, non-hostile and in your own colours, other players within 30 m of
//!   the gate get it too ("A guard nods at Mara. 'That's the one who held the lab.'"), once per 10
//!   minutes per place for everyone together, under the names policy ("that one" for the anonymous).
//! Reputation > Gate barks and Others hear switch them. Checked every 5 s on the reputation watch.
class RZ_RepGates
{
	protected static const float EVERY = 5;
	protected static const float RADIUS = 25;
	protected static const float HEAR_RADIUS = 30;
	protected static const float OTHERS_GAP = 600;

	protected RZ_GameModeComponent m_GM;
	protected float m_fClock;
	protected float m_fNext;
	protected ref map<string, int> m_mLast = new map<string, int>();			// "ident|place" -> life*100000 + cell*1000 + deed hash
	protected ref map<string, float> m_mOthers = new map<string, float>();	// place -> clock of the last others-hear
	protected ref array<float> m_aCellSeconds = {};		// time online players spent in each of the 16 cells (all factions)
	protected float m_fCellLogAt;
	protected static const float CELL_LOG = 1800;

	//------------------------------------------------------------------------------------------------
	void RZ_RepGates(RZ_GameModeComponent gm)
	{
		m_GM = gm;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt)
	{
		m_fClock += dt;
		if (m_fClock < m_fNext)
			return;

		m_fNext = m_fClock + EVERY;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		CountCells(ids);

		RZ_ReputationSettings settings = RZ_ReputationSettings.Get();
		if (!settings.m_bGateBarks)
			return;

		foreach (int i, IEntity me : players)
		{
			if (RZ_Util.IsInVehicle(me))
				continue;

			int f;
			string place;
			vector at;
			if (!GateNear(me.GetOrigin(), f, place, at))
				continue;

			// The gate's own line: the bark, with a rival-colours remark folded in (one per approach)
			string colours = RivalColours(ids[i], f, place);
			if (!Bark(ids[i], me, f, place, at, ids, players, colours) && !colours.IsEmpty())
			{
				RZ_NoticeLanes colourLanes = RZ_NoticeLanes.Get();
				if (!colourLanes || colourLanes.TryGate(ids[i], "colours|" + place))
					m_GM.Say(ids[i], Who(f, place), f, colours);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Time spent per cell, to weight the writing toward where players live (Stranger, Known, Watched,
	//! Mixed Bag first): logged every 30 minutes as "Reputation: time per cell: ...".
	protected void CountCells(array<int> ids)
	{
		while (m_aCellSeconds.Count() < 16)
		{
			m_aCellSeconds.Insert(0);
		}

		foreach (int pid : ids)
		{
			RZ_PlayerProfile p = m_GM.GetProfile(pid);
			if (!p)
				continue;

			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				int cell = RZ_RepNames.GetRep(p.GetFame(f), p.GetInfamy(f));
				if (cell >= 0 && cell < 16)
					m_aCellSeconds[cell] = m_aCellSeconds[cell] + EVERY;
			}
		}

		if (m_fClock - m_fCellLogAt < CELL_LOG)
			return;

		m_fCellLogAt = m_fClock;
		string text = "";
		for (int c = 0; c < 16; c++)
		{
			if (m_aCellSeconds[c] <= 0)
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += string.Format("%1 %2 min", RZ_RepNames.GetName(c), RZ_Util.RoundToInt(m_aCellSeconds[c] / 60));
		}

		if (!text.IsEmpty())
			RZ_Log.Info("Reputation: time per cell (player-faction minutes): " + text);
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest gate within 25 m: a holding (its faction), a road post, a safehouse door (the Haven).
	protected bool GateNear(vector pos, out int f, out string place, out vector at)
	{
		f = -1;
		place = "";
		at = vector.Zero;
		float best = RADIUS;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				float d = RZ_Util.DistXZ(h.m_vPos, pos);
				if (d <= best && RZ_Factions.IsValid(h.m_eFaction))
				{
					best = d;
					f = h.m_eFaction;
					place = "h" + h.m_iId.ToString();
					at = h.m_vPos;
				}
			}
		}

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(RZ_EFaction.MILITARY, pos, RADIUS, near);
			posts.PostsNear(RZ_EFaction.BANDITS, pos, RADIUS, near);
			foreach (RZ_RoadPost post : near)
			{
				float pd = RZ_Util.DistXZ(post.m_vPos, pos);
				if (pd <= best)
				{
					best = pd;
					f = post.m_eFaction;
					place = "p" + post.m_sNear;
					at = post.m_vPos;
				}
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			int idx = houses.FindNearest(pos, best);
			if (idx >= 0)
			{
				f = RZ_EFaction.SURVIVORS;
				place = "s" + idx.ToString();
				at = houses.Get(idx).m_vPos;
			}
		}

		return f >= 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The bark, if one is due, with `colours` (a rival-colours remark, "" none) folded in. True if said.
	protected bool Bark(int pid, IEntity me, int f, string place, vector at, array<int> ids, array<IEntity> players, string colours)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(pid);
		if (!p || p.m_sIdentity.IsEmpty())
			return false;

		// Never while claiming in someone's kit, never to the Hostile manner
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility && hostility.GetDisguise().IsOn() && hostility.GetDisguise().InTheirKit(pid, f))
			return false;

		string manner = RZ_RepVoice.VoiceManner(m_GM, pid, f, null);
		if (manner == "hostile")
			return false;

		p.EnsureArrays();
		int cell = RZ_RepNames.GetRep(p.GetFame(f), p.GetInfamy(f));
		string deed = p.m_aBestDeed[f];
		int stamp = p.m_iLives * 100000 + cell * 1000 + Math.AbsInt(deed.Hash()) % 1000;
		string key = p.m_sIdentity + "|" + place;
		int last;
		if (m_mLast.Find(key, last) && last == stamp)
			return false;

		RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
		if (lanes && !lanes.TryGate(pid, "gate|" + place))
			return false;

		m_mLast.Set(key, stamp);
		string line = Line(pid, f, deed, last == 0);
		string said = line;
		if (!colours.IsEmpty() && (!lanes || lanes.TryGate(pid, "colours|" + place)))
			said = line + " " + colours;

		m_GM.Say(pid, Who(f, place), f, said);
		RZ_Log.Info(string.Format("Reputation: gate bark at %1 for %2", place, RZ_Util.PlayerName(pid)));

		// Others hear it at notoriety 3, in your own colours, once per 10 minutes per place
		if (!RZ_ReputationSettings.Get().m_bOthersHear || RZ_RepNames.Notoriety(p.GetFame(f), p.GetInfamy(f)) < 3)
			return true;

		if (hostility && hostility.GetDisguise().IsOn() && hostility.GetDisguise().Wearing(pid) >= 0 && !hostility.GetDisguise().IsOwnColours(pid, hostility.GetDisguise().Wearing(pid)))
			return true;

		float heard;
		if (m_mOthers.Find(place, heard) && m_fClock - heard < OTHERS_GAP)
			return true;

		m_mOthers.Set(place, m_fClock);
		string name = RZ_EpiNames.Now(pid);
		if (name.IsEmpty())
			name = "that one";

		string told = string.Format("Someone at the gate nods at %1. %2", name, line);
		foreach (int i, IEntity other : players)
		{
			if (ids[i] != pid && RZ_Util.DistXZ(other.GetOrigin(), at) <= HEAR_RADIUS)
				m_GM.Say(ids[i], Who(f, place), f, told);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Earned colours say whose you are (the amber ribbon, the red armband, armour bought at Trusted or
	//! Champion): a guard who sees a rival's on you makes one plain remark, once a visit, saying only
	//! what their hostility already does. The remark if one is due ("" none); the caller says it.
	protected string RivalColours(int pid, int f, string place)
	{
		RZ_Hostility hostility = m_GM.GetHostility();
		if (!hostility || !hostility.GetDisguise().IsOn())
			return "";

		RZ_Disguise d = hostility.GetDisguise();
		int rival = d.Wearing(pid);
		if (rival < 0 || rival == f || !d.IsOwnColours(pid, rival))
			return "";

		RZ_NoticeLanes lanes = RZ_NoticeLanes.Get();
		if (lanes && lanes.PlaceUsed(pid, "colours|" + place))
			return "";

		bool hostile = RZ_Reputation.IsHunted(m_GM.GetTier(pid, f));
		string theirs = RZ_Factions.GetNameInText(rival);
		string line;
		switch (f)
		{
			case RZ_EFaction.MILITARY:
			{
				line = string.Format("\"%1's colours at a Bastion gate. Noted, and logged.\"", RZ_Factions.GetName(rival));
				if (!hostile && RZ_Reputation.IsAtLeast(m_GM.GetTier(pid, f), RZ_ETier.KNOWN))
					line = string.Format("\"%1's colours at a Bastion gate. Bold. You're on the list. So are the colours.\"", RZ_Factions.GetName(rival));

				break;
			}
			case RZ_EFaction.BANDITS: line = string.Format("\"Wearing %1's colours on our road. Brave. Or stupid. We'll find out which.\"", theirs); break;
			case RZ_EFaction.INSTITUTE: line = string.Format("\"Another faction's insignia: %1. Recorded.\"", theirs); break;
			default: line = "\"We don't mind whose colours. Leave the rifle by the door.\""; break;
		}

		return line;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Who(int f, string place)
	{
		if (place.StartsWith("s"))
			return "At the door";

		if (place.StartsWith("p"))
			return "At the barrier";

		return RZ_Factions.GetName(f) + ", at the gate";
	}

	//------------------------------------------------------------------------------------------------
	//! The line: the best deed when there is one, else what they call you, in the faction's voice.
	protected string Line(int pid, int f, string deed, bool first)
	{
		if (!deed.IsEmpty())
		{
			switch (f)
			{
				case RZ_EFaction.SURVIVORS: return string.Format("\"Here's the one who %1. Mind the step.\"", deed);
				case RZ_EFaction.MILITARY: return string.Format("\"That's the one who %1. Let them by.\"", deed);
				case RZ_EFaction.BANDITS: return string.Format("\"Oi, it's the one who %1. Don't charge them twice.\"", deed);
			}

			return string.Format("\"The one who %1. Noted.\"", deed);
		}

		string word = RZ_RepVoice.Word(m_GM, pid, f);
		if (word.IsEmpty())
			word = RZ_RepNames.GetName(RZ_RepNames.GetRep(m_GM.GetProfile(pid).GetFame(f), m_GM.GetProfile(pid).GetInfamy(f)));

		if (first)
			return string.Format("\"%1. Haven't seen you this way before.\"", Cap(word));

		return string.Format("\"%1 again. Something's changed about you.\"", Cap(word));
	}

	//------------------------------------------------------------------------------------------------
	protected static string Cap(string s)
	{
		if (s.IsEmpty())
			return s;

		string first = s.Substring(0, 1);
		first.ToUpper();
		return first + s.Substring(1, s.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! The runtime test's walk through the cells (tools/runtime_test): every cell names itself and the
	//! picture reads, the credit runs out at Fame 75+, the budget stops forgiving at its limit, and the
	//! whole picture covers each kind. The player's Bastion standing is put back afterwards. One line.
	static string SelfTest(RZ_GameModeComponent gm, int playerId)
	{
		RZ_PlayerProfile p = gm.GetProfile(playerId);
		if (!p)
			return "Reputation test: no profile";

		p.EnsureArrays();
		int f = RZ_EFaction.MILITARY;
		int fame0 = p.m_aFame[f];
		int infamy0 = p.m_aInfamy[f];
		int forgiven0 = p.m_aForgiven[f];
		array<int> bands = {5, 25, 55, 85};
		int named = 0;
		for (int a = 0; a < 4; a++)
		{
			for (int b = 0; b < 4; b++)
			{
				p.m_aFame[f] = bands[a];
				p.m_aInfamy[f] = bands[b];
				if (!RZ_RepNames.GetName(RZ_RepNames.GetRep(bands[a], bands[b])).IsEmpty() && !RZ_RepVoice.Picture(gm, playerId).IsEmpty())
					named++;
			}
		}

		// The credit: at Fame 80, 3 Infamy also costs 3 Fame (when the rule is on)
		p.m_aFame[f] = 80;
		p.m_aInfamy[f] = 0;
		gm.AddInfamy(playerId, f, 3, "the runtime test");
		bool credit = !RZ_ReputationSettings.Get().m_bCreditRule || p.m_aFame[f] == 77;

		// The budget: asking for 100 forgives at most what's left of it
		p.m_aInfamy[f] = 100;
		p.m_aForgiven[f] = 0;
		int off = gm.Forgive(playerId, f, 100, "the runtime test");
		int budget = RZ_ReputationSettings.Get().m_iForgiveBudget;
		bool capped = budget <= 0 || off <= budget + budget / 5;

		p.m_aFame[f] = fame0;
		p.m_aInfamy[f] = infamy0;
		p.m_aForgiven[f] = forgiven0;
		string result = string.Format("Reputation test: %1 of 16 cells named, credit %2, budget %3 (forgave %4 of %5)", named, Ok(credit), Ok(capped), off, budget);
		RZ_Log.Info(result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Ok(bool ok)
	{
		if (ok)
			return "ok";

		return "FAILED";
	}

	//------------------------------------------------------------------------------------------------
	//! The bounty board's line about someone: their name, their reputation with the Red Hand, and a dry
	//! note when the book says the Red Hand owes them a hearing too.
	static string BountyWords(RZ_GameModeComponent gm, int targetId, string targetName)
	{
		if (targetId <= 0)
			return targetName;

		RZ_PlayerProfile p = gm.GetProfile(targetId);
		string rep = RZ_RepNames.GetName(RZ_RepNames.GetRep(p.GetFame(RZ_EFaction.BANDITS), p.GetInfamy(RZ_EFaction.BANDITS)));
		string text = string.Format("%1, %2", targetName, rep);
		if (RZ_RepVoice.IsOwed(gm, targetId, RZ_EFaction.BANDITS))
			text += ". The book says they're owed, too";

		return text;
	}
}
