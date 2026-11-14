//! The map's fog on the client (docs/UI.md, "The map's fog"): the squares this character hasn't
//! seen, drawn over the map on a canvas of their own, under the icon layer's markers. The server
//! sends the whole fog as run lengths (World/RZ_MapFog.c, payload kind 25); here the unseen runs are
//! merged into rectangles once per change, and the rectangles are turned into screen polygons only
//! when the map pans or zooms or the fog changes. Owned by the map's icon layer (RZ_MapIconsUI).
class RZ_MapFogDraw
{
	protected CanvasWidget m_wCanvas;
	protected ref array<ref CanvasWidgetCommand> m_aDraw = {};
	protected int m_iSeen = -1;			// the fog's version last parsed
	protected float m_fCheckX = -100000;	// where two fixed spots fell on screen last frame: the map moved if they moved
	protected float m_fCheckY;
	protected float m_fCheckX2;
	protected bool m_bOn;
	protected int m_iColor;

	// The unseen rectangles, in squares: x, z, width, height (z up)
	protected ref array<int> m_aRect = {};
	protected float m_fX0;
	protected float m_fZ0;
	protected float m_fCell;
	protected int m_iW;			// the grid's size in squares
	protected int m_iH;
	protected static const float BEYOND = 30000;	// how far past the grid the fog reaches (m)

	//------------------------------------------------------------------------------------------------
	//! The canvas, made first so the markers' canvas and icons draw above it.
	void Make(Widget root)
	{
		Remove();
		m_wCanvas = RZ_UIW.MakeCanvas(root);
		m_iSeen = -1;
		m_fCheckX = -100000;
	}

	//------------------------------------------------------------------------------------------------
	void Remove()
	{
		RZ_UIW.Remove(m_wCanvas);
		m_wCanvas = null;
		m_aDraw.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! Every frame while the map is open: parse a new fog, redraw if the map moved.
	void Update(SCR_MapEntity mapEnt)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!m_wCanvas || !ui || !mapEnt)
			return;

		bool changed = false;
		if (ui.GetFogVersion() != m_iSeen)
		{
			m_iSeen = ui.GetFogVersion();
			Parse(ui.GetFog());
			changed = true;
		}

		if (!m_bOn)
		{
			if (changed)
			{
				m_aDraw.Clear();
				RZ_UIW.Draw(m_wCanvas, m_aDraw);
			}

			return;
		}

		// The map moved or zoomed: two fixed spots land somewhere else on screen
		float ax, ay, bx, by;
		if (!RZ_UIW.MapToScreen(mapEnt, m_fX0, m_fZ0, ax, ay) || !RZ_UIW.MapToScreen(mapEnt, m_fX0 + 1000, m_fZ0, bx, by))
			return;

		if (!changed && ax == m_fCheckX && ay == m_fCheckY && bx == m_fCheckX2)
			return;

