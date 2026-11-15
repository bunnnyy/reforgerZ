//! The reader (docs/UI.md, "The reader"): notes, letters, orders and flyers open on a sheet of
//! paper in the middle of the screen, a page at a time. You can't move while it's open; danger
//! closes it (UI/RZ_ScreenGuard.c). The text only ever goes to its player.

//------------------------------------------------------------------------------------------------
//! Server: show a readable thing to one player.
class RZ_Reader
{
	protected static const int MAX_TEXT = 5600;

	//------------------------------------------------------------------------------------------------
	//! title: what it is ("Note on the dashboard"); how: one line on how it looks ("Folded twice,
	//! in pencil"), or ""; text: what it says (paragraphs split by line breaks), which may start with
	//! a drawn figure's tag (RZ_PaperFigure: a calendar, chalk on a door, a pencil map). With **UI > Reader**
	//! off, no UI component on the player's controller, or the server at its cap of open screens,
	//! it's a notice card instead, as before; while a screen acts for the player (the object menu
	//! reading what's on a prop), it's that screen's text.
	static void Show(RZ_GameModeComponent gm, int playerId, string title, string how, string text)
	{
		if (!gm)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(playerId);
		RZ_ScreenGuard guard = gm.GetScreenGuard();
		// A screen acting for the player (the object menu reading a prop) shows it in its own text;
		// a drawn page (RZ_PaperFigure) is said in words there
		if (RZ_ScreenCapture.IsCapturing(playerId) || !ui || !guard || !RZ_UISettings.Get().m_bReader || !guard.Open(playerId, RZ_ScreenGuard.KIND_READER, null, "", -1))
		{
			gm.NotifyPlayer(playerId, title, RZ_PaperFigure.AsWords(text));
			return;
		}

		string figure = RZ_PaperFigure.SpecOf(text);
		string body = RZ_PaperFigure.WordsOf(text);
		if (body.Length() > MAX_TEXT)
			body = body.Substring(0, MAX_TEXT - 3) + "...";

		string head = "P" + RZ_UIText.SEP + RZ_UIText.Clean(title, 120) + RZ_UIText.SEP + RZ_UIText.Clean(how, 160) + RZ_UIText.SEP + RZ_UIText.Clean(figure, 200);
		ui.SendPayload(RZ_UIFeed.PAYLOAD_READER, head + "\n" + body);
	}

	//------------------------------------------------------------------------------------------------
	//! A read label that means paper: "Read the note", "Read the flyer" (not "Look at", "Listen",
	//! "Search", "Talk to").
	static bool IsPaperLabel(string label)
	{
		string l = label;
		l.ToLower();
		return l.IndexOf("read") == 0;
	}
}

//------------------------------------------------------------------------------------------------
//! Drawn pages (docs/UI.md, "Drawn pages"): a reader text that is a picture (a calendar crossed off,
//! chalk on a door, a pencil map with a place ringed) carries the picture as a tag at its very start,
//! "[fig:kind;field;field]", then a line break and only the words a picture can't say. The server
//! takes the tag off and sends it in the reader's head line; the reader draws it
//! (RZ_PaperFigureDraw). Anywhere it can't be drawn (a notice card, the object menu's own text) the
//! tag is said in words instead (Describe), so nothing is lost.
class RZ_PaperFigure
{
	static const string OPEN = "[fig:";
	static const string CLOSE = "]";
	static const string FIELD = ";";
	static const int CAPTION_CHARS = 170;	// words up to this many share the figure's page; longer go on the next pages

	//------------------------------------------------------------------------------------------------
	//! This month's page: days 1 to crossedTo struck through in pencil (the last few shaky), day
	//! `ringed` ringed with `note` written in it (0: none).
	static string Calendar(int crossedTo, int ringed, string note)
	{
		return string.Format("%1calendar%2%3%2%4%2%5%6", OPEN, FIELD, crossedTo, ringed, Field(note), CLOSE);
	}

