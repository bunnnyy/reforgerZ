//! The hands-on view, client side (docs/UI.md, "The hands-on view"; the server: Interactions/
//! RZ_HandsOn.c). The camera comes in on the thing in the world; its places to look glow on it, each
//! a button the script keeps over its place; the panel on the right holds what you've found and the
//! game's buttons. Picking a place eases the camera onto it, a short hold fills under its label, then
//! the server says what was seen: it shows at the place, with a flash for a bad find (red) or an odd
//! one (amber). A place already seen shows again at once, for nothing.
//!
//! Pad: D-pad or left stick between the places, A looks, X / Y / R3 the panel's buttons, B closes.
//! Keys: arrows, Enter, R / F / V, Esc. Mouse: click a place or a button; the close X.
class RZ_HandsOnMenu : RZ_MenuBase
{
	protected static const int SPOTS = 6;
	protected static const int LOGS = 10;
	protected static const int ACTS = 3;
	protected static const float HOLD = 1.2;		// a new look takes this long
	protected static const float RELOOK = 0.25;		// one seen already
	protected static const float SEEN_FOR = 3.5;	// what was found stays at the place this long
	protected static const float EASE = 4;			// the camera's ease (per second)

	protected static RZ_HandsOnMenu s_Open;
	protected static string s_sData;
	protected static int s_iVersion;

	protected bool m_bDone;
	protected int m_iParsed = -1;
	protected ref array<string> m_aHead = {};
	protected string m_sText;
	protected ref array<ref array<string>> m_aSpots = {};
	protected ref array<ref array<string>> m_aLogs = {};
	protected ref array<ref array<string>> m_aActs = {};
	protected int m_iSerialSeen;
	protected int m_iSelected;
	protected int m_iHolding = -1;				// the place being looked at now (-1 none)
	protected float m_fHeld;
	protected int m_iFocus = -1;				// the place the camera is on (-1 the whole thing)
	protected float m_fFocusUntil;
	protected int m_iSeenSpot = -1;
	protected float m_fSeenAt = -100;
	protected float m_fFlashAt = -100;
	protected Widget m_wLit;

	protected CameraBase m_Camera;
	protected CameraBase m_PlayerCamera;
	protected vector m_vEye;
	protected vector m_vAt;
	protected bool m_bPlaced;

	//------------------------------------------------------------------------------------------------
	static void OnPayload(RZ_PlayerUIComponent ui, string payload)
	{
		if (payload.StartsWith("X" + RZ_UIText.SEP))
		{
			if (s_Open)
			{
				s_Open.m_bDone = true;
				s_Open.Close();
			}

			return;
		}

		s_sData = payload;
		s_iVersion++;
		if (s_Open || RZ_UIW.OpenMenu(ChimeraMenuPreset.RZ_HandsOnMenu))
			return;

		// No preset or layout here: the game's prompts for this player
		ui.SendHandsAct("failed", "");
	}

