//! Questlines (docs/DESIGN.md, "Questlines"): long faction storylines that span many sessions.
//! A questline is a graph of stages written as one class (Missions/Questlines/). Each stage
//! either offers one mission (a normal RZ_Mission started through the mission manager) or
//! waits for time to pass and the world to be right. How a stage was done (the approach and
//! its outcome) decides the next stage, and the ending changes the world through the systems
//! that already exist, a little and capped.
//!
//! This file is the engine: settings, saved form, one running questline (RZ_QuestRun) and the
//! manager (RZ_Questlines) that starts, ticks, saves and ends them. The author's base class,
//! the stage table and the consequence helpers are in Missions/Questlines/RZ_Questline.c; the
//! approach helpers in Missions/Questlines/RZ_QuestApproach.c; registration in
//! Missions/Questlines/RZ_QuestlineRegistry.c.
//!
//! Unlike story arcs (Director/RZ_StoryArcs.c, short, one at a time, gone at a restart), a
//! questline survives restarts: its stage, flags, notes, timers and who did what are saved, and a
//! stage mission that was running is offered again. A season wipe ends them all.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Director > Questlines.
[BaseContainerProps()]
class RZ_QuestlineSettings
{
	[Attribute("1", desc: "Questlines on: long faction storylines in stages, each a contract or a wait, where how you do a stage changes the ending (docs/DESIGN.md, Questlines)")]
	bool m_bEnabled;

	[Attribute("2", desc: "At most this many questlines run at once, server-wide (never two for one faction)")]
	int m_iMaxActive;

	[Attribute("30", desc: "Average real minutes (with players online) between chances for a new questline")]
	float m_fMinutesBetween;

	[Attribute("0.5", desc: "Chance that one starts when one is considered")]
	float m_fStartChance;

	[Attribute("6", desc: "Hours of play before a questline that fizzled out can start again")]
	float m_fRetryHours;

	[Attribute("1", desc: "A questline that reached an ending doesn't come back this season")]
	bool m_bOncePerSeason;

	[Attribute("1", desc: "Every wait between stages, and how long a stage waits before it fizzles, is multiplied by this (0.05 to 10; 0.1 for a quick test)")]
	float m_fWaitScale;
}

//------------------------------------------------------------------------------------------------
//! What approach a player took at a stage. Saved as an int.
enum RZ_EQuestApproach
{
	NONE,		// was there (a participant of the stage mission who did nothing in particular)
	TALK,		// asked, listened, told the truth
	PERSUADE,	// a check on background and standing
	SNEAK,		// done unseen, with no shots near
	FORCE,		// killed or drove someone off
	BRIBE,		// paid Favor
	BETRAY,		// turned on the people who asked
	SIDE,		// took another faction's side
	// Appended once (docs/DESIGN.md, "Checks and ways"): saved runs keep their numbers
	PRESS,		// threatened (the Fear check)
	TRADE,		// gave items or a service (the price table)
	TRICK,		// a claim: passed as one of theirs, or carried their paper
	LEAVE		// walked away: no pay, no blame
}

//------------------------------------------------------------------------------------------------
class RZ_QuestPartSave
{
	string ident;
	string name;
	int stage;
	int approach;
	string did;
}

//------------------------------------------------------------------------------------------------
class RZ_QuestRunSave
{
	string key;
	int stage = -1;
	float startedAt;
	int startUnix;
	float stageAt;
	int stageUnix;
	float readyAt = -1;
	int branch = -1;
	bool hasPlace;
	float x;
	float z;
	string placeName;
	ref array<string> flagKeys = {};
	ref array<int> flagValues = {};
	ref array<string> textKeys = {};
	ref array<string> textValues = {};
	ref array<int> visitStages = {};
	ref array<int> visitCounts = {};
	ref array<string> noteTitles = {};
	ref array<string> notes = {};
	ref array<ref RZ_QuestPartSave> parts = {};
}

//------------------------------------------------------------------------------------------------
class RZ_QuestlinesSave
{
	float clock;
	float nextChance;
	ref array<ref RZ_QuestRunSave> runs = {};
	ref array<string> doneKeys = {};
	ref array<string> lastKeys = {};
	ref array<float> lastAt = {};
	// Older saves only: world flags and rumours live in the world facts now (RZ_WorldFactsSave)
	ref array<string> worldKeys = {};
	ref array<int> worldValues = {};
	ref array<string> rumours = {};
	ref array<float> rumourUntil = {};
	// The majors' readings, pay caps and season story (Director/RZ_Majors.c)
	ref RZ_MajorsSave majors;
}

//------------------------------------------------------------------------------------------------
//! One player's part in one stage, kept by identity (players come and go).
class RZ_QuestPart
{
	string m_sIdent;
	string m_sName;
	int m_iStage;
	int m_iApproach;
	string m_sDid;
}

//------------------------------------------------------------------------------------------------
//! One running questline: everything about it that changes, all of it saved. The questline
//! class (RZ_Questline) holds no state; it reads and writes this.
class RZ_QuestRun
{
	int m_iId;						// runtime id (missions point at it with m_iQuestRun); not saved
	string m_sKey;
	RZ_Questline m_Line;			// the questline's definition (owned by the manager)
	RZ_Questlines m_Owner;

	int m_iStage = -1;
	float m_fStartedAt;				// play clock (seconds, runs only while someone plays)
	int m_iStartUnix;				// real time
	float m_fStageAt;
	int m_iStageUnix;
	float m_fReadyAt = -1;			// when the stage's wait was over (its give-up timer starts there)
	int m_iBranch = -1;				// the author's branch number, -1 until one is taken

	int m_iMissionId = -1;			// the stage's running mission, -1 = none (never saved)
	float m_fNextTry;
	int m_iOutcome = -1;			// set by a stage mission's approach; -1 = take success or failure
	int m_iEnding = -1;
	string m_sEndText;
	bool m_bDone;

	bool m_bHasPlace;
	vector m_vPlace;
	string m_sPlaceName;

	ref map<string, int> m_mFlags = new map<string, int>();
	ref map<string, string> m_mTexts = new map<string, string>();
	ref map<int, int> m_mVisits = new map<int, int>();
	ref array<string> m_aNoteTitles = {};
	ref array<string> m_aNotes = {};
	ref array<ref RZ_QuestPart> m_aParts = {};

	static const int MAX_NOTES = 12;
	static const int MAX_PARTS = 120;
	static const int MAX_FAME = 15;		// one personal reward
	static const int MAX_FAVOR = 40;

	// ================================================================ Flags and texts
	//------------------------------------------------------------------------------------------------
	//! An int flag set earlier ("house", "confessed", "sided"), `def` if never set.
	int Flag(string key, int def = 0)
	{
		int v;
		if (m_mFlags.Find(key, v))
			return v;

		return def;
	}

	//------------------------------------------------------------------------------------------------
	void SetFlag(string key, int value)
	{
		m_mFlags.Set(key, value);
	}

