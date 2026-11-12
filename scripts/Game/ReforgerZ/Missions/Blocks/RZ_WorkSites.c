//! Work sites (docs/DESIGN.md, "Mission building blocks", Work site): the one way a step that
//! says you DO something with your hands is done. A visible thing stands at the step's place (the
//! "before" look: a marked pit, a pile of planks, a crate), with one world prompt on it ("Dig the
//! latrine"). Hold the prompt (vanilla's hold-progress bar, RZ_WorkSiteAction) for the work's time,
//! 5-20 s, and the world shows the result: the before look goes and the "after" look stands there
//! (the latrine screen, the boarded door, the grave), for the step's life.
//!
//! Owners (errands, the openings, strangers, missions) open a site by key and poll IsDone(key) on
//! their own tick; they Close(key) when their step is over. Nothing here pays or decides anything.
//!
//! Server-checked: the server keeps who has been within reach of each spawned site, unhurt, and
//! since when (once a second). The hold's end (PerformAction on the server) counts only when that
//! player has been there for the work's time, less a second's slack; moving away or being hurt
//! starts the count again. The client's bar starts again too when you're hurt (RZ_WorkSiteAction).
//!
//! Props spawn only within the marks' spawn radius of a player and go past the despawn radius; a
//! site is data the rest of the time, never saved (an owner reopens it). With no site prefab
//! (Shared > Work sites > Site prefab) or Work sites off, Available() is false and owners keep their
//! old rule (stand there for the time).
//! Server only, except the enum and the settings.

//------------------------------------------------------------------------------------------------
//! What kind of work: picks the before and after looks (Shared > Work sites > Looks).
enum RZ_EWork
{
	NONE,
	LATRINE,	// dig a latrine: a marked pit, then the latrine screen
	GRAVE,		// dig a grave, raise a cairn: a marked plot, then a mound and a post
	BOARDS,		// board up a door or window: planks and nails, then the boarded door
	SANDBAGS,	// fill and stack sandbags: a pile of empty sacks, then a sandbag wall
	REPAIR,		// fit a part, fix a machine: a toolbox and parts, then packed away
	INSTALL,	// set something up (a sensor): parts, then the thing on its crate
	SEARCH,		// go through a heap, a shed, a wreck's box: a crate, then rummaged clutter
	TAKE,		// take things off a body, out of a kit bag: the effects, then gone
	FILL,		// fill cans or bottles at water or a pump: empty cans, then carried off
	PAINT,		// paint a number, a sign, a hand: a bare board and a tin, then the painted board
	WRITE,		// write or pin something up: a bare board, then a board with notices
	BURN,		// burn a pile: the pile, then ash
	CARE,		// dress a wound, see to someone lying down: the bed (it stays)
	BENCH,		// work at a table: cleaning, folding, labelling (the table stays)
	DIG,		// dig for something: a stake and a sack, then a heap of turned earth
	HANDS		// labour at something already there (push a car): no looks of its own
}

//------------------------------------------------------------------------------------------------
//! One kind's looks (Shared > Work sites > Looks).
[BaseContainerProps()]
class RZ_WorkLook
{
	[Attribute("0", UIWidgets.ComboBox, "The work these looks are for", "", ParamEnumArray.FromEnum(RZ_EWork))]
	RZ_EWork m_eKind;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Before: what stands there while the work is to do (a marked pit, planks, a crate). A composition of vanilla props, nothing with actions. Empty = only the prompt", "et")]
	ResourceName m_sBefore;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "After: what stands there once it's done (the latrine screen, the boarded door, the grave). Empty = nothing (the thing was taken or packed away)", "et")]
	ResourceName m_sAfter;
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Shared > Work sites.
[BaseContainerProps()]
class RZ_WorkSiteSettings
{
	[Attribute("1", desc: "Work sites: a step that says you do something with your hands (dig, build, board up, fix, search, fill, paint, burn, dress a wound) is a thing at its place with one hold prompt, and the world shows the result. Off, or no Site prefab: those steps are done by staying there for their time, as before (docs/DESIGN.md, Work site)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Site prefab: the invisible interaction box that carries the prompt (RZ_WorkSiteComponent, RZ_WorkSiteAction with a Duration, an RplComponent). Empty = work sites off", "et")]
	ResourceName m_sSitePrefab;

