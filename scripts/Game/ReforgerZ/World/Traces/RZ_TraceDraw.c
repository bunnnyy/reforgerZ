//! Doors you can see (docs/TRACES.md, "Doors you can see"): how a marked door looks.
//!
//! - Decals (the default): the server keeps a short replicated list of the marked doors within 80 m
//!   of any player, at most 48, rebuilt on the 5 s tick only when it changes; each client draws
//!   decals for doors within 70 m of itself (UI/RZ_DoorMarkClient.c). Needs World > Traces > Kit >
//!   Door marks atlas; without it, boards.
//! - Boards (the fallback): a board beside each marked door, through the marks spawner's DOORS pool
//!   (24 on the server, 8 a cluster, up within 60 m, down past 90 m). Needs the author's board in
//!   the kit; without one, that door is text only.
//! - Text: nothing to see; the door's words still come when you stand at it.
//! Never both decals and boards. Server only.
class RZ_TraceDraw
{
	static const float DRAW_RADIUS = 80;
	static const int DRAW_MAX = 48;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Traces m_Traces;
	protected int m_iDrawn;

	//------------------------------------------------------------------------------------------------
	void RZ_TraceDraw(RZ_GameModeComponent gm, RZ_Traces traces)
	{
		m_GM = gm;
		m_Traces = traces;
	}

	//------------------------------------------------------------------------------------------------
	//! The drawing in use: Decals only with the atlas set, Boards only with a board set, else Text.
	int Mode()
	{
		RZ_TraceSettings s = m_Traces.GetSettings();
		RZ_TraceKit kit = s.Kit();
		if (s.m_eDrawing == RZ_EDoorDrawing.TEXT)
			return RZ_EDoorDrawing.TEXT;

		if (s.m_eDrawing == RZ_EDoorDrawing.DECALS && !kit.m_sDoorAtlas.IsEmpty())
			return RZ_EDoorDrawing.DECALS;

		if (!kit.m_sBoardBastion.IsEmpty() || !kit.m_sBoardHaven.IsEmpty() || !kit.m_sBoardRedHand.IsEmpty() || !kit.m_sBoardHalcyon.IsEmpty() || !kit.m_sBoardResidents.IsEmpty())
			return RZ_EDoorDrawing.BOARDS;

		return RZ_EDoorDrawing.TEXT;
	}

	//------------------------------------------------------------------------------------------------
	int DrawnCount()
	{
		return m_iDrawn;
	}

	//------------------------------------------------------------------------------------------------
	//! Every 5 s: the replicated list (decals), or nothing to do (boards follow each door's change).
	void Tick(array<IEntity> players)
	{
		if (Mode() != RZ_EDoorDrawing.DECALS)
		{
			m_GM.SetDoorMarks("");
			if (Mode() == RZ_EDoorDrawing.TEXT)
				m_iDrawn = 0;

			return;
		}

		string list = "";
		int n = 0;
		foreach (RZ_MarkedDoor d : m_Traces.GetDoors().GetDoors())
		{
			if (n >= DRAW_MAX)
				break;

			if (!d.IsBound() || d.m_aLayers.IsEmpty() || RZ_Util.NearestDistance(d.m_vPos, players) > DRAW_RADIUS)
				continue;

			string entry = Entry(d);
			if (entry.IsEmpty())
				continue;

			list += entry + ";";
			n++;
		}

		m_iDrawn = n;
		m_GM.SetDoorMarks(list);
	}

	//------------------------------------------------------------------------------------------------
	//! "x y z nx nz cell alpha cell alpha ...": the door's mark point and wall, then a decal per
	//! layer (oldest first) and its overlay. Cells 0-15 are the door atlas, 16-31 the overlays.
	protected string Entry(RZ_MarkedDoor d)
	{
		string cells = "";
		foreach (RZ_DoorMark m : d.m_aLayers)
		{
			int alpha = Alpha(m);
			int cell = Cell(m);
			if (cell >= 0)
				cells += string.Format(" %1 %2", cell, alpha);

			int over = Overlay(m);
			if (over >= 0)
				cells += string.Format(" %1 %2", over, alpha);
		}

		if (cells.IsEmpty())
			return "";

		vector p = d.m_vPos;
		vector n = d.m_vNormal;
		return string.Format("%1 %2 %3 %4 %5%6", Round1(p[0]), Round1(p[1]), Round1(p[2]), Round1(n[0]), Round1(n[2]), cells);
	}

