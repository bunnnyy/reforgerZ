//! The death brief (docs/UI.md, "The death brief"): when a player dies, a short card of the life
//! that just ended, over the spawn screen: who they were, the day and place, how it ended, how
//! long they lived, and a handful of this life's numbers, then one closing line. A, Space, a click
//! or B gives way to the spawn screen underneath; it never holds up the respawn.
//!
//! Server: the life's numbers are on the profile (reset by StartNewLife). Two are counted here, on
//! hooks in the game mode: the infected put down near the player and the ground covered. At the
//! death (Factions/RZ_FactionPerks.c, OnPlayerDied) the brief is built once and sent to the dying
//! player's machine only, with **UI > Death brief** on.
//! Client: the brief waits until the respawn screen is up (ours or vanilla's), then opens on top of
//! it; a client that can't open the screen (no preset or layout) shows it as a notice card.

//------------------------------------------------------------------------------------------------
modded enum ChimeraMenuPreset // needs its chimeraMenus.conf entry (UI/LAYOUTS.md)
{
	RZ_DeathBriefMenu
}

//------------------------------------------------------------------------------------------------
//! What ended a life, as the brief tells it.
enum RZ_EDeathCause
{
	UNKNOWN,	// nothing the engine recorded: a fall, a crash with nobody driving, bleeding out alone
	TURNED,		// the bite ran out (RZ_GameModeComponent.TurnPlayer)
	INFECTED,	// an infected's blow, or nobody's with one standing over the body
	PLAYER,		// another player
	FACTION,	// Bastion's, the Red Hand's or Halcyon's people
	SURVIVORS,	// other survivors' AI: guards, wanderers, farm people
	OWN			// the player's own doing (their grenade, their car, the respawn button)
}

//------------------------------------------------------------------------------------------------
class RZ_DeathBrief
{
	//! Its payload kind (RZ_PlayerUIComponent.RpcDo_Payload). Kept far from RZ_UIFeed's list, so the
	//! kinds added there never meet it.
	static const int PAYLOAD = 30;

	protected static const float CREDIT_RANGE = 120;	// an infected's death counts for the nearest player this close
	protected static const float STEP_MAX = 300;		// a longer step in one tick is a teleport or a respawn, not travel
	protected static const float SHORT_LIFE = 600;		// under 10 minutes: the short-life closing line
	protected static const float WAIT_MAX = 20;			// client: seconds to wait for the respawn screen, then open anyway
	protected static const float ALIVE_GRACE = 2;		// client: seconds before a live character means the moment has passed

	// Server
	protected static ref map<int, vector> s_mLastPos;
	protected static ref set<int> s_aTurned;

	// Client
	protected static string s_sPending;
	protected static float s_fPendingAt;
	protected static bool s_bWaiting;
	protected static string s_sShown;
	protected static int s_iVersion;

	// ================================================================ Server: counting
	//------------------------------------------------------------------------------------------------
	//! Each server tick, per living player (RZ_GameModeComponent.ServerTick): the ground covered,
	//! on foot or in a car.
	static void Tick(int playerId, RZ_PlayerProfile profile, IEntity ent)
	{
		if (!profile || !ent)
			return;

		if (!s_mLastPos)
			s_mLastPos = new map<int, vector>();

		vector pos = ent.GetOrigin();
		vector last;
		if (s_mLastPos.Find(playerId, last))
		{
			float step = RZ_Util.DistXZ(last, pos);
			if (step <= STEP_MAX)
				profile.m_fLifeMetres += step;
		}

		s_mLastPos.Set(playerId, pos);
	}

	//------------------------------------------------------------------------------------------------
	//! Infected died at pos (RZ_GameModeComponent.OnZombiesKilled): they count for the nearest living
	//! player within CREDIT_RANGE, whoever fired.
	static void OnInfectedKilled(RZ_GameModeComponent gm, vector pos, int count)
	{
		if (!gm || count <= 0)
			return;

		array<int> ids = {};
		array<IEntity> players = {};
		RZ_Util.GetAlivePlayers(ids, players);
		int nearest = -1;
		float best = CREDIT_RANGE;
		foreach (int i, IEntity p : players)
		{
			float d = RZ_Util.DistXZ(p.GetOrigin(), pos);
			if (d < best)
			{
				best = d;
				nearest = i;
			}
		}

		if (nearest < 0)
			return;

		RZ_PlayerProfile profile = gm.GetProfile(ids[nearest]);
		if (profile)
			profile.m_iLifeInfected += count;
	}

