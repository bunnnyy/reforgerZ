//! Which prefab slot on RZ_GameModeComponent > Puzzles a mission spawns.
//! Specific slots (well, bell, safe...) are optional: when empty, the generic
//! puzzle object (or the note) is used, and the action labels say what it is.
enum RZ_EPuzzleProp
{
	CLUE,			// a note on the ground
	BODY,			// a body with something readable (falls back to CLUE)
	OBJECT,			// generic puzzle object: read + 4 use + 4 choice actions
	KEYPAD,			// keypad (falls back to nothing: keypad puzzles need it)
	TERMINAL,		// computer terminal (falls back to KEYPAD)
	WELL,
	BELL,
	FUSEBOX,
	RADIO_MAST,
	SAFE,
	SCALE,			// a crate with storage + puzzle object (no fallback: needs storage)
	CIVILIAN,		// a person to talk to (falls back to OBJECT)
	LANDMARK,		// a distinctive prop when the town has no landmark building
	LIGHT,			// Morse light (no fallback)
	SMUGGLER,		// The Blind Spot's smuggler (Perimeter slot, falls back to CIVILIAN)
	LINE_BODY		// Shot at the Line's body (Perimeter slot, falls back to BODY)
}

//! Base for puzzle missions (docs/MISSIONS.md, "Puzzle missions").
//!
//! - Clues: AddClue() stores text on the server; SpawnClue()/TagClue() put a readable
//!   entity in the world. Reading sends the text to that player only.
//! - Answers: keypad keys, choices and uses arrive in OnCodeEntered/OnChoice/OnUse.
//! - Wrong answers: SoundAlarm() (heat, optional horde), LockOut() (seconds).
//! - Everything spawned through these helpers is deleted in Cleanup().
//!
//! Subclasses implement SetupPuzzle() instead of Setup(), so a failed setup
//! cleans up whatever it had already spawned.
class RZ_PuzzleMission : RZ_Mission
{
	protected ref array<string> m_aClueTitles = {};
	protected ref array<string> m_aClueTexts = {};
	protected ref array<IEntity> m_aProps = {};
	protected ref map<int, string> m_mCodes = new map<int, string>();
	protected ref map<int, RZ_PuzzleObjectComponent> m_mObjects = new map<int, RZ_PuzzleObjectComponent>();
	protected ref RZ_Rng m_Rng;
	protected ref RZ_NoticeTracker m_Notice;
	protected float m_fLockout;
	protected float m_fClock;
	protected int m_iWrong;

	static const int KEY_CLEAR = 10;
	static const int KEY_ENTER = 11;
	protected static const int MAX_CODE = 12;