	//------------------------------------------------------------------------------------------------
	//! Chalk on a door: up to three lines of capitals, then `strokes` tally strokes in gates of five.
	static string DoorChalk(int strokes, string line1, string line2, string line3)
	{
		return string.Format("%1door%2%3%2%4%2%5%2%6%7", OPEN, FIELD, strokes, Field(line1), Field(line2), Field(line3), CLOSE);
	}

	//------------------------------------------------------------------------------------------------
	//! A pencil map of the district: a few roads and villages, `place` written in and ringed twice.
	static string RingedMap(string place)
	{
		return string.Format("%1map%2%3%4", OPEN, FIELD, Field(place), CLOSE);
	}

	//------------------------------------------------------------------------------------------------
	//! A field can't hold the tag's own marks.
	protected static string Field(string s)
	{
		string f = s;
		f.Replace(FIELD, ",");
		f.Replace(CLOSE, ")");
		f.Replace(RZ_UIText.SEP, "/");
		f.Replace("\n", " ");
		return f;
	}

	//------------------------------------------------------------------------------------------------
	//! The figure at the start of `text` ("calendar;14;2;ROADS SHUT"), "" if it has none.
	static string SpecOf(string text)
	{
		if (text.IndexOf(OPEN) != 0)
			return "";

		int close = text.IndexOf(CLOSE);
		if (close < OPEN.Length())
			return "";

		return text.Substring(OPEN.Length(), close - OPEN.Length());
	}

	//------------------------------------------------------------------------------------------------
	//! `text` without its figure's tag: the words under the picture.
	static string WordsOf(string text)
	{
		if (SpecOf(text).IsEmpty())
			return text;

		int close = text.IndexOf(CLOSE);
		string rest = text.Substring(close + 1, text.Length() - close - 1);
		return rest.Trim();
	}

	//------------------------------------------------------------------------------------------------
	//! `text` with its figure said in words, for where it can't be drawn.
	static string AsWords(string text)
	{
		string spec = SpecOf(text);
		if (spec.IsEmpty())
			return text;

		string said = Describe(spec);
		string words = WordsOf(text);
		if (words.IsEmpty())
			return said;

		if (said.IsEmpty())
			return words;

		return said + "\n" + words;
	}

	//------------------------------------------------------------------------------------------------
	//! A figure in words (the notice card's and the object menu's text).
	static string Describe(string spec)
	{
		array<string> f = {};
		RZ_UIText.Split(spec, FIELD, f);
		string kind = RZ_UIText.Str(f, 0);
		if (kind == "calendar")
		{
			string cal = string.Format("This month's page. Every day up to Day %1 is crossed off in pencil, one stroke a morning, and the last few strokes shake.", RZ_UIText.Int(f, 1));
			string note = RZ_UIText.Str(f, 3);
			if (RZ_UIText.Int(f, 2) > 0 && !note.IsEmpty())
				cal += string.Format(" Day %1 has a ring round it and, by it: %2.", RZ_UIText.Int(f, 2), note);

			return cal;
		}

		if (kind == "door")
		{
			string chalk = "";
			for (int i = 2; i <= 4; i++)
			{
				string line = RZ_UIText.Str(f, i);
				if (!line.IsEmpty())
					chalk = chalk + " " + line;
			}

			string door = "Chalk on the inside of the door, in big careful capitals:" + chalk;
			int strokes = RZ_UIText.Int(f, 1);
			if (strokes > 0)
				door += string.Format(" Under it, %1 strokes.", strokes);

			return door;
		}

		if (kind == "map")
			return string.Format("A map of the district in pencil, %1 circled twice.", RZ_UIText.Str(f, 1));

		return "";
	}
}

//------------------------------------------------------------------------------------------------
//! Draws a drawn page's figure (RZ_PaperFigure) into the reader's PaperFigure frame (client), 816 ×
//! 380 layout units: a canvas under for fills (the door), the words as labels (RZ_PaperLabel.layout,
//! made from script), a canvas over for the pencil and chalk strokes. Hand-drawn wobble comes from
//! the spec itself, so a page looks the same every time it's opened.
//! VERIFY: canvases and labels made in a FrameWidget draw in the order they were made (the later
//! on top), as the map's fog and markers do.
class RZ_PaperFigureDraw
{
	static const ResourceName LABEL_LAYOUT = "{5B10833179A5F7BE}UI/layouts/ReforgerZ/RZ_PaperLabel.layout";
	static const float W = 816;
	static const float H = 380;
	protected static const int MONTH = 31;		// the calendar's days ...
	protected static const int FIRST = 3;		// ... the 1st on a Thursday (columns Monday to Sunday) ...
	protected static const int WEEKS = 5;		// ... so five rows

