//! The power station's panel screen, server side (docs/UI.md, "The power panel"; docs/DESIGN.md,
//! "The power grid"). One prompt on the control, **Open the panel**, opens it; everything happens in
//! it: what you see, the door, the breakers, the spin-up, the main switch and the upkeep.
//!
//! The breakers are a small puzzle: six on the board, each with a name, and a card inside the door
//! that says which three to close and in what order, and which one is burnt. Close them in the
//! card's order and the board reads READY; anything else trips the lot back open (the burnt one
//! with a bang that carries). One panel per station, shared by everyone at it.
//!
//! Payload (RZ_UIFeed.PAYLOAD_POWER): P|serial|title|state|text|status|step|left|total|gauge|ready,
//! B|index|name|closed per breaker (from the door's opening on), C|line per line of the card, and
//! A|index|label|enabled|need per button; X|reason closes it. A move comes back as
//! RZ_PlayerUIComponent.RpcAsk_PowerMove(serial, move): 0-5 a breaker, 10 + n button n, 99 the
//! client couldn't open the screen (the world prompts come back for everyone).

//------------------------------------------------------------------------------------------------
class RZ_PowerPanel
{
	static const int BREAKERS = 6;
	static const int ORDER = 3;				// breakers the card says to close
	static const int MOVE_BUTTON = 10;		// 10 + the button's index
	static const int MOVE_FAILED = 99;		// the client couldn't open the screen

	static const int BUTTON_FORCE = 0;
	static const int BUTTON_START = 1;
	static const int BUTTON_SWITCH = 2;
	static const int BUTTON_MAINTAIN = 3;

	protected static const float WRONG_HEAT = 2;
	protected static const float FAULT_HEAT = 8;

	protected RZ_GameModeComponent m_GM;
	protected RZ_PowerGrid m_Grid;
	protected ref array<string> m_aNames = {};		// by position on the board
	protected ref array<int> m_aOrder = {};			// positions to close, in order
	protected int m_iFault = -1;					// the burnt one's position
	protected ref array<bool> m_aClosed = {};
	protected int m_iDone;							// how many of the order are closed
	protected ref map<int, int> m_mSerial = new map<int, int>();	// player -> their screen's serial
	protected int m_iNextSerial = 1;
	protected bool m_bClientFailed;					// a client couldn't open it: world prompts for everyone
	protected bool m_bInMove;						// a move is running: one redraw for the others at its end

	//------------------------------------------------------------------------------------------------
	void RZ_PowerPanel(RZ_GameModeComponent gm, RZ_PowerGrid grid)
	{
		m_GM = gm;
		m_Grid = grid;
		Deal();
	}

