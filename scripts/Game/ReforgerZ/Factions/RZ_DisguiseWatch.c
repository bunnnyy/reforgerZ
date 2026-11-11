//! Borrowed kit, disguise phase 2 (docs/REPUTATION.md, "Borrowed kit"): what happens while you pass,
//! and after a catch.
//! - Again looks: while you pass, every few minutes (Hostility > Again look minutes) that one of
//!   theirs within 15 m can see you (a sight trace), they look again, one step harder each time; a
//!   notice 10 s ahead. Walk off and the look is owed for 30 minutes: claim again and it comes 10 s
//!   after the claim.
//! - Roll call: at the shift change (05:30, 17:30; the Red Hand's dusk meal at 18:30) everyone
//!   passing at one of their holdings is looked at.
//! - Word gets round: a catch, or a kill of one of theirs that retaliation counts, makes every look
//!   at that faction's nearest holding (within 1.5 km) and its road posts one step harder for 30
//!   minutes. One per holding; another trigger restarts it, never queues.
//! - Vouching: Known with Bastion or Halcyon (Trusted with the Red Hand), in your own colours, you
//!   sign one friend through a barrier or Halcyon's gate (dialogue's "Vouch" at the post's speaker or
//!   the gate). For 30 minutes and 1 km you answer for them: caught in that faction's kit, or a hit
//!   or a kill on its people, costs you +5 Infamy once, and 2 hours without vouching.
//! - The Rogue's price: a Rogue leader within 15 m sees through you and asks 20 of his faction's
//!   Favor. Stay and you pay (10 minutes at his seat); walk off before he asks twice and nothing
//!   happens; can't pay and you're caught. If he's found out, everyone who paid him takes +5 Infamy.
//! - Held: caught at a Bastion barrier, walking off while they deal with you is running it.
//! - Seen putting it on: one of theirs within 30 m who can see you when the kit goes on: -1 at
//!   every look within 150 m of there for an hour.
//! - Colours in a feud: in Bastion colours within 150 m of a Red Hand crew that can see you, while
//!   the two are feuding, the crew opens up (the timed grudge).
//! - The nightly call (20:30) reads the names that went on the nail; Halcyon reads the coat count at
//!   the change of shift. Server only.

//------------------------------------------------------------------------------------------------
class RZ_DisguiseVouch
{
	string m_sVoucher;
	string m_sVouched;
	int m_iFaction;
	vector m_vPos;
	float m_fAt;
	bool m_bPaid;
}

//------------------------------------------------------------------------------------------------
class RZ_DisguiseWatch
{
	static const float AGAIN_RADIUS = 15;
	static const float AGAIN_NOTICE = 10;
	static const float OWED_SECONDS = 1800;
	static const float SEEN_RADIUS = 30;
	static const float SEEN_LOOK_RADIUS = 150;
	static const float SEEN_SECONDS = 3600;
	static const float VOUCH_RADIUS = 10;
	static const float VOUCH_SECONDS = 1800;
	static const float VOUCH_RANGE = 1000;
	static const float VOUCH_BAN = 7200;
	static const float ROGUE_PASS = 600;
	static const float ROGUE_ASK = 60;
	static const int ROGUE_INFAMY = 5;
	static const float HELD_LEAVE = 30;
	static const float FEUD_RADIUS = 150;
	static const float STRICT_RADIUS = 35;
	static const float EYE = 1.6;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Disguise m_Disguise;
	protected RZ_HostilitySettings m_Settings;
	protected float m_fClock;
	protected int m_iPhase = -2;

	protected ref map<string, float> m_mAgainAt = new map<string, float>();		// "ident|f" -> clock the next again look is due
	protected ref map<string, int> m_mAgainWarned = new map<string, int>();
	protected ref map<string, float> m_mOwed = new map<string, float>();		// "ident|f" -> clock an owed look lapses
	protected ref map<string, string> m_mPlace = new map<string, string>();	// "ident|f" -> the place of the passed look
	protected ref map<string, float> m_mWord = new map<string, float>();		// "f|holding" -> clock word stops going round
	protected ref array<ref RZ_DisguiseVouch> m_aVouches = {};
	protected ref map<string, float> m_mVouchBan = new map<string, float>();	// ident -> clock
	protected ref map<string, float> m_mRogueAsked = new map<string, float>();	// "ident|leader" -> clock
	protected ref map<string, float> m_mRoguePaid = new map<string, float>();	// "ident|leader" -> clock the 10 minutes end
	protected ref map<int, ref array<string>> m_mRoguePayers = new map<int, ref array<string>>();
	protected ref map<string, int> m_mOwedInfamy = new map<string, int>();		// "ident|f" -> Infamy owed by someone offline
	protected ref map<string, vector> m_mHeldAt = new map<string, vector>();	// ident -> where they're held
	protected ref map<string, float> m_mHeldUntil = new map<string, float>();
	protected ref map<string, int> m_mHeldFaction = new map<string, int>();
	protected ref map<string, vector> m_mSeenAt = new map<string, vector>();	// "ident|f" -> where they were seen putting it on
	protected ref map<string, float> m_mSeenUntil = new map<string, float>();
	protected ref map<string, float> m_mTold = new map<string, float>();
	protected ref map<string, string> m_mHeardName = new map<string, string>();	// ident -> a dead man's name heard on the call