	protected Widget m_wFrame;
	protected ref array<ref CanvasWidgetCommand> m_aBack = {};
	protected ref array<ref CanvasWidgetCommand> m_aFront = {};
	protected CanvasWidget m_wBack;
	protected CanvasWidget m_wFront;
	protected int m_iSeed;

	//------------------------------------------------------------------------------------------------
	//! Draws `spec` in `frame` (what it drew before goes). False if there's nothing to draw.
	bool Draw(Widget frame, string spec)
	{
		Clear();
		m_wFrame = frame;
		if (!frame || spec.IsEmpty())
			return false;

		array<string> f = {};
		RZ_UIText.Split(spec, RZ_PaperFigure.FIELD, f);
		m_iSeed = spec.Length() * 131 + RZ_UIText.Int(f, 1) * 17 + 7;
		m_wBack = RZ_UIW.MakeCanvas(frame);
		string kind = RZ_UIText.Str(f, 0);
		bool drawn = true;
		if (kind == "calendar")
			DrawCalendar(f);
		else if (kind == "door")
			DrawDoor(f);
		else if (kind == "map")
			DrawMap(f);
		else
			drawn = false;

		// The strokes last, over the words (a day is crossed off over its number)
		m_wFront = RZ_UIW.MakeCanvas(frame);
		RZ_UIW.Draw(m_wBack, m_aBack);
		RZ_UIW.Draw(m_wFront, m_aFront);
		return drawn;
	}

	//------------------------------------------------------------------------------------------------
	void Clear()
	{
		m_aBack.Clear();
		m_aFront.Clear();
		m_wBack = null;
		m_wFront = null;
		RZ_UIW.ClearChildren(m_wFrame);
	}