	//------------------------------------------------------------------------------------------------
	protected static string Round1(float v)
	{
		float r = Math.Round(v * 10) / 10;
		return r.ToString();
	}

	//------------------------------------------------------------------------------------------------
	//! The symbol's cell: its place in the door atlas, the infected's and the thumbprint in the
	//! overlays (16 + 4..7); -1 for residents' words (text only).
	static int Cell(RZ_DoorMark m)
	{
		if (m.m_iSymbol <= RZ_EDoorSymbol.BEAK)
			return m.m_iSymbol;

		switch (m.m_iSymbol)
		{
			case RZ_EDoorSymbol.SCRATCH_OUT: return 20;
			case RZ_EDoorSymbol.SCRATCH_IN: return 21;
			case RZ_EDoorSymbol.SMEARS: return 22;
			case RZ_EDoorSymbol.THUMB: return 23;
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! The overlay on top of a layer: grey overpaint, ring, tick, strike or bar, PAID. -1 none.
	static int Overlay(RZ_DoorMark m)
	{
		if (m.Has(RZ_DoorMark.F_OVER) && m.m_iAuthor == RZ_EDoorAuthor.REDHAND)
			return 24;	// grey paint over the crews' marks

		if (m.Has(RZ_DoorMark.F_RING))
			return 16;

		if (m.Has(RZ_DoorMark.F_TICK))
			return 17;

		if (m.Has(RZ_DoorMark.F_STRUCK) || m.Has(RZ_DoorMark.F_BAR))
			return 18;

		if (m.Has(RZ_DoorMark.F_PAID))
			return 19;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Age as alpha: fresh 255, weathered 170, old 90; painted over 60.
	protected int Alpha(RZ_DoorMark m)
	{
		if (m.Has(RZ_DoorMark.F_OVER))
			return 60;

		int stage = m_Traces.GetDoors().Stage(m);
		if (stage == 0)
			return 255;

		if (stage == 1)
			return 170;

		return 90;
	}

	// ================================================================ Boards
	//------------------------------------------------------------------------------------------------
	protected string BoardKey(RZ_MarkedDoor d)
	{
		return "traces.door." + d.m_sKey;
	}

	//------------------------------------------------------------------------------------------------
	//! The top layer's author's board, leaned beside the door and facing out; text only without one.
	void DoorChanged(RZ_MarkedDoor d)
	{
		RZ_Marks marks = RZ_Marks.Get();
		if (!marks || Mode() != RZ_EDoorDrawing.BOARDS)
			return;

		RZ_DoorMark top = d.Top();
		if (!top || !d.IsBound())
		{
			marks.Remove(BoardKey(d));
			return;
		}

		ResourceName board = Board(top.m_iAuthor);
		if (board.IsEmpty())
		{
			marks.Remove(BoardKey(d));
			return;
		}

		float yaw = Math.Atan2(d.m_vNormal[0], d.m_vNormal[2]) * Math.RAD2DEG;
		vector at = RZ_Util.GroundPos(d.m_vPos + d.m_vNormal * 0.3);
		marks.Place(RZ_EMarkPool.DOORS, BoardKey(d), at, board, "", d.Salience(), -1, false, false, yaw);
		m_iDrawn = marks.Count(RZ_EMarkPool.DOORS);
	}

	//------------------------------------------------------------------------------------------------
	void DoorRemoved(RZ_MarkedDoor d)
	{
		RZ_Marks marks = RZ_Marks.Get();
		if (marks)
			marks.Remove(BoardKey(d));
	}

	//------------------------------------------------------------------------------------------------
	protected ResourceName Board(int author)
	{
		RZ_TraceKit kit = m_Traces.GetKit();
		switch (author)
		{
			case RZ_EDoorAuthor.BASTION: return kit.m_sBoardBastion;
			case RZ_EDoorAuthor.HAVEN: return kit.m_sBoardHaven;
			case RZ_EDoorAuthor.REDHAND: return kit.m_sBoardRedHand;
			case RZ_EDoorAuthor.HALCYON: return kit.m_sBoardHalcyon;
		}

		return kit.m_sBoardResidents;
	}
}