	//------------------------------------------------------------------------------------------------
	bool HasFlag(string key)
	{
		return m_mFlags.Contains(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A text set earlier ("town", "keeper"), "" if never set.
	string Text(string key)
	{
		string v;
		m_mTexts.Find(key, v);
		return v;
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string key, string value)
	{
		m_mTexts.Set(key, value);
	}

	// ================================================================ Claims
	//------------------------------------------------------------------------------------------------
	//! Who this questline is to the world facts' claims: "quest:<key>".
	string Owner()
	{
		return RZ_WorldFacts.QuestOwner(m_sKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Hold a subject (RZ_WorldFacts.Safehouse(idx), Leader(id), Wanderer(id), "lab"...) until the
	//! questline ends, so no other story uses it meanwhile. Kept with the run and held again after
	//! a restart. Its own stage missions may use it. False if someone else holds it.
	bool Claim(string subject)
	{
		if (!RZ_WorldFacts.Get().Claim(subject, Owner()))
			return false;

		m_mTexts.Set(CLAIM_PREFIX + subject, "1");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Let go of one subject before the end.
	void Unclaim(string subject)
	{
		RZ_WorldFacts.Get().Release(subject, Owner());
		m_mTexts.Remove(CLAIM_PREFIX + subject);
	}

	//------------------------------------------------------------------------------------------------
	//! After a restart: hold again what the run held (claims aren't saved, the run's list is).
	void Reclaim()
	{
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		foreach (string key, string value : m_mTexts)
		{
			if (key.StartsWith(CLAIM_PREFIX))
				facts.Claim(key.Substring(CLAIM_PREFIX.Length(), key.Length() - CLAIM_PREFIX.Length()), Owner());
		}
	}

	protected static const string CLAIM_PREFIX = "claim:";

	//------------------------------------------------------------------------------------------------
	//! `text` with every {key} replaced by that text: "Talk to {keeper} in {town}".
	string Fill(string text)
	{
		string s = text;
		foreach (string key, string value : m_mTexts)
		{
			s.Replace("{" + key + "}", value);
		}

		// Then the world's own tokens ({day}, {cure_state}...; RZ_WorldFacts.Fill)
		return RZ_WorldFacts.Get().Fill(s);
	}

	//------------------------------------------------------------------------------------------------
	//! Where the story is now (the journal's place and direction). Saved.
	void SetPlace(vector pos, string name)
	{
		m_bHasPlace = true;
		m_vPlace = pos;
		m_sPlaceName = name;
	}

	// ================================================================ The story so far
	//------------------------------------------------------------------------------------------------
	//! A line for the journal's "what you know so far", under the current stage's title. {keys}
	//! are filled in. The same line twice is kept once.
	void Note(string text)
	{
		string filled = Fill(text);
		if (filled.IsEmpty() || m_aNotes.Contains(filled))
			return;

		string title = "";
		if (m_Line)
		{
			RZ_QuestStage st = m_Line.GetStage(m_iStage);
			if (st)
				title = Fill(st.m_sTitle);
		}

		m_aNoteTitles.Insert(title);
		m_aNotes.Insert(filled);
		while (m_aNotes.Count() > MAX_NOTES)
		{
			m_aNotes.RemoveOrdered(0);
			m_aNoteTitles.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The branch the story took (the author's numbers). Saved.
	void SetBranch(int branch)
	{
		m_iBranch = branch;
	}

	//------------------------------------------------------------------------------------------------
	//! A stage mission reports how its stage came out (the author's outcome numbers, >= 10 by
	//! convention; RZ_Questline.OUTCOME_FAILED and OUTCOME_DONE are the engine's).
	void SetOutcome(int outcome)
	{
		m_iOutcome = outcome;
	}

	// ================================================================ Who took part
	//------------------------------------------------------------------------------------------------
	//! This player did `what` at the current stage, by `approach` (RZ_EQuestApproach). Kept by
	//! identity. A second call for the same player and stage replaces the first, unless the new
	//! one is NONE ("was there") and they already did something.
	void Did(RZ_GameModeComponent gm, int playerId, int approach, string what)
	{
		if (!gm || playerId <= 0)
			return;

		string ident = gm.GetProfile(playerId).m_sIdentity;
		if (ident.IsEmpty())
			return;

		// A touch, for a major's clock (RZ_Majors.IsEngaged)
		if (m_Owner)
			m_mFlags.Set("mc.touch", RZ_Util.RoundToInt(m_Owner.GetClock()));

		foreach (RZ_QuestPart p : m_aParts)
		{
			if (p.m_sIdent != ident || p.m_iStage != m_iStage)
				continue;

			if (approach == RZ_EQuestApproach.NONE && p.m_iApproach != RZ_EQuestApproach.NONE)
				return;

			p.m_iApproach = approach;
			p.m_sDid = what;
			return;
		}

		if (m_aParts.Count() >= MAX_PARTS)
			m_aParts.RemoveOrdered(0);

		RZ_QuestPart part = new RZ_QuestPart();
		part.m_sIdent = ident;
		part.m_sName = RZ_Names.Public(playerId);
		part.m_iStage = m_iStage;
		part.m_iApproach = approach;
		part.m_sDid = what;
		m_aParts.Insert(part);
	}

	//------------------------------------------------------------------------------------------------
	bool TookPart(string ident)
	{
		if (ident.IsEmpty())
			return false;

		foreach (RZ_QuestPart p : m_aParts)
		{
			if (p.m_sIdent == ident)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	bool PlayerTookPart(RZ_GameModeComponent gm, int playerId)
	{
		if (!gm || playerId <= 0)
			return false;

		return TookPart(gm.GetProfile(playerId).m_sIdentity);
	}

	//------------------------------------------------------------------------------------------------
	//! True if this identity ever took `approach` in this questline (any stage).
	bool IdentDid(string ident, int approach)
	{
		foreach (RZ_QuestPart p : m_aParts)
		{
			if (p.m_sIdent == ident && p.m_iApproach == approach)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! True if anybody took `approach` at `stage` (-1 = any stage).
	bool AnyDid(int approach, int stage = -1)
	{
		foreach (RZ_QuestPart p : m_aParts)
		{
			if (p.m_iApproach == approach && (stage < 0 || p.m_iStage == stage))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Everyone who took part, once each.
	void GetIdents(notnull array<string> result)
	{
		foreach (RZ_QuestPart p : m_aParts)
		{
			if (!result.Contains(p.m_sIdent))
				result.Insert(p.m_sIdent);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Mara (talked him round), Jonas (was there)" for one stage, "" if nobody.
	string WhoDid(int stage)
	{
		string text = "";
		foreach (RZ_QuestPart p : m_aParts)
		{
			if (p.m_iStage != stage)
				continue;

			if (!text.IsEmpty())
				text += ", ";

			text += string.Format("%1 (%2)", p.m_sName, p.m_sDid);
		}

		return text;
	}

	// ================================================================ Personal rewards
	//------------------------------------------------------------------------------------------------
	//! A personal reward for the player who did a step, on top of the stage mission's own pay:
	//! Fame (negative lowers it; at most 15 either way) and Favor (0 to 40) with `faction`.
	void Reward(RZ_GameModeComponent gm, int playerId, int faction, int fame, int favor, string why)
	{
		if (!gm || playerId <= 0 || !RZ_Factions.IsValid(faction))
			return;

		fame = Math.ClampInt(fame, -MAX_FAME, MAX_FAME);
		favor = Math.ClampInt(favor, 0, MAX_FAVOR);

		// A major's pay stays inside its caps (RZ_Majors.ClipPay)
		bool major = m_Line && m_Line.IsMajor() && RZ_Majors.Acting().IsEmpty();
		if (major)
			RZ_Majors.Enter(m_sKey);

		string ident = gm.GetProfile(playerId).m_sIdentity;
		fame = RZ_Majors.ClipPay(ident, faction, "fame", fame);
		favor = RZ_Majors.ClipPay(ident, faction, "favor", favor);
		if (major)
			RZ_Majors.Leave();

		string text = why;
		if (fame != 0)
		{
			gm.AddFame(playerId, faction, fame, false);
			text += string.Format("\n%1 Fame with %2.", Signed(fame), RZ_Factions.GetName(faction));
		}

		if (favor > 0)
		{
			gm.AddFavor(playerId, faction, favor);
			text += string.Format("\n+%1 Favor.", favor);
		}

		string title = "";
		if (m_Line)
			title = m_Line.m_sTitle;

		gm.NotifyPlayer(playerId, title, text.Trim());
	}

	//------------------------------------------------------------------------------------------------
	static string Signed(int v)
	{
		if (v > 0)
			return "+" + v.ToString();

		return v.ToString();
	}

	// ================================================================ Time
	//------------------------------------------------------------------------------------------------
	//! Minutes of play (someone online) since this stage began.
	float StageMinutes()
	{
		if (!m_Owner)
			return 0;

		return (m_Owner.GetClock() - m_fStageAt) / 60;
	}

	//------------------------------------------------------------------------------------------------
	//! Real hours since this stage began (0 if the clock isn't known).
	float StageRealHours()
	{
		int now = RZ_Guidance.UnixNow();
		if (now <= 0 || m_iStageUnix <= 0)
			return 0;

		float secs = now - m_iStageUnix;
		return secs / 3600;
	}

	//------------------------------------------------------------------------------------------------
	//! Hours of play since the questline began.
	float PlayHours()
	{
		if (!m_Owner)
			return 0;

		return (m_Owner.GetClock() - m_fStartedAt) / 3600;
	}

	//------------------------------------------------------------------------------------------------
	//! Real days since the questline began.
	float RealDays()
	{
		int now = RZ_Guidance.UnixNow();
		if (now <= 0 || m_iStartUnix <= 0)
			return 0;

		float secs = now - m_iStartUnix;
		return secs / 86400;
	}

	//------------------------------------------------------------------------------------------------
	int Visits(int stage)
	{
		int v;
		m_mVisits.Find(stage, v);
		return v;
	}

	// ================================================================ Save
	//------------------------------------------------------------------------------------------------
	RZ_QuestRunSave ToSave()
	{
		RZ_QuestRunSave s = new RZ_QuestRunSave();
		s.key = m_sKey;
		s.stage = m_iStage;
		s.startedAt = m_fStartedAt;
		s.startUnix = m_iStartUnix;
		s.stageAt = m_fStageAt;
		s.stageUnix = m_iStageUnix;
		s.readyAt = m_fReadyAt;
		s.branch = m_iBranch;
		s.hasPlace = m_bHasPlace;
		s.x = m_vPlace[0];
		s.z = m_vPlace[2];
		s.placeName = m_sPlaceName;
		foreach (string fk, int fv : m_mFlags)
		{
			s.flagKeys.Insert(fk);
			s.flagValues.Insert(fv);
		}

		foreach (string tk, string tv : m_mTexts)
		{
			s.textKeys.Insert(tk);
			s.textValues.Insert(tv);
		}

		foreach (int vs, int vc : m_mVisits)
		{
			s.visitStages.Insert(vs);
			s.visitCounts.Insert(vc);
		}

		foreach (int i, string n : m_aNotes)
		{
			s.notes.Insert(n);
			s.noteTitles.Insert(m_aNoteTitles[i]);
		}

		foreach (RZ_QuestPart p : m_aParts)
		{
			RZ_QuestPartSave ps = new RZ_QuestPartSave();
			ps.ident = p.m_sIdent;
			ps.name = p.m_sName;
			ps.stage = p.m_iStage;
			ps.approach = p.m_iApproach;
			ps.did = p.m_sDid;
			s.parts.Insert(ps);
		}

		return s;
	}

	//------------------------------------------------------------------------------------------------
	void FromSave(RZ_QuestRunSave s)
	{
		m_sKey = s.key;
		m_iStage = s.stage;
		m_fStartedAt = s.startedAt;
		m_iStartUnix = s.startUnix;
		m_fStageAt = s.stageAt;
		m_iStageUnix = s.stageUnix;
		m_fReadyAt = s.readyAt;
		m_iBranch = s.branch;
		m_bHasPlace = s.hasPlace;
		m_vPlace = Vector(s.x, 0, s.z);
		m_sPlaceName = s.placeName;
		if (s.flagKeys && s.flagValues)
		{
			for (int i = 0; i < s.flagKeys.Count() && i < s.flagValues.Count(); i++)
			{
				m_mFlags.Set(s.flagKeys[i], s.flagValues[i]);
			}
		}

		if (s.textKeys && s.textValues)
		{
			for (int j = 0; j < s.textKeys.Count() && j < s.textValues.Count(); j++)
			{
				m_mTexts.Set(s.textKeys[j], s.textValues[j]);
			}
		}

		if (s.visitStages && s.visitCounts)
		{
			for (int k = 0; k < s.visitStages.Count() && k < s.visitCounts.Count(); k++)
			{
				m_mVisits.Set(s.visitStages[k], s.visitCounts[k]);
			}
		}

		if (s.notes && s.noteTitles)
		{
			for (int n = 0; n < s.notes.Count() && n < s.noteTitles.Count(); n++)
			{
				m_aNotes.Insert(s.notes[n]);
				m_aNoteTitles.Insert(s.noteTitles[n]);
			}
		}

		if (s.parts)
		{
			foreach (RZ_QuestPartSave ps : s.parts)
			{
				if (!ps || ps.ident.IsEmpty())
					continue;

				RZ_QuestPart p = new RZ_QuestPart();
				p.m_sIdent = ps.ident;
				p.m_sName = ps.name;
				p.m_iStage = ps.stage;
				p.m_iApproach = ps.approach;
				p.m_sDid = ps.did;
				m_aParts.Insert(p);
			}
		}
	}
}

//------------------------------------------------------------------------------------------------
//! The questline manager, on RZ_GameModeComponent's tick.
class RZ_Questlines
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_QuestlineSettings m_Settings;
	protected ref array<ref RZ_Questline> m_aLines = {};
	protected ref array<ref RZ_QuestRun> m_aRuns = {};
	protected ref RZ_QuestlinesSave m_Restored;
	protected ref array<string> m_aDone = {};
	protected ref array<string> m_aLastKeys = {};
	protected ref array<float> m_aLastAt = {};
	protected float m_fClock;
	protected float m_fNextChance;
	protected float m_fCheck;
	protected int m_iNextRunId = 1;
	protected bool m_bHasCureFinale;	// a registered questline is a cure finale route (RZ_Questline.IsCureFinale)
	protected ref RZ_Majors m_Majors;	// the majors' clocks and season story (Director/RZ_Majors.c)
	protected float m_fMajorCheck;

	protected static const float CHECK_SECONDS = 10;
	protected static const float RETRY_SECONDS = 180;	// a stage mission that couldn't start tries again after this

	//------------------------------------------------------------------------------------------------
	void RZ_Questlines(RZ_GameModeComponent gm, RZ_QuestlineSettings settings)
	{
		m_GM = gm;
		m_Settings = settings;
		m_Majors = new RZ_Majors(gm, this, gm.GetMajorSettings());
		RZ_QuestlineRegistry.RegisterAll(this);
	}

	//------------------------------------------------------------------------------------------------
	RZ_Majors GetMajors() { return m_Majors; }

	//------------------------------------------------------------------------------------------------
	bool IsEnabled() { return m_Settings.m_bEnabled; }
	float GetClock() { return m_fClock; }
	array<ref RZ_QuestRun> GetRuns() { return m_aRuns; }

	//------------------------------------------------------------------------------------------------
	//! Flags shared by every questline for this season (the crossings between questlines, see
	//! docs/QUESTLINES.md): the season flags of the world facts (RZ_WorldFacts.Flag), so missions,
	//! arcs and voice read and write the same ones. Saved, and wiped with the season. Name them
	//! "<questline key>.<flag>". Every ending sets "<key>.ending" to its ending number by itself.
	int WorldFlag(string key, int def = 0)
	{
		return RZ_WorldFacts.Get().Flag(key, def);
	}

	//------------------------------------------------------------------------------------------------
	bool HasWorldFlag(string key)
	{
		return RZ_WorldFacts.Get().HasFlag(key);
	}

	//------------------------------------------------------------------------------------------------
	void SetWorldFlag(string key, int value)
	{
		RZ_WorldFacts.Get().SetFlag(key, value);
	}

	//------------------------------------------------------------------------------------------------
	//! A questline that hasn't ended is running now (by key).
	bool IsRunning(string key)
	{
		return FindRunByKey(key) != null;
	}

	//------------------------------------------------------------------------------------------------
	//! A questline reached one of its endings this season (fizzling doesn't count).
	bool HasEnded(string key)
	{
		return HasWorldFlag(key + ".ending");
	}

	//------------------------------------------------------------------------------------------------
	//! A registered questline's key, or one designed and not built yet (RZ_QuestlineRegistry).
	bool IsKnownKey(string key)
	{
		return FindLine(key) != null || RZ_QuestlineRegistry.IsDesigned(key);
	}

	//------------------------------------------------------------------------------------------------
	//! The wait scale, 0.05 to 10.
	float WaitScale()
	{
		return Math.Clamp(m_Settings.m_fWaitScale, 0.05, 10);
	}

	//------------------------------------------------------------------------------------------------
	//! Add a questline (a subclass of RZ_Questline). Other mods can call this too.
	void Register(typename type)
	{
		RZ_Questline line = RZ_Questline.Cast(type.Spawn());
		if (!line)
		{
			RZ_Log.Warn("Questlines: " + type.ToString() + " isn't an RZ_Questline.");
			return;
		}

		line.Define();
		if (line.m_sKey.IsEmpty() || line.StageCount() == 0)
		{
			RZ_Log.Warn("Questlines: " + type.ToString() + " has no key or no stages; skipped.");
			return;
		}

		if (FindLine(line.m_sKey))
		{
			RZ_Log.Warn("Questlines: two questlines use the key '" + line.m_sKey + "'; the second is skipped.");
			return;
		}

		m_aLines.Insert(line);
		if (line.IsCureFinale())
			m_bHasCureFinale = true;
	}

	//------------------------------------------------------------------------------------------------
	//! A cure finale questline is registered and questlines are on: research then stops at 90
	//! percent until a finale takes it further (RZ_CureResearch.IsAtFinaleGate).
	bool HasCureFinale()
	{
		return m_bHasCureFinale && m_Settings.m_bEnabled && RZ_CureFinale.IsOn();
	}

	//------------------------------------------------------------------------------------------------
	RZ_Questline FindLine(string key)
	{
		foreach (RZ_Questline line : m_aLines)
		{
			if (line.m_sKey == key)
				return line;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	RZ_QuestRun FindRun(int id)
	{
		if (id < 0)
			return null;

		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_iId == id && !run.m_bDone)
				return run;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! The run a stage mission belongs to, null if none.
	RZ_QuestRun RunOf(RZ_Mission m)
	{
		if (!m)
			return null;

		return FindRun(m.m_iQuestRun);
	}

	//------------------------------------------------------------------------------------------------
	RZ_QuestRun FindRunByKey(string key)
	{
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_sKey == key && !run.m_bDone)
				return run;
		}

		return null;
	}

	// ================================================================ Save / load
	//------------------------------------------------------------------------------------------------
	void Restore(RZ_QuestlinesSave saved)
	{
		m_Restored = saved;
	}

	//------------------------------------------------------------------------------------------------
	RZ_QuestlinesSave Save()
	{
		RZ_QuestlinesSave data = new RZ_QuestlinesSave();
		data.clock = m_fClock;
		data.nextChance = m_fNextChance;
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (!run.m_bDone)
				data.runs.Insert(run.ToSave());
		}

		foreach (string done : m_aDone)
		{
			data.doneKeys.Insert(done);
		}

		foreach (int i, string key : m_aLastKeys)
		{
			data.lastKeys.Insert(key);
			data.lastAt.Insert(m_aLastAt[i]);
		}

		data.majors = m_Majors.Save();
		return data;
	}

	//------------------------------------------------------------------------------------------------
	void Setup()
	{
		if (!m_Settings.m_bEnabled)
		{
			m_Restored = null;
			RZ_Log.Info("Questlines are off.");
			return;
		}

		m_fNextChance = Math.Max(10, m_Settings.m_fMinutesBetween * 0.5) * 60;
		if (m_Restored)
			LoadRestored();

		m_Restored = null;

		string keys = "";
		foreach (RZ_Questline line : m_aLines)
		{
			if (!keys.IsEmpty())
				keys += ", ";

			keys += line.m_sTitle;
		}

		if (keys.IsEmpty())
			keys = "none";

		string running = "";
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (!running.IsEmpty())
				running += ", ";

			running += run.m_Line.m_sTitle;
			RZ_QuestStage st = run.m_Line.GetStage(run.m_iStage);
			if (st)
				running += " (" + run.Fill(st.m_sTitle) + ")";
		}

		if (running.IsEmpty())
			running = "none";

		RZ_Log.Info(string.Format("Questlines: on, %1 registered (%2), at most %3 at once, running %4.", m_aLines.Count(), keys, Math.MaxInt(1, m_Settings.m_iMaxActive), running));
	}

	//------------------------------------------------------------------------------------------------
	protected void LoadRestored()
	{
		m_fClock = m_Restored.clock;
		m_fNextChance = m_Restored.nextChance;
		m_Majors.Restore(m_Restored.majors);
		if (m_Restored.doneKeys)
		{
			foreach (string done : m_Restored.doneKeys)
			{
				m_aDone.Insert(done);
			}
		}

		// Saves from before the world facts kept their world flags and rumours here: hand them over
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		if (m_Restored.worldKeys && m_Restored.worldValues)
		{
			for (int wi = 0; wi < m_Restored.worldKeys.Count() && wi < m_Restored.worldValues.Count(); wi++)
			{
				if (!facts.HasFlag(m_Restored.worldKeys[wi]))
					facts.SetFlag(m_Restored.worldKeys[wi], m_Restored.worldValues[wi]);
			}
		}

		if (m_Restored.lastKeys && m_Restored.lastAt)
		{
			for (int i = 0; i < m_Restored.lastKeys.Count() && i < m_Restored.lastAt.Count(); i++)
			{
				m_aLastKeys.Insert(m_Restored.lastKeys[i]);
				m_aLastAt.Insert(m_Restored.lastAt[i]);
			}
		}

		if (m_Restored.rumours && m_Restored.rumourUntil)
		{
			for (int r = 0; r < m_Restored.rumours.Count() && r < m_Restored.rumourUntil.Count(); r++)
			{
				facts.AdoptRumour(m_Restored.rumours[r], (m_Restored.rumourUntil[r] - m_fClock) / 3600);
			}
		}

		if (!m_Restored.runs)
			return;

		foreach (RZ_QuestRunSave s : m_Restored.runs)
		{
			if (!s)
				continue;

			RZ_Questline line = FindLine(s.key);
			if (!line || !line.GetStage(s.stage))
			{
				RZ_Log.Warn("Questlines: the saved questline '" + s.key + "' (or its stage) no longer exists; dropped.");
				continue;
			}

			RZ_QuestRun run = NewRun(line);
			run.FromSave(s);
			m_aRuns.Insert(run);
			run.Reclaim();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_QuestRun NewRun(RZ_Questline line)
	{
		RZ_QuestRun run = new RZ_QuestRun();
		run.m_iId = m_iNextRunId;
		m_iNextRunId++;
		run.m_sKey = line.m_sKey;
		run.m_Line = line;
		run.m_Owner = this;
		return run;
	}

	// ================================================================ Tick
	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		// Like the arcs, the questlines' clock only runs while someone plays
		if (!m_Settings.m_bEnabled || ids.IsEmpty())
			return;

		m_fClock += dt;
		m_fCheck += dt;
		if (m_fCheck < CHECK_SECONDS)
			return;

		m_fCheck = 0;
		for (int i = m_aRuns.Count() - 1; i >= 0; i--)
		{
			RZ_QuestRun run = m_aRuns[i];
			if (!run.m_bDone)
				TickRun(run, ids.Count());

			if (run.m_bDone)
				m_aRuns.Remove(i);
		}

		// The majors: their clocks, and a major whose start is due starts now (no roll)
		m_Majors.Tick(CHECK_SECONDS, ids);
		m_fMajorCheck += CHECK_SECONDS;
		if (m_fMajorCheck >= 60)
		{
			m_fMajorCheck = 0;
			TryStartMajors();
			TryStartRoutes();
		}

		if (m_fClock < m_fNextChance)
			return;

		float mean = Math.Max(5, m_Settings.m_fMinutesBetween) * 60;
		m_fNextChance = m_fClock + Math.RandomFloat(mean * 0.75, mean * 1.25);

		// Between beats a major's faction's smaller stories start twice as often
		float chance = m_Settings.m_fStartChance;
		if (m_Majors.AnyBetween())
			chance = Math.Min(1, chance * 2);

		if (Math.RandomFloat01() >= chance)
			return;

		TryStartAny();
	}

	//------------------------------------------------------------------------------------------------
	protected void TickRun(RZ_QuestRun run, int playerCount)
	{
		RZ_QuestStage st = run.m_Line.GetStage(run.m_iStage);
		if (!st)
		{
			EndRun(run);
			return;
		}

		// A stage mission is out there: its end comes through OnMissionFinished
		if (run.m_iMissionId >= 0)
		{
			if (!m_GM.GetMissions().FindById(run.m_iMissionId))
				StageResult(run, RZ_Questline.OUTCOME_FAILED);

			return;
		}

		float scale = WaitScale();
		if (run.m_fReadyAt < 0)
		{
			if (!IsWaitOver(run, st, scale))
				return;

			run.m_fReadyAt = m_fClock;
		}

		// The wait is over, but the stage can't happen: it waits, then fizzles (a major never
		// fizzles: its act's clock decides when the default moves it on; a finale route waits)
		if (!run.m_Line.IsMajor() && !run.m_Line.IsCureFinale() && st.m_fGiveUpHours > 0 && m_fClock - run.m_fReadyAt >= st.m_fGiveUpHours * 3600 * scale)
		{
			run.m_iEnding = -1;
			run.m_sEndText = run.Fill(st.m_sFizzle);
			EndRun(run);
			return;
		}

		if (!run.m_Line.IsReady(m_GM, run, st.m_iId))
			return;

		if (!st.m_bMission)
		{
			StageResult(run, RZ_Questline.OUTCOME_DONE);
			return;
		}

		if (m_fClock < run.m_fNextTry)
			return;

		run.m_fNextTry = m_fClock + RETRY_SECONDS;

		// At most two majors' stage missions are offered at once (RZ_Majors.MayOffer)
		if (run.m_Line.IsMajor() && !m_Majors.MayOffer(run))
			return;

		StartStageMission(run, st, playerCount);
	}

	//------------------------------------------------------------------------------------------------
	//! Play minutes, or real hours, or both (the stage says), times the wait scale.
	protected bool IsWaitOver(RZ_QuestRun run, RZ_QuestStage st, float scale)
	{
		bool hasPlay = st.m_fWaitMinutes > 0;
		bool hasReal = st.m_fWaitRealHours > 0;
		if (!hasPlay && !hasReal)
			return true;

		bool playOver = run.StageMinutes() >= st.m_fWaitMinutes * scale;
		bool realOver = hasReal && run.StageRealHours() >= st.m_fWaitRealHours * scale;
		if (!hasPlay)
			return realOver;

		if (!hasReal)
			return playOver;

		if (st.m_bWaitBoth)
			return playOver && realOver;

		return playOver || realOver;
	}

	//------------------------------------------------------------------------------------------------
	protected void StartStageMission(RZ_QuestRun run, RZ_QuestStage st, int playerCount)
	{
		RZ_Mission m = run.m_Line.MakeMission(m_GM, run, st.m_iId);
		if (!m)
			return;

		m.m_iQuestRun = run.m_iId;
		// What the questline holds (its house, its leader) is the stage mission's to use
		m.m_sClaimParent = run.Owner();
		m.Configure(m_GM);

		// The same check as any mission (RZ_MissionManager.CanRunNow): world gates and facts, No
		// Vehicles, one of each type, its claims free. Stage missions are never picked at random,
		// so their weight doesn't count
		if (!m_GM.GetMissions().StartIfCan(m, playerCount))
			return;

		run.m_iMissionId = m.m_iId;
		run.m_iOutcome = -1;
		run.m_fReadyAt = m_fClock;	// a restart while it runs gives the stage a fresh give-up window
		RZ_Log.Info(string.Format("Questline %1: stage '%2' is on (mission %3).", run.m_Line.m_sTitle, run.Fill(st.m_sTitle), m.m_iId));

		// Whoever was part of the story hears it goes on
		RZ_Questline.TellParticipants(m_GM, run, run.m_Line.m_sTitle, run.Fill(st.m_sTitle) + ": " + run.m_Line.JournalText(m_GM, run));
	}

	//------------------------------------------------------------------------------------------------
	//! The current stage came out as `outcome`: the questline decides what's next.
	protected void StageResult(RZ_QuestRun run, int outcome)
	{
		int from = run.m_iStage;
		run.m_iMissionId = -1;
		bool major = run.m_Line.IsMajor();
		if (major)
			RZ_Majors.Enter(run.m_sKey);

		int next = run.m_Line.Next(m_GM, run, from, outcome);
		if (major && (next == RZ_Questline.END || !run.m_Line.GetStage(next)))
			next = MajorGoesOn(run);

		if (major)
			RZ_Majors.Leave();

		if (next == RZ_Questline.END || !run.m_Line.GetStage(next))
		{
			EndRun(run);
			return;
		}

		EnterStage(run, next);
	}

	//------------------------------------------------------------------------------------------------
	//! A major's stage came to an end of the absorbed line inside it (R1's, B1's, H1's endings):
	//! that becomes an act's result and the major goes on. A fizzle inside a major is its act's
	//! default instead (a major never fizzles). Returns the next stage, or END at the major's end.
	protected int MajorGoesOn(RZ_QuestRun run)
	{
		if (run.m_iEnding >= 0)
		{
			int cont = run.m_Line.AfterInnerEnding(m_GM, run, run.m_iEnding);
			if (cont == RZ_Questline.END)
				return RZ_Questline.END;

			run.m_iEnding = -1;
			run.m_sEndText = "";
			return cont;
		}

		RZ_MajorAct act = run.m_Line.ActOf(run.m_iStage);
		if (!act)
			return RZ_Questline.END;

		RZ_Log.Info(string.Format("Majors: %1 stage %2 went quiet; its act takes its default.", run.m_sKey, run.m_iStage));
		run.m_sEndText = "";
		RZ_Majors.Enter(run.m_sKey, true);
		int next = run.m_Line.ActDefault(m_GM, run, act);
		RZ_Majors.Enter(run.m_sKey);
		RZ_Majors.MarkBeat(run);
		return next;
	}

	//------------------------------------------------------------------------------------------------
	//! Moves a major on to `stage` (its act's default, a hard stop): a stage mission still out goes
	//! with it. END: the run ends (on the ending its default set).
	void JumpTo(RZ_QuestRun run, int stage)
	{
		if (!run || run.m_bDone)
			return;

		Withdraw(run);
		if (stage == RZ_Questline.END || !run.m_Line.GetStage(stage))
		{
			EndRun(run);
			return;
		}

		EnterStage(run, stage);
	}

	//------------------------------------------------------------------------------------------------
	//! Takes a run's stage mission off the board without deciding the stage (it's offered again).
	void Withdraw(RZ_QuestRun run)
	{
		if (!run || run.m_iMissionId < 0)
			return;

		RZ_Mission m = m_GM.GetMissions().FindById(run.m_iMissionId);
		run.m_iMissionId = -1;
		run.m_fNextTry = m_fClock + RETRY_SECONDS;
		if (m && m.m_eState == RZ_EMissionState.ACTIVE)
			m.m_eState = RZ_EMissionState.FAILED;
	}

	//------------------------------------------------------------------------------------------------
	protected void EnterStage(RZ_QuestRun run, int stage)
	{
		RZ_QuestStage st = run.m_Line.GetStage(stage);
		int visits = run.Visits(stage) + 1;
		if (visits > Math.MaxInt(1, st.m_iMaxVisits) && !run.m_Line.IsMajor() && !run.m_Line.IsCureFinale())
		{
			// A loop that keeps failing ends here, cleanly
			run.m_iEnding = -1;
			run.m_sEndText = run.Fill(st.m_sFizzle);
			EndRun(run);
			return;
		}

		run.m_mVisits.Set(stage, visits);
		run.m_iStage = stage;
		run.m_fStageAt = m_fClock;
		run.m_iStageUnix = RZ_Guidance.UnixNow();
		run.m_fReadyAt = -1;
		run.m_fNextTry = 0;
		run.m_iOutcome = -1;
		RZ_Log.Info(string.Format("Questline %1: stage '%2'.", run.m_Line.m_sTitle, run.Fill(st.m_sTitle)));
		bool major = run.m_Line.IsMajor() && RZ_Majors.Acting().IsEmpty();
		if (major)
			RZ_Majors.Enter(run.m_sKey);

		run.m_Line.OnStageBegin(m_GM, run, stage);
		if (major)
			RZ_Majors.Leave();

		// A major's act opens: its reason, for contracts and the margin cards (Missions/Story/RZ_ReasonPosts.c)
		RZ_ReasonPosts.MajorAct(m_GM, run, stage);
	}

	//------------------------------------------------------------------------------------------------
	//! An ending (m_iEnding >= 0) applies its consequences; otherwise the questline fizzles with
	//! its line of text. Either way the participants online are told.
	protected void EndRun(RZ_QuestRun run)
	{
		if (run.m_bDone)
			return;

		run.m_bDone = true;

		// Whatever it held (RZ_WorldFacts claims) is free again
		m_GM.GetFacts().ReleaseAll(run.Owner());

		// A stage mission still out there goes with it
		if (run.m_iMissionId >= 0)
		{
			RZ_Mission m = m_GM.GetMissions().FindById(run.m_iMissionId);
			if (m && m.m_eState == RZ_EMissionState.ACTIVE)
				m.m_eState = RZ_EMissionState.FAILED;

			run.m_iMissionId = -1;
		}

		string title = run.m_Line.m_sTitle;
		if (run.m_iEnding >= 0)
		{
			// Other questlines can read how this one ended: WorldFlag("<key>.ending")
			SetWorldFlag(run.m_sKey + ".ending", run.m_iEnding);
			bool major = run.m_Line.IsMajor() && RZ_Majors.Acting().IsEmpty();
			if (major)
				RZ_Majors.Enter(run.m_sKey);

			run.m_Line.OnEnding(m_GM, run, run.m_iEnding);
			if (major)
				RZ_Majors.Leave();
			if (!m_aDone.Contains(run.m_sKey))
				m_aDone.Insert(run.m_sKey);

			RZ_Log.Info(string.Format("Questline ended: %1 (ending %2, branch %3).", title, run.m_iEnding, run.m_iBranch));
		}
		else
		{
			if (run.m_sEndText.IsEmpty())
				run.m_sEndText = string.Format("Nobody followed it up. %1 went quiet.", title);

			RZ_Questline.Headline(m_GM, run.m_sEndText, false);
			RZ_Log.Info(string.Format("Questline fizzled: %1.", title));
		}

		NoteRun(run.m_sKey);
		if (!run.m_sEndText.IsEmpty())
			RZ_Questline.TellParticipants(m_GM, run, title, run.m_sEndText);

		// Everyone who was part of it: a past job in their notebook's Journal
		RZ_JobHistory.QuestEnded(m_GM, run);
	}

	//------------------------------------------------------------------------------------------------
	protected void NoteRun(string key)
	{
		int idx = m_aLastKeys.Find(key);
		if (idx >= 0)
		{
			m_aLastAt[idx] = m_fClock;
			return;
		}

		m_aLastKeys.Insert(key);
		m_aLastAt.Insert(m_fClock);
	}

	// ================================================================ Starting
	//------------------------------------------------------------------------------------------------
	protected void TryStartAny()
	{
		if (CountActive() >= Math.MaxInt(1, m_Settings.m_iMaxActive))
			return;

		array<int> order = {};
		for (int i = 0; i < m_aLines.Count(); i++)
		{
			order.Insert(i);
		}

		for (int k = order.Count() - 1; k > 0; k--)
		{
			int j = Math.RandomInt(0, k + 1);
			int tmp = order[k];
			order[k] = order[j];
			order[j] = tmp;
		}

		// Between beats, the major's faction's smaller stories go first
		for (int pass = 0; pass < 2; pass++)
		{
			foreach (int idx : order)
			{
				bool between = m_Majors.IsBetween(m_aLines[idx].m_iFaction);
				if ((pass == 0) != between)
					continue;

				if (CanConsider(m_aLines[idx]) && Begin(m_aLines[idx].m_sKey))
					return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected int CountActive()
	{
		int n = 0;
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (!run.m_bDone && !InMajorLane(run.m_Line))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! A major, or an inner line a major starts (HC1, HC3 under Proof): outside Max active and the
	//! one-per-faction rule.
	protected bool InMajorLane(RZ_Questline line)
	{
		return line && (line.IsMajor() || !line.StartsOnItsOwn());
	}

	//------------------------------------------------------------------------------------------------
	//! Every major not running and not done this season starts as soon as its CanStart says so.
	protected void TryStartMajors()
	{
		if (!RZ_Majors.IsOn())
			return;

		foreach (RZ_Questline line : m_aLines)
		{
			if (!line.IsMajor() || FindRunByKey(line.m_sKey) || m_aDone.Contains(line.m_sKey))
				continue;

			Begin(line.m_sKey);
		}
	}

	// ================================================================ The finale's routes
	//------------------------------------------------------------------------------------------------
	//! The finale's lane (docs/FINALE.md): at the gate every route not running and not ended this
	//! season begins (outside Max active and the faction slots).
	protected void TryStartRoutes()
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (!finale || !finale.IsOpen() || finale.IsFrozen())
			return;

		foreach (RZ_Questline line : m_aLines)
		{
			if (!line.IsCureFinale() || FindRunByKey(line.m_sKey) || m_aDone.Contains(line.m_sKey))
				continue;

			Begin(line.m_sKey);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A route finished (RZ_CureFinale.Finish): every other route's stage work is called off, with
	//! no pay and no penalty, and its items are gone. They wait (frozen) until the cure.
	void FreezeRoutes(string finisherKey)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_bDone || !run.m_Line.IsCureFinale() || run.m_sKey == finisherKey)
				continue;

			Withdraw(run);
			if (finale)
				finale.GetItems().ClearRoute(RZ_CureFinale.RouteOfKey(run.m_sKey));

			RZ_FinaleRoute route = RZ_FinaleRoute.Cast(run.m_Line);
			if (route && run.m_iStage != RZ_FinaleRoute.S_AFTER)
				JumpTo(run, RZ_FinaleRoute.S_AFTER);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A stand was lost: the finisher's route makes the batch again (its remake step).
	void RouteRemake(string finisherKey)
	{
		RZ_QuestRun run = FindRunByKey(finisherKey);
		if (run && RZ_FinaleRoute.Cast(run.m_Line))
			JumpTo(run, RZ_FinaleRoute.S_REMAKE);
	}

	//------------------------------------------------------------------------------------------------
	//! The cure: every route ends; the controller's goes to its release (the terms). A controller
	//! whose run is gone gets its default terms at once.
	void EndRoutes(string controllerKey)
	{
		bool released = false;
		for (int i = m_aRuns.Count() - 1; i >= 0; i--)
		{
			RZ_QuestRun run = m_aRuns[i];
			RZ_FinaleRoute route = RZ_FinaleRoute.Cast(run.m_Line);
			if (run.m_bDone || !route)
				continue;

			bool mine = run.m_sKey == controllerKey;
			if (mine)
				released = true;

			Withdraw(run);
			JumpTo(run, route.AtCure(m_GM, run, mine));
		}

		if (!released)
			ReleaseDefault(controllerKey);
	}

	//------------------------------------------------------------------------------------------------
	//! The controller's default terms, now (no release scene, or the epilogue's wait ran out).
	//! RZ_CureFinale.OnTerms ignores it once terms are in.
	void ReleaseDefault(string controllerKey)
	{
		RZ_CureFinale finale = RZ_CureFinale.Get();
		int c = RZ_CureFinale.RouteOfKey(controllerKey);
		if (!finale || c < 0)
			return;

		RZ_FinaleRoute line = RZ_FinaleRoute.Cast(FindLine(controllerKey));
		int terms = 1;
		if (line)
			terms = line.DefaultTerms(m_GM);

		finale.OnTerms(c, terms, 0, null);
	}

	//------------------------------------------------------------------------------------------------
	//! Not running, its slot (faction) free, not done this season, not fizzled lately.
	protected bool CanConsider(RZ_Questline line)
	{
		if (FindRunByKey(line.m_sKey) || InMajorLane(line))
			return false;

		if (m_Settings.m_bOncePerSeason && m_aDone.Contains(line.m_sKey))
			return false;

		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (!run.m_bDone && !InMajorLane(run.m_Line) && run.m_Line.GetSlot() == line.GetSlot())
				return false;
		}

		int idx = m_aLastKeys.Find(line.m_sKey);
		if (idx >= 0 && m_fClock - m_aLastAt[idx] < m_Settings.m_fRetryHours * 3600)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Start a questline now (also for testing from other code). False if it can't run.
	bool Begin(string key)
	{
		RZ_Questline line = FindLine(key);
		if (!m_Settings.m_bEnabled || !line || FindRunByKey(key))
			return false;

		if (line.IsMajor() && !RZ_Majors.IsOn())
			return false;

		RZ_QuestRun run = NewRun(line);
		run.m_fStartedAt = m_fClock;
		run.m_iStartUnix = RZ_Guidance.UnixNow();
		if (!line.CanStart(m_GM, run))
		{
			// Anything it claimed while looking goes back
			m_GM.GetFacts().ReleaseAll(run.Owner());
			return false;
		}

		m_aRuns.Insert(run);
		RZ_Log.Info("Questline started: " + line.m_sTitle);
		line.OnBegin(m_GM, run);
		EnterStage(run, line.FirstStage());
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Start a questline now if the usual rules allow it (Max active, one per faction, not done
	//! this season, not fizzled lately): for content that leads into one (a contract's outcome).
	//! False if it can't; the usual schedule may still start it later.
	bool TryBegin(string key)
	{
		RZ_Questline line = FindLine(key);
		if (!m_Settings.m_bEnabled || !line || CountActive() >= Math.MaxInt(1, m_Settings.m_iMaxActive) || !CanConsider(line))
			return false;

		return Begin(key);
	}

	// ================================================================ Hooks from other systems
	//------------------------------------------------------------------------------------------------
	//! A mission ended (RZ_GameModeComponent.OnContractFinished, every mission).
	void OnMissionFinished(RZ_Mission m, bool success)
	{
		if (!m || m.m_iQuestRun < 0)
			return;

		RZ_QuestRun run = FindRun(m.m_iQuestRun);
		if (!run || run.m_iMissionId != m.m_iId)
			return;

		// Everyone who was there counts as part of the story
		foreach (int playerId : m.m_aParticipants)
		{
			run.Did(m_GM, playerId, RZ_EQuestApproach.NONE, "was there");
		}

		int outcome = run.m_iOutcome;
		if (outcome < 0)
		{
			outcome = RZ_Questline.OUTCOME_FAILED;
			if (success)
				outcome = RZ_Questline.OUTCOME_DONE;
		}

		StageResult(run, outcome);
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand (re)starts (RZ_CureResearch.StartFinalStand): every running line hears it.
	void OnFinalStandStart()
	{
		array<RZ_QuestRun> runs = {};
		foreach (RZ_QuestRun r : m_aRuns)
		{
			if (!r.m_bDone)
				runs.Insert(r);
		}

		foreach (RZ_QuestRun run : runs)
		{
			bool major = run.m_Line.IsMajor();
			if (major)
				RZ_Majors.Enter(run.m_sKey);

			run.m_Line.OnFinalStandStart(m_GM, run);
			if (major)
				RZ_Majors.Leave();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The clean room changed holder during the stand (RZ_CureFinale.TakeRoom).
	void OnStandHolder(int from, int to)
	{
		array<RZ_QuestRun> runs = {};
		foreach (RZ_QuestRun r : m_aRuns)
		{
			if (!r.m_bDone)
				runs.Insert(r);
		}

		foreach (RZ_QuestRun run : runs)
		{
			bool major = run.m_Line.IsMajor();
			if (major)
				RZ_Majors.Enter(run.m_sKey);

			run.m_Line.OnStandHolder(m_GM, run, from, to);
			if (major)
				RZ_Majors.Leave();
		}

		RZ_Log.Info(string.Format("Questlines: the clean room changed holder (%1 to %2).", from, to));
	}

	//------------------------------------------------------------------------------------------------
	//! The final stand ended: won (the cure) or lost (the batch).
	void OnFinalStandEnd(bool won)
	{
		array<RZ_QuestRun> runs = {};
		foreach (RZ_QuestRun r : m_aRuns)
		{
			if (!r.m_bDone)
				runs.Insert(r);
		}

		foreach (RZ_QuestRun run : runs)
		{
			bool major = run.m_Line.IsMajor();
			if (major)
				RZ_Majors.Enter(run.m_sKey);

			run.m_Line.OnFinalStandEnd(m_GM, run, won);
			if (major)
				RZ_Majors.Leave();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A wanderer is talked to: a questline may add a line. "" if none.
	string OnWandererTalk(int playerId, int wandererId)
	{
		string text = "";
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_bDone)
				continue;

			string line = run.m_Line.OnWandererTalk(m_GM, run, playerId, wandererId);
			if (line.IsEmpty())
				continue;

			if (!text.IsEmpty())
				text += "\n";

			text += run.Fill(line);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! A leader is talked to: a questline may add a line. "" if none.
	string OnLeaderTalk(int playerId, int leaderId)
	{
		string text = "";
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_bDone)
				continue;

			string line = run.m_Line.OnLeaderTalk(m_GM, run, playerId, leaderId);
			if (line.IsEmpty())
				continue;

			if (!text.IsEmpty())
				text += "\n";

			text += run.Fill(line);
		}

		return text;
	}

	//------------------------------------------------------------------------------------------------
	//! Gossip for wanderers and leaders: a running questline's rumour or an ending's, "" if none.
	string Rumour()
	{
		array<string> options = {};
		foreach (RZ_QuestRun run : m_aRuns)
		{
			if (run.m_bDone)
				continue;

			string line = run.m_Line.Rumour(m_GM, run);
			if (!line.IsEmpty())
				options.Insert(run.Fill(line));
		}

		// The world's gossip: endings, world changes (RZ_WorldFacts.Changed)
		RZ_WorldFacts.Get().GetRumours(options);
		if (options.IsEmpty())
			return "";

		return options.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! A line wanderers and leaders repeat for `hours` of play (an ending's aftermath). Kept with
	//! the world's gossip (RZ_WorldFacts.AddRumour), so it's saved even with questlines off.
	void AddRumour(string text, float hours)
	{
		RZ_WorldFacts.Get().AddRumour(text, hours);
	}

	// ================================================================ Journal
	//------------------------------------------------------------------------------------------------
	//! The journal's brief: the questline's own, and for a major the season view under it (its act,
	//! the rule or stance in force, the leans, any last call).
	string JournalFor(RZ_QuestRun run)
	{
		string text = run.m_Line.JournalText(m_GM, run);
		if (!run.m_Line.IsMajor())
			return text;

		string season = m_Majors.SeasonText(run);
		if (season.IsEmpty())
			return text;

		return text + "\n\n" + season;
	}

	//------------------------------------------------------------------------------------------------
	//! Does this player see the questline in their journal? Anyone who took part (by identity),
	//! and anyone the current stage's mission was offered to or who is taking part in it.
	bool IsVisibleTo(RZ_QuestRun run, int playerId)
	{
		if (!run || run.m_bDone || playerId <= 0)
			return false;

		if (run.PlayerTookPart(m_GM, playerId))
			return true;

		// The majors are the season's stories: everyone sees them (the journal's season view)
		if (run.m_Line && run.m_Line.IsMajor())
			return true;

		// A finale route: its own locked players always see it
		RZ_CureFinale finale = RZ_CureFinale.Get();
		if (finale && run.m_Line && run.m_Line.IsCureFinale() && finale.RouteOfPlayer(playerId) == RZ_CureFinale.RouteOfKey(run.m_sKey))
			return true;

		RZ_Mission m = m_GM.GetMissions().FindById(run.m_iMissionId);
		if (!m)
			return false;

		return m.m_aOfferedTo.Contains(playerId) || m.m_aParticipants.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A player id for an identity that's online, -1 if they aren't.
	int OnlineId(string ident)
	{
		if (ident.IsEmpty())
			return -1;

		array<int> all = {};
		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (m_GM.GetProfile(playerId).m_sIdentity == ident)
				return playerId;
		}

		return -1;
	}
}
