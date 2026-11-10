//! Bites and infection (docs/DESIGN.md, "Zombies, bites and infection"). Server only, owned by
//! the game mode. What a bite does from the moment RZ_GameModeComponent.InfectPlayer() is called:
//!
//!  1. The golden window (2 min): the bite hasn't taken yet and the timer stands still. Cleaning
//!     it now (alcohol or antiseptic ~70%, disinfectant and a bandage ~90%, a treatment by another
//!     player ~90%) stops it. One try per bite. A "Clean the bite" spot follows the bitten player.
//!  2. It takes hold, in three stages over about an hour (x the background and cure multipliers):
//!     Fever (mild), Weakness (stamina drains, others can see it), Final (minutes left).
//!  3. Treatments: an antidote cures, Halcyon's trial dose cures 80% of the time, a stabiliser
//!     pauses the timer for 20 min, and five low-grade meds (each once per life, +30 min at most
//!     between them) buy time with side effects. Only an antidote, a lucky trial dose or a lucky
//!     Red Hand cut dose cures.
//!
//! Stamina, sway and blur go through RZ_Symptoms (engine hooks still to be checked in Workbench); without them the
//! effect falls back to noise or a little health drain.

//------------------------------------------------------------------------------------------------
enum RZ_EInfectionStage
{
	NONE,
	FEVER,
	WEAKNESS,
	FINAL
}

//------------------------------------------------------------------------------------------------
//! Treatments that are items. ANTIDOTE is only used for search finds (its prefab is Cure > Antidote,
//! and it's used through RZ_InjectAntidoteAction).
enum RZ_EMed
{
	STABILISER,
	ANTIBIOTICS,	// expired antibiotics
	VET_ANTIVIRAL,
	CUT_DOSE,		// the Red Hand's cut dose
	SUPPRESSANT,	// fever suppressant
	REMEDY,			// home remedy
	ANTIDOTE
}

//------------------------------------------------------------------------------------------------
//! One chance to find a treatment when searching a type of building (a loot table entry).
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sType")]
class RZ_MedFind
{
	[Attribute("pharmacy", desc: "Building type: pharmacy (pharmacies and hospitals), garage, farm (vet clinics count), police, shop, house, body (infected bodies, some of them Halcyon couriers) or car")]
	string m_sType = "pharmacy";

	[Attribute("1", UIWidgets.ComboBox, "What is found", "", ParamEnumArray.FromEnum(RZ_EMed))]
	RZ_EMed m_iMed = RZ_EMed.ANTIBIOTICS;

	[Attribute("0.05", desc: "Chance per search (doubled behind a lock)")]
	float m_fChance = 0.05;

	[Attribute("0", desc: "Added to the chance per cure breakthrough (25, 50, 75 percent)")]
	float m_fPerMilestone = 0;

	//------------------------------------------------------------------------------------------------
	static RZ_MedFind Make(string type, RZ_EMed med, float chance, float perMilestone)
	{
		RZ_MedFind f = new RZ_MedFind();
		f.m_sType = type;
		f.m_iMed = med;
		f.m_fChance = chance;
		f.m_fPerMilestone = perMilestone;
		return f;
	}
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Survival > Infection.
[BaseContainerProps()]
class RZ_InfectionSettings
{
	// ---------------------------------------------------------------- Bites
	[Attribute("2", desc: "Bite chance x this with 3 or more infected within 5 m (surrounded)")]
	float m_fSurroundedMult = 2;

	[Attribute("1.5", desc: "Bite chance x this for a second hit within 4 s (grabbed and held)")]
	float m_fGrabbedMult = 1.5;

	[Attribute("1.5", desc: "Bite chance x this with stamina under a quarter (needs the stamina hook)")]
	float m_fTiredMult = 1.5;

	[Attribute("1.5", desc: "Bite chance x this under 40% health")]
	float m_fHurtMult = 1.5;

	[Attribute("0.6", desc: "The bite chance never goes above this")]
	float m_fMaxBiteChance = 0.6;

	[Attribute("jacket,coat,parka,leather,smock,anorak,m65,gorka,hoodie", desc: "Protective jacket keywords (worn clothing prefab names)")]
	string m_sJacketWords = "jacket,coat,parka,leather,smock,anorak,m65,gorka,hoodie";

	[Attribute("0.8", desc: "Bite chance x this wearing a jacket")]
	float m_fJacketMult = 0.8;

	[Attribute("glove,gauntlet", desc: "Glove keywords")]
	string m_sGloveWords = "glove,gauntlet";

	[Attribute("0.8", desc: "Bite chance x this wearing gloves")]
	float m_fGloveMult = 0.8;

	[Attribute("helmet,helm_,_helm,ssh68,pasgt,motorcycle", desc: "Helmet keywords")]
	string m_sHelmetWords = "helmet,helm_,_helm,ssh68,pasgt,motorcycle";

	[Attribute("0.85", desc: "Bite chance x this wearing a helmet")]
	float m_fHelmetMult = 0.85;

	// ---------------------------------------------------------------- The golden window and the stages
	[Attribute("120", desc: "Golden window: seconds before a bite takes hold, while cleaning it can stop it. 0 = bites take hold at once")]
	float m_fGoldenSeconds = 120;

	[Attribute("vodka,whisky,whiskey,alcohol,spirit,moonshine,brandy,slivovic,liquor,rum_bottle", desc: "Alcohol keywords (cleaning a bite, the home remedy)")]
	string m_sAlcoholWords = "vodka,whisky,whiskey,alcohol,spirit,moonshine,brandy,slivovic,liquor,rum_bottle";

