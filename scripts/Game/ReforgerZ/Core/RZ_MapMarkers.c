//! Thin wrapper around the vanilla map marker system, so missions and the lab
//! can show up on everyone's map. Turn it off with RZ_GameModeComponent >
//! "Map markers" if it misbehaves. If it doesn't compile in your game version,
//! empty the bodies of Add() and Remove(): nothing else depends on them.
class RZ_MapMarkers
{
	protected static ref map<int, ref SCR_MapMarkerBase> s_mMarkers;
	protected static int s_iNext = 1;

	//------------------------------------------------------------------------------------------------
	//! Server: a marker everyone sees. Returns a handle for Remove(), -1 on failure.
	static int Add(vector pos, string text)
	{
		SCR_MapMarkerManagerComponent mgr = SCR_MapMarkerManagerComponent.GetInstance(); // VERIFY: marker manager singleton
		if (!mgr)
			return -1;

		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM); // VERIFY: marker type enum
		marker.SetWorldPos(pos[0], pos[2]); // VERIFY: SetWorldPos(x, y)
		marker.SetCustomText(text); // VERIFY: SetCustomText
		mgr.InsertStaticMarker(marker, false, true); // VERIFY: InsertStaticMarker(marker, isLocal, isServerMarker)

		if (!s_mMarkers)
			s_mMarkers = new map<int, ref SCR_MapMarkerBase>();

		int handle = s_iNext;
		s_iNext++;
		s_mMarkers.Set(handle, marker);
		return handle;
	}

	//------------------------------------------------------------------------------------------------
	static void Remove(int handle)
	{
		if (handle < 0 || !s_mMarkers)
			return;

		SCR_MapMarkerBase marker;
		if (!s_mMarkers.Find(handle, marker))
			return;

		s_mMarkers.Remove(handle);
		SCR_MapMarkerManagerComponent mgr = SCR_MapMarkerManagerComponent.GetInstance(); // VERIFY
		if (mgr && marker)
			mgr.RemoveStaticMarker(marker); // VERIFY: RemoveStaticMarker
	}
}
