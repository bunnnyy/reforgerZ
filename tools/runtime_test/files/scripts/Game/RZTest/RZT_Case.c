// Test-only (not part of the mod): the runner's case (Guidance/RZ_OpeningCase.c, docs/DESIGN.md
// "The case"), on the fake player at the route's first town, after the openings' check:
// - A place for the runner is found by the road (or a yard), the runner lies dead there and the case
//   stands by him with its tag (HALCYON ... KEEP COLD and a date).
// - In the tester's pack, the case's step is done (Carries) and the lead to Halcyon's station is
//   handed; 15 s with it does nothing to the lead (only the reply ends it).
// - It warms: an hour in the open and it reads warm.
// - A death keeps it as data (off the body) and the next tick puts it back in the pack.
// - At a Halcyon speaker the hand-over reply is listed; picking it takes the case, starts Proof for
//   this character (when Proof runs), banks the season's live tissue (case.tissue 1) and ends the
//   lead. A second case from the same player this season counts for nothing (case.tissue stays 1).
// Lines: "RZT case OK|FAIL <what>" and "RZT case: N checks, M failed".

class RZT_Case
{
	static bool s_bDone;
	static int s_iSec = -1;
	static int s_iChecks;
	static int s_iFails;
	static IEntity s_Body;
	static IEntity s_Item;
	static int s_iLead = -1;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT case %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static IEntity NewCase(RZ_GameModeComponent gm, RZ_OpeningCase c, IEntity me)
	{
		IEntity item = RZ_Util.SpawnPrefab(RZ_OpeningCase.CasePrefab(gm), me.GetOrigin() + Vector(0, 0.5, 0), 0);
		RZ_ColdCaseComponent cc = RZ_ColdCaseComponent.Of(item);
		if (cc)
			cc.SetTag(c.Tag(), RZT_Fake.ID);

		Hold(me, item, RZ_Util.InsertIntoInventory(me, item));
		return item;
	}

	//------------------------------------------------------------------------------------------------
	//! A case that couldn't go into the pack (on this server a text prefab has no registered resource
	//! id, so replication refuses it): the tester holds it through the case's test stand-in.
	static void Hold(IEntity me, IEntity item, bool inPack)
	{
		if (!item)
			return;

		// The insert can say yes and still not hold it (a 1x1 case fits, replication refuses it): look
		if (inPack && RZ_ColdCaseComponent.CarriedBy(me) == item)
			return;

		RZ_ColdCaseComponent.s_TestHolder = me;
		RZ_ColdCaseComponent.s_TestCarried = item;
		Print("RZT case: the case couldn't go into the tester's pack on this server (an unregistered text prefab); held through the test stand-in");
	}