	//------------------------------------------------------------------------------------------------
	//! The bite ran out and the game mode is about to kill the character (TurnPlayer).
	static void MarkTurned(int playerId)
	{
		if (!s_aTurned)
			s_aTurned = new set<int>();

		s_aTurned.Insert(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The player left: nothing of theirs is kept here.
	static void Forget(int playerId)
	{
		if (s_mLastPos)
			s_mLastPos.Remove(playerId);

		if (s_aTurned)
			s_aTurned.RemoveItem(playerId);
	}

	// ================================================================ Server: the brief
	//------------------------------------------------------------------------------------------------
	//! A player died at `at` (RZ_FactionPerks.OnPlayerDied, before anything else hears of it).
	//! killer: the player whose damage killed them, -1 if none. Sends the brief to them only.
	static void OnDied(RZ_GameModeComponent gm, int victim, int killer, vector at, IEntity body)
	{
		bool turned = s_aTurned && s_aTurned.Contains(victim);
		Forget(victim);
		if (!gm || victim <= 0 || !RZ_UISettings.Get().m_bDeathBrief)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(victim);
		RZ_PlayerProfile profile = gm.GetProfile(victim);
		if (!ui || !profile)
			return;

		string killerName = "";
		int faction = -1;
		int cause = RZ_EDeathCause.TURNED;
		if (!turned)
			cause = CauseOf(gm, victim, killer, at, body, faction);

		if (cause == RZ_EDeathCause.PLAYER)
			killerName = RZ_Names.Public(killer);

		ui.SendPayload(PAYLOAD, Build(gm, victim, profile, at, cause, faction, killerName));
	}

	//------------------------------------------------------------------------------------------------
	//! What killed them. faction: set for FACTION.
	protected static int CauseOf(RZ_GameModeComponent gm, int victim, int killer, vector at, IEntity body, out int faction)
	{
		faction = -1;
		if (killer > 0)
			return RZ_EDeathCause.PLAYER;

		Instigator inst = InstigatorOf(body);
		if (inst && inst.GetInstigatorPlayerID() == victim)
			return RZ_EDeathCause.OWN;

		IEntity by;
		if (inst)
			by = inst.GetInstigatorEntity(); // VERIFY: a tick after the death the instigator still gives the AI or infected that dealt the last blow (a fall gives none)

		if (by)
		{
			string key = RZ_Util.GetCharacterFactionKey(by);
			for (int f = RZ_EFaction.MILITARY; f < RZ_Factions.COUNT; f++)
			{
				if (!key.IsEmpty() && key == RZ_Factions.GetAIKey(f))
				{
					faction = f;
					return RZ_EDeathCause.FACTION;
				}
			}

			if (RZ_Factions.IsPlayerSide(key))
				return RZ_EDeathCause.SURVIVORS;

			if (!key.IsEmpty())
				return RZ_EDeathCause.INFECTED;
		}

		// Nobody recorded: an infected standing over the body is answer enough
		if (gm.GetSpawner() && gm.GetSpawner().IsZombieNear(at, 6))
			return RZ_EDeathCause.INFECTED;

		return RZ_EDeathCause.UNKNOWN;
	}

	//------------------------------------------------------------------------------------------------
	//! Who dealt the damage that killed this character, null if nothing was recorded. Read after death.
	protected static Instigator InstigatorOf(IEntity victim)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(victim);
		if (!c)
			return null;

		SCR_CharacterDamageManagerComponent dmg = SCR_CharacterDamageManagerComponent.Cast(c.GetDamageManager());
		if (!dmg)
			return null;

		return dmg.GetInstigator();
	}

	//------------------------------------------------------------------------------------------------
	//! H|name|who|day and place, then L|label|value per line, then C|closing line.
	protected static string Build(RZ_GameModeComponent gm, int victim, RZ_PlayerProfile p, vector at, int cause, int faction, string killerName)
	{
		p.EnsureArrays();
		RZ_Background bg = gm.GetPlayerBackground(victim);
		string name = RZ_Util.PlayerName(victim);
		if (name.IsEmpty())
			name = "You";

		string who = string.Format("Life %1", p.m_iLives);
		if (bg)
			who = string.Format("%1, life %2", bg.m_sName, p.m_iLives);

		string place = RZ_SettlementScanner.GridReference(at);
		RZ_Settlement s = gm.GetSettlement(gm.FindNearestSettlement(at));
		if (s)
			place = s.m_sName;

		string payload = string.Format("H|%1|%2|%3\n", RZ_UIText.Clean(name, 40), RZ_UIText.Clean(who, 60),
			RZ_UIText.Clean(string.Format("Day %1, near %2", RZ_SeasonClock.Day(), place), 60));

		payload += Line("How it ended", CauseText(cause, faction, killerName));
		payload += Line("Lived", LivedText(p.m_fLifeSeconds));
		payload += Line("Infected put down", CountText(p.m_iLifeInfected));
		int kills = 0;
		for (int k = 0; k < RZ_Factions.COUNT; k++)
		{
			kills += p.m_aKills[k];
		}

		if (kills > 0)
			payload += Line("People killed", kills.ToString());

		payload += Line("Jobs done", CountText(p.m_iLifeMissions));
		payload += Line("Travelled", RZ_SayLine.Distance(p.m_fLifeMetres));
		int bestFaction = -1;
		int bestFame = 0;
		for (int f = 0; f < RZ_Factions.COUNT; f++)
		{
			if (p.m_aFameGained[f] > bestFame)
			{
				bestFame = p.m_aFameGained[f];
				bestFaction = f;
			}
		}

		if (bestFaction >= 0)
			payload += Line("Standing", string.Format("+%1 Fame with %2", bestFame, RZ_Factions.GetNameInText(bestFaction)));

		payload += "C" + RZ_UIText.SEP + RZ_UIText.Clean(Closing(cause, p.m_fLifeSeconds), 200);
		return payload;
	}

	//------------------------------------------------------------------------------------------------
	protected static string Line(string label, string value)
	{
		return "L" + RZ_UIText.SEP + RZ_UIText.Clean(label, 30) + RZ_UIText.SEP + RZ_UIText.Clean(value, 60) + "\n";
	}

	//------------------------------------------------------------------------------------------------
	protected static string CountText(int n)
	{
		if (n <= 0)
			return "None";

		return n.ToString();
	}

	//------------------------------------------------------------------------------------------------
	protected static string CauseText(int cause, int faction, string killerName)
	{
		if (cause == RZ_EDeathCause.PLAYER && killerName.IsEmpty())
			return "Killed by another survivor";

		if (cause == RZ_EDeathCause.PLAYER)
			return "Killed by " + killerName;

		switch (cause)
		{
			case RZ_EDeathCause.TURNED: return "Turned";
			case RZ_EDeathCause.INFECTED: return "The infected";
			case RZ_EDeathCause.SURVIVORS: return "Killed by other survivors";
			case RZ_EDeathCause.OWN: return "Your own doing";
			case RZ_EDeathCause.FACTION: return "Killed by " + RZ_Factions.GetNameInText(faction);
		}

		return "Nobody saw how";
	}

	//------------------------------------------------------------------------------------------------
	//! "41 minutes, Day 3" or "2 hours 5 min, Days 3 to 4": played time, and the season days it spanned.
	protected static string LivedText(float seconds)
	{
		int mins = RZ_Util.RoundToInt(seconds / 60);
		string span = "Under a minute";
		if (mins == 1)
			span = "A minute";
		else if (mins > 1 && mins < 60)
			span = string.Format("%1 minutes", mins);
		else if (mins >= 60)
		{
			int hours = mins / 60;
			int rest = mins % 60;
			span = string.Format("%1 hours", hours);
			if (hours == 1)
				span = "1 hour";

			if (rest > 0)
				span = string.Format("%1 %2 min", span, rest);
		}

		int lastDay = RZ_SeasonClock.Day();
		float startMinutes = Math.Max(0, RZ_SeasonClock.PlayedMinutes() - seconds / 60);
		int firstDay = Math.Floor(startMinutes / Math.Max(1, RZ_SeasonClock.DayMinutes()));
		firstDay = Math.Min(firstDay + 1, lastDay);
		if (firstDay == lastDay)
			return string.Format("%1, Day %2", span, lastDay);

		return string.Format("%1, Days %2 to %3", span, firstDay, lastDay);
	}

	//------------------------------------------------------------------------------------------------
	//! One line in the narrator's plain voice (docs/WRITING.md): no joke, nothing about the body.
	protected static string Closing(int cause, float seconds)
	{
		if (seconds < SHORT_LIFE)
			return "Not long. It rarely is, out here.";

		switch (cause)
		{
			case RZ_EDeathCause.TURNED: return "It took its time, and you used what it gave you.";
			case RZ_EDeathCause.INFECTED: return "The street is quiet again. It won't stay that way.";
			case RZ_EDeathCause.PLAYER: return "Someone made a choice out there. They'll carry it, or they won't.";
			case RZ_EDeathCause.FACTION: return "Somebody will write it down as something else.";
			case RZ_EDeathCause.SURVIVORS: return "Nobody fires first out here. Everybody says so.";
			case RZ_EDeathCause.OWN: return "It happens. It happens more in the Zone.";
		}

		return "Nobody saw it. Out here, that's most of them.";
	}

	// ================================================================ Client
	//------------------------------------------------------------------------------------------------
	//! The brief arrived (RZ_PlayerUIComponent.RpcDo_Payload): hold it until the respawn screen is up.
	static void OnPayload(string payload)
	{
		if (payload.IsEmpty() || System.IsConsoleApp())
			return;

		s_sPending = payload;
		s_fPendingAt = RZ_UIW.Now();
		if (s_bWaiting)
			return;

		s_bWaiting = true;
		GetGame().GetCallqueue().CallLater(Wait, 250, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Client, four times a second while a brief waits.
	protected static void Wait()
	{
		float age = RZ_UIW.Now() - s_fPendingAt;
		IEntity me = RZ_UIW.LocalEntity();
		bool alive = me && RZ_Util.IsCharacterAlive(me);
		if (alive && age > ALIVE_GRACE)
		{
			// Already in the next life: the moment has passed
			StopWaiting();
			return;
		}

		if (!RespawnScreenUp() && age < WAIT_MAX)
			return;

		StopWaiting();
		Show();
	}

	//------------------------------------------------------------------------------------------------
	protected static void StopWaiting()
	{
		s_bWaiting = false;
		GetGame().GetCallqueue().Remove(Wait);
	}

	//------------------------------------------------------------------------------------------------
	//! Our spawn screen, or vanilla's deploy menu or role selection, is open.
	protected static bool RespawnScreenUp()
	{
		MenuManager mm = GetGame().GetMenuManager();
		if (mm && mm.FindMenuByPreset(ChimeraMenuPreset.RZ_SpawnMenu))
			return true;

		return SCR_DeployMenuMain.GetDeployMenu() != null || SCR_RoleSelectionMenu.GetRoleSelectionMenu() != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the screen on top of whatever is up; without one, a notice card.
	protected static void Show()
	{
		s_sShown = s_sPending;
		s_sPending = "";
		s_iVersion++;
		if (RZ_DeathBriefMenu.IsShowing())
			return;

		// VERIFY: a menu opened while the spawn screen (or vanilla's deploy menu) is open goes on top and takes the input; the screen under it ignores A and B until it closes
		if (RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_DeathBriefMenu))
			return;

		ShowAsCard();
	}

	//------------------------------------------------------------------------------------------------
	//! The fallback: the same lines as one notice card.
	protected static void ShowAsCard()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm)
			return;

		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sShown, recs);
		string title = "";
		string text = "";
		string closing = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
				title = string.Format("%1, %2", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2));
			else if (tag == "L")
				text += string.Format("%1: %2\n", RZ_UIText.Str(f, 1), RZ_UIText.Str(f, 2));
			else if (tag == "C")
				closing = RZ_UIText.Str(f, 1);
		}

		gm.ShowNotice(title, text + closing, RZ_ENoticeTag.HIGH);
	}

	//------------------------------------------------------------------------------------------------
	static string Shown()
	{
		return s_sShown;
	}

	//------------------------------------------------------------------------------------------------
	static int Version()
	{
		return s_iVersion;
	}
}

