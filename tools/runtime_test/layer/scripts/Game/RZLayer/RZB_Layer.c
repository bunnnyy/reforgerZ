// Test-only behaviour layer test driver (tools/runtime_test/run.sh layer). Not part of the mod.
// In the site audit's Arland world with the mod's own game mode settings (places.py prepare: Bacon's
// prefabs in every slot) and two tribe slots (layer.py prepare), it puts the runtime test's fake player
// (id 901, a civilian Bacon attacks) near a tribe, starts a hunt with the function /tribes hunt calls
// (RZ_TribeBodies.StartHunt, force), and walks the layer through its modes, twice (noon and night):
//
//   HUNT     the pack is put out by the mod's own tribe code and steered to the player
//   ENGAGE   the first release: Bacon's chase and melee on the player (healed every second)
//   AWAY     the player is moved 450 m off: take-back, recover, steered again, the hunt lost
//   HOME     the pack walks home
//   HEAR     the game mode's own shot handler 105-125 m from a steered group, then the player gone again
//   SHOT     the player 21-26 m from a steered body for 3 s (still blind), then a shot: released at once
//   AWAY2    moved off again: take-back without a hunt
//   CONTACT  the player 5-7 m from a steered body, in its sight: the contact release
//   PUTAWAY  the player 800 m or more from every body: the bodies go back to data
//
// Every second it prints one row per tribe group ("RZB g"), per body ("RZB b": the layer's mode, the
// hook's mode, distance to the player, speed, distance to its slot, whether Bacon's own perception
// has the player, the stored perception factor, Bacon's melee stamp) and for the player ("RZB f":
// health lost that second). tools/runtime_test/layer.py reads them and prints the results table.
// It changes nothing in the layer: the only patches (layer.py prepare, on the throwaway copy) make
// the mod's player-entity lookups find the fake player.

//------------------------------------------------------------------------------------------------
//! The player-entity lookups, with the fake player in them (layer.py prepare swaps the mod's
//! GetGame().GetPlayerManager() calls under Zombies/ and in the game mode for these).
class RZB_Who
{
	static IEntity Ent(int playerId)
	{
		if (playerId == RZT_Fake.ID)
			return RZT_Fake.s_Ent;

		return GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
	}

	static int Id(IEntity e)
	{
		if (e && e == RZT_Fake.s_Ent)
			return RZT_Fake.ID;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(e);
	}

	static bool Online(int playerId)
	{
		if (playerId == RZT_Fake.ID)
			return RZT_Fake.s_Ent != null;

		return GetGame().GetPlayerManager().GetPlayerController(playerId) != null;
	}
}

//------------------------------------------------------------------------------------------------
class RZB
{
	static string F(float v)
	{
		return v.ToString(-1, 1);
	}

	static string F2(float v)
	{
		return v.ToString(-1, 2);
	}

	static string P(vector v)
	{
		return string.Format("%1,%2", RZ_Util.RoundToInt(v[0]), RZ_Util.RoundToInt(v[2]));
	}

	static int B(bool b)
	{
		if (b)
			return 1;

		return 0;
	}

	//! Health 0..1, -1 unknown
	static float Health(IEntity e)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(e);
		if (!c || !c.GetDamageManager())
			return -1;

		return c.GetDamageManager().GetHealthScaled();
	}

	//! Seconds since the observer's own perception saw the target, -1 if it doesn't have it at all
	static float SinceSeen(IEntity observer, IEntity target)
	{
		if (!observer || !target)
			return -1;

		PerceptionComponent perc = PerceptionComponent.Cast(observer.FindComponent(PerceptionComponent));
		if (!perc)
			return -1;

		BaseTarget bt = perc.FindTargetPerceptionObject(target);
		if (!bt)
			return -1;

		return bt.GetTimeSinceSeen();
	}
}

//------------------------------------------------------------------------------------------------
//! What Bacon's own character holds: its loiter point, and the slot our hook hands its Roam.
class RZB_Bacon
{
	static string Loiter(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter c = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!c)
			return "-";

		return RZB.P(c.GetLoiterPoint());
	}

	static string HookSlot(IEntity e)
	{
		Bacon_622120A5448725E3_InfectedCharacter c = Bacon_622120A5448725E3_InfectedCharacter.Cast(e);
		if (!c || !c.m_bRZSlot)
			return "-";

		return RZB.P(c.m_vRZSlot);
	}

	//! The group's current waypoint, and how many it has
	static string Waypoint(IEntity unit)
	{
		SCR_AIGroup group = SCR_AIGroup.Cast(unit);
		if (!group)
			return "nogroup";

		array<AIWaypoint> all = {};
		group.GetWaypoints(all);
		AIWaypoint wp = group.GetCurrentWaypoint();
		if (!wp)
			return string.Format("-/%1", all.Count());

		return string.Format("%1/%2", RZB.P(wp.GetOrigin()), all.Count());
	}
}

