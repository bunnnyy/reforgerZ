//! Carrying things somewhere (docs/DESIGN.md, "Carrying"): a task whose words say carry, stack or
//! bring in is done by really moving the things. They stand in a pile at the place (crates by the
//! road, chairs in the hall); you pick one up (a short hold, the work sites' prompt and animation),
//! walk it to where the words say (indoors on the ground floor when they say "in"), and put it down
//! there, where it stays, each on the last. While you carry one you're slowed (no running) and it
//! shows in your arms; raising your weapon, getting into a vehicle or going down puts it down where
//! you stand, to be picked up again. The step is done when the last is put down.
//!
//! Built on the work sites (Missions/Blocks/RZ_WorkSites.c): the pick-up is a TAKE site whose look
//! is one of the things, the put-down a site at the stack. Data always; props only near players.
//! Server only.

//------------------------------------------------------------------------------------------------
//! One carrying job: one player, N things, from a pile to a stack.
class RZ_CarryJob
{
	string m_sKey;				// the owner's key ("errand.12"); its sites are key.pick, key.drop, key.loose<n>
	int m_iOnly;				// the player doing it
	int m_iCount;				// how many things
	int m_iDelivered;			// put down at the stack
	bool m_bCarrying;			// one in the player's arms now
	int m_iFrom = -1;			// where the one carried came from: -1 the pile, else a loose one's index
	ResourceName m_sProp;		// one thing, as it stands (a base-game prop)
	ResourceName m_sHeld;		// one thing in the arms (look only: no physics), "" none
	string m_sThing;			// "crate"
	string m_sThings;			// "crates"
	vector m_vPick;				// the pile
	bool m_bPickInside;
	string m_sPickName;			// "by the road", "inside, on the ground floor"
	vector m_vDrop;				// the stack's foot
	bool m_bDropInside;
	string m_sDropName;			// "indoors, on the ground floor", "outside the door"
	float m_fYaw;				// the pile's and the stack's heading
	bool m_bStack = true;		// put down one on another (crates, sacks); false: in a row (chairs)
	float m_fItemHeight;		// measured off the first one put down (0 not yet)
	ref array<vector> m_aStack = {};		// where each one put down stands
	ref array<vector> m_aLoose = {};		// put down elsewhere, to be picked up again
	ref array<int> m_aLooseKey = {};		// each loose one's site number
	int m_iNextLoose;
	vector m_vLast;				// the carrier's last place (put down there if they're gone)
	IEntity m_Held;				// weak: the thing in their arms
	ref array<IEntity> m_aPileProps = {};	// weak: the pile's others (the pick site shows one)
	ref array<IEntity> m_aStackProps = {};	// weak: what's been put down
	bool m_bClosed;

	//------------------------------------------------------------------------------------------------
	bool IsDone()
	{
		return m_iDelivered >= m_iCount;
	}

	//------------------------------------------------------------------------------------------------
	//! Still at the pile (not carried, not put down, not lying loose).
	int InPile()
	{
		int n = m_iCount - m_iDelivered - m_aLoose.Count();
		if (m_bCarrying)
			n--;

		return Math.Max(0, n);
	}

	//------------------------------------------------------------------------------------------------
	string PickKey()
	{
		return m_sKey + ".pick";
	}

	//------------------------------------------------------------------------------------------------
	string DropKey()
	{
		return m_sKey + ".drop";
	}

	//------------------------------------------------------------------------------------------------
	string LooseKey(int i)
	{
		return m_sKey + ".loose" + m_aLooseKey[i].ToString();
	}
}

//------------------------------------------------------------------------------------------------
//! The carrying jobs, ticked by the work sites once a second.
class RZ_Carries
{
	protected static const float PICK_HOLD = 2;		// picking one up (s)
	protected static const float DROP_HOLD = 2;		// putting it down at the stack (s)
	protected static const float ROW_STEP = 0.75;	// the pile's and a row's spacing (m)
	protected static const float DEFAULT_HEIGHT = 0.45;	// a thing's height until one is measured (m)
	protected static const int STACK_HIGH = 3;		// a stack's height in things, then a new one beside it

	protected RZ_WorkSites m_Sites;
	protected ref array<ref RZ_CarryJob> m_aJobs = {};
	protected bool m_bWarnedHeld;

	//------------------------------------------------------------------------------------------------
	void RZ_Carries(RZ_WorkSites sites)
	{
		m_Sites = sites;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_Carries Get()
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return null;

		return sites.Carries();
	}

