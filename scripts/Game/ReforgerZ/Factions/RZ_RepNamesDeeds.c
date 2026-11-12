//! Deed phrases from missions (docs/REPUTATION.md, "Deed phrases"): what people say you did when a
//! job or an ending is your biggest deed with a faction. Lowercase, past tense, short; `{town}` is
//! the job's nearest named settlement (RZ_Mission.DeedPhrase).
//!
//! - **Contracts and events** without their own `m_sDeed`, by class name; a class not listed keeps
//!   the fallback, "did a job for us near {town}".
//! - **Questline and major endings**, by "<key>:<ending>", told with the ending's Fame
//!   (RZ_Questline.ChangeStanding). The finale's routes read "route:<ending>".
//! Server only; no state.
class RZ_RepDeeds
{
	//------------------------------------------------------------------------------------------------
	//! The phrase for a mission class with no m_sDeed of its own, "" if none (the fallback stays).
	static string ForMission(RZ_Mission m)
	{
		if (!m)
			return "";

		return ForClass(m.ClassName());
	}

	//------------------------------------------------------------------------------------------------
	//! The phrase for an ending of questline `key`, "" if none.
	static string ForEnding(string key, int ending)
	{
		if (key.StartsWith("route_"))
			key = "route";

		return ForKey(string.Format("%1:%2", key, ending));
	}