	//------------------------------------------------------------------------------------------------
	void RZ_DisguiseWatch(RZ_GameModeComponent gm, RZ_Disguise disguise, RZ_HostilitySettings settings)
	{
		m_GM = gm;
		m_Disguise = disguise;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	protected string Ident(int playerId)
	{
		return m_GM.GetProfile(playerId).m_sIdentity;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Key(string ident, int f)
	{
		return ident + "|" + f.ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected bool Tell(string key, float gap)
	{
		float last;
		if (m_mTold.Find(key, last) && m_fClock - last < gap)
			return false;

		m_mTold.Set(key, m_fClock);
		return true;
	}

	// ================================================================ Their people
	//------------------------------------------------------------------------------------------------
	//! `f`'s people within `radius` of pos: holding guards and named people, road post crews, a
	//! mission's watchers. Characters, alive.
	void NearestPeople(int f, vector pos, float radius, notnull array<IEntity> result)
	{
		array<IEntity> units = {};
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				if (h.m_eFaction == f && h.m_bSpawned && RZ_Util.DistXZ(h.m_vPos, pos) <= radius + 300)
					units.InsertAll(h.m_aBodies);
			}
		}

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(f, pos, radius + 100, near);
			foreach (RZ_RoadPost post : near)
			{
				units.InsertAll(post.m_aBodies);
			}
		}

		m_Disguise.WatchersOf(f, units);
		foreach (IEntity u : units)
		{
			array<IEntity> members = {};
			RZ_Util.GetUnitMembers(u, members);
			foreach (IEntity m : members)
			{
				if (m && !result.Contains(m) && RZ_Util.IsCharacterAlive(m) && RZ_Util.DistXZ(m.GetOrigin(), pos) <= radius)
					result.Insert(m);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest of `f`'s people within `radius` who can see this character (one sight trace,
	//! from their eyes to your chest), null if none.
	IEntity SeenBy(int f, IEntity me, float radius)
	{
		if (!me)
			return null;

		array<IEntity> people = {};
		NearestPeople(f, me.GetOrigin(), radius, people);
		IEntity best;
		float bestDist = radius + 1;
		foreach (IEntity p : people)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), me.GetOrigin());
			if (d < bestDist)
			{
				best = p;
				bestDist = d;
			}
		}

		if (!best || !CanSee(best, me))
			return null;

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A sight trace from a person's eyes to a character's chest. VERIFY: TraceMove with ENTS | WORLD
	//! from one character's head stops on walls and vehicles between them, and reports the target
	//! when it reaches them.
	static bool CanSee(IEntity viewer, IEntity target)
	{
		if (!viewer || !target)
			return false;

		TraceParam param = new TraceParam();
		param.Start = viewer.GetOrigin() + Vector(0, EYE, 0);
		param.End = target.GetOrigin() + Vector(0, 1.2, 0);
		param.Exclude = viewer;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		float hit = GetGame().GetWorld().TraceMove(param, null);
		if (hit >= 0.97)
			return true;

		return param.TraceEnt == target;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players, bool kitRead)
	{
		m_fClock += dt;
		foreach (int i, IEntity p : players)
		{
			int playerId = ids[i];
			Held(playerId, p);
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (m_Disguise.IsPassing(playerId, f))
					Again(playerId, f, p);
			}

			if (kitRead)
				Feud(playerId, p);
		}

		if (kitRead)
		{
			RollCall(ids, players);
			PayOwed();
		}
	}

	// ================================================================ Again looks
	//------------------------------------------------------------------------------------------------
	//! A passed look: the again looks start (10 s after it, if one was owed here).
	void OnPassed(int playerId, int f, string place)
	{
		string key = Key(Ident(playerId), f);
		m_mPlace.Set(key, place);
		m_mAgainWarned.Remove(key);
		float owed;
		if (m_mOwed.Find(key, owed) && m_fClock < owed)
		{
			m_mOwed.Remove(key);
			m_mAgainAt.Set(key, m_fClock + AGAIN_NOTICE);
			RZ_Log.Info(string.Format("Disguise: %1 owed a look by %2: it comes in 10 s", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f)));
			return;
		}