	//------------------------------------------------------------------------------------------------
	override protected int DataVersion()
	{
		return s_iVersion * 16 + m_iSelected;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnOpened()
	{
		s_Open = this;
		InputManager input = GetGame().GetInputManager();
		input.AddActionListener("MenuUp", EActionTrigger.DOWN, OnPrev);
		input.AddActionListener("MenuLeft", EActionTrigger.DOWN, OnPrev);
		input.AddActionListener("MenuDown", EActionTrigger.DOWN, OnNext);
		input.AddActionListener("MenuRight", EActionTrigger.DOWN, OnNext);
		input.AddActionListener("MenuSelect", EActionTrigger.DOWN, OnLook);
		input.AddActionListener("MenuRefresh", EActionTrigger.DOWN, OnAct0);
		input.AddActionListener("MenuFavourite", EActionTrigger.DOWN, OnAct1);
		input.AddActionListener("MenuDelete", EActionTrigger.DOWN, OnAct2);
		for (int i = 0; i < SPOTS; i++)
		{
			RZ_UIW.OnClick(m_wRoot, "Spot" + i.ToString(), m_Click);
			RZ_UIW.SetTag(RZ_UIW.Find(m_wRoot, "Spot" + i.ToString()), i);
			RZ_UIW.Icon(m_wRoot, "SpotRing" + i.ToString(), "marker_disc");
		}

		for (int a = 0; a < ACTS; a++)
		{
			RZ_UIW.OnClick(m_wRoot, "Act" + a.ToString(), m_Click);
			RZ_UIW.SetTag(RZ_UIW.Find(m_wRoot, "Act" + a.ToString()), a);
		}

		bool prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuSelect", "Look");
		prompts = RZ_UIW.AddPrompt(m_wRoot, "Prompts", "MenuBack", "Close") && prompts;
		RZ_UIW.Show(m_wRoot, "PromptsText", !prompts);
		StartCamera();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		InputManager input = GetGame().GetInputManager();
		input.RemoveActionListener("MenuUp", EActionTrigger.DOWN, OnPrev);
		input.RemoveActionListener("MenuLeft", EActionTrigger.DOWN, OnPrev);
		input.RemoveActionListener("MenuDown", EActionTrigger.DOWN, OnNext);
		input.RemoveActionListener("MenuRight", EActionTrigger.DOWN, OnNext);
		input.RemoveActionListener("MenuSelect", EActionTrigger.DOWN, OnLook);
		input.RemoveActionListener("MenuRefresh", EActionTrigger.DOWN, OnAct0);
		input.RemoveActionListener("MenuFavourite", EActionTrigger.DOWN, OnAct1);
		input.RemoveActionListener("MenuDelete", EActionTrigger.DOWN, OnAct2);
		StopCamera();
		if (s_Open == this)
			s_Open = null;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui && !m_bDone)
			ui.SendHandsAct("closed", "");

		m_bDone = true;
		super.OnMenuClose();
	}

