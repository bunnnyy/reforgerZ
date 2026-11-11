//! The voice bank (docs/WRITING.md, "The voice bank"): the single-line pools people say face to face
//! (leaders' openings by standing, people at holdings, their routine asides), as data. Code defaults
//! live in RZ_VoiceDefaults.c, one Line() each, so the offline tools can read them; Voice > Lines adds
//! a line or replaces a default with the same id, and Voice > Disabled switches ids off. A pick reads
//! the season mood (as the speaker's faction feels it), grief near the speaker, and the listener's
//! joke memory: a joke heard in the last 24 real hours is swapped for a plain line, and a pool that
//! runs out of jokes falls back to plain lines. Server only.

//------------------------------------------------------------------------------------------------
enum RZ_ERegister
{
	PLAIN,
	WRY,
	JOKE,
	TAUNT,
	BLEAK,
	SILENCE
}

//------------------------------------------------------------------------------------------------
//! One line in the bank. Made in code with RZ_Util.NewWithDefaults, never `new`.
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sId")]
class RZ_VoiceLine
{
	[Attribute("", desc: "Stable id (haven.opener.warm.1). A line here with a default's id replaces it")]
	string m_sId;

	[Attribute("-1", desc: "Faction: -1 any, 0 the Haven, 1 Bastion, 2 the Red Hand, 3 Halcyon")]
	int m_iFaction;

	[Attribute("opener", desc: "Kind: opener, greeting, person, aside_meal, aside_night, aside_shift, infected, route (after the 90% lock), look_pass, look_caught")]
	string m_sKind;

	[Attribute("", desc: "Manner (openers): unknown, warm, devoted, wary, fond, needed, hostile, owed; empty = any")]
	string m_sManner;

	[Attribute("0", UIWidgets.ComboBox, "Register: plain lines fill gaps; wry, joke and taunt lines follow the mood's joke share and joke memory", "", ParamEnumArray.FromEnum(RZ_ERegister))]
	RZ_ERegister m_eRegister;

	[Attribute("0", desc: "Moods it's said in, a bitmask: 1 Early, 2 Grim, 4 Bleak, 8 Hope, 16 Epilogue. 0 = any")]
	int m_iMoods;

	[Attribute("0", desc: "Leader traits it suits, a bitmask of RZ_ELeaderTrait (1 Greedy, 2 Paranoid, 4 Merciful...). 0 = any")]
	int m_iTraits;

	[Attribute("", desc: "Optional conditions, in the conversation syntax (flag:book.burnt, cure>=50, epilogue). Empty = always")]
	string m_sWhen;

	[Attribute("1", desc: "Weight against the other lines that fit")]
	float m_fWeight;

	[Attribute("", desc: "The line. Tokens: the world's ({day}...), plus {place}")]
	string m_sText;

	ref RZ_DCond m_When;	// compiled once
}

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Voice. Read on the server.
[BaseContainerProps()]
class RZ_VoiceSettings
{
	[Attribute("1", desc: "The season mood: the broadcast and the voice bank read one ladder (Epilogue, Hope, Bleak, Grim, Early) and each faction's temper. Off = the broadcast's built table")]
	bool m_bSeasonMood;

	[Attribute(desc: "Lines to add to the voice bank, or to replace a default with the same id")]
	ref array<ref RZ_VoiceLine> m_aLines;

	[Attribute(desc: "Voice bank ids to switch off")]
	ref array<string> m_aDisabled;

	[Attribute("0", desc: "Debug: log every pick ([RZ] voice: ...)")]
	bool m_bLog;

	[Attribute("0", UIWidgets.ComboBox, "Debug: force the season mood (the runtime test cycles moods it wouldn't reach)", "", ParamEnumArray.FromEnum(RZ_EForceMood))]
	int m_iForceMood;