	//------------------------------------------------------------------------------------------------
	//! Puzzles give their own clues and grid references: never a rough area.
	override bool NeedsExactLocation()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Setup(RZ_GameModeComponent gm)
	{
		m_Rng = new RZ_Rng(Math.RandomInt(1, 2000000000) + m_iId * 7919);
		if (SetupPuzzle(gm))
			return true;

		Cleanup(gm);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Pick places, generate the puzzle, spawn clues and objects. False = can't run here.
	protected bool SetupPuzzle(RZ_GameModeComponent gm)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void Update(RZ_GameModeComponent gm, float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		if (m_fLockout > 0)
			m_fLockout -= dt;

		super.Update(gm, dt, ids, players);
	}

	//------------------------------------------------------------------------------------------------
	//! Puzzle missions need the note prefab; most need nothing else.
	static bool HasPrefab(RZ_GameModeComponent gm, RZ_EPuzzleProp kind)
	{
		return !gm.GetPuzzlePrefab(kind).IsEmpty();
	}

	// ================================================================ Clues
	//------------------------------------------------------------------------------------------------
	//! Store a clue's text. Returns its index.
	protected int AddClue(string title, string text)
	{
		m_aClueTitles.Insert(title);
		m_aClueTexts.Insert(text);
		return m_aClueTexts.Count() - 1;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetClueText(int idx, string text)
	{
		if (idx >= 0 && idx < m_aClueTexts.Count())
			m_aClueTexts[idx] = text;
	}

	//------------------------------------------------------------------------------------------------
	//! Override for text that depends on the reader (★ backgrounds) or on state (fuse box lamps).
	string GetClueText(RZ_GameModeComponent gm, int playerId, int idx)
	{
		if (idx < 0 || idx >= m_aClueTexts.Count())
			return "";

		return m_aClueTexts[idx];
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_ReadClueAction.
	void HandleRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
		if (idx < 0 || idx >= m_aClueTexts.Count())
			return;

		AddParticipant(playerId);
		string clueText = GetClueText(gm, playerId, idx);
		gm.NotifyPlayer(playerId, m_aClueTitles[idx], clueText);
		if (gm.GetUIFeed())
			gm.GetUIFeed().OnClueRead(playerId, m_iId, m_aClueTitles[idx], clueText);

		OnClueRead(gm, playerId, idx, source);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClueRead(RZ_GameModeComponent gm, int playerId, int idx, IEntity source)
	{
	}

	// ================================================================ Spawning
	//------------------------------------------------------------------------------------------------
	protected IEntity SpawnProp(RZ_GameModeComponent gm, RZ_EPuzzleProp kind, vector pos, float yaw = -1)
	{
		if (yaw < 0)
			yaw = m_Rng.Range(0, 360);

		IEntity e = RZ_Util.SpawnPrefab(gm.GetPuzzlePrefab(kind), RZ_Util.GroundPos(pos), yaw);
		if (e)
			m_aProps.Insert(e);

		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Track something spawned elsewhere (crate, AI group) so Cleanup() removes it.
	protected void TrackProp(IEntity e)
	{
		if (e && !m_aProps.Contains(e))
			m_aProps.Insert(e);
	}

	//------------------------------------------------------------------------------------------------
	//! A readable clue entity. Falls back to a plain note if the prefab lacks RZ_ClueComponent.
	protected IEntity SpawnClue(RZ_GameModeComponent gm, RZ_EPuzzleProp kind, vector pos, int clueIdx, string readLabel)
	{
		IEntity e = SpawnProp(gm, kind, pos);
		if (TagClue(e, clueIdx, readLabel))
			return e;

		if (kind != RZ_EPuzzleProp.CLUE)
		{
			IEntity note = SpawnProp(gm, RZ_EPuzzleProp.CLUE, RZ_Util.RandomPointInRing(pos, 0.8, 1.5));
			if (TagClue(note, clueIdx, readLabel))
				return note;
		}

		RZ_Log.Warn("Puzzle clue prefab has no RZ_ClueComponent; clue can't be read.");
		return e;
	}

	//------------------------------------------------------------------------------------------------
	//! Make an existing prop readable (wreck, body, plaque). False if it has no RZ_ClueComponent.
	protected bool TagClue(IEntity e, int clueIdx, string readLabel)
	{
		return RZ_ClueComponent.Tag(e, m_iId, clueIdx, readLabel);
	}

	//------------------------------------------------------------------------------------------------
	//! A puzzle object. Labels: "|" separated per action slot, e.g. "Flip switch 1|Flip switch 2".
	//! clueIdx >= 0 also makes it readable (status, inscription).
	protected IEntity SpawnObject(RZ_GameModeComponent gm, RZ_EPuzzleProp kind, vector pos, int objIdx, string useLabels, string choiceLabels, int clueIdx = -1, string readLabel = "")
	{
		IEntity e = SpawnProp(gm, kind, pos);
		if (!e)
			return null;

		RZ_PuzzleObjectComponent comp = RZ_PuzzleObjectComponent.Tag(e, m_iId, objIdx, useLabels, choiceLabels);
		if (!comp)
		{
			RZ_Log.Warn("Puzzle object prefab has no RZ_PuzzleObjectComponent; it can't be used.");
			return e;
		}

		m_mObjects.Set(objIdx, comp);
		if (clueIdx >= 0)
			TagClue(e, clueIdx, readLabel);

		return e;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetObjectLabels(int objIdx, string useLabels, string choiceLabels)
	{
		RZ_PuzzleObjectComponent comp;
		if (m_mObjects.Find(objIdx, comp) && comp)
			comp.SetLabels(useLabels, choiceLabels);
	}

	//------------------------------------------------------------------------------------------------
	//! A landmark for a puzzle: a real building of one of `kinds` near `center`, or a
	//! fallback prop (Puzzles > Landmark) placed at `fallbackPos`. Null if neither works.
	protected RZ_Landmark FindOrPlaceLandmark(RZ_GameModeComponent gm, vector center, float radius, array<string> kinds, vector fallbackPos, array<ref RZ_Landmark> avoid = null)
	{
		array<ref RZ_Landmark> found = {};
		RZ_LandmarkFinder.Find(center, radius, kinds, found);
		foreach (RZ_Landmark l : found)
		{
			if (!IsNearAny(l.m_vPos, avoid, 60))
				return l;
		}

		if (!HasPrefab(gm, RZ_EPuzzleProp.LANDMARK))
			return null;

		IEntity e = SpawnProp(gm, RZ_EPuzzleProp.LANDMARK, fallbackPos);
		if (!e)
			return null;

		RZ_Landmark spawned = new RZ_Landmark();
		spawned.m_vPos = e.GetOrigin();
		spawned.m_sKind = "landmark";
		spawned.m_sName = gm.GetLandmarkFallbackName();
		spawned.m_bSpawned = true;
		return spawned;
	}

	//------------------------------------------------------------------------------------------------
	protected static bool IsNearAny(vector pos, array<ref RZ_Landmark> others, float dist)
	{
		if (!others)
			return false;

		foreach (RZ_Landmark o : others)
		{
			if (o && RZ_Util.DistXZ(pos, o.m_vPos) < dist)
				return true;
		}

		return false;
	}

	// ================================================================ Answers
	//------------------------------------------------------------------------------------------------
	protected bool CheckLockout(RZ_GameModeComponent gm, int playerId)
	{
		if (m_fLockout <= 0)
			return true;

		gm.NotifyPlayer(playerId, m_sTitle, string.Format("Nothing responds. Try again in %1 seconds.", RZ_Util.RoundToInt(m_fLockout)));
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_KeypadAction.
	void HandleKey(RZ_GameModeComponent gm, int playerId, int objIdx, int key, IEntity source)
	{
		if (m_eState != RZ_EMissionState.ACTIVE || !CheckLockout(gm, playerId))
			return;

		AddParticipant(playerId);
		string code;
		m_mCodes.Find(objIdx, code);

		if (key == KEY_ENTER)
		{
			m_mCodes.Set(objIdx, "");
			UpdateDisplay(objIdx, "");
			OnCodeEntered(gm, playerId, objIdx, code, source);
			return;
		}

		if (key == KEY_CLEAR)
			code = "";
		else if (key >= 0 && key <= 9 && code.Length() < MAX_CODE)
			code += key.ToString();

		m_mCodes.Set(objIdx, code);
		UpdateDisplay(objIdx, code);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDisplay(int objIdx, string code)
	{
		RZ_PuzzleObjectComponent comp;
		if (m_mObjects.Find(objIdx, comp) && comp)
			comp.SetDisplay(code);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleChoiceAction.
	void HandleChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
		if (m_eState != RZ_EMissionState.ACTIVE || !CheckLockout(gm, playerId))
			return;

		AddParticipant(playerId);
		OnChoice(gm, playerId, objIdx, option, source);
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_PuzzleUseAction.
	void HandleUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
		if (m_eState != RZ_EMissionState.ACTIVE || !CheckLockout(gm, playerId))
			return;

		AddParticipant(playerId);
		OnUse(gm, playerId, objIdx, value, source, user);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCodeEntered(RZ_GameModeComponent gm, int playerId, int objIdx, string code, IEntity source)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnChoice(RZ_GameModeComponent gm, int playerId, int objIdx, int option, IEntity source)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void OnUse(RZ_GameModeComponent gm, int playerId, int objIdx, int value, IEntity source, IEntity user)
	{
	}

	//------------------------------------------------------------------------------------------------
	//! True if any player is within radius of pos; they all become participants.
	protected bool ReachedBy(array<int> ids, array<IEntity> players, vector pos, float radius)
	{
		bool reached = false;
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), pos) > radius)
				continue;

			AddParticipant(ids[i]);
			reached = true;
		}

		return reached;
	}

	//------------------------------------------------------------------------------------------------
	//! Players this mission talks to: offered it, or already taking part.
	protected bool IsInvolved(int playerId)
	{
		return m_aOfferedTo.Contains(playerId) || m_aParticipants.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Real seconds a night mission should last: until dawn if it's dark now, else until
	//! dawn after the coming night. Clamped to [minSeconds, maxSeconds]; `fallback` if
	//! the clock speed isn't known yet.
	protected static float NightDuration(float minSeconds, float maxSeconds, float fallback)
	{
		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_END);
		if (secs < 0)
			return fallback;

		return Math.Clamp(secs + 600, minSeconds, maxSeconds);
	}

	//------------------------------------------------------------------------------------------------
	//! For GetWeight of night missions: dark now, or dark within `withinSeconds`.
	protected static bool IsNightSoon(float withinSeconds)
	{
		if (RZ_GameTime.IsNight())
			return true;

		float secs = RZ_GameTime.RealSecondsUntil(RZ_GameTime.NIGHT_START);
		return secs >= 0 && secs <= withinSeconds;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddParticipant(int playerId)
	{
		if (playerId > 0 && !m_aParticipants.Contains(playerId))
			m_aParticipants.Insert(playerId);
	}

	// ================================================================ Consequences
	//------------------------------------------------------------------------------------------------
	//! A wrong answer is loud: heat here, and optionally a horde sent at it.
	protected void SoundAlarm(RZ_GameModeComponent gm, vector pos, float heat, bool horde)
	{
		gm.GetHeat().AddHeat(pos, heat);
		if (!horde)
			return;

		gm.GetSpawner().SpawnHorde(pos, 300, m_iId);
		gm.NotifyHorde(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected void LockOut(float seconds)
	{
		m_fLockout = Math.Max(m_fLockout, seconds);
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_NoticeTracker GetNotice()
	{
		if (!m_Notice)
			m_Notice = new RZ_NoticeTracker();

		return m_Notice;
	}

	//------------------------------------------------------------------------------------------------
	protected void SpawnRewardAt(RZ_GameModeComponent gm, vector pos)
	{
		gm.SpawnRewardCrate(RZ_Util.RandomPointInRing(pos, 1, 3));
	}

	//------------------------------------------------------------------------------------------------
	//! "Morton, grid 047-112"
	protected string PlaceText(RZ_GameModeComponent gm, vector pos)
	{
		int idx = gm.FindNearestSettlement(pos);
		RZ_Settlement s = gm.GetSettlement(idx);
		string grid = RZ_SettlementScanner.GridReference(pos);
		if (s && RZ_Util.DistXZ(s.m_vCenter, pos) <= s.m_fRadius * 1.5)
			return string.Format("%1, grid %2", s.m_sName, grid);

		return "grid " + grid;
	}

	//------------------------------------------------------------------------------------------------
	//! Settlement indices not in `used`, within [minDist, maxDist] of `from`, shuffled.
	protected void CollectSettlements(RZ_GameModeComponent gm, vector from, float minDist, float maxDist, array<int> used, notnull array<int> result)
	{
		foreach (int i, RZ_Settlement s : gm.GetSettlements())
		{
			if (used && used.Contains(i))
				continue;

			float d = RZ_Util.DistXZ(from, s.m_vCenter);
			if (d < minDist || d > maxDist)
				continue;

			if (gm.GetMissions().IsSettlementBusy(i))
				continue;

			result.Insert(i);
		}

		m_Rng.ShuffleInts(result);
	}

	//------------------------------------------------------------------------------------------------
	override void GetPropSpots(notnull array<vector> result)
	{
		super.GetPropSpots(result);
		foreach (IEntity e : m_aProps)
		{
			if (e)
				result.Insert(e.GetOrigin());
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Cleanup(RZ_GameModeComponent gm)
	{
		foreach (IEntity e : m_aProps)
		{
			RZ_Util.DeleteUnit(e);
		}

		m_aProps.Clear();
		m_mObjects.Clear();
	}
}
