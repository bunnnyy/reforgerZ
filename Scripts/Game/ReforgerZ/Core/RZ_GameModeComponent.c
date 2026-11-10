[ComponentEditorProps(category: "ReforgerZ", description: "ReforgerZ zombie survival logic. Attach to the GameMode entity of any world.")]
class RZ_GameModeComponentClass : SCR_BaseGameModeComponentClass
{
}

//! The hub of the mode. Owns every system, runs the server tick, handles player
//! spawns and backgrounds, saves state, and sends notifications to clients.
//!
//! Server-authoritative: everything except the replicated summary values
//! (stage, cure progress) exists only on the server.
class RZ_GameModeComponent : SCR_BaseGameModeComponent
{
	// ---------------------------------------------------------------- General
	[Attribute("5", UIWidgets.Slider, "Server tick interval (seconds)", "1 30 1", category: "ReforgerZ")]
	protected float m_fTickSeconds;

	[Attribute("SURV", desc: "Faction key players spawn as", category: "ReforgerZ")]
	protected string m_sPlayerFactionKey;

	[Attribute("0", desc: "Ignore the saved state on start (season wipe)", category: "ReforgerZ")]
	protected bool m_bWipeOnStart;

	[Attribute("120", desc: "Seconds between state saves", category: "ReforgerZ")]
	protected float m_fSaveIntervalSeconds;

	// ---------------------------------------------------------------- World
	[Attribute("300", desc: "Grid cell (m) used to find building clusters without a map label", category: "ReforgerZ|World")]
	protected float m_fScanCellSize;

	[Attribute("8", desc: "Buildings needed in one cell to count as a settlement", category: "ReforgerZ|World")]
	protected int m_iScanMinBuildings;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Spawn point prefab (SCR_SpawnPoint), placed in random towns", "et", category: "ReforgerZ|World")]
	protected ResourceName m_sSpawnPointPrefab;

	[Attribute("6", desc: "Number of active spawn points", category: "ReforgerZ|World")]
	protected int m_iSpawnPointCount;

	[Attribute("20", desc: "Minutes between moving one spawn point to a new town", category: "ReforgerZ|World")]
	protected float m_fSpawnRotateMinutes;

	// ---------------------------------------------------------------- Director / zombies
	[Attribute(desc: "Stage overrides. Leave empty to use the built-in defaults", category: "ReforgerZ|Director")]
	protected ref array<ref RZ_StageSettings> m_aStages;

	[Attribute(desc: "Zombie prefabs and density", category: "ReforgerZ|Zombies")]
	protected ref RZ_ZombieSettings m_ZombieSettings;

	[Attribute("0.5", desc: "Heat per tick each player gives off just by being somewhere", category: "ReforgerZ|Zombies")]
	protected float m_fHeatPresence;

	[Attribute("4", desc: "Extra heat per tick while in a vehicle", category: "ReforgerZ|Zombies")]
	protected float m_fHeatVehicle;

	// ---------------------------------------------------------------- Backgrounds
	[Attribute(desc: "Civilian backgrounds. Leave empty to use the built-in eight", category: "ReforgerZ|Backgrounds")]
	protected ref array<ref RZ_Background> m_aBackgrounds;

	[Attribute("20", desc: "Base minutes from a bite to turning", category: "ReforgerZ|Backgrounds")]
	protected float m_fInfectionMinutes;

	// ---------------------------------------------------------------- Cure
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Lab prefab with RZ_LabComponent (spawned if none is placed)", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sLabPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Research sample item with RZ_SampleItemComponent", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sSamplePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Antidote item the lab produces after the second breakthrough", "et", category: "ReforgerZ|Cure")]
	protected ResourceName m_sAntidotePrefab;

	[Attribute("2", desc: "Research percent per sample (x background multiplier)", category: "ReforgerZ|Cure")]
	protected float m_fResearchPerSample;