	[Attribute("antiseptic,iodine,betadine,swab,peroxide,disinfect,chlorhex", desc: "Antiseptic and disinfectant keywords")]
	string m_sAntisepticWords = "antiseptic,iodine,betadine,swab,peroxide,disinfect,chlorhex";

	[Attribute("0.7", desc: "Chance that cleaning a fresh bite with alcohol or antiseptic stops it")]
	float m_fCleanChance = 0.7;

	[Attribute("0.9", desc: "Chance that disinfectant and a bandage (or a first-aid kit, or another player's treatment) stops it")]
	float m_fDisinfectChance = 0.9;

	[Attribute("6", desc: "Seconds cleaning a bite takes (stay close)")]
	float m_fCleanSeconds = 6;

	[Attribute("10", desc: "Final stage: the last this many minutes (at most 30% of the whole infection)")]
	float m_fFinalMinutes = 10;

	// ---------------------------------------------------------------- Treatments
	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Stabiliser: pauses the infection. An item with RZ_MedItemComponent (STABILISER) and RZ_UseMedAction. Halcyon's quartermaster sells it from a stock (Known); rare in pharmacies and hospitals", "et")]
	ResourceName m_sStabiliserPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Expired antibiotics (ANTIBIOTICS): common in pharmacies and houses; every safehouse hands them out", "et")]
	ResourceName m_sAntibioticsPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Veterinary antiviral (VET_ANTIVIRAL): farms and vet clinics", "et")]
	ResourceName m_sVetAntiviralPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Red Hand cut dose (CUT_DOSE): give the Red Hand quartermaster a shop action for it (Known, cheap)", "et")]
	ResourceName m_sCutDosePrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Fever suppressant (SUPPRESSANT): pharmacies", "et")]
	ResourceName m_sSuppressantPrefab;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Home remedy (REMEDY): brewed on a stove from alcohol and herbs", "et")]
	ResourceName m_sRemedyPrefab;

	[Attribute("20", desc: "Minutes a stabiliser pauses the infection")]
	float m_fStabiliserMinutes = 20;

	[Attribute("30", desc: "Most minutes the low-grade meds add between them in one life (a stabiliser doesn't count)")]
	float m_fMedCapMinutes = 30;

	[Attribute("0.8", desc: "Halcyon's trial dose: chance it cures (else the turn comes twice as fast)")]
	float m_fTrialCureChance = 0.8;

	[Attribute("3", desc: "Halcyon Fame for taking the trial dose, whatever happens")]
	int m_iTrialFame = 3;

	[Attribute("0.5", desc: "Red Hand cut dose: chance it cures (else the turn comes faster)")]
	float m_fCutDoseCureChance = 0.5;

	[Attribute("0.3", desc: "Expired antibiotics: chance they do nothing at all")]
	float m_fAntibioticsFailChance = 0.3;

	[Attribute("herb,chamomile,camomile,mint,nettle,garlic,teabag,tea_bag,thyme", desc: "Herb keywords (the home remedy)")]
	string m_sHerbWords = "herb,chamomile,camomile,mint,nettle,garlic,teabag,tea_bag,thyme";

	[Attribute("20", desc: "Seconds brewing a home remedy on a stove")]
	float m_fBrewSeconds = 20;

	[Attribute(desc: "Where treatments are found when searching (empty = the built-in list in docs/DESIGN.md)")]
	ref array<ref RZ_MedFind> m_aFinds;
}

//------------------------------------------------------------------------------------------------
//! Timers for one sick player (not saved: a restart just restarts the rhythm).
class RZ_SickState
{
	float m_fNextSelf;
	float m_fNextDrain;
	float m_fNextShow;
	float m_fNextSide;
	int m_iWoundSpot = -1;
}

//------------------------------------------------------------------------------------------------
class RZ_Infection
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_InfectionSettings m_Settings;
	protected ref array<ref RZ_MedFind> m_aFinds = {};
	protected ref map<int, ref RZ_SickState> m_mStates = new map<int, ref RZ_SickState>();
	protected float m_fClock;

	protected static const float WOUND_FOLLOW = 4;			// the bite spot moves with the player past this (m)
	protected static const float SHOW_RADIUS = 15;			// others see the Weakness and Final stages this close
	protected static const float SIDE_EFFECT_SECONDS = 600;
	protected static const float SUPPRESS_SECONDS = 1800;
	protected static const float FEVER_FLOOR = 0.3;			// the cut dose's fever never takes health below this share