	//------------------------------------------------------------------------------------------------
	static void End()
	{
		Print(string.Format("RZT case: %1 checks, %2 failed", s_iChecks, s_iFails));
		s_bDone = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from RZT_Tick until done. False while it runs.
	static bool Tick(RZ_GameModeComponent gm, vector town)
	{
		if (s_bDone)
			return true;

		s_iSec++;
		RZ_OpeningCase c = RZ_OpeningCase.Get();
		IEntity me = RZT_Fake.s_Ent;
		if (!c || !me || !gm.GetErrands() || !gm.GetErrands().IsOn())
		{
			Print("RZT case: skipped (the case, the errands or the tester missing)");
			s_bDone = true;
			return true;
		}

		if (RZ_OpeningCase.CasePrefab(gm).IsEmpty())
		{
			Print("RZT case: skipped (Guidance > Cold case is empty)");
			s_bDone = true;
			return true;
		}

		if (s_iSec == 0)
		{
			vector place;
			string text;
			bool found = c.RunnerPlace(town, place, text);
			Check(found, "a place for the runner near the first town (" + text + ")");
			if (!found)
			{
				End();
				return true;
			}

			s_Body = c.SpawnRunner(place, RZT_Fake.ID);
			Check(s_Body != null && !RZ_Util.IsCharacterAlive(s_Body), "the runner lies there, dead");
			RZ_PlaceThing thing = new RZ_PlaceThing();
			thing.m_eKind = RZ_EThingKind.ITEM;
			thing.m_sSlot = "case";
			thing.m_fRange = 4;
			Check(RZ_PlaceThings.Near(place, thing), "the case stands by him");
			return false;
		}

		if (s_iSec == 1)
		{
			s_Item = NewCase(gm, c, me);
			RZ_ColdCaseComponent cc = RZ_ColdCaseComponent.Of(s_Item);
			Check(cc && cc.GetTag().Contains("HALCYON") && cc.GetTag().Contains("KEEP COLD") && cc.GetTag().Contains("Packed"), "its tag: " + RZ_ColdCaseComponent.ReadText(cc));
			Check(RZ_OpeningCase.Carries(me), "in the tester's pack it counts as carried (the step's done)");
			c.Begin(RZT_Fake.ID, me, "Test Contact", "Test Contact, a neighbour", me.GetOrigin());
			s_iLead = c.LeadOf(RZT_Fake.ID);
			Check(s_iLead >= 0, "the lead to Halcyon's station is handed and tracked");
			return false;
		}

		if (s_iSec < 17)
			return false;

		if (s_iSec == 17)
		{
			RZ_Errand lead = gm.GetErrands().FindById(s_iLead);
			Check(lead && lead.m_eState == RZ_EErrandState.HELD, "15 s with the case: the lead still waits (only the reply ends it)");

			// An hour in the open
			RZ_ColdCaseComponent warm = RZ_ColdCaseComponent.Of(s_Item);
			if (warm)
				warm.Warm(1);

			Check(warm && warm.IsWarm(), "an hour out of the cold, it reads warm");

			// A death: off the body, kept as data
			c.OnPlayerDied(RZT_Fake.ID, me);
			Check(!RZ_OpeningCase.Carries(me) && gm.GetProfile(RZT_Fake.ID).m_bCaseHeld, "a death keeps it as data, not on the body");
			return false;
		}

		// The respawn: the tick gives it back to a player who stands (the fake tester isn't one, so here
		// it's asked for directly)
		if (s_iSec == 18)
		{
			c.GiveBack(RZT_Fake.ID, me);
			return false;
		}

		if (s_iSec < 20)
			return false;

		if (s_iSec == 20)
		{
			bool given = !gm.GetProfile(RZT_Fake.ID).m_bCaseHeld && RZ_ColdCaseComponent.s_LastMade != null;
			if (!RZ_OpeningCase.Carries(me) && given)
				Hold(me, RZ_ColdCaseComponent.s_LastMade, false);

			Check(given && RZ_OpeningCase.Carries(me), "after the death it's given back to the tester");
			RZ_ColdCaseComponent back = RZ_ColdCaseComponent.Of(RZ_ColdCaseComponent.CarriedBy(me));
			Check(back && back.IsWarm(), "and it's as warm as it was");

			RZ_TalkSession s = new RZ_TalkSession();
			s.m_iPlayer = RZT_Fake.ID;
			s.m_iFaction = RZ_EFaction.INSTITUTE;
			s.m_sName = "Test Halcyon";
			array<ref RZ_TalkReply> replies = {};
			c.AddReplies(s, me, replies);
			Check(!replies.IsEmpty() && replies[0].m_iId == RZ_OpeningCase.REPLY_HALCYON, "a Halcyon speaker lists the hand-over first");

			RZ_WorldFacts facts = gm.GetFacts();
			int before = facts.Flag("case.tissue");
			c.Pick(s, RZ_OpeningCase.REPLY_HALCYON, me);
			Check(!RZ_OpeningCase.Carries(me), "picking it takes the case");
			Check(gm.GetProfile(RZT_Fake.ID).m_iCaseState == RZ_OpeningCase.STATE_HALCYON, "the case went to Halcyon");
			Check(before != 0 || facts.Flag("case.tissue") == 1, "the season's first live case is banked as tissue");
			Check(c.LeadOf(RZT_Fake.ID) < 0, "the lead ended with the hand-over");
			RZ_Questlines lines = gm.GetQuestlines();
			RZ_QuestRun proof;
			if (lines)
				proof = lines.FindRunByKey("proof");

			if (proof)
				Check(RZ_OpeningCase.TookPart(gm, proof, RZT_Fake.ID), "Proof counts the tester as taking part");
			else
				Print("RZT case: Proof isn't running, its part not checked");

			foreach (string said : s.m_aNew)
			{
				Print("RZT case: they said: " + said);
			}

			// A second case from the same player this season
			NewCase(gm, c, me);
			Check(RZ_OpeningCase.Carries(me), "a second case in the tester's hands");
			c.Pick(s, RZ_OpeningCase.REPLY_HALCYON, me);
			Check(facts.Flag("case.tissue") <= 1 && !RZ_OpeningCase.Carries(me), "a second case this season is taken and banks nothing more");

			// Sold to the Red Hand: Get It Back is handed, at their holding
			NewCase(gm, c, me);
			RZ_TalkSession rh = new RZ_TalkSession();
			rh.m_iPlayer = RZT_Fake.ID;
			rh.m_iFaction = RZ_EFaction.BANDITS;
			array<ref RZ_TalkReply> rhReplies = {};
			c.AddReplies(rh, me, rhReplies);
			Check(!rhReplies.IsEmpty() && rhReplies[0].m_iId == RZ_OpeningCase.REPLY_SELL, "a Red Hand speaker lists the sale first");
			c.Pick(rh, RZ_OpeningCase.REPLY_SELL, me);
			Check(gm.GetProfile(RZT_Fake.ID).m_iCaseState == RZ_OpeningCase.STATE_SOLD && !RZ_OpeningCase.Carries(me), "sold: the case is gone, Get It Back is next");
			RZ_ColdCaseComponent.s_TestCarried = null;
			RZ_ColdCaseComponent.s_TestHolder = null;
			End();
			return true;
		}

		return false;
	}
}
