// Test-only (not part of the mod): a character who lasts (Backgrounds/RZ_Character.c; docs/DESIGN.md,
// "A character who lasts"). Once, on the fake player, with everything put back after:
// - A death keeps standing, the habit, the fog and the character number, counts a death, and
//   clears the bite; a quest item (a job's goods in the pack) comes off the body as data.
// - Coming round: the nearest camp that takes you in (the Haven at Known); none when every
//   faction hunts you; a bitten death is never carried in; the quest item comes back in the pack.
// - A reset: a new character (number +1, no deaths), reputation from the background, and Watched
//   with a faction the last one was hunted by.
// - An old save: its life number becomes the character number.
// Lines: "RZT persist OK|FAIL <what>" and "RZT persist: N checks, M failed".

class RZT_Persist
{
	static bool s_bDone;
	static int s_iChecks;
	static int s_iFails;

	//------------------------------------------------------------------------------------------------
	static void Check(bool ok, string what)
	{
		s_iChecks++;
		if (!ok)
			s_iFails++;

		string verdict = "OK";
		if (!ok)
			verdict = "FAIL";

		Print(string.Format("RZT persist %1 %2", verdict, what));
	}

	//------------------------------------------------------------------------------------------------
	static void Tick(RZ_GameModeComponent gm)
	{
		if (s_bDone || !RZT_Fake.s_Ent)
			return;

		s_bDone = true;
		int id = RZT_Fake.ID;
		IEntity me = RZT_Fake.s_Ent;
		RZ_PlayerProfile p = gm.GetProfile(id);
		p.EnsureArrays();

		// What's put back after
		array<int> fame = {};
		array<int> infamy = {};
		fame.Copy(p.m_aFame);
		infamy.Copy(p.m_aInfamy);
		int character = p.m_iCharacter;
		int deaths = p.m_iDeaths;
		string bgId = p.m_sBackgroundId;
		if (character <= 0)
			p.m_iCharacter = 1;
		if (bgId.IsEmpty())
			p.m_sBackgroundId = gm.GetDefaultBackground().m_sId;

		// 1. A death
		Check(p.HasCharacter(), "the fake player has a character who goes on");
		p.m_aFame[RZ_EFaction.SURVIVORS] = 20;
		p.m_aInfamy[RZ_EFaction.SURVIVORS] = 0;
		int habit = p.m_iHabit;
		int fogWords = p.m_aFog.Count();
		int before = p.m_iCharacter;
		IEntity pack = RZ_Util.SpawnPrefab("{06B68C58B72EAAC6}Prefabs/Items/Equipment/Backpacks/Backpack_ALICE_Medium.et", me.GetOrigin() + Vector(0, 0.3, 0), 0);
		if (pack)
			RZ_Util.InsertIntoInventory(me, pack);

		IEntity goods = RZ_MissionGoods.Give(me, "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et", "the test");
		bool inPack = goods && RZ_Util.GetCarrierPlayerId(goods) == id;
		p.m_fWoundLeft = 30;
		RZ_Character.OnDied(gm, id, me, me.GetOrigin());
		Check(p.m_iCharacter == before, "a death keeps the character number");
		Check(p.m_iDeaths == deaths + 1, string.Format("a death is counted (%1)", p.m_iDeaths));
		Check(p.m_aFame[RZ_EFaction.SURVIVORS] == 20, "a death keeps Fame");
		Check(p.m_iHabit == habit, "a death keeps the habit");
		Check(p.m_aFog.Count() == fogWords, "a death keeps the map's fog");
		Check(p.m_fWoundLeft <= 0 && p.m_fInfectionLeft < 0, "the bite goes with the body");
		Check(p.m_bDiedBitten, "the death is marked bitten");
		if (inPack)
		{
			Check(p.m_aQiPrefab.Count() == 1, string.Format("the quest item is held as data (%1)", p.m_aQiPrefab.Count()));
			Check(!goods, "the quest item is off the body");
		}
		else
		{
			Print("RZT persist: the goods didn't go into the tester's pack; the hold isn't checked");
		}

		// 2. Coming round
		vector at;
		string name;
		int faction = -1;
		string why = RZ_Character.FindCamp(gm, id, p, at, name, faction);
		Check(why.Contains("bitten"), "a bitten death is never carried in (" + why + ")");
		p.m_bDiedBitten = false;
		RZ_Safehouses houses = gm.GetSafehouses();
		if (houses && houses.Count() > 0)
		{
			vector house = houses.Get(0).m_vPos;
			p.m_fDiedX = house[0];
			p.m_fDiedZ = house[2];
			p.m_aInCamps.Clear();
			p.m_aInUnix.Clear();
			why = RZ_Character.FindCamp(gm, id, p, at, name, faction);
			Check(why.IsEmpty(), "Known with the Haven, a death by its safehouse comes round at a camp (" + name + why + ")");
			Check(faction == RZ_EFaction.SURVIVORS, string.Format("the camp is the Haven's (%1)", faction));
			Check(RZ_Util.DistXZ(at, house) < 400, string.Format("the camp is near where they fell (%1 m)", RZ_Util.DistXZ(at, house)));
		}
		else
		{
			Print("RZT persist: no safehouse on this map; the Haven camp isn't checked");
		}

		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			p.m_aFame[f] = 0;
			p.m_aInfamy[f] = 60;
		}

