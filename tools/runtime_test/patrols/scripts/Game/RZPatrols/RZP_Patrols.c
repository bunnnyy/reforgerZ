// Test-only road patrols driver (tools/runtime_test/run.sh patrols). Not part of the mod.
// In the site audit's Arland world with the mod's own game mode settings (places.py prepare), which,
// unlike the full test's world, has the map's road network: waits for the land grid and the holdings,
// puts the fake player by the first holding, runs the road patrols check (RZT_RoadPatrols.c), prints
// "RZP done" and closes the server.

//------------------------------------------------------------------------------------------------
modded class RZ_GameModeComponent
{
	protected int m_iRZPSec;
	protected int m_iRZPPhase;

	protected static const int RZP_LAND_WAIT = 1500;	// seconds to wait for the land grid at most
	protected static const int RZP_SETTLE = 30;			// then this long for the holdings

	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		Print(string.Format("RZP start: world %1", GetGame().GetWorldFile()));
		GetGame().GetCallqueue().CallLater(RZP_Tick, 1000, true);
	}

	void RZP_Tick()
	{
		m_iRZPSec++;
		if (m_iRZPPhase >= 3)
			return;

		if (m_iRZPPhase == 0)
		{
			RZ_Land land = GetLand();
			bool ready = !land || !land.IsEnabled() || land.IsReady();
			if (!ready && m_iRZPSec < RZP_LAND_WAIT)
				return;

			m_iRZPPhase = 1;
			m_iRZPSec = 0;
			return;
		}

		if (m_iRZPPhase == 1)
		{
			if (m_iRZPSec < RZP_SETTLE)
				return;

			RZ_Territory territory = GetTerritory();
			vector at = vector.Zero;
			if (territory)
			{
				foreach (RZ_Holding h : territory.GetHoldings())
				{
					if (h && h.m_eFaction == RZ_EFaction.MILITARY)
					{
						at = h.m_vPos;
						break;
					}
				}

				if (at == vector.Zero && !territory.GetHoldings().IsEmpty())
					at = territory.GetHoldings()[0].m_vPos;
			}

			Print(string.Format("RZP the fake player by a holding at %1 (%2 holdings)", RZ_SettlementScanner.GridReference(at), territory.GetHoldings().Count()));
			RZT_Fake.Spawn(RZ_Mission.LandPointNear(at, 60, 120));
			m_iRZPPhase = 2;
			return;
		}

		if (!RZT_RoadPatrols.Tick(this) && m_iRZPSec < 900)
			return;

		m_iRZPPhase = 3;
		Print(string.Format("RZP done at %1 s", m_iRZPSec));
		GetGame().GetCallqueue().CallLater(RZP_Close, 2000, false);
	}

	void RZP_Close()
	{
		GetGame().RequestClose();
	}
}