		m_mAgainAt.Set(key, m_fClock + AgainSeconds());
	}

	//------------------------------------------------------------------------------------------------
	protected float AgainSeconds()
	{
		return Math.Max(1, m_Settings.m_fAgainLookMinutes) * 60;
	}

	//------------------------------------------------------------------------------------------------
	protected void Again(int playerId, int f, IEntity me)
	{
		string key = Key(Ident(playerId), f);
		float due;
		if (!m_mAgainAt.Find(key, due))
		{
			m_mAgainAt.Set(key, m_fClock + AgainSeconds());
			return;
		}

		if (m_fClock < due - AGAIN_NOTICE)
			return;

		string place;
		m_mPlace.Find(key, place);
		IEntity watcher = SeenBy(f, me, AGAIN_RADIUS);
		if (!watcher)
		{
			// Nobody of theirs sees you when it's due: the look is owed for 30 minutes
			if (m_fClock >= due)
			{
				float owedUntil;
				if (!m_mOwed.Find(key, owedUntil) || owedUntil < m_fClock)
					m_mOwed.Set(key, m_fClock + OWED_SECONDS);
			}

			return;
		}

		int warned;
		m_mAgainWarned.Find(key, warned);
		if (warned == 0)
		{
			m_mAgainWarned.Set(key, 1);
			m_Disguise.ShowComingNow(playerId, f, 2, place, "", RZ_Factions.GetName(f), "Someone's looking at you again.");
			if (m_fClock < due)
				return;
		}

		if (m_fClock < due)
			return;

		m_mAgainWarned.Remove(key);
		m_mOwed.Remove(key);
		m_mAgainAt.Set(key, m_fClock + AgainSeconds());
		RZ_Log.Info(string.Format("Disguise: again look at %1 by %2 at %3", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), place));
		m_Disguise.Look(playerId, f, 2, place, "", -1);
	}

	//------------------------------------------------------------------------------------------------
	//! The pass ended (time, a catch): the again looks stop. An owed look stays.
	void OnPassEnded(int playerId, int f)
	{
		string key = Key(Ident(playerId), f);
		m_mAgainAt.Remove(key);
		m_mAgainWarned.Remove(key);
	}

	// ================================================================ Roll call
	//------------------------------------------------------------------------------------------------
	//! At the shift change (the Red Hand: the dusk meal), a look at everyone passing at a holding.
	//! Also the nightly call (20:30) and Halcyon's coat count at the shift change.
	protected void RollCall(array<int> ids, array<IEntity> players)
	{
		int phase = RZ_Territory.RoutinePhase(RZ_GameTime.GetHour());
		if (phase == m_iPhase)
			return;

		bool first = m_iPhase == -2;
		m_iPhase = phase;
		if (first || phase == RZ_Territory.PHASE_NONE)
			return;

		if (phase == RZ_Territory.PHASE_NIGHT)
			NightlyCall(ids, players);

		if (phase == RZ_Territory.PHASE_MORNING || phase == RZ_Territory.PHASE_EVENING)
			CoatCount();

		if (!m_Settings.m_bRollCall)
			return;

		RZ_Territory territory = m_GM.GetTerritory();
		foreach (int i, IEntity p : players)
		{
			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				if (!m_Disguise.IsPassing(ids[i], f))
					continue;

				bool callNow = phase == RZ_Territory.PHASE_MORNING || phase == RZ_Territory.PHASE_EVENING;
				if (f == RZ_EFaction.BANDITS)
					callNow = phase == RZ_Territory.PHASE_MEAL;

				if (!callNow)
					continue;

				string place = "";
				if (territory && territory.IsActive())
				{
					RZ_Holding h = territory.FindHolding(f, p.GetOrigin(), RZ_Disguise.KNOW_RADIUS);
					if (h)
						place = "h" + h.m_iId.ToString();
				}

				if (place.IsEmpty() && f == RZ_EFaction.INSTITUTE && m_GM.HasLab() && RZ_Util.DistXZ(m_GM.GetLabPosition(), p.GetOrigin()) <= RZ_Disguise.KNOW_RADIUS)
					place = "lab";

				if (place.IsEmpty())
					continue;

				string line = "Roll call. Names, in a line.";
				if (f == RZ_EFaction.BANDITS)
					line = "The dusk meal. Everyone sits where they always sit.";

				m_GM.Say(ids[i], RZ_Factions.GetName(f), f, line);
				RZ_Log.Info(string.Format("Disguise: roll call for %1 by %2 at %3", RZ_Util.PlayerName(ids[i]), RZ_Factions.GetName(f), place));
				m_Disguise.Look(ids[i], f, 2, place, "", -1);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! 20:30: Bastion reads the names that went on the nail since the last call. Whoever hears it has a
	//! dead man's name to give (a cover detail, never at his own post).
	protected void NightlyCall(array<int> ids, array<IEntity> players)
	{
		array<string> names = {};
		m_Disguise.GetMarks().TakeUnread(RZ_EFaction.MILITARY, names);
		if (names.IsEmpty())
			return;

		string list = "";
		foreach (int i, string n : names)
		{
			if (i >= 4)
				break;

			if (!list.IsEmpty())
				list += "; ";

			list += n;
		}

		string line = string.Format("\"The nightly numbers. And the names: %1. On the nail. Bastion out.\"", list);
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.OpenChannelLine(RZ_EFaction.MILITARY, line);

		string heard = m_Disguise.GetMarks().LatestName(RZ_EFaction.MILITARY);
		foreach (int j, IEntity p : players)
		{
			if (!heard.IsEmpty() && HearsChannel(ids[j], p, RZ_EFaction.MILITARY))
				m_mHeardName.Set(Ident(ids[j]), heard);
		}

		RZ_Log.Info(string.Format("Disguise: the nightly call read %1 names", names.Count()));
	}

	//------------------------------------------------------------------------------------------------
	//! Whoever hears `f`'s open channel (as RZ_FactionLife's chatter): a radio, and Known with them or
	//! a Radio Operator.
	protected bool HearsChannel(int playerId, IEntity p, int f)
	{
		if (!RZ_Radio.HasRadio(m_GM, playerId, p))
			return false;

		return RZ_Radio.IsRadioOperator(m_GM, playerId) || RZ_Reputation.IsAtLeast(m_GM.GetTier(playerId, f), RZ_ETier.KNOWN);
	}

	//------------------------------------------------------------------------------------------------
	//! A dead man's name this player heard on the nightly call, "" none.
	string HeardName(int playerId)
	{
		string name;
		m_mHeardName.Find(Ident(playerId), name);
		return name;
	}

	//------------------------------------------------------------------------------------------------
	protected void CoatCount()
	{
		array<string> names = {};
		m_Disguise.GetMarks().TakeUnread(RZ_EFaction.INSTITUTE, names);
		string count = m_Disguise.GetMarks().CoatCount();
		if (count.IsEmpty() && names.IsEmpty())
			return;

		string line = count;
		if (!names.IsEmpty())
			line = string.Format("%1 \"Added to the authors: %2.\"", count, names[0]).Trim();

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life)
			life.OpenChannelLine(RZ_EFaction.INSTITUTE, line);
	}

	// ================================================================ Word gets round
	//------------------------------------------------------------------------------------------------
	//! A catch (kill false) or a counted kill of one of theirs at pos: word gets round at `f`'s
	//! nearest holding within the radius, and its road posts, for 30 minutes. Restarted, never queued;
	//! the line only when it wasn't already going round. True if a holding was in reach.
	bool WordGetsRound(int f, vector pos, bool kill)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive() || !RZ_Factions.IsValid(f) || f == RZ_EFaction.SURVIVORS)
			return false;

		RZ_Holding h = territory.FindHolding(f, pos, Math.Max(100, m_Settings.m_fWordRoundRadius));
		if (!h)
			return false;

		string key = f.ToString() + "|" + h.m_iId.ToString();
		float until;
		bool going = m_mWord.Find(key, until) && m_fClock < until;
		m_mWord.Set(key, m_fClock + Math.Max(1, m_Settings.m_fWordRoundMinutes) * 60);
		RZ_Log.Info(string.Format("Disguise: word gets round at %1 (%2, %3)", h.GetTitle(), WhatWord(kill), RestartWord(going)));
		if (going)
			return true;

		string line = WordLine(f, h.m_sName, kill);
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (life && !line.IsEmpty())
			life.OpenChannelLine(f, line);

		// Leader news and wanderers' road news carry it for an hour
		string rumour = string.Format("Heard someone walked into %1 in borrowed kit. They walked out without it.", h.m_sName);
		if (kill)
			rumour = string.Format("Heard %1 lost one of theirs near %2. They're checking faces.", RZ_Factions.GetNameInText(f), h.m_sName);

		RZ_WorldFacts.Get().AddRumour(rumour, 1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static string WhatWord(bool kill)
	{
		if (kill)
			return "a kill";

		return "a catch";
	}

	//------------------------------------------------------------------------------------------------
	protected static string RestartWord(bool going)
	{
		if (going)
			return "restarted";

		return "new";
	}

	//------------------------------------------------------------------------------------------------
	protected static string WordLine(int f, string place, bool kill)
	{
		if (f == RZ_EFaction.MILITARY)
		{
			if (kill)
				return string.Format("\"All stations: %1 has a man down. Faces checked at the wire. Bastion out.\"", place);

			return string.Format("\"All stations: civilian in our kit at %1. Check faces. Bastion out.\"", place);
		}

		if (f == RZ_EFaction.BANDITS)
		{
			if (kill)
				return string.Format("\"Somebody dropped one of ours by %1. Nobody walks through on a smile till dusk.\"", place);

			return "\"Someone's wearing hands we didn't paint. Ask names at the toll.\"";
		}

		if (kill)
			return "\"A member of staff was killed near the station. Please stay inside the wire.\"";

		return "\"Security notice: a coat was presented at the station by someone not in the record. Please check numbers.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! Word is going round at a holding of `f` within its radius of pos: -1 at every look there.
	bool WordRound(int f, vector pos)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return false;

		float radius = Math.Max(100, m_Settings.m_fWordRoundRadius);
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction != f || RZ_Util.DistXZ(h.m_vPos, pos) > radius)
				continue;

			float until;
			if (m_mWord.Find(f.ToString() + "|" + h.m_iId.ToString(), until) && m_fClock < until)
				return true;
		}

		return false;
	}

	// ================================================================ Seen putting it on
	//------------------------------------------------------------------------------------------------
	//! The kit went on: one of theirs within 30 m who can see you remembers it for an hour.
	void OnPutOn(int playerId, int f, IEntity me)
	{
		if (!SeenBy(f, me, SEEN_RADIUS))
			return;

		string key = Key(Ident(playerId), f);
		m_mSeenAt.Set(key, me.GetOrigin());
		m_mSeenUntil.Set(key, m_fClock + SEEN_SECONDS);
		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), "One of theirs watched you put it on. Around here, that face won't pass so easily.");
		RZ_Log.Info(string.Format("Disguise: %1 seen putting on %2's kit", RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f)));
	}

	//------------------------------------------------------------------------------------------------
	bool SeenPuttingOn(int playerId, int f, vector pos)
	{
		string key = Key(Ident(playerId), f);
		vector at;
		float until;
		return m_mSeenAt.Find(key, at) && m_mSeenUntil.Find(key, until) && m_fClock < until && RZ_Util.DistXZ(at, pos) <= SEEN_LOOK_RADIUS;
	}

	// ================================================================ Held
	//------------------------------------------------------------------------------------------------
	//! Caught: held where you stand while they deal with you; whoever vouched for you answers; word
	//! gets round. True if a holding was in reach (its word line, or it was already going round).
	bool OnCaught(int playerId, int f, IEntity me)
	{
		OnPassEnded(playerId, f);
		if (!me)
			return false;

		string ident = Ident(playerId);
		m_mHeldAt.Set(ident, me.GetOrigin());
		m_mHeldUntil.Set(ident, m_fClock + RZ_Disguise.HELD_SECONDS);
		m_mHeldFaction.Set(ident, f);
		OnVouchedTrouble(playerId, f, me.GetOrigin(), "caught in their kit");
		return WordGetsRound(f, me.GetOrigin(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! Walking off while held at a Bastion barrier is running it: the run's Infamy and the timed grudge.
	protected void Held(int playerId, IEntity me)
	{
		string ident = Ident(playerId);
		float until;
		if (!m_mHeldUntil.Find(ident, until))
			return;

		vector at;
		m_mHeldAt.Find(ident, at);
		int f;
		m_mHeldFaction.Find(ident, f);
		if (m_fClock >= until)
		{
			m_mHeldUntil.Remove(ident);
			return;
		}

		if (RZ_Util.DistXZ(me.GetOrigin(), at) <= HELD_LEAVE)
			return;

		m_mHeldUntil.Remove(ident);
		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (f != RZ_EFaction.MILITARY || !posts)
			return;

		array<RZ_RoadPost> near = {};
		posts.PostsNear(f, at, 60, near);
		if (near.IsEmpty())
			return;

		int infamy = posts.RunInfamy();
		m_GM.AddInfamy(playerId, f, infamy, "walked off while held at a checkpoint", RZ_RepFlag.ACT);
		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.AddGrudge(playerId, f, at);

		m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("\"Stop. STOP.\" You walked off while they were dealing with you. That's running the checkpoint. +%1 Bastion Infamy.", infamy));
		RZ_Log.Info(string.Format("Disguise: %1 walked off while held: ran the checkpoint", RZ_Util.PlayerName(playerId)));
	}

	// ================================================================ Vouching
	//------------------------------------------------------------------------------------------------
	//! Banned from vouching (2 hours after a vouch went wrong).
	bool VouchBanned(int playerId)
	{
		float until;
		return m_mVouchBan.Find(Ident(playerId), until) && m_fClock < until;
	}

	//------------------------------------------------------------------------------------------------
	//! The one vouch's liability, paid elsewhere (a lie found out): 2 hours without vouching.
	void BanVoucher(string ident)
	{
		m_mVouchBan.Set(ident, m_fClock + VOUCH_BAN);
	}

	//------------------------------------------------------------------------------------------------
	//! The road post or Halcyon gate a speaker stands at (`post` set, or `gate` the place key); false
	//! if neither.
	protected bool BarrierOf(int f, IEntity speaker, out RZ_RoadPost post, out string gate)
	{
		post = null;
		gate = "";
		if (!speaker)
			return false;

		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (posts)
		{
			array<RZ_RoadPost> near = {};
			posts.PostsNear(f, speaker.GetOrigin(), 40, near);
			if (!near.IsEmpty())
			{
				post = near[0];
				return true;
			}
		}

		if (f != RZ_EFaction.INSTITUTE)
			return false;

		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding h = territory.FindHolding(f, speaker.GetOrigin(), 80);
			if (h)
			{
				gate = "h" + h.m_iId.ToString();
				return true;
			}
		}

		if (m_GM.HasLab() && RZ_Util.DistXZ(m_GM.GetLabPosition(), speaker.GetOrigin()) <= 80)
		{
			gate = "lab";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Can `me` sign `other` through here: vouching on, not banned, Known with Bastion or Halcyon
	//! (Trusted with the Red Hand), in your own colours (your face and their colours), at a road post's
	//! barrier or Halcyon's gate, with them within 10 m and not Hunted.
	bool CanSignIn(int me, int other, int f, IEntity speaker)
	{
		if (!m_Disguise.IsOn() || !m_Settings.m_bVouching || f == RZ_EFaction.SURVIVORS || !RZ_Factions.IsValid(f) || VouchBanned(me))
			return false;

		RZ_ETier need = RZ_ETier.KNOWN;
		if (f == RZ_EFaction.BANDITS)
			need = RZ_ETier.TRUSTED;

		if (!RZ_Reputation.IsAtLeast(m_GM.GetTier(me, f), need) || m_GM.GetTier(me, f) == RZ_ETier.WILDCARD)
			return false;

		if (m_Disguise.KitLevel(me, f) <= 0 || !m_Disguise.IsOwnColours(me, f) || RZ_Reputation.IsHunted(m_GM.GetTier(other, f)))
			return false;

		IEntity a = GetGame().GetPlayerManager().GetPlayerControlledEntity(me);
		IEntity b = GetGame().GetPlayerManager().GetPlayerControlledEntity(other);
		if (!a || !b || RZ_Util.DistXZ(a.GetOrigin(), b.GetOrigin()) > VOUCH_RADIUS)
			return false;

		RZ_RoadPost post;
		string gate;
		return BarrierOf(f, speaker, post, gate);
	}

	//------------------------------------------------------------------------------------------------
	//! Signs `other` through (Bastion: waved through unsearched; the Red Hand: the Known toll instead
	//! of a turn-round; Halcyon: a Watched visitor in for 10 minutes). `me` answers for them. The line
	//! said, "" if it didn't apply.
	string SignIn(int me, int other, int f, IEntity speaker)
	{
		if (!CanSignIn(me, other, f, speaker))
			return "";

		RZ_RoadPost post;
		string gate;
		BarrierOf(f, speaker, post, gate);
		RZ_Checkpoints posts = m_GM.GetCheckpoints();
		if (!posts)
			return "";

		if (post)
			posts.SignInAtPost(post, other);
		else
			posts.SignInAtGate(other, gate, Math.Max(1, m_Settings.m_fGatePassMinutes) * 60);

		RZ_DisguiseVouch v = new RZ_DisguiseVouch();
		v.m_sVoucher = Ident(me);
		v.m_sVouched = Ident(other);
		v.m_iFaction = f;
		v.m_vPos = speaker.GetOrigin();
		v.m_fAt = m_fClock;
		m_aVouches.Insert(v);
		while (m_aVouches.Count() > 64)
		{
			m_aVouches.RemoveOrdered(0);
		}

		RZ_Log.Info(string.Format("Disguise: %1 signed %2 through for %3", RZ_Util.PlayerName(me), RZ_Util.PlayerName(other), RZ_Factions.GetName(f)));
		m_GM.NotifyPlayer(other, RZ_Factions.GetName(f), string.Format("%1 signs you in. No look, no search. They answer for you for half an hour.", RZ_Util.PlayerName(me)));
		if (f == RZ_EFaction.MILITARY)
			return "\"Sign here.\" The sergeant reads the name twice, then waves your friend through.";

		if (f == RZ_EFaction.BANDITS)
			return "\"Yours, is it? Then it's the friends' rate.\"";

		return "\"Visitor. Ten minutes, and they don't touch anything.\"";
	}

	//------------------------------------------------------------------------------------------------
	//! The one they vouched for got into trouble with `f` near the vouch: the voucher answers, once.
	void OnVouchedTrouble(int playerId, int f, vector pos, string what)
	{
		string ident = Ident(playerId);
		foreach (RZ_DisguiseVouch v : m_aVouches)
		{
			if (v.m_bPaid || v.m_sVouched != ident || v.m_iFaction != f || m_fClock - v.m_fAt > VOUCH_SECONDS || RZ_Util.DistXZ(v.m_vPos, pos) > VOUCH_RANGE)
				continue;

			v.m_bPaid = true;
			BanVoucher(v.m_sVoucher);
			int amount = Math.Max(0, m_Settings.m_iVouchInfamy);
			int voucher = m_GM.FindPlayerByIdentity(v.m_sVoucher);
			if (voucher > 0)
			{
				m_GM.AddInfamy(voucher, f, amount, "vouched for someone who " + what, RZ_RepFlag.SHARE);
				m_GM.NotifyPlayer(voucher, RZ_Factions.GetName(f), string.Format("The one you signed in was %1. You answer for it: +%2 Infamy with %3, and nobody takes your word at a barrier for 2 hours.", what, amount, RZ_Factions.GetNameInText(f)));
			}
			else
			{
				OweInfamy(v.m_sVoucher, f, amount);
			}

			RZ_Log.Info(string.Format("Disguise: voucher %1 answers for %2 with %3 (%4)", v.m_sVoucher, RZ_Util.PlayerName(playerId), RZ_Factions.GetName(f), what));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Infamy for someone offline, paid when they're next on.
	protected void OweInfamy(string ident, int f, int amount)
	{
		string key = Key(ident, f);
		int had;
		m_mOwedInfamy.Find(key, had);
		m_mOwedInfamy.Set(key, had + amount);
	}

	//------------------------------------------------------------------------------------------------
	protected void PayOwed()
	{
		if (m_mOwedInfamy.IsEmpty())
			return;

		array<string> paid = {};
		foreach (string key, int amount : m_mOwedInfamy)
		{
			int bar = key.LastIndexOf("|");
			if (bar <= 0)
			{
				paid.Insert(key);
				continue;
			}

			string ident = key.Substring(0, bar);
			int f = key.Substring(bar + 1, key.Length() - bar - 1).ToInt();
			int playerId = m_GM.FindPlayerByIdentity(ident);
			if (playerId <= 0)
				continue;

			m_GM.AddInfamy(playerId, f, amount, "named in the records", RZ_RepFlag.SHARE);
			m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(f), string.Format("While you were away your name came up in %1's records. +%2 Infamy.", RZ_Factions.GetNameInText(f), amount));
			paid.Insert(key);
		}

		foreach (string done : paid)
		{
			m_mOwedInfamy.Remove(done);
		}
	}

	// ================================================================ The Rogue's price
	//------------------------------------------------------------------------------------------------
	//! A Rogue leader of `f` within 15 m sees through this player. True: he plays along (asked, or
	//! paid); false: he calls the guard (caught). The first time he asks; still there at the next
	//! check, you pay.
	bool RoguePrice(int playerId, int f, RZ_Leader l)
	{
		int price = m_Settings.m_iRoguePrice;
		if (!l || price <= 0 || l.m_eLoyalty != RZ_ELoyalty.ROGUE || l.IsStronghold())
			return false;

		string key = Ident(playerId) + "|" + l.m_iId.ToString();
		float paidUntil;
		if (m_mRoguePaid.Find(key, paidUntil) && m_fClock < paidUntil)
			return true;

		float asked;
		if (!m_mRogueAsked.Find(key, asked) || m_fClock - asked > ROGUE_ASK)
		{
			m_mRogueAsked.Set(key, m_fClock);
			m_GM.Say(playerId, l.GetHeader(), f, string.Format("\"Nice jacket. %1 Favor and I never saw it.\" Stay and you've paid; walk off and you were never here.", price));
			RZ_Log.Info(string.Format("Disguise: Rogue %1 asked %2 for his price", l.m_sName, RZ_Util.PlayerName(playerId)));
			return true;
		}

		if (!m_GM.SpendFavor(playerId, f, price))
			return false;

		m_mRogueAsked.Remove(key);
		m_mRoguePaid.Set(key, m_fClock + ROGUE_PASS);
		array<string> payers;
		if (!m_mRoguePayers.Find(l.m_iId, payers))
		{
			payers = {};
			m_mRoguePayers.Set(l.m_iId, payers);
		}

		if (!payers.Contains(Ident(playerId)))
			payers.Insert(Ident(playerId));

		m_GM.Say(playerId, l.GetHeader(), f, string.Format("The Favor goes in a drawer. %1 writes something down. \"Ten minutes. Then I start remembering faces.\"", l.m_sShort));
		RZ_Log.Info(string.Format("Disguise: %1 paid Rogue %2 %3 Favor (10 minutes at his seat)", RZ_Util.PlayerName(playerId), l.m_sName, price));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A Rogue leader's work came out: everyone who paid him to look away is named in the records.
	void OnRogueFound(RZ_Leader l)
	{
		array<string> payers;
		if (!l || !m_mRoguePayers.Find(l.m_iId, payers))
			return;

		foreach (string ident : payers)
		{
			int playerId = m_GM.FindPlayerByIdentity(ident);
			if (playerId > 0)
			{
				m_GM.AddInfamy(playerId, l.m_eFaction, ROGUE_INFAMY, "paid a rogue to look away", RZ_RepFlag.SHARE);
				m_GM.NotifyPlayer(playerId, RZ_Factions.GetName(l.m_eFaction), string.Format("%1's drawer was opened. Your name was in it, next to a jacket. +%2 Infamy.", l.m_sShort, ROGUE_INFAMY));
			}
			else
			{
				OweInfamy(ident, l.m_eFaction, ROGUE_INFAMY);
			}
		}

		RZ_Log.Info(string.Format("Disguise: Rogue %1 found out: %2 payers named", l.m_sName, payers.Count()));
		m_mRoguePayers.Remove(l.m_iId);
	}

	// ================================================================ Hits, kills, colours
	//------------------------------------------------------------------------------------------------
	//! From retaliation: a player's shot hit (or killed) one of `f`'s people at `at`. While passing,
	//! a hit always gives you away (only an infected within 3 m of the victim excuses it): within 35 m
	//! of their people a catch, else exposed. The voucher answers too.
	void OnTheirsHit(int shooter, int f, bool killed, vector at)
	{
		if (shooter <= 0 || m_GM.GetSpawner().IsZombieNear(at, 3))
			return;

		OnVouchedTrouble(shooter, f, at, "shooting their people");
		if (!m_Disguise.IsPassing(shooter, f))
			return;

		IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(shooter);
		if (me && m_Disguise.NearTheirs(f, me.GetOrigin(), RZ_Disguise.CATCH_RADIUS))
			m_Disguise.Caught(shooter, f, me, "\"That one. The one in our kit.\"");
		else
			m_Disguise.Expose(shooter, f, "You hit one of theirs.");
	}

	//------------------------------------------------------------------------------------------------
	//! A kill of one of theirs that retaliation counts: word gets round. Hits never do.
	void OnKillCounted(int f, vector at)
	{
		WordGetsRound(f, at, true);
	}

	//------------------------------------------------------------------------------------------------
	//! In Bastion colours, during a feud between Bastion and the Red Hand, a Red Hand crew within
	//! 150 m that can see you opens up (the timed grudge, every look restarting it).
	protected void Feud(int playerId, IEntity me)
	{
		if (!m_Disguise.WearsColours(playerId, RZ_EFaction.MILITARY))
			return;

		RZ_FactionLife life = m_GM.GetFactionLife();
		if (!life || !life.HasFeud(RZ_EFaction.MILITARY, RZ_EFaction.BANDITS))
			return;

		if (!SeenBy(RZ_EFaction.BANDITS, me, FEUD_RADIUS))
			return;

		RZ_Hostility hostility = m_GM.GetHostility();
		if (hostility)
			hostility.AddGrudge(playerId, RZ_EFaction.BANDITS, me.GetOrigin());

		if (Tell(playerId.ToString() + "|feud", 120))
		{
			m_GM.Say(playerId, RZ_Factions.GetName(RZ_EFaction.BANDITS), RZ_EFaction.BANDITS, "\"Green! Green on the road!\" In a feud, the crews shoot army colours on sight, whoever's inside them.");
			RZ_Log.Info(string.Format("Disguise: %1 in Bastion colours seen by the Red Hand during a feud", RZ_Util.PlayerName(playerId)));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A strict stance (Shoots from: Watched or Strangers): in their kit, beyond 35 m of their people,
	//! they hold fire without a claim.
	bool StrictCover(int playerId, int f, IEntity me)
	{
		RZ_EHostileFrom from = m_Settings.GetFrom(f);
		if (from != RZ_EHostileFrom.WATCHED && from != RZ_EHostileFrom.STRANGERS)
			return false;

		if (!me || !m_Disguise.InTheirKit(playerId, f) || m_Disguise.IsBlown(playerId, f))
			return false;

		array<IEntity> people = {};
		NearestPeople(f, me.GetOrigin(), STRICT_RADIUS, people);
		return people.IsEmpty();
	}
}