		p.m_aInCamps.Clear();
		p.m_aInUnix.Clear();
		why = RZ_Character.FindCamp(gm, id, p, at, name, faction);
		Check(!why.IsEmpty(), "hunted by everyone, no camp takes them in (" + why + ")");

		int back = RZ_Character.GiveBackQuestItems(gm, id, me, p);
		if (inPack)
			Check(back == 1, string.Format("the quest item comes back in the pack (%1)", back));

		Check(p.m_aQiPrefab.IsEmpty(), "nothing is left held after coming round");

		// 3. A reset, on a profile of its own
		RZ_PlayerProfile r = new RZ_PlayerProfile();
		r.EnsureArrays();
		r.m_sIdentity = "rzt-persist";
		r.m_iCharacter = 3;
		r.m_iDeaths = 7;
		r.m_sBackgroundId = "x";
		r.m_bResetPending = true;
		Check(!r.HasCharacter(), "a confirmed reset ends the character");
		r.StartCharacter(gm.GetDefaultBackground());
		array<int> hunted = {RZ_EFaction.BANDITS};
		RZ_Character.CarryTheName(r, hunted);
		Check(r.m_iCharacter == 4 && r.m_iDeaths == 0, string.Format("a reset starts a new character (%1, %2 deaths)", r.m_iCharacter, r.m_iDeaths));
		Check(r.HasCharacter(), "the new character goes on");
		RZ_ETier t = RZ_Reputation.GetTier(r.m_aFame[RZ_EFaction.BANDITS], r.m_aInfamy[RZ_EFaction.BANDITS]);
		Check(t == RZ_ETier.WATCHED, "hunted by the Red Hand last time, the new character starts Watched with them");

		// 4. An old save
		RZ_PlayerProfile old = new RZ_PlayerProfile();
		old.m_iLives = 5;
		old.EnsureArrays();
		Check(old.m_iCharacter == 5 && old.m_iLives == 0, string.Format("an old save's life becomes its character (%1)", old.m_iCharacter));

		// Put back
		for (int g = 0; g < RZ_Factions.COUNT; g++)
		{
			p.m_aFame[g] = fame[g];
			p.m_aInfamy[g] = infamy[g];
		}

		p.m_iCharacter = character;
		p.m_iDeaths = deaths;
		p.m_sBackgroundId = bgId;
		p.m_bDiedBitten = false;
		p.m_aInCamps.Clear();
		p.m_aInUnix.Clear();
		Print(string.Format("RZT persist: %1 checks, %2 failed", s_iChecks, s_iFails));
	}
}
