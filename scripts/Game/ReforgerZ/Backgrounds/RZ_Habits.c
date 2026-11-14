//! Habits (docs/DESIGN.md, "Habits"): one small thing a character is known for, picked at the
//! opening's door in the contact's first talk (Guidance/RZ_Openings.c). Small and capped, a
//! ReforgerZ effect (not a perk tree), shown on the Standing page. It belongs to the character it was
//! picked by (RZ_OpeningCase.CharacterStamp, the one "this character" helper): a new character asks
//! again, except at a veteran's door, which keeps the last one's. Server only, except the names.

//------------------------------------------------------------------------------------------------
enum RZ_EPlayerHabit
{
	NONE,
	STEADY,		// Steady hands: holds at work sites 10% shorter
	LIGHT,		// Light feet: 10% quieter while crouched
	LIAR		// A good liar: one persuade per conversation one step easier
}

//------------------------------------------------------------------------------------------------
class RZ_Habits
{
	static const float STEADY_SPEED = 1.11;		// a hold's seconds / this: 10% shorter
	static const float LIGHT_CROUCH = 0.9;		// crouched visibility x this: 10% quieter

	//------------------------------------------------------------------------------------------------
	//! This character's habit, NONE if it hasn't one (or it was an earlier character's).
	static int Of(RZ_PlayerProfile profile)
	{
		if (!profile || profile.m_iHabit == RZ_EPlayerHabit.NONE)
			return RZ_EPlayerHabit.NONE;

		if (profile.m_iHabitLife != RZ_OpeningCase.CharacterStamp(profile))
			return RZ_EPlayerHabit.NONE;

		return profile.m_iHabit;
	}

	//------------------------------------------------------------------------------------------------
	static int OfPlayer(int playerId)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return RZ_EPlayerHabit.NONE;

		return Of(gm.GetProfile(playerId));
	}

	//------------------------------------------------------------------------------------------------
	static void Set(RZ_PlayerProfile profile, int habit)
	{
		if (!profile)
			return;

		profile.m_iHabit = habit;
		profile.m_iHabitLife = RZ_OpeningCase.CharacterStamp(profile);
	}

	//------------------------------------------------------------------------------------------------
	//! A veteran's door asks nothing: the last character's habit carries to this one. False: none to carry.
	static bool Carry(RZ_PlayerProfile profile)
	{
		if (!profile || profile.m_iHabit == RZ_EPlayerHabit.NONE)
			return false;

		profile.m_iHabitLife = RZ_OpeningCase.CharacterStamp(profile);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A work site's hold for this player: its seconds are divided by this (1: as it is).
	static float HoldSpeed(int playerId)
	{
		if (OfPlayer(playerId) == RZ_EPlayerHabit.STEADY)
			return STEADY_SPEED;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	//! Crouched, this player's stealth visibility is multiplied by this (1: as it is).
	static float CrouchMult(int playerId)
	{
		if (OfPlayer(playerId) == RZ_EPlayerHabit.LIGHT)
			return LIGHT_CROUCH;

		return 1;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsLiar(int playerId)
	{
		return OfPlayer(playerId) == RZ_EPlayerHabit.LIAR;
	}

	//------------------------------------------------------------------------------------------------
	static string Name(int habit)
	{
		switch (habit)
		{
			case RZ_EPlayerHabit.STEADY: return "Steady hands";
			case RZ_EPlayerHabit.LIGHT: return "Light feet";
			case RZ_EPlayerHabit.LIAR: return "A good liar";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! What it does, for the Standing page.
	static string Effect(int habit)
	{
		switch (habit)
		{
			case RZ_EPlayerHabit.STEADY: return "holds at work 10% shorter";
			case RZ_EPlayerHabit.LIGHT: return "10% quieter crouched";
			case RZ_EPlayerHabit.LIAR: return "one persuade a talk is easier";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The Standing page's line: "HABIT: STEADY HANDS (holds at work 10% shorter)"; "" with none.
	static string StandingLine(RZ_PlayerProfile profile)
	{
		int habit = Of(profile);
		if (habit == RZ_EPlayerHabit.NONE)
			return "";

		string name = Name(habit);
		name.ToUpper();
		return string.Format("HABIT: %1 (%2)", name, Effect(habit));
	}
}