//------------------------------------------------------------------------------------------------
//! The death brief's screen (client). A, Enter, Space, a click or B gives way to the spawn screen
//! under it. The close always waits a moment, so the press that closes it never reaches the screen
//! under it (A's MenuSelect fires on release, after Space's and the click's).
class RZ_DeathBriefMenu : RZ_MenuBase
{
	protected static RZ_DeathBriefMenu s_Open;
	protected static const float GRACE = 0.6;		// presses this soon after it opens don't count (still firing, still running)
	protected static const float LEAVE_DELAY = 0.4;	// a press that isn't MenuSelect closes it this much later

	protected float m_fOpenedAt;
	protected float m_fLeaveAt = -1;		// close at this time, -1 = not yet
	protected bool m_bLeaveNow;			// close on the next update

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		m_fOpenedAt = RZ_UIW.Now();
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.AddActionListener("RZ_TalkSkip", EActionTrigger.DOWN, OnSkip);
		RZ_UIW.OnClick(m_wRoot, "DismissArea", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Continue");
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		RZ_UIW.Focus(m_wRoot, "CloseButton");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnSelect);
		input.RemoveActionListener("RZ_TalkSkip", EActionTrigger.DOWN, OnSkip);
		if (s_Open == this)
			s_Open = null;

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		float now = RZ_UIW.Now();
		// Spawned some other way (a server-side respawn): the screen has nothing left to sit on
		IEntity me = RZ_UIW.LocalEntity();
		if (me && RZ_Util.IsCharacterAlive(me) && now - m_fOpenedAt > 1)
			m_bLeaveNow = true;

