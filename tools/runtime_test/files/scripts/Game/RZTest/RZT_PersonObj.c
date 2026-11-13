// Test-only (not part of the mod): a person objective ("Go back to Dr Sabine Vogel at Morton")
// points at that person's body when it's out, within a few metres, never at their spot or the
// place's centre (docs/UI.md, "The quest tracker"). Each errand whose way back is the current
// objective is checked once while the giver's body is out.
// Lines: "RZT objplace FAIL person ..." and "RZT person objectives: N checked, N wrong".

class RZT_PersonObj
{
	static int s_iChecked;
	static int s_iWrong;
	static ref set<int> s_aSeen = new set<int>();
	static const float NEAR = 5;

	//------------------------------------------------------------------------------------------------
	static string Summary()
	{
		return string.Format("%1 checked, %2 wrong", s_iChecked, s_iWrong);
	}
}

modded class RZ_QuestTracker
{
	//------------------------------------------------------------------------------------------------
	override protected void ErrandObjectives(int playerId, int errandId, RZ_ObjList list)
	{
		super.ErrandObjectives(playerId, errandId, list);
		if (RZT_PersonObj.s_aSeen.Contains(errandId) || !m_GM.GetErrands())
			return;

		RZ_Errand e = m_GM.GetErrands().FindById(errandId);
		if (!e || !e.m_bStepDone)
			return;

		IEntity body = RZ_ObjPlace.BodyOf(m_GM, e.m_sGiver);
		RZ_ObjPlace at = list.PlaceOf(RZ_EObjState.CURRENT);
		if (!body || !at)
			return;

		RZT_PersonObj.s_aSeen.Insert(errandId);
		RZT_PersonObj.s_iChecked++;
		float d = RZ_Util.DistXZ(at.m_vPos, body.GetOrigin());
		if (d > RZT_PersonObj.NEAR)
		{
			RZT_PersonObj.s_iWrong++;
			Print(string.Format("RZT objplace FAIL person errand %1 '%2': the tracker points at %3, %4 stands at %5 (%6 m off)",
				errandId, list.First(RZ_EObjState.CURRENT), RZ_SettlementScanner.GridReference(at.m_vPos), e.m_sGiver, RZ_SettlementScanner.GridReference(body.GetOrigin()), RZ_Util.RoundToInt(d)));
		}

		Print("RZT person objectives (so far): " + RZT_PersonObj.Summary());
	}
}
