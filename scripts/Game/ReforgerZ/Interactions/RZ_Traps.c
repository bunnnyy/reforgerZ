//! Things players put down, and fire (docs/DESIGN.md, "World interactions"). Everything here is
//! capped per player, decays on its own and isn't saved, so nothing builds up on the map.
//!
//! - Barricades: drop a bundle of planks in a doorway or window and board it up. It lasts
//!   about a night, infected hitting it make noise, and enough hits break it. The town counts
//!   as Barricaded while one stands (RZ_TownMemory.ReportBarricaded).
//! - Tin-can tripwires (noise only, for anyone but the one who set it) and spike traps (kill
//!   infected that walk onto them, a few times).
//! - Generators: a portable one or one found in a shed. Running: lights where the hook is
//!   used (RZ_PowerGrid.IsPoweredAt), radios nearby pick up every open channel, standing next
//!   to it charges your radio and torch (a "charged" flag), and it's loud. It uses fuel.
//! - Fire: a thrown molotov, or a full fuel can held in your hands at a building's door ("Douse
//!   the doorway and set it alight", a burn spot only that player and others holding fuel see),
//!   burns THAT building (RZ_TownMemory.ReportBuildingBurned): no loot there until it's cooled
//!   and loot has come back, and no infected spawn right around it meanwhile. A town is Burned
//!   only if most of its buildings are. Strongholds, outposts, safehouses, quartermasters and the
//!   lab can't burn.

//------------------------------------------------------------------------------------------------
enum RZ_EPlaceable
{
	BARRICADE,
	TRIPWIRE,
	SPIKES,
	GENERATOR
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Traps, generators and fire.
[BaseContainerProps()]
class RZ_TrapSettings
{
	[Attribute("1", desc: "Barricades, tripwires, spike traps and generators players put down")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Boarded-up doorway or window (a prop with collision) that a planks bundle becomes. Empty = New-player guidance > Barricade, else the planks stay where they are (cosmetic)", "et")]
	ResourceName m_sBarricadePrefab;

	[Attribute("60", desc: "Real minutes a barricade lasts (about a night)")]
	float m_fBarricadeMinutes;

	[Attribute("3", desc: "Barricades per player")]
	int m_iBarricadesPerPlayer;

	[Attribute("8", desc: "Hits (an infected against it on a tick) before a barricade breaks")]
	int m_iBarricadeHits;

	[Attribute("6", desc: "Heat each hit on a barricade makes")]
	float m_fBarricadeHitHeat;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A set tripwire (optional: without it the kit itself lies where it was set)", "et")]
	ResourceName m_sTripwireSetPrefab;

	[Attribute("3", desc: "Tripwires per player")]
	int m_iTripwiresPerPlayer;

	[Attribute("60", desc: "Real minutes a tripwire lasts")]
	float m_fTripwireMinutes;

	[Attribute("20", desc: "Heat a rattling tripwire makes")]
	float m_fTripwireHeat;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "A set spike trap (optional: without it the kit itself lies where it was set)", "et")]
	ResourceName m_sSpikeSetPrefab;

	[Attribute("2", desc: "Spike traps per player")]
	int m_iSpikesPerPlayer;

	[Attribute("60", desc: "Real minutes a spike trap lasts")]
	float m_fSpikeMinutes;

	[Attribute("3", desc: "Infected a spike trap kills before it's spent")]
	int m_iSpikeKills;

	[Attribute("generator,genset", desc: "Generator prefab name keywords (found ones in sheds and farms)")]
	string m_sGeneratorWords;

	[Attribute("6", desc: "Heat a running generator makes per minute")]
	float m_fGeneratorHeatPerMinute;

	[Attribute("30", desc: "Real minutes a generator runs on one fuel item")]
	float m_fGeneratorFuelMinutes;

	[Attribute("40", desc: "Lights (RZ_PowerGrid.IsPoweredAt) within this distance of a running generator")]
	float m_fGeneratorLightRadius;

	[Attribute("30", desc: "Players with a radio this close to a running generator hear every open channel")]
	float m_fGeneratorRadioRadius;

	[Attribute("60", desc: "Seconds next to a running generator (15 m) to charge your radio and torch")]
	float m_fChargeSeconds;

	[Attribute("30", desc: "Real minutes the charge lasts")]
	float m_fChargedMinutes;

	[Attribute("1", desc: "Running generators per player")]
	int m_iGeneratorsPerPlayer;

	[Attribute("jerrycan,jerry_can,fuelcan,fuel_can,canister", desc: "Fuel keywords (generators, setting fires)")]
	string m_sFuelWords;

	// ---------------------------------------------------------------- Fire
	[Attribute("1", desc: "Fire: a thrown molotov, or a full fuel can poured at a doorway, burns that building (one building each)")]
	bool m_bFire;

	[Attribute("1", desc: "Fuel in your hands: Douse the doorway and set it alight shows only while you hold a full fuel can (the can is used up). Off = it shows with a full can anywhere on you")]
	bool m_bFuelInHands;

	[Attribute("5", desc: "Seconds to douse a doorway and set it going (stay close)")]
	float m_fBurnSeconds;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fire effect spawned on a burning building for a few minutes (optional)", "et")]
	ResourceName m_sFireEffectPrefab;

	[Attribute("180", desc: "Seconds the fire effect stays")]
	float m_fFireEffectSeconds;

	[Attribute("30", desc: "Heat a building going up makes")]
	float m_fBurnHeat;

	[Attribute("120", desc: "Real minutes a burned building has no loot, and keeps infected from spawning within 30 m")]
	float m_fBurnMinutes;

	[Attribute("1", desc: "Infection taken off the town when one of its buildings burns")]
	float m_fBurnInfectionDrop;