	//------------------------------------------------------------------------------------------------
	//! Carrying is on (Shared > Work sites > Carry for real) and work sites are up.
	static bool Available()
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		return RZ_WorkSites.Available() && sites.Settings().m_bCarry;
	}

	//------------------------------------------------------------------------------------------------
	//! A job for `only`: `count` of `prop` from the pile at `pick` to the stack at `drop` (each on the
	//! floor it's given when `inside`). `thing`/`things` name one and several in the prompts and the
	//! tracker; `pickName`/`dropName` say where. Returns false when carrying is off (the owner keeps
	//! its old rule) or the pile's site won't open.
	bool Open(string key, int only, int count, ResourceName prop, string thing, string things, vector pick, bool pickInside, string pickName, vector drop, bool dropInside, string dropName, bool stack)
	{
		if (!Available() || key.IsEmpty() || prop.IsEmpty() || count <= 0)
			return false;

		RZ_CarryJob old = Find(key);
		if (old && !old.m_bClosed)
			return true;

		if (old)
			Drop(old);

		RZ_CarryJob j = new RZ_CarryJob();
		j.m_sKey = key;
		j.m_iOnly = only;
		j.m_iCount = count;
		j.m_sProp = prop;
		j.m_sHeld = HeldFor(thing);
		j.m_sThing = thing;
		j.m_sThings = things;
		j.m_vPick = pick;
		j.m_bPickInside = pickInside;
		j.m_sPickName = pickName;
		j.m_vDrop = drop;
		j.m_bDropInside = dropInside;
		j.m_sDropName = dropName;
		j.m_bStack = stack;
		j.m_fYaw = Math.RandomFloat(0, 360);
		m_aJobs.Insert(j);
		if (!OpenPick(j))
		{
			m_aJobs.RemoveItem(j);
			return false;
		}

		RZ_Log.Info(string.Format("Carry: %1, %2 %3 from %4 (%5) to %6 (%7)", key, count, things, RZ_SettlementScanner.GridReference(pick), pickName, RZ_SettlementScanner.GridReference(drop), dropName));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool Has(string key)
	{
		RZ_CarryJob j = Find(key);
		return j && !j.m_bClosed;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDone(string key)
	{
		RZ_CarryJob j = Find(key);
		return j && j.IsDone();
	}

	//------------------------------------------------------------------------------------------------
	//! The job for the tracker (null none).
	RZ_CarryJob Job(string key)
	{
		RZ_CarryJob j = Find(key);
		if (!j || j.m_bClosed)
			return null;

		return j;
	}

	//------------------------------------------------------------------------------------------------
	//! The owner's step is over: its sites go, the arms are emptied. What was put down stays until
	//! nobody is near, then goes with the rest.
	void Close(string key)
	{
		RZ_CarryJob j = Find(key);
		if (!j || j.m_bClosed)
			return;

		Release(j);
		m_Sites.Close(j.PickKey());
		m_Sites.Close(j.DropKey());
		for (int i = 0; i < j.m_aLoose.Count(); i++)
		{
			m_Sites.Close(j.LooseKey(i));
		}

		DeletePile(j);
		j.m_bClosed = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The place to go now: the stack while carrying, else the nearest thing to pick up.
	vector NowAt(RZ_CarryJob j, vector from)
	{
		if (j.m_bCarrying)
			return j.m_vDrop;

		vector best = m_Sites.PosOf(j.PickKey());
		if (best == vector.Zero || j.InPile() <= 0)
			best = j.m_vPick;

		float bestDist = RZ_Util.DistXZ(from, best);
		if (j.InPile() <= 0)
			bestDist = float.MAX;

		foreach (vector loose : j.m_aLoose)
		{
			float d = RZ_Util.DistXZ(from, loose);
			if (d < bestDist)
			{
				bestDist = d;
				best = loose;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! The tracker's two lines: picking one up (where), and putting it down (where), with the count.
	void Lines(RZ_CarryJob j, out string pickLine, out string dropLine)
	{
		string where = j.m_sPickName;
		if (j.InPile() <= 0 && !j.m_aLoose.IsEmpty())
			where = "where you put it down";

		pickLine = string.Format("Pick up a %1 %2 (%3 of %4 carried).", j.m_sThing, where, j.m_iDelivered, j.m_iCount);
		dropLine = string.Format("Put the %1 down %2.", j.m_sThing, j.m_sDropName);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	//! Once a second (RZ_WorkSites.Tick): what the carrier's hands did, and the props near players.
	void Tick(array<int> ids, array<IEntity> players, float near, float far)
	{
		for (int k = m_aJobs.Count() - 1; k >= 0; k--)
		{
			RZ_CarryJob j = m_aJobs[k];
			if (j.m_bClosed)
			{
				if (!AnyNear(players, j.m_vDrop, far))
					Drop(j);

				continue;
			}

			int idx = ids.Find(j.m_iOnly);
			IEntity me;
			if (idx >= 0)
				me = players[idx];

			if (j.m_bCarrying)
				TickCarrying(j, me);
			else
				TickPicking(j, me);

			Props(j, players, near, far);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TickCarrying(RZ_CarryJob j, IEntity me)
	{
		// Gone, down, in a vehicle or a weapon up: it's put down where they stand
		if (!me || !RZ_Util.IsCharacterAlive(me) || RZ_Util.IsInVehicle(me) || WeaponUp(me))
		{
			vector at = j.m_vLast;
			if (me && !RZ_Util.IsInVehicle(me))
				at = me.GetOrigin();

			PutDownHere(j, at);
			return;
		}

		j.m_vLast = me.GetOrigin();
		if (!m_Sites.IsDone(j.DropKey()))
			return;

		// Put down at the stack: it stays there, on the last
		m_Sites.Close(j.DropKey());
		Release(j);
		j.m_aStack.Insert(StackSpot(j, j.m_aStack.Count()));
		j.m_iDelivered++;
		RZ_Log.Info(string.Format("Carry: %1, %2 of %3 %4 put down %5", j.m_sKey, j.m_iDelivered, j.m_iCount, j.m_sThings, j.m_sDropName));
		if (j.IsDone())
		{
			Notify(j, string.Format("That's the last of the %1.", j.m_sThings));
			return;
		}

		Notify(j, string.Format("%1 of %2. Back for the next.", j.m_iDelivered, j.m_iCount));
		OpenPick(j);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickPicking(RZ_CarryJob j, IEntity me)
	{
		if (me)
			j.m_vLast = me.GetOrigin();

		int from = -2;
		if (j.InPile() > 0 && m_Sites.IsDone(j.PickKey()))
			from = -1;

		for (int i = 0; i < j.m_aLoose.Count() && from == -2; i++)
		{
			if (m_Sites.IsDone(j.LooseKey(i)))
				from = i;
		}

		if (from == -2)
			return;

		// Picked up: one in the arms, the stack's spot opens, the other pick-ups wait
		if (from == -1)
		{
			m_Sites.Close(j.PickKey());
		}
		else
		{
			m_Sites.Close(j.LooseKey(from));
			j.m_aLoose.Remove(from);
			j.m_aLooseKey.Remove(from);
		}

		m_Sites.Close(j.PickKey());
		for (int c = 0; c < j.m_aLoose.Count(); c++)
		{
			m_Sites.Close(j.LooseKey(c));
		}

		j.m_bCarrying = true;
		j.m_iFrom = from;
		Hold(j, me);
		string label = string.Format("Put the %1 down", j.m_sThing);
		m_Sites.Open(j.DropKey(), RZ_EWork.HANDS, label, DROP_HOLD, StackSpot(j, j.m_aStack.Count()), j.m_iOnly, "", "", j.m_fYaw, j.m_bDropInside);
		Notify(j, string.Format("You have a %1. Put it down %2. Raise your weapon and you put it down where you stand.", j.m_sThing, j.m_sDropName));
	}

	//------------------------------------------------------------------------------------------------
	//! Put down somewhere else: it lies there to be picked up again (a site of its own).
	protected void PutDownHere(RZ_CarryJob j, vector at)
	{
		m_Sites.Close(j.DropKey());
		Release(j);
		bool inside = at[1] > RZ_Util.GroundPos(at)[1] + 0.3;
		if (!inside)
			at = RZ_Util.GroundPos(at);

		j.m_aLoose.Insert(at);
		j.m_aLooseKey.Insert(j.m_iNextLoose);
		j.m_iNextLoose++;
		RZ_Log.Info(string.Format("Carry: %1, a %2 put down at %3, not at the stack", j.m_sKey, j.m_sThing, RZ_SettlementScanner.GridReference(at)));
		Notify(j, string.Format("You put the %1 down. Pick it up again when you're ready.", j.m_sThing));
		OpenPick(j);
	}

	//------------------------------------------------------------------------------------------------
	//! The pick-up prompts: at the pile (while anything is in it) and at each one lying loose.
	protected bool OpenPick(RZ_CarryJob j)
	{
		string label = string.Format("Pick up a %1", j.m_sThing);
		bool any = false;
		if (j.InPile() > 0)
		{
			vector at = j.m_vPick;
			if (m_Sites.Open(j.PickKey(), RZ_EWork.TAKE, label, PICK_HOLD, at, j.m_iOnly, j.m_sProp, "", j.m_fYaw, j.m_bPickInside) >= 0)
				any = true;
		}

		for (int i = 0; i < j.m_aLoose.Count(); i++)
		{
			vector loose = j.m_aLoose[i];
			bool inside = loose[1] > RZ_Util.GroundPos(loose)[1] + 0.3;
			if (m_Sites.Open(j.LooseKey(i), RZ_EWork.TAKE, label, PICK_HOLD, loose, j.m_iOnly, j.m_sProp, "", j.m_fYaw, inside) >= 0)
				any = true;
		}

		return any;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the n-th one put down stands: stacked STACK_HIGH high, the stacks side by side (a row
	//! for things that don't stack).
	protected vector StackSpot(RZ_CarryJob j, int n)
	{
		float rad = j.m_fYaw * Math.DEG2RAD;
		vector side = Vector(Math.Cos(rad), 0, -Math.Sin(rad));
		int column = n;
		int layer = 0;
		if (j.m_bStack)
		{
			column = n / STACK_HIGH;
			layer = n % STACK_HIGH;
		}

		float h = j.m_fItemHeight;
		if (h <= 0)
			h = DEFAULT_HEIGHT;

		vector at = j.m_vDrop + side * (column * ROW_STEP);
		if (!j.m_bDropInside)
			at = RZ_Util.GroundPos(at);

		at[1] = at[1] + layer * h;
		return at;
	}

	// ================================================================ The carrier
	//------------------------------------------------------------------------------------------------
	//! In the arms: slowed (no running), and the thing shown against the chest.
	protected void Hold(RZ_CarryJob j, IEntity me)
	{
		RZ_PlayerEffectsComponent fx = RZ_PlayerEffectsComponent.GetFor(j.m_iOnly);
		if (fx)
			fx.SendCarry(true, m_Sites.Settings().m_fCarrySpeed, string.Format("Carrying a %1", j.m_sThing));

		if (!me || j.m_sHeld.IsEmpty())
			return;

		float rad = me.GetYawPitchRoll()[0] * Math.DEG2RAD;
		vector ahead = Vector(Math.Sin(rad), 0, Math.Cos(rad));
		vector at = me.GetOrigin() + ahead * 0.38 + Vector(0, 1.0, 0);
		j.m_Held = RZ_Util.SpawnPrefab(j.m_sHeld, at, me.GetYawPitchRoll()[0]);
		if (!j.m_Held)
			return;

		me.AddChild(j.m_Held, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM); // VERIFY: a look-only thing (no physics) added as the character's child follows them on every client, as the perimeter's drone does
		if (j.m_Held.GetParent() == me)
			return;

		if (!m_bWarnedHeld)
		{
			m_bWarnedHeld = true;
			RZ_Log.Info("Carry: the thing in the arms didn't attach to the character; carried things aren't shown in the arms");
		}

		RZ_Util.DeleteUnit(j.m_Held);
		j.m_Held = null;
	}

	//------------------------------------------------------------------------------------------------
	//! The arms emptied: full speed again, nothing shown.
	protected void Release(RZ_CarryJob j)
	{
		if (j.m_Held)
		{
			RZ_Util.DeleteUnit(j.m_Held);
			j.m_Held = null;
		}

		if (j.m_bCarrying)
		{
			RZ_PlayerEffectsComponent fx = RZ_PlayerEffectsComponent.GetFor(j.m_iOnly);
			if (fx)
				fx.SendCarry(false, 1, "");
		}

		j.m_bCarrying = false;
		j.m_iFrom = -1;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool WeaponUp(IEntity me)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(me);
		if (!c)
			return false;

		CharacterControllerComponent ctrl = c.GetCharacterController();
		return ctrl && ctrl.IsWeaponRaised(); // VERIFY: a player's raised weapon is known to the server (RZ_Disguise reads it the same way)
	}

	//------------------------------------------------------------------------------------------------
	protected void Notify(RZ_CarryJob j, string text)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm)
			gm.NotifyPlayer(j.m_iOnly, RZ_Util.Capitalize(j.m_sThings), text);
	}

	// ================================================================ Props
	//------------------------------------------------------------------------------------------------
	//! The pile's others beside the pick-up's own, and what's been put down, up near players only.
	protected void Props(RZ_CarryJob j, array<IEntity> players, float near, float far)
	{
		bool pileNear = AnyNear(players, j.m_vPick, near);
		bool pileFar = !AnyNear(players, j.m_vPick, far);
		int want = Math.Max(0, j.InPile() - 1);
		if (pileFar || j.m_aPileProps.Count() != want)
			DeletePile(j);

		if (pileNear && j.m_aPileProps.IsEmpty() && want > 0)
		{
			vector first = m_Sites.PosOf(j.PickKey());
			if (first == vector.Zero)
				first = j.m_vPick;

			float rad = j.m_fYaw * Math.DEG2RAD;
			vector side = Vector(Math.Cos(rad), 0, -Math.Sin(rad));
			for (int i = 0; i < want; i++)
			{
				vector at = first + side * ((i + 1) * ROW_STEP);
				if (!j.m_bPickInside)
					at = RZ_Util.GroundPos(at);

				IEntity e = RZ_Util.SpawnPrefab(j.m_sProp, at, j.m_fYaw);
				if (e)
					j.m_aPileProps.Insert(e);
			}
		}

		bool stackNear = AnyNear(players, j.m_vDrop, near);
		bool stackFar = !AnyNear(players, j.m_vDrop, far);
		if (stackFar)
		{
			DeleteStack(j);
			return;
		}

		if (!stackNear || j.m_aStackProps.Count() >= j.m_aStack.Count())
			return;

		// Spawned in order, each on the one below; the first measures how tall one is
		for (int n = j.m_aStackProps.Count(); n < j.m_aStack.Count(); n++)
		{
			IEntity put = RZ_Util.SpawnPrefab(j.m_sProp, StackSpot(j, n), j.m_fYaw);
			if (!put)
				break;

			j.m_aStackProps.Insert(put);
			if (j.m_fItemHeight <= 0)
				Measure(j, put);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! One thing's height from its spawned box: the stack's spots above the first are worked out again.
	protected void Measure(RZ_CarryJob j, IEntity e)
	{
		vector bmin;
		vector bmax;
		e.GetWorldBounds(bmin, bmax);
		float h = bmax[1] - bmin[1];
		if (h < 0.1 || h > 1.5)
			return;

		j.m_fItemHeight = h;
		for (int n = 0; n < j.m_aStack.Count(); n++)
		{
			j.m_aStack[n] = StackSpot(j, n);
		}

		// Those already up above the first go to their measured place
		for (int m = 1; m < j.m_aStackProps.Count(); m++)
		{
			IEntity up = j.m_aStackProps[m];
			if (up)
				up.SetOrigin(j.m_aStack[m]);
		}

		if (m_Sites.Has(j.DropKey()))
		{
			m_Sites.Close(j.DropKey());
			string label = string.Format("Put the %1 down", j.m_sThing);
			m_Sites.Open(j.DropKey(), RZ_EWork.HANDS, label, DROP_HOLD, StackSpot(j, j.m_aStack.Count()), j.m_iOnly, "", "", j.m_fYaw, j.m_bDropInside);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeletePile(RZ_CarryJob j)
	{
		foreach (IEntity e : j.m_aPileProps)
		{
			if (e)
				RZ_Util.DeleteUnit(e);
		}

		j.m_aPileProps.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteStack(RZ_CarryJob j)
	{
		foreach (IEntity e : j.m_aStackProps)
		{
			if (e)
				RZ_Util.DeleteUnit(e);
		}

		j.m_aStackProps.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected void Drop(RZ_CarryJob j)
	{
		Release(j);
		DeletePile(j);
		DeleteStack(j);
		m_aJobs.RemoveItem(j);
	}

	// ================================================================ Helpers
	//------------------------------------------------------------------------------------------------
	//! What shows in the arms for a thing (Shared > Work sites > Carried box or Carried chair).
	protected ResourceName HeldFor(string thing)
	{
		RZ_WorkSiteSettings s = m_Sites.Settings();
		if (thing.Contains("chair"))
			return s.m_sCarriedChair;

		return s.m_sCarriedBox;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_CarryJob Find(string key)
	{
		foreach (RZ_CarryJob j : m_aJobs)
		{
			if (j.m_sKey == key)
				return j;
		}

		return null;
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
}
