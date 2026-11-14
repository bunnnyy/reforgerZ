//! A duty's beat up close (docs/ERRANDS.md, "Scenes and beats"; the view: Interactions/RZ_HandsOn.c):
//! the beats whose things to do stand at one spot (an arrival at the decon line, a bag at the
//! incinerator, a bed on the ward, a kit on the bunk, a thing off the tarp) open the camera on what
//! stands there. A beat of looks (fewer than its places) shows each look as a glowing place on it,
//! each a short hold, and what it showed stays on it; the calls that end it are the panel's buttons
//! (or places too, when there are more than three). Everything said out here is in the panel's log.
//! Choosing up close is the same as the prompt's hold, and the next beat at the same spot opens up
//! close by itself. Beats with someone to talk to stay the talk. Server only.
class RZ_BeatHandsOn : RZ_HandsOnGame
{
	protected RZ_DutyScenes m_Owner;
	protected RZ_DutyScene m_Scene;
	protected int m_iBeat;

	//------------------------------------------------------------------------------------------------
	void RZ_BeatHandsOn(RZ_DutyScenes owner, RZ_DutyScene scene, int beat)
	{
		m_Owner = owner;
		m_Scene = scene;
		m_iBeat = beat;
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_DutyBeat Beat()
	{
		if (!m_Owner || !m_Scene)
			return null;

		return m_Owner.OpenBeatOf(m_Scene, m_iBeat);
	}

	//------------------------------------------------------------------------------------------------
	//! The options at the beat's spot, split into places to look and the panel's buttons.
	protected void Split(notnull array<int> spots, notnull array<int> acts)
	{
		RZ_DutyBeat b = Beat();
		if (!b)
			return;

		foreach (int i, RZ_DutyOption o : b.m_aOptions)
		{
			if (o.m_fOut > RZ_DutyScenes.SAME_SPOT)
				continue;

			if (b.m_iPicks > 1 && !o.m_bEnds)
				spots.Insert(i);
			else
				acts.Insert(i);
		}

		// More calls than buttons: they're places too
		if (acts.Count() > 3)
		{
			foreach (int a : acts)
			{
				spots.Insert(a);
			}

			acts.Clear();
		}
	}

	//------------------------------------------------------------------------------------------------
	override IEntity Subject()
	{
		if (!m_Owner || !m_Scene)
			return null;

		return m_Owner.BeatEntity(m_Scene);
	}

	//------------------------------------------------------------------------------------------------
	override vector Where()
	{
		RZ_DutyBeat b = Beat();
		if (!b)
			return vector.Zero;

		return b.m_vAt;
	}

	//------------------------------------------------------------------------------------------------
	override int Frame()
	{
		RZ_DutyBeat b = Beat();
		if (b && b.m_iBody >= 0)
			return RZ_HandsOn.FRAME_LYING;

		IEntity e = Subject();
		if (e && ChimeraCharacter.Cast(e))
			return RZ_HandsOn.FRAME_PERSON;

		return RZ_HandsOn.FRAME_THING;
	}

	//------------------------------------------------------------------------------------------------
	override string Title()
	{
		RZ_DutyBeat b = Beat();
		if (b && !b.m_sName.IsEmpty())
			return b.m_sName;

		return m_Scene.m_sTitle;
	}

	//------------------------------------------------------------------------------------------------
	override string Status()
	{
		RZ_DutyBeat b = Beat();
		if (!b || b.m_iPicks <= 1)
			return m_Scene.m_sTitle;

		int left = Math.Max(0, b.m_iPicks - b.m_iPicked);
		if (left == 0)
			return "No looks left here: make the call";

		if (left == 1)
			return "1 look left here";

		return string.Format("%1 looks left here", left);
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		RZ_DutyBeat b = Beat();
		if (!b)
			return "";

		return b.m_sLine;
	}

	//------------------------------------------------------------------------------------------------
	override void Spots(notnull array<ref RZ_HandsOnSpot> spots)
	{
		RZ_DutyBeat b = Beat();
		if (!b)
			return;

		array<int> places = {};
		array<int> acts = {};
		Split(places, acts);
		int n = places.Count();
		foreach (int k, int i : places)
		{
			RZ_DutyOption o = b.m_aOptions[i];
			RZ_HandsOnSpot sp = new RZ_HandsOnSpot();
			sp.m_sLabel = o.m_sLabel;
			// Set out across the thing as you see it, in a shallow arc
			sp.m_sBone = "@view";
			float across = 0;
			if (n > 1)
				across = (k / (n - 1.0)) * 2 - 1;

			sp.m_vOffset = Vector(across * 0.55, 0.15 - Math.AbsFloat(across) * 0.25, 0.25);
			if (o.m_bDone)
			{
				sp.m_iState = RZ_HandsOn.SPOT_SEEN;
				sp.m_sSeen = o.m_sSaid;
			}
			else if (b.m_iPicks > 1 && !o.m_bEnds && b.m_iPicked >= b.m_iPicks)
			{
				sp.m_iState = RZ_HandsOn.SPOT_SHUT;
			}

			spots.Insert(sp);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Log(notnull array<string> lines, notnull array<int> tones)
	{
		lines.Copy(m_Scene.m_aLog);
		tones.Copy(m_Scene.m_aLogTones);
	}

	//------------------------------------------------------------------------------------------------
	override void Actions(notnull array<string> labels)
	{
		RZ_DutyBeat b = Beat();
		if (!b)
			return;

		array<int> places = {};
		array<int> acts = {};
		Split(places, acts);
		foreach (int i : acts)
		{
			labels.Insert(b.m_aOptions[i].m_sLabel);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void Look(int spot, int playerId)
	{
		array<int> places = {};
		array<int> acts = {};
		Split(places, acts);
		if (spot < 0 || spot >= places.Count())
			return;

		RZ_DutyBeat b = Beat();
		if (b && b.m_aOptions[places[spot]].m_bDone)
			return;	// seen already: it shows again for nothing

		m_Owner.HandsChoose(m_Scene, m_iBeat, places[spot], playerId);
	}

	//------------------------------------------------------------------------------------------------
	override bool Act(int index, int playerId)
	{
		array<int> places = {};
		array<int> acts = {};
		Split(places, acts);
		if (index < 0 || index >= acts.Count())
			return false;

		m_Owner.HandsChoose(m_Scene, m_iBeat, acts[index], playerId);
		return Beat() == null;
	}
}
