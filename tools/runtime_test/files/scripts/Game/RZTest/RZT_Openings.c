// Test-only (not part of the mod): the openings' play-test faults (docs/DESIGN.md, "Openings"),
// before the missions start, at the route's first town, on the fake player:
// - The Fever: its son lies there (a body on the care work site, laid down), the tracker's place is
//   on him, and standing by the giver for 20 s does nothing; the hold done with a dressing on you
//   finishes it and takes the dressing.
// - A hand-over (The Cut): 15 s by the giver with the thing on you does nothing; RZ_Errands.FinishNow
//   (the reply) finishes it there and then, and only with the thing.
// - Every handed step (all backgrounds) ends where it's done: none waits to be handed in by walking back.
// - The rules check's errand (RZT_Rules.c): a named person's ask at a work site near them; its crate
//   stands there, the hold does the step, 15 s back with the giver doesn't hand it in while the talk
//   screen is on, and the hand-in reply does (counted "by reply").
// - Doors: a real outside door is found near the town, and a door-like prop is refused in the open.
// Lines: "RZT openings OK|FAIL <what>" and "RZT openings: N checks, M failed".

class RZT_Openings
{
	static bool s_bDone;
	static int s_iSec = -1;
	static int s_iChecks;
	static int s_iFails;
	static ref RZ_Errand s_Fever;
	static ref RZ_Errand s_Cut;
	static ref RZ_Errand s_Rules;
	static ref RZT_RulesDef s_RulesDef;		// held here: an errand keeps only a weak reference to its definition
	static string s_sRulesGiver;
	static bool s_bTalkWas;
	static ResourceName s_sReplyWas;
	static vector s_vGiver;

