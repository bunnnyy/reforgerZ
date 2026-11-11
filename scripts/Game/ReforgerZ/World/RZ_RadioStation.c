//! The radio station (docs/DESIGN.md, "Time and environment"). One broadcast point per
//! map: a placed RZ_RadioStationMarkerEntity, else a real radio mast or antenna found by
//! prefab name near a town, else the highest ground near a mid-sized town. A player takes it
//! over by staying at the console for a short while with no infected close, then can send
//! one of a few preset messages to everyone on the server, filled in with real places.
//!
//! Cosmetic and social only: no Fame, no Favor, nothing changes in the world. Messages are
//! signed with the sender's name. One broadcast per 10 minutes server-wide (5 while the
//! power grid's region includes the station). The console exists only while a player is near.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > World > Radio station.
[BaseContainerProps()]
class RZ_RadioStationSettings
{
	[Attribute("1", desc: "A radio station players can take over and broadcast from (cosmetic and social only, docs/DESIGN.md, Time and environment)")]
	bool m_bEnabled;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Console: a radio set or desk with the generic puzzle object components (RZ_PuzzleObjectComponent, RZ_ClueComponent, RZ_ReadClueAction, 4 RZ_PuzzleChoiceAction). Empty = Puzzles > Radio mast, then the generic object", "et")]
	ResourceName m_sConsolePrefab;

	[Attribute("30", desc: "Seconds a player must stay at the console (no infected within 15 m) to take it over")]
	float m_fTakeoverSeconds;

	[Attribute("10", desc: "Real minutes between broadcasts, server-wide (halved while the station has power)")]
	float m_fCooldownMinutes;
}

//------------------------------------------------------------------------------------------------
class RZ_RadioStationSave
{
	float x;
	float z;
	string name;
}

//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "ReforgerZ", description: "The radio station players can take over. If one is placed, the station isn't picked automatically.")]
class RZ_RadioStationMarkerEntityClass : GenericEntityClass
{
}

//! Optional. Put one where the broadcast point is (a radio building, a mast on a hill).
class RZ_RadioStationMarkerEntity : GenericEntity
{
	[Attribute("", desc: "Name (empty = \"the radio station near <nearest town>\")")]
	protected string m_sName;

	protected static ref array<RZ_RadioStationMarkerEntity> s_aAll;

	//------------------------------------------------------------------------------------------------
	static array<RZ_RadioStationMarkerEntity> GetAll()
	{
		if (!s_aAll)
			s_aAll = new array<RZ_RadioStationMarkerEntity>();

		return s_aAll;
	}

	string GetMarkerName() { return m_sName; }

	//------------------------------------------------------------------------------------------------
	void RZ_RadioStationMarkerEntity(IEntitySource src, IEntity parent)
	{
		GetAll().Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_RadioStationMarkerEntity()
	{
		if (s_aAll)
			s_aAll.RemoveItem(this);
	}
}

//------------------------------------------------------------------------------------------------
class RZ_RadioStation
{
	static const int TAG = -8;				// RZ_PuzzleObjectComponent / RZ_ClueComponent mission id

	protected static const float VISIBLE_RADIUS = 300;
	protected static const float DESPAWN_MARGIN = 50;
	protected static const float CONSOLE_RADIUS = 25;
	protected static const float LOSE_RADIUS = 150;
	protected static const float ZOMBIE_RADIUS = 15;

	protected RZ_GameModeComponent m_GM;
	protected RZ_RadioStationSettings m_Settings;
	protected ref RZ_RadioStationSave m_Restored;
	protected bool m_bActive;
	protected vector m_vPos;
	protected string m_sName;
	protected int m_iHolder = -1;			// the player who holds the station
	protected int m_iClaimant = -1;			// taking it over
	protected float m_fClaimLeft;
	protected float m_fClock;
	protected float m_fNextBroadcast;
	protected IEntity m_Body;
	protected RZ_PuzzleObjectComponent m_Comp;

