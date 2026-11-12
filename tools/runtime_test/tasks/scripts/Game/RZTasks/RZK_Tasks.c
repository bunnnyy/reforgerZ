// Test-only task place audit driver (tools/runtime_test/run.sh tasks). Not part of the mod.
// In the site audit's worlds with the mod's own game mode settings (places.py prepare): waits for the
// land grid and the holdings, runs the place audit (World/RZ_PlaceAudit.c) with a time budget a
// second, prints its summary and closes the server.

//------------------------------------------------------------------------------------------------
modded class RZ_GameModeComponent
{
	protected int m_iRZKSec;
	protected int m_iRZKPhase;
	protected int m_iRZKReadyAt;
	protected ref RZ_PlaceAudit m_RZKAudit;

	protected static const int RZK_LAND_WAIT = 1500;	// seconds to wait for the land grid at most
	protected static const int RZK_SETTLE = 30;		// then this long for holdings, safehouses and posts
	protected static const int RZK_BUDGET_MS = 400;	// the audit's work a second

	override void OnGameModeStart()
	{
		super.OnGameModeStart();
		Print(string.Format("RZK start: world %1", GetGame().GetWorldFile()));
		GetGame().GetCallqueue().CallLater(RZK_Tick, 1000, true);
	}

	void RZK_Tick()
	{
		m_iRZKSec++;
		if (m_iRZKPhase >= 3)
			return;

		if (m_iRZKPhase == 0)
		{
			RZ_Land land = GetLand();
			bool ready = !land || !land.IsEnabled() || land.IsReady();
			if (!ready && m_iRZKSec < RZK_LAND_WAIT)
			{
				if (m_iRZKSec % 60 == 0)
					Print(string.Format("RZK t=%1 s: waiting for the land grid", m_iRZKSec));

				return;
			}

			Print(string.Format("RZK land ready (%1) after %2 s", ready, m_iRZKSec));
			m_iRZKReadyAt = m_iRZKSec + RZK_SETTLE;
			m_iRZKPhase = 1;
			return;
		}

		if (m_iRZKPhase == 1)
		{
			if (m_iRZKSec < m_iRZKReadyAt)
				return;

			m_RZKAudit = new RZ_PlaceAudit(this);
			m_RZKAudit.Start();
			Print(string.Format("RZK audit started at %1 s", m_iRZKSec));
			m_iRZKPhase = 2;
			return;
		}

		if (!m_RZKAudit.Step(RZK_BUDGET_MS))
		{
			if (m_iRZKSec % 30 == 0)
				Print(string.Format("RZK t=%1 s: auditing", m_iRZKSec));

			return;
		}

		m_iRZKPhase = 3;
		int under = m_RZKAudit.Summary();
		Print(string.Format("RZK done: %1 kinds under the threshold (%2), at %3 s", under, RZ_PlaceAudit.s_iThreshold, m_iRZKSec));
		GetGame().GetCallqueue().CallLater(RZK_Close, 2000, false);
	}

	void RZK_Close()
	{
		GetGame().RequestClose();
	}
}