	static const ResourceName DRESSING = "{A81F501D3EF6F38E}Prefabs/Items/Medicine/FieldDressing_01/FieldDressing_US_01.et";
	static const ResourceName SITE = "{4B78886FD8E5AC98}Prefabs/ReforgerZ/Work/RZ_WorkSite.et";
	static const ResourceName PERSON = "{8C7093AF368F496A}Prefabs/Characters/Factions/CIV/GenericCivilians/Character_CIV_CottonShirt_1.et";
	static const ResourceName BOARDED = "{828D8E56E748FB2D}Prefabs/ReforgerZ/Interactions/RZ_BoardedDoorway.et";

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT openings %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static void GiveDressing()
	{
		IEntity me = RZT_Fake.s_Ent;
		Resource res = Resource.Load(DRESSING);
		if (!me || !res || !res.IsValid())
			return;

		EntitySpawnParams p = new EntitySpawnParams();
		p.TransformMode = ETransformMode.WORLD;
		p.Transform[3] = me.GetOrigin() + Vector(0, 0.5, 0);
		IEntity item = GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), p);
		bool inside = RZ_Util.InsertIntoInventory(me, item);
		Print(string.Format("RZT openings: a field dressing into the tester's pack: %1 (medical now %2)", inside, RZ_Errands.CountItems(me, RZ_EErrandItem.MEDICAL)));
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second from RZT_Tick until done. False while it runs.
	static bool Tick(RZ_GameModeComponent gm, vector town)
	{
		if (s_bDone)
			return true;

		s_iSec++;
		RZ_Errands errands = gm.GetErrands();
		RZ_Openings openings = RZ_Openings.Get();
		IEntity me = RZT_Fake.s_Ent;
		if (!errands || !errands.IsOn() || !openings || !me)
		{
			Print("RZT openings: skipped (errands, openings or the tester missing)");
			s_bDone = true;
			return true;
		}

		RZ_OpeningChain chain = openings.RZT_Chain();
		if (s_iSec == 0)
		{
			// The test world leaves work sites and the duty person empty: the game mode's own
			RZ_WorkSites sites0 = RZ_WorkSites.Get();
			if (sites0 && !RZ_WorkSites.Available())
			{
				sites0.Settings().m_bEnabled = true;
				sites0.Settings().m_sSitePrefab = SITE;
				Print("RZT openings: work sites turned on for the check (the game mode's Site prefab)");
			}

			if (RZ_ErrandSettings.Get().Props().m_sDutyPerson.IsEmpty())
				RZ_ErrandSettings.Get().Props().m_sDutyPerson = PERSON;

			s_vGiver = RZ_Util.GroundPos(town + Vector(4, 0, 4));
			RZT_Fake.MoveTo(s_vGiver + Vector(2, 0, 0));
			s_Fever = errands.Hand(chain.Def(RZ_EOpeningTask.FEVER), RZT_Fake.ID, "rzt:fever", "Pavel Test", "Pavel Test, a patient's father", RZ_EFaction.SURVIVORS, s_vGiver, s_vGiver, "Pavel Test", me);
			Check(s_Fever != null, "the fever's step was handed");
			if (!s_Fever)
			{
				s_bDone = true;
				return true;
			}

			Print(string.Format("RZT openings: the fever's step '%1' at %2", s_Fever.Fill(s_Fever.GetWay().StepText()), RZ_SettlementScanner.GridReference(s_Fever.m_vPlace)));
			return false;
		}

		if (s_iSec == 6)
		{
			string key = RZ_Errands.WorkKey(s_Fever);
			RZ_WorkSites sites = RZ_WorkSites.Get();
			Check(sites && sites.Has(key), "the fever is a care work site (a hold at the son, not a talk)");
			IEntity son;
			if (sites)
				son = sites.RZT_Person(key);

			Check(son != null, "the son is there, a body on the site");
			if (son)
			{
				float d = RZ_Util.DistXZ(son.GetOrigin(), s_Fever.m_vPlace);
				Check(d <= 1.5, string.Format("the tracker's place is on the son (%1 m from him)", RZ_Util.RoundToInt(d)));
				Check(RZ_Util.DistXZ(son.GetOrigin(), s_vGiver) > 0.5, "the son isn't the giver");
				CharacterControllerComponent ctrl = CharacterControllerComponent.Cast(son.FindComponent(CharacterControllerComponent));
				// VERIFY: IsUnconscious on the server after The Round's LieDown
				bool lying = ctrl && ctrl.IsUnconscious();
				Print(string.Format("RZT openings: the son lies down (unconscious state): %1", lying));
			}

			// Standing by the giver with nothing done must never finish it
			RZT_Fake.MoveTo(s_vGiver);
			return false;
		}

		if (s_iSec == 30)
		{
			Check(s_Fever.m_eState == RZ_EErrandState.HELD && !s_Fever.m_bStepDone, "24 s by the giver did not finish the fever (no proximity finish)");
			RZT_Fake.MoveTo(s_Fever.m_vPlace + Vector(0.8, 0, 0));
			return false;
		}

		if (s_iSec == 32)
		{
			GiveDressing();
			RZ_WorkSites sites2 = RZ_WorkSites.Get();
			int worked = 0;
			if (sites2)
				worked = sites2.CompleteAllFor(RZ_Errands.WorkKey(s_Fever), RZT_Fake.ID, me.GetOrigin(), 25);

			Print(string.Format("RZT openings: the son's hold done for the tester (%1 site)", worked));
			return false;
		}

		if (s_iSec == 44)
		{
			Check(s_Fever.m_eState == RZ_EErrandState.DONE, "the hold at the son finished the fever");
			Check(RZ_Errands.CountItems(me, RZ_EErrandItem.MEDICAL) == 0, "the dressing was used on him");

			// The Cut: finished by the reply there and then, and only with the thing on you
			RZT_Fake.MoveTo(s_vGiver + Vector(1, 0, 0));
			s_Cut = errands.Hand(chain.Def(RZ_EOpeningTask.CUT), RZT_Fake.ID, "rzt:cut", "Pavel Test", "Pavel Test, a patient's father", RZ_EFaction.SURVIVORS, s_vGiver, s_vGiver, "Pavel Test", me);
			Check(s_Cut != null, "the cut's step was handed");
			if (!s_Cut)
			{
				s_bDone = true;
				return true;
			}

			Check(!errands.FinishNow(s_Cut, me), "the hand-over is refused without a dressing");
			GiveDressing();
			// Now stand by the giver with the dressing on you: nothing may finish it but the reply
			RZT_Fake.MoveTo(s_vGiver + Vector(1, 0, 0));
			return false;
		}

		if (s_iSec == 60)
		{
			Check(s_Cut.m_eState == RZ_EErrandState.HELD && !s_Cut.m_bStepDone, "15 s by the giver with a dressing did not finish the cut (only the reply does)");
			Check(errands.FinishNow(s_Cut, me), "the hand-over reply finishes it at once");
			Check(s_Cut.m_eState == RZ_EErrandState.DONE, "the cut is done in the same second, not later");

			// Every handed step ends where it's done: none waits to be handed in by walking back
			int waits = 0;
			string which = "";
			array<RZ_OpeningStepDef> defs = {chain.m_Lead, chain.m_LeadNote, chain.m_TalkFirst, chain.m_TalkNext};
			for (int ti = 0; ti < 40; ti++)
			{
				RZ_OpeningStepDef td = chain.Def(ti);
				if (td)
					defs.Insert(td);
			}

			foreach (RZ_OpeningStepDef walkDef : chain.m_aWalk)
			{
				defs.Insert(walkDef);
			}

			foreach (RZ_OpeningStepDef sd : defs)
			{
				if (!sd)
					continue;

				foreach (RZ_ErrandWay sw : sd.m_aWays)
				{
					if (!sw.m_bEndsThere)
					{
						waits++;
						which += " " + sd.m_sKey;
					}
				}
			}

			Check(waits == 0, string.Format("%1 opening steps checked; %2 wait to be handed in by walking back%3", defs.Count(), waits, which));
			return false;
		}

		if (s_iSec == 62)
		{
			IEntity door;
			vector centre;
			vector normal;
			bool found = RZ_DoorGeometry.NearestOuterDoor(town, 0, 150, false, door, centre, normal);
			string where = "none";
			if (found)
				where = string.Format("%1 m from the centre", RZ_Util.RoundToInt(RZ_Util.DistXZ(town, centre)));

			Check(found, "a real outside door near the town (" + where + ")");
			if (found)
				Check(RZ_DoorGeometry.DoorPropFits(BOARDED, centre, "rzt"), "a boarded doorway fits in a real doorway");

			vector open = RZ_Mission.LandPointNear(town, 400, 800);
			IEntity none;
			vector c2;
			vector n2;
			if (!RZ_DoorGeometry.NearestOuterDoor(open, 0, RZ_DoorGeometry.DOOR_PROP_REACH, false, none, c2, n2))
				Check(!RZ_DoorGeometry.DoorPropFits(BOARDED, open, "rzt"), "a boarded doorway is refused in the open");

			array<string> names = {};
			RZ_Rng rng = new RZ_Rng(7);
			for (int i = 0; i < 6; i++)
			{
				names.Insert(RZ_OpeningText.Name(rng));
			}

			Print(string.Format("RZT openings: contacts %1; the nurse's: %2; sent on to: %3", names, RZ_OpeningText.Who("nurse"), RZ_OpeningText.NextWho("nurse")));

			// The rules check's errand (RZT_Rules.c): a named person's ask at a work site near them
			RZ_NamedPeople people = errands.GetPeople();
			RZ_NamedPerson giver;
			for (int h = 0; h < 40 && !giver && people; h++)
			{
				giver = people.Get(RZ_NamedPeople.HoldingKey(h, 0));
			}

			if (giver)
			{
				s_sRulesGiver = giver.m_sKey;
				vector stepAt = RZ_Mission.LandPointNear(giver.m_vPos, 10, 18);
				s_RulesDef = new RZT_RulesDef();
				s_Rules = errands.RZT_Give(s_RulesDef, RZT_Fake.ID, giver.m_sKey, stepAt);
			}

			Check(s_Rules != null, "the rules check's errand was taken from a named person");
			if (!s_Rules)
			{
				Print(string.Format("RZT openings: %1 checks, %2 failed", s_iChecks, s_iFails));
				s_bDone = true;
				return true;
			}

			RZT_Fake.MoveTo(s_Rules.m_vPlace + Vector(0.8, 0, 0));
			return false;
		}

		if (s_iSec == 76)
		{
			// Rule 2: what its words name stands there, 10 s and more after arriving
			RZ_WorkSites sites3 = RZ_WorkSites.Get();
			string rulesKey = RZ_Errands.WorkKey(s_Rules);
			RZT_Rules.s_aChecked.Insert(s_Rules.m_iId);
			RZT_Rules.s_iThings++;
			bool standing = sites3 && sites3.PropOf(rulesKey) != null;
			if (!standing)
			{
				RZT_Rules.s_iMissing++;
				Print(string.Format("RZT rules FAIL thing: errand %1 '%2' at %3: nothing stands at its work site 10 s after arriving", s_Rules.m_iId, s_Rules.Fill(s_Rules.GetWay().StepText()), RZ_SettlementScanner.GridReference(s_Rules.m_vPlace)));
			}

			Check(standing, "the crate stands at the rules errand's work site");
			int done3 = 0;
			if (sites3)
				done3 = sites3.CompleteAllFor(rulesKey, RZT_Fake.ID, RZT_Fake.s_Ent.GetOrigin(), 25);

			Print(string.Format("RZT openings: the rules errand's hold done for the tester (%1 site)", done3));
			return false;
		}

		if (s_iSec == 88)
		{
			Check(s_Rules.m_bStepDone, "the rules errand's step is done at its place");
			// Back with the giver: with the talk screen on, only the reply hands it in. The test world
			// has no talk screen, so it's switched on for this part (the server only reads the flag
			// and that a reply layout is named) and put back after
			s_bTalkWas = RZ_DialogueSettings.Get().m_bTalkScreen;
			s_sReplyWas = RZ_UISettings.Get().m_sTalkReplyLayout;
			RZ_DialogueSettings.Get().m_bTalkScreen = true;
			if (s_sReplyWas.IsEmpty())
				RZ_UISettings.Get().m_sTalkReplyLayout = "{00000000000000A1}UI/RZT_Reply.layout";

			RZ_NamedPerson back = errands.GetPeople().Get(s_sRulesGiver);
			if (back)
				RZT_Fake.MoveTo(back.m_vPos + Vector(1, 0, 0));

			return false;
		}

		if (s_iSec == 106)
		{
			Check(RZ_Dialogue.IsOn(), "the talk screen is on for the hand-in check");
			Check(s_Rules.m_eState == RZ_EErrandState.HELD, "15 s back with the giver did not hand the errand in (the talk screen is on)");
			if (s_Rules.m_eState == RZ_EErrandState.HELD)
				errands.RZT_ReplyHandIn(s_Rules, RZT_Fake.ID);

			RZ_DialogueSettings.Get().m_bTalkScreen = s_bTalkWas;
			RZ_UISettings.Get().m_sTalkReplyLayout = s_sReplyWas;

			Check(s_Rules.m_eState == RZ_EErrandState.DONE, "the hand-in reply finished the errand");
			Print("RZT rules (so far): " + RZT_Rules.Summary());
			Print(string.Format("RZT openings: %1 checks, %2 failed", s_iChecks, s_iFails));
			s_bDone = true;
			return true;
		}

		return false;
	}
}

modded class RZ_Openings
{
	RZ_OpeningChain RZT_Chain()
	{
		return m_Chain;
	}
}

modded class RZ_WorkSites
{
	IEntity RZT_Person(string key)
	{
		RZ_WorkSite s = Find(key);
		if (!s)
			return null;

		return s.m_Person;
	}
}
