//! The status strip and locked rows, server side (docs/UI.md, "The status strip and locked rows").
//! A thing in the object menu can supply up to four readout tiles (POWER ON, FUEL ~25 MIN), one line
//! saying what would change them, a label over its read's text (LAST HEARD · BASTION), and for each
//! row it can't do now the full reason and a short tag in capitals (NEEDS POWER). The cooking screen
//! uses the same tiles (RZ_Crafting.StoveStatus).
//!
//! World interaction spots get theirs from their own systems: the house radio (RZ_SecureRadio), a
//! found generator (RZ_Traps), a stove and a well (RZ_Crafting). Any other grouped action can
//! override RZ_GroupedAction.MenuStatus (on the opener) and Locked (on a row); a row's lock is by
//! default its own CanBePerformed and reason, with a tag looked up from the reason (TagFor).
//!
//! Records: S|label|value|state|left per tile. State is STATE_*; left is the seconds the value counts
//! down on the client (0: none).

//------------------------------------------------------------------------------------------------
class RZ_StatusTile
{
	string m_sLabel;
	string m_sValue;
	int m_iState;
	int m_iLeft;
}

//------------------------------------------------------------------------------------------------
class RZ_ObjectStatus
{
	static const int STATE_DIM = 0;		// unknown or idle: dim
	static const int STATE_OK = 1;		// on, working: olive
	static const int STATE_OFF = 2;		// off, blocked, missing: red
	static const int STATE_PART = 3;	// partly, under way: amber
	static const int MAX_TILES = 4;

	static const string BUSY = "You're already busy with something. Finish it first, or step away to stop.";

	ref array<ref RZ_StatusTile> m_aTiles = {};
	string m_sHint;			// what would change it ("A running generator within 40 m would power it.")
	string m_sReadLabel;	// over the read's text ("LAST HEARD · BASTION")

	//------------------------------------------------------------------------------------------------
	//! One tile; past the fourth, nothing.
	void Add(string label, string value, int state, int left = 0)
	{
		if (m_aTiles.Count() >= MAX_TILES)
			return;

		RZ_StatusTile t = new RZ_StatusTile();
		t.m_sLabel = label;
		t.m_sValue = value;
		t.m_iState = state;
		t.m_iLeft = left;
		m_aTiles.Insert(t);
	}

	//------------------------------------------------------------------------------------------------
	//! The tiles as S records, each on a new line.
	string Records()
	{
		string recs = "";
		foreach (RZ_StatusTile t : m_aTiles)
		{
			recs += string.Format("\nS|%1|%2|%3|%4", RZ_UIText.Clean(t.m_sLabel, 16), RZ_UIText.Clean(t.m_sValue, 26), t.m_iState, t.m_iLeft);
		}

		return recs;
	}

	//------------------------------------------------------------------------------------------------
	//! "~25 MIN", at least a minute.
	static string Minutes(float seconds)
	{
		return string.Format("~%1 MIN", Math.Max(1, RZ_Util.RoundToInt(seconds / 60)));
	}

	//------------------------------------------------------------------------------------------------
	//! YES (olive) or NO (red), as a tile.
	void YesNo(string label, bool yes)
	{
		if (yes)
			Add(label, "YES", STATE_OK);
		else
			Add(label, "NO", STATE_OFF);
	}

	//------------------------------------------------------------------------------------------------
	//! The world interaction spot `owner` is, or null.
	static RZ_Spot SpotOf(IEntity owner)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!owner || !hub)
			return null;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Cast(owner.FindComponent(RZ_PuzzleObjectComponent));
		if (!comp || comp.GetMissionId() != RZ_Interactions.TAG)
			return null;

		return hub.FindSpot(comp.GetObjectIndex());
	}

	//------------------------------------------------------------------------------------------------
	//! The object menu's tiles, hint and read label for `owner`: a spot's from its system, anything
	//! else's from the menu's opener.
	static void Fill(IEntity owner, IEntity user, int playerId, RZ_GroupedAction opener, notnull RZ_ObjectStatus st)
	{
		RZ_Spot s = SpotOf(owner);
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!s)
		{
			if (opener)
				opener.MenuStatus(owner, user, st);

			return;
		}

		switch (s.m_iKind)
		{
			case RZ_ESpotKind.RADIO: hub.GetRadio().HouseRadioStatus(playerId, s, st); return;
			case RZ_ESpotKind.GENERATOR: hub.GetTraps().FoundGeneratorStatus(s, user, st); return;
			case RZ_ESpotKind.STOVE: hub.GetCrafting().StoveStatus(playerId, s, st); return;
			case RZ_ESpotKind.WELL: hub.GetCrafting().WellStatus(user, st); return;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A spot's choice `option` can't be done now: what its own code would say, and the tag. False
	//! for anything that isn't a spot, or that can be done.
	static bool SpotLock(IEntity owner, IEntity user, int option, out string reason, out string tag)
	{
		RZ_Spot s = SpotOf(owner);
		RZ_Interactions hub = RZ_Interactions.Get();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(user);
		if (!s || !user || playerId <= 0)
			return false;

		// What the hub refuses first (RZ_Interactions.HandleChoice): work under way; a house
		// radio's View radio only shows, so it's fine mid-task
		bool viewing = s.m_iKind == RZ_ESpotKind.RADIO && option == 3;
		if (hub.HasTask(playerId) && !viewing)
		{
			reason = BUSY;
			tag = "BUSY";
			return true;
		}

		switch (s.m_iKind)
		{
			case RZ_ESpotKind.RADIO: return hub.GetRadio().HouseRadioLock(playerId, s, option, user, reason, tag);
			case RZ_ESpotKind.GENERATOR: return hub.GetTraps().FoundGeneratorLock(playerId, s, option, user, reason, tag);
			case RZ_ESpotKind.STOVE:
			{
				// A fireplace that isn't burning cooks nothing: every row says why
				string fireWhy = hub.GetCrafting().FireOutWhy(s);
				if (!fireWhy.IsEmpty())
				{
					reason = fireWhy;
					tag = "FIRE OUT";
					return true;
				}

				return hub.GetCrafting().StoveLock(option, user, reason, tag);
			}
			case RZ_ESpotKind.WELL: return hub.GetCrafting().WellLock(option, user, reason, tag);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A short tag for an action's own reason (its SetCannotPerformReason): the known ones by name,
	//! a reason of two words or fewer as it is, anything else NOT NOW.
	static string TagFor(string reason)
	{
		switch (reason)
		{
			case "No samples": return "NO SAMPLES";
			case "The cure is out": return "CURE OUT";
			case "The cold store is full": return "STORE FULL";
			case "Nothing to hand over": return "NOTHING";
			case "Locked": return "LOCKED";
		}

		array<string> words = {};
		reason.Split(" ", words, true);
		if (words.IsEmpty() || words.Count() > 2)
			return "NOT NOW";

		string tag = reason;
		tag.ToUpper();
		return tag;
	}
}