	[Attribute("25", desc: "Heat a molotov makes where it lands")]
	float m_fMolotovHeat;

	[Attribute("3", desc: "Infected within this distance (m) of a molotov die")]
	float m_fMolotovRadius;

	[Attribute("10", desc: "A molotov burns the building within this distance (m) of where it lands")]
	float m_fMolotovReach;
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Something a player puts down: a planks bundle (barricade), a tin-can tripwire, a spike trap or a portable generator. Add RZ_PlaceAction (and a second one with Refuel for a generator).")]
class RZ_PlaceableComponentClass : ScriptComponentClass
{
}

//! On a kit item. Used from the ground, where it lies.
class RZ_PlaceableComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "What it is", "", ParamEnumArray.FromEnum(RZ_EPlaceable))]
	protected RZ_EPlaceable m_eKind;

	RZ_EPlaceable GetKind() { return m_eKind; }
}

//------------------------------------------------------------------------------------------------
//! "Board up the doorway", "Set the tripwire", "Set the spike trap", "Start or stop the
//! generator"; with Refuel on, "Refuel the generator". Put it on the kit's ActionsManagerComponent.
class RZ_PlaceAction : ScriptedUserAction
{
	[Attribute("0", desc: "Refuel (generators only)")]
	protected bool m_bRefuel;

	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Set it up";
		RZ_PlaceableComponent comp = RZ_PlaceableComponent.Cast(GetOwner().FindComponent(RZ_PlaceableComponent));
		if (!comp)
			return true;

		if (m_bRefuel)
		{
			outName = "Refuel the generator";
			return true;
		}

		switch (comp.GetKind())
		{
			case RZ_EPlaceable.BARRICADE: outName = "Board up the doorway"; break;
			case RZ_EPlaceable.TRIPWIRE: outName = "Set the tin-can tripwire"; break;
			case RZ_EPlaceable.SPIKES: outName = "Set the spike trap"; break;
			case RZ_EPlaceable.GENERATOR: outName = "Start or stop the generator"; break;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!Replication.IsServer())
			return;

		RZ_Interactions hub = RZ_Interactions.Get();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		RZ_PlaceableComponent comp = RZ_PlaceableComponent.Cast(pOwnerEntity.FindComponent(RZ_PlaceableComponent));
		if (!hub || playerId <= 0 || !comp)
			return;

		hub.GetTraps().HandlePlace(playerId, comp.GetKind(), m_bRefuel, pOwnerEntity, pUserEntity);
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "Molotov: a throwable bottle that burns the building it lands by, and the infected right where it lands")]
class RZ_MolotovComponentClass : ScriptComponentClass
{
}

//! Put this on a throwable item (e.g. based on a vanilla grenade without the explosion). It only
//! ignites after a real throw: the carrier's "OnGrenadeThrown" event names this bottle. Dropped
//! from the inventory it's just a bottle.
class RZ_MolotovComponent : ScriptComponent
{
	protected IEntity m_Carrier;	// the character whose throws we listen to (server)
	protected bool m_bThrown;
	protected int m_iThrower = -1;

	protected static ref array<RZ_MolotovComponent> s_aAll;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		if (!s_aAll)
			s_aAll = new array<RZ_MolotovComponent>();

