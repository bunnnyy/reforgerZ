//! Thin wrapper around the vanilla map marker system, so missions and the lab
//! can show up on everyone's map. Turn it off with RZ_GameModeComponent >
//! "Map markers" if it misbehaves.
class RZ_MapMarkers
{
	protected static ref map<int, ref SCR_MapMarkerBase> s_mMarkers;
	protected static int s_iNext = 1;

	//------------------------------------------------------------------------------------------------
	//! Server: a marker everyone sees. Returns a handle for Remove(), -1 on failure.
	static int Add(vector pos, string text)
	{
		SCR_MapMarkerManagerComponent mgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (!mgr)
			return -1;

		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
		marker.SetWorldPos(pos[0], pos[2]);
		marker.SetCustomText(text);
		mgr.InsertStaticMarker(marker, false, true);

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
		SCR_MapMarkerManagerComponent mgr = SCR_MapMarkerManagerComponent.GetInstance();
		if (mgr && marker)
			mgr.RemoveStaticMarker(marker);
	}
}