	// ================================================================ The data
	//------------------------------------------------------------------------------------------------
	protected void Parse()
	{
		array<ref array<string>> recs = {};
		RZ_MenuBase.Records(s_sData, recs);
		m_aHead.Clear();
		m_aSpots.Clear();
		m_aLogs.Clear();
		m_aActs.Clear();
		m_sText = "";
		foreach (array<string> f : recs)
		{
			string tag = RZ_UIText.Str(f, 0);
			if (tag == "H")
				m_aHead = f;
			else if (tag == "T")
				m_sText = RZ_UIText.Str(f, 1);
			else if (tag == "P")
				m_aSpots.Insert(f);
			else if (tag == "L")
				m_aLogs.Insert(f);
			else if (tag == "A")
				m_aActs.Insert(f);
			else if (tag == "E")
				OnSeen(f);
		}

		m_sText.Replace("^", "\n");
		if (m_iSelected >= m_aSpots.Count())
			m_iSelected = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! A look's result, once: it shows at its place, the camera stays there a moment, a flash for
	//! what's wrong.
	protected void OnSeen(array<string> f)
	{
		int serial = RZ_UIText.Int(f, 4);
		if (serial <= m_iSerialSeen)
			return;

		m_iSerialSeen = serial;
		m_iSeenSpot = RZ_UIText.Int(f, 1);
		m_fSeenAt = RZ_UIW.Now();
		m_iFocus = m_iSeenSpot;
		m_fFocusUntil = RZ_UIW.Now() + SEEN_FOR;
		int tone = RZ_UIText.Int(f, 3);
		RZ_UIW.SetText(m_wRoot, "SeenText", RZ_UIText.Str(f, 2));
		RZ_UIW.Tint(m_wRoot, "SeenBar", ToneColour(tone));
		if (tone == RZ_HandsOn.TONE_BAD || tone == RZ_HandsOn.TONE_ODD)
		{
			m_fFlashAt = RZ_UIW.Now();
			Color flash = RZ_SiteBuilderMenu.Lin(200, 40, 30);
			if (tone == RZ_HandsOn.TONE_ODD)
				flash = RZ_SiteBuilderMenu.Lin(232, 178, 58);

			RZ_UIW.Tint(m_wRoot, "Flash", flash);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static Color ToneColour(int tone)
	{
		switch (tone)
		{
			case RZ_HandsOn.TONE_CLEAN: return RZ_SiteBuilderMenu.Lin(151, 196, 89);
			case RZ_HandsOn.TONE_ODD: return RZ_SiteBuilderMenu.Lin(232, 178, 58);
			case RZ_HandsOn.TONE_BAD: return RZ_SiteBuilderMenu.Lin(240, 110, 100);
		}

		return RZ_SiteBuilderMenu.Lin(190, 186, 176);
	}

	// ================================================================ Drawing
	//------------------------------------------------------------------------------------------------
	override protected void Redraw()
	{
		if (m_iParsed != s_iVersion)
		{
			m_iParsed = s_iVersion;
			Parse();
		}

		RZ_UIW.SetText(m_wRoot, "Title", RZ_UIText.Str(m_aHead, 1));
		RZ_UIW.SetText(m_wRoot, "Status", RZ_UIText.Str(m_aHead, 2));
		RZ_UIW.SetText(m_wRoot, "Text", m_sText);
		for (int l = 0; l < LOGS; l++)
		{
			string li = l.ToString();
			bool there = l < m_aLogs.Count();
			RZ_UIW.Show(m_wRoot, "Log" + li, there);
			RZ_UIW.Show(m_wRoot, "LogDot" + li, there);
			if (!there)
				continue;

			RZ_UIW.SetText(m_wRoot, "Log" + li, RZ_UIText.Str(m_aLogs[l], 1));
			RZ_UIW.Tint(m_wRoot, "LogDot" + li, ToneColour(RZ_UIText.Int(m_aLogs[l], 2)));
		}

		RZ_UIW.Show(m_wRoot, "FoundHead", !m_aLogs.IsEmpty());
		RZ_UIW.Show(m_wRoot, "FoundRule", !m_aLogs.IsEmpty());
		array<string> keys = {"X", "Y", "R3"};
		for (int a = 0; a < ACTS; a++)
		{
			string ai = a.ToString();
			bool shown = a < m_aActs.Count();
			RZ_UIW.Show(m_wRoot, "Act" + ai, shown);
			if (!shown)
				continue;

			RZ_UIW.SetText(m_wRoot, "ActKey" + ai, keys[a]);
			RZ_UIW.SetText(m_wRoot, "ActText" + ai, RZ_UIText.Str(m_aActs[a], 2));
			Color fill = RZ_SiteBuilderMenu.Lin(36, 40, 44);
			if (m_wLit && m_wLit.GetName() == "Act" + ai)
				fill = RZ_SiteBuilderMenu.Lin(70, 78, 86);

			RZ_UIW.Tint(m_wRoot, "ActBg" + ai, fill);
		}

		Color closeFill = RZ_SiteBuilderMenu.Lin(27, 31, 34);
		if (m_wLit && m_wLit.GetName() == "CloseButton")
			closeFill = RZ_SiteBuilderMenu.Lin(70, 78, 86);

		RZ_UIW.Tint(m_wRoot, "CloseBg", closeFill);
		DrawSpots();
	}

	//------------------------------------------------------------------------------------------------
	//! Each place's look: open amber, seen in what it showed, shut grey; the picked one lit.
	protected void DrawSpots()
	{
		for (int i = 0; i < SPOTS; i++)
		{
			string idx = i.ToString();
			if (i >= m_aSpots.Count())
			{
				RZ_UIW.Show(m_wRoot, "Spot" + idx, false);
				continue;
			}

			array<string> s = m_aSpots[i];
			int state = RZ_UIText.Int(s, 7);
			string label = RZ_UIText.Str(s, 2);
			Color ring = RZ_SiteBuilderMenu.Lin(232, 178, 58);
			Color ink = RZ_SiteBuilderMenu.Lin(236, 233, 225);
			if (state == RZ_HandsOn.SPOT_SEEN)
			{
				ring = ToneColour(RZ_UIText.Int(s, 8));
				label = RZ_UIText.Str(s, 9);
				if (label.Length() > 40)
					label = label.Substring(0, 37) + "...";
			}
			else if (state == RZ_HandsOn.SPOT_SHUT)
			{
				ring = RZ_SiteBuilderMenu.Lin(110, 108, 102);
				ink = RZ_SiteBuilderMenu.Lin(150, 147, 140);
			}

			RZ_UIW.Tint(m_wRoot, "SpotRing" + idx, ring);
			RZ_UIW.SetText(m_wRoot, "SpotKey" + idx, (i + 1).ToString());
			RZ_UIW.SetText(m_wRoot, "SpotLabel" + idx, label);
			RZ_UIW.Tint(m_wRoot, "SpotLabel" + idx, ink);
			Color bg = RZ_SiteBuilderMenu.Lin(14, 15, 17);
			bool lit = m_wLit && m_wLit.GetName() == "Spot" + idx;
			if (i == m_iSelected || lit)
				bg = RZ_SiteBuilderMenu.Lin(58, 64, 71);

			RZ_UIW.Tint(m_wRoot, "SpotLabelBg" + idx, bg);
		}
	}

	// ================================================================ Every frame
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		float now = RZ_UIW.Now();
		Hover();
		// The hold: it fills under the label, then the look goes to the server
		if (m_iHolding >= 0)
		{
			m_fHeld += tDelta;
			float need = HoldFor(m_iHolding);
			Widget fill = RZ_UIW.Find(m_wRoot, "SpotFill" + m_iHolding.ToString());
			if (fill)
			{
				fill.SetVisible(true);
				RZ_UIW.SetSize(fill, 292 * Math.Clamp(m_fHeld / need, 0, 1), 4);
			}

			if (m_fHeld >= need)
			{
				if (fill)
					fill.SetVisible(false);

				RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
				if (ui)
					ui.SendHandsAct("look", m_iHolding.ToString());

				m_iFocus = m_iHolding;
				m_fFocusUntil = now + SEEN_FOR;
				m_iHolding = -1;
			}
		}

		if (m_iHolding < 0 && m_iFocus >= 0 && now > m_fFocusUntil)
			m_iFocus = -1;

		MoveCamera(tDelta);
		PlaceSpots();
		// The flash fades in a second; what was found fades after its time
		Widget flashW = RZ_UIW.Find(m_wRoot, "Flash");
		RZ_UIW.Fade(flashW, 0.45 * (1 - Math.Clamp((now - m_fFlashAt) / 1.0, 0, 1)));
		Widget seenW = RZ_UIW.Find(m_wRoot, "Seen");
		float age = now - m_fSeenAt;
		float seenAlpha = 1;
		if (age > SEEN_FOR - 0.5)
			seenAlpha = Math.Clamp((SEEN_FOR - age) / 0.5, 0, 1);

		RZ_UIW.Fade(seenW, seenAlpha);
	}

	//------------------------------------------------------------------------------------------------
	protected float HoldFor(int i)
	{
		if (i < m_aSpots.Count() && RZ_UIText.Int(m_aSpots[i], 7) == RZ_HandsOn.SPOT_SEEN)
			return RELOOK;

		return HOLD;
	}

	//------------------------------------------------------------------------------------------------
	//! The button under the mouse lights (polled: enter and leave don't come through its picture).
	protected void Hover()
	{
		Widget lit = null;
		if (RZ_UIW.ByMouse())
		{
			Widget p = WidgetManager.GetWidgetUnderCursor();
			while (p && p != m_wRoot && !lit)
			{
				string n = p.GetName();
				if ((n.StartsWith("Spot") && n.Length() == 5) || (n.StartsWith("Act") && n.Length() == 4) || n == "CloseButton")
					lit = p;

				p = p.GetParent();
			}
		}

		if (lit == m_wLit)
			return;

		m_wLit = lit;
		Redraw();
	}

	// ================================================================ The thing and its places
	//------------------------------------------------------------------------------------------------
	protected IEntity Subject()
	{
		RplId id = RZ_UIText.Int(m_aHead, 7);
		if (RZ_UIText.Int(m_aHead, 7) == 0)
			return null;

		RplComponent rc = RplComponent.Cast(Replication.FindItem(id));
		if (!rc)
			return null;

		return rc.GetEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected vector Origin()
	{
		IEntity e = Subject();
		if (e)
			return e.GetOrigin();

		return Vector(RZ_UIText.Str(m_aHead, 3).ToFloat(), RZ_UIText.Str(m_aHead, 4).ToFloat(), RZ_UIText.Str(m_aHead, 5).ToFloat());
	}

	//------------------------------------------------------------------------------------------------
	//! The thing's frame: right, up, forward and its origin.
	protected void Frame(out vector right, out vector fwd, out vector origin)
	{
		IEntity e = Subject();
		if (e)
		{
			vector m[4];
			e.GetWorldTransform(m);
			right = m[0];
			fwd = m[2];
			origin = m[3];
			return;
		}

		float yaw = RZ_UIText.Str(m_aHead, 6).ToFloat() * Math.DEG2RAD;
		fwd = Vector(Math.Sin(yaw), 0, Math.Cos(yaw));
		right = Vector(Math.Cos(yaw), 0, -Math.Sin(yaw));
		origin = Origin();
	}

	//------------------------------------------------------------------------------------------------
	//! Place `i` in the world: on its bone when the thing is a person the client can see, else its
	//! offset in the thing's own frame.
	protected vector SpotPos(int i)
	{
		array<string> s = m_aSpots[i];
		vector off = Vector(RZ_UIText.Str(s, 4).ToFloat(), RZ_UIText.Str(s, 5).ToFloat(), RZ_UIText.Str(s, 6).ToFloat());
		string bone = RZ_UIText.Str(s, 3);
		IEntity e = Subject();
		// "@view": set out across the thing as you see it (x to your right, y up from its middle, z toward you)
		if (bone == "@view")
		{
			vector side = ViewSide();
			vector camRight = Vector(-side[2], 0, side[0]);
			return ViewMiddle() + camRight * off[0] + vector.Up * off[1] + side * off[2];
		}

		if (e && !bone.IsEmpty())
		{
			Animation anim = e.GetAnimation();
			if (anim)
			{
				TNodeId node = anim.GetBoneIndex(bone);
				vector bm[4];
				if (node != -1 && anim.GetBoneMatrix(node, bm)) // VERIFY: a character's bone, in its model space, by the vanilla names (Head, Neck1, LeftForeArm)
					return e.CoordToParent(bm[3]) + off;
			}
		}

		vector right;
		vector fwd;
		vector origin;
		Frame(right, fwd, origin);
		return origin + right * off[0] + vector.Up * off[1] + fwd * off[2];
	}

	//------------------------------------------------------------------------------------------------
	//! Which way you look at the thing from: from it toward you, flat.
	protected vector ViewSide()
	{
		vector right;
		vector fwd;
		vector origin;
		Frame(right, fwd, origin);
		IEntity me = SCR_PlayerController.GetLocalControlledEntity();
		if (me)
		{
			vector toMe = me.GetOrigin() - origin;
			toMe[1] = 0;
			if (toMe.Length() > 0.3)
				return toMe.Normalized();
		}

		return fwd;
	}

	//------------------------------------------------------------------------------------------------
	//! The thing's middle: its box's centre when the client sees it, else a little over its origin.
	protected vector ViewMiddle()
	{
		IEntity e = Subject();
		if (e)
		{
			vector bmin;
			vector bmax;
			e.GetWorldBounds(bmin, bmax);
			vector mid = (bmin + bmax) * 0.5;
			// A person: chest height, not their middle
			if (ChimeraCharacter.Cast(e))
				mid[1] = e.GetOrigin()[1] + 1.25;

			return mid;
		}

		return Origin() + Vector(0, 0.8, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Each place's button over its place; what was found beside its place.
	protected void PlaceSpots()
	{
		WorkspaceWidget ws = GetGame().GetWorkspace();
		BaseWorld world = GetGame().GetWorld();
		if (!ws || !world)
			return;

		for (int i = 0; i < SPOTS && i < m_aSpots.Count(); i++)
		{
			Widget w = RZ_UIW.Find(m_wRoot, "Spot" + i.ToString());
			if (!w)
				continue;

			vector sp = ws.ProjWorldToScreen(SpotPos(i), world);
			bool front = sp[2] > 0;
			w.SetVisible(front && (m_iHolding < 0 || m_iHolding == i));
			if (front)
				RZ_UIW.SetPos(w, sp[0] - 20, sp[1] - 28);

			if (i == m_iSeenSpot && front)
				RZ_UIW.SetPos(RZ_UIW.Find(m_wRoot, "Seen"), sp[0] + 40, sp[1] + 34);
		}
	}

	// ================================================================ The camera
	//------------------------------------------------------------------------------------------------
	protected void StartCamera()
	{
		CameraManager cameras = GetGame().GetCameraManager();
		if (!cameras)
			return;

		m_PlayerCamera = cameras.CurrentCamera();
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.MatrixIdentity4(params.Transform);
		params.Transform[3] = Origin() + Vector(0, 2, 0);
		m_Camera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase, GetGame().GetWorld(), params));
		if (!m_Camera)
			return;

		m_Camera.SetVerticalFOV(50);
		m_Camera.SetNearPlane(0.05); // VERIFY: CameraBase.SetNearPlane, so a close-up isn't clipped
		m_Camera.SetFarPlane(1500);
		m_bPlaced = false;
		MoveCamera(0);
		cameras.SetCamera(m_Camera);
	}

	//------------------------------------------------------------------------------------------------
	protected void StopCamera()
	{
		CameraManager cameras = GetGame().GetCameraManager();
		if (cameras && m_PlayerCamera)
			cameras.SetCamera(m_PlayerCamera);

		if (m_Camera)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Camera);

		m_Camera = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Where the camera wants to be: the whole thing from your side, a little left of the panel; or
	//! close on the place being looked at.
	protected void Want(out vector eye, out vector at)
	{
		vector right;
		vector fwd;
		vector origin;
		Frame(right, fwd, origin);
		// From the side you're standing on
		vector side = ViewSide();

		float dist = 1.9;
		float high = 1.55;
		at = origin + Vector(0, 1.3, 0);
		switch (RZ_UIText.Int(m_aHead, 8))
		{
			case RZ_HandsOn.FRAME_LYING:
			{
				dist = 1.3;
				high = 1.8;
				at = origin + Vector(0, 0.3, 0);
				break;
			}
			case RZ_HandsOn.FRAME_SMALL:
			{
				dist = 0.9;
				high = 1.5;
				at = origin + Vector(0, 0.9, 0);
				break;
			}
			case RZ_HandsOn.FRAME_THING:
			{
				dist = 1.7;
				high = 1.6;
				at = origin + Vector(0, 0.5, 0);
				break;
			}
		}

		IEntity e = Subject();
		if (e && RZ_UIText.Int(m_aHead, 8) >= RZ_HandsOn.FRAME_SMALL)
		{
			vector bmin;
			vector bmax;
			e.GetWorldBounds(bmin, bmax);
			at = (bmin + bmax) * 0.5;
			float size = Math.Max(Math.Max(bmax[0] - bmin[0], bmax[2] - bmin[2]), bmax[1] - bmin[1]);
			dist = Math.Clamp(size * 1.8, 0.7, 3);
			high = at[1] - origin[1] + Math.Clamp(size, 0.4, 1.5);
		}

		eye = origin + side * dist + Vector(0, high, 0);
		// The panel covers the right: the thing sits left of the middle
		vector look = at - eye;
		look[1] = 0;
		if (look.Length() > 0.01)
			look.Normalize();

		vector camRight = Vector(look[2], 0, -look[0]);
		at = at + camRight * (dist * 0.28);
		int focus = m_iFocus;
		if (m_iHolding >= 0)
			focus = m_iHolding;

		if (focus >= 0 && focus < m_aSpots.Count())
		{
			vector spot = SpotPos(focus);
			vector back = (eye - spot).Normalized();
			eye = spot + back * 0.75;
			at = spot + camRight * 0.2;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void MoveCamera(float dt)
	{
		if (!m_Camera || m_aHead.IsEmpty())
			return;

		vector eye;
		vector at;
		Want(eye, at);
		if (!m_bPlaced)
		{
			m_vEye = eye;
			m_vAt = at;
			m_bPlaced = true;
		}
		else
		{
			float k = Math.Clamp(dt * EASE, 0, 1);
			m_vEye = m_vEye + (eye - m_vEye) * k;
			m_vAt = m_vAt + (at - m_vAt) * k;
		}

		vector dir = (m_vAt - m_vEye).Normalized();
		vector mat[4];
		Math3D.DirectionAndUpMatrix(dir, vector.Up, mat);
		mat[3] = m_vEye;
		m_Camera.SetWorldTransform(mat);
	}

	// ================================================================ Input
	//------------------------------------------------------------------------------------------------
	protected void OnPrev(float value = 0, EActionTrigger reason = 0)
	{
		if (m_aSpots.IsEmpty() || m_iHolding >= 0)
			return;

		m_iSelected = (m_iSelected + m_aSpots.Count() - 1) % m_aSpots.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNext(float value = 0, EActionTrigger reason = 0)
	{
		if (m_aSpots.IsEmpty() || m_iHolding >= 0)
			return;

		m_iSelected = (m_iSelected + 1) % m_aSpots.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLook(float value = 0, EActionTrigger reason = 0)
	{
		if (RZ_UIW.ByMouse())
			return;	// a click on a place does it

		Look(m_iSelected);
	}

	//------------------------------------------------------------------------------------------------
	protected void Look(int i)
	{
		if (m_iHolding >= 0 || i < 0 || i >= m_aSpots.Count())
			return;

		if (RZ_UIText.Int(m_aSpots[i], 7) == RZ_HandsOn.SPOT_SHUT)
			return;

		m_iSelected = i;
		m_iHolding = i;
		m_fHeld = 0;
		m_iFocus = i;
		m_fSeenAt = -100;
		DrawSpots();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAct0(float value = 0, EActionTrigger reason = 0) { Act(0); }
	protected void OnAct1(float value = 0, EActionTrigger reason = 0) { Act(1); }
	protected void OnAct2(float value = 0, EActionTrigger reason = 0) { Act(2); }

	//------------------------------------------------------------------------------------------------
	protected void Act(int i)
	{
		if (i >= m_aActs.Count() || m_iHolding >= 0)
			return;

		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetLocal();
		if (ui)
			ui.SendHandsAct("act", i.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override protected bool OnHoverRow(Widget w, string name, int tag, bool on)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnClickOther(Widget w, string name)
	{
		int tag = RZ_UIW.GetTag(w);
		if (name.StartsWith("Spot"))
			Look(tag);
		else if (name.StartsWith("Act"))
			Act(tag);
	}
}