	//------------------------------------------------------------------------------------------------
	//! The month's grid printed in faint ink, the weekdays over it, the dates; every day up to the
	//! crossed one struck through bottom left to top right, one stroke a morning, the last four
	//! shakier each day; the ringed day ringed, its note written in it.
	protected void DrawCalendar(array<string> f)
	{
		int crossed = Math.ClampInt(RZ_UIText.Int(f, 1), 0, MONTH);
		int ringed = RZ_UIText.Int(f, 2);
		string note = RZ_UIText.Str(f, 3);
		Color ink = RZ_UIW.Rgb(28, 26, 22);
		Color pencil = Color.FromRGBA(58, 58, 64, 235);
		int printed = Color.FromRGBA(28, 26, 22, 110).PackToInt();
		int lead = pencil.PackToInt();
		float cw = W / 7;
		float top = 36;
		float ch = (H - top) / WEEKS;

		array<string> weekdays = {"M", "T", "W", "T", "F", "S", "S"};
		foreach (int c, string wd : weekdays)
		{
			Label(wd, c * cw, 0, cw, 32, RZ_UIW.Rgb(95, 92, 85));
		}

		for (int r = 0; r <= WEEKS; r++)
		{
			StraightLine(m_aBack, 0, top + r * ch, W, top + r * ch, printed, 1.5);
		}

		for (int col = 0; col <= 7; col++)
		{
			StraightLine(m_aBack, col * cw, top, col * cw, top + WEEKS * ch, printed, 1.5);
		}

		for (int d = 1; d <= MONTH; d++)
		{
			int cell = FIRST + d - 1;
			int column = cell % 7;
			float x = column * cw;
			float y = top + (cell / 7) * ch;
			Label(d.ToString(), x + 4, y + 2, 44, 30, ink);
			if (d <= crossed)
			{
				// One stroke a morning; the last four shake more each day
				float shake = 0.8;
				int late = crossed - d;
				if (late < 4)
					shake = 2 + (3 - late) * 1.8;

				Pencil(m_aFront, x + 14, y + ch - 10, x + cw - 14, y + 10, shake, d, lead, 3);
			}

			if (d == ringed)
			{
				Ring(m_aFront, x + 26, y + 18, 30, 21, d, lead, 2.5);
				if (!note.IsEmpty())
					Label(note, x, y + ch - 32, cw * 1.6, 30, pencil);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The inside of a door, painted dark, two panels and a handle; the chalk words on it; under
	//! them the strokes in gates of five (four up, the fifth across them).
	protected void DrawDoor(array<string> f)
	{
		int strokes = Math.ClampInt(RZ_UIText.Int(f, 1), 0, 40);
		float x0 = 188;
		float x1 = 628;
		Rect(m_aBack, x0, 0, x1, H, Color.FromRGBA(62, 56, 48, 255).PackToInt());
		int edge = Color.FromRGBA(88, 80, 68, 255).PackToInt();
		Box(m_aBack, x0 + 24, 20, x1 - 24, 176, edge, 3);
		Box(m_aBack, x0 + 24, 200, x1 - 24, H - 20, edge, 3);
		Rect(m_aBack, x1 - 52, 186, x1 - 34, 214, Color.FromRGBA(150, 140, 120, 255).PackToInt());

		Color chalk = Color.FromRGBA(236, 232, 220, 240);
		for (int i = 0; i < 3; i++)
		{
			string line = RZ_UIText.Str(f, 2 + i);
			if (!line.IsEmpty())
				Label(line, x0 + 30, 34 + i * 46, x1 - x0 - 60, 40, chalk);
		}

		// Gates of five: four strokes up, the fifth across them
		int white = chalk.PackToInt();
		float sx = x0 + 70;
		float sy = 230;
		for (int s = 0; s < strokes; s++)
		{
			int gate = s / 5;
			int inGate = s % 5;
			float gx = sx + gate * 120;
			float gy = sy;
			// A sixth gate goes on a second row
			if (gate >= 3)
			{
				gx = sx + (gate - 3) * 120;
				gy = sy + 70;
			}

			if (inGate < 4)
				Pencil(m_aFront, gx + inGate * 20, gy + 54, gx + inGate * 20 + 3, gy, 1.5, s, white, 5);
			else
				Pencil(m_aFront, gx - 10, gy + 44, gx + 72, gy + 8, 1.5, s, white, 5);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A pencil sketch of the district: three roads, two villages as squares, and the one they went
	//! to written in at the end of the main road and ringed twice.
	protected void DrawMap(array<string> f)
	{
		string place = RZ_UIText.Str(f, 1);
		Color pencil = Color.FromRGBA(58, 58, 64, 235);
		int lead = pencil.PackToInt();
		// The main road to the place, a crossing road, a lane off it, and on past the place
		array<float> mainRoad = {30, 320, 210, 262, 400, 206, 560, 150, 640, 128};
		array<float> crossRoad = {120, 30, 190, 150, 210, 262, 250, 370};
		array<float> lane = {400, 206, 470, 300, 530, 372};
		array<float> onward = {640, 128, 790, 70};
		PencilPath(m_aFront, mainRoad, 1.2, 1, lead, 3);
		PencilPath(m_aFront, crossRoad, 1.2, 2, lead, 3);
		PencilPath(m_aFront, lane, 1.2, 3, lead, 3);
		PencilPath(m_aFront, onward, 1.2, 4, lead, 3);
		Rect(m_aFront, 182, 142, 198, 158, lead);
		Rect(m_aFront, 462, 292, 478, 308, lead);
		Rect(m_aFront, 632, 120, 648, 136, lead);
		if (place.IsEmpty())
			return;

		Label(place, 530, 70, 220, 32, pencil);
		Ring(m_aFront, 640, 96, 132, 50, 11, lead, 3);
		Ring(m_aFront, 642, 98, 120, 44, 23, lead, 2.5);
	}

	//------------------------------------------------------------------------------------------------
	//! A word on the page: x, y, width and height in the figure's units.
	protected void Label(string text, float x, float y, float width, float height, Color c)
	{
		Widget word = RZ_UIW.Create(LABEL_LAYOUT, m_wFrame);
		if (!word)
			return;

		RZ_UIW.SetPos(word, x, y);
		RZ_UIW.SetSize(word, width, height);
		RZ_UIW.SetText(word, "LabelText", text);
		RZ_UIW.Tint(word, "LabelText", c);
	}

	//------------------------------------------------------------------------------------------------
	//! A line through `pts` (x, y pairs in the figure's units), `width` units wide.
	protected void Stroke(array<ref CanvasWidgetCommand> into, array<float> pts, int color, float width)
	{
		LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = RZ_UIW.Native(width);
		array<float> v = {};
		foreach (float p : pts)
		{
			v.Insert(RZ_UIW.Native(p));
		}

		line.m_Vertices = v;
		into.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	//! A straight line from (x0, y0) to (x1, y1), in the figure's units.
	protected void StraightLine(array<ref CanvasWidgetCommand> into, float x0, float y0, float x1, float y1, int color, float width)
	{
		array<float> pts = {};
		pts.Insert(x0);
		pts.Insert(y0);
		pts.Insert(x1);
		pts.Insert(y1);
		Stroke(into, pts, color, width);
	}

	//------------------------------------------------------------------------------------------------
	//! A rectangle's outline, corners (x0, y0) and (x1, y1).
	protected void Box(array<ref CanvasWidgetCommand> into, float x0, float y0, float x1, float y1, int color, float width)
	{
		LineDrawCommand line = new LineDrawCommand();
		line.m_iColor = color;
		line.m_fWidth = RZ_UIW.Native(width);
		line.m_Vertices = {RZ_UIW.Native(x0), RZ_UIW.Native(y0), RZ_UIW.Native(x1), RZ_UIW.Native(y0), RZ_UIW.Native(x1), RZ_UIW.Native(y1), RZ_UIW.Native(x0), RZ_UIW.Native(y1)};
		line.m_bShouldEnclose = true;
		into.Insert(line);
	}

	//------------------------------------------------------------------------------------------------
	//! A filled rectangle, corners (x0, y0) and (x1, y1).
	protected void Rect(array<ref CanvasWidgetCommand> into, float x0, float y0, float x1, float y1, int color)
	{
		PolygonDrawCommand poly = new PolygonDrawCommand();
		poly.m_iColor = color;
		poly.m_Vertices = {RZ_UIW.Native(x0), RZ_UIW.Native(y0), RZ_UIW.Native(x1), RZ_UIW.Native(y0), RZ_UIW.Native(x1), RZ_UIW.Native(y1), RZ_UIW.Native(x0), RZ_UIW.Native(y1)};
		into.Insert(poly);
	}

	//------------------------------------------------------------------------------------------------
	//! A hand-drawn stroke from (x0, y0) to (x1, y1): six pieces, each point off the straight line by
	//! up to `shake` units; `n` picks the wobble.
	protected void Pencil(array<ref CanvasWidgetCommand> into, float x0, float y0, float x1, float y1, float shake, int n, int color, float width)
	{
		array<float> pts = {};
		for (int k = 0; k <= 6; k++)
		{
			float t = k / 6.0;
			pts.Insert(x0 + (x1 - x0) * t + Noise(n * 31 + k) * shake);
			pts.Insert(y0 + (y1 - y0) * t + Noise(n * 37 + k + 101) * shake);
		}

		Stroke(into, pts, color, width);
	}

	//------------------------------------------------------------------------------------------------
	//! A hand-drawn line through the points of a path, each piece wobbling as a Pencil stroke does.
	protected void PencilPath(array<ref CanvasWidgetCommand> into, array<float> path, float shake, int n, int color, float width)
	{
		array<float> pts = {};
		int count = path.Count() / 2;
		for (int i = 0; i < count - 1; i++)
		{
			float ax = path[i * 2];
			float ay = path[i * 2 + 1];
			float bx = path[i * 2 + 2];
			float by = path[i * 2 + 3];
			for (int k = 0; k < 4; k++)
			{
				float t = k / 4.0;
				pts.Insert(ax + (bx - ax) * t + Noise(n * 53 + i * 7 + k) * shake);
				pts.Insert(ay + (by - ay) * t + Noise(n * 59 + i * 11 + k + 37) * shake);
			}
		}

		if (count > 0)
		{
			pts.Insert(path[count * 2 - 2]);
			pts.Insert(path[count * 2 - 1]);
		}

		Stroke(into, pts, color, width);
	}

	//------------------------------------------------------------------------------------------------
	//! A ring drawn by hand round (cx, cy): a little more than once round, not quite closing on itself,
	//! a little wider as it goes; radii in the figure's units.
	protected void Ring(array<ref CanvasWidgetCommand> into, float cx, float cy, float rx, float ry, int n, int color, float width)
	{
		array<float> pts = {};
		float start = Noise(n * 13) * Math.PI;
		int steps = 32;
		for (int k = 0; k <= steps; k++)
		{
			float a = start + 1.12 * 2 * Math.PI * k / steps;
			float grow = 1 + 0.08 * k / steps + Noise(n * 17 + k) * 0.04;
			pts.Insert(cx + Math.Cos(a) * rx * grow);
			pts.Insert(cy + Math.Sin(a) * ry * grow);
		}

		Stroke(into, pts, color, width);
	}

	//------------------------------------------------------------------------------------------------
	//! A steady wobble from -1 to 1 for `i` (the page's seed mixed in), the same every time.
	protected float Noise(int i)
	{
		int h = (m_iSeed * 31 + i * 7919 + i * i * 13) % 9973;
		if (h < 0)
			h = -h;

		int spread = h % 2001;
		return spread / 1000.0 - 1;
	}
}

//------------------------------------------------------------------------------------------------
//! The reader's screen (client). Opened by a PAYLOAD_READER; an empty one from the server closes
//! it. LB/RB or Q/E (or the arrows) turn the page, B or Esc closes.
class RZ_ReaderMenu : RZ_MenuBase
{
	protected static RZ_ReaderMenu s_Open;

	protected ref array<string> m_aPages = {};
	protected int m_iPage;
	protected int m_iParsed = -1;
	protected bool m_bServerClosed;
	protected bool m_bFigure;					// this paper has a drawn figure on its first page
	protected ref RZ_PaperFigureDraw m_Figure;

	// PaperText's place on the paper (RZ_ReaderMenu.layout), and under a figure (PaperFigure: 150 to 530)
	protected static const float TEXT_X = 72;
	protected static const float TEXT_Y = 160;
	protected static const float TEXT_W = 816;
	protected static const float TEXT_BOTTOM = 680;
	protected static const float FIGURE_TEXT_Y = 544;

	//------------------------------------------------------------------------------------------------
	static bool IsShowing()
	{
		return s_Open != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The server closed it (danger): no need to tell the server back.
	static void CloseFromServer()
	{
		if (!s_Open)
			return;

		s_Open.m_bServerClosed = true;
		s_Open.Close();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		GetGame().GetInputManager().AddActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().AddActionListener("MenuTabRight", EActionTrigger.DOWN, OnNext);
		GetGame().GetInputManager().AddActionListener("MenuLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().AddActionListener("MenuRight", EActionTrigger.DOWN, OnNext);
		RZ_UIW.OnClick(m_wRoot, "PrevButton", m_Click);
		RZ_UIW.OnClick(m_wRoot, "NextButton", m_Click);
		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabLeft", "");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuTabRight", "Pages") && prompts;
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		RZ_UIW.Focus(m_wRoot, "CloseButton");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		GetGame().GetInputManager().RemoveActionListener("MenuTabLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().RemoveActionListener("MenuTabRight", EActionTrigger.DOWN, OnNext);
		GetGame().GetInputManager().RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnPrev);
		GetGame().GetInputManager().RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnNext);
		if (s_Open == this)
			s_Open = null;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bServerClosed)
			ui.SendScreenClosed(RZ_ScreenGuard.KIND_READER);

		super.OnMenuClose();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		if (name == "PrevButton")
			OnPrev();
		else if (name == "NextButton")
			OnNext();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrev(float value = 0, EActionTrigger reason = 0)
	{
		m_iPage = Math.Max(0, m_iPage - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNext(float value = 0, EActionTrigger reason = 0)
	{
		m_iPage = Math.Min(Math.Max(0, m_aPages.Count() - 1), m_iPage + 1);
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return m_iPage;

		return ui.GetReaderVersion() * 100 + m_iPage;
	}

	//------------------------------------------------------------------------------------------------
	//! P|title|how|figure (RZ_PaperFigure, "" none), then the text (docs/UI.md, "The reader").
	override protected void Redraw()
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (!ui)
			return;

		string payload = ui.GetReader();
		int nl = payload.IndexOf("\n");
		string head = payload;
		string text = "";
		if (nl >= 0)
		{
			head = payload.Substring(0, nl);
			text = payload.Substring(nl + 1, payload.Length() - nl - 1);
		}

		array<string> f = {};
		RZ_UIText.Split(head, RZ_UIText.SEP, f);
		if (m_iParsed != ui.GetReaderVersion())
		{
			m_iParsed = ui.GetReaderVersion();
			RZ_UIText.Paginate(text, RZ_UISettings.Get().m_iReaderPageChars, m_aPages);
			m_iPage = 0;
			// A drawn page (P|title|how|figure): the figure is page one, with the words under it when
			// they're short; longer words follow on their own pages
			m_bFigure = false;
			string spec = RZ_UIText.Str(f, 3);
			if (!spec.IsEmpty())
			{
				if (!m_Figure)
					m_Figure = new RZ_PaperFigureDraw();

				m_bFigure = m_Figure.Draw(RZ_UIW.Find(m_wRoot, "PaperFigure"), spec);
			}
			else if (m_Figure)
			{
				m_Figure.Clear();
			}

			if (m_bFigure)
			{
				string words = text.Trim();
				array<string> drawn = {};
				if (words.Length() <= RZ_PaperFigure.CAPTION_CHARS)
				{
					drawn.Insert(words);
				}
				else
				{
					drawn.Insert("");
					foreach (string later : m_aPages)
					{
						drawn.Insert(later);
					}
				}

				m_aPages = drawn;
			}
		}

		int count = Math.Max(1, m_aPages.Count());
		m_iPage = Math.Clamp(m_iPage, 0, count - 1);
		string how = RZ_UIText.Str(f, 2);
		RZ_UIW.SetText(m_wRoot, "PaperTitle", RZ_UIText.Str(f, 1));
		RZ_UIW.SetText(m_wRoot, "PaperHow", how);
		RZ_UIW.Show(m_wRoot, "PaperHow", !how.IsEmpty());
		string page = "";
		if (!m_aPages.IsEmpty())
			page = m_aPages[m_iPage];

		// The figure's page: the picture, and the text under it; other pages: the text alone
		bool figurePage = m_bFigure && m_iPage == 0;
		RZ_UIW.Show(m_wRoot, "PaperFigure", figurePage);
		Widget paperText = RZ_UIW.Find(m_wRoot, "PaperText");
		if (figurePage)
		{
			RZ_UIW.SetPos(paperText, TEXT_X, FIGURE_TEXT_Y);
			RZ_UIW.SetSize(paperText, TEXT_W, TEXT_BOTTOM - FIGURE_TEXT_Y);
		}
		else
		{
			RZ_UIW.SetPos(paperText, TEXT_X, TEXT_Y);
			RZ_UIW.SetSize(paperText, TEXT_W, TEXT_BOTTOM - TEXT_Y);
		}

		RZ_UIW.SetText(m_wRoot, "PaperText", page);
		RZ_UIW.SetText(m_wRoot, "PaperPages", string.Format("%1 / %2", m_iPage + 1, count));
		RZ_UIW.Show(m_wRoot, "PrevButton", m_iPage > 0);
		RZ_UIW.Show(m_wRoot, "NextButton", m_iPage < count - 1);
	}
}