		m_fCheckX = ax;
		m_fCheckY = ay;
		m_fCheckX2 = bx;
		Draw(mapEnt);
	}

	//------------------------------------------------------------------------------------------------
	//! F|x0|z0|cell|w|h|opacity|1 then R|runs (unseen first). Unseen runs merged down rows into
	//! rectangles: a run that starts and ends where the one below did grows that rectangle.
	protected void Parse(string payload)
	{
		m_aRect.Clear();
		m_bOn = false;
		if (payload.IsEmpty())
			return;

		array<string> lines = {};
		RZ_UIText.Split(payload, "\n", lines);
		array<string> head = {};
		string runText = "";
		foreach (string ln : lines)
		{
			if (ln.StartsWith("F|"))
				RZ_UIText.Split(ln, RZ_UIText.SEP, head);
			else if (ln.StartsWith("R|"))
				runText = ln.Substring(2, ln.Length() - 2);
		}

		if (head.Count() < 7)
			return;

		m_fX0 = RZ_UIText.Int(head, 1);
		m_fZ0 = RZ_UIText.Int(head, 2);
		m_fCell = Math.Max(1, RZ_UIText.Int(head, 3));
		int w = RZ_UIText.Int(head, 4);
		int h = RZ_UIText.Int(head, 5);
		float alpha = Math.Clamp(RZ_UIText.Int(head, 6) / 100.0, 0, 1);
		m_iColor = Color.FromRGBA(46, 48, 50, RZ_Util.RoundToInt(alpha * 255)).PackToInt();
		if (w <= 0 || h <= 0)
			return;

		m_iW = w;
		m_iH = h;

		array<string> parts = {};
		RZ_UIText.Split(runText, ",", parts);

		// Unseen runs per row, then merged: open[k] is a rectangle still growing, keyed by "start,end"
		map<string, int> open = new map<string, int>();
		map<string, int> next = new map<string, int>();
		int partIdx = 0;
		bool unseen = true;
		int left = 0;
		if (!parts.IsEmpty())
			left = parts[0].ToInt();

		for (int z = 0; z < h; z++)
		{
			next.Clear();
			int x = 0;
			while (x < w)
			{
				// Skip used-up runs
				while (left <= 0 && partIdx + 1 < parts.Count())
				{
					partIdx++;
					unseen = !unseen;
					left = parts[partIdx].ToInt();
				}

				if (left <= 0)
				{
					// The runs ran out: the rest counts as seen
					unseen = false;
					left = w * h;
				}

				int span = Math.Min(left, w - x);
				if (unseen)
				{
					string key = x.ToString() + "," + (x + span).ToString();
					int idx;
					if (open.Find(key, idx))
					{
						m_aRect[idx * 4 + 3] = m_aRect[idx * 4 + 3] + 1;
					}
					else
					{
						idx = m_aRect.Count() / 4;
						m_aRect.Insert(x);
						m_aRect.Insert(z);
						m_aRect.Insert(span);
						m_aRect.Insert(1);
					}

					next.Insert(key, idx);
				}

				x += span;
				left -= span;
			}

			open.Clear();
			foreach (string k, int v : next)
			{
				open.Insert(k, v);
			}
		}

		m_bOn = true;
	}

	//------------------------------------------------------------------------------------------------
	//! The rectangles as screen polygons in the fog's grey.
	//! VERIFY: PolygonDrawCommand on a CanvasWidget over the full-screen map draws filled quads in
	//! screen pixels, a few hundred a frame without a hitch, and its alpha shows the map's roads and
	//! coast faintly through
	protected void Draw(SCR_MapEntity mapEnt)
	{
		m_aDraw.Clear();
		DrawBeyond(mapEnt);
		int n = m_aRect.Count() / 4;
		for (int i = 0; i < n; i++)
		{
			float wx0 = m_fX0 + m_aRect[i * 4] * m_fCell;
			float wz0 = m_fZ0 + m_aRect[i * 4 + 1] * m_fCell;
			float wx1 = wx0 + m_aRect[i * 4 + 2] * m_fCell;
			float wz1 = wz0 + m_aRect[i * 4 + 3] * m_fCell;
			float sx0, sy0, sx1, sy1;
			if (!RZ_UIW.MapToScreen(mapEnt, wx0, wz0, sx0, sy0) || !RZ_UIW.MapToScreen(mapEnt, wx1, wz1, sx1, sy1))
				continue;

			float px0 = RZ_UIW.Native(sx0);
			float py0 = RZ_UIW.Native(sy0);
			float px1 = RZ_UIW.Native(sx1);
			float py1 = RZ_UIW.Native(sy1);
			PolygonDrawCommand quad = new PolygonDrawCommand();
			quad.m_iColor = m_iColor;
			quad.m_Vertices = {px0, py0, px1, py0, px1, py1, px0, py1};
			m_aDraw.Insert(quad);
		}

		RZ_UIW.Draw(m_wCanvas, m_aDraw);
	}

	//------------------------------------------------------------------------------------------------
	//! Everything past the fog's grid (the sea and the map's margins) is never seen: four bands
	//! round the grid, BEYOND metres wide, so no edge of the map shows through.
	protected void DrawBeyond(SCR_MapEntity mapEnt)
	{
		float gx0 = m_fX0;
		float gz0 = m_fZ0;
		float gx1 = m_fX0 + m_iW * m_fCell;
		float gz1 = m_fZ0 + m_iH * m_fCell;
		float ox0 = gx0 - BEYOND;
		float oz0 = gz0 - BEYOND;
		float ox1 = gx1 + BEYOND;
		float oz1 = gz1 + BEYOND;
		AddQuad(mapEnt, ox0, oz0, gx0, oz1);	// west
		AddQuad(mapEnt, gx1, oz0, ox1, oz1);	// east
		AddQuad(mapEnt, gx0, oz0, gx1, gz0);	// south
		AddQuad(mapEnt, gx0, gz1, gx1, oz1);	// north
	}

	//------------------------------------------------------------------------------------------------
	protected void AddQuad(SCR_MapEntity mapEnt, float wx0, float wz0, float wx1, float wz1)
	{
		float sx0, sy0, sx1, sy1;
		if (!RZ_UIW.MapToScreen(mapEnt, wx0, wz0, sx0, sy0) || !RZ_UIW.MapToScreen(mapEnt, wx1, wz1, sx1, sy1))
			return;

		float px0 = RZ_UIW.Native(sx0);
		float py0 = RZ_UIW.Native(sy0);
		float px1 = RZ_UIW.Native(sx1);
		float py1 = RZ_UIW.Native(sy1);
		PolygonDrawCommand quad = new PolygonDrawCommand();
		quad.m_iColor = m_iColor;
		quad.m_Vertices = {px0, py0, px1, py0, px1, py1, px0, py1};
		m_aDraw.Insert(quad);
	}
}
