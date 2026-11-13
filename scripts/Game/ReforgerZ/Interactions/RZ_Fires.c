//! Fires a player lights (docs/DESIGN.md, "Fire"): the fireplace of Make Fire Campfire & Wood, or any
//! fireplace with vanilla's fireplace component. Read through vanilla's component only, so no class
//! of another mod is named: a fireplace is lit while its fire is on.

//------------------------------------------------------------------------------------------------
class RZ_Fires
{
	//------------------------------------------------------------------------------------------------
	//! The thing is a fireplace (vanilla's fireplace component is on it).
	static bool IsFireplace(IEntity e)
	{
		return e && e.FindComponent(SCR_FireplaceComponent) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Its fire is burning. False for anything that isn't a fireplace.
	static bool IsLit(IEntity e)
	{
		if (!e)
			return false;

		SCR_FireplaceComponent fire = SCR_FireplaceComponent.Cast(e.FindComponent(SCR_FireplaceComponent));
		if (!fire)
			return false;

		return fire.IsOn(); // VERIFY: on the server, a player-built fireplace's vanilla fire reads on from Light fire until it's put out or burns down (the campfire mod turns it with ToggleLight)
	}
}
