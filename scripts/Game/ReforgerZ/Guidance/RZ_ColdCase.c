//! The runner's cold case (docs/DESIGN.md, "The case"): Halcyon's sealed case, found in a dead
//! runner's pack at the end of an opening (Guidance/RZ_OpeningCase.c). A real item: picked up,
//! carried, dropped and taken off a body like any. Its tag is set when it's made and replicated for
//! the read action ("HALCYON. FIELD STATION. LIVE. KEEP COLD." and its date); how warm it is lives
//! on the server and is said when it's read. Guidance > Cold case prefab; without one, no case.

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "The runner's cold case: its tag and how warm it is. Put it on the cold case prefab (Guidance > Cold case) with an RplComponent; add RZ_ReadCaseTagAction.")]
class RZ_ColdCaseComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class RZ_ColdCaseComponent : ScriptComponent
{
	[RplProp()]
	protected string m_sTag;		// "HALCYON. FIELD STATION. LIVE. KEEP COLD." and its date

	protected float m_fWarm;		// server only: 0 cold, 1 warm (Guidance/RZ_OpeningCase.c warms it)
	protected int m_iFinder;		// server only: the player whose opening found it, 0 none

	//------------------------------------------------------------------------------------------------
	//! Server only.
	void SetTag(string tag, int finder)
	{
		m_sTag = tag;
		m_iFinder = finder;
		Replication.BumpMe(); // needs the item's RplComponent
	}

	string GetTag() { return m_sTag; }
	int GetFinder() { return m_iFinder; }
	float GetWarm() { return m_fWarm; }
	bool IsWarm() { return m_fWarm >= 1; }

	//------------------------------------------------------------------------------------------------
	//! Server only: dt seconds at `rate` (1 a cold case warms in Warm minutes; half at night).
	void Warm(float amount)
	{
		m_fWarm = Math.Clamp(m_fWarm + amount, 0, 1);
	}

	//------------------------------------------------------------------------------------------------
	static RZ_ColdCaseComponent Of(IEntity e)
	{
		if (!e)
			return null;

		return RZ_ColdCaseComponent.Cast(e.FindComponent(RZ_ColdCaseComponent));
	}

	//------------------------------------------------------------------------------------------------
	//! The case this character carries (in any of their storage), null if none.
	static IEntity CarriedBy(IEntity character)
	{
		if (!character)
			return null;

		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return null;

		array<IEntity> items = {};
		inv.GetItems(items); // VERIFY: GetItems() also finds the case in a bag inside another bag (as for keys)
		foreach (IEntity item : items)
		{
			if (Of(item))
				return item;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! What reading it says: the tag, and how it feels in the hand.
	static string ReadText(RZ_ColdCaseComponent c)
	{
		string text = string.Format("\"%1\"", c.GetTag());
		if (c.IsWarm())
			text += "\n\nThe case is warm. Whatever's in it has been out of the cold too long.";
		else if (c.GetWarm() > 0.5)
			text += "\n\nThe case is cool, not cold. It won't stay cool much longer.";
		else
			text += "\n\nThe case is cold to the touch.";

		return text;
	}
}

//------------------------------------------------------------------------------------------------
//! "Read the tag": on the cold case prefab, for a case lying on the ground or in a container. It
//! opens the tag in the reader, with how warm the case is.
class RZ_ReadCaseTagAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		outName = "Read the tag";
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

		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!gm || playerId <= 0)
			return;

		RZ_ColdCaseComponent c = RZ_ColdCaseComponent.Of(pOwnerEntity);
		if (!c || c.GetTag().IsEmpty())
		{
			gm.NotifyPlayer(playerId, "A cold case", "A sealed case. The tag's been torn off.");
			return;
		}

		RZ_Reader.Show(gm, playerId, "The runner's case", "A luggage tag on the handle, in marker", RZ_ColdCaseComponent.ReadText(c));
	}
}