	//------------------------------------------------------------------------------------------------
	//! The screen is used: UI > Power breaker layout set (the client checks the same) and no client
	//! failed to open it. Otherwise the control keeps its old world prompts.
	bool IsUsed()
	{
		return !m_bClientFailed && !RZ_UISettings.Get().m_sPowerBreakerLayout.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	bool IsOpenFor(int playerId)
	{
		return m_mSerial.Contains(playerId);
	}

	// ================================================================ The breakers
	//------------------------------------------------------------------------------------------------
	//! A new board and card: six names in random places, three to close in order, one burnt.
	protected void Deal()
	{
		array<string> pool = {"CONTROL", "FEED A", "FEED B", "PUMPS", "YARD", "LIGHTS", "TOWN 1", "TOWN 2", "HEATING", "WORKSHOP"};
		m_aNames.Clear();
		while (m_aNames.Count() < BREAKERS && !pool.IsEmpty())
		{
			int pick = Math.RandomInt(0, pool.Count());
			m_aNames.Insert(pool[pick]);
			pool.Remove(pick);
		}

		array<int> places = {};
		for (int i = 0; i < BREAKERS; i++)
		{
			places.Insert(i);
		}

		m_aOrder.Clear();
		while (m_aOrder.Count() < ORDER)
		{
			int at = Math.RandomInt(0, places.Count());
			m_aOrder.Insert(places[at]);
			places.Remove(at);
		}

		m_iFault = places[Math.RandomInt(0, places.Count())];
		Trip();
	}

	//------------------------------------------------------------------------------------------------
	//! Every breaker open again.
	protected void Trip()
	{
		m_aClosed.Clear();
		for (int i = 0; i < BREAKERS; i++)
		{
			m_aClosed.Insert(false);
		}

		m_iDone = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The board is set: every breaker on the card closed, in order.
	bool IsReady()
	{
		return m_iDone >= m_aOrder.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Back to a dark board (the generator died or failed): the same card, every breaker tripped.
	void Reset()
	{
		Trip();
	}

	//------------------------------------------------------------------------------------------------
	//! The world prompts' way (no screen): the breakers reset in one go.
	void SetAll()
	{
		Trip();
		foreach (int p : m_aOrder)
		{
			m_aClosed[p] = true;
		}

		m_iDone = m_aOrder.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! The card inside the door.
	protected void CardLines(notnull array<string> lines)
	{
		lines.Clear();
		lines.Insert("AFTER A BLACKOUT, CLOSE IN THIS ORDER:");
		foreach (int i, int p : m_aOrder)
		{
			int n = i + 1;
			lines.Insert(string.Format("%1.  %2", n, m_aNames[p]));
		}

		lines.Insert(string.Format("Leave %1 open. It's burnt out.", m_aNames[m_iFault]));
	}

	//------------------------------------------------------------------------------------------------
	//! One breaker flipped (the screen; the server checks the step).
	protected void Flip(int playerId, int index)
	{
		if (index < 0 || index >= BREAKERS)
			return;

		vector pos = m_Grid.GetPosition();
		if (m_aClosed[index])
		{
			Trip();
			m_GM.GetHeat().AddHeat(pos, WRONG_HEAT);
			m_GM.NotifyPlayer(playerId, "Power station", "You pull it open and the rest drop out with it. Start again from the card.");
			return;
		}

		if (index == m_iFault)
		{
			Trip();
			m_GM.GetHeat().AddHeat(pos, FAULT_HEAT);
			m_GM.NotifyPlayer(playerId, "Power station", string.Format("%1, the burnt one. It bangs and spits sparks, and every breaker drops. That carried.", m_aNames[index]));
			return;
		}

		if (m_iDone < m_aOrder.Count() && m_aOrder[m_iDone] == index)
		{
			m_aClosed[index] = true;
			m_iDone++;
			if (IsReady())
				m_GM.NotifyPlayer(playerId, "Power station", "The last one holds. The board reads READY. The generator can start.");
			else
				m_GM.NotifyPlayer(playerId, "Power station", string.Format("%1 holds.", m_aNames[index]));

			return;
		}

		Trip();
		m_GM.GetHeat().AddHeat(pos, WRONG_HEAT);
		m_GM.NotifyPlayer(playerId, "Power station", string.Format("%1 won't hold out of turn. It drops and takes the others with it. Start again from the card.", m_aNames[index]));
	}

	// ================================================================ The screen
	//------------------------------------------------------------------------------------------------
	//! Opens the panel for the player at the control `owner`. False: no screen here (the caller
	//! tells them what they see as a notice).
	bool Open(int playerId, IEntity owner)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (!IsUsed() || !ui || !guard || !owner)
			return false;

		if (!guard.Open(playerId, RZ_ScreenGuard.KIND_POWER, owner, RZ_Util.Capitalize(m_Grid.GetName()), -1))
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_POWER, "F|busy");
			return false;
		}

		m_mSerial.Set(playerId, m_iNextSerial);
		m_iNextSerial++;
		Send(playerId, "");
		RZ_Log.Info(string.Format("Power panel: opened for player %1 (%2).", playerId, m_Grid.StepName()));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The screen guard or the player closed it.
	void OnScreenGone(int playerId)
	{
		m_mSerial.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! A player left the server.
	void Forget(int playerId)
	{
		m_mSerial.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Closes every open panel (the control went away).
	void CloseAll(string reason)
	{
		array<int> players = {};
		foreach (int playerId, int serial : m_mSerial)
		{
			players.Insert(playerId);
		}

		foreach (int p : players)
		{
			Close(p, reason);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Close(int playerId, string reason)
	{
		m_mSerial.Remove(playerId);
		if (m_GM.GetScreenGuard())
			m_GM.GetScreenGuard().Close(playerId);

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_POWER, "X|" + reason);
	}

	//------------------------------------------------------------------------------------------------
	//! Something changed at the station: every open panel is drawn again (`except` keeps its own
	//! status line, sent separately).
	void Refresh(int except = -1)
	{
		if (m_bInMove)
			return;

		foreach (int playerId, int serial : m_mSerial)
		{
			if (playerId != except)
				Send(playerId, "");
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A line for a player: into their open panel's status line, else a notice.
	void Tell(int playerId, string title, string text)
	{
		if (IsOpenFor(playerId))
		{
			Send(playerId, text);
			return;
		}

		m_GM.NotifyPlayer(playerId, title, text);
	}

	//------------------------------------------------------------------------------------------------
	//! A move from the screen (RZ_PlayerUIComponent.RpcAsk_PowerMove). Checked here: the screen is
	//! open on this serial, the control is there and the player beside it; then the step's own code.
	void Move(int playerId, int serial, int move)
	{
		int current;
		if (!m_mSerial.Find(playerId, current) || serial != current)
			return;

		RZ_ScreenGuard guard = m_GM.GetScreenGuard();
		if (move == MOVE_FAILED)
		{
			ClientFailed(playerId);
			return;
		}

		if (!guard || !guard.IsOpen(playerId, RZ_ScreenGuard.KIND_POWER))
		{
			OnScreenGone(playerId);
			return;
		}

		guard.Touch(playerId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		IEntity body = m_Grid.GetBody();
		if (!user || !body || !RZ_Util.IsCharacterAlive(user))
		{
			Close(playerId, "gone");
			return;
		}

		if (RZ_Util.DistXZ(user.GetOrigin(), body.GetOrigin()) > RZ_DialogueSettings.Get().m_fRange)
			return;

		int step = m_Grid.GetStep();
		m_bInMove = true;
		RZ_ScreenCapture.Begin(playerId);
		if (move >= 0 && move < BREAKERS)
		{
			if (step == RZ_PowerGrid.STEP_OPEN)
				Flip(playerId, move);
		}
		else
		{
			array<int> kinds = {};
			array<string> labels = {};
			array<string> needs = {};
			array<bool> can = {};
			Buttons(playerId, user, kinds, labels, needs, can);
			int b = move - MOVE_BUTTON;
			if (b >= 0 && b < kinds.Count())
			{
				if (can[b])
					Press(playerId, kinds[b], user);
				else
					m_GM.NotifyPlayer(playerId, "Power station", needs[b]);
			}
		}

		string said = RZ_ScreenCapture.End();
		m_bInMove = false;
		if (IsOpenFor(playerId))
			Send(playerId, said);
		else if (!said.IsEmpty())
			m_GM.NotifyPlayer(playerId, RZ_ScreenCapture.LastTitle(), said);

		Refresh(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void Press(int playerId, int kind, IEntity user)
	{
		switch (kind)
		{
			case BUTTON_FORCE: m_Grid.ForceDoor(playerId); break;
			case BUTTON_START: m_Grid.StartGenerator(playerId); break;
			case BUTTON_SWITCH: m_Grid.ThrowSwitch(playerId); break;
			case BUTTON_MAINTAIN: m_Grid.Maintain(playerId, user); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The step's buttons: what each does, its label, why it can't be pressed yet ("" if it can).
	protected void Buttons(int playerId, IEntity user, notnull array<int> kinds, notnull array<string> labels, notnull array<string> needs, notnull array<bool> can)
	{
		int step = m_Grid.GetStep();
		if (step == RZ_PowerGrid.STEP_DARK)
		{
			kinds.Insert(BUTTON_FORCE);
			labels.Insert("Force the door");
			needs.Insert("");
			can.Insert(true);
		}
		else if (step == RZ_PowerGrid.STEP_OPEN)
		{
			kinds.Insert(BUTTON_START);
			labels.Insert("Start the generator");
			if (IsReady())
			{
				needs.Insert("");
				can.Insert(true);
			}
			else
			{
				needs.Insert("The breakers first");
				can.Insert(false);
			}
		}
		else if (step == RZ_PowerGrid.STEP_READY)
		{
			kinds.Insert(BUTTON_SWITCH);
			labels.Insert("Throw the main switch");
			needs.Insert("");
			can.Insert(true);
		}
		else if (step == RZ_PowerGrid.STEP_POWERED)
		{
			kinds.Insert(BUTTON_MAINTAIN);
			labels.Insert("Refuel and grease the generator");
			string why = m_Grid.MaintainBlocked(playerId);
			needs.Insert(why);
			can.Insert(why.IsEmpty());
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A client couldn't open the screen (no preset or layout there): the world prompts come back for
	//! everyone (every client has the same mod), and this player is told what they see.
	protected void ClientFailed(int playerId)
	{
		m_bClientFailed = true;
		OnScreenGone(playerId);
		if (m_GM.GetScreenGuard())
			m_GM.GetScreenGuard().Close(playerId);

		RZ_Log.Warn("Power panel: a client couldn't open it (no RZ_PowerMenu preset or layout?). The control's world prompts are back.");
		m_Grid.OnPanelFailed(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Flat(string text, int maxLen)
	{
		string t = text;
		t.Replace("\n", "^");
		return RZ_UIText.Clean(t, maxLen);
	}

	//------------------------------------------------------------------------------------------------
	//! The player's panel as it stands, with `status` (what the last move said).
	protected void Send(int playerId, string status)
	{
		int serial;
		if (!m_mSerial.Find(playerId, serial))
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		IEntity user = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerId);
		if (!ui || !user)
		{
			m_mSerial.Remove(playerId);
			return;
		}

		int step = m_Grid.GetStep();
		float left = 0;
		float total = 0;
		if (step == RZ_PowerGrid.STEP_SPINNING)
		{
			left = m_Grid.GetSpinLeft();
			total = m_Grid.GetSpinTotal();
		}

		int gauge = -1;
		if (step == RZ_PowerGrid.STEP_POWERED)
			gauge = RZ_Util.RoundToInt(m_Grid.GetCondition() * 100);

		int ready = 0;
		if (IsReady() || step > RZ_PowerGrid.STEP_OPEN)
			ready = 1;

		string title = RZ_UIText.Clean(RZ_Util.Capitalize(m_Grid.GetName()), 80);
		string state = RZ_UIText.Clean(m_Grid.StateLine(), 80);
		string payload = string.Format("P|%1|%2|%3|%4|%5", serial, title, state, Flat(m_Grid.Description(), 600), Flat(status, 400));
		payload += string.Format("|%1|%2|%3|%4|%5", step, RZ_Util.RoundToInt(left), RZ_Util.RoundToInt(total), gauge, ready);

		// The board and the card, once the door is open
		if (step != RZ_PowerGrid.STEP_DARK)
		{
			foreach (int i, string name : m_aNames)
			{
				int closed = 0;
				if (m_aClosed[i] || step > RZ_PowerGrid.STEP_OPEN)
					closed = 1;

				payload += string.Format("\nB|%1|%2|%3", i, RZ_UIText.Clean(name, 20), closed);
			}

			array<string> lines = {};
			CardLines(lines);
			foreach (string ln : lines)
			{
				payload += "\nC|" + RZ_UIText.Clean(ln, 80);
			}
		}

		array<int> kinds = {};
		array<string> labels = {};
		array<string> needs = {};
		array<bool> can = {};
		Buttons(playerId, user, kinds, labels, needs, can);
		foreach (int b, int kind : kinds)
		{
			int on = 0;
			if (can[b])
				on = 1;

			payload += string.Format("\nA|%1|%2|%3|%4", b, RZ_UIText.Clean(labels[b], 60), on, RZ_UIText.Clean(needs[b], 60));
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_POWER, payload);
	}
}
