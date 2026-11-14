//! A sorting duty's subject up close (docs/ERRANDS.md, "The sorting duties"; the view: Interactions/
//! RZ_HandsOn.c). Its one prompt opens the camera on it: the person in the line, the body on its
//! board, the tray on the bench, the crate in the store. Its three places to look sit where they are
//! on it (a person's forearm, eyes and brow on their bones; the rest on its real box), each a look
//! (a short hold) out of the duty's few, and one already seen shows again for nothing. The panel keeps
//! everything said about it (the tell as you walked up, every look, running out of looks) and its one
//! button takes it up: the prompt then waits for the hold, and you carry it to its place as before.
//! Server only.
class RZ_DutyHandsOn : RZ_HandsOnGame
{
	protected RZ_DutyGames m_Owner;
	protected RZ_DutyGame m_Game;
	protected int m_iIndex;

	//------------------------------------------------------------------------------------------------
	void RZ_DutyHandsOn(RZ_DutyGames owner, RZ_DutyGame game, int index)
	{
		m_Owner = owner;
		m_Game = game;
		m_iIndex = index;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DutySubject Subj()
	{
		if (!m_Game || m_iIndex < 0 || m_iIndex >= m_Game.m_aSubjects.Count())
			return null;

		return m_Game.m_aSubjects[m_iIndex];
	}

	//------------------------------------------------------------------------------------------------
	override IEntity Subject()
	{
		if (!m_Owner || !m_Game)
			return null;

		return m_Owner.SubjectEntity(m_Game, m_iIndex);
	}

	//------------------------------------------------------------------------------------------------
	override vector Where()
	{
		RZ_DutySubject s = Subj();
		if (!s)
			return vector.Zero;

		return s.m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	override float Facing()
	{
		RZ_DutySubject s = Subj();
		if (!s)
			return 0;

		return s.m_fYaw;
	}

	//------------------------------------------------------------------------------------------------
	override int Frame()
	{
		switch (m_Game.m_iKind)
		{
			case RZ_DutyGame.KIND_LINE: return RZ_HandsOn.FRAME_PERSON;
			case RZ_DutyGame.KIND_DEAD: return RZ_HandsOn.FRAME_LYING;
			case RZ_DutyGame.KIND_BENCH: return RZ_HandsOn.FRAME_SMALL;
		}

		return RZ_HandsOn.FRAME_THING;
	}

	//------------------------------------------------------------------------------------------------
	override string Title()
	{
		RZ_DutySubject s = Subj();
		if (!s)
			return "";

		return s.m_sTitle;
	}

	//------------------------------------------------------------------------------------------------
	override string Status()
	{
		if (m_Game.m_iLooks <= 0)
			return "No looks left: sort on what you know";

		if (m_Game.m_iLooks == 1)
			return "1 look left for the whole row";

		return string.Format("%1 looks left for the whole row", m_Game.m_iLooks);
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		RZ_DutySubject s = Subj();
		if (!s)
			return "";

		return string.Format("%1.\n\n%2", s.m_sTell, RZ_DutyGame.Rules(m_Game.m_iKind));
	}

	//------------------------------------------------------------------------------------------------
	override void Spots(notnull array<ref RZ_HandsOnSpot> spots)
	{
		RZ_DutySubject s = Subj();
		if (!s)
			return;

		for (int sign = 0; sign < 3; sign++)
		{
			RZ_HandsOnSpot sp = new RZ_HandsOnSpot();
			sp.m_sLabel = RZ_DutyGame.SignPrompt(m_Game.m_iKind, sign);
			Anchor(sign, sp);
			int look = m_Game.SignLook(s, sign);
			if (look != RZ_DutyGame.SIGN_HIDDEN)
			{
				sp.m_iState = RZ_HandsOn.SPOT_SEEN;
				sp.m_sSeen = RZ_DutyGame.SignSeen(m_Game.m_iKind, sign, look);
				sp.m_iTone = RZ_HandsOn.TONE_CLEAN;
				if (look == RZ_DutyGame.SIGN_BAD)
					sp.m_iTone = RZ_HandsOn.TONE_BAD;
				else if (look == RZ_DutyGame.SIGN_ODD)
					sp.m_iTone = RZ_HandsOn.TONE_ODD;
			}
			else if (m_Game.m_iLooks <= 0 || s.m_bTaken)
			{
				sp.m_iState = RZ_HandsOn.SPOT_SHUT;
			}

			spots.Insert(sp);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Where sign `sign` sits on the subject. A person: on their bones (left forearm, between the eyes,
	//! the brow), with offsets for a figure the client can't see. The rest: on its real box, read from
	//! its entity (the long side is its length).
	protected void Anchor(int sign, RZ_HandsOnSpot sp)
	{
		if (m_Game.m_iKind == RZ_DutyGame.KIND_LINE)
		{
			if (sign == 0)
			{
				sp.m_sBone = "LeftForeArm";
				sp.m_vOffset = Vector(-0.28, 1.05, 0.1);
			}
			else if (sign == 1)
			{
				sp.m_sBone = "Head";
				sp.m_vOffset = Vector(0, 1.62, 0.1);
			}
			else
			{
				sp.m_sBone = "Head";
				sp.m_vOffset = Vector(0, 1.74, 0.08);
			}

			return;
		}

		// The box in its own frame: half-sizes along its length and across, and its top
		vector bmin = Vector(-0.4, 0, -0.9);
		vector bmax = Vector(0.4, 0.4, 0.9);
		IEntity e = Subject();
		if (e)
			e.GetBounds(bmin, bmax);

		vector mid = (bmin + bmax) * 0.5;
		float top = bmax[1];
		bool alongZ = (bmax[2] - bmin[2]) >= (bmax[0] - bmin[0]);
		float halfL = (bmax[2] - bmin[2]) * 0.5;
		float halfW = (bmax[0] - bmin[0]) * 0.5;
		if (!alongZ)
		{
			halfL = (bmax[0] - bmin[0]) * 0.5;
			halfW = (bmax[2] - bmin[2]) * 0.5;
		}

		// t along the length (-1 one end, 1 the other), w across (-1 to 1), h up from the base (0..1)
		float t = 0;
		float w = 0;
		float h = 1;
		switch (m_Game.m_iKind)
		{
			case RZ_DutyGame.KIND_DEAD:
			{
				// The arms at the side, the neck at the head end, the tag at the feet
				if (sign == 0) { t = 0.15; w = 0.7; h = 0.8; }
				else if (sign == 1) { t = 0.7; w = 0; h = 0.9; }
				else { t = -0.85; w = 0; h = 0.7; }
				break;
			}
			case RZ_DutyGame.KIND_BENCH:
			{
				// The seal on the lid, the glass held to the lamp, the label on its face
				if (sign == 0) { t = 0; w = 0; h = 1; }
				else if (sign == 1) { t = 0.6; w = 0.3; h = 0.85; }
				else { t = -0.4; w = 1; h = 0.5; }
				break;
			}
			default:
			{
				// A crate: what's in it from the top, the seal on its lid's edge, the chit on its side
				if (sign == 0) { t = 0; w = 0; h = 1; }
				else if (sign == 1) { t = 0.9; w = 0; h = 1; }
				else { t = 0; w = 1; h = 0.55; }
				break;
			}
		}

		float up = bmin[1] + (top - bmin[1]) * h;
		if (alongZ)
			sp.m_vOffset = Vector(mid[0] + w * halfW, up, mid[2] + t * halfL);
		else
			sp.m_vOffset = Vector(mid[0] + t * halfL, up, mid[2] + w * halfW);
	}

	//------------------------------------------------------------------------------------------------
	override void Log(notnull array<string> lines, notnull array<int> tones)
	{
		RZ_DutySubject s = Subj();
		if (!s)
			return;

		lines.Copy(s.m_aLog);
		tones.Copy(s.m_aTones);
	}

	//------------------------------------------------------------------------------------------------
	override void Actions(notnull array<string> labels)
	{
		RZ_DutySubject s = Subj();
		if (s && !s.m_bTaken)
			labels.Insert(RZ_DutyGame.TakePrompt(m_Game.m_iKind));
	}

	//------------------------------------------------------------------------------------------------
	override void Look(int spot, int playerId)
	{
		if (playerId == m_Game.m_iPlayer)
			m_Owner.HandsLook(m_Game, m_iIndex, spot);
	}

	//------------------------------------------------------------------------------------------------
	override bool Act(int index, int playerId)
	{
		if (index != 0 || playerId != m_Game.m_iPlayer)
			return false;

		m_Owner.HandsTake(m_Game, m_iIndex, playerId);
		return true;
	}
}