	//------------------------------------------------------------------------------------------------
	void RZ_RadioStation(RZ_GameModeComponent gm, RZ_RadioStationSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
	}

	//------------------------------------------------------------------------------------------------
	bool IsActive() { return m_bActive; }
	string GetName() { return m_sName; }
	vector GetPosition() { return m_vPos; }

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_RadioStationSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_RadioStationSave Save()
	{
		RZ_RadioStationSave data = new RZ_RadioStationSave();
		data.x = m_vPos[0];
		data.z = m_vPos[2];
		data.name = m_sName;
		return data;
	}

	// ================================================================ Setup
	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!m_Settings.m_bEnabled || m_GM.GetSettlements().IsEmpty())
		{
			RZ_Log.Info("Radio station is off.");
			return;
		}

		string from = "picked";
		array<RZ_RadioStationMarkerEntity> markers = RZ_RadioStationMarkerEntity.GetAll();
		if (!markers.IsEmpty() && markers[0])
		{
			m_vPos = RZ_Util.GroundPos(markers[0].GetOrigin());
			m_sName = markers[0].GetMarkerName();
			from = "marker";
		}
		else if (m_Restored && m_Restored.name != "")
		{
			m_vPos = RZ_Util.GroundPos(Vector(m_Restored.x, 0, m_Restored.z));
			m_sName = m_Restored.name;
			from = "save";
		}
		else
		{
			PickSite();
		}

		m_Restored = null;
		if (m_sName.IsEmpty())
			m_sName = "the radio station near " + NearTown(m_vPos);