	// ---------------------------------------------------------------- Rewards
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Loot crate spawned as a mission reward", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sRewardCratePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Friendly survivor AI group for holdout missions (optional)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sSurvivorGroupPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Destructible zombie nest, e.g. from Zombie Hive (optional, enables Burn the Nest)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sNestPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bandit dead drop with RZ_SellSamplesToBanditsAction (optional, enables the bandit path)", "et", category: "ReforgerZ|Missions")]
	protected ResourceName m_sDeadDropPrefab;

	// ---------------------------------------------------------------- Survival
	[Attribute("3", desc: "Heat per shot fired by a player (times the background noise multiplier)", category: "ReforgerZ|Survival")]
	protected float m_fHeatPerShot;

	[Attribute("0.3", desc: "Chance that a zombie hit infects. 0 disables built-in bite detection", category: "ReforgerZ|Survival")]
	protected float m_fBiteChance;

	[Attribute("2.5", desc: "How close (m) a zombie must be for a health drop to count as its hit", category: "ReforgerZ|Survival")]
	protected float m_fBiteReach;

	// ---------------------------------------------------------------- Debug
	[Attribute("1", desc: "Log a world status summary every few minutes", category: "ReforgerZ|Debug")]
	protected bool m_bStatusLog;

	// ---------------------------------------------------------------- Replicated summary
	[RplProp()]
	protected float m_fReplCureProgress;

	[RplProp()]
	protected int m_iReplStage;

	[RplProp()]
	protected bool m_bReplCured;

	// ---------------------------------------------------------------- Runtime (server)
	protected static RZ_GameModeComponent s_Instance;

	protected ref array<ref RZ_Settlement> m_aSettlements = {};
	protected ref RZ_Director m_Director;
	protected ref RZ_HeatMap m_Heat;
	protected ref RZ_ZombieSpawner m_Spawner;
	protected ref RZ_MissionManager m_Missions;
	protected ref RZ_CureResearch m_Cure;
	protected ref RZ_BiteDetector m_BiteDetector;
	protected float m_fStatusTimer;
	protected ref map<string, ref RZ_PlayerProfile> m_mProfiles = new map<string, ref RZ_PlayerProfile>();
	protected ref array<IEntity> m_aSpawnPoints = {};
	protected ref set<int> m_aWelcomed = new set<int>();

	protected IEntity m_Lab;
	protected vector m_vLabPos;
	protected bool m_bHasLab;
	protected float m_fSaveTimer;
	protected float m_fSpawnRotateTimer;
	protected bool m_bServerRunning;

	//------------------------------------------------------------------------------------------------
	static RZ_GameModeComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		if (Replication.IsServer())
			InitServer();
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_GameModeComponent()
	{
		if (GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(ShowNextHint);

		if (m_bServerRunning && GetGame() && GetGame().GetCallqueue())
		{
			GetGame().GetCallqueue().Remove(ServerTick);
			GetGame().GetCallqueue().Remove(BiteCheck);
			SaveState();
		}

		if (s_Instance == this)
			s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitServer()
	{
		if (!m_ZombieSettings)
			m_ZombieSettings = new RZ_ZombieSettings();

		if (!m_aBackgrounds || m_aBackgrounds.IsEmpty())
		{
			m_aBackgrounds = new array<ref RZ_Background>();
			RZ_Background.CreateDefaults(m_aBackgrounds);
		}

		m_Heat = new RZ_HeatMap();
		m_Cure = new RZ_CureResearch(this);
		m_Director = new RZ_Director(this, m_aStages);
		m_Spawner = new RZ_ZombieSpawner(this, m_ZombieSettings);
		m_Missions = new RZ_MissionManager(this);
		m_BiteDetector = new RZ_BiteDetector(this, m_fBiteChance, m_fBiteReach);

		bool loaded = false;
		if (!m_bWipeOnStart)
			loaded = LoadState();

		if (m_aSettlements.IsEmpty())
		{
			RZ_SettlementScanner scanner = new RZ_SettlementScanner();
			scanner.Scan(m_fScanCellSize, m_iScanMinBuildings, m_aSettlements);
		}

		if (m_aSettlements.IsEmpty())
			RZ_Log.Error("No settlements found on this map. Place RZ_SettlementMarkerEntity entities in the world.");

		SetupLab();
		RefreshSpawnPoints(true);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetOwner());
		if (gameMode)
			gameMode.GetOnPlayerSpawned().Insert(OnPlayerSpawned); // VERIFY: invoker name

		UpdateReplicated();
		m_bServerRunning = true;
		GetGame().GetCallqueue().CallLater(ServerTick, m_fTickSeconds * 1000, true);
		// Bites need a faster check than the main tick to catch individual hits
		GetGame().GetCallqueue().CallLater(BiteCheck, 1000, true);

		RZ_Log.Info(string.Format("Started. %1 settlements, stage %2, cure %3 percent, state %4.",
			m_aSettlements.Count(), RZ_StageSettings.GetStageName(m_Director.GetStage()),
			RZ_Util.RoundToInt(m_Cure.m_fProgress), loaded));
	}

	//------------------------------------------------------------------------------------------------
	protected void ServerTick()
	{
		float dt = m_fTickSeconds;
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);

		UpdatePlayerHeat(ids, players);
		m_Heat.Decay(dt);
		m_Director.Tick(dt);
		m_Spawner.Tick(dt, players);
		m_Missions.Tick(dt, ids, players);
		m_Cure.Tick(dt, players);
		UpdateInfections(dt, ids);

		foreach (int i, IEntity ent : players)
		{
			RZ_Objectives.Tick(this, ids[i], GetProfile(ids[i]), ent, dt);
		}

		if (m_bStatusLog)
		{
			m_fStatusTimer += dt;
			if (m_fStatusTimer >= 300)
			{
				m_fStatusTimer = 0;
				LogStatus(ids.Count());
			}
		}

		m_fSpawnRotateTimer += dt;
		if (m_fSpawnRotateTimer >= m_fSpawnRotateMinutes * 60)
		{
			m_fSpawnRotateTimer = 0;
			RefreshSpawnPoints(false);
		}

		m_fSaveTimer += dt;
		if (m_fSaveTimer >= m_fSaveIntervalSeconds)
		{
			m_fSaveTimer = 0;
			SaveState();
		}
	}

	// ================================================================ Accessors
	array<ref RZ_Settlement> GetSettlements() { return m_aSettlements; }
	RZ_Director GetDirector() { return m_Director; }
	RZ_HeatMap GetHeat() { return m_Heat; }
	RZ_ZombieSpawner GetSpawner() { return m_Spawner; }
	RZ_MissionManager GetMissions() { return m_Missions; }
	RZ_CureResearch GetCure() { return m_Cure; }
	vector GetLabPosition() { return m_vLabPos; }
	ResourceName GetSurvivorGroupPrefab() { return m_sSurvivorGroupPrefab; }
	ResourceName GetNestPrefab() { return m_sNestPrefab; }
	ResourceName GetDeadDropPrefab() { return m_sDeadDropPrefab; }

	float GetReplicatedCureProgress() { return m_fReplCureProgress; }
	RZ_EOutbreakStage GetReplicatedStage() { return m_iReplStage; }
	bool IsReplicatedCured() { return m_bReplCured; }

	//------------------------------------------------------------------------------------------------
	RZ_Settlement GetSettlement(int idx)
	{
		if (idx < 0 || idx >= m_aSettlements.Count())
			return null;

		return m_aSettlements[idx];
	}

	//------------------------------------------------------------------------------------------------
	int FindNearestSettlement(vector pos)
	{
		int best = -1;
		float bestDist = float.MAX;
		foreach (int i, RZ_Settlement s : m_aSettlements)
		{
			float d = RZ_Util.DistXZ(pos, s.m_vCenter);
			if (d < bestDist)
			{
				bestDist = d;
				best = i;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Public API for other mods: report a loud event (gunfire, explosion, alarm...).
	static void ReportNoise(vector pos, float amount)
	{
		if (s_Instance && s_Instance.m_Heat)
			s_Instance.m_Heat.AddHeat(pos, amount);
	}

	// ================================================================ Players, backgrounds, reputation
	//------------------------------------------------------------------------------------------------
	protected string GetIdentity(int playerId)
	{
		string ident = GetGame().GetBackendApi().GetPlayerIdentityId(playerId); // VERIFY
		if (ident.IsEmpty())
			ident = "local_" + GetGame().GetPlayerManager().GetPlayerName(playerId);

		return ident;
	}

	//------------------------------------------------------------------------------------------------
	RZ_PlayerProfile GetProfile(int playerId)
	{
		string ident = GetIdentity(playerId);
		RZ_PlayerProfile profile;
		if (!m_mProfiles.Find(ident, profile))
		{
			profile = new RZ_PlayerProfile();
			profile.m_sIdentity = ident;
			m_mProfiles.Insert(ident, profile);
		}

		return profile;
	}

	//------------------------------------------------------------------------------------------------
	RZ_Background GetBackground(string id)
	{
		foreach (RZ_Background bg : m_aBackgrounds)
		{
			if (bg.m_sId == id)
				return bg;
		}

		return m_aBackgrounds[m_aBackgrounds.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	RZ_Background GetPlayerBackground(int playerId)
	{
		return GetBackground(GetProfile(playerId).m_sBackgroundId);
	}

	//------------------------------------------------------------------------------------------------
	//! The background is whichever loadout the player picked in the deploy menu.
	protected RZ_Background ResolveBackground(int playerId)
	{
		PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (pc)
		{
			SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(pc.FindComponent(SCR_PlayerLoadoutComponent));
			if (loadoutComp)
			{
				SCR_BasePlayerLoadout loadout = loadoutComp.GetLoadout(); // VERIFY
				if (loadout)
				{
					string loadoutName = loadout.GetLoadoutName();
					foreach (RZ_Background bg : m_aBackgrounds)
					{
						if (bg.m_sLoadoutName == loadoutName)
							return bg;
					}

					RZ_Log.Warn("No background matches loadout '" + loadoutName + "'");
				}
			}
		}

		return m_aBackgrounds[m_aBackgrounds.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		RZ_Background bg = ResolveBackground(playerId);
		RZ_PlayerProfile profile = GetProfile(playerId);
		profile.StartNewLife(bg);
		RZ_Objectives.Assign(this, profile, bg, controlledEntity.GetOrigin());

		m_BiteDetector.Reset(playerId);
		RegisterGunfire(controlledEntity);

		// 1. Who you are this life
		string intro = string.Format("Perk: %1\nFlaw: %2\n\nYour goal: %3",
			bg.m_sPerk, bg.m_sFlaw, RZ_Objectives.Describe(this, profile));
		NotifyPlayer(playerId, bg.m_sName, intro);

		// 2. The state of the world (first spawn of this session only)
		if (!m_aWelcomed.Contains(playerId))
		{
			m_aWelcomed.Insert(playerId);
			string lab = "";
			if (m_bHasLab)
			{
				lab = string.Format("\nThe Institute's lab: %1. Bring it samples from special infected.",
					RZ_Util.DirectionText(controlledEntity.GetOrigin(), m_vLabPos));
			}

			NotifyPlayer(playerId, "Outbreak: " + RZ_StageSettings.GetStageName(m_Director.GetStage()),
				string.Format("Cure research: %1 percent.%2", RZ_Util.RoundToInt(m_Cure.m_fProgress), lab));
		}

		// 3. What's on the radio
		NotifyPlayer(playerId, "Radio", m_Missions.DescribeActiveFor(controlledEntity));
	}

	//------------------------------------------------------------------------------------------------
	//! Every player gets the brief with the distance and direction from where they are.
	void AnnounceMission(RZ_Mission mission)
	{
		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			string where = mission.GetLocationName(this);
			IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
			if (ent)
				where = where + ", " + RZ_Util.DirectionText(ent.GetOrigin(), mission.m_vPos);

			NotifyPlayer(playerId, "New mission: " + mission.m_sTitle, string.Format("%1\n(%2)", mission.m_sBrief, where));
		}
	}

	// ================================================================ Gunfire
	//------------------------------------------------------------------------------------------------
	protected void RegisterGunfire(IEntity character)
	{
		EventHandlerManagerComponent ev = EventHandlerManagerComponent.Cast(character.FindComponent(EventHandlerManagerComponent));
		if (ev)
			ev.RegisterScriptHandler("OnProjectileShot", this, OnPlayerShot); // VERIFY: event name and signature
	}

	//------------------------------------------------------------------------------------------------
	//! Every shot is heard. Hunters are quieter, Mechanics and Radio Operators louder.
	//! TODO: lower heat for suppressed weapons.
	protected void OnPlayerShot(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		IEntity shooter = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
		if (!shooter)
			return;

		m_Heat.AddHeat(shooter.GetOrigin(), m_fHeatPerShot * GetPlayerBackground(playerID).m_fNoiseMult);
	}

	//------------------------------------------------------------------------------------------------
	protected void BiteCheck()
	{
		m_BiteDetector.Check();
	}

	//------------------------------------------------------------------------------------------------
	protected void LogStatus(int playerCount)
	{
		int safe, contested, infested, overrun;
		foreach (RZ_Settlement s : m_aSettlements)
		{
			switch (s.GetState())
			{
				case RZ_ESettlementState.SAFE: safe++; break;
				case RZ_ESettlementState.CONTESTED: contested++; break;
				case RZ_ESettlementState.INFESTED: infested++; break;
				case RZ_ESettlementState.OVERRUN: overrun++; break;
			}
		}

		RZ_Log.Info(string.Format("Status | %1 (%2 min) | avg infection %3 | towns safe/contested/infested/overrun %4/%5/%6/%7 | zombies %8 | missions %9",
			RZ_StageSettings.GetStageName(m_Director.GetStage()), RZ_Util.RoundToInt(m_Director.GetStageMinutes()),
			RZ_Util.RoundToInt(m_Director.GetAverageInfection()), safe, contested, infested, overrun,
			m_Spawner.GetTrackedCount(), m_Missions.GetActive().Count()));
		RZ_Log.Info(string.Format("Status | players %1 | cure %2 percent (milestone %3, final stand %4)",
			playerCount, RZ_Util.RoundToInt(m_Cure.m_fProgress), m_Cure.m_iMilestone, m_Cure.IsFinalStand()));
	}

	//------------------------------------------------------------------------------------------------
	void AddReputation(int playerId, RZ_EFaction faction, int amount)
	{
		if (amount == 0 || playerId <= 0)
			return;

		int rep = GetProfile(playerId).AddRep(faction, amount);
		string sign = "+";
		if (amount < 0)
			sign = "";

		NotifyPlayer(playerId, "Reputation", string.Format("%1: %2%3 (%4)",
			RZ_Factions.GetName(faction), sign, amount, RZ_Factions.GetStanding(rep)));
	}

	// ================================================================ Infection
	//------------------------------------------------------------------------------------------------
	//! Public API: call this from a zombie mod's bite or damage event.
	void InfectPlayer(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		if (profile.IsInfected())
			return;

		RZ_Background bg = GetPlayerBackground(playerId);
		float seconds = m_fInfectionMinutes * 60 * bg.m_fInfectionTimeMult * m_Cure.GetInfectionTimeMultiplier();
		profile.m_fInfectionLeft = seconds;
		NotifyPlayer(playerId, "Infected", string.Format(
			"You've been bitten. You have about %1 minutes. Find an antidote, or decide whether to tell anyone.",
			RZ_Util.RoundToInt(seconds / 60)));
	}

	//------------------------------------------------------------------------------------------------
	//! Public API: call from the antidote item's use action.
	void CurePlayer(int playerId)
	{
		RZ_PlayerProfile profile = GetProfile(playerId);
		if (!profile.IsInfected())
			return;

		profile.m_fInfectionLeft = -1;
		NotifyPlayer(playerId, "Cured", "The fever breaks. You'll live, for now.");
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateInfections(float dt, array<int> ids)
	{
		foreach (int playerId : ids)
		{
			RZ_PlayerProfile profile = GetProfile(playerId);
			if (!profile.IsInfected())
				continue;

			float before = profile.m_fInfectionLeft;
			profile.m_fInfectionLeft = profile.m_fInfectionLeft - dt;

			if (before > 300 && profile.m_fInfectionLeft <= 300)
				NotifyPlayer(playerId, "Infection", "The fever is getting worse. You have about 5 minutes.");

			if (profile.m_fInfectionLeft <= 0)
			{
				profile.m_fInfectionLeft = -1;
				TurnPlayer(playerId);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The player dies and gets up as one of them.
	protected void TurnPlayer(int playerId)
	{
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ent)
			return;

		vector pos = ent.GetOrigin();
		RZ_Util.KillCharacter(ent);
		m_Spawner.SpawnZombie(pos, false, -1, -1);
		NotifyAll("Turned", GetGame().GetPlayerManager().GetPlayerName(playerId) + " has turned.");
	}

	// ================================================================ Heat
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerHeat(array<int> ids, array<IEntity> players)
	{
		foreach (int i, IEntity ent : players)
		{
			float noise = GetPlayerBackground(ids[i]).m_fNoiseMult;
			float heat = m_fHeatPresence * noise;
			if (RZ_Util.IsInVehicle(ent))
				heat += m_fHeatVehicle * noise;

			m_Heat.AddHeat(ent.GetOrigin(), heat);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Players near the horde's target are warned, and so are Radio Operators anywhere.
	void NotifyHorde(vector target)
	{
		int idx = FindNearestSettlement(target);
		string where = RZ_SettlementScanner.GridReference(target);
		RZ_Settlement s = GetSettlement(idx);
		if (s)
			where = s.m_sName;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), target) < 800)
				NotifyPlayer(ids[i], "Horde", "Something big heard you. A horde is moving in.");
			else if (GetPlayerBackground(ids[i]).m_bRadioIntel)
				NotifyPlayer(ids[i], "Radio chatter", "Survivors report a horde moving toward " + where + ".");
		}
	}

	// ================================================================ Zombies, cure, rewards
	//------------------------------------------------------------------------------------------------
	void OnZombiesKilled(int settlementIdx, int missionId, bool special, vector pos, int count)
	{
		RZ_Settlement s = GetSettlement(settlementIdx);
		if (s)
			s.AddInfection(-m_ZombieSettings.m_fInfectionDropPerKill * count);

		if (special)
			SpawnSamples(pos, 1);

		m_Missions.OnZombiesKilled(settlementIdx, missionId, special, pos, count);
	}

	//------------------------------------------------------------------------------------------------
	void DeliverSamples(int playerId, int count)
	{
		m_Cure.AddSamples(playerId, count, m_fResearchPerSample);
	}

	//------------------------------------------------------------------------------------------------
	void SpawnSamples(vector pos, int count)
	{
		for (int i = 0; i < count; i++)
		{
			RZ_Util.SpawnPrefab(m_sSamplePrefab, RZ_Util.RandomPointInRing(pos, 0.3, 1.5) + Vector(0, 0.3, 0));
		}
	}

	//------------------------------------------------------------------------------------------------
	void SpawnAntidote()
	{
		if (!m_bHasLab)
			return;

		RZ_Util.SpawnPrefab(m_sAntidotePrefab, RZ_Util.RandomPointInRing(m_vLabPos, 2, 5) + Vector(0, 0.3, 0));
		NotifyAll("Institute", "A fresh antidote dose is ready at the lab.");
	}

	//------------------------------------------------------------------------------------------------
	void SpawnRewardCrate(vector pos)
	{
		RZ_Util.SpawnPrefab(m_sRewardCratePrefab, RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
	}

	//------------------------------------------------------------------------------------------------
	void OnCureChanged()
	{
		UpdateReplicated();
	}

	//------------------------------------------------------------------------------------------------
	void OnStageChanged(RZ_EOutbreakStage stage)
	{
		UpdateReplicated();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateReplicated()
	{
		m_fReplCureProgress = m_Cure.m_fProgress;
		m_bReplCured = m_Cure.IsCured();
		m_iReplStage = m_Director.GetStage();
		Replication.BumpMe();
	}

	// ================================================================ Lab and spawn points
	//------------------------------------------------------------------------------------------------
	protected void SetupLab()
	{
		RZ_LabComponent placed = RZ_LabComponent.GetFirst();
		if (placed)
		{
			m_Lab = placed.GetOwner();
			m_vLabPos = m_Lab.GetOrigin();
			m_bHasLab = true;
			return;
		}

		// Saved position from a previous run
		if (!m_bHasLab)
		{
			int idx = -1;
			foreach (int i, RZ_Settlement s : m_aSettlements)
			{
				if (s.m_bHasLab)
					idx = i;
			}

			if (idx < 0)
				idx = ChooseLabSettlement();

			RZ_Settlement labTown = GetSettlement(idx);
			if (!labTown)
				return;

			labTown.m_bHasLab = true;
			m_vLabPos = RZ_Util.RandomPointInRing(labTown.m_vCenter, 20, Math.Max(30, labTown.m_fRadius * 0.4));
			m_bHasLab = true;
		}

		m_vLabPos = RZ_Util.GroundPos(m_vLabPos);
		m_Lab = RZ_Util.SpawnPrefab(m_sLabPrefab, m_vLabPos);
		if (!m_Lab)
			RZ_Log.Warn("No lab prefab set. Samples can't be delivered until one is configured or placed.");
	}

	//------------------------------------------------------------------------------------------------
	//! Prefer a town (size 2). It's busy enough to be dangerous but not a city death-trap.
	protected int ChooseLabSettlement()
	{
		array<int> towns = {};
		foreach (int i, RZ_Settlement s : m_aSettlements)
		{
			if (s.m_iSize == 2)
				towns.Insert(i);
		}

		if (!towns.IsEmpty())
			return towns.GetRandomElement();

		if (m_aSettlements.IsEmpty())
			return -1;

		return Math.RandomInt(0, m_aSettlements.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn points sit in random, not-too-infected towns and move over time, so
	//! the deploy menu offers different starting places every life.
	protected void RefreshSpawnPoints(bool all)
	{
		if (m_sSpawnPointPrefab.IsEmpty() || m_aSettlements.IsEmpty())
			return;

		if (all)
		{
			foreach (IEntity e : m_aSpawnPoints)
			{
				RZ_Util.DeleteUnit(e);
			}

			m_aSpawnPoints.Clear();
		}
		else if (!m_aSpawnPoints.IsEmpty())
		{
			RZ_Util.DeleteUnit(m_aSpawnPoints[0]);
			m_aSpawnPoints.Remove(0);
		}

		// Drop references to anything deleted elsewhere
		for (int i = m_aSpawnPoints.Count() - 1; i >= 0; i--)
		{
			if (!m_aSpawnPoints[i])
				m_aSpawnPoints.Remove(i);
		}

		array<int> candidates = {};
		foreach (int idx, RZ_Settlement s : m_aSettlements)
		{
			if (s.m_fInfection < 70)
				candidates.Insert(idx);
		}

		if (candidates.IsEmpty())
		{
			foreach (int idx2, RZ_Settlement s2 : m_aSettlements)
			{
				candidates.Insert(idx2);
			}
		}

		int guard = 0;
		while (m_aSpawnPoints.Count() < m_iSpawnPointCount && guard < 50)
		{
			guard++;
			RZ_Settlement town = m_aSettlements[candidates.GetRandomElement()];
			vector pos = RZ_Util.RandomPointInRing(town.m_vCenter, 0, town.m_fRadius * 0.8);
			if (RZ_Util.IsWater(pos))
				continue;

			IEntity ent = RZ_Util.SpawnPrefab(m_sSpawnPointPrefab, pos, Math.RandomFloat(0, 360));
			if (!ent)
				return;

			SCR_SpawnPoint sp = SCR_SpawnPoint.Cast(ent);
			if (sp)
			{
				sp.SetFactionKey(m_sPlayerFactionKey); // VERIFY
				sp.SetSpawnPointName(town.m_sName); // VERIFY
			}

			m_aSpawnPoints.Insert(ent);
		}
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	protected string GetWorldId()
	{
		return GetGame().GetWorldFile(); // VERIFY
	}

	//------------------------------------------------------------------------------------------------
	protected bool LoadState()
	{
		RZ_SaveData data = RZ_Persistence.Load();
		if (!data)
			return false;

		if (data.world != GetWorldId())
		{
			RZ_Log.Info("Saved state is from another map, starting a fresh outbreak.");
			return false;
		}

		foreach (RZ_SettlementSave ss : data.settlements)
		{
			RZ_Settlement s = new RZ_Settlement();
			s.m_sName = ss.name;
			s.m_vCenter = Vector(ss.x, ss.y, ss.z);
			s.m_fRadius = ss.radius;
			s.m_iSize = ss.size;
			s.m_fInfection = ss.infection;
			s.m_bHasLab = ss.lab;
			m_aSettlements.Insert(s);
		}

		foreach (RZ_PlayerProfile p : data.profiles)
		{
			m_mProfiles.Set(p.m_sIdentity, p);
		}

		m_Director.Restore(data.stage, data.stageMinutes);
		m_Cure.Restore(data.cureProgress, data.cureMilestone, data.cured);

		m_bHasLab = data.hasLab;
		m_vLabPos = Vector(data.labX, 0, data.labZ);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void SaveState()
	{
		if (!m_Director)
			return;

		RZ_SaveData data = new RZ_SaveData();
		data.world = GetWorldId();
		data.stage = m_Director.GetStage();
		data.stageMinutes = m_Director.GetStageMinutes();
		data.cureProgress = m_Cure.m_fProgress;
		data.cureMilestone = m_Cure.m_iMilestone;
		data.cured = m_Cure.IsCured();
		data.hasLab = m_bHasLab;
		data.labX = m_vLabPos[0];
		data.labZ = m_vLabPos[2];

		foreach (RZ_Settlement s : m_aSettlements)
		{
			RZ_SettlementSave ss = new RZ_SettlementSave();
			ss.name = s.m_sName;
			ss.x = s.m_vCenter[0];
			ss.y = s.m_vCenter[1];
			ss.z = s.m_vCenter[2];
			ss.radius = s.m_fRadius;
			ss.size = s.m_iSize;
			ss.infection = s.m_fInfection;
			ss.lab = s.m_bHasLab;
			data.settlements.Insert(ss);
		}

		foreach (string ident, RZ_PlayerProfile p : m_mProfiles)
		{
			data.profiles.Insert(p);
		}

		RZ_Persistence.Save(data);
	}

	// ================================================================ Notifications
	//------------------------------------------------------------------------------------------------
	void NotifyAll(string title, string text)
	{
		RpcDo_Notify(-1, title, text);
		Rpc(RpcDo_Notify, -1, title, text);
	}

	//------------------------------------------------------------------------------------------------
	void NotifyPlayer(int playerId, string title, string text)
	{
		RpcDo_Notify(playerId, title, text);
		Rpc(RpcDo_Notify, playerId, title, text);
	}

	//------------------------------------------------------------------------------------------------
	//! Broadcast, with each client filtering for itself. Fine at this message rate.
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Notify(int targetPlayerId, string title, string text)
	{
		if (System.IsConsoleApp())
			return; // dedicated server has no screen

		if (targetPlayerId != -1 && SCR_PlayerController.GetLocalPlayerId() != targetPlayerId)
			return;

		QueueHint(title, text);
	}

	// ---------------------------------------------------------------- Client hint queue
	//! The hint widget shows one hint at a time, so messages that arrive together
	//! (e.g. the three on spawn) are queued and shown one after another.
	protected ref array<string> m_aHintTitles = {};
	protected ref array<string> m_aHintTexts = {};
	protected bool m_bHintShowing;

	protected static const int HINT_SECONDS = 9;
	protected static const int HINT_QUEUE_MAX = 6;

	//------------------------------------------------------------------------------------------------
	protected void QueueHint(string title, string text)
	{
		// Under heavy traffic, drop the oldest messages rather than lag minutes behind
		if (m_aHintTitles.Count() >= HINT_QUEUE_MAX)
		{
			m_aHintTitles.Remove(0);
			m_aHintTexts.Remove(0);
		}

		m_aHintTitles.Insert(title);
		m_aHintTexts.Insert(text);
		if (!m_bHintShowing)
			ShowNextHint();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowNextHint()
	{
		if (m_aHintTitles.IsEmpty())
		{
			m_bHintShowing = false;
			return;
		}

		m_bHintShowing = true;
		string title = m_aHintTitles[0];
		string text = m_aHintTexts[0];
		m_aHintTitles.Remove(0);
		m_aHintTexts.Remove(0);

		SCR_HintManagerComponent.ShowCustomHint(text, title, HINT_SECONDS); // VERIFY
		GetGame().GetCallqueue().CallLater(ShowNextHint, HINT_SECONDS * 1000, false);
	}
}