		s_aAll.Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);

		Listen(null);
		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Listen to this character's throws instead of the last carrier's (null: stop).
	protected void Listen(IEntity carrier)
	{
		if (carrier == m_Carrier)
			return;

		if (m_Carrier)
		{
			EventHandlerManagerComponent old = EventHandlerManagerComponent.Cast(m_Carrier.FindComponent(EventHandlerManagerComponent));
			if (old)
				old.RemoveScriptHandler("OnGrenadeThrown", this, OnThrown);
		}

		m_Carrier = carrier;
		m_bThrown = false;
		if (!carrier)
			return;

		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(carrier.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler("OnGrenadeThrown", this, OnThrown); // VERIFY: the character's OnGrenadeThrown fires on the server for a thrown molotov, with the bottle as the entity or the weapon's owner
	}

	//------------------------------------------------------------------------------------------------
	protected void OnThrown(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		if (entity == owner || (weapon && weapon.GetOwner() == owner))
		{
			m_bThrown = true;
			m_iThrower = playerID;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Poll()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		IEntity root = owner.GetRootParent(); // carried items are children of the character (as for noisemakers)
		if (root && root != owner)
		{
			Listen(root);
			return;
		}

		if (!m_bThrown)
			return;

		RZ_Interactions hub = RZ_Interactions.Get();
		if (hub)
			hub.GetTraps().Ignite(owner.GetOrigin(), m_iThrower);

		SCR_EntityHelper.DeleteEntityAndChildren(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_Traps.Tick on the server.
	static void TickAll()
	{
		if (!s_aAll)
			return;

		for (int i = s_aAll.Count() - 1; i >= 0; i--)
		{
			RZ_MolotovComponent m = s_aAll[i];
			if (!m)
			{
				s_aAll.Remove(i);
				continue;
			}

			m.Poll();
		}
	}
}

//------------------------------------------------------------------------------------------------
//! One thing a player put down.
class RZ_Placed
{
	RZ_EPlaceable m_eKind;
	string m_sOwner;		// identity
	int m_iOwner;			// player id
	IEntity m_Ent;
	vector m_vPos;
	float m_fUntil;
	int m_iUses;
	float m_fCooldown;
	bool m_bKit;			// the kit itself is the trap (no set prefab)
	int m_iTown = -1;
}

//------------------------------------------------------------------------------------------------
//! A generator: a portable one on the ground, or one found in a shed (keyed by its spot).
class RZ_Generator
{
	IEntity m_Ent;
	string m_sKey;
	vector m_vPos;
	bool m_bRunning;
	float m_fFuel;			// seconds left
	string m_sOwner;
	int m_iOwner = -1;
}

//------------------------------------------------------------------------------------------------
class RZ_Burn
{
	vector m_vPos;
	float m_fAt;
}

//------------------------------------------------------------------------------------------------
class RZ_Traps
{
	protected static const float HIT_RADIUS = 2.5;
	protected static const float SPIKE_RADIUS = 1.8;
	protected static const float TRIP_COOLDOWN = 30;
	protected static const int TRIP_USES = 6;
	protected static const float CHARGE_RADIUS = 15;
	protected static const float SCORCH_RADIUS = 30;
	protected static const float SAME_BUILDING = 6;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_TrapSettings m_Settings;
	protected ref array<ref RZ_Placed> m_aPlaced = {};
	protected ref array<ref RZ_Generator> m_aGenerators = {};
	protected ref array<ref RZ_Burn> m_aBurns = {};
	protected ref array<IEntity> m_aFires = {};
	protected ref array<float> m_aFireUntil = {};
	protected ref map<int, float> m_mCharge = new map<int, float>();
	protected ref map<string, float> m_mCharged = new map<string, float>();
	protected ref array<string> m_aGeneratorWords = {};
	protected ref map<string, vector> m_mDoorway = new map<string, vector>();	// building key: where its burn spot stands

	// The burn prompt (any machine: the client hides it unless its own hands hold fuel)
	static const string BURN_LABEL = "Douse the doorway and set it alight";
	protected static const float BURN_REACH = 8;		// a building this close (m) to someone holding fuel gets the prompt
	protected static const float DOORWAY_OUT = 1.1;	// the spot stands this far (m) outside the door (a locked door's own spot is at 0.6)
	protected static ref RZ_TrapSettings s_ClientSettings;
	protected static ref array<string> s_aFuelWords;
	protected static string s_sFuelWordsFrom;

	//------------------------------------------------------------------------------------------------
	void RZ_Traps(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_TrapSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		RZ_ItemHelper.Words(m_Settings.m_sGeneratorWords, m_aGeneratorWords);
	}

	//------------------------------------------------------------------------------------------------
	bool IsGeneratorName(string name)
	{
		return m_Settings.m_bEnabled && RZ_ItemHelper.NameHas(name, m_aGeneratorWords) && !name.Contains("power_station") && !name.Contains("powerstation");
	}

	// ================================================================ Save / load (burned buildings only)
	//------------------------------------------------------------------------------------------------
	void SaveInto(RZ_InteractionSave data)
	{
		foreach (RZ_Burn b : m_aBurns)
		{
			data.burnX.Insert(b.m_vPos[0]);
			data.burnZ.Insert(b.m_vPos[2]);
			data.burnAt.Insert(b.m_fAt);
		}
	}

	//------------------------------------------------------------------------------------------------
	void RestoreFrom(RZ_InteractionSave data)
	{
		for (int i = 0; i < data.burnX.Count() && i < data.burnZ.Count() && i < data.burnAt.Count(); i++)
		{
			RZ_Burn b = new RZ_Burn();
			b.m_vPos = Vector(data.burnX[i], 0, data.burnZ[i]);
			b.m_fAt = data.burnAt[i];
			m_aBurns.Insert(b);
		}
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		RZ_MolotovComponent.TickAll();
		TickPlaced(dt);
		TickGenerators(dt, ids, players);
		TickFire();
		TickBurnSpots(players);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickPlaced(float dt)
	{
		float now = m_Hub.GetClock();
		for (int i = m_aPlaced.Count() - 1; i >= 0; i--)
		{
			RZ_Placed p = m_aPlaced[i];
			bool gone = !p.m_Ent;
			if (!gone && p.m_bKit)
			{
				IEntity root = p.m_Ent.GetRootParent(); // as for noisemakers: picked up = child of a character
				gone = root && root != p.m_Ent;
			}

			if (gone || now >= p.m_fUntil)
			{
				Remove(i, !gone);
				continue;
			}

			p.m_fCooldown = Math.Max(0, p.m_fCooldown - dt);
			switch (p.m_eKind)
			{
				case RZ_EPlaceable.BARRICADE: TickBarricade(i, p); break;
				case RZ_EPlaceable.TRIPWIRE: TickTripwire(i, p); break;
				case RZ_EPlaceable.SPIKES: TickSpikes(i, p); break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Infected against the boards make noise, and enough of them break it.
	protected void TickBarricade(int idx, RZ_Placed p)
	{
		if (!m_GM.GetSpawner().IsZombieNear(p.m_vPos, HIT_RADIUS))
			return;

		m_GM.GetHeat().AddHeat(p.m_vPos, m_Settings.m_fBarricadeHitHeat);
		p.m_iUses++;
		if (p.m_iUses < m_Settings.m_iBarricadeHits)
			return;

		m_Hub.NotifyNear(p.m_vPos, 120, -1, "Barricade", "Wood splinters somewhere close. Something just came through a boarded door.");
		Remove(idx, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Tin cans: anyone but the one who set it, or an infected, rattles them. Noise only.
	protected void TickTripwire(int idx, RZ_Placed p)
	{
		if (p.m_fCooldown > 0)
			return;

		bool tripped = m_GM.GetSpawner().IsZombieNear(p.m_vPos, HIT_RADIUS);
		if (!tripped)
		{
			array<int> ids = {};
			array<IEntity> players = {};
			RZ_Util.GetAlivePlayers(ids, players);
			foreach (int i, IEntity pl : players)
			{
				if (ids[i] != p.m_iOwner && RZ_Util.DistXZ(pl.GetOrigin(), p.m_vPos) <= HIT_RADIUS)
					tripped = true;
			}
		}

		if (!tripped)
			return;

		p.m_fCooldown = TRIP_COOLDOWN;
		p.m_iUses++;
		m_GM.GetHeat().AddHeat(p.m_vPos, m_Settings.m_fTripwireHeat);
		IEntity owner = GetGame().GetPlayerManager().GetPlayerControlledEntity(p.m_iOwner);
		if (owner && RZ_Util.DistXZ(owner.GetOrigin(), p.m_vPos) <= 400)
			m_GM.NotifyPlayer(p.m_iOwner, "Tripwire", string.Format("The cans rattle, %1 from you. Something walked into your wire.", RZ_Util.DirectionText(owner.GetOrigin(), p.m_vPos)));

		if (p.m_iUses >= TRIP_USES)
			Remove(idx, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Spikes: an infected that walks onto it dies. Players aren't hurt (it's in plain sight).
	protected void TickSpikes(int idx, RZ_Placed p)
	{
		IEntity z = m_GM.GetSpawner().FindZombieNear(p.m_vPos, SPIKE_RADIUS);
		if (!z)
			return;

		RZ_Util.KillCharacter(z);
		p.m_iUses++;
		if (p.m_iUses >= m_Settings.m_iSpikeKills)
			Remove(idx, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void Remove(int idx, bool deleteEntity)
	{
		RZ_Placed p = m_aPlaced[idx];
		if (deleteEntity && p.m_Ent)
			SCR_EntityHelper.DeleteEntityAndChildren(p.m_Ent);

		RZ_EPlaceable kind = p.m_eKind;
		int town = p.m_iTown;
		vector pos = p.m_vPos;
		m_aPlaced.Remove(idx);
		if (kind == RZ_EPlaceable.BARRICADE && town >= 0 && !HasBarricadeIn(town))
			RZ_TownMemory.ReportBarricaded(pos, false);
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasBarricadeIn(int town)
	{
		foreach (RZ_Placed p : m_aPlaced)
		{
			if (p.m_eKind == RZ_EPlaceable.BARRICADE && p.m_iTown == town)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected int CountOwned(string ident, RZ_EPlaceable kind)
	{
		int n = 0;
		foreach (RZ_Placed p : m_aPlaced)
		{
			if (p.m_sOwner == ident && p.m_eKind == kind)
				n++;
		}

		return n;
	}

	// ================================================================ Putting things down
	//------------------------------------------------------------------------------------------------
	//! From RZ_PlaceAction on a kit lying on the ground.
	void HandlePlace(int playerId, RZ_EPlaceable kind, bool refuel, IEntity kit, IEntity user)
	{
		if (!m_Settings.m_bEnabled || !kit || !user)
			return;

		IEntity root = kit.GetRootParent();
		if (root && root != kit)
		{
			m_GM.NotifyPlayer(playerId, "Not here", "Put it down where you want it first.");
			return;
		}

		if (kind == RZ_EPlaceable.GENERATOR)
		{
			HandlePortableGenerator(playerId, refuel, kit, user);
			return;
		}

		if (FindPlaced(kit))
		{
			m_GM.NotifyPlayer(playerId, "Already set", "It's set. Pick it up to take it with you.");
			return;
		}

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		int cap = m_Settings.m_iTripwiresPerPlayer;
		float minutes = m_Settings.m_fTripwireMinutes;
		ResourceName setPrefab = m_Settings.m_sTripwireSetPrefab;
		string text = "You string the wire ankle-high and hang the cans. Anyone who walks through it will know, and so will everything else.";
		if (kind == RZ_EPlaceable.BARRICADE)
		{
			cap = m_Settings.m_iBarricadesPerPlayer;
			minutes = m_Settings.m_fBarricadeMinutes;
			setPrefab = BarricadePrefab();
			text = "You hammer the planks across the frame. It won't stop a crowd, but it'll hold the odd one, and you'll hear them try. For an hour, more or less.";
			if (!NearBuilding(kit.GetOrigin()))
			{
				m_GM.NotifyPlayer(playerId, "Barricade", "There's no door or window here to board up. Put the planks down in a doorway.");
				return;
			}
		}
		else if (kind == RZ_EPlaceable.SPIKES)
		{
			cap = m_Settings.m_iSpikesPerPlayer;
			minutes = m_Settings.m_fSpikeMinutes;
			setPrefab = m_Settings.m_sSpikeSetPrefab;
			text = "You wedge the board into the ground, nails up, and scuff dirt over it. The dead don't look where they walk.";
		}

		if (CountOwned(ident, kind) >= cap)
		{
			m_GM.NotifyPlayer(playerId, "Too many", string.Format("You've already set %1 of those. Let one go first (they don't last long anyway).", cap));
			return;
		}

		RZ_Placed p = new RZ_Placed();
		p.m_eKind = kind;
		p.m_sOwner = ident;
		p.m_iOwner = playerId;
		p.m_vPos = kit.GetOrigin();
		p.m_fUntil = m_Hub.GetClock() + Math.Max(1, minutes) * 60;
		p.m_iTown = m_GM.FindNearestSettlement(p.m_vPos);
		p.m_Ent = kit;
		p.m_bKit = true;
		if (!setPrefab.IsEmpty())
		{
			vector angles = user.GetAngles(); // rotation around X, Y, Z in degrees: [1] is the yaw
			IEntity deployed = RZ_Util.SpawnPrefab(setPrefab, RZ_Util.GroundPos(kit.GetOrigin()), angles[1]);
			if (deployed)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(kit);
				p.m_Ent = deployed;
				p.m_bKit = false;
			}
		}

		m_aPlaced.Insert(p);
		m_GM.GetHeat().AddHeat(p.m_vPos, 2);
		if (kind == RZ_EPlaceable.BARRICADE)
			RZ_TownMemory.ReportBarricaded(p.m_vPos, true);

		m_GM.NotifyPlayer(playerId, "Set", text);
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName BarricadePrefab()
	{
		if (!m_Settings.m_sBarricadePrefab.IsEmpty())
			return m_Settings.m_sBarricadePrefab;

		RZ_GuidanceSettings guide = m_GM.GetGuidanceSettings();
		if (guide)
			return guide.m_sBarricadePrefab;

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_Placed FindPlaced(IEntity e)
	{
		foreach (RZ_Placed p : m_aPlaced)
		{
			if (p.m_Ent == e)
				return p;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected bool m_bFoundBuilding;

	//------------------------------------------------------------------------------------------------
	//! A building within 12 m (a doorway or window to board up).
	protected bool NearBuilding(vector pos)
	{
		m_bFoundBuilding = false;
		GetGame().GetWorld().QueryEntitiesBySphere(pos, 12, OnBuildingEntity, null, EQueryEntitiesFlags.STATIC);
		return m_bFoundBuilding;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnBuildingEntity(IEntity e)
	{
		if (SCR_DestructibleBuildingEntity.Cast(e))
		{
			m_bFoundBuilding = true;
			return false; // returning false stops the query early
		}

		string name = RZ_ItemHelper.LowerName(e);
		if (name.Contains("/buildings/") || name.Contains("house"))
		{
			m_bFoundBuilding = true;
			return false;
		}

		return true;
	}

	// ================================================================ Generators
	//------------------------------------------------------------------------------------------------
	protected RZ_Generator FindGenerator(IEntity ent, string key)
	{
		foreach (RZ_Generator g : m_aGenerators)
		{
			if (ent && g.m_Ent == ent)
				return g;

			if (!key.IsEmpty() && g.m_sKey == key)
				return g;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void HandlePortableGenerator(int playerId, bool refuel, IEntity kit, IEntity user)
	{
		RZ_Generator g = FindGenerator(kit, "");
		if (!g)
		{
			g = new RZ_Generator();
			g.m_Ent = kit;
			g.m_vPos = kit.GetOrigin();
			g.m_fFuel = m_Settings.m_fGeneratorFuelMinutes * 60 * 0.5; // a portable one comes half full
			m_aGenerators.Insert(g);
		}

		g.m_vPos = kit.GetOrigin();
		if (refuel)
			Refuel(playerId, g, user);
		else
			Toggle(playerId, g);
	}

	//------------------------------------------------------------------------------------------------
	void FoundGeneratorLabels(RZ_Spot s, out string read, out string choiceLabels)
	{
		read = "Look at the generator";
		choiceLabels = "Start or stop the generator|Refuel the generator";
	}

	//------------------------------------------------------------------------------------------------
	void DescribeFoundGenerator(int playerId, RZ_Spot s)
	{
		RZ_Generator g = FindGenerator(null, s.m_sKey);
		string text = "A diesel generator under a tarp. The tank gauge reads empty.";
		if (g && g.m_bRunning)
			text = string.Format("Running, loud and steady. About %1 minutes of fuel left.", RZ_Util.RoundToInt(g.m_fFuel / 60));
		else if (g && g.m_fFuel > 0)
			text = string.Format("Cold, with about %1 minutes of fuel in it.", RZ_Util.RoundToInt(g.m_fFuel / 60));

		m_GM.NotifyPlayer(playerId, "Generator", text);
	}

	//------------------------------------------------------------------------------------------------
	//! 0 start or stop, 1 refuel. A found generator starts out empty.
	void HandleFoundGenerator(int playerId, RZ_Spot s, int option, IEntity user)
	{
		if (!m_Settings.m_bEnabled)
			return;

		RZ_Generator g = FindGenerator(null, s.m_sKey);
		if (!g)
		{
			g = new RZ_Generator();
			g.m_sKey = s.m_sKey;
			g.m_vPos = s.m_vPos;
			m_aGenerators.Insert(g);
		}

		if (option == 1)
			Refuel(playerId, g, user);
		else if (option == 0)
			Toggle(playerId, g);
	}

	//------------------------------------------------------------------------------------------------
	protected void Refuel(int playerId, RZ_Generator g, IEntity user)
	{
		if (RZ_ItemHelper.Take(user, m_Settings.m_sFuelWords + RZ_ItemHelper.NOT_EMPTY, 1) <= 0)
		{
			m_GM.NotifyPlayer(playerId, "Generator", "You've no fuel for it. A jerrycan, from a garage or a wreck.");
			return;
		}

		g.m_fFuel = Math.Min(g.m_fFuel + m_Settings.m_fGeneratorFuelMinutes * 60, m_Settings.m_fGeneratorFuelMinutes * 60 * 2);
		m_GM.NotifyPlayer(playerId, "Generator", string.Format("You top up the tank: about %1 minutes of running.", RZ_Util.RoundToInt(g.m_fFuel / 60)));
	}

	//------------------------------------------------------------------------------------------------
	protected void Toggle(int playerId, RZ_Generator g)
	{
		if (g.m_bRunning)
		{
			g.m_bRunning = false;
			m_GM.NotifyPlayer(playerId, "Generator", "It coughs and stops. The silence is enormous.");
			return;
		}

		if (g.m_fFuel <= 0)
		{
			m_GM.NotifyPlayer(playerId, "Generator", "It turns over and dies. No fuel.");
			return;
		}

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		if (CountRunning(ident) >= m_Settings.m_iGeneratorsPerPlayer)
		{
			m_GM.NotifyPlayer(playerId, "Generator", "You're already running one. Two would bring every dead thing in the county.");
			return;
		}

		g.m_bRunning = true;
		g.m_sOwner = ident;
		g.m_iOwner = playerId;
		m_GM.GetHeat().AddHeat(g.m_vPos, 8);
		m_GM.NotifyPlayer(playerId, "Generator", "It catches and roars. Lights, if there's anything wired to it; a radio nearby will pick up every open channel; stand close for a minute and your batteries charge. And everything for half a mile can hear it.");
	}

	//------------------------------------------------------------------------------------------------
	protected int CountRunning(string ident)
	{
		int n = 0;
		foreach (RZ_Generator g : m_aGenerators)
		{
			if (g.m_bRunning && g.m_sOwner == ident)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickGenerators(float dt, array<int> ids, array<IEntity> players)
	{
		for (int i = m_aGenerators.Count() - 1; i >= 0; i--)
		{
			RZ_Generator g = m_aGenerators[i];
			if (g.m_sKey.IsEmpty() && !g.m_Ent)
			{
				m_aGenerators.Remove(i);
				continue;
			}

			if (g.m_Ent)
			{
				IEntity root = g.m_Ent.GetRootParent();
				if (root && root != g.m_Ent)
				{
					// Picked up: it stops
					g.m_bRunning = false;
					continue;
				}

				g.m_vPos = g.m_Ent.GetOrigin();
			}

			if (!g.m_bRunning)
				continue;

			g.m_fFuel = g.m_fFuel - dt;
			m_GM.GetHeat().AddHeat(g.m_vPos, m_Settings.m_fGeneratorHeatPerMinute * dt / 60);
			if (g.m_fFuel <= 0)
			{
				g.m_fFuel = 0;
				g.m_bRunning = false;
				m_Hub.NotifyNear(g.m_vPos, 150, -1, "Generator", "The generator sputters and dies. Out of fuel.");
			}
		}

		// Charging: stand next to a running generator for a minute
		foreach (int j, IEntity p : players)
		{
			int playerId = ids[j];
			float charge;
			m_mCharge.Find(playerId, charge);
			if (!IsRunningNear(p.GetOrigin(), CHARGE_RADIUS))
			{
				m_mCharge.Remove(playerId);
				continue;
			}

			charge += dt;
			m_mCharge.Set(playerId, charge);
			if (charge >= m_Settings.m_fChargeSeconds && !IsCharged(playerId))
			{
				m_mCharged.Set(m_GM.GetProfile(playerId).m_sIdentity, m_Hub.GetClock() + m_Settings.m_fChargedMinutes * 60);
				m_GM.NotifyPlayer(playerId, "Charged", "You run a lead off the generator: your radio and torch batteries are full. A charged radio digs a scrambled channel out of the static faster.");
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	bool IsRunningNear(vector pos, float radius)
	{
		foreach (RZ_Generator g : m_aGenerators)
		{
			if (g.m_bRunning && RZ_Util.DistXZ(g.m_vPos, pos) <= radius)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Lights hook: a running generator within its light radius.
	bool IsLitAt(vector pos)
	{
		return IsRunningNear(pos, m_Settings.m_fGeneratorLightRadius);
	}

	//------------------------------------------------------------------------------------------------
	//! A radio this close to a running generator picks up every open channel.
	bool IsRadioBoosted(vector pos)
	{
		return IsRunningNear(pos, m_Settings.m_fGeneratorRadioRadius);
	}

	//------------------------------------------------------------------------------------------------
	bool IsCharged(int playerId)
	{
		float until;
		if (!m_mCharged.Find(m_GM.GetProfile(playerId).m_sIdentity, until))
			return false;

		return m_Hub.GetClock() < until;
	}

	// ================================================================ Fire
	//------------------------------------------------------------------------------------------------
	bool IsBurned(vector buildingPos)
	{
		float keep = m_Settings.m_fBurnMinutes * 60;
		foreach (RZ_Burn b : m_aBurns)
		{
			if (m_Hub.GetClock() - b.m_fAt < keep && RZ_Util.DistXZ(b.m_vPos, buildingPos) <= SAME_BUILDING)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! For the zombie spawner: no infected spawn right around a building that's still smouldering.
	bool IsScorched(vector pos)
	{
		float keep = m_Settings.m_fBurnMinutes * 60;
		foreach (RZ_Burn b : m_aBurns)
		{
			if (m_Hub.GetClock() - b.m_fAt < keep && RZ_Util.DistXZ(b.m_vPos, pos) <= SCORCH_RADIUS)
				return true;
		}

		return false;
	}

	// ================================================================ Fuel at the doorway
	//------------------------------------------------------------------------------------------------
	//! Any machine: the settings the client checks with (the game mode's, else the defaults).
	protected static RZ_TrapSettings SharedSettings()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetTrapSettings())
			return gm.GetTrapSettings();

		if (!s_ClientSettings)
			s_ClientSettings = RZ_TrapSettings.Cast(RZ_Util.NewWithDefaults("RZ_TrapSettings"));

		return s_ClientSettings;
	}

	//------------------------------------------------------------------------------------------------
	protected static array<string> FuelWords(RZ_TrapSettings settings)
	{
		string from = settings.m_sFuelWords + RZ_ItemHelper.NOT_EMPTY;
		if (!s_aFuelWords || s_sFuelWordsFrom != from)
		{
			s_aFuelWords = new array<string>();
			RZ_ItemHelper.Words(from, s_aFuelWords);
			s_sFuelWordsFrom = from;
		}

		return s_aFuelWords;
	}

	//------------------------------------------------------------------------------------------------
	//! Any machine: the full fuel can this character would pour. With Fuel in your hands on, only
	//! the one held in the hands; off, any full can it carries. null if none.
	static IEntity FireFuel(IEntity character)
	{
		RZ_TrapSettings settings = SharedSettings();
		if (!character || !settings || !settings.m_bFire)
			return null;

		array<string> words = FuelWords(settings);
		IEntity held = RZ_ItemHelper.HeldItem(character);
		if (RZ_ItemHelper.Matches(held, words))
			return held;

		if (settings.m_bFuelInHands)
			return null;

		return RZ_ItemHelper.FirstMatch(character, words);
	}

	//------------------------------------------------------------------------------------------------
	//! On the 5 s tick: a burn spot at the nearest outside door of the nearest building that can
	//! burn, within BURN_REACH of each player who has fuel for it (one per player; two players at
	//! the same building share it). Spots nobody with fuel is near go at once.
	protected void TickBurnSpots(array<IEntity> players)
	{
		array<string> wanted = {};
		if (m_Settings.m_bFire)
		{
			foreach (IEntity p : players)
			{
				if (!p || RZ_Util.IsInVehicle(p) || !FireFuel(p))
					continue;

				IEntity building = FindBurnable(p.GetOrigin());
				if (!building)
					continue;

				vector bpos = building.GetOrigin();
				string bkey = RZ_Interactions.PosKey(bpos, 4);
				string key = "burn_" + bkey;
				if (wanted.Contains(key))
					continue;

				wanted.Insert(key);
				m_Hub.EnsureSpot(RZ_Interactions.KIND_BURN, key, Doorway(building, bkey, p.GetOrigin()), bpos, "house", "the doorway");
			}
		}

		m_Hub.RemoveSpotsOfKind(RZ_Interactions.KIND_BURN, wanted);
		if (m_mDoorway.Count() > 200)
			m_mDoorway.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest building within BURN_REACH that can burn (not burned, not a protected place).
	protected IEntity FindBurnable(vector pos)
	{
		array<IEntity> buildings = {};
		RZ_BuildingQuery.FindBuildings(pos, BURN_REACH, buildings, 16);
		IEntity best;
		float bestDist = float.MAX;
		foreach (IEntity b : buildings)
		{
			vector bpos = b.GetOrigin();
			if (IsBurned(bpos) || m_Hub.IsProtected(bpos))
				continue;

			float d = RZ_Util.DistXZ(bpos, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = b;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Where a building's burn spot stands: just outside its outside door nearest `from` (the
	//! doors are the building's door children, as for locks), else just outside the side of its
	//! box nearest `from` (the entrance side). Remembered per building.
	protected vector Doorway(IEntity building, string buildingKey, vector from)
	{
		vector known;
		if (m_mDoorway.Find(buildingKey, known))
			return known;

		vector at = building.GetOrigin();
		bool found = false;
		float best = float.MAX;
		array<IEntity> doors = {};
		RZ_DoorGeometry.FindDoors(building, 0, doors);
		foreach (IEntity door : doors)
		{
			if (!door || !RZ_DoorGeometry.IsOuterDoor(building, door))
				continue;

			vector outside = RZ_DoorGeometry.OutsidePoint(building, door, DOORWAY_OUT);
			float d = RZ_Util.DistXZ(outside, from);
			if (d < best)
			{
				best = d;
				at = outside;
				found = true;
			}
		}

		if (!found)
			at = RZ_DoorGeometry.SidePoint(building, from, DOORWAY_OUT);

		at = RZ_Util.GroundPos(at);
		m_mDoorway.Set(buildingKey, at);
		return at;
	}

	//------------------------------------------------------------------------------------------------
	//! Douse the doorway and set it alight (the burn spot): fuel in your hands, a few seconds.
	void StartBurn(int playerId, RZ_Spot s, IEntity user)
	{
		string title = "Fire";
		if (!m_Settings.m_bFire || !s)
			return;

		if (m_Hub.IsProtected(s.m_vBuilding))
		{
			m_GM.NotifyPlayer(playerId, title, "Not here. People live here, or someone holds it, and they'd have it out before it caught.");
			return;
		}

		if (IsBurned(s.m_vBuilding))
		{
			m_GM.NotifyPlayer(playerId, title, "It's already burned.");
			return;
		}

		if (!FireFuel(user))
		{
			m_GM.NotifyPlayer(playerId, title, NoFuelText());
			return;
		}

		float seconds = Math.Max(1, m_Settings.m_fBurnSeconds);
		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_BURN, s.m_iId, 0, seconds, "the fire", false);
		m_GM.NotifyPlayer(playerId, title, string.Format("You slosh it over the doorframe and the floor inside. About %1 seconds, then get clear.", RZ_Util.RoundToInt(seconds)));
	}

	//------------------------------------------------------------------------------------------------
	protected string NoFuelText()
	{
		if (m_Settings.m_bFuelInHands)
			return "You'd need a full can of fuel in your hands to set it going.";

		return "You'd need a full can of fuel to set it going.";
	}

	//------------------------------------------------------------------------------------------------
	//! The can is checked again and used up (the whole can), then the building burns.
	void CompleteBurn(int playerId, RZ_Spot s, IEntity user)
	{
		if (!s)
			return;

		IEntity can = FireFuel(user);
		if (!can)
		{
			m_GM.NotifyPlayer(playerId, "Fire", NoFuelText());
			return;
		}

		if (IsBurned(s.m_vBuilding) || m_Hub.IsProtected(s.m_vBuilding))
			return;

		RZ_ItemHelper.DeleteCarried(user, can);
		Burn(s.m_vBuilding, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A molotov landed: heat, the infected right there die, and the building it's by burns.
	void Ignite(vector pos, int playerId)
	{
		m_GM.GetHeat().AddHeat(pos, m_Settings.m_fMolotovHeat);
		for (int i = 0; i < 3; i++)
		{
			IEntity z = m_GM.GetSpawner().FindZombieNear(pos, m_Settings.m_fMolotovRadius);
			if (!z)
				break;

			RZ_Util.KillCharacter(z);
		}

		SpawnFire(pos);
		if (m_GM.GetTribes())
			m_GM.GetTribes().OnFire(pos, playerId);

		if (!m_Settings.m_bFire)
			return;

		vector building;
		if (FindBuildingNear(pos, m_Settings.m_fMolotovReach, building) && !m_Hub.IsProtected(building) && !IsBurned(building))
			Burn(building, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected vector m_vQueryPos;
	protected vector m_vBestBuilding;
	protected float m_fBestBuilding;

	//------------------------------------------------------------------------------------------------
	protected bool FindBuildingNear(vector pos, float radius, out vector building)
	{
		m_vQueryPos = pos;
		m_fBestBuilding = float.MAX;
		GetGame().GetWorld().QueryEntitiesBySphere(pos, radius, OnFireBuilding, null, EQueryEntitiesFlags.STATIC);
		building = m_vBestBuilding;
		return m_fBestBuilding < float.MAX;
	}

	//------------------------------------------------------------------------------------------------
	protected bool OnFireBuilding(IEntity e)
	{
		bool isBuilding = SCR_DestructibleBuildingEntity.Cast(e) != null;
		if (!isBuilding)
		{
			string name = RZ_ItemHelper.LowerName(e);
			isBuilding = name.Contains("/buildings/") || name.Contains("house");
		}

		if (!isBuilding)
			return true;

		float d = RZ_Util.DistXZ(e.GetOrigin(), m_vQueryPos);
		if (d < m_fBestBuilding)
		{
			m_fBestBuilding = d;
			m_vBestBuilding = e.GetOrigin();
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! World events (Gas Station Fire): this one building catches. False if fire is
	//! off, or it can't burn (a protected place) or already has. quiet = no "smoke" notice nearby.
	bool BurnBuilding(vector buildingPos, bool quiet)
	{
		if (!m_Settings.m_bFire || m_Hub.IsProtected(buildingPos) || IsBurned(buildingPos))
			return false;

		Burn(buildingPos, -1, quiet);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! World events: the building nearest pos within radius catches (see BurnBuilding). False if none.
	bool BurnNear(vector pos, float radius, bool quiet)
	{
		vector building;
		if (!FindBuildingNear(pos, radius, building))
			return false;

		return BurnBuilding(building, quiet);
	}

	//------------------------------------------------------------------------------------------------
	//! Fire is on (Traps > Fire).
	bool IsFireOn()
	{
		return m_Settings.m_bFire;
	}

	//------------------------------------------------------------------------------------------------
	//! One building burns: data, plus a fire effect for a few minutes.
	protected void Burn(vector pos, int playerId, bool quiet = false)
	{
		RZ_Burn b = new RZ_Burn();
		b.m_vPos = pos;
		b.m_fAt = m_Hub.GetClock();
		m_aBurns.Insert(b);
		while (m_aBurns.Count() > 200)
		{
			m_aBurns.RemoveOrdered(0); // the oldest burn goes, not whichever was last
		}

		RZ_TownMemory.ReportBuildingBurned(pos);
		if (m_GM.GetTribes())
			m_GM.GetTribes().OnFire(pos, playerId);

		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s && RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius + 100)
			s.AddInfection(-m_Settings.m_fBurnInfectionDrop);

		m_GM.GetHeat().AddHeat(pos, m_Settings.m_fBurnHeat);
		SpawnFire(pos);
		m_Hub.RelabelBuilding(RZ_Interactions.PosKey(pos, 4));
		if (playerId > 0)
			m_GM.NotifyPlayer(playerId, "Fire", "It catches all at once, with a sound like a breath. Nothing in there will be worth finding for a long while, and nothing will crawl out of it either.");

		if (!quiet)
			m_Hub.NotifyNear(pos, 800, playerId, "Smoke", "Black smoke is going up from a building nearby.");
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnFire(vector pos)
	{
		if (m_Settings.m_sFireEffectPrefab.IsEmpty())
			return;

		IEntity fire = RZ_Util.SpawnPrefab(m_Settings.m_sFireEffectPrefab, RZ_Util.GroundPos(pos), 0);
		if (!fire)
			return;

		m_aFires.Insert(fire);
		m_aFireUntil.Insert(m_Hub.GetClock() + m_Settings.m_fFireEffectSeconds);
	}

	//------------------------------------------------------------------------------------------------
	protected void TickFire()
	{
		float now = m_Hub.GetClock();
		for (int i = m_aFires.Count() - 1; i >= 0; i--)
		{
			if (m_aFires[i] && now < m_aFireUntil[i])
				continue;

			if (m_aFires[i])
				SCR_EntityHelper.DeleteEntityAndChildren(m_aFires[i]);

			m_aFires.Remove(i);
			m_aFireUntil.Remove(i);
		}

		float keep = m_Settings.m_fBurnMinutes * 60;
		for (int j = m_aBurns.Count() - 1; j >= 0; j--)
		{
			if (now - m_aBurns[j].m_fAt >= keep)
				m_aBurns.RemoveOrdered(j);
		}
	}
}
