//! The hands-on view (docs/UI.md, "The hands-on view"): a game's thing up close, in the world. The
//! camera comes in on the real person or prop standing there, its places to look at glow on it, and a
//! panel beside it keeps what you've found. Picking a place eases the camera onto it, you look (a
//! short hold), and what you see shows at the place, with a flash for a bad find; a place already
//! looked at shows again for nothing. The game's own buttons (take them up, ...) sit in the panel.
//! It is one prop of the game, never the whole game: the game is still played with your feet and
//! hands at the place (docs/UI.md, "Default first, then menus").
//!
//! A game feeds it as data (RZ_HandsOnGame, like the object menu's rows); the server holds the truth
//! and checks every look. Payload (RZ_UIFeed.PAYLOAD_HANDS): H, T, P, L, A and E records; X|reason
//! closes. The client (UI/RZ_HandsOnMenu.c) answers "look", "act", "closed" or "failed". A client
//! that can't show it says "failed" and the game goes back to its prompts for that player. Server
//! only, but for the records' format.

//------------------------------------------------------------------------------------------------
//! A place on the thing to look at.
class RZ_HandsOnSpot
{
	string m_sLabel;				// what you do there ("Have them roll their sleeves")
	string m_sBone;					// a person's bone it sits on ("" none: the offset alone)
	vector m_vOffset;				// from the bone, or from the thing's origin in its own frame (x right, y up, z forward)
	int m_iState;					// RZ_HandsOn.SPOT_*
	int m_iTone;					// RZ_HandsOn.TONE_* of what was seen
	string m_sSeen;					// what was seen there ("" not yet)
}

//------------------------------------------------------------------------------------------------
//! What a game puts in the view. Every method is asked again each time the view is sent.
class RZ_HandsOnGame
{
	//------------------------------------------------------------------------------------------------
	//! The thing up close (null: only its place, Where()).
	IEntity Subject() { return null; }

	//------------------------------------------------------------------------------------------------
	//! Where it stands (when it has no entity, or the client can't see it).
	vector Where() { return vector.Zero; }

	//------------------------------------------------------------------------------------------------
	//! Which way it faces, in degrees (the same).
	float Facing() { return 0; }

	//------------------------------------------------------------------------------------------------
	//! How the camera frames it: RZ_HandsOn.FRAME_*.
	int Frame() { return RZ_HandsOn.FRAME_THING; }

	//------------------------------------------------------------------------------------------------
	string Title() { return ""; }

	//------------------------------------------------------------------------------------------------
	//! Under the title: looks left, the clock.
	string Status() { return ""; }

	//------------------------------------------------------------------------------------------------
	//! What's free to see and the rules.
	string Text() { return ""; }

	//------------------------------------------------------------------------------------------------
	void Spots(notnull array<ref RZ_HandsOnSpot> spots) {}

	//------------------------------------------------------------------------------------------------
	//! "What you found": everything said about it, oldest first, each with its tone.
	void Log(notnull array<string> lines, notnull array<int> tones) {}

	//------------------------------------------------------------------------------------------------
	//! The panel's buttons (up to three), in order.
	void Actions(notnull array<string> labels) {}

	//------------------------------------------------------------------------------------------------
	//! Place `spot` looked at by `playerId` (checked: it's there). What was seen goes in the spot
	//! (m_sSeen, m_iTone) and, when it's new, in the log.
	void Look(int spot, int playerId) {}

	//------------------------------------------------------------------------------------------------
	//! Button `index` pressed. Return true to close the view.
	bool Act(int index, int playerId) { return false; }

	//------------------------------------------------------------------------------------------------
	//! The view closed (the player, the guard, the game).
	void Closed(int playerId) {}

	//------------------------------------------------------------------------------------------------
	//! This player's client can't show the view: the game's prompts for them.
	void Failed(int playerId) {}
}

//------------------------------------------------------------------------------------------------
class RZ_HandsOn
{
	static const int FRAME_PERSON = 0;		// someone standing: the camera at chest height in front
	static const int FRAME_LYING = 1;		// someone lying: from the side and above
	static const int FRAME_SMALL = 2;		// a small thing on a bench or table: close and above
	static const int FRAME_THING = 3;		// a crate, a pot, a stove: a step back and above

	static const int SPOT_OPEN = 0;
	static const int SPOT_SEEN = 1;			// looked at: shows again for nothing
	static const int SPOT_SHUT = 2;			// can't be looked at now (no looks left)