		m_bActive = true;
		RZ_Log.Info(string.Format("Radio station: %1 at grid %2 (%3).", m_sName, RZ_SettlementScanner.GridReference(m_vPos), from));
	}

	//------------------------------------------------------------------------------------------------
	//! A real mast or antenna near a town, else the highest ground near a mid-sized town.
	protected void PickSite()
	{
		array<ref RZ_Settlement> list = m_GM.GetSettlements();
		array<string> kinds = {"radio"};
		RZ_Perimeter perimeter = m_GM.GetPerimeter();
		foreach (RZ_Settlement s : list)
		{
			RZ_Landmark l = RZ_LandmarkFinder.FindNearest(s.m_vCenter, s.m_fRadius + 300, kinds);
			if (!l || (perimeter && perimeter.IsNearLine(l.m_vPos)))
				continue;

			m_vPos = RZ_Util.GroundPos(l.m_vPos);
			m_sName = string.Format("%1 near %2", l.m_sName, s.m_sName);
			return;
		}

		RZ_Settlement town = list[0];
		foreach (RZ_Settlement candidate : list)
		{
			if (candidate.m_iSize == 2)
			{
				town = candidate;
				break;
			}
		}

		m_vPos = RZ_Mission.HighPointNear(town.m_vCenter, town.m_fRadius + 100, town.m_fRadius + 600);
		m_sName = "the relay mast above " + town.m_sName;
	}

	//------------------------------------------------------------------------------------------------
	protected string NearTown(vector pos)
	{
		RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(pos));
		if (s)
			return s.m_sName;

		return RZ_SettlementScanner.GridReference(pos);
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		if (!m_bActive)
			return;

		m_fClock += dt;
		UpdateBody(players);
		TickHolder();
		TickClaim(dt);
	}

	//------------------------------------------------------------------------------------------------
	//! The holder keeps the station until they walk away, die or leave.
	protected void TickHolder()
	{
		if (m_iHolder <= 0)
			return;

		IEntity holder = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iHolder);
		if (holder && RZ_Util.IsCharacterAlive(holder) && RZ_Util.DistXZ(holder.GetOrigin(), m_vPos) <= LOSE_RADIUS)
			return;

		m_iHolder = -1;
		UpdateLabels();
	}

	//------------------------------------------------------------------------------------------------
	protected void TickClaim(float dt)
	{
		if (m_iClaimant <= 0)
			return;

		IEntity claimant = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iClaimant);
		if (!claimant || RZ_Util.DistXZ(claimant.GetOrigin(), m_vPos) > CONSOLE_RADIUS)
		{
			if (claimant)
				m_GM.NotifyPlayer(m_iClaimant, "Radio station", "You walked away from the console. Nobody's on the air.");

			m_iClaimant = -1;
			return;
		}

		if (m_GM.GetSpawner().IsZombieNear(m_vPos, ZOMBIE_RADIUS))
		{
			m_fClaimLeft = m_Settings.m_fTakeoverSeconds;
			return;
		}

		m_fClaimLeft = m_fClaimLeft - dt;
		if (m_fClaimLeft > 0)
			return;

		m_iHolder = m_iClaimant;
		m_iClaimant = -1;
		UpdateLabels();
		m_GM.NotifyPlayer(m_iHolder, "Radio station", "The set warms up. The needle finds the band everyone listens to. The station is yours while you stay. Pick a message.");
	}

	// ================================================================ The console
	//------------------------------------------------------------------------------------------------
	protected ResourceName ConsolePrefab()
	{
		if (!m_Settings.m_sConsolePrefab.IsEmpty())
			return m_Settings.m_sConsolePrefab;

		return m_GM.GetPuzzlePrefab(RZ_EPuzzleProp.RADIO_MAST);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateBody(array<IEntity> players)
	{
		float nearest = RZ_Util.NearestDistance(m_vPos, players);
		if (m_Body && nearest > VISIBLE_RADIUS + DESPAWN_MARGIN)
		{
			RZ_Util.DeleteUnit(m_Body);
			m_Body = null;
			m_Comp = null;
			return;
		}

		if (m_Body || nearest > VISIBLE_RADIUS)
			return;

		m_Body = RZ_Util.SpawnPrefab(ConsolePrefab(), m_vPos, Math.RandomFloat(0, 360));
		if (!m_Body)
			return;

		m_Comp = RZ_PuzzleObjectComponent.Tag(m_Body, TAG, 0, "", ChoiceLabels());
		RZ_ClueComponent.Tag(m_Body, TAG, 0, "Look at the set");
	}

	//------------------------------------------------------------------------------------------------
	protected string ChoiceLabels()
	{
		if (m_iHolder > 0)
			return "Broadcast: horde warning|Broadcast: food for a safehouse|Broadcast: meet here|Broadcast: still here";

		return "Take over the station";
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateLabels()
	{
		if (m_Comp)
			m_Comp.SetLabels("", ChoiceLabels());
	}

	//------------------------------------------------------------------------------------------------
	protected float CooldownSeconds()
	{
		float secs = Math.Max(1, m_Settings.m_fCooldownMinutes) * 60;
		if (RZ_PowerGrid.IsPoweredAt(m_vPos))
			secs = secs * 0.5;

		return secs;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction.
	void HandleRead(int playerId)
	{
		string who = "nobody";
		if (m_iHolder > 0)
			who = RZ_Util.PlayerName(m_iHolder);

		string power = "It runs on a battery bank that's seen better days.";
		if (RZ_PowerGrid.IsPoweredAt(m_vPos))
			power = "There's mains power: the set hums, clear and strong.";

		string wait = "It's ready to send.";
		if (m_fClock < m_fNextBroadcast)
			wait = string.Format("The transmitter is cooling down: about %1 minutes.", RZ_Util.RoundToInt((m_fNextBroadcast - m_fClock) / 60 + 0.5));

		m_GM.NotifyPlayer(playerId, "Radio station", string.Format("A transmitter that still reaches every radio in the Zone. On the air: %1. %2 %3", who, power, wait));
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleChoiceAction on the console.
	void HandleChoice(int playerId, int option, IEntity owner, IEntity user)
	{
		if (!user)
			return;

		if (m_iHolder <= 0)
		{
			if (option != 0)
				return;

			if (m_iClaimant > 0 && m_iClaimant != playerId)
			{
				m_GM.NotifyPlayer(playerId, "Radio station", RZ_Util.PlayerName(m_iClaimant) + " is already at the dials.");
				return;
			}

			m_iClaimant = playerId;
			m_fClaimLeft = m_Settings.m_fTakeoverSeconds;
			m_GM.NotifyPlayer(playerId, "Radio station", string.Format("You start bringing the set up. Stay at the console for %1 seconds, with none of them close.", RZ_Util.RoundToInt(m_fClaimLeft)));
			return;
		}

		if (playerId != m_iHolder)
		{
			m_GM.NotifyPlayer(playerId, "Radio station", RZ_Util.PlayerName(m_iHolder) + " has the station. Ask them nicely, or wait for them to leave.");
			return;
		}

		if (m_fClock < m_fNextBroadcast)
		{
			m_GM.NotifyPlayer(playerId, "Radio station", string.Format("The transmitter is still too hot. About %1 minutes.", RZ_Util.RoundToInt((m_fNextBroadcast - m_fClock) / 60 + 0.5)));
			return;
		}

		string text = Message(option, playerId, user.GetOrigin());
		if (text.IsEmpty())
			return;

		m_fNextBroadcast = m_fClock + CooldownSeconds();
		m_GM.NotifyAll("Radio: " + m_sName, text);
		RZ_Log.Info(string.Format("Radio station: broadcast %1 by player %2.", option, playerId));
		// The epilogue's reel keeps each player's latest broadcast from before the cure
		if (RZ_Epilogue.Get())
			RZ_Epilogue.Get().OnBroadcast(playerId, text);
	}

	//------------------------------------------------------------------------------------------------
	//! The preset messages, filled in with real places. Signed with the sender's name.
	protected string Message(int option, int playerId, vector from)
	{
		string name = RZ_Util.PlayerName(playerId);
		switch (option)
		{
			case 0:
			{
				vector hot;
				float heat;
				string where = "anywhere loud";
				if (m_GM.GetHeat().GetHottest(hot, heat) && heat >= 20)
					where = NearTown(hot);

				return string.Format("\"This is %1. There's a horde moving around %2. Stay clear of it, and keep it down if you're close.\"", name, where);
			}
			case 1:
			{
				RZ_Safehouses houses = m_GM.GetSafehouses();
				RZ_Safehouse hungry;
				if (houses)
				{
					for (int i = 0; i < houses.Count(); i++)
					{
						RZ_Safehouse h = houses.Get(i);
						if (!hungry || h.m_iFood < hungry.m_iFood)
							hungry = h;
					}
				}

				if (!hungry)
					return string.Format("\"This is %1. If you've got food to spare, somebody near you needs it more. Share it.\"", name);

				return string.Format("\"This is %1. The safehouse in %2 is running low. If you've got tins, that's where they should go.\"", name, hungry.m_sName);
			}
			case 2:
			{
				return string.Format("\"This is %1, at %2, grid %3. We're friendly. Come quiet, come unarmed if you can.\"", name, m_sName, RZ_SettlementScanner.GridReference(m_vPos));
			}
			case 3:
			{
				// Processing: the "Going home" preset (the epilogue week)
				if (RZ_Epilogue.Get() && RZ_Epilogue.Get().Phase() >= 3)
					return string.Format("\"This is %1, at %2. Going home.\"", name, m_sName);

				int day = 11;
				if (m_GM.GetBroadcast())
					day = m_GM.GetBroadcast().GetDay();

				return string.Format("\"This is %1. %2, and we're still here. Whoever's listening: so are you. Keep going.\"", name, RZ_NoteText.Day(day));
			}
		}

		return "";
	}
}
