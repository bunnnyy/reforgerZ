//! What stands on a duty's table or ground while its beat is open (docs/ERRANDS.md, "Duties"): a
//! display copy of the thing a trader brought or a passer-by holds out, never something to pick up.
//! Each is a mesh with nothing to use (Prefabs/ReforgerZ/Work/Things/RZ_Show_*.et), slotted on
//! Missions > Errands > Things; an empty slot shows nothing. Server only.

//------------------------------------------------------------------------------------------------
//! The display copies (Missions > Errands > Things). All optional: empty = words only, as before.
[BaseContainerProps()]
class RZ_DutyThingProps
{
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Cash (optional): banknotes on show, nothing to pick up (RZ_Show_Cash.et). The swap table, the count", "et")]
	ResourceName m_sCash;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Gold bar (optional): on show (RZ_Show_GoldBar.et). The count", "et")]
	ResourceName m_sGoldBar;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pearls (optional): a necklace on show (RZ_Show_Pearls.et). The swap table, the count", "et")]
	ResourceName m_sPearls;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Cards (optional): a stack of bank cards on show (RZ_Show_Cards.et). The count", "et")]
	ResourceName m_sCards;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Passport, American (optional): on show (RZ_Show_PassportUS.et). Gate duty", "et")]
	ResourceName m_sPassportUS;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Passport, British (optional): on show (RZ_Show_PassportUK.et). The swap table's travel papers, the toll, gate duty", "et")]
	ResourceName m_sPassportUK;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Passport, Soviet (optional): on show (RZ_Show_PassportRU.et). Gate duty, the roll call's card", "et")]
	ResourceName m_sPassportRU;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Soldier's ID (optional): an army identity card on show (RZ_Show_IDUS.et). Gate duty", "et")]
	ResourceName m_sSoldiersID;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Pills (optional): a bottle of painkillers on show (RZ_Show_Pills.et). The swap table's aspirin, the count, kit inspection", "et")]
	ResourceName m_sPills;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Blindfold (optional): on the ground at the lock-up's door (RZ_Show_Blindfold.et)", "et")]
	ResourceName m_sBlindfold;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bed, dug (optional): a garden bed turned over and raked, bare soil (the garden's seed beat). Empty = words only", "et")]
	ResourceName m_sBedDug;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bed, planted (optional): a garden bed with a row of young plants (the garden's watering beat). Empty = words only", "et")]
	ResourceName m_sBedPlanted;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Bed, ripe (optional): a garden bed with a crop standing tall (the garden's lifting beat). Empty = words only", "et")]
	ResourceName m_sBedRipe;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Lock-up pen (optional): what stands beside a Haven or Bastion lock-up's door, a holding pen with chairs and a table (JTF Bronze Props' JTF_Detainee_US_01.et). Empty = the door only", "et")]
	ResourceName m_sLockUpPen;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Lock-up room (optional): what stands beside a Red Hand lock-up's door, a table, two chairs and a lamp (JTF Bronze Props' JTF_InterrogationRoom_01.et). Empty = the door only", "et")]
	ResourceName m_sLockUpRoom;
}

//------------------------------------------------------------------------------------------------
class RZ_DutyThings
{
	//------------------------------------------------------------------------------------------------
	static RZ_DutyThingProps Props()
	{
		return RZ_ErrandSettings.Get().Things();
	}

	//------------------------------------------------------------------------------------------------
	//! The display copy for a beat, by its duty and key ("swap:ring", "count:gold"), "" when nothing
	//! stands there. Only what's in plain sight: a toll's pack or a kit's bag isn't shown before
	//! it's looked in, and the incinerator's bags are sealed.
	static ResourceName For(string key)
	{
		RZ_DutyThingProps p = Props();
		if (!p)
			return "";

		switch (key)
		{
			case "swap:cash": return p.m_sCash;
			case "swap:pearls": return p.m_sPearls;
			case "swap:papers": return p.m_sPassportUK;
			case "swap:aspirin": return p.m_sPills;
			case "count:gold": return p.m_sGoldBar;
			case "count:pearls": return p.m_sPearls;
			case "count:cards": return p.m_sCards;
			case "count:cash": return p.m_sCash;
			case "count:medicine": return p.m_sPills;
			case "toll:forged": return p.m_sPassportUK;
			case "gate:clean": return p.m_sPassportUS;
			case "gate:trader": return p.m_sPassportUK;
			case "gate:list": return p.m_sPassportRU;
			case "gate:soldier": return p.m_sSoldiersID;
			case "gate:ident": return p.m_sSoldiersID;
			case "kit:syrette": return p.m_sPills;
			case "rollcall:card": return p.m_sPassportRU;
			case "lockup:door": return p.m_sBlindfold;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What dresses a lock-up's door: the pen for the Haven and Bastion, the room for the Red Hand,
	//! nothing for Halcyon's isolation room.
	static ResourceName LockUp(int faction)
	{
		RZ_DutyThingProps p = Props();
		if (!p)
			return "";

		if (faction == RZ_EFaction.BANDITS)
			return p.m_sLockUpRoom;

		// Halcyon's isolation room is indoors, behind a hatch: nothing stands beside it
		if (faction == RZ_EFaction.INSTITUTE)
			return "";

		return p.m_sLockUpPen;
	}
}
