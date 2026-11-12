//! A tribe group's plan in the behaviour layer (docs/INFECTED.md, "How they move and hunt"): its
//! owner's order (hold a ring, walk somewhere, go home), its senses (the notice meter, leads from
//! shots, memory of the last-known spot), its formation and slots (spacing enforced, dry, off stuck
//! marks), and its part in a hunt (RZ_HuntLink: rush, stalk, flank, encircle, flush, bait, herd,
//! ambush, give up with a hand-off). Each body's mode is switched here (release to Bacon, take
//! back, wait below stairs, the unstick ladder).

//------------------------------------------------------------------------------------------------
enum RZ_EStrategy
{
	NONE,
	RUSH,		// straight in, fanned out
	STALK,		// shadow from cover, strike at a moment
	PINCER,		// two groups to flank points either side, strike together
	ENCIRCLE,	// a ring with a gap, closing
	FLUSH,		// one pushes in noisily, the rest wait on the far side
	BAIT,		// the young go in loud, the hunters come up the target's back trail
	HERD,		// a ring whose gap faces a crawler town: pressed toward its edge
	AMBUSH		// one group lies still by a crossing or a habit place ahead
}

//------------------------------------------------------------------------------------------------
//! What a hunt's owner knows about its target's ground, for the strategies that need it
//! (docs/INFECTED.md, Hunt strategies). Built by the tribes (RZ_TribeBodies.Context).
class RZ_HuntContext
{
	bool m_bHidden;				// in a wood or a building: flush
	bool m_bYoung;				// the hunt has young with it: bait
	int m_iHerdTown = -1;		// a crawler town within 500 m: herd
	vector m_vHerdTown;
	float m_fHerdRadius;
	bool m_bAmbush;				// a crossing or a habit place within 300 m ahead: ambush
	vector m_vAmbush;
	bool m_bHabit;				// ...and it's a habit place (+2)
	bool m_bNoFlank;			// the two weak days after a succession: no pincer, ring or herd
}

//------------------------------------------------------------------------------------------------
enum RZ_EPlanOrder
{
	HOLD,		// a ring round a spot (the lair, a post, a kill site, the vigil)
	WALK,		// to a spot (a trail, a day walk, a noise)
	HOME		// to a spot, giving up whoever it had
}

//------------------------------------------------------------------------------------------------
enum RZ_EGait
{
	WALKSTOP,	// no group waypoint, slots under 25 m: Bacon walks and stops (lab B7: 0.45-0.8 m/s)
	MARCH,		// a held waypoint (RUN), capped at the kind's march speed (about 2 m/s)
	RUN			// a held waypoint, no cap (3.7 m/s)
}

//------------------------------------------------------------------------------------------------
enum RZ_EFormation
{
	CLUSTER,
	FILE,
	LINE,
	WEDGE,
	RING,
	PAIRS
}

//------------------------------------------------------------------------------------------------
enum RZ_EHuntState
{
	APPROACH,
	SET,
	STRIKE,
	SEARCH,
	OVER
}

//------------------------------------------------------------------------------------------------
//! One hunt: the groups after one player, what they know of them, and the strategy.
class RZ_HuntLink
{
	int m_iTarget = -1;
	IEntity m_TargetEnt;			// weak
	RZ_EStrategy m_eStrategy;
	bool m_bRelentless;				// a night hunt on its trail: it ends by the hide rule, not by our memory alone
	bool m_bDay;
	float m_fStartedAt;
	vector m_vLastKnown;
	vector m_vMoving;				// the target's last heading (flat, normalised; zero when still)
	float m_fLastSeenAt;
	vector m_vGap;					// the encircle's gap faces this way from the target
	float m_fRing = 120;
	float m_fArrivedAt = -1;		// the first element in position
	bool m_bStrike;
	bool m_bOver;
	ref array<RZ_GroupPlan> m_aPlans = {};	// weak
	// Flush, bait, herd, ambush
	vector m_vFlushFrom;			// where the target was when the flush began
	bool m_bFlushSet;
	int m_iHerdTown = -1;
	vector m_vHerdTown;
	float m_fHerdRadius;
	bool m_bHerdStirred;
	vector m_vAmbush;
	// The re-plan: the strategies plan round this spot, moved only when the target has gone 25 m
	vector m_vPlanned;
	bool m_bPlanned;

	static const int MAX_ELEMENTS = 3;
	static const float REPLAN = 25;
	static const float CLUSTER_RADIUS = 150;