	protected static ref RZ_VoiceSettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	static RZ_VoiceSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetVoiceSettings())
			return gm.GetVoiceSettings();

		if (!s_Defaults)
			s_Defaults = RZ_VoiceSettings.Cast(RZ_Util.NewWithDefaults("RZ_VoiceSettings"));

		return s_Defaults;
	}
}

//------------------------------------------------------------------------------------------------
enum RZ_EForceMood
{
	OFF,
	EARLY,
	GRIM,
	BLEAK,
	HOPE,
	EPILOGUE
}

//------------------------------------------------------------------------------------------------
class RZ_VoiceBank
{
	protected static const int JOKE_MEMORY = 24;
	protected static const int JOKE_SECONDS = 86400;

	protected static ref array<ref RZ_VoiceLine> s_aLines;

	//------------------------------------------------------------------------------------------------
	//! The defaults, with Voice > Lines over them by id and Voice > Disabled taken out. Built once.
	protected static array<ref RZ_VoiceLine> Lines()
	{
		if (s_aLines)
			return s_aLines;

		s_aLines = new array<ref RZ_VoiceLine>();
		RZ_VoiceDefaults.Fill(s_aLines);
		RZ_VoiceSettings vs = RZ_VoiceSettings.Get();
		if (vs.m_aLines)
		{
			foreach (RZ_VoiceLine extra : vs.m_aLines)
			{
				if (!extra || extra.m_sText.IsEmpty())
					continue;

				int at = FindId(extra.m_sId);
				if (at >= 0)
					s_aLines.Remove(at);

				s_aLines.Insert(extra);
			}
		}

		if (vs.m_aDisabled)
		{
			foreach (string id : vs.m_aDisabled)
			{
				int gone = FindId(id);
				if (gone >= 0)
					s_aLines.Remove(gone);
			}
		}

		foreach (RZ_VoiceLine line : s_aLines)
		{
			line.m_When = RZ_DialogueDB.Compile(line.m_sWhen);
		}

		RZ_Log.Info(string.Format("Voice: %1 lines in the bank", s_aLines.Count()));
		return s_aLines;
	}