	static const int TONE_PLAIN = 0;
	static const int TONE_CLEAN = 1;
	static const int TONE_ODD = 2;
	static const int TONE_BAD = 3;

	protected static const float GUARD_RANGE = 6;	// further than this from the thing, the view closes
	protected static const float LOOK_GAP = 1.2;	// a look takes this long at least (the client's hold)

	protected static ref map<int, ref RZ_HandsOnGame> s_mGames;
	protected static ref map<int, float> s_mLastLook;
	protected static ref map<int, int> s_mSerial;	// each look's result, numbered, so the client shows it once
	protected static ref map<int, string> s_mEvent;	// the last look's result record
	protected static ref array<int> s_aFailed;		// players whose client can't show it
	protected static bool s_bTicking;

	//------------------------------------------------------------------------------------------------
	protected static void Ensure()
	{
		if (s_mGames)
			return;

		s_mGames = new map<int, ref RZ_HandsOnGame>();
		s_mLastLook = new map<int, float>();
		s_mSerial = new map<int, int>();
		s_mEvent = new map<int, string>();
		s_aFailed = {};
	}

	//------------------------------------------------------------------------------------------------
	//! This player's client can show the view (it hasn't said it can't).
	static bool CanShow(int playerId)
	{
		Ensure();
		return playerId > 0 && !s_aFailed.Contains(playerId) && RZ_PlayerUIComponent.GetFor(playerId) != null;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsOpen(int playerId)
	{
		Ensure();
		return s_mGames.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! The game open for this player, or null.
	static RZ_HandsOnGame GameOf(int playerId)
	{
		Ensure();
		return s_mGames.Get(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens the view on `game` for the player (another one they had open closes first).
	static void Open(int playerId, RZ_HandsOnGame game)
	{
		Ensure();
		if (!game || !CanShow(playerId))
			return;

		RZ_HandsOnGame before = s_mGames.Get(playerId);
		if (before && before != game)
			before.Closed(playerId);

		s_mGames.Set(playerId, game);
		s_mLastLook.Set(playerId, 0);
		s_mEvent.Set(playerId, "");
		RZ_Log.Info(string.Format("Hands-on: %1 opened \"%2\".", RZ_Util.PlayerName(playerId), game.Title()));
		Send(playerId);
		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Guard, 1000, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The game changed: sent again to whoever has it open.
	static void Refresh(RZ_HandsOnGame game)
	{
		Ensure();
		foreach (int pid, RZ_HandsOnGame g : s_mGames)
		{
			if (g == game)
				Send(pid);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Closed by the server: the client is told, the game hears it.
	static void Close(int playerId, string reason = "")
	{
		Ensure();
		RZ_HandsOnGame game = s_mGames.Get(playerId);
		if (!game)
			return;

		s_mGames.Remove(playerId);
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_HANDS, "X" + RZ_UIText.SEP + RZ_UIText.Clean(reason));

		game.Closed(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Every view of this game closes (it's over or gone).
	static void CloseGame(RZ_HandsOnGame game)
	{
		Ensure();
		array<int> who = {};
		foreach (int pid, RZ_HandsOnGame g : s_mGames)
		{
			if (g == game)
				who.Insert(pid);
		}

		foreach (int p : who)
		{
			Close(p);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: a view whose player walked off, died or left closes.
	protected static void Guard()
	{
		array<int> shut = {};
		foreach (int pid, RZ_HandsOnGame g : s_mGames)
		{
			IEntity p = GetGame().GetPlayerManager().GetPlayerControlledEntity(pid);
			vector at = g.Where();
			if (g.Subject())
				at = g.Subject().GetOrigin();

			if (!p || !RZ_Util.IsCharacterAlive(p) || RZ_Util.DistXZ(p.GetOrigin(), at) > GUARD_RANGE)
				shut.Insert(pid);
		}

		foreach (int s : shut)
		{
			Close(s);
		}

		if (s_mGames.IsEmpty())
		{
			s_bTicking = false;
			GetGame().GetCallqueue().Remove(Guard);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! What the client sent (RZ_PlayerUIComponent.RpcAsk_HandsAct).
	static void ClientAct(int playerId, string act, string arg)
	{
		Ensure();
		if (act == "failed")
		{
			if (!s_aFailed.Contains(playerId))
				s_aFailed.Insert(playerId);

			RZ_HandsOnGame lost = s_mGames.Get(playerId);
			s_mGames.Remove(playerId);
			RZ_Log.Info(string.Format("Hands-on: %1's client can't show the view; the prompts instead.", RZ_Util.PlayerName(playerId)));
			if (lost)
				lost.Failed(playerId);

			return;
		}

		RZ_HandsOnGame game = s_mGames.Get(playerId);
		if (!game)
			return;

		if (act == "closed")
		{
			s_mGames.Remove(playerId);
			game.Closed(playerId);
			return;
		}

		if (act == "look")
		{
			float now = GetGame().GetWorld().GetWorldTime() * 0.001;
			if (now - s_mLastLook.Get(playerId) < LOOK_GAP * 0.5)
				return;

			array<ref RZ_HandsOnSpot> spots = {};
			game.Spots(spots);
			int i = arg.ToInt();
			if (i < 0 || i >= spots.Count() || spots[i].m_iState == SPOT_SHUT)
				return;

			s_mLastLook.Set(playerId, now);
			game.Look(i, playerId);
			// What it found now, for the client to show at the place once
			array<ref RZ_HandsOnSpot> after = {};
			game.Spots(after);
			int serial = s_mSerial.Get(playerId) + 1;
			s_mSerial.Set(playerId, serial);
			if (i < after.Count())
			{
				RZ_UILine ev = new RZ_UILine("E");
				ev.AddInt(i).Add(after[i].m_sSeen).AddInt(after[i].m_iTone).AddInt(serial);
				s_mEvent.Set(playerId, ev.End());
			}

			Send(playerId);
			return;
		}

		if (act == "act")
		{
			if (game.Act(arg.ToInt(), playerId))
				Close(playerId);
			else if (s_mGames.Contains(playerId))
				Send(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Send(int playerId)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_HandsOnGame game = s_mGames.Get(playerId);
		if (!ui || !game)
			return;

		ui.SendPayload(RZ_UIFeed.PAYLOAD_HANDS, Payload(game) + s_mEvent.Get(playerId));
	}

	//------------------------------------------------------------------------------------------------
	//! H|title|status|x|y|z|yaw|rplId|frame, T|text, P|i|label|bone|ox|oy|oz|state|tone|seen,
	//! L|line|tone, A|i|label.
	static string Payload(RZ_HandsOnGame game)
	{
		vector pos = game.Where();
		float yaw = game.Facing();
		int rpl = 0;
		IEntity subject = game.Subject();
		if (subject)
		{
			pos = subject.GetOrigin();
			yaw = subject.GetYawPitchRoll()[0];
			RplComponent rc = RplComponent.Cast(subject.FindComponent(RplComponent));
			if (rc)
				rpl = rc.Id();
		}

		string text = game.Text();
		text.Replace("\n", "^");
		RZ_UILine head = new RZ_UILine("H");
		head.Add(game.Title()).Add(game.Status()).Add(pos[0].ToString()).Add(pos[1].ToString()).Add(pos[2].ToString()).Add(yaw.ToString()).AddInt(rpl).AddInt(game.Frame());
		string data = head.End();
		RZ_UILine body = new RZ_UILine("T");
		body.Add(text);
		data += body.End();
		array<ref RZ_HandsOnSpot> spots = {};
		game.Spots(spots);
		foreach (int i, RZ_HandsOnSpot s : spots)
		{
			RZ_UILine line = new RZ_UILine("P");
			line.AddInt(i).Add(s.m_sLabel).Add(s.m_sBone).Add(s.m_vOffset[0].ToString()).Add(s.m_vOffset[1].ToString()).Add(s.m_vOffset[2].ToString());
			data += line.AddInt(s.m_iState).AddInt(s.m_iTone).Add(s.m_sSeen).End();
		}

		array<string> lines = {};
		array<int> tones = {};
		game.Log(lines, tones);
		int first = Math.Max(0, lines.Count() - 10);
		for (int l = first; l < lines.Count(); l++)
		{
			int tone = 0;
			if (l < tones.Count())
				tone = tones[l];

			RZ_UILine logLine = new RZ_UILine("L");
			logLine.Add(lines[l]).AddInt(tone);
			data += logLine.End();
		}

		array<string> labels = {};
		game.Actions(labels);
		foreach (int a, string label : labels)
		{
			RZ_UILine actLine = new RZ_UILine("A");
			actLine.AddInt(a).Add(label);
			data += actLine.End();
		}

		return data;
	}
}