	//------------------------------------------------------------------------------------------------
	bool IsAlive()
	{
		if (m_bOver)
			return false;

		foreach (RZ_GroupPlan p : m_aPlans)
		{
			if (p && p.GetLink() == this)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	int CountPlans()
	{
		int n = 0;
		foreach (RZ_GroupPlan p : m_aPlans)
		{
			if (p && p.GetLink() == this)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! The plan that is this hunt's element i, or null.
	RZ_GroupPlan ElementPlan(int element)
	{
		foreach (RZ_GroupPlan p : m_aPlans)
		{
			if (p && p.GetLink() == this && p.GetElement() == element)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The spot the strategies plan round (docs/INFECTED.md, Formations): the last-known spot, moved
	//! only when the target has gone 25 m from it, so the flank and shadow points don't jitter. The
	//! strike always runs at the last-known spot itself.
	vector Planned()
	{
		if (!m_bPlanned || RZ_Util.DistXZ(m_vPlanned, m_vLastKnown) >= REPLAN)
		{
			m_vPlanned = m_vLastKnown;
			m_bPlanned = true;
		}

		return m_vPlanned;
	}

	//------------------------------------------------------------------------------------------------
	//! The longest time to its place among this hunt's elements still on their way, other than one.
	float SlowestEta(RZ_GroupPlan except)
	{
		float slowest = 0;
		foreach (RZ_GroupPlan p : m_aPlans)
		{
			if (p && p != except && p.GetLink() == this && p.GetHuntState() == RZ_EHuntState.APPROACH)
				slowest = Math.Max(slowest, p.GetEta());
		}

		return slowest;
	}

	//------------------------------------------------------------------------------------------------
	//! Every element of the hunt is in position.
	bool AllSet()
	{
		foreach (RZ_GroupPlan p : m_aPlans)
		{
			if (p && p.GetLink() == this && p.GetHuntState() == RZ_EHuntState.APPROACH)
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Picks a strategy by temperament (docs/INFECTED.md, Hunt strategies): a strategy whose needs
	//! aren't met weighs 0, and the roll picks the top two 70/30. By day only rush, stalk and a
	//! two-group pincer; a Rising alpha's tribe hunts with the cautious weights; in the weak days no
	//! flank. A habit place ahead adds 2 to the ambush.
	//! temper: RZ_ETemperament (0 bold, 1 cautious, 2 cunning, 3 territorial, 4 nomadic).
	static RZ_EStrategy Choose(int temper, int groups, int bodies, bool day, bool rising, RZ_HuntContext ctx = null)
	{
		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (!bh || !bh.GetSettings().m_bStrategies)
			return RZ_EStrategy.RUSH;

		int t = temper;
		if (rising)
			t = 1;

		array<float> rush = {3, 0, 0, 1, 1};
		array<float> stalk = {1, 3, 2, 1, 2};
		array<float> pincer = {2, 1, 2, 3, 1};
		array<float> ring = {3, 0.5, 1, 2, 1};
		array<float> flush = {2, 1, 1, 2, 1};
		array<float> bait = {0, 0, 3, 0, 1};
		array<float> herd = {1, 1, 2, 0, 2};
		array<float> ambush = {0, 2, 3, 2, 0};
		t = Math.ClampInt(t, 0, 4);
		array<float> w = {rush[t], stalk[t], pincer[t], ring[t], flush[t], bait[t], herd[t], ambush[t]};
		if (groups < 2)
			w[2] = 0;

		if (bodies < 5 || day)
			w[3] = 0;

		// Night-only, each needing its ground
		if (day || !ctx || !ctx.m_bHidden || groups < 2)
			w[4] = 0;

		if (day || !ctx || !ctx.m_bYoung)
			w[5] = 0;

		if (day || !ctx || ctx.m_iHerdTown < 0 || bodies < 5)
			w[6] = 0;

		if (day || !ctx || !ctx.m_bAmbush)
			w[7] = 0;
		else if (ctx.m_bHabit)
			w[7] = w[7] + 2;

		if (ctx && ctx.m_bNoFlank)
		{
			w[2] = 0;
			w[3] = 0;
			w[6] = 0;
		}

		// The top two, 70/30
		int best = -1;
		int second = -1;
		for (int i = 0; i < 8; i++)
		{
			if (w[i] <= 0)
				continue;

			if (best < 0 || w[i] > w[best])
			{
				second = best;
				best = i;
			}
			else if (second < 0 || w[i] > w[second])
			{
				second = i;
			}
		}

		if (best < 0)
			return RZ_EStrategy.RUSH;

		int pick = best;
		if (second >= 0 && Math.RandomFloat01() < 0.3)
			pick = second;

		switch (pick)
		{
			case 1: return RZ_EStrategy.STALK;
			case 2: return RZ_EStrategy.PINCER;
			case 3: return RZ_EStrategy.ENCIRCLE;
			case 4: return RZ_EStrategy.FLUSH;
			case 5: return RZ_EStrategy.BAIT;
			case 6: return RZ_EStrategy.HERD;
			case 7: return RZ_EStrategy.AMBUSH;
		}

		return RZ_EStrategy.RUSH;
	}

	//------------------------------------------------------------------------------------------------
	//! The ground a context gives this hunt (the herd's town and its gap, the ambush's place).
	void Apply(RZ_HuntContext ctx)
	{
		if (!ctx)
			return;

		m_iHerdTown = ctx.m_iHerdTown;
		m_vHerdTown = ctx.m_vHerdTown;
		m_fHerdRadius = ctx.m_fHerdRadius;
		m_vAmbush = ctx.m_vAmbush;
		if (m_eStrategy == RZ_EStrategy.HERD)
		{
			vector gap = m_vHerdTown - m_vLastKnown;
			gap[1] = 0;
			if (gap.Length() > 1)
			{
				gap.Normalize();
				m_vGap = gap;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A hunt on one player by these groups (a strategy picked unless one is given). Null with no
	//! group or no layer.
	static RZ_HuntLink Start(array<RZ_GroupPlan> plans, int playerId, RZ_EStrategy strategy, int temper, bool rising, bool day, bool relentless, RZ_HuntContext ctx = null)
	{
		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (!bh || plans.IsEmpty())
			return null;

		int bodies = 0;
		foreach (RZ_GroupPlan counted : plans)
		{
			bodies += counted.Living();
		}

		// At most 3 elements on one player
		int elements = Math.Min(plans.Count(), MAX_ELEMENTS);
		RZ_EStrategy chosen = strategy;
		if (chosen == RZ_EStrategy.NONE)
			chosen = Choose(temper, elements, bodies, day, rising, ctx);

		// At most 2 strategies near one player cluster: a third takes one already running there
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (ent)
		{
			array<int> near = {};
			bh.StrategiesNear(ent.GetOrigin(), CLUSTER_RADIUS, near);
			if (near.Count() >= 2 && near.Find(chosen) < 0)
				chosen = near[0];
		}

		RZ_HuntLink link = bh.NewLink(playerId, chosen, relentless, day);
		link.Apply(ctx);
		foreach (int i, RZ_GroupPlan p : plans)
		{
			if (i >= elements)
				break;

			p.JoinHunt(link, i, elements);
		}

		return link;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_GroupPlan
{
	protected ref array<ref RZ_BodyState> m_aBodies = {};
	RZ_EBodyKind m_eKind = RZ_EBodyKind.HUNTER;
	int m_iYoungFirst;				// the first this many members are young (a day party)
	protected bool m_bDriven;
	protected bool m_bChecked;

	// The owner's order
	protected RZ_EPlanOrder m_eOrder = RZ_EPlanOrder.HOLD;
	protected vector m_vGoal;
	protected bool m_bHasGoal;
	protected float m_fRingMin = 2;
	protected float m_fRingMax = 8;
	protected RZ_EGait m_eGait = RZ_EGait.WALKSTOP;
	protected RZ_EFormation m_eFormation = RZ_EFormation.CLUSTER;
	protected bool m_bArrived;

	// Owner flags
	bool m_bSenses = true;			// notices players by itself (off for scouts: the scout rule)
	bool m_bDeaf;					// takes no leads (the lair ring by day)
	bool m_bDay;
	int m_iTemper;
	bool m_bRising;
	IEntity m_KeepClear;			// weak: the alpha; this group's slots keep 8 m off it
	float m_fHearMult = 1;			// its tribe's shots habit: it hears shots from 1.5 times as far
	float m_fShriekAt = -100;		// its last shriek (RZ_Hearing)
	protected vector m_vVia;		// going round to another door (RZ_InfectedDoors), or the regroup, until m_fViaUntil
	protected float m_fViaUntil;
	protected int m_iPeak;			// the most bodies it had in this hunt (losses: down by half)
	protected bool m_bRegrouped;	// the regroup, once a target
	protected int m_iRegroupTarget = -1;
	protected RZ_EGait m_eLeadGait = RZ_EGait.WALKSTOP;
	protected float m_fLookFreezeAt = -100;
	protected bool m_bAsleep;		// the resting ring by day, its AI off

	// The hunt
	protected RZ_HuntLink m_Link;	// weak: owned by RZ_Behaviour
	protected int m_iElement;
	protected int m_iElements = 1;
	protected RZ_EHuntState m_eHunt;
	protected float m_fHuntAt;
	protected bool m_bLeg2;
	protected float m_fFreezeUntil;
	protected bool m_bRoared;
	protected float m_fSearchUntil;
	protected float m_fSaidAt;
	protected bool m_bAmbushSet;
	protected float m_fEta = -1;	// seconds to its place on the approach (the slower element starts first)
	protected float m_fPick = 0.5;	// this hunt's roll for distances picked in a range (fixed, no jitter)

	// Senses
	protected ref array<int> m_aMeterIds = {};
	protected ref array<float> m_aMeters = {};
	protected ref array<int> m_aGaveUpIds = {};
	protected ref array<float> m_aGaveUpUntil = {};
	protected int m_iNoticed = -1;
	protected float m_fToldAt = -100;
	protected vector m_vLead;
	protected float m_fLeadUntil;

	// Movement
	protected vector m_vCentre;
	protected vector m_vAnchor;
	protected vector m_vHeading = "0 0 1";
	protected bool m_bWaypointOn;
	protected vector m_vWaypointAt;
	protected float m_fWaypointAt = -100;
	protected float m_fNextFull;
	protected float m_fFallbackAt = -100;
	protected vector m_vFallbackTo;
	protected RZ_TrackedZombie m_Unit;	// weak: the tracked unit this plan belongs to

	static const float GIVE_UP_BLOCK = 600;		// s a group won't take the same player again after giving up
	static const float SEARCH_SECONDS = 45;
	static const float ETA_SLACK = 5;			// s an element may arrive before the slowest
	static const float STARE_RADIUS = 40;		// m: a young stares at a house door this close

	// ================================================================ For the owner
	//------------------------------------------------------------------------------------------------
	bool IsDriven() { return m_bDriven; }
	array<ref RZ_BodyState> GetBodies() { return m_aBodies; }
	RZ_HuntLink GetLink() { return m_Link; }
	RZ_EHuntState GetHuntState() { return m_eHunt; }
	vector Centre() { return m_vCentre; }
	bool Arrived() { return m_bArrived; }
	bool IsHunting() { return m_Link != null && !m_Link.m_bOver; }
	RZ_EPlanOrder GetOrder() { return m_eOrder; }
	int GetElement() { return m_iElement; }
	float GetEta() { return m_fEta; }

	//------------------------------------------------------------------------------------------------
	int Living()
	{
		int n = 0;
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A player this group became aware of by itself (its notice meter filled), once; -1 none.
	int TakeNoticed()
	{
		int id = m_iNoticed;
		m_iNoticed = -1;
		return id;
	}

	//------------------------------------------------------------------------------------------------
	//! Stand in a ring round pos, rmin to rmax out (the lair, a post, a kill site, the vigil).
	void OrderHold(vector pos, float rmin, float rmax)
	{
		if (m_eOrder == RZ_EPlanOrder.HOLD && m_bHasGoal && RZ_Util.DistXZ(m_vGoal, pos) < 2)
			return;

		m_eOrder = RZ_EPlanOrder.HOLD;
		m_vGoal = pos;
		m_bHasGoal = true;
		m_fRingMin = rmin;
		m_fRingMax = Math.Max(rmin, rmax);
		m_eGait = RZ_EGait.WALKSTOP;
		m_eFormation = RZ_EFormation.RING;
		m_bArrived = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Walk to goal in a formation (a trail at night in file, a day walk in a loose line).
	void OrderWalk(vector goal, RZ_EGait gait, RZ_EFormation formation)
	{
		if (m_eOrder == RZ_EPlanOrder.WALK && m_bHasGoal && RZ_Util.DistXZ(m_vGoal, goal) < 2 && m_eGait == gait)
			return;

		m_eOrder = RZ_EPlanOrder.WALK;
		m_vGoal = goal;
		m_bHasGoal = true;
		m_eGait = gait;
		m_eFormation = formation;
		m_bArrived = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Go home (the long call, the leash, the commitment, the hide rule): out of its hunt, the
	//! player it had blocked for 10 minutes, engaged bodies taken back unless a player is close.
	void OrderHome(vector goal)
	{
		LeaveHunt(true);
		m_eOrder = RZ_EPlanOrder.HOME;
		m_vGoal = goal;
		m_bHasGoal = true;
		m_eGait = RZ_EGait.MARCH;
		m_eFormation = RZ_EFormation.FILE;
		m_bArrived = false;
		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (!bh || !m_bDriven)
			return;

		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body && b.m_eMode == RZ_EBodyMode.ENGAGE && !PlayerNear(b.m_Body.GetOrigin(), 80))
				bh.TakeBack(b, goal);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Join a hunt as one of its elements.
	void JoinHunt(RZ_HuntLink link, int element, int elements)
	{
		if (!link)
			return;

		m_Link = link;
		link.m_aPlans.Insert(this);
		m_iElement = element;
		m_iElements = Math.Max(1, elements);
		m_eHunt = RZ_EHuntState.APPROACH;
		m_fHuntAt = Now();
		m_bLeg2 = false;
		m_bRoared = false;
		m_fEta = -1;
		m_fPick = Math.RandomFloat01();
		m_bAmbushSet = false;
		m_iPeak = Living();
		if (link.m_iTarget != m_iRegroupTarget)
			m_bRegrouped = false;
		if (link.m_eStrategy == RZ_EStrategy.RUSH)
			m_eHunt = RZ_EHuntState.STRIKE;
	}

	//------------------------------------------------------------------------------------------------
	//! Hand-off (docs/INFECTED.md, Hunt strategies): another group of its tribe lost the target; this
	//! one is nearer its last-known spot, so it walks to look there, primed to notice them. The same
	//! target only: a hand-off never switches players.
	void TakeHandOff(RZ_Behaviour bh, int playerId, vector lastKnown, vector heading)
	{
		if (m_Link)
			return;

		m_vLead = RZ_Util.GroundPos(lastKnown + heading * 20);
		m_fLeadUntil = bh.Now() + 90;
		SetMeter(playerId, Math.Max(Meter(playerId), 0.6));
		for (int i = m_aGaveUpIds.Count() - 1; i >= 0; i--)
		{
			if (m_aGaveUpIds[i] == playerId)
			{
				m_aGaveUpIds.Remove(i);
				m_aGaveUpUntil.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This group gives up: the nearest group of its tribe that's closer to the last-known spot, and
	//! not hunting, takes the hand-off.
	protected void HandOff(RZ_Behaviour bh, RZ_HuntLink link)
	{
		RZ_TrackedZombie me = Owner();
		if (!me || me.m_iTribe < 0 || link.m_iTarget <= 0)
			return;

		float myDist = RZ_Util.DistXZ(m_vCentre, link.m_vLastKnown);
		RZ_GroupPlan best;
		float bestDist = myDist;
		array<ref RZ_TrackedZombie> tracked = RZ_GameModeComponent.GetInstance().GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (z == me || z.m_iTribe != me.m_iTribe || !z.m_Plan || !z.m_Plan.IsDriven() || z.m_Plan.IsHunting())
				continue;

			float d = RZ_Util.DistXZ(z.m_Plan.Centre(), link.m_vLastKnown);
			if (d < bestDist && d < 600)
			{
				bestDist = d;
				best = z.m_Plan;
			}
		}

		if (!best)
			return;

		best.TakeHandOff(bh, link.m_iTarget, link.m_vLastKnown, link.m_vMoving);
		bh.CountHandOff();
	}

	//------------------------------------------------------------------------------------------------
	//! Out of its hunt. gaveUp: the target is blocked for this group for 10 minutes.
	void LeaveHunt(bool gaveUp)
	{
		if (!m_Link)
			return;

		if (gaveUp && m_Link.m_iTarget > 0)
		{
			m_aGaveUpIds.Insert(m_Link.m_iTarget);
			m_aGaveUpUntil.Insert(Now() + GIVE_UP_BLOCK);
		}

		m_Link = null;
		m_eHunt = RZ_EHuntState.OVER;
	}

	//------------------------------------------------------------------------------------------------
	//! Every body to Bacon at once (a scout that called, a lair ring walked into).
	void ReleaseAll()
	{
		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (!bh || !m_bDriven)
			return;

		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body)
				bh.Release(b);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected float Now()
	{
		RZ_Behaviour bh = RZ_Behaviour.Get();
		if (bh)
			return bh.Now();

		return RZ_BaconBody.Now();
	}

	//------------------------------------------------------------------------------------------------
	protected static bool PlayerNear(vector pos, float radius)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return RZ_Util.NearestDistance(pos, players) <= radius;
	}

	// ================================================================ The tick
	//------------------------------------------------------------------------------------------------
	void Tick(RZ_Behaviour bh, RZ_TrackedZombie z, array<int> ids, array<IEntity> players)
	{
		m_Unit = z;
		Sync(bh, z);
		if (m_aBodies.IsEmpty())
			return;

		float nearest = RZ_Util.NearestDistance(m_vCentre, players);
		if (!m_bDriven)
		{
			Fallback(bh, z);
			return;
		}

		if (Rest(bh, nearest))
			return;

		if (nearest > 300 && bh.Now() < m_fNextFull)
			return;

		m_fNextFull = bh.Now() + 3;
		if (m_bSenses && nearest <= 250)
			Sense(bh, ids, players);

		if (m_Link && m_Link.m_bOver)
			LeaveHunt(m_Link.m_iTarget > 0);

		if (m_Link)
			Hunt(bh, ids, players);
		else
			Order(bh);

		Bodies(bh, z, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! The living members, learned in join order: the first m_iYoungFirst are young.
	protected void Sync(RZ_Behaviour bh, RZ_TrackedZombie z)
	{
		array<IEntity> living = {};
		RZ_Util.GetUnitMembers(z.m_Entity, living);
		for (int i = m_aBodies.Count() - 1; i >= 0; i--)
		{
			IEntity known = m_aBodies[i].m_Body;
			if (!known || !RZ_Util.IsCharacterAlive(known) || living.Find(known) < 0)
				m_aBodies.Remove(i);
		}

		if (!m_bChecked && !living.IsEmpty())
		{
			m_bChecked = true;
			m_bDriven = RZ_Behaviour.IsOn();
			foreach (IEntity m : living)
			{
				if (!RZ_BaconBody.IsBacon(m))
					m_bDriven = false;
			}
		}

		vector sum = vector.Zero;
		int n = 0;
		foreach (int k, IEntity member : living)
		{
			if (!RZ_Util.IsCharacterAlive(member))
				continue;

			sum = sum + member.GetOrigin();
			n++;
			if (Find(member))
				continue;

			RZ_BodyState b = new RZ_BodyState();
			b.m_Body = member;
			b.m_eKind = m_eKind;
			if (k < m_iYoungFirst)
				b.m_eKind = RZ_EBodyKind.YOUNG;

			b.m_eMode = RZ_EBodyMode.STEER;
			b.m_fModeAt = bh.Now();
			b.m_fSpeedFactor = Math.RandomFloat(0.88, 1);
			b.m_vJitter = RZ_Util.RandomPointInRing(vector.Zero, 1, 3);
			RZ_KindProfile kp = bh.Kind(b.m_eKind);
			if (kp.m_fStanceMax > 0)
				b.m_fStance = Math.Clamp(Math.RandomFloat(kp.m_fStanceMin, kp.m_fStanceMax) + Math.RandomFloat(-0.04, 0.04), 0.5, 1);

			b.m_vWatchFrom = member.GetOrigin();
			b.m_fWatchAt = bh.Now();
			b.m_fHealth = RZ_Util.GetCharacterHealth(member);
			b.m_fBurstAt = bh.Now() + Math.RandomFloat(3, 8);
			m_aBodies.Insert(b);
			if (m_bDriven && RZ_BaconBody.GetMode(member) == RZ_EBodyMode.OFF)
				RZ_Behaviour.Blind(member, b.m_eKind);
		}

		if (n > 0)
		{
			float fn = n;
			m_vCentre = sum * (1 / fn);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_BodyState Find(IEntity body)
	{
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body == body)
				return b;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Bodies the layer can't drive (another zombie mod, or the layer off): held waypoints only.
	protected void Fallback(RZ_Behaviour bh, RZ_TrackedZombie z)
	{
		ResourceName wp = RZ_GameModeComponent.GetInstance().GetSpawner().GetMoveWaypointPrefab();
		vector to = m_vGoal;
		float gap = 5;
		if (m_Link)
		{
			to = m_Link.m_vLastKnown;
			if (m_Link.m_TargetEnt)
				to = m_Link.m_TargetEnt.GetOrigin();

			gap = 20;
		}
		else if (!m_bHasGoal)
		{
			return;
		}

		if (bh.Now() - m_fFallbackAt < gap || (RZ_Util.DistXZ(to, m_vFallbackTo) < 15 && bh.Now() - m_fFallbackAt < 60))
			return;

		if (RZ_Util.NearestDistance(m_vCentre, PlayersNow()) < 80 && !m_Link)
			return;

		m_fFallbackAt = bh.Now();
		m_vFallbackTo = to;
		RZ_GroupOrders.Order(z.m_Entity, to, wp, false);
	}

	//------------------------------------------------------------------------------------------------
	protected static array<IEntity> PlayersNow()
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		return players;
	}

	// ================================================================ Senses
	//------------------------------------------------------------------------------------------------
	//! The notice meter (docs/INFECTED.md, Senses), once a second for players within sight: it fills
	//! with stealth's visibility, the leader's facing, nearness and line of sight, and falls 0.1 a
	//! second otherwise. 0.4: suspicious (it stops, one sound). 1: aware. A hunt's target is
	//! remembered where it was last seen, never where it really is.
	protected void Sense(RZ_Behaviour bh, array<int> ids, array<IEntity> players)
	{
		RZ_BodyState leader = Leader();
		if (!leader)
			return;

		float sight = bh.SightOf(leader.m_eKind);
		float rate = bh.GetSettings().m_fNoticeRate;
		foreach (int i, IEntity p : players)
		{
			int id = ids[i];
			float d = RZ_Util.DistXZ(m_vCentre, p.GetOrigin());
			bool isTarget = m_Link && m_Link.m_iTarget == id;
			if (d > sight * 1.3 && !isTarget)
			{
				Decay(id);
				continue;
			}

			bool los = false;
			if (d <= sight * 1.5)
				los = bh.LineOfSight(leader.m_Body, p);

			if (isTarget)
			{
				// Relentless (a night hunt on its trail): lying still doesn't hide you; in sight is enough
				bool seen = los && (m_Link.m_bRelentless || RZ_Stealth.GetVisibility(id) >= 0.3 || d < 40);
				if (seen)
				{
					vector moved = p.GetOrigin() - m_Link.m_vLastKnown;
					moved[1] = 0;
					if (moved.Length() > 2)
					{
						moved.Normalize();
						m_Link.m_vMoving = moved;
					}

					m_Link.m_vLastKnown = p.GetOrigin();
					m_Link.m_fLastSeenAt = bh.Now();
				}

				continue;
			}

			if (Blocked(id, d, bh.Now()))
				continue;

			float add = 0;
			if (los)
			{
				float facing = 0.25;
				if (RZ_Behaviour.AngleOff(leader.m_Body, p.GetOrigin()) <= 70)
					facing = 1;

				float nearness = Math.Max(0, 1 - d / sight);
				add = rate * RZ_Stealth.GetVisibility(id) * facing * Math.Pow(nearness, 1.5);
				if (d <= 12)
					add = Math.Max(add, 0.5);
			}

			float meter = Meter(id);
			if (add > 0)
				meter += add;
			else
				meter -= 0.1;

			meter = Math.Clamp(meter, 0, 1);
			SetMeter(id, meter);
			if (meter >= 0.4 && bh.Now() - m_fToldAt > 20)
			{
				// Suspicious: it stops and makes one sound (heard before seen)
				m_fToldAt = bh.Now();
				m_fFreezeUntil = bh.Now() + Math.RandomFloat(2, 4);
				RZ_BaconBody.Say(leader.m_Body, "SOUND_IDLE");
			}

			if (meter >= 1 && !m_Link)
			{
				m_iNoticed = id;
				RZ_TrackedZombie me = Owner();
				if (me && (leader.m_eKind == RZ_EBodyKind.YOUNG || m_iYoungFirst > 0))
					bh.GetHearing().MaybeShriek(this, leader.m_Body, me.m_iTribe);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Gave up on this player in the last 10 minutes: only a fresh sighting within 50 m counts.
	protected bool Blocked(int id, float d, float now)
	{
		for (int i = m_aGaveUpIds.Count() - 1; i >= 0; i--)
		{
			if (now >= m_aGaveUpUntil[i])
			{
				m_aGaveUpIds.Remove(i);
				m_aGaveUpUntil.Remove(i);
				continue;
			}

			if (m_aGaveUpIds[i] == id && d > 50)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected float Meter(int id)
	{
		int at = m_aMeterIds.Find(id);
		if (at < 0)
			return 0;

		return m_aMeters[at];
	}

	//------------------------------------------------------------------------------------------------
	protected void SetMeter(int id, float value)
	{
		int at = m_aMeterIds.Find(id);
		if (at >= 0)
		{
			m_aMeters[at] = value;
			return;
		}

		if (value <= 0)
			return;

		m_aMeterIds.Insert(id);
		m_aMeters.Insert(value);
	}

	//------------------------------------------------------------------------------------------------
	protected void Decay(int id)
	{
		int at = m_aMeterIds.Find(id);
		if (at < 0)
			return;

		m_aMeters[at] = m_aMeters[at] - 0.1;
		if (m_aMeters[at] <= 0)
		{
			m_aMeterIds.Remove(at);
			m_aMeters.Remove(at);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A shot heard (RZ_Behaviour.OnShot): a body within 30 m of an unsuppressed shot is released; a
	//! group in reach gets a lead (the spot, 15% of the distance off) and walks to look, unless it's
	//! deaf (the lair ring by day) or hunting.
	void OnShot(RZ_Behaviour bh, int playerId, vector at, float d, float radius, bool suppressed)
	{
		if (!m_bDriven)
			return;

		if (!suppressed)
		{
			foreach (RZ_BodyState b : m_aBodies)
			{
				if (b.m_Body && b.m_eMode != RZ_EBodyMode.ENGAGE && RZ_Util.DistXZ(b.m_Body.GetOrigin(), at) <= RZ_Behaviour.SHOT_CONTACT)
					bh.Release(b);
			}
		}

		RZ_BodyState leader = Leader();
		if (m_bDeaf || m_Link || !leader)
			return;

		if (d > radius * bh.Kind(leader.m_eKind).m_fHearing * m_fHearMult)
			return;

		// The reactions table (docs/INFECTED.md, Reactions): the young rush it; hunters, scouts and
		// guards freeze 3 s, then come round to the shooter's side; an alpha comes roaring if bold,
		// holds if cautious, else like the hunters
		vector lead = RZ_Util.RandomPointInRing(at, 0, d * 0.15 + 2);
		RZ_EGait gait = RZ_EGait.MARCH;
		bool flank = true;
		if (leader.m_eKind == RZ_EBodyKind.YOUNG)
		{
			gait = RZ_EGait.RUN;
			flank = false;
		}
		else if (leader.m_eKind == RZ_EBodyKind.ALPHA && m_iTemper == RZ_ETemperament.CAUTIOUS)
		{
			return;
		}
		else if (leader.m_eKind == RZ_EBodyKind.ALPHA && m_iTemper == RZ_ETemperament.BOLD)
		{
			gait = RZ_EGait.RUN;
			flank = false;
			RZ_BaconBody.Say(leader.m_Body, "SOUND_TARGET");
		}

		if (flank)
		{
			m_fFreezeUntil = bh.Now() + 3;
			vector toShot = at - m_vCentre;
			toShot[1] = 0;
			vector side = Vector(-toShot[2], 0, toShot[0]);
			if (Math.RandomFloat01() < 0.5)
				side = side * -1;

			lead = lead + side * 0.35;
		}

		m_vLead = RZ_Util.GroundPos(lead);
		m_fLeadUntil = bh.Now() + 90;
		m_eLeadGait = gait;
	}

	//------------------------------------------------------------------------------------------------
	//! A lead from a sound, a scent or a shriek (RZ_Hearing): a spot err off, walked to look; never a
	//! fix. Not while hunting or deaf (the lair ring by day); a fresh lead (under 30 s old) is replaced
	//! only by a nearer one.
	bool TakeLead(RZ_Behaviour bh, vector at, float err)
	{
		if (!m_bDriven || m_bDeaf || m_Link || !Leader())
			return false;

		float now = bh.Now();
		if (now < m_fLeadUntil && now > m_fLeadUntil - 60 && RZ_Util.DistXZ(m_vCentre, at) >= RZ_Util.DistXZ(m_vCentre, m_vLead))
			return false;

		m_vLead = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(at, 0, err));
		m_fLeadUntil = now + 90;
		m_eLeadGait = RZ_EGait.WALKSTOP;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Another way in (RZ_InfectedDoors): for 25 s the group goes round to `via`, another open outside
	//! door of the building its target is in; bodies banging at the locked one are taken back to it.
	void GoRound(RZ_Behaviour bh, vector via)
	{
		m_vVia = via;
		m_fViaUntil = bh.Now() + 25;
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body && b.m_eMode == RZ_EBodyMode.ENGAGE && RZ_Util.DistXZ(b.m_Body.GetOrigin(), via) > 4)
				bh.TakeBack(b, via);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A hunting group of young and a loud noise within 400 m ahead of it: it leaves the hunt and runs
	//! to the noise (RZ_Hearing.Reckless). True if it went.
	bool ChaseNoise(RZ_Behaviour bh, vector at)
	{
		RZ_BodyState leader = Leader();
		if (!m_Link || !leader || leader.m_eKind != RZ_EBodyKind.YOUNG || AnyEngaged())
			return false;

		vector to = at - m_vCentre;
		to[1] = 0;
		float d = to.Length();
		if (d > 400 || d < 20)
			return false;

		to.Normalize();
		if (vector.Dot(to, m_vHeading) < 0.3)
			return false;

		LeaveHunt(false);
		m_vLead = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(at, 0, d * 0.15 + 2));
		m_fLeadUntil = bh.Now() + 90;
		m_eLeadGait = RZ_EGait.RUN;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! How far this group hears a sound of `radius`: its leader's kind and its pack's shots habit.
	float HearingRange(RZ_Behaviour bh, float radius)
	{
		RZ_BodyState leader = Leader();
		if (!leader)
			return 0;

		return radius * bh.Kind(leader.m_eKind).m_fHearing * m_fHearMult;
	}

	//------------------------------------------------------------------------------------------------
	//! How far this group smells blood (its leader's kind).
	float BloodRange(RZ_Behaviour bh)
	{
		RZ_BodyState leader = Leader();
		if (!leader)
			return 0;

		return bh.Kind(leader.m_eKind).m_fBlood;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_BodyState Leader()
	{
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body)
				return b;
		}

		return null;
	}

	// ================================================================ Orders
	//------------------------------------------------------------------------------------------------
	//! The owner's order as an anchor, a heading, a formation and a gait.
	protected void Order(RZ_Behaviour bh)
	{
		if (!m_bHasGoal)
		{
			m_vAnchor = m_vCentre;
			return;
		}

		vector goal = m_vGoal;
		RZ_EGait gait = m_eGait;
		// A lead from a shot: walk to look, then back to the order
		if (!m_bDeaf && bh.Now() < m_fLeadUntil)
		{
			goal = m_vLead;
			gait = m_eLeadGait;
			if (RZ_Util.DistXZ(m_vCentre, m_vLead) < 10)
				m_fLeadUntil = Math.Min(m_fLeadUntil, bh.Now() + 20);
		}

		vector dir = goal - m_vCentre;
		dir[1] = 0;
		float d = dir.Length();
		if (d > 0.5)
		{
			dir.Normalize();
			m_vHeading = dir;
		}

		if (m_eOrder == RZ_EPlanOrder.HOLD && goal == m_vGoal)
		{
			m_vAnchor = m_vGoal;
			m_bArrived = d < m_fRingMax + 10;
			SetWaypoint(gait, goal);
			return;
		}

		if (d < 8)
		{
			m_bArrived = true;
			m_vAnchor = goal;
		}
		else
		{
			float ahead = 18;
			if (gait != RZ_EGait.WALKSTOP)
				ahead = 25;

			m_vAnchor = m_vCentre + dir * Math.Min(d, ahead);
		}

		SetWaypoint(gait, goal);
	}

	//------------------------------------------------------------------------------------------------
	//! Walk-stop takes the group's waypoint off its list (Roam walks to slots under 25 m only with
	//! none); a march or a run puts the held waypoint at the goal, at most every 5 s.
	protected void SetWaypoint(RZ_EGait gait, vector goal)
	{
		RZ_TrackedZombie z = Owner();
		if (!z)
			return;

		if (gait == RZ_EGait.WALKSTOP)
		{
			if (m_bWaypointOn)
			{
				RZ_GroupOrders.Clear(z.m_Entity);
				m_bWaypointOn = false;
			}

			return;
		}

		float now = Now();
		RZ_Behaviour bh = RZ_Behaviour.Get();
		float gap = 5;
		if (bh)
			gap = Math.Max(1, bh.GetSettings().m_fOrderGap);

		if (m_bWaypointOn && (RZ_Util.DistXZ(goal, m_vWaypointAt) < 15 || now - m_fWaypointAt < gap))
			return;

		if (bh && !bh.TakeOrder())
			return;

		ResourceName wp = RZ_GameModeComponent.GetInstance().GetSpawner().GetMoveWaypointPrefab();
		if (RZ_GroupOrders.Order(z.m_Entity, goal, wp, false))
		{
			m_bWaypointOn = true;
			m_vWaypointAt = goal;
			m_fWaypointAt = now;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The tracked unit this plan belongs to.
	protected RZ_TrackedZombie Owner()
	{
		if (m_Unit && m_Unit.m_Plan == this)
			return m_Unit;

		return null;
	}

	// ================================================================ Hunts
	//------------------------------------------------------------------------------------------------
	//! This group's part in its hunt: approach by the strategy, set, strike at a moment, search the
	//! last-known spot when the target is lost, then give up.
	protected void Hunt(RZ_Behaviour bh, array<int> ids, array<IEntity> players)
	{
		RZ_HuntLink link = m_Link;
		IEntity target = link.m_TargetEnt;
		if (!target || !RZ_Util.IsCharacterAlive(target))
		{
			link.m_bOver = true;
			LeaveHunt(false);
			Order(bh);
			return;
		}

		float now = bh.Now();
		if (React(bh, link, target))
			return;

		// Going round to another door, or the regroup: there first, at a run
		if (now < m_fViaUntil)
		{
			if (RZ_Util.DistXZ(m_vCentre, m_vVia) > 4)
			{
				m_vAnchor = m_vVia;
				m_eFormation = RZ_EFormation.CLUSTER;
				SetWaypoint(RZ_EGait.RUN, m_vVia);
				return;
			}

			m_fViaUntil = 0;
		}

		RZ_BodyState leader = Leader();
		float memory = 30;
		if (leader)
			memory = bh.Kind(leader.m_eKind).m_fMemory;

		if (link.m_bRelentless)
			memory = Math.Max(memory, 120);

		vector t = link.m_vLastKnown;
		float d = RZ_Util.DistXZ(m_vCentre, t);
		vector toT = t - m_vCentre;
		toT[1] = 0;
		if (toT.Length() > 0.5)
		{
			toT.Normalize();
			m_vHeading = toT;
		}

		// Lost: search the last-known spot, then give up (the strike is Bacon's until then)
		if (m_eHunt != RZ_EHuntState.SEARCH && now - link.m_fLastSeenAt > memory && d < 25 && !AnyEngaged())
		{
			m_eHunt = RZ_EHuntState.SEARCH;
			m_fSearchUntil = now + SEARCH_SECONDS;
		}

		if (m_eHunt == RZ_EHuntState.SEARCH)
		{
			if (now - link.m_fLastSeenAt < 2)
			{
				m_eHunt = RZ_EHuntState.STRIKE;
			}
			else if (now >= m_fSearchUntil)
			{
				if (link.CountPlans() <= 1)
					link.m_bOver = true;

				HandOff(bh, link);
				LeaveHunt(true);
				Order(bh);
				return;
			}
			else
			{
				m_vAnchor = RZ_Util.GroundPos(RZ_Util.RandomPointInRing(t, 0, 15));
				m_eFormation = RZ_EFormation.LINE;
				SetWaypoint(RZ_EGait.WALKSTOP, t);
				return;
			}
		}

		if (now - link.m_fLastSeenAt > memory && d >= 25 && m_eHunt != RZ_EHuntState.STRIKE)
		{
			// Lost before it got there: go to the last-known spot at a march
			m_vAnchor = t;
			m_eFormation = RZ_EFormation.LINE;
			SetWaypoint(RZ_EGait.MARCH, t);
			return;
		}

		if (link.m_bStrike && m_eHunt != RZ_EHuntState.STRIKE)
			m_eHunt = RZ_EHuntState.STRIKE;

		switch (link.m_eStrategy)
		{
			case RZ_EStrategy.STALK: Stalk(bh, link, target, d); break;
			case RZ_EStrategy.PINCER: Pincer(bh, link, target, d); break;
			case RZ_EStrategy.ENCIRCLE: Encircle(bh, link, target, d); break;
			case RZ_EStrategy.FLUSH: Flush(bh, link, target, d); break;
			case RZ_EStrategy.BAIT: Bait(bh, link, target, d); break;
			case RZ_EStrategy.HERD: Herd(bh, link, target, d); break;
			case RZ_EStrategy.AMBUSH: Ambush(bh, link, target, d); break;
			default: m_eHunt = RZ_EHuntState.STRIKE; break;
		}

		if (m_eHunt == RZ_EHuntState.STRIKE)
			Strike(bh, link, d);
	}

	//------------------------------------------------------------------------------------------------
	//! The reactions table's hunt rows (docs/INFECTED.md, Reactions). True: this group is out of the
	//! hunt (or regrouping started), nothing more this tick.
	protected bool React(RZ_Behaviour bh, RZ_HuntLink link, IEntity target)
	{
		RZ_BodyState leader = Leader();
		if (!leader)
			return false;

		RZ_EBodyKind kind = leader.m_eKind;
		// A player in a vehicle: crawlers let it go beyond 20 m, the young chase it to 60 m, the rest 30 m
		if (RZ_Util.IsInVehicle(target))
		{
			float limit = 30;
			if (kind == RZ_EBodyKind.YOUNG)
				limit = 60;
			else if (kind == RZ_EBodyKind.CRAWLER)
				limit = 20;

			if (RZ_Util.DistXZ(m_vCentre, target.GetOrigin()) > limit)
			{
				LeaveHunt(true);
				Order(bh);
				return true;
			}
		}

		// A wounded alpha (under half its health) goes home with its guard
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (!b.m_Body || b.m_eKind != RZ_EBodyKind.ALPHA || b.m_bWentHome)
				continue;

			float full = RZ_Util.GetCharacterMaxHealth(b.m_Body);
			if (full > 0 && b.m_fHealth >= 0 && b.m_fHealth < full * 0.5)
			{
				b.m_bWentHome = true;
				GoHome(bh);
				return true;
			}
		}

		// Losses: down by half
		int living = Living();
		if (living > m_iPeak)
			m_iPeak = living;

		if (m_iPeak < 2 || living * 2 > m_iPeak || kind == RZ_EBodyKind.YOUNG || kind == RZ_EBodyKind.CRAWLER)
			return false;

		if (kind == RZ_EBodyKind.ALPHA && m_iTemper == RZ_ETemperament.BOLD)
			return false;

		if (kind == RZ_EBodyKind.ALPHA && m_iTemper == RZ_ETemperament.CAUTIOUS)
		{
			GoHome(bh);
			return true;
		}

		if (m_bRegrouped)
		{
			// Halved again after the regroup: give up
			if (link.CountPlans() <= 1)
				link.m_bOver = true;

			GoHome(bh);
			return true;
		}

		Regroup(bh, link);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The regroup (once a target): 60-100 m back from the target toward cover, at a run, then on with
	//! the hunt. Engaged bodies are taken back unless a player is close in sight.
	protected void Regroup(RZ_Behaviour bh, RZ_HuntLink link)
	{
		m_bRegrouped = true;
		m_iRegroupTarget = link.m_iTarget;
		m_iPeak = Living();
		vector away = m_vCentre - link.m_vLastKnown;
		away[1] = 0;
		if (away.Length() < 1)
			away = RZ_Behaviour.RandomDir();

		away.Normalize();
		m_vVia = CoverPoint(bh, link.m_vLastKnown, away, Math.RandomFloat(60, 100));
		m_fViaUntil = bh.Now() + 40;
		array<IEntity> players = PlayersNow();
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body && b.m_eMode == RZ_EBodyMode.ENGAGE && !PlayerInSight(b.m_Body.GetOrigin(), players, RZ_Behaviour.NEVER_TAKE_BACK))
				bh.TakeBack(b, m_vVia);
		}

		bh.CountRegroup();
	}

	//------------------------------------------------------------------------------------------------
	//! Out of the hunt and back where it came from.
	protected void GoHome(RZ_Behaviour bh)
	{
		RZ_TrackedZombie z = Owner();
		vector home = m_vCentre;
		if (z)
			home = z.m_vHome;

		OrderHome(home);
	}

	//------------------------------------------------------------------------------------------------
	protected bool AnyEngaged()
	{
		foreach (RZ_BodyState b : m_aBodies)
		{
			if (b.m_Body && b.m_eMode == RZ_EBodyMode.ENGAGE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "A moment" (docs/INFECTED.md, Hunt strategies): the target stops for 10 s, gets down, or
	//! looks away from the group.
	protected bool Moment(RZ_Behaviour bh, RZ_HuntLink link, IEntity target)
	{
		if (bh.StillFor(link.m_iTarget) >= 10)
			return true;

		if (RZ_Senses.GetStance(target) >= 1)
			return true;

		return RZ_Behaviour.AngleOff(target, m_vCentre) > 100;
	}

	//------------------------------------------------------------------------------------------------
	//! Strike: the leader roars (a tell, 1 s before), then the group fans in at a run and each body
	//! is released to Bacon within its kind's reach (Bodies).
	protected void Strike(RZ_Behaviour bh, RZ_HuntLink link, float d)
	{
		RZ_BodyState leader = Leader();
		if (!m_bRoared && leader)
		{
			m_bRoared = true;
			m_fHuntAt = bh.Now();
			RZ_BaconBody.Say(leader.m_Body, "SOUND_TARGET");
		}

		m_eFormation = RZ_EFormation.LINE;
		m_vAnchor = link.m_vLastKnown;
		SetWaypoint(RZ_EGait.RUN, link.m_vLastKnown);
	}

	//------------------------------------------------------------------------------------------------
	//! Stalk (by day 80-150 m, at night 50-100 m): a shadow point behind the target's line of travel,
	//! toward cover; walk-stop, quiet; frozen when the target looks its way within 120 m; strike at a
	//! moment, or when the time runs out.
	protected void Stalk(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		float now = bh.Now();
		float timeout = 360;
		float r = bh.GetSettings().m_fStalkNight;
		if (link.m_bDay)
		{
			timeout = 240;
			r = bh.GetSettings().m_fStalkDay;
		}

		if (m_eHunt == RZ_EHuntState.APPROACH && (now - m_fHuntAt > timeout || (d <= r + 30 && Moment(bh, link, target))))
		{
			m_eHunt = RZ_EHuntState.STRIKE;
			return;
		}

		// Seen looking our way: freeze, low and silent
		if (d < 120 && RZ_Behaviour.AngleOff(target, m_vCentre) < 30)
			m_fFreezeUntil = now + Math.RandomFloat(3, 6);

		vector planned = link.Planned();
		vector b = m_vCentre - planned;
		b[1] = 0;
		if (b.Length() < 1)
			b = RZ_Behaviour.RandomDir();

		b.Normalize();
		if (link.m_vMoving.Length() > 0.5)
		{
			// Turned toward behind the target's line of travel, up to 60 degrees
			vector behind = -link.m_vMoving;
			b = b + behind * 0.6;
			b[1] = 0;
			b.Normalize();
		}

		vector shadow = CoverPoint(bh, planned, b, r);
		m_vAnchor = shadow;
		m_eFormation = RZ_EFormation.FILE;
		SetWaypoint(RZ_EGait.WALKSTOP, shadow);
	}

	//------------------------------------------------------------------------------------------------
	//! Pincer: two groups go out to flank points either side (a sidestep first, so neither crosses
	//! the target's front), held back, and strike together when both are set or 60 s after the
	//! first is. One group alone stalks instead.
	protected void Pincer(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		if (link.CountPlans() < 2)
		{
			Stalk(bh, link, target, d);
			return;
		}

		float now = bh.Now();
		if (m_eHunt == RZ_EHuntState.SET)
		{
			if (link.AllSet() || (link.m_fArrivedAt > 0 && now - link.m_fArrivedAt > 60) || Moment(bh, link, target))
			{
				link.m_bStrike = true;
				m_eHunt = RZ_EHuntState.STRIKE;
			}

			return;
		}

		float rf = bh.GetSettings().m_fFlankNight;
		if (link.m_bDay)
			rf = bh.GetSettings().m_fFlankDay;

		float alpha = bh.GetSettings().m_fFlankAngle * Math.DEG2RAD;
		float s = 1;
		if (m_iElement % 2 == 0)
			s = -1;

		vector planned = link.Planned();
		vector u = planned - m_vCentre;
		u[1] = 0;
		if (u.Length() < 1)
			u = Vector(0, 0, 1);

		u.Normalize();
		vector p = Vector(u[2], 0, -u[0]);
		vector flank = planned - u * (rf * Math.Cos(alpha)) + p * (rf * Math.Sin(alpha) * s);
		flank = CoverPoint(bh, planned, Dir(planned, flank), rf);
		vector leg1 = m_vCentre + p * (s * rf * Math.Sin(alpha));
		vector goal = flank;
		if (!m_bLeg2)
		{
			goal = RZ_Util.GroundPos(leg1);
			if (RZ_Util.DistXZ(m_vCentre, goal) < 20)
				m_bLeg2 = true;
		}

		if (m_bLeg2 && RZ_Util.DistXZ(m_vCentre, flank) < 20)
		{
			m_eHunt = RZ_EHuntState.SET;
			if (link.m_fArrivedAt < 0)
				link.m_fArrivedAt = now;
		}

		RZ_EGait gait = RZ_EGait.RUN;
		if (link.m_bDay)
			gait = RZ_EGait.MARCH;

		// The ETA: the nearer element waits until the further one would arrive with it
		float route = RZ_Util.DistXZ(m_vCentre, flank);
		if (!m_bLeg2)
			route = RZ_Util.DistXZ(m_vCentre, goal) + RZ_Util.DistXZ(goal, flank);

		if (m_eHunt == RZ_EHuntState.APPROACH && WaitForSlower(link, route, gait))
			return;

		m_vAnchor = goal;
		m_eFormation = RZ_EFormation.PAIRS;
		SetWaypoint(gait, goal);
	}

	//------------------------------------------------------------------------------------------------
	//! Encircle (5+ bodies): a ring of 120 m round the target with a 60 degree gap (always a way
	//! out), closing at about 0.7 m/s once set; strike at 35 m, at a moment, or after 3 minutes.
	protected void Encircle(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		float now = bh.Now();
		if (m_iElement == 0)
		{
			if (link.m_fArrivedAt < 0 && (d < link.m_fRing + 30 || now - m_fHuntAt > 60))
				link.m_fArrivedAt = now;

			if (link.m_fArrivedAt > 0)
				link.m_fRing = Math.Max(30, link.m_fRing - 0.7);

			if (link.m_fRing <= 35 || now - m_fHuntAt > 180 || (link.m_fArrivedAt > 0 && Moment(bh, link, target)))
				link.m_bStrike = true;
		}

		if (link.m_fArrivedAt < 0 && WaitForSlower(link, Math.Max(0, d - link.m_fRing), RZ_EGait.WALKSTOP))
			return;

		m_vAnchor = link.Planned();
		m_eFormation = RZ_EFormation.RING;
		SetWaypoint(RZ_EGait.WALKSTOP, link.m_vLastKnown);
	}

	//------------------------------------------------------------------------------------------------
	//! Flush (a target in a wood or a building, 2 groups): the first pushes in noisily from its side,
	//! the rest wait 40-60 m off on the far side; all strike when the target moves 8 m or breaks
	//! cover, or after 90 s.
	protected void Flush(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		if (link.CountPlans() < 2)
		{
			Stalk(bh, link, target, d);
			return;
		}

		float now = bh.Now();
		if (!link.m_bFlushSet)
		{
			link.m_bFlushSet = true;
			link.m_vFlushFrom = link.m_vLastKnown;
		}

		if (RZ_Util.DistXZ(link.m_vLastKnown, link.m_vFlushFrom) > 8 || now - m_fHuntAt > 90)
		{
			link.m_bStrike = true;
			m_eHunt = RZ_EHuntState.STRIKE;
			return;
		}

		RZ_GroupPlan pusher = link.ElementPlan(0);
		if (m_iElement == 0 || !pusher)
		{
			// Noisy, straight in at a walk
			if (now - m_fSaidAt > 6)
			{
				m_fSaidAt = now;
				RZ_BodyState leader = Leader();
				if (leader)
					RZ_BaconBody.Say(leader.m_Body, "SOUND_IDLE");
			}

			vector toward = Dir(m_vCentre, link.m_vLastKnown);
			m_vAnchor = RZ_Util.GroundPos(link.m_vLastKnown - toward * 10);
			m_eFormation = RZ_EFormation.LINE;
			SetWaypoint(RZ_EGait.WALKSTOP, m_vAnchor);
			return;
		}

		vector planned = link.Planned();
		vector farSide = Dir(pusher.Centre(), planned);
		vector wait = RZ_Util.GroundPos(planned + farSide * (40 + 20 * m_fPick));
		if (m_eHunt == RZ_EHuntState.APPROACH && RZ_Util.DistXZ(m_vCentre, wait) < 20)
			m_eHunt = RZ_EHuntState.SET;

		m_vAnchor = wait;
		m_eFormation = RZ_EFormation.LINE;
		SetWaypoint(RZ_EGait.MARCH, wait);
	}

	//------------------------------------------------------------------------------------------------
	//! Bait (cunning, with young): the young go in loud (their own rush); the hunters come up the
	//! target's back trail at a walk, quiet, and strike when it turns to the young, when the young are
	//! in a fight with it, or after 90 s.
	protected void Bait(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		float now = bh.Now();
		bool engaged = bh.EngagedNear(link.m_vLastKnown, 15);
		bool turned = RZ_Behaviour.AngleOff(target, m_vCentre) > 100;
		if (m_eHunt == RZ_EHuntState.APPROACH && (engaged || (turned && d < 60) || now - m_fHuntAt > 90))
		{
			link.m_bStrike = true;
			m_eHunt = RZ_EHuntState.STRIKE;
			return;
		}

		vector behind = link.m_vMoving;
		if (behind.Length() < 0.5)
			behind = RZ_Behaviour.Facing(target);

		vector trail = RZ_Util.GroundPos(link.m_vLastKnown - behind * Math.Max(25, Math.Min(d, 60)));
		m_vAnchor = trail;
		m_eFormation = RZ_EFormation.FILE;
		SetWaypoint(RZ_EGait.WALKSTOP, trail);
	}

	//------------------------------------------------------------------------------------------------
	//! Herd (a crawler town within 500 m, 5+ bodies): a ring whose gap faces the town, closing; the
	//! young press from the far side. Strike at the town's edge, at 35 m, or after 4 minutes; the town
	//! is stirred (+20).
	protected void Herd(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		float now = bh.Now();
		if (m_iElement == 0)
		{
			if (link.m_fArrivedAt < 0 && (d < link.m_fRing + 30 || now - m_fHuntAt > 60))
				link.m_fArrivedAt = now;

			if (link.m_fArrivedAt > 0)
				link.m_fRing = Math.Max(30, link.m_fRing - 0.7);

			bool atEdge = RZ_Util.DistXZ(link.m_vLastKnown, link.m_vHerdTown) <= link.m_fHerdRadius + 20;
			if (atEdge || link.m_fRing <= 35 || now - m_fHuntAt > 240)
			{
				link.m_bStrike = true;
				if (!link.m_bHerdStirred)
				{
					link.m_bHerdStirred = true;
					RZ_Crawlers crawlers = RZ_GameModeComponent.GetInstance().GetSpawner().GetCrawlers();
					if (crawlers && crawlers.IsEnabled())
						crawlers.AddStir(link.m_vHerdTown, 20);
				}
			}
		}

		if (link.m_fArrivedAt < 0 && WaitForSlower(link, Math.Max(0, d - link.m_fRing), RZ_EGait.WALKSTOP))
			return;

		m_vAnchor = link.Planned();
		m_eFormation = RZ_EFormation.RING;
		SetWaypoint(RZ_EGait.WALKSTOP, link.m_vLastKnown);
	}

	//------------------------------------------------------------------------------------------------
	//! Ambush (a crossing or a habit place within 300 m ahead): the first group lies still 20-40 m off
	//! it, behind terrain or trees, quiet; it strikes when the target passes within 30 m, and after 5
	//! minutes it stalks instead. Other groups stalk.
	protected void Ambush(RZ_Behaviour bh, RZ_HuntLink link, IEntity target, float d)
	{
		if (m_iElement != 0)
		{
			Stalk(bh, link, target, d);
			return;
		}

		float now = bh.Now();
		if (now - m_fHuntAt > 300)
		{
			link.m_eStrategy = RZ_EStrategy.STALK;
			return;
		}

		if (m_bAmbushSet && d <= 30)
		{
			link.m_bStrike = true;
			m_eHunt = RZ_EHuntState.STRIKE;
			return;
		}

		vector away = Dir(link.m_vLastKnown, link.m_vAmbush);
		vector lie = CoverPoint(bh, link.m_vAmbush, away, 20 + 20 * m_fPick);
		if (!m_bAmbushSet && RZ_Util.DistXZ(m_vCentre, lie) < 15)
		{
			m_bAmbushSet = true;
			m_eHunt = RZ_EHuntState.SET;
		}

		if (m_bAmbushSet)
			m_fFreezeUntil = now + 2;

		m_vAnchor = lie;
		m_eFormation = RZ_EFormation.CLUSTER;
		RZ_EGait gait = RZ_EGait.MARCH;
		if (m_bAmbushSet)
			gait = RZ_EGait.WALKSTOP;

		SetWaypoint(gait, lie);
	}

	//------------------------------------------------------------------------------------------------
	protected static vector Dir(vector from, vector to)
	{
		vector d = to - from;
		d[1] = 0;
		if (d.Length() < 0.01)
			return Vector(0, 0, 1);

		d.Normalize();
		return d;
	}

	//------------------------------------------------------------------------------------------------
	//! The formation ETA (docs/INFECTED.md, Formations): this element needs eta seconds to its place.
	//! True while another element of the hunt still needs more than 5 s longer: this one holds where
	//! it is, so the slower element starts first and both arrive together.
	protected bool WaitForSlower(RZ_HuntLink link, float dist, RZ_EGait gait)
	{
		float speed = 0.6;
		if (gait == RZ_EGait.MARCH)
			speed = 2;
		else if (gait == RZ_EGait.RUN)
			speed = 3.7;

		m_fEta = dist / speed;
		if (link.SlowestEta(this) - m_fEta <= ETA_SLACK)
			return false;

		m_vAnchor = m_vCentre;
		SetWaypoint(RZ_EGait.WALKSTOP, m_vCentre);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A point r from center, in direction dir or up to 15 degrees either side, preferring one the
	//! terrain hides from center, then a forest edge point of the land grid (inside the trees, the open
	//! side in view), then a wooded square, and on a tie the lower ground: 5 cheap checks and the
	//! edge points of their squares (docs/INFECTED.md, Formations, "cover").
	protected vector CoverPoint(RZ_Behaviour bh, vector center, vector dir, float r)
	{
		RZ_Land land = RZ_GameModeComponent.GetInstance().GetLand();
		bool grid = land && land.IsReady();
		vector best = RZ_Util.GroundPos(center + dir * r);
		int bestScore = -1;
		array<float> offs = {0, -15, 15, -7.5, 7.5};
		array<vector> cands = {};
		foreach (float deg : offs)
		{
			float a = deg * Math.DEG2RAD;
			vector rd = Vector(dir[0] * Math.Cos(a) - dir[2] * Math.Sin(a), 0, dir[0] * Math.Sin(a) + dir[2] * Math.Cos(a));
			cands.Insert(RZ_Util.GroundPos(center + rd * r));
		}

		// A candidate's square's forest edge point, when it keeps about the same distance and side
		if (grid)
		{
			for (int i = 0; i < 5; i++)
			{
				vector edge;
				if (!land.GetEdgePoint(land.CellAt(cands[i]), edge))
					continue;

				float er = RZ_Util.DistXZ(center, edge);
				if (er < r * 0.6 || er > r * 1.4 || vector.Dot(Dir(center, edge), dir) < 0.7)
					continue;

				cands.Insert(RZ_Util.GroundPos(edge));
			}
		}

		foreach (int ci, vector pt : cands)
		{
			if (RZ_Util.IsWater(pt))
				continue;

			int score = 0;
			if (RZ_InfectedBodies.TerrainBlocks(center, pt))
				score += 3;

			if (ci >= 5)
				score += 2;
			else if (grid)
			{
				int idx = land.CellAt(pt);
				if (idx >= 0 && land.GetTreeClass(idx) != RZ_Land.TREES_NONE)
					score += 1;
			}

			// Low ground preferred on a leg: a tie goes to the lower point
			if (score > bestScore || (score == bestScore && pt[1] < best[1] - 1))
			{
				bestScore = score;
				best = pt;
			}
		}

		return best;
	}

	// ================================================================ Bodies
	//------------------------------------------------------------------------------------------------
	//! Each body: its slot in the formation, its gait, and its mode (release, take back, stairs,
	//! the unstick ladder, LOD).
	protected void Bodies(RZ_Behaviour bh, RZ_TrackedZombie z, array<int> ids, array<IEntity> players)
	{
		float now = bh.Now();
		array<vector> slots = {};
		Slots(bh, slots);
		bool freeze = now < m_fFreezeUntil;
		bool striking = m_Link && m_eHunt == RZ_EHuntState.STRIKE && now - m_fHuntAt >= 1;
		bool quiet = m_Link && (m_Link.m_eStrategy == RZ_EStrategy.STALK || m_Link.m_eStrategy == RZ_EStrategy.BAIT || m_Link.m_eStrategy == RZ_EStrategy.AMBUSH);
		bool stalking = m_Link && m_eHunt != RZ_EHuntState.STRIKE && quiet;
		IEntity target;
		if (m_Link)
			target = m_Link.m_TargetEnt;

		Tells(bh, players, now);
		freeze = now < m_fFreezeUntil;
		array<vector> released = {};
		foreach (int i, RZ_BodyState b : m_aBodies)
		{
			if (!b.m_Body)
				continue;

			vector pos = b.m_Body.GetOrigin();
			IEntity closest = NearestPlayer(pos, players);
			float nd = 100000;
			if (closest)
				nd = RZ_Util.DistXZ(pos, closest.GetOrigin());

			bh.UpdateLod(b, nd);
			float hp = RZ_Util.GetCharacterHealth(b.m_Body);
			b.m_bHit = b.m_fHealth > 0 && hp >= 0 && hp < b.m_fHealth - 0.5;
			b.m_fHealth = hp;
			IEntity foe = target;
			if (!foe)
				foe = closest;

			if (b.m_eMode == RZ_EBodyMode.ENGAGE && Wounded(bh, b, pos, foe, now))
				continue;

			if (b.m_eMode == RZ_EBodyMode.ENGAGE && bh.NearFire(pos))
			{
				// Fire: an engaged body near it is taken back and moved 20 m off
				vector off = pos - m_vCentre;
				if (foe)
					off = pos - foe.GetOrigin();

				off[1] = 0;
				if (off.Length() < 1)
					off = RZ_Behaviour.RandomDir();

				off.Normalize();
				bh.TakeBack(b, RZ_Util.GroundPos(pos + off * 20));
				continue;
			}

			switch (b.m_eMode)
			{
				case RZ_EBodyMode.STEER:
				{
					if (ShouldRelease(bh, b, pos, closest, nd, foe, striking, released))
					{
						// The reaction delay: 0.2-1.2 s, unless hit or touched
						if (b.m_bHit || nd <= RZ_Behaviour.CONTACT_BLIND)
							bh.Release(b);
						else
							bh.ReleaseSoon(b);

						released.Insert(pos);
						break;
					}

					vector slot = slots[i];
					vector stare;
					if (StareSlot(b, pos, now, stare))
						slot = stare;

					if (freeze)
						slot = pos;

					if (now < b.m_fBrokeOffUntil)
						slot = b.m_vBrokeOffAt;

					// Fire: no slot within a burning building's reach
					if (!freeze && bh.NearFire(slot))
						slot = pos;

					SteerBody(bh, b, pos, slot, stalking, freeze, players);
					break;
				}
				case RZ_EBodyMode.ENGAGE:
				{
					EngagedBody(bh, b, pos, foe, players);
					break;
				}
				case RZ_EBodyMode.RECOVER:
				{
					RecoverBody(bh, b, pos, closest, nd, foe);
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A wounded hunter (under 40% of its health) breaks off to 50-80 m from its target and holds
	//! there until the next approach (2 minutes). True: taken back this tick.
	protected bool Wounded(RZ_Behaviour bh, RZ_BodyState b, vector pos, IEntity foe, float now)
	{
		if (b.m_eKind != RZ_EBodyKind.HUNTER || !foe || now < b.m_fBrokeOffUntil)
			return false;

		float full = RZ_Util.GetCharacterMaxHealth(b.m_Body);
		if (full <= 0 || b.m_fHealth < 0 || b.m_fHealth >= full * 0.4)
			return false;

		vector away = pos - foe.GetOrigin();
		away[1] = 0;
		if (away.Length() < 1)
			away = RZ_Behaviour.RandomDir();

		away.Normalize();
		b.m_vBrokeOffAt = CoverPoint(bh, foe.GetOrigin(), away, Math.RandomFloat(50, 80));
		b.m_fBrokeOffUntil = now + 120;
		bh.TakeBack(b, b.m_vBrokeOffAt);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The kinds' tells while not hunting (docs/INFECTED.md, The kinds): hunters freeze for 3-5 s when
	//! a player within 120 m looks their way (at most every 20 s); the young jump between slots (every
	//! 6-12 s a new place in the group, 3-7 m off), and one jump in three, with a house's outside door
	//! within 40 m, is a stare at that house instead (StareAt).
	protected void Tells(RZ_Behaviour bh, array<IEntity> players, float now)
	{
		if (m_Link)
			return;

		if (m_eKind == RZ_EBodyKind.HUNTER && now - m_fLookFreezeAt >= 20)
		{
			foreach (IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), m_vCentre) > 120 || RZ_Behaviour.AngleOff(p, m_vCentre) > 15)
					continue;

				if (RZ_InfectedBodies.TerrainBlocks(p.GetOrigin(), m_vCentre))
					continue;

				m_fLookFreezeAt = now;
				m_fFreezeUntil = Math.Max(m_fFreezeUntil, now + Math.RandomFloat(3, 5));
				break;
			}
		}

		foreach (RZ_BodyState b : m_aBodies)
		{
			if (!b.m_Body || b.m_eKind != RZ_EBodyKind.YOUNG || now < b.m_fJumpAt)
				continue;

			b.m_fJumpAt = now + Math.RandomFloat(6, 12);
			b.m_vJitter = RZ_Util.RandomPointInRing(vector.Zero, 3, 7);
			if (CurrentGait() == RZ_EGait.WALKSTOP && Math.RandomFloat01() < 0.34)
				StareAt(b, now);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The young's stare at a house: facing a body isn't set by script, so the stare is walked. The
	//! body goes to a spot 4 m out from an outside door, then to one 1.5 m out, and arrives facing the
	//! door; it stands there 6-10 s. Only learned buildings near players (the door locks' list), so
	//! nothing is queried.
	protected void StareAt(RZ_BodyState b, float now)
	{
		RZ_Interactions hub = RZ_GameModeComponent.GetInstance().GetInteractions();
		if (!hub || !hub.GetDoors())
			return;

		vector at = b.m_Body.GetOrigin();
		IEntity best;
		vector bestOut;
		float bestD = STARE_RADIUS;
		foreach (string key, RZ_BuildingDoors bd : hub.GetDoors().GetBuildings())
		{
			if (!bd || !bd.m_Building || RZ_Util.DistXZ(bd.m_vPos, at) > STARE_RADIUS + 30)
				continue;

			foreach (IEntity door : bd.m_aDoors)
			{
				if (!door)
					continue;

				float dd = RZ_Util.DistXZ(door.GetOrigin(), at);
				if (dd >= bestD)
					continue;

				// Outward: from the building's middle through the door
				vector outward = door.GetOrigin() - bd.m_vPos;
				outward[1] = 0;
				if (outward.Length() < 0.5)
					continue;

				outward.Normalize();
				best = door;
				bestOut = outward;
				bestD = dd;
			}
		}

		if (!best)
			return;

		vector d0 = best.GetOrigin();
		b.m_vStareFrom = RZ_Util.GroundPos(d0 + bestOut * 4);
		b.m_vStare = RZ_Util.GroundPos(d0 + bestOut * 1.5);
		if (RZ_Util.IsWater(b.m_vStareFrom) || RZ_Util.IsWater(b.m_vStare))
			return;

		b.m_bStareClose = false;
		b.m_fStareUntil = now + Math.RandomFloat(14, 20);
		b.m_fJumpAt = b.m_fStareUntil;
	}

	//------------------------------------------------------------------------------------------------
	//! The slot a staring young walks to (the far spot, then the near one), or false when it isn't
	//! staring.
	protected bool StareSlot(RZ_BodyState b, vector pos, float now, out vector slot)
	{
		if (now >= b.m_fStareUntil || m_Link || b.m_eKind != RZ_EBodyKind.YOUNG)
			return false;

		if (!b.m_bStareClose && RZ_Util.DistXZ(pos, b.m_vStareFrom) < 1.5)
			b.m_bStareClose = true;

		slot = b.m_vStareFrom;
		if (b.m_bStareClose)
			slot = b.m_vStare;

		// Wandered off from the group: the stare ends
		if (RZ_Util.DistXZ(slot, m_vCentre) > STARE_RADIUS + 10)
		{
			b.m_fStareUntil = -1;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The resting ring asleep (L14, as the crawlers lie up): a deaf ring (the lair by day) with no
	//! player within 250 m has its AI off; a player within 220 m, a hit, or any order but the ring
	//! wakes it. True: asleep, nothing more to do.
	protected bool Rest(RZ_Behaviour bh, float nearest)
	{
		bool want = m_bDeaf && !m_Link && m_eOrder == RZ_EPlanOrder.HOLD && nearest > 250 && bh.GetSettings().m_bSleepLying;
		if (m_bAsleep && (!m_bDeaf || m_Link || m_eOrder != RZ_EPlanOrder.HOLD || nearest <= 220 || !bh.GetSettings().m_bSleepLying))
		{
			m_bAsleep = false;
			foreach (RZ_BodyState w : m_aBodies)
			{
				RZ_CrawlerLife.SetAI(w.m_Body, true);
			}

			bh.CountRingWoke();
			return false;
		}

		if (!m_bAsleep && want)
		{
			m_bAsleep = true;
			foreach (RZ_BodyState s : m_aBodies)
			{
				if (s.m_Body && s.m_eMode == RZ_EBodyMode.STEER)
					RZ_CrawlerLife.SetAI(s.m_Body, false);
			}

			bh.CountRingSlept();
		}

		return m_bAsleep;
	}

	//------------------------------------------------------------------------------------------------
	bool IsAsleep() { return m_bAsleep; }

	//------------------------------------------------------------------------------------------------
	protected static IEntity NearestPlayer(vector pos, array<IEntity> players)
	{
		IEntity best;
		float bestDist = 1000000;
		foreach (IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = p;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The control rule's releases (docs/INFECTED.md): contact (8 m in sight, 3 m blind), a hit, the
	//! strike order within its reach, or a member of its group released within 15 m this second.
	protected bool ShouldRelease(RZ_Behaviour bh, RZ_BodyState b, vector pos, IEntity closest, float nd, IEntity foe, bool striking, array<vector> released)
	{
		if (b.m_bHit)
			return true;

		if (nd <= RZ_Behaviour.CONTACT_BLIND)
			return true;

		// A wounded hunter holding off comes back only in the next approach
		if (bh.Now() < b.m_fBrokeOffUntil)
			return false;

		if (closest && nd <= bh.GetSettings().m_fContactRadius && !RZ_InfectedBodies.TerrainBlocks(pos, closest.GetOrigin()))
			return true;

		if (striking && foe && RZ_Util.DistXZ(pos, foe.GetOrigin()) <= bh.Kind(b.m_eKind).m_fEngage)
			return true;

		foreach (vector r : released)
		{
			if (RZ_Util.DistXZ(r, pos) <= RZ_Behaviour.SHARE_RELEASE)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A steered body: its slot (or the unstick ladder's), its speed cap for the gait, its stance and
	//! voice, and the stuck watch (moved under 1.5 m in 10 s while its slot is 8 m off, or a climb
	//! refused: the ladder's next step).
	protected void SteerBody(RZ_Behaviour bh, RZ_BodyState b, vector pos, vector slot, bool stalking, bool freeze, array<IEntity> players)
	{
		float now = bh.Now();
		vector want = slot;
		if (b.m_iLadder > 0 && now < b.m_fLadderUntil)
			want = b.m_vLadderSlot;

		RZ_BaconBody.SetSlot(b.m_Body, want);
		b.m_vSlot = want;

		RZ_KindProfile kp = bh.Kind(b.m_eKind);
		RZ_EGait gait = CurrentGait();
		float cap = 1;
		if (gait == RZ_EGait.WALKSTOP)
			cap = kp.m_fWalkCap;
		else if (gait == RZ_EGait.MARCH)
			cap = kp.m_fMarchCap;

		if (cap < 1)
			cap = cap * b.m_fSpeedFactor;

		bh.SetCap(b, cap);
		float stance = b.m_fStance;
		if (freeze)
			stance = 0.6;

		RZ_BaconBody.SetStance(b.m_Body, stance);
		RZ_BaconBody.SetMute(b.m_Body, (stalking || freeze) && kp.m_bMuteStalking);

		// The stuck watch
		bool refused = now - RZ_BaconBody.ClimbRefusedAt(b.m_Body) < 2;
		bool farFromSlot = RZ_Util.DistXZ(pos, want) > 8;
		if (RZ_Util.DistXZ(pos, b.m_vWatchFrom) >= RZ_Behaviour.STUCK_MOVE || (!farFromSlot && !refused))
		{
			if (RZ_Util.DistXZ(pos, b.m_vWatchFrom) >= 5)
				b.m_iLadder = 0;

			b.m_vWatchFrom = pos;
			b.m_fWatchAt = now;
			return;
		}

		if (!refused && now - b.m_fWatchAt < RZ_Behaviour.STUCK_SECONDS)
			return;

		if (b.m_iLadder > 0 && now < b.m_fLadderUntil)
			return;

		NextLadderStep(bh, b, pos, want, players);
		b.m_vWatchFrom = pos;
		b.m_fWatchAt = now;
	}

	//------------------------------------------------------------------------------------------------
	//! The gait the plan is using now.
	protected RZ_EGait CurrentGait()
	{
		if (m_bWaypointOn)
		{
			if (m_Link && m_eHunt == RZ_EHuntState.STRIKE)
				return RZ_EGait.RUN;

			if (m_eGait == RZ_EGait.RUN)
				return RZ_EGait.RUN;

			return RZ_EGait.MARCH;
		}

		return RZ_EGait.WALKSTOP;
	}

	//------------------------------------------------------------------------------------------------
	//! The unstick ladder (docs/INFECTED.md, Navigation): sidestep 4 m, back off 10-20 m, another way
	//! 45 degrees off, out of sight a step ahead, then give up (the slot drops to the group's anchor).
	protected void NextLadderStep(RZ_Behaviour bh, RZ_BodyState b, vector pos, vector want, array<IEntity> players)
	{
		float now = bh.Now();
		bh.MarkStuck(pos);
		bh.CountUnstick();
		b.m_iLadder++;
		vector fwd = Dir(pos, want);
		vector side = Vector(fwd[2], 0, -fwd[0]);
		switch (b.m_iLadder)
		{
			case 1:
			{
				b.m_vLadderSlot = RZ_Util.GroundPos(pos + side * 4);
				b.m_fLadderUntil = now + 6;
				return;
			}
			case 2:
			{
				b.m_vLadderSlot = RZ_Util.GroundPos(pos - fwd * Math.RandomFloat(10, 20));
				b.m_fLadderUntil = now + 10;
				return;
			}
			case 3:
			{
				float a = 45 * Math.DEG2RAD;
				if (Math.RandomFloat01() < 0.5)
					a = -a;

				vector rd = Vector(fwd[0] * Math.Cos(a) - fwd[2] * Math.Sin(a), 0, fwd[0] * Math.Sin(a) + fwd[2] * Math.Cos(a));
				b.m_vLadderSlot = RZ_Util.GroundPos(pos + rd * 25);
				b.m_fLadderUntil = now + 20;
				return;
			}
			case 4:
			{
				vector ahead = RZ_Util.GroundPos(pos + fwd * Math.Min(15, RZ_Util.DistXZ(pos, want)));
				if (bh.GetSettings().m_bUnstickOutOfSight && bh.UnseenSpot(pos, players) && bh.UnseenSpot(ahead, players) && RZ_Behaviour.WaterDepth(ahead) <= 0.05)
				{
					vector mat[4];
					b.m_Body.GetWorldTransform(mat);
					mat[3] = ahead;
					b.m_Body.SetWorldTransform(mat); // VERIFY: moving an AI character this way on the server is picked up by its movement (lab: SetOrigin out of sight, untested in play)
					b.m_Body.Update();
				}

				b.m_fLadderUntil = now + 2;
				return;
			}
		}

		// Give up: back to the group's anchor
		b.m_iLadder = 0;
		b.m_vLadderSlot = m_vAnchor;
		b.m_fLadderUntil = now;
	}

	//------------------------------------------------------------------------------------------------
	//! An engaged body (Bacon's): RUN kept wanted, the young's bursts, the hunter's lunge; taken back
	//! when the fight moves away, when its target is lost or gone, or when the group goes home; and
	//! the stairs rule (a target 2.5 m up within 15 m across, unreached for 20 s: it waits below).
	protected void EngagedBody(RZ_Behaviour bh, RZ_BodyState b, vector pos, IEntity foe, array<IEntity> players)
	{
		float now = bh.Now();
		RZ_KindProfile kp = bh.Kind(b.m_eKind);
		if (!bh.IsSprinting(b.m_Body))
			RZ_Behaviour.WantMove(b.m_Body, EMovementType.RUN);
		if (!foe || !RZ_Util.IsCharacterAlive(foe))
		{
			if (now - b.m_fModeAt >= RZ_Behaviour.MIN_ENGAGED && !PlayerInSight(pos, players, RZ_Behaviour.NEVER_TAKE_BACK))
				bh.TakeBack(b, m_vAnchor);

			return;
		}

		vector fp = foe.GetOrigin();
		float d = RZ_Util.DistXZ(pos, fp);
		bh.TryBurst(b, foe, d);
		int foeId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(foe);
		bh.TryLunge(b, foe, foeId);
		bh.TryLeap(b, foe);
		bh.TryCharge(b, foe, foeId, d);

		// Stairs: they don't climb them reliably (lab B11), so they wait below
		float up = fp[1] - pos[1];
		if (up > RZ_Behaviour.STAIRS_UP && d <= RZ_Behaviour.STAIRS_ACROSS)
		{
			if (b.m_fBelowSince <= 0)
				b.m_fBelowSince = now;

			if (now - b.m_fBelowSince >= RZ_Behaviour.STAIRS_WAIT)
			{
				bh.TakeBack(b, pos);
				b.m_bBelow = true;
				return;
			}
		}
		else
		{
			b.m_fBelowSince = 0;
		}

		if (now - b.m_fModeAt < RZ_Behaviour.MIN_ENGAGED)
			return;

		bool keep = PlayerInSight(pos, players, RZ_Behaviour.NEVER_TAKE_BACK);
		// Stuck in a fight: closed under 1 m in 8 s on a target within 40 m, out of reach
		if (b.m_fClosing < 0 || now - b.m_fClosingAt >= 8)
		{
			bool stuck = b.m_fClosing >= 0 && d > 2.5 && d < 40 && b.m_fClosing - d < 1;
			b.m_fClosing = d;
			b.m_fClosingAt = now;
			if (stuck && !keep)
			{
				vector fwd = Dir(pos, fp);
				bh.TakeBack(b, RZ_Util.GroundPos(pos + Vector(fwd[2], 0, -fwd[0]) * 4));
				bh.CountUnstick();
				return;
			}
		}

		if (keep)
		{
			b.m_fFarSince = 0;
			return;
		}

		bool far = d > kp.m_fEngage * bh.GetSettings().m_fTakeBackMult;
		if (far)
		{
			if (b.m_fFarSince <= 0)
				b.m_fFarSince = now;
		}
		else
		{
			b.m_fFarSince = 0;
		}

		bool lost = m_Link && now - m_Link.m_fLastSeenAt > kp.m_fMemory && d > 20;
		bool home = m_eOrder == RZ_EPlanOrder.HOME && !m_Link;
		if ((b.m_fFarSince > 0 && now - b.m_fFarSince >= 8) || lost || home)
		{
			vector slot = m_vAnchor;
			if (m_Link)
				slot = m_Link.m_vLastKnown;

			bh.TakeBack(b, slot);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A recovering body: steered again once Bacon's own target has cleared; released again on
	//! contact or a hit after 5 s; waiting below stairs until the target comes down.
	protected void RecoverBody(RZ_Behaviour bh, RZ_BodyState b, vector pos, IEntity closest, float nd, IEntity foe)
	{
		float now = bh.Now();
		RZ_BaconBody.SetSlot(b.m_Body, b.m_vSlot);
		bh.SetCap(b, bh.Kind(b.m_eKind).m_fMarchCap);
		if (now - b.m_fModeAt < RZ_Behaviour.MIN_RECOVER)
			return;

		if (b.m_bHit || nd <= RZ_Behaviour.CONTACT_BLIND)
		{
			bh.Release(b);
			return;
		}

		if (b.m_bBelow)
		{
			if (foe && Math.AbsFloat(foe.GetOrigin()[1] - pos[1]) < 1.5)
				bh.Release(b);

			return;
		}

		if (closest && nd <= bh.GetSettings().m_fContactRadius && !RZ_InfectedBodies.TerrainBlocks(pos, closest.GetOrigin()))
		{
			bh.Release(b);
			return;
		}

		if (now - b.m_fModeAt >= RZ_Behaviour.RECOVER_CLEAR)
			bh.Resteer(b);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool PlayerInSight(vector pos, array<IEntity> players, float radius)
	{
		foreach (IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) <= radius && !RZ_InfectedBodies.TerrainBlocks(pos, p.GetOrigin()))
				return true;
		}

		return false;
	}

	// ================================================================ Formations
	//------------------------------------------------------------------------------------------------
	//! One slot per body (docs/INFECTED.md, Formations): file, loose line, wedge, flanking pairs, a
	//! ring, a fan on a strike; spread wider as the group nears its target (w = 4 + 0.1 (200 - d),
	//! 4 to 20 m); 3 m apart at least and 8 m off the alpha (enforced: lab B20); off stuck marks; and
	//! dry, with the line to it dry too (lab B16).
	protected void Slots(RZ_Behaviour bh, array<vector> slots)
	{
		int n = m_aBodies.Count();
		float c = (n - 1) * 0.5;
		vector h = m_vHeading;
		vector p = Vector(h[2], 0, -h[0]);
		float w = 4;
		vector target = m_vAnchor;
		float dt = 0;
		if (m_Link)
		{
			target = m_Link.m_vLastKnown;
			dt = RZ_Util.DistXZ(m_vCentre, target);
			w = Math.Clamp(4 + 0.1 * (200 - dt), 4, 20);
		}

		float phase = m_iElement * 1.3;
		for (int i = 0; i < n; i++)
		{
			RZ_BodyState b = m_aBodies[i];
			float fi = i;
			vector off = vector.Zero;
			switch (m_eFormation)
			{
				case RZ_EFormation.FILE:
				{
					off = p * (b.m_vJitter[0] * 0.3) + h * (-fi * 5);
					break;
				}
				case RZ_EFormation.LINE:
				{
					off = p * ((fi - c) * w) + h * (b.m_vJitter[2] * 0.7);
					break;
				}
				case RZ_EFormation.WEDGE:
				{
					int k = (i + 1) / 2;
					float sgn = 1;
					if (i % 2 == 0)
						sgn = -1;

					off = p * (sgn * k * w) + h * (-k * 4);
					break;
				}
				case RZ_EFormation.PAIRS:
				{
					int pair = i / 2;
					int inPair = i % 2;
					off = p * (inPair * 2.5 + pair * 6) + h * (-inPair * 4);
					break;
				}
				case RZ_EFormation.RING:
				{
					off = RingOffset(i, n, phase, b);
					break;
				}
				default:
				{
					off = b.m_vJitter * 3;
					break;
				}
			}

			// A ring stands round its anchor: the hunt's planned spot, re-planned at 25 m
			vector s = m_vAnchor + off;

			// A strike fans out on an arc round the target, closing
			if (m_Link && m_eHunt == RZ_EHuntState.STRIKE && dt > 30)
			{
				float r = Math.Max(10, dt - 20);
				float theta = (fi - c) * w / Math.Max(20, dt);
				vector back = -h;
				vector rot = Vector(back[0] * Math.Cos(theta) - back[2] * Math.Sin(theta), 0, back[0] * Math.Sin(theta) + back[2] * Math.Cos(theta));
				s = target + rot * r;
			}

			slots.Insert(RZ_Util.GroundPos(s + b.m_vJitter));
		}

		Spread(bh, slots);
		for (int j = 0; j < n; j++)
		{
			vector from = m_vCentre;
			if (m_aBodies[j].m_Body)
				from = m_aBodies[j].m_Body.GetOrigin();

			vector slot = bh.AvoidMarks(slots[j], h);
			if (RZ_Util.DistXZ(slot, m_aBodies[j].m_vFormSlot) > 4 || RZ_Util.DistXZ(from, slot) > 30)
				slot = bh.DrySlot(from, slot);
			else
				slot = m_aBodies[j].m_vFormSlot;

			m_aBodies[j].m_vFormSlot = slot;
			slots[j] = slot;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A ring slot: by day at the lair, the kill site and the vigil evenly round (rmin to rmax out);
	//! in an encircle, this element's share of 300 degrees, leaving a 60 degree gap.
	protected vector RingOffset(int i, int n, float phase, RZ_BodyState b)
	{
		float fi = i;
		float fn = Math.Max(1, n);
		if (m_Link && (m_Link.m_eStrategy == RZ_EStrategy.ENCIRCLE || m_Link.m_eStrategy == RZ_EStrategy.HERD))
		{
			int elements = Math.Max(1, m_iElements);
			float share = 300.0 / elements;
			float start = 30 + m_iElement * share;
			float deg = start + (fi + 0.5) * share / fn;
			float gapA = Math.Atan2(m_Link.m_vGap[0], m_Link.m_vGap[2]);
			float a = gapA + deg * Math.DEG2RAD;
			return Vector(Math.Sin(a), 0, Math.Cos(a)) * m_Link.m_fRing;
		}

		float ang = phase + fi * Math.PI2 / fn;
		float t = (b.m_vJitter[0] + 3) / 6;
		float r = m_fRingMin + (m_fRingMax - m_fRingMin) * Math.Clamp(t, 0, 1);
		return Vector(Math.Sin(ang), 0, Math.Cos(ang)) * r;
	}

	//------------------------------------------------------------------------------------------------
	//! Slots pushed apart to the spacing minimum, and 8 m off the alpha.
	protected void Spread(RZ_Behaviour bh, array<vector> slots)
	{
		float minGap = Math.Max(1, bh.GetSettings().m_fSpacing);
		int n = slots.Count();
		for (int pass = 0; pass < 2; pass++)
		{
			for (int i = 0; i < n; i++)
			{
				for (int j = i + 1; j < n; j++)
				{
					vector d = slots[j] - slots[i];
					d[1] = 0;
					float len = d.Length();
					if (len >= minGap)
						continue;

					if (len < 0.1)
						d = Vector(m_vHeading[2], 0, -m_vHeading[0]);
					else
						d.Normalize();

					slots[j] = RZ_Util.GroundPos(slots[j] + d * (minGap - len));
				}
			}
		}

		if (!m_KeepClear)
			return;

		vector a = m_KeepClear.GetOrigin();
		for (int k = 0; k < n; k++)
		{
			vector off = slots[k] - a;
			off[1] = 0;
			float dist = off.Length();
			if (dist >= 8)
				continue;

			if (dist < 0.1)
				off = Vector(1, 0, 0);
			else
				off.Normalize();

			slots[k] = RZ_Util.GroundPos(a + off * 8);
		}
	}
}