	//------------------------------------------------------------------------------------------------
	protected static string ForClass(string name)
	{
		switch (name)
		{
			// Bastion
			case "RZ_Mission_SecureTheShipment": return "secured the shipment near {town}";
			case "RZ_Mission_BlowTheBridge": return "blew the bridge near {town}";
			case "RZ_Mission_FireMission": return "called the guns down near {town}";
			case "RZ_Mission_NoReply": return "went to see why {town} stopped answering";
			case "RZ_Mission_StruckOff": return "went after a man struck off the roster near {town}";
			case "RZ_Mission_ClearSettlement": return "took back {town}";
			case "RZ_Mission_LastHelicopter": return "held the field for the last helicopter near {town}";
			case "RZ_Mission_RaidRedHand": return "raided the Red Hand near {town}";
			case "RZ_Mission_DeserterMap": return "followed the deserter's map near {town}";
			case "RZ_Job_HeartsAndMinds": return "carried Bastion's parcels round {town}";
			case "RZ_Job_RealOrders": return "carried the real orders to {town}";
			case "RZ_Job_TrainingDay": return "stood in for training day at {town}";
			case "RZ_Job_LocalLaw": return "settled a matter of law in {town}";
			case "RZ_Job_RiflesForSale": return "moved the rifles near {town}";
			case "RZ_Job_OffBookRequisition": return "requisitioned off the book in {town}";
			case "RZ_Job_BurnTheLedger": return "burnt a ledger in {town}";
			case "RZ_Job_DesertionRoute": return "found the desertion route near {town}";

			// The Haven
			case "RZ_Mission_FeverEvacuation": return "got people out of the fever in {town}";
			case "RZ_Mission_RefugeeColumn": return "walked the refugees through {town}";
			case "RZ_Mission_TheInformant": return "found the informant in {town}";
			case "RZ_Mission_RecoverTheBus": return "brought the bus back from {town}";
			case "RZ_Mission_Memorial": return "put up the memorial at {town}";
			case "RZ_Mission_CourierJob": return "carried the letters to {town}";
			case "RZ_Mission_WalkHimOut": return "walked a man out of {town}";
			case "RZ_Mission_TheLedger": return "settled the ledger in {town}";
			case "RZ_Mission_QuietArrangement": return "made a quiet arrangement in {town}";
			case "RZ_Mission_TheList": return "found the list in {town}";
			case "RZ_Mission_BittenFriend": return "stayed with a bitten friend near {town}";
			case "RZ_Mission_TraderCaravan": return "saw the trader's caravan through {town}";
			case "RZ_Job_OurOwnFirst": return "fed the houses first at {town}";
			case "RZ_Job_BringThemHome": return "fetched a family home from {town}";
			case "RZ_Job_OverTheFence": return "traded over the fence at {town}";
			case "RZ_Job_RationRun": return "ran the rations to {town}";
			case "RZ_Job_NamesForBastion": return "carried names to Bastion near {town}";
			case "RZ_Job_ShutTheDoor": return "shut a door in {town}";
			case "RZ_Job_HiddenStores": return "found the hidden stores in {town}";

			// The Red Hand
			case "RZ_Mission_DoubleAgent": return "played both sides near {town}";
			case "RZ_Mission_KillTheOfficer": return "went after the officer near {town}";
			case "RZ_Mission_TakeTheLab": return "took the lab";
			case "RZ_Mission_LearningTheDrones": return "learned the drones near {town}";
			case "RZ_Mission_PitsDebtor": return "settled the pit's debtor in {town}";
			case "RZ_Mission_TiedDown": return "freed a tied-down truck near {town}";
			case "RZ_Mission_BanditDeadDrop": return "made the dead drop near {town}";
			case "RZ_Mission_FenceTheGoods": return "fenced the goods in {town}";
			case "RZ_Mission_PharmacyHeist": return "cleared out the chemist's in {town}";
			case "RZ_Mission_FightPit": return "fought in the pit at {town}";
			case "RZ_Mission_SmugglersRun": return "made the smuggler's run past {town}";
			case "RZ_Mission_ZombieBait": return "walked the dead where the crews wanted, near {town}";
			case "RZ_Mission_Arson": return "set the fire in {town}";
			case "RZ_Mission_SabotageRelay": return "took the relay down near {town}";
			case "RZ_Mission_StealAntidotes": return "lifted the antidotes in {town}";
			case "RZ_Job_OurCrossroads": return "held the crossroads at {town}";
			case "RZ_Job_HonourAmongThieves": return "settled it between thieves in {town}";
			case "RZ_Job_HouseRules": return "kept the house rules at {town}";
			case "RZ_Job_DebtCollection": return "collected a debt in {town}";
			case "RZ_Job_SoftTouch": return "did the soft-touch job in {town}";
			case "RZ_Job_CoupTalk": return "carried the coup talk round {town}";
			case "RZ_Job_SellOutRivalCrew": return "sold out a rival crew near {town}";
			case "RZ_Job_SkimTheToll": return "skimmed the toll at {town}";

			// Halcyon
			case "RZ_Mission_SpecimenFromTheNest": return "brought a specimen out of the nest near {town}";
			case "RZ_Mission_MedicalDrone": return "recovered the medical drone near {town}";
			case "RZ_Mission_PatientZero": return "traced patient zero near {town}";
			case "RZ_Mission_LiveCapture": return "brought one in alive from {town}";
			case "RZ_Mission_SerumTrial": return "ran the serum trial at {town}";
			case "RZ_Mission_MobileLab": return "kept the mobile lab running near {town}";
			case "RZ_Mission_SampleHunt": return "tracked down the lone one near {town}";
			case "RZ_Mission_LostColleague": return "went looking for a lost colleague near {town}";
			case "RZ_Mission_HuntScreamer": return "hunted the screamer near {town}";
			case "RZ_Mission_LabDefence": return "held the lab";
			case "RZ_Job_SecondOpinion": return "carried a second opinion to {town}";
			case "RZ_Job_PatientsNotSamples": return "stood up for the patients at {town}";
			case "RZ_Job_TheNotebook": return "carried a notebook to {town}";
			case "RZ_Job_UnsanctionedTrial": return "helped with a trial nobody signed for, at {town}";
			case "RZ_Job_LeakTheData": return "leaked the data at {town}";
			case "RZ_Job_SellTheFormula": return "sold the formula near {town}";
			case "RZ_Job_PrivateCollection": return "found a private collection in {town}";

			// Leaders, the Perimeter, the unarmed jobs
			case "RZ_Mission_ReinIn": return "reined in a leader at {town}";
			case "RZ_Mission_BlindSpot": return "found the drones' blind spot near {town}";
			case "RZ_Mission_TheCrossing": return "walked someone to the wire near {town}";
			case "RZ_Mission_ShotAtTheLine": return "found out who was shot at the line near {town}";
			case "RZ_Mission_CarKeys": return "found the car keys in {town}";
			case "RZ_Mission_LeadThemAway": return "led them away from {town}";
			case "RZ_Mission_TrailOfClues": return "followed the trail near {town}";
			case "RZ_Mission_BarterChain": return "traded up and down {town}";
			case "RZ_Mission_Distraction": return "drew the dead off at {town}";
			case "RZ_Mission_QuietCount": return "counted them quietly at {town}";

			// Puzzles
			case "RZ_Mission_TheRat": return "found the rat in {town}";
			case "RZ_Mission_PatientZeroLedger": return "read patient zero's ledger at {town}";
			case "RZ_Mission_PoisonedWell": return "found what was in the well at {town}";
			case "RZ_Mission_SmugglersLedger": return "worked out the smuggler's ledger at {town}";
			case "RZ_Mission_JunctionBoxes": return "fixed the junction boxes at {town}";
			case "RZ_Mission_WeighingVault": return "opened the weighing vault at {town}";
			case "RZ_Mission_LastExperiment": return "finished the last experiment at {town}";
			case "RZ_Mission_SilentVillage": return "found out why {town} went silent";
			case "RZ_Mission_TheWitness": return "found the witness in {town}";
			case "RZ_Mission_NumbersStation": return "found the numbers station near {town}";
			case "RZ_Mission_TwoBearings": return "found the spot from two bearings near {town}";
			case "RZ_Mission_ClockworkHorde": return "timed the horde at {town}";
			case "RZ_Mission_TimeLockSafe": return "opened the time-lock safe in {town}";
			case "RZ_Mission_BellOrder": return "rang the bells in order at {town}";
			case "RZ_Mission_LightsInTheWindow": return "read the lights in the windows at {town}";

			// World events
			case "RZ_Event_EveryRidgeCalls": return "kept still the night every ridge called, near {town}";
			case "RZ_Event_RedDusk": return "came through Red Dusk near {town}";
			case "RZ_Event_TribeMoves": return "kept out of a pack's way near {town}";
			case "RZ_Event_Gathering": return "broke up a gathering near {town}";
			case "RZ_Event_NightOfManyCalls": return "held out the night of many calls near {town}";
			case "RZ_Event_SirenTest": return "answered the siren at {town}";
			case "RZ_Event_Storm": return "came through the storm near {town}";
			case "RZ_Event_WanderingTrader": return "looked after the trader near {town}";
			case "RZ_Event_BastionShelling": return "came through the shelling at {town}";
			case "RZ_Event_MassGrave": return "found the grave near {town}";
			case "RZ_Event_StashNote": return "followed a stash note near {town}";
			case "RZ_Mission_TurnedPlayer": return "put down one of the turned near {town}";
			case "RZ_Event_AbandonedConvoy": return "went through the abandoned convoy near {town}";
			case "RZ_Event_RemnantCamp": return "cleared the old army camp near {town}";
			case "RZ_Event_LockedBunker": return "opened the bunker near {town}";
			case "RZ_Event_WeddingParty": return "was at the wedding near {town}";
			case "RZ_Event_DistressBase": return "answered a distress call near {town}";
			case "RZ_Event_DistressCall": return "answered a distress call near {town}";
			case "RZ_Event_PrisonBreak": return "was there for the prison break at {town}";
			case "RZ_Event_TheSniper": return "dealt with the sniper at {town}";
			case "RZ_Event_RivalSurvivors": return "met the other survivors near {town}";
			case "RZ_Event_CollapseShelter": return "dug people out of the shelter in {town}";
			case "RZ_Event_FogBank": return "came through the fog at {town}";
			case "RZ_Event_PowerSurge": return "dealt with the power surge at {town}";
			case "RZ_Event_GasStationFire": return "put out the petrol station fire at {town}";
			case "RZ_Event_TunnelRats": return "cleared the tunnels at {town}";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! "<key>:<ending>" (the questlines' and majors' ending numbers, docs/QUESTLINES.md).
	protected static string ForKey(string key)
	{
		switch (key)
		{
			// The Haven
			case "list:0": return "had the names on the list read";
			case "list:1": return "buried the list";
			case "list:2": return "gave the list to the census";
			case "list:3": return "sold the list";
			case "door:0": return "kept the door the house's own";
			case "door:1": return "put the door under Bastion";
			case "door:2": return "paid for the door in tins";
			case "door:3": return "took the house's debt on";
			case "walk:0": return "opened the lab road for the bitten";
			case "walk:1": return "kept the walks at the edge of town";
			case "walk:2": return "brought the cut dose to the houses";
			case "haven_short_weight:0": return "made the weight full";
			case "haven_short_weight:1": return "let the weight stay short";
			case "every_name:10": return "saw every name read";
			case "every_name:11": return "closed the book on the names";
			case "every_name:12": return "fed the far house first";
			case "every_name:13": return "had the houses counted on Bastion's cards";
			case "every_name:14": return "made the arrangement at the door";

			// Bastion
			case "orders:0": return "put the orders in the stove";
			case "orders:1": return "told the men what the orders said";
			case "orders:2": return "read the orders out on the open channel";
			case "orders:3": return "gave the Haven the orders";
			case "orders:10": return "stood by the call";
			case "orders:11": return "put the call out to all stations";
			case "orders:12": return "answered for Bastion, Command";
			case "orders:13": return "saw the men expended";
			case "census:0": return "counted the houses for the census";
			case "census:1": return "forged the census";
			case "census:2": return "refused the census";
			case "census:3": return "gave the Red Hand the count";
			case "pilot:0": return "kept the last pilot here";
			case "pilot:1": return "grounded the last pilot";
			case "pilot:2": return "let the last pilot fly for the line";
			case "pilot:3": return "burnt the last helicopter for the Red Hand";

			// The Red Hand
			case "book:0": return "balanced the book";
			case "book:1": return "held the book over three leaders";
			case "book:2": return "gave the book's page to Bastion";
			case "book:3": return "burnt the page";
			case "book:10": return "made the book balance";
			case "book:11": return "sold the book by the page";
			case "book:12": return "sent the book off loud";
			case "book:13": return "put the book in Bastion's file";
			case "book:14": return "left the book not balancing";
			case "cargo:0": return "had the cargo crew purged";
			case "cargo:1": return "took a cut of the cargo";
			case "cargo:2": return "brought Bastion's raid down on the cargo";
			case "cargo:3": return "freed the cargo and told the Haven";
			case "well:0": return "made the well story work";
			case "well:1": return "did the well";
			case "well:2": return "faked the well";
			case "well:3": return "stood up for the well";

			// Halcyon
			case "weeks:0": return "kept the three weeks in the record";
			case "weeks:1": return "put the three weeks on the open channel";
			case "weeks:2": return "gave the three weeks to Bastion";
			case "other_two:0": return "put names on the other two";
			case "other_two:1": return "brought the other two home";
			case "other_two:2": return "kept the work on the other two going";
			case "other_two:3": return "sold the other two";
			case "authors:0": return "put the lab under Bastion's guard";
			case "authors:1": return "opened the lab's doors";
			case "authors:2": return "got the Red Hand its batch";
			case "authors:3": return "left the lab to stand alone";
			case "proof:10": return "put the whole of it on paper";
			case "proof:11": return "put the paper under seal";
			case "proof:12": return "kept the paper at Day 1";
			case "proof:13": return "left the paper unsigned";
			case "proof:14": return "sold the paper at two tins a copy";

			// The Long Table
			case "table:0": return "walked the farm's family out";
			case "table:1": return "burnt the farm";
			case "table:2": return "gave the farm to the lab";
			case "table:3": return "paid the farm to go";
			case "table:4": return "ended the farm by hand";
			case "place:1": return "sat at the long table and came back with proof";

			// The finale's routes
			case "route:10": return "brought the cure home";
			case "route:11": return "finished the cure and lost it";
			case "route:13": return "handed the cure over";
		}

		return "";
	}
}