	//------------------------------------------------------------------------------------------------
	void RZ_Infection(RZ_GameModeComponent gm, RZ_InfectionSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		if (m_Settings.m_aFinds && !m_Settings.m_aFinds.IsEmpty())
		{
			foreach (RZ_MedFind f : m_Settings.m_aFinds)
			{
				if (f)
					m_aFinds.Insert(f);
			}
		}
		else
		{
			DefaultFinds(m_aFinds);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_InfectionSettings GetSettings() { return m_Settings; }

	//------------------------------------------------------------------------------------------------
	//! The built-in find list (docs/DESIGN.md, "Treatments").
	static void DefaultFinds(notnull array<ref RZ_MedFind> list)
	{
		list.Insert(RZ_MedFind.Make("pharmacy", RZ_EMed.ANTIBIOTICS, 0.12, 0));
		list.Insert(RZ_MedFind.Make("house", RZ_EMed.ANTIBIOTICS, 0.03, 0));
		list.Insert(RZ_MedFind.Make("body", RZ_EMed.ANTIBIOTICS, 0.02, 0));
		list.Insert(RZ_MedFind.Make("pharmacy", RZ_EMed.SUPPRESSANT, 0.06, 0));
		list.Insert(RZ_MedFind.Make("shop", RZ_EMed.SUPPRESSANT, 0.02, 0));
		list.Insert(RZ_MedFind.Make("farm", RZ_EMed.VET_ANTIVIRAL, 0.06, 0));
		list.Insert(RZ_MedFind.Make("pharmacy", RZ_EMed.STABILISER, 0.01, 0.005));
		list.Insert(RZ_MedFind.Make("pharmacy", RZ_EMed.ANTIDOTE, 0.004, 0.004));
		list.Insert(RZ_MedFind.Make("body", RZ_EMed.ANTIDOTE, 0.002, 0.002));
	}

	//------------------------------------------------------------------------------------------------
	ResourceName MedPrefab(int kind)
	{
		switch (kind)
		{
			case RZ_EMed.STABILISER: return m_Settings.m_sStabiliserPrefab;
			case RZ_EMed.ANTIBIOTICS: return m_Settings.m_sAntibioticsPrefab;
			case RZ_EMed.VET_ANTIVIRAL: return m_Settings.m_sVetAntiviralPrefab;
			case RZ_EMed.CUT_DOSE: return m_Settings.m_sCutDosePrefab;
			case RZ_EMed.SUPPRESSANT: return m_Settings.m_sSuppressantPrefab;
			case RZ_EMed.REMEDY: return m_Settings.m_sRemedyPrefab;
			case RZ_EMed.ANTIDOTE: return m_GM.GetAntidotePrefab();
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	static string MedName(int kind)
	{
		switch (kind)
		{
			case RZ_EMed.STABILISER: return "a stabiliser";
			case RZ_EMed.ANTIBIOTICS: return "expired antibiotics";
			case RZ_EMed.VET_ANTIVIRAL: return "a veterinary antiviral";
			case RZ_EMed.CUT_DOSE: return "a Red Hand cut dose";
			case RZ_EMed.SUPPRESSANT: return "a fever suppressant";
			case RZ_EMed.REMEDY: return "a home remedy";
			case RZ_EMed.ANTIDOTE: return "an antidote";
		}

		return "something";
	}

	//------------------------------------------------------------------------------------------------
	//! Guidance tip id for a med's first use.
	protected static string MedTip(int kind)
	{
		switch (kind)
		{
			case RZ_EMed.STABILISER: return "med_stabiliser";
			case RZ_EMed.ANTIBIOTICS: return "med_antibiotics";
			case RZ_EMed.VET_ANTIVIRAL: return "med_vet";
			case RZ_EMed.CUT_DOSE: return "med_cutdose";
			case RZ_EMed.SUPPRESSANT: return "med_suppressant";
			case RZ_EMed.REMEDY: return "med_remedy";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_SickState State(int playerId)
	{
		RZ_SickState st;
		if (!m_mStates.Find(playerId, st))
		{
			st = new RZ_SickState();
			m_mStates.Set(playerId, st);
		}

		return st;
	}

	//------------------------------------------------------------------------------------------------
	//! A new life: the old body's timers and bite spot go.
	void OnPlayerSpawned(int playerId)
	{
		RemoveWoundSpot(playerId);
		m_mStates.Remove(playerId);
	}

	// ================================================================ The bite
	//------------------------------------------------------------------------------------------------
	//! From RZ_GameModeComponent.InfectPlayer, after the timer is set: the golden window opens.
	void OnBite(int playerId, RZ_PlayerProfile profile)
	{
		if (m_Settings.m_fGoldenSeconds <= 0)
		{
			TakeHold(playerId, profile);
			return;
		}

		profile.m_fWoundLeft = m_Settings.m_fGoldenSeconds;
		profile.m_bWoundCleaned = false;
		m_GM.NotifyPlayer(playerId, "Bitten", string.Format("It got through. Teeth, not nails. It hasn't taken yet: you have about %1 minutes to clean it. Alcohol or antiseptic might do it; disinfectant and a bandage are better. Clean the bite, now.",
			Math.Max(1, RZ_Util.RoundToInt(m_Settings.m_fGoldenSeconds / 60))));
		RZ_Guidance.Report(playerId, "clean");
		PlaceWoundSpot(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The golden window closed without a clean: the stages start.
	protected void TakeHold(int playerId, RZ_PlayerProfile profile)
	{
		profile.m_fWoundLeft = -1;
		profile.m_iInfectionStage = RZ_EInfectionStage.FEVER;
		RemoveWoundSpot(playerId);
		m_GM.NotifyPlayer(playerId, "Infected", string.Format("It's taken. The bite is hot and the skin around it has gone grey. You have about %1 minutes. Fever first, then weakness, then the end. An antidote cures it; Halcyon's lab runs a trial dose for the bitten; a safehouse will slow it if you tell them.",
			RZ_Util.RoundToInt(profile.m_fInfectionLeft / 60)));

		// Whoever is nearest says something about it (always at least a short line)
		if (m_GM.GetGuidance())
			m_GM.GetGuidance().OnBitten(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Cured (antidote, trial dose, cut dose, a clean in time) or turned.
	void OnCured(int playerId)
	{
		RemoveWoundSpot(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Where in the infection a player is.
	RZ_EInfectionStage StageOf(RZ_PlayerProfile p)
	{
		if (!p.HasInfectionTaken())
			return RZ_EInfectionStage.NONE;

		float total = Math.Max(60, p.m_fInfectionTotal);
		float finalSeconds = Math.Min(m_Settings.m_fFinalMinutes * 60, total * 0.3);
		if (p.m_fInfectionLeft <= finalSeconds)
			return RZ_EInfectionStage.FINAL;

		if (p.m_fInfectionLeft <= total * 0.5)
			return RZ_EInfectionStage.WEAKNESS;

		return RZ_EInfectionStage.FEVER;
	}

	//------------------------------------------------------------------------------------------------
	//! Others can see it (Weakness or Final, and no fever suppressant): safehouses may catch a
	//! hidden bite. 0 = no, 1 = Weakness, 2 = Final.
	int VisibleSickness(RZ_PlayerProfile p)
	{
		if (p.m_fSymptomsHiddenLeft > 0)
			return 0;

		RZ_EInfectionStage stage = StageOf(p);
		if (stage == RZ_EInfectionStage.FINAL)
			return 2;

		if (stage == RZ_EInfectionStage.WEAKNESS)
			return 1;

		return 0;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		foreach (int i, IEntity ent : players)
		{
			int playerId = ids[i];
			RZ_PlayerProfile p = m_GM.GetProfile(playerId);
			TickSideEffect(dt, playerId, p, ent);
			if (p.m_fSymptomsHiddenLeft > 0)
				p.m_fSymptomsHiddenLeft = Math.Max(0, p.m_fSymptomsHiddenLeft - dt);

			if (!p.IsInfected())
				continue;

			if (p.m_fWoundLeft > 0)
			{
				p.m_fWoundLeft = p.m_fWoundLeft - dt;
				if (p.m_fWoundLeft <= 0)
					TakeHold(playerId, p);
				else
					FollowWound(playerId, ent, p);

				continue;
			}

			if (p.m_fPauseLeft > 0)
			{
				p.m_fPauseLeft = Math.Max(0, p.m_fPauseLeft - dt);
				if (p.m_fPauseLeft <= 0)
					m_GM.NotifyPlayer(playerId, "Infection", "Whatever was holding it back has worn off. The clock is running again.");
			}
			else
			{
				p.m_fInfectionLeft = p.m_fInfectionLeft - dt;
			}

			if (p.m_fInfectionLeft <= 0)
			{
				p.ClearInfection();
				OnCured(playerId);
				m_GM.TurnPlayer(playerId);
				continue;
			}

			RZ_EInfectionStage stage = StageOf(p);
			if (stage != p.m_iInfectionStage)
			{
				p.m_iInfectionStage = stage;
				OnStageChanged(playerId, p, stage);
			}

			TickSymptoms(playerId, p, ent, stage);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnStageChanged(int playerId, RZ_PlayerProfile p, RZ_EInfectionStage stage)
	{
		int minutes = Math.Max(1, RZ_Util.RoundToInt(p.m_fInfectionLeft / 60));
		switch (stage)
		{
			case RZ_EInfectionStage.FEVER:
				m_GM.NotifyPlayer(playerId, "Fever", string.Format("The fever's back down to a simmer. About %1 minutes left.", minutes));
				break;
			case RZ_EInfectionStage.WEAKNESS:
				m_GM.NotifyPlayer(playerId, "Weakness", string.Format("Your legs are heavy and your hands won't stop shaking. You run out of breath on the stairs. Anyone who looks at you can see it now. About %1 minutes.", minutes));
				break;
			case RZ_EInfectionStage.FINAL:
				m_GM.NotifyPlayer(playerId, "The end of it", string.Format("Minutes, not hours. About %1. If there's an antidote anywhere near, this is the time.", minutes));
				break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What each stage does to the body, and what others see.
	protected void TickSymptoms(int playerId, RZ_PlayerProfile p, IEntity ent, RZ_EInfectionStage stage)
	{
		RZ_SickState st = State(playerId);
		bool hidden = p.m_fSymptomsHiddenLeft > 0;
		vector pos = ent.GetOrigin();

		if (stage == RZ_EInfectionStage.FEVER)
		{
			if (m_fClock >= st.m_fNextSelf)
			{
				st.m_fNextSelf = m_fClock + 240;
				if (!hidden)
					m_GM.NotifyPlayer(playerId, "Fever", "You're sweating in the cold. Everything is a little too bright.");
			}

			return;
		}

		float drainEvery = 30;
		float drain = 0.15;
		float noise = 2;
		float showEvery = 180;
		if (stage == RZ_EInfectionStage.FINAL)
		{
			drainEvery = 20;
			drain = 0.25;
			noise = 3;
			showEvery = 120;
		}

		// Stamina drains; without the hook, laboured breathing and stumbling make a little noise
		if (m_fClock >= st.m_fNextDrain)
		{
			st.m_fNextDrain = m_fClock + drainEvery;
			if (!RZ_Symptoms.DrainStamina(ent, drain))
				m_GM.GetHeat().AddHeat(pos, noise);
		}

		if (m_fClock >= st.m_fNextSelf)
		{
			if (stage == RZ_EInfectionStage.FINAL)
			{
				st.m_fNextSelf = m_fClock + 60;
				m_GM.NotifyPlayer(playerId, "The end of it", string.Format("About %1 minutes. Your hands don't feel like yours.", Math.Max(1, RZ_Util.RoundToInt(p.m_fInfectionLeft / 60))));
			}
			else
			{
				st.m_fNextSelf = m_fClock + 240;
				string selfLine = "Every step costs more than the last.";
				if (hidden)
					selfLine = "The pills keep the fever down. You feel almost normal. You're not.";

				m_GM.NotifyPlayer(playerId, "Weakness", selfLine);
			}
		}

		if (hidden || m_fClock < st.m_fNextShow)
			return;

		st.m_fNextShow = m_fClock + showEvery;
		string seen = "%1 looks grey and unsteady, sweating through their clothes. They keep holding one arm.";
		if (stage == RZ_EInfectionStage.FINAL)
			seen = "%1 is burning up. Grey lips, a wet cough, eyes that don't quite follow you. Whatever it is, it's nearly done.";

		NotifyOthersNear(pos, SHOW_RADIUS, playerId, "Someone's sick", string.Format(seen, RZ_Util.PlayerName(playerId)));
	}

	//------------------------------------------------------------------------------------------------
	protected void NotifyOthersNear(vector pos, float radius, int except, string title, string text)
	{
		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		foreach (int i, IEntity other : players)
		{
			if (ids[i] != except && RZ_Util.DistXZ(other.GetOrigin(), pos) <= radius)
				m_GM.NotifyPlayer(ids[i], title, text);
		}
	}

	// ================================================================ Side effects
	//------------------------------------------------------------------------------------------------
	protected void StartSideEffect(RZ_PlayerProfile p, int kind, float seconds)
	{
		p.m_iSideEffect = kind;
		p.m_fSideEffectLeft = seconds;
	}

	//------------------------------------------------------------------------------------------------
	protected void TickSideEffect(float dt, int playerId, RZ_PlayerProfile p, IEntity ent)
	{
		if (p.m_iSideEffect < 0)
			return;

		p.m_fSideEffectLeft = p.m_fSideEffectLeft - dt;
		if (p.m_fSideEffectLeft <= 0)
		{
			p.m_iSideEffect = -1;
			m_GM.NotifyPlayer(playerId, "Better", "The worst of the side effects has passed.");
			return;
		}

		RZ_SickState st = State(playerId);
		if (m_fClock < st.m_fNextSide)
			return;

		vector pos = ent.GetOrigin();
		switch (p.m_iSideEffect)
		{
			case RZ_EMed.ANTIBIOTICS:
			{
				// Nausea: bursts of lost stamina (else a little health)
				st.m_fNextSide = m_fClock + 90;
				if (!RZ_Symptoms.DrainStamina(ent, 0.4))
					DrainHealth(playerId, ent, 0.02, 0.5);

				m_GM.NotifyPlayer(playerId, "Nausea", "Your stomach turns over and you have to stop, hands on your knees.");
				break;
			}
			case RZ_EMed.VET_ANTIVIRAL:
			{
				// Coughing fits: noise
				st.m_fNextSide = m_fClock + Math.RandomFloat(60, 90);
				m_GM.GetHeat().AddHeat(pos, 12);
				m_GM.NotifyPlayer(playerId, "Coughing", "A coughing fit doubles you over. Loud. Anything nearby heard that.");
				break;
			}
			case RZ_EMed.CUT_DOSE:
			{
				// Fever: a slow health drain, never below a floor
				st.m_fNextSide = m_fClock + 30;
				DrainHealth(playerId, ent, 0.015, FEVER_FLOOR);
				break;
			}
			case RZ_EMed.REMEDY:
			{
				// Drunk: sway and blur where the engine allows, else stumbling (stamina and noise)
				st.m_fNextSide = m_fClock + 30;
				if (!RZ_Symptoms.Disorient(ent))
				{
					RZ_Symptoms.DrainStamina(ent, 0.2);
					m_GM.GetHeat().AddHeat(pos, 3);
				}

				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Takes `share` of full health, never below `floorShare` of it. The bite detector forgets the
	//! old value, so the drop doesn't read as a hit.
	protected void DrainHealth(int playerId, IEntity ent, float share, float floorShare)
	{
		float full = RZ_Util.GetCharacterMaxHealth(ent);
		float hp = RZ_Util.GetCharacterHealth(ent);
		if (full <= 0 || hp < 0)
			return;

		float target = Math.Max(full * floorShare, hp - full * share);
		if (target >= hp)
			return;

		if (RZ_Util.SetCharacterHealth(ent, target))
			m_GM.ForgetBiteHealth(playerId);
	}

	// ================================================================ Cleaning a fresh bite
	//------------------------------------------------------------------------------------------------
	protected void PlaceWoundSpot(int playerId)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		IEntity ent = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		if (!hub || !hub.IsActive() || !ent || !p.IsWoundFresh())
			return;

		RZ_SickState st = State(playerId);
		if (st.m_iWoundSpot >= 0)
			hub.RemoveSpot(st.m_iWoundSpot);

		st.m_iWoundSpot = hub.AddSpot(RZ_Interactions.KIND_WOUND, ent.GetOrigin(), "wound", "the bite", playerId, hub.GetClock() + p.m_fWoundLeft);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveWoundSpot(int playerId)
	{
		RZ_SickState st;
		if (!m_mStates.Find(playerId, st) || st.m_iWoundSpot < 0)
			return;

		RZ_Interactions hub = RZ_Interactions.Get();
		if (hub)
			hub.RemoveSpot(st.m_iWoundSpot);

		st.m_iWoundSpot = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The "Clean the bite" spot stays at the bitten player's feet.
	protected void FollowWound(int playerId, IEntity ent, RZ_PlayerProfile p)
	{
		if (p.m_bWoundCleaned)
			return;

		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub || !hub.IsActive())
			return;

		RZ_SickState st = State(playerId);
		RZ_Spot s = hub.FindSpot(st.m_iWoundSpot);
		if (s && RZ_Util.DistXZ(s.m_vPos, ent.GetOrigin()) <= WOUND_FOLLOW)
			return;

		PlaceWoundSpot(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Labels on the bite spot.
	void WoundLabels(out string read, out string choiceLabels)
	{
		read = "Look at the bite";
		choiceLabels = "Clean the bite (alcohol or antiseptic)|Disinfect and bandage it";
	}

	//------------------------------------------------------------------------------------------------
	void DescribeWound(int playerId, RZ_Spot s)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(s.m_iRef);
		string text = "A ragged bite, still bleeding. It hasn't taken yet. Wash it out: alcohol, antiseptic, or better, disinfectant and a bandage.";
		if (!p.IsWoundFresh())
			text = "Too late to wash it out. It's taken.";
		else if (p.m_bWoundCleaned)
			text = "Cleaned and dressed. Now you wait.";

		m_GM.NotifyPlayer(playerId, "The bite", text);
	}

	//------------------------------------------------------------------------------------------------
	//! "Clean the bite" (0: alcohol or antiseptic) or "Disinfect and bandage it" (1). Anyone can do it
	//! for the bitten player, with their own supplies.
	void StartClean(int playerId, RZ_Spot s, int option, IEntity user)
	{
		int patient = s.m_iRef;
		RZ_PlayerProfile p = m_GM.GetProfile(patient);
		if (!p.IsWoundFresh())
		{
			m_GM.NotifyPlayer(playerId, "The bite", "Too late to wash it out. It's taken.");
			return;
		}

		if (p.m_bWoundCleaned)
		{
			m_GM.NotifyPlayer(playerId, "The bite", "It's been cleaned already. There's nothing more to wash out.");
			return;
		}

		if (!HasCleaningSupplies(user, option))
		{
			string need = "You need something with alcohol in it, or antiseptic.";
			if (option == 1)
				need = "You need disinfectant or antiseptic and a bandage (or a first-aid kit).";

			m_GM.NotifyPlayer(playerId, "The bite", need);
			return;
		}

		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub)
			return;

		hub.StartTask(playerId, user, RZ_Interactions.TASK_CLEAN, s.m_iId, patient * 2 + option, m_Settings.m_fCleanSeconds, "cleaning the bite", false);
		m_GM.NotifyPlayer(playerId, "The bite", string.Format("Hold still. About %1 seconds. This will hurt.", RZ_Util.RoundToInt(m_Settings.m_fCleanSeconds)));
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasCleaningSupplies(IEntity user, int option)
	{
		RZ_InteractionSettings items = InteractionSettings();
		if (option == 1)
		{
			if (!items)
				return false;

			if (RZ_ItemHelper.Has(user, items.m_sMedkitWords))
				return true;

			return RZ_ItemHelper.Has(user, m_Settings.m_sAntisepticWords) && RZ_ItemHelper.Has(user, items.m_sBandageWords);
		}

		return RZ_ItemHelper.Has(user, m_Settings.m_sAlcoholWords) || RZ_ItemHelper.Has(user, m_Settings.m_sAntisepticWords);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_InteractionSettings InteractionSettings()
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		if (!hub)
			return null;

		return hub.GetSettings();
	}

	//------------------------------------------------------------------------------------------------
	//! The timed clean finished. arg = patient * 2 + option.
	void CompleteClean(int playerId, int arg, IEntity user)
	{
		int patient = arg / 2;
		int option = arg - patient * 2;
		RZ_PlayerProfile p = m_GM.GetProfile(patient);
		if (!p.IsWoundFresh() || p.m_bWoundCleaned)
			return;

		if (!HasCleaningSupplies(user, option))
		{
			m_GM.NotifyPlayer(playerId, "The bite", "What you needed is gone from your pack.");
			return;
		}

		float chance = m_Settings.m_fCleanChance;
		RZ_InteractionSettings items = InteractionSettings();
		if (option == 1)
		{
			chance = m_Settings.m_fDisinfectChance;
			if (items && RZ_ItemHelper.Take(user, items.m_sMedkitWords, 1) <= 0)
			{
				RZ_ItemHelper.Take(user, m_Settings.m_sAntisepticWords, 1);
				RZ_ItemHelper.Take(user, items.m_sBandageWords, 1);
			}
		}
		else if (RZ_ItemHelper.Take(user, m_Settings.m_sAntisepticWords, 1) <= 0)
		{
			RZ_ItemHelper.Take(user, m_Settings.m_sAlcoholWords, 1);
		}

		CleanRoll(patient, p, chance, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Another player's "Treat their wounds" on a fresh bite counts as a proper clean.
	//! True if it was a fresh bite not yet cleaned (then the patient and helper have been told).
	bool CleanByTreatment(int patient, int helper)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(patient);
		if (!p.IsWoundFresh() || p.m_bWoundCleaned)
			return false;

		CleanRoll(patient, p, m_Settings.m_fDisinfectChance, helper);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A safehouse a fresh bite is declared at washes it out properly.
	bool CleanAtSafehouse(int patient)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(patient);
		if (!p.IsWoundFresh() || p.m_bWoundCleaned)
			return false;

		CleanRoll(patient, p, m_Settings.m_fDisinfectChance, -1);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One try per bite. Nobody is told whether it worked until the window closes.
	protected void CleanRoll(int patient, RZ_PlayerProfile p, float chance, int helper)
	{
		p.m_bWoundCleaned = true;
		RemoveWoundSpot(patient);
		m_GM.NotifyPlayer(patient, "The bite", "Scrubbed raw and dressed. It burns like hell. Whatever was in it, you may have got it out in time. You'll know in a minute or two.");
		if (Math.RandomFloat01() < chance)
		{
			// The bite never takes. The player finds out a minute later, as if the window had closed
			p.ClearInfection();
			GetGame().GetCallqueue().CallLater(ToldClean, 60000, false, patient);
		}

		if (helper > 0 && helper != patient)
			m_GM.NotifyPlayer(helper, "The bite", string.Format("You clean %1's bite as well as anyone could. Now they wait.", RZ_Util.PlayerName(patient)));
	}

	//------------------------------------------------------------------------------------------------
	protected void ToldClean(int patient)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(patient);
		if (!p || p.IsInfected())
			return;

		m_GM.NotifyPlayer(patient, "The bite", "The heat's gone out of it. Just a bite now: sore, ugly, clean. It didn't take.");
	}

	// ================================================================ Meds
	//------------------------------------------------------------------------------------------------
	//! A treatment item is used (RZ_UseMedAction or RZ_ConsumableMed). True if it was taken (the
	//! item goes); false with a message if it wasn't.
	bool UseMed(int playerId, int kind)
	{
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		string title = MedTitle(kind);
		if (!p.IsInfected())
		{
			m_GM.NotifyPlayer(playerId, title, "You don't feel sick. Better save it.");
			return false;
		}

		if (p.IsWoundFresh())
		{
			m_GM.NotifyPlayer(playerId, title, "It hasn't taken yet. Wash the bite out first; pills won't do anything for an open wound.");
			return false;
		}

		if (p.HasUsedMed(kind))
		{
			m_GM.NotifyPlayer(playerId, title, "You've had that already this time. More would only make you sick.");
			return false;
		}

		p.MarkMedUsed(kind);
		string tip = MedTip(kind);
		if (!tip.IsEmpty())
			RZ_Guidance.Report(playerId, tip);

		switch (kind)
		{
			case RZ_EMed.STABILISER:
			{
				p.m_fPauseLeft = p.m_fPauseLeft + m_Settings.m_fStabiliserMinutes * 60;
				m_GM.NotifyPlayer(playerId, title, string.Format("Cold all the way up the arm, then nothing. The fever stops where it is. It'll hold for about %1 minutes. Use them.", RZ_Util.RoundToInt(m_Settings.m_fStabiliserMinutes)));
				return true;
			}
			case RZ_EMed.ANTIBIOTICS:
			{
				StartSideEffect(p, kind, SIDE_EFFECT_SECONDS);
				if (Math.RandomFloat01() < m_Settings.m_fAntibioticsFailChance)
				{
					m_GM.NotifyPlayer(playerId, title, "Two years out of date, and it shows: nothing changes, except your stomach.");
					return true;
				}

				float gained = Extend(p, 600);
				m_GM.NotifyPlayer(playerId, title, ExtendText(gained, "The fever eases a little. Your stomach doesn't like it."));
				return true;
			}
			case RZ_EMed.VET_ANTIVIRAL:
			{
				float paused = Math.Min(1200, MedRoom(p));
				p.m_fMedExtension = p.m_fMedExtension + paused;
				p.m_fPauseLeft = p.m_fPauseLeft + paused;
				StartSideEffect(p, kind, Math.Max(SIDE_EFFECT_SECONDS, paused));
				if (paused <= 0)
					m_GM.NotifyPlayer(playerId, title, "Meant for cattle. It does nothing for you now except the cough.");
				else
					m_GM.NotifyPlayer(playerId, title, string.Format("Meant for cattle. The fever holds still for about %1 minutes, and your chest fills up. You'll cough.", RZ_Util.RoundToInt(paused / 60)));

				return true;
			}
			case RZ_EMed.CUT_DOSE:
			{
				StartSideEffect(p, kind, SIDE_EFFECT_SECONDS);
				if (Math.RandomFloat01() < m_Settings.m_fCutDoseCureChance)
				{
					m_GM.CurePlayer(playerId);
					m_GM.NotifyPlayer(playerId, title, "Whatever they cut it with, there was antidote in it. The grey goes out of the bite. The fever stays for a while.");
					return true;
				}

				p.m_fInfectionLeft = Math.Max(60, p.m_fInfectionLeft * 0.6);
				m_GM.NotifyPlayer(playerId, title, string.Format("Mostly chalk, and something that makes it worse. The fever roars up. About %1 minutes now.", Math.Max(1, RZ_Util.RoundToInt(p.m_fInfectionLeft / 60))));
				return true;
			}
			case RZ_EMed.SUPPRESSANT:
			{
				p.m_fSymptomsHiddenLeft = SUPPRESS_SECONDS;
				m_GM.NotifyPlayer(playerId, title, "The sweating stops and the colour comes back to your face. Nobody would know. It isn't slowing anything.");
				return true;
			}
			case RZ_EMed.REMEDY:
			{
				StartSideEffect(p, kind, 300);
				float added = Extend(p, 300);
				m_GM.NotifyPlayer(playerId, title, ExtendText(added, "Hot, bitter and mostly alcohol. The world tilts a bit. The fever backs off a little."));
				return true;
			}
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static string MedTitle(int kind)
	{
		string name = MedName(kind);
		if (name.Length() > 2 && name.Substring(0, 2) == "a ")
			name = name.Substring(2, name.Length() - 2);
		else if (name.Length() > 3 && name.Substring(0, 3) == "an ")
			name = name.Substring(3, name.Length() - 3);

		string first = name.Substring(0, 1);
		first.ToUpper(); // in place, like ToLower()
		return first + name.Substring(1, name.Length() - 1);
	}

	//------------------------------------------------------------------------------------------------
	//! Seconds the low-grade meds may still add this life.
	protected float MedRoom(RZ_PlayerProfile p)
	{
		return Math.Max(0, m_Settings.m_fMedCapMinutes * 60 - p.m_fMedExtension);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds up to `seconds` to the time left, within the cap. Returns what was added.
	protected float Extend(RZ_PlayerProfile p, float seconds)
	{
		float added = Math.Min(seconds, MedRoom(p));
		if (added <= 0)
			return 0;

		p.m_fMedExtension = p.m_fMedExtension + added;
		p.m_fInfectionLeft = p.m_fInfectionLeft + added;
		return added;
	}

	//------------------------------------------------------------------------------------------------
	protected static string ExtendText(float added, string text)
	{
		if (added <= 0)
			return "It goes down bitter and does nothing. You've had all the time pills can buy.";

		return string.Format("%1 (+%2 min)", text, Math.Max(1, RZ_Util.RoundToInt(added / 60)));
	}

	// ================================================================ Halcyon's trial dose
	//------------------------------------------------------------------------------------------------
	//! RZ_TrialDoseAction on Halcyon's quartermaster: for anyone bitten, once per life, free. About
	//! 80% cure; otherwise the turn comes twice as fast. A little Halcyon Fame either way.
	void TrialDose(int playerId)
	{
		string title = "Halcyon trial";
		RZ_PlayerProfile p = m_GM.GetProfile(playerId);
		RZ_ETier tier = m_GM.GetTier(playerId, RZ_EFaction.INSTITUTE);
		if (RZ_Reputation.IsHunted(tier))
		{
			m_GM.NotifyPlayer(playerId, title, "\"Not you.\" The guard doesn't even open the door.");
			return;
		}

		if (!p.IsInfected())
		{
			m_GM.NotifyPlayer(playerId, title, "\"The trial is for the bitten. Come back if you need it. I hope you don't.\"");
			return;
		}

		if (p.IsWoundFresh())
		{
			m_GM.NotifyPlayer(playerId, title, "\"It hasn't taken yet. Wash it out, now, properly. Come back if it takes.\"");
			return;
		}

		if (p.m_bTrialDoseTaken)
		{
			m_GM.NotifyPlayer(playerId, title, "\"You've had your dose. A second one would kill you faster than the bite. I'm sorry.\"");
			return;
		}

		p.m_bTrialDoseTaken = true;
		m_GM.AddFame(playerId, RZ_EFaction.INSTITUTE, m_Settings.m_iTrialFame);
		if (Math.RandomFloat01() < m_Settings.m_fTrialCureChance)
		{
			m_GM.CurePlayer(playerId);
			m_GM.NotifyPlayer(playerId, title, "A signature, a needle, an hour on a cot with a researcher writing down your pulse. Then the fever breaks. \"Eight in ten,\" she says. \"You were one of the eight.\"");
			return;
		}

		p.m_fInfectionLeft = Math.Max(60, p.m_fInfectionLeft * 0.5);
		m_GM.NotifyPlayer(playerId, title, string.Format("A signature, a needle, and then it burns. The researcher stops writing. \"I'm sorry. It goes faster now.\" About %1 minutes.",
			Math.Max(1, RZ_Util.RoundToInt(p.m_fInfectionLeft / 60))));
	}

	// ================================================================ Finds
	//------------------------------------------------------------------------------------------------
	//! One roll per search for a treatment in this building type (at most one). Gives it and returns
	//! its name, or "".
	string RollMedFind(string type, bool locked, IEntity user, vector at)
	{
		int milestone = 0;
		if (m_GM.GetCure())
			milestone = Math.Min(3, m_GM.GetCure().m_iMilestone);

		foreach (RZ_MedFind f : m_aFinds)
		{
			if (f.m_sType != type)
				continue;

			float chance = f.m_fChance + f.m_fPerMilestone * milestone;
			if (locked)
				chance = chance * 2;

			if (Math.RandomFloat01() >= chance)
				continue;

			ResourceName prefab = MedPrefab(f.m_iMed);
			if (prefab.IsEmpty() || !RZ_ItemHelper.Give(prefab, user, at))
				continue;

			if (f.m_iMed == RZ_EMed.ANTIDOTE && type == "body")
				return "an antidote in a Halcyon courier's cold box";

			return MedName(f.m_iMed);
		}

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! Engine hooks for what sickness does to the body. Each returns false when the engine call isn't
//! available, and the caller falls back to noise or a little health drain.
class RZ_Symptoms
{
	//------------------------------------------------------------------------------------------------
	//! Takes `share` (0-1) of full stamina. False without a stamina component.
	static bool DrainStamina(IEntity ent, float share)
	{
		if (!ent)
			return false;

		CharacterStaminaComponent stamina = CharacterStaminaComponent.Cast(ent.FindComponent(CharacterStaminaComponent)); // VERIFY: CharacterStaminaComponent on the character (else SCR_CharacterStaminaComponent)
		if (!stamina)
			return false;

		stamina.AddStamina(-share); // VERIFY: AddStamina(float) on the 0-1 scale, server-side and replicated (negative drains)
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Stamina 0-1, -1 if it can't be read.
	static float GetStamina(IEntity ent)
	{
		if (!ent)
			return -1;

		CharacterStaminaComponent stamina = CharacterStaminaComponent.Cast(ent.FindComponent(CharacterStaminaComponent));
		if (!stamina)
			return -1;

		return stamina.GetStamina(); // VERIFY: GetStamina() returns 0-1
	}

	//------------------------------------------------------------------------------------------------
	//! Sway and blur for the drunk home remedy. No script hook is known for camera sway or a blur
	//! effect from the server, so this returns false and the caller uses stamina and noise instead.
	static bool Disorient(IEntity ent)
	{
		return false; // VERIFY: wire a client post-process blur or an aim-sway modifier here if the game version has one
	}
}