		if (m_bLeaveNow || (m_fLeaveAt >= 0 && now >= m_fLeaveAt))
		{
			m_bLeaveNow = false;
			m_fLeaveAt = -1;
			Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A or Enter: MenuSelect fires on the release, so nothing of this press is left to fall through.
	protected void OnSelect(float value = 0, EActionTrigger reason = 0)
	{
		if (RZ_UIW.Now() - m_fOpenedAt >= GRACE)
			m_bLeaveNow = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Space, a click, or A's press (RZ_TalkSkip): close a moment later, after A's release.
	protected void OnSkip(float value = 0, EActionTrigger reason = 0)
	{
		if (RZ_UIW.Now() - m_fOpenedAt >= GRACE)
			LeaveSoon();
	}

	//------------------------------------------------------------------------------------------------
	protected void LeaveSoon()
	{
		if (m_fLeaveAt < 0)
			m_fLeaveAt = RZ_UIW.Now() + LEAVE_DELAY;
	}

	//------------------------------------------------------------------------------------------------
	//! B or Esc: on the release too, so on the next update.
	override protected void Back()
	{
		m_bLeaveNow = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The close button and the dimmed screen around the card: a pad's A on the focused button may
	//! come as a click on the press, so these wait too.
	override void OnWidgetClick(Widget w)
	{
		if (RZ_UIW.Now() - m_fOpenedAt >= GRACE)
			LeaveSoon();
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		return RZ_DeathBrief.Version();
	}

	//------------------------------------------------------------------------------------------------
	//! H|name|who|day and place, L|label|value per line, C|closing line.
	override protected void Redraw()
	{
		array<ref array<string>> recs = {};
		Records(RZ_DeathBrief.Shown(), recs);
		string labels = "";
		string values = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
			{
				string name = RZ_UIText.Str(f, 1);
				name.ToUpper();
				string kicker = RZ_UIText.Str(f, 3);
				kicker.ToUpper();
				RZ_UIW.SetText(m_wRoot, "BriefName", name);
				RZ_UIW.SetText(m_wRoot, "BriefWho", RZ_UIText.Str(f, 2));
				RZ_UIW.SetText(m_wRoot, "BriefKicker", kicker);
			}
			else if (tag == "L")
			{
				labels += RZ_UIText.Str(f, 1) + "\n";
				values += RZ_UIText.Str(f, 2) + "\n";
			}
			else if (tag == "C")
			{
				RZ_UIW.SetText(m_wRoot, "BriefClosing", RZ_UIText.Str(f, 1));
			}
		}

		RZ_UIW.SetText(m_wRoot, "BriefLabels", labels);
		RZ_UIW.SetText(m_wRoot, "BriefValues", values);
	}
}