	[Attribute(desc: "The looks: before and after, one entry per kind of work (a kind with no entry has only the prompt)")]
	ref array<ref RZ_WorkLook> m_aLooks;

	[Attribute("3", UIWidgets.Slider, "Reach (m): you work within this of the site; further away, or hurt, and the count starts again", "1.5 6 0.5")]
	float m_fReach;

	[Attribute("1", UIWidgets.Slider, "Time scale: every hold takes its time x this (each step sets its own, 5-20 s)", "0.25 3 0.05")]
	float m_fTimeScale;

	[Attribute("40", UIWidgets.Slider, "Sites at once, server-wide. A new one past this drops the oldest whose step is over (its result still standing); one whose step is under way is never dropped", "8 200 1")]
	int m_iMaxSites;

	//------------------------------------------------------------------------------------------------
	RZ_WorkLook Look(RZ_EWork kind)
	{
		if (!m_aLooks)
			return null;

		foreach (RZ_WorkLook l : m_aLooks)
		{
			if (l && l.m_eKind == kind)
				return l;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
//! One site: data always, props only near players.
class RZ_WorkSite
{
	int m_iId;
	string m_sKey;				// the owner's key ("errand.12", "mission.4.point.1")
	RZ_EWork m_eKind;
	string m_sLabel;			// the prompt: "Dig the latrine"
	float m_fSeconds;			// how long the hold takes
	float m_fFastSeconds;		// for the players in m_aFast (a background's bonus), 0 = none
	ref array<int> m_aFast = {};
	vector m_vPos;
	float m_fYaw;
	int m_iOnly;				// only this player may do it (0: anyone)
	bool m_bUnseen;				// only while hidden (built stealth): seen, and the count starts again
	ResourceName m_sBefore;
	ResourceName m_sAfter;
	bool m_bDone;
	int m_iDoneBy;
	bool m_bClosed;				// the owner is done with it: dropped once nobody is near
	float m_fOpenedAt;

	// Props (null while nobody is near)
	IEntity m_Box;
	IEntity m_BeforeProp;
	IEntity m_AfterProp;

	// Who is working at it now: player, since when (the sites' clock), health then
	ref array<int> m_aNear = {};
	ref array<float> m_aSince = {};
	ref array<float> m_aHealth = {};

	//------------------------------------------------------------------------------------------------
	float SecondsFor(int playerId)
	{
		if (m_fFastSeconds > 0 && m_aFast.Contains(playerId))
			return m_fFastSeconds;

		return m_fSeconds;
	}

	//------------------------------------------------------------------------------------------------
	string FastText()
	{
		string text = "|";
		foreach (int id : m_aFast)
		{
			text = text + id.ToString() + "|";
		}

		return text;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_WorkSites
{
	protected static const float SLACK = 1.5;			// seconds the server forgives (its 1 s poll, the hold's network trip)
	protected static const float LIFT = 0.6;			// the prompt stands this high over the ground

	protected RZ_SharedSettings m_Shared;
	protected ref array<ref RZ_WorkSite> m_aSites = {};
	protected int m_iNextId = 1;
	protected float m_fClock;
	protected float m_fSpawnTimer;
	protected bool m_bWarnedComponent;
	protected bool m_bWarnedCap;

	//------------------------------------------------------------------------------------------------
	void RZ_WorkSites(RZ_SharedSettings shared)
	{
		m_Shared = shared;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_WorkSites Get()
	{
		RZ_Shared shared = RZ_Shared.Get();
		if (!shared)
			return null;

		return shared.GetWorkSites();
	}

	//------------------------------------------------------------------------------------------------
	RZ_WorkSiteSettings Settings()
	{
		return m_Shared.WorkSites();
	}

	//------------------------------------------------------------------------------------------------
	//! Work sites are on and have a prompt to carry: owners use them; otherwise their old rule.
	static bool Available()
	{
		RZ_WorkSites sites = Get();
		if (!sites)
			return false;

		RZ_WorkSiteSettings s = sites.Settings();
		return s && s.m_bEnabled && !s.m_sSitePrefab.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The hold for a step that used to take `stayed` seconds there: 5 s, plus up to 15 s by how long
	//! it was (2 minutes or more: 20 s), so a quicker way (a background's) stays quicker.
	static float HoldFor(float stayed)
	{
		return 5 + 15 * Math.Clamp(stayed / 120, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	//! A hold as it's used: kept to 3-30 s, x Time scale.
	static float Scaled(float hold)
	{
		float scale = 1;
		RZ_WorkSites sites = Get();
		if (sites && sites.Settings())
			scale = sites.Settings().m_fTimeScale;

		if (hold <= 0)
			hold = 10;

		return Math.Clamp(hold, 3, 30) * scale;
	}

	//------------------------------------------------------------------------------------------------
	//! The prompt when the owner gives none.
	static string DefaultLabel(RZ_EWork kind)
	{
		switch (kind)
		{
			case RZ_EWork.LATRINE: return "Dig the latrine";
			case RZ_EWork.GRAVE: return "Dig the grave";
			case RZ_EWork.BOARDS: return "Board it up";
			case RZ_EWork.SANDBAGS: return "Fill and stack the sandbags";
			case RZ_EWork.REPAIR: return "Fix it";
			case RZ_EWork.INSTALL: return "Set it up";
			case RZ_EWork.SEARCH: return "Search it";
			case RZ_EWork.TAKE: return "Take them";
			case RZ_EWork.FILL: return "Fill the cans";
			case RZ_EWork.PAINT: return "Paint it";
			case RZ_EWork.WRITE: return "Write it up";
			case RZ_EWork.BURN: return "Burn it";
			case RZ_EWork.CARE: return "Dress the wound";
			case RZ_EWork.BENCH: return "Do the work";
			case RZ_EWork.DIG: return "Dig";
			case RZ_EWork.HANDS: return "Lend a hand";
		}

		return "Work";
	}

	//------------------------------------------------------------------------------------------------
	//! A step's line without its time there, for a step held at a work site: "(30 s there)" goes,
	//! "(20 s there, nothing in your hands)" keeps "(nothing in your hands)". Only the first
	//! parenthesis whose first part is a time.
	static string WithoutTime(string text)
	{
		int open = text.IndexOf("(");
		while (open >= 0)
		{
			int close = text.IndexOfFrom(open, ")");
			if (close < 0)
				return text;

			string inner = text.Substring(open + 1, close - open - 1);
			string first = inner;
			string rest;
			int comma = inner.IndexOf(", ");
			if (comma >= 0)
			{
				first = inner.Substring(0, comma);
				rest = inner.Substring(comma + 2, inner.Length() - comma - 2);
			}

			if (IsTime(first))
			{
				string head = text.Substring(0, open);
				string tail = text.Substring(close + 1, text.Length() - close - 1);
				if (!rest.IsEmpty())
					return head + "(" + rest + ")" + tail;

				if (head.Length() > 0 && head.Substring(head.Length() - 1, 1) == " ")
					head = head.Substring(0, head.Length() - 1);

				return head + tail;
			}

			open = text.IndexOfFrom(close, "(");
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! "30 s there", "2 real minutes there", "a minute", "half a minute at the table".
	protected static bool IsTime(string part)
	{
		if (part.IndexOf("a minute") == 0 || part.IndexOf("half a minute") == 0)
			return true;

		if (part.IsEmpty() || !"0123456789".Contains(part.Substring(0, 1)))
			return false;

		return part.Contains(" s") || part.Contains("minute");
	}

	//------------------------------------------------------------------------------------------------
	//! A site's spot `minR`-`maxR` from `place` that meets the place need `work.site` (docs/PLACES.md:
	//! dry, not steep, not behind a lock); `place` itself when none does (it was already picked
	//! through its own need).
	static vector Spot(vector place, float minR, float maxR)
	{
		vector at = RZ_PlaceFind.Near(place, minR, maxR, RZ_PlaceNeeds.Get("work.site"));
		if (at == vector.Zero)
			return place;

		return at;
	}

	// ================================================================ Owners
	//------------------------------------------------------------------------------------------------
	//! Opens (or reopens, by key) a site at `pos`. hold: the hold in seconds (Scaled is applied);
	//! only: the one player who may do it (0 anyone); before/after: this site's own looks (empty:
	//! the kind's). Returns its id, -1 when work sites aren't available.
	int Open(string key, RZ_EWork kind, string label, float hold, vector pos, int only = 0, ResourceName before = "", ResourceName after = "", float yaw = -1)
	{
		if (!Available() || key.IsEmpty())
			return -1;

		RZ_WorkSite s = Find(key);
		if (s && !s.m_bClosed)
			return s.m_iId;

		if (s)
			Drop(s);

		MakeRoom();
		s = new RZ_WorkSite();
		s.m_iId = m_iNextId;
		m_iNextId++;
		s.m_sKey = key;
		s.m_eKind = kind;
		s.m_sLabel = label;
		if (label.IsEmpty())
			s.m_sLabel = DefaultLabel(kind);

		s.m_fSeconds = Scaled(hold);
		s.m_vPos = RZ_Util.GroundPos(pos);
		s.m_fYaw = yaw;
		if (yaw < 0)
			s.m_fYaw = Math.RandomFloat(0, 360);

		s.m_iOnly = only;
		RZ_WorkLook look = Settings().Look(kind);
		s.m_sBefore = before;
		s.m_sAfter = after;
		if (look && before.IsEmpty())
			s.m_sBefore = look.m_sBefore;

		if (look && after.IsEmpty())
			s.m_sAfter = look.m_sAfter;

		s.m_fOpenedAt = m_fClock;
		m_aSites.Insert(s);
		RZ_PlaceNeeds.Verify("work.site", s.m_vPos, s.m_sLabel);
		RZ_Log.Info(string.Format("Work sites: %1 \"%2\" (%3, %4 s) at %5", key, s.m_sLabel, typename.EnumToString(RZ_EWork, kind), s.m_fSeconds, RZ_SettlementScanner.GridReference(s.m_vPos)));
		return s.m_iId;
	}

	//------------------------------------------------------------------------------------------------
	//! A faster hold for this player (a background's bonus: seconds / speed). Owners call it when they
	//! learn who it is; the prompt picks it up on that player's machine.
	void SetFast(string key, int playerId, float speed)
	{
		RZ_WorkSite s = Find(key);
		if (!s || s.m_bDone || speed <= 1 || s.m_aFast.Contains(playerId))
			return;

		s.m_fFastSeconds = Math.Max(3, s.m_fSeconds / speed);
		s.m_aFast.Insert(playerId);
		Push(s);
	}

	//------------------------------------------------------------------------------------------------
	//! Done unseen (built stealth): the count runs only while the worker is hidden.
	void SetUnseen(string key)
	{
		RZ_WorkSite s = Find(key);
		if (s)
			s.m_bUnseen = true;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(string key)
	{
		RZ_WorkSite s = Find(key);
		return s && !s.m_bClosed;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDone(string key)
	{
		RZ_WorkSite s = Find(key);
		return s && s.m_bDone;
	}

	//------------------------------------------------------------------------------------------------
	//! Who finished it (0 nobody yet).
	int DoneBy(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return 0;

		return s.m_iDoneBy;
	}

	//------------------------------------------------------------------------------------------------
	//! The owner's step is over. An open site goes now; a done one stays (its after look) until
	//! nobody is near.
	void Close(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return;

		if (!s.m_bDone)
		{
			Drop(s);
			return;
		}

		s.m_bClosed = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds the server has counted `playerId` at it without a break (0 when they aren't there):
	//! for an owner that times something mid-way (a guard's second look).
	float WorkedFor(string key, int playerId)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return 0;

		int idx = s.m_aNear.Find(playerId);
		if (idx < 0)
			return 0;

		return m_fClock - s.m_aSince[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! The site's place (vector.Zero if none).
	vector PosOf(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return vector.Zero;

		return s.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks it done for `playerId` without the hold: the runtime test's fake player, which can't
	//! press anything. Not used by the mod itself.
	bool CompleteFor(string key, int playerId)
	{
		RZ_WorkSite s = Find(key);
		if (!s || s.m_bDone)
			return false;

		Finish(s, playerId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every open site whose key starts with `prefix`, done for `playerId` (the runtime test's fake
	//! player at a mission's sites). Returns how many.
	int CompleteAllFor(string prefix, int playerId)
	{
		int n = 0;
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_bDone || s.m_bClosed || s.m_sKey.IndexOf(prefix) != 0)
				continue;

			Finish(s, playerId);
			n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Sites open now (the runtime test's check).
	int Count()
	{
		return m_aSites.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! The prop up at the site now (its box while open, its after look once done), or null.
	IEntity PropOf(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return null;

		if (s.m_bDone)
			return s.m_AfterProp;

		return s.m_Box;
	}

	// ================================================================ The hold
	//------------------------------------------------------------------------------------------------
	//! The hold finished on `user`'s machine (RZ_WorkSiteAction.PerformAction, on the server).
	void TryComplete(int siteId, IEntity user)
	{
		RZ_WorkSite s = FindId(siteId);
		if (!s || s.m_bDone || !user)
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
		if (playerId <= 0)
			return;

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (s.m_iOnly > 0 && playerId != s.m_iOnly)
		{
			if (gm)
				gm.NotifyPlayer(playerId, s.m_sLabel, "This isn't your job. Whoever took it has to do it.");

			return;
		}

		if (s.m_bUnseen && !RZ_Stealth.IsHidden(playerId))
		{
			if (gm)
				gm.NotifyPlayer(playerId, s.m_sLabel, "They'd see you like that. Get low, keep out of the light, and start again.");

			return;
		}

		int idx = s.m_aNear.Find(playerId);
		float need = s.SecondsFor(playerId) - SLACK;
		if (idx < 0 || m_fClock - s.m_aSince[idx] < need)
		{
			if (gm)
				gm.NotifyPlayer(playerId, s.m_sLabel, string.Format("Not finished. It takes about %1 s at it without stopping, and a knock starts it again.", RZ_Util.RoundToInt(s.SecondsFor(playerId))));

			return;
		}

		Finish(s, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish(RZ_WorkSite s, int playerId)
	{
		s.m_bDone = true;
		s.m_iDoneBy = playerId;
		DeleteProps(s);
		if (HasPlayerNear(s.m_vPos, DespawnRadius()))
			SpawnAfter(s);

		RZ_Log.Info(string.Format("Work sites: %1 \"%2\" done by %3", s.m_sKey, s.m_sLabel, RZ_Util.PlayerName(playerId)));
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! Once a second (RZ_Shared.SecondTick): who is at which site, and props near players.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		if (m_aSites.IsEmpty())
			return;

		WatchWorkers(ids, players);
		m_fSpawnTimer += dt;
		if (m_fSpawnTimer < 2)
			return;

		m_fSpawnTimer = 0;
		float spawn = SpawnRadius();
		float despawn = DespawnRadius();
		for (int i = m_aSites.Count() - 1; i >= 0; i--)
		{
			RZ_WorkSite s = m_aSites[i];
			bool near = AnyNear(players, s.m_vPos, spawn);
			bool far = !AnyNear(players, s.m_vPos, despawn);
			if (s.m_bClosed && far)
			{
				Drop(s);
				continue;
			}

			if (far)
			{
				DeleteProps(s);
				continue;
			}

			if (!near)
				continue;

			if (s.m_bDone)
			{
				if (!s.m_AfterProp)
					SpawnAfter(s);

				continue;
			}

			if (!s.m_Box)
				SpawnBefore(s);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Each spawned open site: a player within reach is working there since they came; one who left
	//! or was hurt starts again.
	protected void WatchWorkers(array<int> ids, array<IEntity> players)
	{
		float reach = Settings().m_fReach;
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_bDone || !s.m_Box)
			{
				s.m_aNear.Clear();
				s.m_aSince.Clear();
				s.m_aHealth.Clear();
				continue;
			}

			for (int k = s.m_aNear.Count() - 1; k >= 0; k--)
			{
				int at = ids.Find(s.m_aNear[k]);
				if (at < 0 || RZ_Util.DistXZ(players[at].GetOrigin(), s.m_vPos) > reach)
				{
					s.m_aNear.Remove(k);
					s.m_aSince.Remove(k);
					s.m_aHealth.Remove(k);
				}
			}

			foreach (int i, int playerId : ids)
			{
				IEntity p = players[i];
				if (!p || RZ_Util.DistXZ(p.GetOrigin(), s.m_vPos) > reach)
					continue;

				float health = RZ_Util.GetCharacterHealth(p);
				int idx = s.m_aNear.Find(playerId);
				if (idx < 0)
				{
					s.m_aNear.Insert(playerId);
					s.m_aSince.Insert(m_fClock);
					s.m_aHealth.Insert(health);
					continue;
				}

				// Hurt, or seen at unseen work: the count starts again from now
				if (health >= 0 && s.m_aHealth[idx] >= 0 && health < s.m_aHealth[idx] - 0.01)
					s.m_aSince[idx] = m_fClock;

				if (s.m_bUnseen && !RZ_Stealth.IsHidden(playerId))
					s.m_aSince[idx] = m_fClock;

				s.m_aHealth[idx] = health;
			}
		}
	}

	// ================================================================ Props
	//------------------------------------------------------------------------------------------------
	protected void SpawnBefore(RZ_WorkSite s)
	{
		if (!s.m_sBefore.IsEmpty() && !s.m_BeforeProp)
			s.m_BeforeProp = RZ_Util.SpawnPrefab(s.m_sBefore, s.m_vPos, s.m_fYaw);

		vector at = s.m_vPos;
		at[1] = at[1] + LIFT;
		s.m_Box = RZ_Util.SpawnPrefab(Settings().m_sSitePrefab, at, s.m_fYaw);
		Push(s);
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnAfter(RZ_WorkSite s)
	{
		if (s.m_sAfter.IsEmpty() || s.m_AfterProp)
			return;

		s.m_AfterProp = RZ_Util.SpawnPrefab(s.m_sAfter, s.m_vPos, s.m_fYaw);
	}

	//------------------------------------------------------------------------------------------------
	//! The box's prompt: label, time, who.
	protected void Push(RZ_WorkSite s)
	{
		if (!s.m_Box)
			return;

		RZ_WorkSiteComponent comp = RZ_WorkSiteComponent.Cast(s.m_Box.FindComponent(RZ_WorkSiteComponent));
		if (!comp)
		{
			if (!m_bWarnedComponent)
			{
				m_bWarnedComponent = true;
				RZ_Log.Warn("Work sites: the Site prefab has no RZ_WorkSiteComponent, so it shows no prompt (docs/WORKBENCH_SETUP.md, Work sites).");
			}

			return;
		}

		comp.Set(s.m_iId, s.m_sLabel, s.m_fSeconds, s.m_fFastSeconds, s.FastText(), s.m_iOnly);
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteProps(RZ_WorkSite s)
	{
		if (s.m_Box)
			RZ_Util.DeleteUnit(s.m_Box);

		if (s.m_BeforeProp)
			RZ_Util.DeleteUnit(s.m_BeforeProp);

		if (s.m_AfterProp)
			RZ_Util.DeleteUnit(s.m_AfterProp);

		s.m_Box = null;
		s.m_BeforeProp = null;
		s.m_AfterProp = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void Drop(RZ_WorkSite s)
	{
		DeleteProps(s);
		m_aSites.RemoveItem(s);
	}

	//------------------------------------------------------------------------------------------------
	//! At the cap: the oldest closed site goes, else the oldest done one.
	protected void MakeRoom()
	{
		if (m_aSites.Count() < Settings().m_iMaxSites)
			return;

		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_bClosed)
			{
				Drop(s);
				return;
			}
		}

		// None closed: every one is a step still under way, so the cap gives (logged once)
		if (!m_bWarnedCap)
		{
			m_bWarnedCap = true;
			RZ_Log.Info(string.Format("Work sites: %1 open at once, past Sites at once; none dropped (each is a step under way).", m_aSites.Count()));
		}
	}

	// ================================================================ Helpers
	//------------------------------------------------------------------------------------------------
	protected RZ_WorkSite Find(string key)
	{
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_sKey == key)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_WorkSite FindId(int id)
	{
		foreach (RZ_WorkSite s : m_aSites)
		{
			if (s.m_iId == id)
				return s;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected float SpawnRadius()
	{
		return m_Shared.m_fMarkSpawnRadius;
	}

	//------------------------------------------------------------------------------------------------
	protected float DespawnRadius()
	{
		return Math.Max(m_Shared.m_fMarkDespawnRadius, m_Shared.m_fMarkSpawnRadius + 20);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool AnyNear(array<IEntity> players, vector pos, float radius)
	{
		foreach (IEntity p : players)
		{
			if (p && RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasPlayerNear(vector pos, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return AnyNear(players, pos, radius);
	}
}
