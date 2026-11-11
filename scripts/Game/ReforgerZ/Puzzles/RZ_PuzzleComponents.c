//! Puzzle building blocks: clues players read, and objects players answer with.
//!
//! Text never leaves the server except through gm.NotifyPlayer(): a clue entity
//! only knows its mission id and clue index, and the mission looks the text up
//! when someone reads it. Action labels (e.g. "Seal the well", "Accuse Mirek")
//! are the one exception: they're short and are replicated on the object so
//! one generic prefab can serve every puzzle. If that replication doesn't work
//! in your game version, the actions fall back to the labels set on the prefab.

//------------------------------------------------------------------------------------------------
//! Splits "a|b|c" into slots without relying on string.Split() behaviour for empty parts.
class RZ_PuzzleLabels
{
	//------------------------------------------------------------------------------------------------
	static void Parse(string text, notnull array<string> result)
	{
		result.Clear();
		if (text.IsEmpty())
			return;

		string part = "";
		int len = text.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = text.Substring(i, 1);
			if (ch == "|")
			{
				result.Insert(part);
				part = "";
			}
			else
			{
				part += ch;
			}
		}

		result.Insert(part);
	}

	//------------------------------------------------------------------------------------------------
	static string Join(array<string> parts)
	{
		string text = "";
		foreach (int i, string p : parts)
		{
			if (i > 0)
				text += "|";

			text += p;
		}

		return text;
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A readable clue (note, plaque, dog tag, wreck panel). Add RZ_ReadClueAction to the ActionsManagerComponent. The mission sets its id and clue index when it spawns or tags the entity.")]
class RZ_ClueComponentClass : ScriptComponentClass
{
}

//! Put this on a note prefab, and also on props a mission may tag as clues
//! (helicopter wreck, body, plaque, civilian). Needs an RplComponent on the prefab
//! for the replicated read label.
class RZ_ClueComponent : ScriptComponent
{
	static const string NO_READ = "-";	// a read label that hides the read action (a spot with only a choice)

	protected int m_iMissionId = -1;
	protected int m_iClueIndex = -1;

	[RplProp()]
	protected string m_sReadLabel;

	//------------------------------------------------------------------------------------------------
	//! Server only.
	void SetClue(int missionId, int clueIndex, string readLabel)
	{
		m_iMissionId = missionId;
		m_iClueIndex = clueIndex;
		m_sReadLabel = readLabel;
		Replication.BumpMe(); // needs an RplComponent on the prefab
	}

	int GetMissionId() { return m_iMissionId; }
	int GetClueIndex() { return m_iClueIndex; }
	string GetReadLabel() { return m_sReadLabel; }

	//------------------------------------------------------------------------------------------------
	static bool Tag(IEntity e, int missionId, int clueIndex, string readLabel)
	{
		if (!e)
			return false;

		RZ_ClueComponent comp = RZ_ClueComponent.Cast(e.FindComponent(RZ_ClueComponent));
		if (!comp)
			return false;

		comp.SetClue(missionId, clueIndex, readLabel);
		return true;
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A puzzle object (keypad, lever, bell, well, fuse box, terminal, scale, choice board). Add RZ_KeypadAction / RZ_PuzzleUseAction / RZ_PuzzleChoiceAction. Needs an RplComponent.")]
class RZ_PuzzleObjectComponentClass : ScriptComponentClass
{
}

//! Routes answers to the mission that owns the object. The mission can label each
//! action slot; slots without a label are hidden, so one prefab with four use and
//! four choice actions serves every puzzle.
class RZ_PuzzleObjectComponent : ScriptComponent
{
	protected int m_iMissionId = -1;
	protected int m_iObjectIndex = -1;

	[RplProp(onRplName: "OnLabelsReplicated")]
	protected string m_sUseLabels;

	[RplProp(onRplName: "OnLabelsReplicated")]
	protected string m_sChoiceLabels;

	[RplProp()]
	protected bool m_bLabelled;

	[RplProp()]
	protected string m_sDisplay;	// keypad display ("4 7 _")

	protected ref array<string> m_aUse = {};
	protected ref array<string> m_aChoice = {};
	protected string m_sParsedUse;
	protected string m_sParsedChoice;

	//------------------------------------------------------------------------------------------------
	//! Server only. Labels are "|" separated, one per action slot (value / option index).
	void Setup(int missionId, int objectIndex, string useLabels, string choiceLabels)
	{
		m_iMissionId = missionId;
		m_iObjectIndex = objectIndex;
		m_sUseLabels = useLabels;
		m_sChoiceLabels = choiceLabels;
		m_bLabelled = !useLabels.IsEmpty() || !choiceLabels.IsEmpty();
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Server only. Change the labels later (e.g. after a cellar is opened).
	void SetLabels(string useLabels, string choiceLabels)
	{
		Setup(m_iMissionId, m_iObjectIndex, useLabels, choiceLabels);
	}

	//------------------------------------------------------------------------------------------------
	void SetDisplay(string display)
	{
		m_sDisplay = display;
		Replication.BumpMe();
	}

	int GetMissionId() { return m_iMissionId; }
	int GetObjectIndex() { return m_iObjectIndex; }
	string GetDisplay() { return m_sDisplay; }

	//------------------------------------------------------------------------------------------------
	protected void OnLabelsReplicated()
	{
		m_sParsedUse = "";
		m_sParsedChoice = "";
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureParsed()
	{
		if (m_sParsedUse != m_sUseLabels || m_aUse.IsEmpty())
		{
			RZ_PuzzleLabels.Parse(m_sUseLabels, m_aUse);
			m_sParsedUse = m_sUseLabels;
		}

		if (m_sParsedChoice != m_sChoiceLabels || m_aChoice.IsEmpty())
		{
			RZ_PuzzleLabels.Parse(m_sChoiceLabels, m_aChoice);
			m_sParsedChoice = m_sChoiceLabels;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Label for an action slot, or "" if the mission didn't label it.
	string GetSlotLabel(bool choice, int slot)
	{
		EnsureParsed();
		array<string> labels = m_aUse;
		if (choice)
			labels = m_aChoice;

		if (slot < 0 || slot >= labels.Count())
			return "";

		return labels[slot];
	}

	//------------------------------------------------------------------------------------------------
	//! Hidden only when the mission labelled this object and left this slot empty.
	//! If labels never arrived (replication not working), everything stays visible.
	bool IsSlotHidden(bool choice, int slot)
	{
		if (!m_bLabelled)
			return false;

		return GetSlotLabel(choice, slot).IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	static RZ_PuzzleObjectComponent Tag(IEntity e, int missionId, int objectIndex, string useLabels, string choiceLabels)
	{
		if (!e)
			return null;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(e.FindComponent(RZ_PuzzleObjectComponent));
		if (comp)
			comp.Setup(missionId, objectIndex, useLabels, choiceLabels);

		return comp;
	}
}

//------------------------------------------------------------------------------------------------
//! Shared lookups for the puzzle actions (server side).
class RZ_PuzzleActionHelper
{
	//------------------------------------------------------------------------------------------------
	static int GetPlayerId(IEntity user)
	{
		if (!user)
			return -1;

		return GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(user);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_PuzzleMission FindMission(RZ_GameModeComponent gm, int missionId)
	{
		if (!gm || !gm.GetMissions() || missionId < 0)
			return null;

		RZ_PuzzleMission m = RZ_PuzzleMission.Cast(gm.GetMissions().FindById(missionId));
		if (!m || m.m_eState != RZ_EMissionState.ACTIVE)
			return null;

		return m;
	}

	//------------------------------------------------------------------------------------------------
	//! Wanderers (RZ_Wanderers.TAG), safehouse occupants (RZ_Safehouses.TAG_OCCUPANT), faction
	//! leaders (RZ_Leaders.TAG), named outpost people (RZ_Territory.TAG) and the Long Table's people (RZ_LongTable.TAG) reuse the puzzle civilian; guidance clues (RZ_GuidanceWorld.TAG) the note;
	//! the power station (RZ_PowerGrid.TAG), radio station (RZ_RadioStation.TAG) and world interaction spots (RZ_Interactions.TAG) the generic object.
	//! Their choice actions come here instead of to a mission.
	//! True if `owner` is one of them (and the choice was handled).
	static bool RouteChoice(RZ_GameModeComponent gm, IEntity owner, int playerId, int option, IEntity user)
	{
		if (!gm || !owner)
			return false;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp)
			return false;

		// A person whose slots are an errand's ways right now (Missions/Errands/RZ_Errands.c)
		if (comp.GetMissionId() < 0 && gm.GetErrands() && gm.GetErrands().HandleEngagedChoice(owner, playerId, option, user))
			return true;

		if (comp.GetMissionId() == RZ_Wanderers.TAG)
		{
			if (gm.GetWanderers())
				gm.GetWanderers().HandleChoice(playerId, comp.GetObjectIndex(), option, owner, user);

			return true;
		}

		// A safehouse's named person (World/RZ_NamedPeople.c)
		if (comp.GetMissionId() == RZ_NamedPeople.TAG_HOUSE)
		{
			if (gm.GetErrands())
				gm.GetErrands().HandleHouseTalk(playerId, comp.GetObjectIndex(), option, owner, user);

			return true;
		}

		if (comp.GetMissionId() == RZ_Safehouses.TAG_OCCUPANT)
		{
			if (gm.GetSafehouses())
				gm.GetSafehouses().HandleOccupantTalk(playerId, comp.GetObjectIndex());

			return true;
		}

		if (comp.GetMissionId() == RZ_Leaders.TAG)
		{
			if (gm.GetLeaders())
				gm.GetLeaders().HandleChoice(playerId, comp.GetObjectIndex(), option, owner, user);

			return true;
		}

		if (comp.GetMissionId() == RZ_Territory.TAG)
		{
			// "Anything need doing?" is the errands' (slot 1)
			if (gm.GetErrands() && gm.GetErrands().HandleHoldingChoice(playerId, comp.GetObjectIndex(), option, owner, user))
				return true;

			if (gm.GetTerritory())
				gm.GetTerritory().HandlePersonTalk(playerId, comp.GetObjectIndex());

			return true;
		}

		// A road post's speaker at the barrier (Dialogue > Trees)
		if (comp.GetMissionId() == RZ_Checkpoints.TAG_POST)
		{
			if (gm.GetCheckpoints())
				gm.GetCheckpoints().HandleSpeakerTalk(playerId, comp.GetObjectIndex());

			return true;
		}

		// The Long Table's people: eat, trade a jar, declare a bite; the host on the road
		if (comp.GetMissionId() == RZ_LongTable.TAG)
		{
			if (gm.GetLongTable())
				gm.GetLongTable().HandleChoice(playerId, comp.GetObjectIndex(), option, owner, user);

			return true;
		}

		// Guidance clues hide their choice slots; nothing to do if one is used anyway
		if (comp.GetMissionId() == RZ_GuidanceWorld.TAG)
			return true;

		// The power station's control object and the radio station's console
		if (comp.GetMissionId() == RZ_PowerGrid.TAG)
		{
			if (gm.GetPowerGrid())
				gm.GetPowerGrid().HandleChoice(playerId, option, owner, user);

			return true;
		}

		if (comp.GetMissionId() == RZ_RadioStation.TAG)
		{
			if (gm.GetRadioStation())
				gm.GetRadioStation().HandleChoice(playerId, option, owner, user);

			return true;
		}

		// World interactions: search spots, wrecks, workbenches, wells, bells, radios, cars
		if (comp.GetMissionId() == RZ_Interactions.TAG)
		{
			if (gm.GetInteractions())
				gm.GetInteractions().HandleChoice(playerId, comp.GetObjectIndex(), option, owner, user);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Same for RZ_PuzzleUseAction: a workbench's last four recipes. True if handled.
	static bool RouteUse(RZ_GameModeComponent gm, IEntity owner, int playerId, int value, IEntity user)
	{
		if (!gm || !owner)
			return false;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp || comp.GetMissionId() != RZ_Interactions.TAG)
			return false;

		if (gm.GetInteractions())
			gm.GetInteractions().HandleUse(playerId, comp.GetObjectIndex(), value, owner, user);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Same for "Talk to ..." (RZ_ReadClueAction).
	static bool RouteRead(RZ_GameModeComponent gm, IEntity owner, int playerId, RZ_ClueComponent comp)
	{
		if (!gm || !comp)
			return false;

		if (comp.GetMissionId() == RZ_Wanderers.TAG)
		{
			if (gm.GetWanderers())
				gm.GetWanderers().HandleTalk(playerId, comp.GetClueIndex(), owner);

			return true;
		}

		if (comp.GetMissionId() == RZ_Safehouses.TAG_OCCUPANT)
		{
			if (gm.GetSafehouses())
				gm.GetSafehouses().HandleOccupantTalk(playerId, comp.GetClueIndex());

			return true;
		}

		if (comp.GetMissionId() == RZ_Leaders.TAG)
		{
			if (gm.GetLeaders())
				gm.GetLeaders().HandleTalk(playerId, comp.GetClueIndex(), owner);

			return true;
		}

		// Named guards and occupants at outposts and strongholds
		if (comp.GetMissionId() == RZ_Territory.TAG)
		{
			if (gm.GetTerritory())
				gm.GetTerritory().HandlePersonTalk(playerId, comp.GetClueIndex());

			return true;
		}

		// A safehouse's named people (World/RZ_NamedPeople.c): their own greeting
		if (comp.GetMissionId() == RZ_NamedPeople.TAG_HOUSE)
		{
			if (gm.GetErrands())
				gm.GetErrands().HandleHouseTalk(playerId, comp.GetClueIndex(), -1, owner, null);

			return true;
		}

		// A road post's speaker at the barrier (Dialogue > Trees)
		if (comp.GetMissionId() == RZ_Checkpoints.TAG_POST)
		{
			if (gm.GetCheckpoints())
				gm.GetCheckpoints().HandleSpeakerTalk(playerId, comp.GetClueIndex());

			return true;
		}

		// The Long Table's people, and its tells on the road (a boot on the verge)
		if (comp.GetMissionId() == RZ_LongTable.TAG)
		{
			if (gm.GetLongTable())
				gm.GetLongTable().HandleRead(playerId, comp.GetClueIndex(), owner);

			return true;
		}

		// New-player guidance: spawn scenario clues and safehouse flyers
		if (comp.GetMissionId() == RZ_GuidanceWorld.TAG)
		{
			if (gm.GetGuidance())
				gm.GetGuidance().HandleRead(playerId, comp.GetClueIndex(), owner);

			return true;
		}

		if (comp.GetMissionId() == RZ_PowerGrid.TAG)
		{
			if (gm.GetPowerGrid())
				gm.GetPowerGrid().HandleRead(playerId);

			return true;
		}

		if (comp.GetMissionId() == RZ_RadioStation.TAG)
		{
			if (gm.GetRadioStation())
				gm.GetRadioStation().HandleRead(playerId);

			return true;
		}

		// World interactions: a spot ("Look at the fridge") or a readable thing a search turned up
		if (comp.GetMissionId() == RZ_Interactions.TAG || comp.GetMissionId() == RZ_Interactions.TAG_NOTE)
		{
			if (gm.GetInteractions())
				gm.GetInteractions().HandleRead(playerId, comp.GetMissionId(), comp.GetClueIndex(), owner);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! "Read" / "Talk to": what RZ_ReadClueAction does, also called by the talk screen's greeting.
	static void DoRead(RZ_GameModeComponent gm, IEntity owner, int playerId)
	{
		RZ_ClueComponent comp = RZ_ClueComponent.Cast(owner.FindComponent(RZ_ClueComponent));
		if (RouteRead(gm, owner, playerId, comp))
			return;

		RZ_PuzzleMission m;
		if (comp)
			m = FindMission(gm, comp.GetMissionId());

		if (!m)
		{
			// A person or thing an ordinary mission tagged (unarmed missions)
			if (comp && gm.GetMissions())
			{
				RZ_Mission plain = gm.GetMissions().FindById(comp.GetMissionId());
				if (plain && plain.m_eState == RZ_EMissionState.ACTIVE && plain.HandleTalk(gm, playerId, comp.GetClueIndex()))
					return;
			}

			gm.NotifyPlayer(playerId, "Nothing", "Whatever was written here has run in the rain. You can't make out a word.");
			return;
		}

		m.HandleRead(gm, playerId, comp.GetClueIndex(), owner);

		// "Notes like that usually point somewhere" (new-player guidance, once)
		if (gm.GetGuidance())
			gm.GetGuidance().OnClueRead(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A choice slot (RZ_PuzzleChoiceAction), also a reply in the talk screen.
	static void DoChoice(RZ_GameModeComponent gm, IEntity owner, int playerId, int option, IEntity user)
	{
		if (RouteChoice(gm, owner, playerId, option, user))
			return;

		int objectIndex;
		RZ_PuzzleMission m = FindForObject(gm, owner, objectIndex);
		if (m)
		{
			m.HandleChoice(gm, playerId, objectIndex, option, owner);
			return;
		}

		// A person an ordinary mission put down with choices (Survivors Trapped's family)
		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp || comp.GetMissionId() < 0 || !gm.GetMissions())
			return;

		RZ_Mission plain = gm.GetMissions().FindById(comp.GetMissionId());
		if (plain && plain.m_eState == RZ_EMissionState.ACTIVE)
			plain.HandleObjectChoice(gm, playerId, comp.GetObjectIndex(), option);
	}

	//------------------------------------------------------------------------------------------------
	//! A use slot (RZ_PuzzleUseAction), also a reply in the talk screen.
	static void DoUse(RZ_GameModeComponent gm, IEntity owner, int playerId, int value, IEntity user)
	{
		// A workbench's recipes 4-7 (world interactions)
		if (RouteUse(gm, owner, playerId, value, user))
			return;

		int objectIndex;
		RZ_PuzzleMission m = FindForObject(gm, owner, objectIndex);
		if (!m)
		{
			gm.NotifyPlayer(playerId, "Nothing happens", "Whatever this was for, it's over.");
			return;
		}

		m.HandleUse(gm, playerId, objectIndex, value, owner, user);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_PuzzleMission FindForObject(RZ_GameModeComponent gm, IEntity owner, out int objectIndex)
	{
		objectIndex = -1;
		if (!owner)
			return null;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp)
			return null;

		objectIndex = comp.GetObjectIndex();
		return FindMission(gm, comp.GetMissionId());
	}
}