//------------------------------------------------------------------------------------------------
modded class RZ_GameModeComponent
{
	protected int m_iRZBSec;
	protected int m_iRZBStage;
	protected int m_iRZBStageAt;
	protected int m_iRZBMark;
	protected int m_iRZBPass;
	protected int m_iRZBReadyAt = -1;
	protected float m_fRZBLastT;
	protected float m_fRZBHp = 1;
	protected bool m_bRZBDeadSaid;
	protected RZ_Tribe m_RZBTribe;
	protected vector m_vRZBShot;
	protected float m_fRZBHome;
	protected float m_fRZBLost;
	protected bool m_bRZBShot;
	protected int m_iRZBIds;
	protected ref array<IEntity> m_aRZBBodies = {};
	protected ref array<int> m_aRZBBodyIds = {};
	protected ref array<vector> m_aRZBLast = {};
	protected ref array<float> m_aRZBHp = {};

	protected static const int RZB_LAND_WAIT = 1500;
	protected static const int RZB_PASSES = 2;

	protected static const int RZB_WAIT = 0;
	protected static const int RZB_SETUP = 1;
	protected static const int RZB_HUNT = 2;
	protected static const int RZB_ENGAGE = 3;
	protected static const int RZB_AWAY = 4;
	protected static const int RZB_HOME = 5;
	protected static const int RZB_HEAR = 6;
	protected static const int RZB_SHOTSET = 7;
	protected static const int RZB_SHOT = 8;
	protected static const int RZB_AWAY2 = 9;
	protected static const int RZB_CONTACTSET = 10;
	protected static const int RZB_CONTACT = 11;
	protected static const int RZB_PUTAWAY = 12;
	protected static const int RZB_NEXT = 13;
	protected static const int RZB_DONE = 14;

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		Print(string.Format("RZB start: world %1", GetGame().GetWorldFile()));
		GetGame().GetCallqueue().CallLater(RZB_Tick, 1000, true);
		GetGame().GetCallqueue().CallLater(RZB_Heal, 50, true);
	}

	//------------------------------------------------------------------------------------------------
	protected int RZB_In()
	{
		return m_iRZBSec - m_iRZBStageAt;
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Ev(string text)
	{
		Print(string.Format("RZB ev t=%1 st=%2 pass=%3 | %4", m_iRZBSec, m_iRZBStage, m_iRZBPass, text));
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Status()
	{
		string text = "";
		if (GetBehaviour())
			text = GetBehaviour().StatusText();

		string tribes = "";
		if (GetTribes() && GetTribes().GetBodies())
			tribes = GetTribes().GetBodies().StatusText();

		Print(string.Format("RZB status t=%1 st=%2 pass=%3 | %4 | %5", m_iRZBSec, m_iRZBStage, m_iRZBPass, text, tribes));
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Go(int stage, string why)
	{
		RZB_Status();
		Print(string.Format("RZB stage t=%1 from=%2 to=%3 pass=%4 | %5", m_iRZBSec, m_iRZBStage, stage, m_iRZBPass, why));
		m_iRZBStage = stage;
		m_iRZBStageAt = m_iRZBSec;
		m_iRZBMark = 0;
		RZB_Parts();
	}

	//------------------------------------------------------------------------------------------------
	//! The tribe's parts as data: where each is, whether it has bodies, who it is after.
	protected void RZB_Parts()
	{
		if (!m_RZBTribe)
			return;

		vector fp = "0 0 0";
		if (RZT_Fake.s_Ent)
			fp = RZT_Fake.s_Ent.GetOrigin();

		foreach (RZ_TribePart p : m_RZBTribe.m_aParts)
		{
			string a = string.Format("RZB part t=%1 st=%2 u=%3 kind=%4 n=%5 young=%6 pos=%7 rest=%8", m_iRZBSec, m_iRZBStage, p.m_iId, typename.EnumToString(RZ_ETribePart, p.m_eKind), p.m_iCount, p.m_iYoung, RZB.P(p.m_vPos), RZB.P(p.m_vRest));
			string b = string.Format(" out=%1 tgt=%2 held=%3 pack=%4 body=%5 df=%6 dl=%7", RZB.B(p.m_bOut), p.m_iTarget, RZB.B(p.m_bHeldBack), RZB.B(p.m_bPack), RZB.B(p.HasBody()), RZ_Util.RoundToInt(RZ_Util.DistXZ(p.m_vPos, fp)), RZ_Util.RoundToInt(RZ_Util.DistXZ(p.m_vPos, m_RZBTribe.m_vLair)));
			Print(a + b + string.Format(" home=%1 hold=%2 torest=%3", RZB.B(p.m_bHome), RZ_Util.RoundToInt(p.m_fHoldUntil - GetTribes().GetClock()), RZ_Util.RoundToInt(RZ_Util.DistXZ(p.m_vPos, p.m_vRest))));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every living body of a tribe group the layer drives, with its state and its plan.
	protected void RZB_Driven(notnull array<RZ_BodyState> states, notnull array<RZ_GroupPlan> plans)
	{
		array<ref RZ_TrackedZombie> tracked = GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Entity || z.m_iTribe < 0 || !z.m_Plan || !z.m_Plan.IsDriven())
				continue;

			foreach (RZ_BodyState b : z.m_Plan.GetBodies())
			{
				if (!b.m_Body || !RZ_Util.IsCharacterAlive(b.m_Body))
					continue;

				states.Insert(b);
				plans.Insert(z.m_Plan);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every living tribe body, driven or not.
	protected void RZB_AllBodies(notnull array<IEntity> bodies)
	{
		array<ref RZ_TrackedZombie> tracked = GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Entity || z.m_iTribe < 0)
				continue;

			array<IEntity> members = {};
			RZ_Util.GetUnitMembers(z.m_Entity, members);
			foreach (IEntity m : members)
			{
				if (m && RZ_Util.IsCharacterAlive(m))
					bodies.Insert(m);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int RZB_TribeUnits()
	{
		int n = 0;
		array<ref RZ_TrackedZombie> tracked = GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (z.m_Entity && z.m_iTribe >= 0)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected int RZB_Count(RZ_EBodyMode mode)
	{
		array<RZ_BodyState> states = {};
		array<RZ_GroupPlan> plans = {};
		RZB_Driven(states, plans);
		int n = 0;
		foreach (RZ_BodyState b : states)
		{
			if (b.m_eMode == mode)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected float RZB_NearestBody(vector pos)
	{
		array<IEntity> bodies = {};
		RZB_AllBodies(bodies);
		float best = 100000;
		foreach (IEntity e : bodies)
		{
			best = Math.Min(best, RZ_Util.DistXZ(e.GetOrigin(), pos));
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A dry spot rmin to rmax from centre, at least `clear` from every tribe body, outside no man's
	//! land and the line's band, on open level
	//! ground where one is found (RZ_CarRoom.Fits: nothing across it, nothing overhead), with the
	//! terrain's sight line to `eye` open when `sight`.
	protected bool RZB_Point(vector centre, float rmin, float rmax, float clear, bool sight, vector eye, out vector result)
	{
		array<IEntity> bodies = {};
		RZB_AllBodies(bodies);
		for (int i = 0; i < 120; i++)
		{
			vector cand = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(centre, rmin, rmax));
			if (RZ_Util.IsWater(cand) || RZ_Behaviour.WaterDepth(cand) > 0.2)
				continue;

			if (RZ_Util.NearestDistance(cand, bodies) < clear)
				continue;

			// Never in no man's land (a hunt ends there) or in the line's band
			if (GetLand().InNoMansLand(cand) || RZ_Perimeter.IsNearLineAt(cand))
				continue;

			if (sight && RZ_InfectedBodies.TerrainBlocks(eye, cand))
				continue;

			if (i < 80 && !RZ_CarRoom.Fits(cand, 0))
				continue;

			result = cand;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! The fake player stands at pos, alive and whole.
	protected void RZB_PutFake(vector pos)
	{
		if (RZT_Fake.s_Ent && !RZ_Util.IsCharacterAlive(RZT_Fake.s_Ent))
		{
			RZ_Util.DeleteUnit(RZT_Fake.s_Ent);
			RZT_Fake.s_Ent = null;
		}

		if (!RZT_Fake.s_Ent)
			RZT_Fake.Spawn(pos);
		else
			RZT_Fake.MoveTo(pos);

		RZ_Util.HealCharacter(RZT_Fake.s_Ent);
		m_fRZBHp = 1;
		m_bRZBDeadSaid = false;
	}

	//------------------------------------------------------------------------------------------------
	//! A dry spot `clear` to 620 m from where he stands and `clear` or more from every body: far
	//! enough to be lost, near enough that the bodies he leaves aren't put away (700 m).
	protected bool RZB_AwayPoint(vector from, float clear, out vector result)
	{
		array<IEntity> bodies = {};
		RZB_AllBodies(bodies);
		for (int i = 0; i < 600; i++)
		{
			vector cand = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(from, clear + 10, 620));
			if (RZ_Util.IsWater(cand) || RZ_Behaviour.WaterDepth(cand) > 0.2 || RZ_Perimeter.IsNearLineAt(cand))
				continue;

			if (RZ_Util.NearestDistance(cand, bodies) < clear)
				continue;

			if (i < 450 && GetLand().InNoMansLand(cand))
				continue;

			result = cand;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Far from every body: 420 m or more, tried round the lair and round where he stands.
	protected bool RZB_FakeAway(float clear)
	{
		vector from = m_RZBTribe.m_vLair;
		if (RZT_Fake.s_Ent)
			from = RZT_Fake.s_Ent.GetOrigin();

		vector p;
		if (!RZB_AwayPoint(from, clear, p) && !RZB_Point(from, clear, clear + 150, clear, false, from, p) && !RZB_Point(m_RZBTribe.m_vLair, clear + 100, clear + 700, clear, false, from, p))
		{
			RZB_Ev(string.Format("away: no spot %1 m from every body", clear));
			return false;
		}

		RZB_PutFake(p);
		RZB_Ev(string.Format("fake moved away to %1, nearest body %2 m, lair %3 m", RZB.P(p), RZ_Util.RoundToInt(RZB_NearestBody(p)), RZ_Util.RoundToInt(RZ_Util.DistXZ(p, m_RZBTribe.m_vLair))));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The game mode's own shot handler, as a player's unsuppressed shot.
	protected void RZB_Fire(string what)
	{
		IEntity fake = RZT_Fake.s_Ent;
		if (!fake)
			return;

		m_vRZBShot = fake.GetOrigin();
		m_bRZBShot = true;
		int steerBefore = RZB_Count(RZ_EBodyMode.STEER);
		int engageBefore = RZB_Count(RZ_EBodyMode.ENGAGE);
		OnPlayerShot(RZT_Fake.ID, null, fake);
		RZB_Ev(string.Format("shot %1 at %2, nearest body %3 m; steer %4 > %5, engage %6 > %7", what, RZB.P(m_vRZBShot), RZB.F(RZB_NearestBody(m_vRZBShot)), steerBefore, RZB_Count(RZ_EBodyMode.STEER), engageBefore, RZB_Count(RZ_EBodyMode.ENGAGE)));
	}

	//------------------------------------------------------------------------------------------------
	protected int RZB_BodyId(IEntity body, out vector last, vector pos)
	{
		int at = m_aRZBBodies.Find(body);
		if (at < 0)
		{
			m_iRZBIds++;
			m_aRZBBodies.Insert(body);
			m_aRZBBodyIds.Insert(m_iRZBIds);
			m_aRZBLast.Insert(pos);
			m_aRZBHp.Insert(-1);
			last = pos;
			return m_iRZBIds;
		}

		last = m_aRZBLast[at];
		m_aRZBLast[at] = pos;
		return m_aRZBBodyIds[at];
	}

	//------------------------------------------------------------------------------------------------
	//! A body's health fell since the last second: by how much, and who the damage manager blames.
	protected void RZB_Damage(IEntity body, int id, float d)
	{
		int at = m_aRZBBodies.Find(body);
		if (at < 0)
			return;

		float hp = RZ_Util.GetCharacterHealth(body);
		float was = m_aRZBHp[at];
		m_aRZBHp[at] = hp;
		if (was < 0 || hp >= was - 0.01)
			return;

		string who = "none";
		int type = -1;
		bool uncon = false;
		ChimeraCharacter c = ChimeraCharacter.Cast(body);
		if (c && c.GetDamageManager())
		{
			Instigator inst = c.GetDamageManager().GetInstigator();
			if (inst)
			{
				type = inst.GetInstigatorType();
				IEntity ie = inst.GetInstigatorEntity();
				if (ie == body)
					who = "self";
				else if (ie && ie == RZT_Fake.s_Ent)
					who = "fake";
				else if (ie)
					who = ie.ClassName();
			}
		}

		if (c && c.GetCharacterController())
			uncon = c.GetCharacterController().IsUnconscious();

		Print(string.Format("RZB dmg t=%1 st=%2 i=%3 from=%4 to=%5 by=%6 type=%7 uncon=%8 d=%9", m_iRZBSec, m_iRZBStage, id, RZB.F2(was), RZB.F2(hp), who, type, RZB.B(uncon), RZB.F(d)));
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_BodyRow(RZ_TrackedZombie z, RZ_BodyState b, IEntity fake, vector fp, float dt, float now, int unit)
	{
		vector pos = b.m_Body.GetOrigin();
		vector last;
		int id = RZB_BodyId(b.m_Body, last, pos);
		float v = RZ_Util.DistXZ(pos, last) / Math.Max(0.1, dt);
		float d = -1;
		int tg = 0;
		float ss = -1;
		int tb = -1;
		if (fake)
		{
			d = RZ_Util.DistXZ(pos, fp);
			array<IEntity> one = {fake};
			int idx;
			float since;
			vector lastPos;
			if (RZ_Senses.FindPlayerTarget(b.m_Body, one, idx, since, lastPos))
				tg = 1;

			ss = RZB.SinceSeen(b.m_Body, fake);
			tb = RZB.B(RZ_InfectedBodies.TerrainBlocks(pos, fp));
		}

		RZB_Damage(b.m_Body, id, d);
		float mel = -1;
		float stamp = RZ_BaconBody.MeleeAt(b.m_Body);
		if (stamp > 0)
			mel = now - stamp;

		string a = string.Format("RZB b t=%1 st=%2 u=%3 i=%4 k=%5 m=%6 hm=%7 d=%8 v=%9", m_iRZBSec, m_iRZBStage, unit, id, typename.EnumToString(RZ_EBodyKind, b.m_eKind), typename.EnumToString(RZ_EBodyMode, b.m_eMode), RZ_BaconBody.GetMode(b.m_Body), RZB.F(d), RZB.F2(v));
		string c = string.Format(" sd=%1 tg=%2 ss=%3 pf=%4 hp=%5 mel=%6 p=%7 tb=%8 cap=%9", RZB.F(RZ_Util.DistXZ(pos, b.m_vSlot)), tg, RZB.F(ss), RZB.F2(RZ_Senses.GetOwnFactor(b.m_Body)), RZB.F2(RZB.Health(b.m_Body)), RZB.F(mel), RZB.P(pos), tb, RZB.F2(b.m_fCap));
		Print(a + c + string.Format(" slot=%1 hook=%2 loiter=%3 y=%4", RZB.P(b.m_vSlot), RZB_Bacon.HookSlot(b.m_Body), RZB_Bacon.Loiter(b.m_Body), RZB.F(pos[1])));
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_GroupRow(RZ_TrackedZombie z, vector fp, float now, int unit)
	{
		RZ_GroupPlan plan = z.m_Plan;
		vector c = plan.Centre();
		float seen = -1;
		string strat = "-";
		RZ_HuntLink link = plan.GetLink();
		if (link)
		{
			seen = now - link.m_fLastSeenAt;
			strat = typename.EnumToString(RZ_EStrategy, link.m_eStrategy);
		}

		float ds = -1;
		if (m_bRZBShot)
			ds = RZ_Util.DistXZ(c, m_vRZBShot);

		string kind = "-";
		int tgt = -1;
		RZ_TribePart part = RZ_TribePart.Cast(z.m_Part);
		if (part)
		{
			kind = typename.EnumToString(RZ_ETribePart, part.m_eKind);
			tgt = part.m_iTarget;
		}

		string a = string.Format("RZB g t=%1 st=%2 u=%3 part=%4 n=%5 drv=%6 hunt=%7 hs=%8 ord=%9", m_iRZBSec, m_iRZBStage, unit, kind, plan.Living(), RZB.B(plan.IsDriven()), RZB.B(plan.IsHunting()), typename.EnumToString(RZ_EHuntState, plan.GetHuntState()), typename.EnumToString(RZ_EPlanOrder, plan.GetOrder()));
		string b = string.Format(" arr=%1 slp=%2 deaf=%3 c=%4 dl=%5 df=%6 ds=%7 seen=%8 strat=%9", RZB.B(plan.Arrived()), RZB.B(plan.IsAsleep()), RZB.B(plan.m_bDeaf), RZB.P(c), RZ_Util.RoundToInt(RZ_Util.DistXZ(c, m_RZBTribe.m_vLair)), RZ_Util.RoundToInt(RZ_Util.DistXZ(c, fp)), RZB.F(ds), RZB.F(seen), strat);
		Print(a + b + string.Format(" tgt=%1 wp=%2", tgt, RZB_Bacon.Waypoint(z.m_Entity)));
	}

	//------------------------------------------------------------------------------------------------
	//! Twenty times a second: the health the player lost since the last look is added up and he is made
	//! whole, so a fight can be measured for as long as it lasts (a pack takes most of a man's
	//! health in a second).
	void RZB_Heal()
	{
		IEntity fake = RZT_Fake.s_Ent;
		if (!fake || !RZ_Util.IsCharacterAlive(fake))
			return;

		float hp = RZB.Health(fake);
		if (hp < 0 || hp >= m_fRZBHp - 0.002)
			return;

		m_fRZBLost += m_fRZBHp - hp;
		RZ_Util.HealCharacter(fake);
		m_fRZBHp = RZB.Health(fake);
	}

	//------------------------------------------------------------------------------------------------
	//! One second's rows: the tribe's groups, their bodies, the player.
	protected void RZB_Sample()
	{
		float now = RZ_BaconBody.Now();
		float dt = now - m_fRZBLastT;
		m_fRZBLastT = now;
		IEntity fake = RZT_Fake.s_Ent;
		bool alive = fake && RZ_Util.IsCharacterAlive(fake);
		vector fp = "0 0 0";
		if (fake)
			fp = fake.GetOrigin();

		int others = 0;
		int undriven = 0;
		array<ref RZ_TrackedZombie> tracked = GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (!z.m_Entity)
				continue;

			if (z.m_iTribe < 0 || !z.m_Plan)
			{
				if (fake && RZ_Util.DistXZ(z.m_vLastPos, fp) < 150)
					others += z.Agents();

				continue;
			}

			int unit = -1;
			if (z.m_Part)
				unit = z.m_Part.m_iId;

			RZB_GroupRow(z, fp, now, unit);
			if (!z.m_Plan.IsDriven())
				undriven += z.Agents();

			foreach (RZ_BodyState b : z.m_Plan.GetBodies())
			{
				if (b.m_Body && RZ_Util.IsCharacterAlive(b.m_Body))
					RZB_BodyRow(z, b, fake, fp, dt, now, unit);
			}
		}

		RZB_Heal();
		float hp = -1;
		if (fake)
			hp = RZB.Health(fake);

		float lost = m_fRZBLost;
		m_fRZBLost = 0;
		int phase = -1;
		if (GetTribes())
			phase = GetTribes().GetPhase();

		Print(string.Format("RZB f t=%1 st=%2 p=%3 hp=%4 alive=%5 lost=%6 others=%7 undriven=%8 hour=%9", m_iRZBSec, m_iRZBStage, RZB.P(fp), RZB.F2(hp), RZB.B(alive), RZB.F2(lost), others, undriven, RZB.F(RZ_GameTime.GetHour())) + string.Format(" phase=%1 dt=%2 nomans=%3", phase, RZB.F2(dt), RZB.B(fake && GetLand().InNoMansLand(fp))));
		if (fake && !alive && !m_bRZBDeadSaid)
		{
			m_bRZBDeadSaid = true;
			RZB_Ev(string.Format("the fake player died at %1 (%2 other infected within 150 m); a new one stands there", RZB.P(fp), others));
			// A real player stays dead for a while, and the hunt's tick ends a hunt whose target is
			// dead (RZ_TribeBodies.HuntTick). The test's new player stands there within the second,
			// so the hunt would never see the death: it is ended here with the same call
			if (m_RZBTribe && m_RZBTribe.m_Hunt && m_RZBTribe.m_Hunt.m_iTarget == RZT_Fake.ID)
				GetTribes().GetBodies().EndHunt(m_RZBTribe, false);

			RZB_PutFake(fp);
		}

		if (m_iRZBSec % 15 == 0)
			RZB_Status();

		if (m_iRZBSec % 30 == 0)
			RZB_Parts();
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_SetHour(float hour)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity tm = world.GetTimeAndWeatherManager();
		if (!tm)
			return;

		tm.SetTimeOfTheDay(hour, true);
		tm.SetIsDayAutoAdvanced(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Land grid and tribes there: the biggest tribe is the one tested.
	protected void RZB_Wait()
	{
		RZ_Land land = GetLand();
		bool ready = !land || !land.IsEnabled() || land.IsReady();
		RZ_Tribes tribes = GetTribes();
		int n = 0;
		if (tribes)
			n = tribes.GetTribeList().Count();

		if ((!ready || n == 0) && m_iRZBSec < RZB_LAND_WAIT)
		{
			if (m_iRZBSec % 30 == 0)
				Print(string.Format("RZB t=%1 s: waiting (land ready %2, tribes %3)", m_iRZBSec, ready, n));

			return;
		}

		if (n == 0 || !GetBehaviour() || !RZ_Behaviour.IsOn())
		{
			Print(string.Format("RZB FAILED: no tribe to test (land ready %1, tribes %2, behaviour on %3)", ready, n, RZ_Behaviour.IsOn()));
			m_iRZBStage = RZB_DONE;
			GetGame().GetCallqueue().CallLater(RZB_Close, 2000, false);
			return;
		}

		if (m_iRZBReadyAt < 0)
		{
			m_iRZBReadyAt = m_iRZBSec + 20;
			return;
		}

		if (m_iRZBSec < m_iRZBReadyAt)
			return;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			Print(string.Format("RZB tribe id=%1 place=%2 lair=%3 temper=%4 members=%5 parts=%6", t.m_iId, t.m_sPlace, RZB.P(t.m_vLair), typename.EnumToString(RZ_ETemperament, t.m_eTemper), t.Members(), t.m_aParts.Count()));
			if (!m_RZBTribe || t.Members() > m_RZBTribe.Members())
				m_RZBTribe = t;
		}

		m_fRZBLastT = RZ_BaconBody.Now();
		RZB_Go(RZB_SETUP, "land and tribes ready");
	}

	//------------------------------------------------------------------------------------------------
	//! The hour, then the player 380-450 m from the lair on open ground.
	protected void RZB_Setup()
	{
		if (m_iRZBMark == 0)
		{
			float hour = 12;
			if (m_iRZBPass % 2 == 1)
				hour = 23;

			RZB_SetHour(hour);
			vector lair = m_RZBTribe.m_vLair;
			vector p;
			if (!RZB_Point(lair, 380, 450, 150, false, lair, p))
				p = RZ_Util.GroundPos(lair + Vector(400, 0, 0));

			RZB_PutFake(p);
			m_bRZBShot = false;
			RZB_Ev(string.Format("pass %1 at hour %2: fake at %3, %4 m from the lair of %5", m_iRZBPass, hour, RZB.P(p), RZ_Util.RoundToInt(RZ_Util.DistXZ(p, lair)), m_RZBTribe.m_sPlace));
			m_iRZBMark = 1;
			return;
		}

		if (RZB_In() >= 25)
			RZB_Go(RZB_HUNT, "settled");
	}

	//------------------------------------------------------------------------------------------------
	//! The hunt, started by the function /tribes hunt calls; until the first release.
	protected void RZB_Hunt()
	{
		if (m_iRZBMark == 0)
		{
			bool ok = GetTribes().GetBodies().StartHunt(m_RZBTribe, RZT_Fake.ID, true);
			string strat = "-";
			if (m_RZBTribe.m_Hunt)
				strat = typename.EnumToString(RZ_EStrategy, m_RZBTribe.m_Hunt.m_eStrategy);

			RZB_Ev(string.Format("StartHunt(force) returned %1, strategy %2", ok, strat));
			m_iRZBMark = 1;
			RZB_Parts();
			return;
		}

		int engaged = RZB_Count(RZ_EBodyMode.ENGAGE);
		if (engaged > 0)
		{
			RZB_Go(RZB_ENGAGE, string.Format("first release after %1 s (%2 engaged)", RZB_In(), engaged));
			return;
		}

		if (!m_RZBTribe.m_Hunt && m_iRZBMark == 1)
		{
			m_iRZBMark = 2;
			RZB_Ev(string.Format("the hunt ended by itself after %1 s, before any release", RZB_In()));
		}

		if (m_iRZBMark == 2 && RZB_In() >= 120)
		{
			RZB_Go(RZB_AWAY, "no hunt and no release in 120 s");
			return;
		}

		if (RZB_In() >= 480)
			RZB_Go(RZB_AWAY, "no release in 480 s");
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Engage()
	{
		if (RZB_In() >= 50)
			RZB_Go(RZB_AWAY, "50 s of the fight");
	}

	//------------------------------------------------------------------------------------------------
	//! Moved off: until nobody is engaged or recovering and the hunt is over.
	protected void RZB_Away(int next, int least, int most)
	{
		if (m_iRZBMark == 0)
		{
			RZB_FakeAway(420);
			m_iRZBMark = 1;
			return;
		}

		int busy = RZB_Count(RZ_EBodyMode.ENGAGE) + RZB_Count(RZ_EBodyMode.RECOVER);
		bool hunting = m_RZBTribe.m_Hunt != null;
		if (RZB_In() >= least && busy == 0 && !hunting)
		{
			RZB_Go(next, string.Format("all steered and the hunt over after %1 s", RZB_In()));
			return;
		}

		if (RZB_In() >= most)
			RZB_Go(next, string.Format("timeout: %1 still engaged or recovering, hunt on %2", busy, hunting));
	}

	//------------------------------------------------------------------------------------------------
	//! Metres the parts sent home still have to their rest spots at the lair, as data.
	protected float RZB_HomeLeft()
	{
		float sum = 0;
		foreach (RZ_TribePart p : m_RZBTribe.m_aParts)
		{
			if (p.m_bHome)
				sum += RZ_Util.DistXZ(p.m_vPos, p.m_vRest);
		}

		return sum;
	}

	//------------------------------------------------------------------------------------------------
	//! The walk home. A part sent home holds where it is for up to 10 minutes first (RZ_Tribes.PathHome),
	//! so the first pass waits for the walk itself (11 minutes at most); the others watch 60 s.
	protected void RZB_Home()
	{
		if (m_iRZBMark == 0)
		{
			m_fRZBHome = RZB_HomeLeft();
			m_iRZBMark = 1;
			RZB_Ev(string.Format("home: the parts sent home have %1 m to walk in all", RZ_Util.RoundToInt(m_fRZBHome)));
			return;
		}

		float came = m_fRZBHome - RZB_HomeLeft();
		int most = 60;
		if (m_iRZBPass == 0)
			most = 660;

		if (RZB_In() >= 60 && came >= 40)
		{
			RZB_Go(RZB_HEAR, string.Format("walking home: %1 m nearer their rest spots after %2 s", RZ_Util.RoundToInt(came), RZB_In()));
			return;
		}

		if (RZB_In() >= most)
			RZB_Go(RZB_HEAR, string.Format("still holding: %1 m nearer their rest spots after %2 s", RZ_Util.RoundToInt(came), RZB_In()));
	}

	//------------------------------------------------------------------------------------------------
	//! A dry spot 720 m or more from every tribe body, anywhere on the land round the lair.
	protected bool RZB_FarPoint(out vector result)
	{
		array<IEntity> bodies = {};
		RZB_AllBodies(bodies);
		for (int i = 0; i < 800; i++)
		{
			vector cand = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(m_RZBTribe.m_vLair, 720, 3500));
			if (RZ_Util.IsWater(cand) || RZ_Behaviour.WaterDepth(cand) > 0.2 || RZ_Perimeter.IsNearLineAt(cand))
				continue;

			if (RZ_Util.NearestDistance(cand, bodies) < 720)
				continue;

			result = cand;
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A shot 105-125 m from a steered group that isn't hunting, deaf or asleep; then the player gone.
	protected void RZB_Hear()
	{
		if (m_iRZBMark == 0)
		{
			array<RZ_BodyState> states = {};
			array<RZ_GroupPlan> plans = {};
			RZB_Driven(states, plans);
			RZ_GroupPlan pick;
			foreach (RZ_GroupPlan plan : plans)
			{
				if (plan.IsHunting() || plan.IsAsleep() || plan.m_bDeaf)
					continue;

				if (!pick || plan.Living() > pick.Living())
					pick = plan;
			}

			vector x;
			if (!pick || !RZB_Point(pick.Centre(), 105, 125, 90, false, pick.Centre(), x))
			{
				RZB_Go(RZB_SHOTSET, "hearing skipped: no steered group that takes leads, or no spot");
				return;
			}

			RZB_PutFake(x);
			RZB_Ev(string.Format("hearing: fake %1 m from a %2 group's centre", RZ_Util.RoundToInt(RZ_Util.DistXZ(x, pick.Centre())), typename.EnumToString(RZ_EBodyKind, pick.m_eKind)));
			RZB_Fire("hearing");
			m_iRZBMark = 1;
			return;
		}

		if (m_iRZBMark == 1)
		{
			RZB_FakeAway(420);
			m_iRZBMark = 2;
			return;
		}

		if (RZB_In() >= 75)
			RZB_Go(RZB_SHOTSET, "75 s after the shot");
	}

	//------------------------------------------------------------------------------------------------
	//! A steered body of a group that isn't hunting or asleep, nearest first; or any steered body.
	protected RZ_BodyState RZB_PickSteered()
	{
		array<RZ_BodyState> states = {};
		array<RZ_GroupPlan> plans = {};
		RZB_Driven(states, plans);
		RZ_BodyState pick;
		RZ_BodyState any;
		foreach (int i, RZ_BodyState b : states)
		{
			if (b.m_eMode != RZ_EBodyMode.STEER || plans[i].IsAsleep())
				continue;

			if (!any)
				any = b;

			if (!pick && !plans[i].IsHunting())
				pick = b;
		}

		if (pick)
			return pick;

		return any;
	}

	//------------------------------------------------------------------------------------------------
	//! The player 21-26 m from a steered body, in its sight, nobody within 12 m: 3 s, then the shot.
	protected void RZB_ShotSet()
	{
		if (m_iRZBMark == 0)
		{
			RZ_BodyState b = RZB_PickSteered();
			vector p;
			if (!b || !RZB_Point(b.m_Body.GetOrigin(), 21, 26, 12, true, b.m_Body.GetOrigin(), p))
			{
				RZB_Go(RZB_AWAY2, "shot release skipped: no steered body, or no spot 21-26 m from one");
				return;
			}

			RZB_PutFake(p);
			RZB_Ev(string.Format("shot release: fake at %1, nearest body %2 m", RZB.P(p), RZB.F(RZB_NearestBody(p))));
			m_iRZBMark = 1;
			return;
		}

		if (RZB_In() < 4)
			return;

		RZB_Fire("release");
		RZB_Go(RZB_SHOT, "shot fired");
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Shot()
	{
		if (RZB_In() >= 25)
			RZB_Go(RZB_AWAY2, "25 s after the shot");
	}

	//------------------------------------------------------------------------------------------------
	//! The player 5-7 m from a steered body, in its sight.
	protected void RZB_ContactSet()
	{
		RZ_BodyState b = RZB_PickSteered();
		vector p;
		if (!b || !RZB_Point(b.m_Body.GetOrigin(), 5, 7, 3.5, true, b.m_Body.GetOrigin(), p))
		{
			if (RZB_In() < 20)
				return;

			RZB_Go(RZB_PUTAWAY, "contact skipped: no steered body, or no spot 5-7 m from one");
			return;
		}

		RZB_PutFake(p);
		RZB_Ev(string.Format("contact: fake at %1, nearest body %2 m", RZB.P(p), RZB.F(RZB_NearestBody(p))));
		RZB_Go(RZB_CONTACT, "placed");
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Contact()
	{
		if (RZB_In() >= 25)
			RZB_Go(RZB_PUTAWAY, "25 s after the contact");
	}

	//------------------------------------------------------------------------------------------------
	//! The player 800 m or more from every body (or gone, on a map too small for that): the bodies
	//! go back to data.
	protected void RZB_PutAway()
	{
		if (m_iRZBMark == 0)
		{
			m_iRZBMark = 1;
			vector p;
			if (RZB_FarPoint(p))
			{
				RZB_PutFake(p);
				RZB_Ev(string.Format("put away: fake at %1, nearest body %2 m", RZB.P(p), RZ_Util.RoundToInt(RZB_NearestBody(p))));
				return;
			}

			if (RZT_Fake.s_Ent)
				RZ_Util.DeleteUnit(RZT_Fake.s_Ent);

			RZT_Fake.s_Ent = null;
			RZB_Ev("put away: no spot 720 m from every body, the fake player is taken out of the world");
			return;
		}

		int units = RZB_TribeUnits();
		if (units == 0)
		{
			RZB_Go(RZB_NEXT, string.Format("every tribe body back to data after %1 s", RZB_In()));
			return;
		}

		if (RZB_In() >= 300)
			RZB_Go(RZB_NEXT, string.Format("timeout: %1 tribe groups still have bodies", units));
	}

	//------------------------------------------------------------------------------------------------
	protected void RZB_Next()
	{
		m_iRZBPass++;
		if (m_iRZBPass < RZB_PASSES)
		{
			RZB_Go(RZB_SETUP, "next pass");
			return;
		}

		RZB_Status();
		Print(string.Format("RZB done: %1 passes in %2 s", m_iRZBPass, m_iRZBSec));
		m_iRZBStage = RZB_DONE;
		GetGame().GetCallqueue().CallLater(RZB_Close, 2000, false);
	}

	//------------------------------------------------------------------------------------------------
	void RZB_Tick()
	{
		m_iRZBSec++;
		if (m_iRZBStage == RZB_DONE)
			return;

		if (m_iRZBStage == RZB_WAIT)
		{
			RZB_Wait();
			return;
		}

		RZB_Sample();
		switch (m_iRZBStage)
		{
			case RZB_SETUP: RZB_Setup(); break;
			case RZB_HUNT: RZB_Hunt(); break;
			case RZB_ENGAGE: RZB_Engage(); break;
			case RZB_AWAY: RZB_Away(RZB_HOME, 30, 240); break;
			case RZB_HOME: RZB_Home(); break;
			case RZB_HEAR: RZB_Hear(); break;
			case RZB_SHOTSET: RZB_ShotSet(); break;
			case RZB_SHOT: RZB_Shot(); break;
			case RZB_AWAY2: RZB_Away(RZB_CONTACTSET, 30, 150); break;
			case RZB_CONTACTSET: RZB_ContactSet(); break;
			case RZB_CONTACT: RZB_Contact(); break;
			case RZB_PUTAWAY: RZB_PutAway(); break;
			case RZB_NEXT: RZB_Next(); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void RZB_Close()
	{
		GetGame().RequestClose();
	}
}