	//------------------------------------------------------------------------------------------------
	protected static int FindId(string id)
	{
		if (id.IsEmpty())
			return -1;

		foreach (int i, RZ_VoiceLine line : s_aLines)
		{
			if (line.m_sId == id)
				return i;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsJoke(RZ_ERegister r)
	{
		return r == RZ_ERegister.WRY || r == RZ_ERegister.JOKE || r == RZ_ERegister.TAUNT;
	}

	//------------------------------------------------------------------------------------------------
	//! A line of `kind` for `faction` (and `manner`, "" any) said to `playerId` (0: nobody in
	//! particular, no joke memory) by someone at `pos` (`leader` for trait lines). "" if no line fits:
	//! the caller keeps its built line.
	static string PickFor(RZ_GameModeComponent gm, string kind, int faction, string manner, RZ_Leader leader, int playerId, vector pos, string place = "")
	{
		RZ_WorldFacts facts = gm.GetFacts();
		if (!facts)
			return "";

		RZ_EMood mood = facts.MoodFor(faction);
		int moodBit = RZ_SeasonMood.Bit(mood);
		RZ_DCtx ctx = new RZ_DCtx();
		ctx.m_GM = gm;
		ctx.m_iPlayer = playerId;
		ctx.m_iFaction = faction;
		ctx.m_Leader = leader;
		RZ_PlayerProfile profile;
		if (playerId > 0)
			profile = gm.GetProfile(playerId);

		array<RZ_VoiceLine> plain = {};
		array<RZ_VoiceLine> jokes = {};
		foreach (RZ_VoiceLine line : Lines())
		{
			if (line.m_sKind != kind || (line.m_iFaction >= 0 && line.m_iFaction != faction))
				continue;

			if (!line.m_sManner.IsEmpty() && line.m_sManner != manner)
				continue;

			if (line.m_iMoods != 0 && (line.m_iMoods & moodBit) == 0)
				continue;

			if (line.m_iTraits != 0 && (!leader || !HasAnyTrait(leader, line.m_iTraits)))
				continue;

			if (line.m_When && !line.m_When.IsEmpty() && !RZ_DialogueCond.Holds(ctx, line.m_When))
				continue;

			if (IsJoke(line.m_eRegister))
			{
				if (!HeardLately(profile, line.m_sId))
					jokes.Insert(line);
			}
			else
			{
				plain.Insert(line);
			}
		}

		// Nobody at a seat whose leader refuses a player jokes with them
		bool refused = leader && playerId > 0 && gm.GetLeaders() && gm.GetLeaders().Attitude(leader, playerId) <= -2;
		bool joke = !refused && !jokes.IsEmpty() && RZ_ToneGuard.AllowsJoke(pos) && Math.RandomFloat01() < 1.0 / RZ_SeasonMood.JokeOneIn(mood);
		RZ_VoiceLine picked;
		if (joke || plain.IsEmpty())
		{
			if (!joke)
				return "";

			picked = Weighted(jokes);
		}
		else
		{
			picked = Weighted(plain);
		}

		if (!picked)
			return "";

		if (IsJoke(picked.m_eRegister))
			NoteJoke(profile, picked.m_sId);

		if (RZ_VoiceSettings.Get().m_bLog)
		{
			string listener = "all";
			if (profile)
				listener = profile.m_sIdentity;

			RZ_Log.Info(string.Format("voice: face %1 %2 %3 mood=%4 listener=%5", RZ_DialogueDB.FactionWord(faction),
				typename.EnumToString(RZ_ERegister, picked.m_eRegister), picked.m_sId, RZ_SeasonMood.Name(mood), listener));
		}

		string text = picked.m_sText;
		text.Replace("{place}", place);
		return facts.Fill(text);
	}

	//------------------------------------------------------------------------------------------------
	protected static bool HasAnyTrait(RZ_Leader l, int mask)
	{
		for (int t = 0; t < 8; t++)
		{
			if ((mask & (1 << t)) != 0 && l.HasTrait(t))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_VoiceLine Weighted(array<RZ_VoiceLine> lines)
	{
		float total = 0;
		foreach (RZ_VoiceLine l : lines)
		{
			total += Math.Max(0.01, l.m_fWeight);
		}

		float roll = Math.RandomFloat(0, total);
		foreach (RZ_VoiceLine line : lines)
		{
			roll -= Math.Max(0.01, line.m_fWeight);
			if (roll <= 0)
				return line;
		}

		if (lines.IsEmpty())
			return null;

		return lines[lines.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! Joke memory: the last 24 jokes this player heard, with the real time (saved on the profile,
	//! across lives).
	protected static bool HeardLately(RZ_PlayerProfile p, string id)
	{
		if (!p || id.IsEmpty())
			return false;

		p.EnsureArrays();
		int i = p.m_aJokeIds.Find(id);
		if (i < 0 || i >= p.m_aJokeUnix.Count())
			return false;

		return RZ_SeasonClock.Now() - p.m_aJokeUnix[i] < JOKE_SECONDS;
	}

	//------------------------------------------------------------------------------------------------
	protected static void NoteJoke(RZ_PlayerProfile p, string id)
	{
		if (!p || id.IsEmpty())
			return;

		p.EnsureArrays();
		int i = p.m_aJokeIds.Find(id);
		if (i >= 0 && i < p.m_aJokeUnix.Count())
		{
			p.m_aJokeIds.Remove(i);
			p.m_aJokeUnix.Remove(i);
		}

		while (p.m_aJokeIds.Count() >= JOKE_MEMORY)
		{
			p.m_aJokeIds.Remove(0);
			if (!p.m_aJokeUnix.IsEmpty())
				p.m_aJokeUnix.Remove(0);
		}

		p.m_aJokeIds.Insert(id);
		p.m_aJokeUnix.Insert(RZ_SeasonClock.Now());
	}
}
