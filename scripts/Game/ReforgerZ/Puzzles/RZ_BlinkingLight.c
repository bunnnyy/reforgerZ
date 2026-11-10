//! International Morse code, and turning a message into an on/off schedule.
class RZ_Morse
{
	//------------------------------------------------------------------------------------------------
	static string Code(string ch)
	{
		switch (ch)
		{
			case "A": return ".-";
			case "B": return "-...";
			case "C": return "-.-.";
			case "D": return "-..";
			case "E": return ".";
			case "F": return "..-.";
			case "G": return "--.";
			case "H": return "....";
			case "I": return "..";
			case "J": return ".---";
			case "K": return "-.-";
			case "L": return ".-..";
			case "M": return "--";
			case "N": return "-.";
			case "O": return "---";
			case "P": return ".--.";
			case "Q": return "--.-";
			case "R": return ".-.";
			case "S": return "...";
			case "T": return "-";
			case "U": return "..-";
			case "V": return "...-";
			case "W": return ".--";
			case "X": return "-..-";
			case "Y": return "-.--";
			case "Z": return "--..";
			case "0": return "-----";
			case "1": return ".----";
			case "2": return "..---";
			case "3": return "...--";
			case "4": return "....-";
			case "5": return ".....";
			case "6": return "-....";
			case "7": return "--...";
			case "8": return "---..";
			case "9": return "----.";
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! ".-- --- .-.." (letters separated by spaces, words by " / ").
	static string Encode(string text)
	{
		string upper = text;
		upper.ToUpper();
		string result = "";
		int len = upper.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = upper.Substring(i, 1);
			if (ch == " ")
			{
				result += "/ ";
				continue;
			}

			string code = Code(ch);
			if (!code.IsEmpty())
				result += code + " ";
		}

		return result.Trim();
	}

	//------------------------------------------------------------------------------------------------
	//! Standard timing in units: dot 1 on, dash 3 on, 1 off inside a letter,
	//! 3 off between letters, 7 between words, then a long pause before it repeats.
	static void ToSchedule(string text, notnull array<bool> units)
	{
		units.Clear();
		string upper = text;
		upper.ToUpper();
		int len = upper.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = upper.Substring(i, 1);
			if (ch == " ")
			{
				AddUnits(units, false, 4);	// 3 already added after the last letter
				continue;
			}

			string code = Code(ch);
			int clen = code.Length();
			for (int j = 0; j < clen; j++)
			{
				if (code.Substring(j, 1) == "-")
					AddUnits(units, true, 3);
				else
					AddUnits(units, true, 1);

				if (j < clen - 1)
					AddUnits(units, false, 1);
			}

			AddUnits(units, false, 3);
		}

		AddUnits(units, false, 11);	// 14 units of dark before it starts again
	}

	//------------------------------------------------------------------------------------------------
	protected static void AddUnits(array<bool> units, bool on, int count)
	{
		for (int i = 0; i < count; i++)
		{
			units.Insert(on);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The card a scout post keeps: the alphabet, for players to decode by hand.
	static string AlphabetCard()
	{
		string card = "";
		string letters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		for (int i = 0; i < 26; i++)
		{
			string ch = letters.Substring(i, 1);
			card += ch + " " + Code(ch);
			if (i < 25)
				card += ",  ";
		}

		return card;
	}
}

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A light the server switches on and off (Morse signals). Needs an RplComponent. Without this component, the light prefab is spawned and deleted for each flash instead.")]
class RZ_BlinkLightComponentClass : ScriptComponentClass
{
}

//! Put this on a lamp prefab that has a light (child LightEntity or emissive mesh).
//! The on/off state is replicated; each client shows or hides the entity and its
//! children locally.
class RZ_BlinkLightComponent : ScriptComponent
{
	[RplProp(onRplName: "ApplyVisual")]
	protected bool m_bOn = true;

	//------------------------------------------------------------------------------------------------
	//! Server only.
	void SetOn(bool on)
	{
		if (on == m_bOn)
			return;

		m_bOn = on;
		Replication.BumpMe();
		ApplyVisual();
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyVisual()
	{
		SetVisible(GetOwner(), m_bOn, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static void SetVisible(IEntity e, bool on, int depth)
	{
		if (!e || depth > 2)
			return;

		if (on)
			e.SetFlags(EntityFlags.VISIBLE, true); // VERIFY: SetFlags/ClearFlags(EntityFlags.VISIBLE) hides a light and its mesh, on clients too
		else
			e.ClearFlags(EntityFlags.VISIBLE, true);

		IEntity child = e.GetChildren();
		while (child)
		{
			SetVisible(child, on, depth + 1);
			child = child.GetSibling();
		}
	}
}

//! Drives one Morse light from the server, one step per Morse unit through the
//! call queue (not per frame). The mission starts it only while it's dark and
//! someone is close enough to see it.
class RZ_BlinkingLight : Managed
{
	protected ResourceName m_sPrefab;
	protected vector m_vPos;
	protected IEntity m_Light;				// the persistent light (component mode)
	protected IEntity m_Flash;				// the spawned flash (spawn mode)
	protected RZ_BlinkLightComponent m_Comp;
	protected ref array<bool> m_aUnits = {};
	protected int m_iStep;
	protected bool m_bRunning;
	protected float m_fUnitSeconds = 1.5;

	protected static const float SPAWN_MODE_UNIT = 3;	// slower when every flash is a spawn

	//------------------------------------------------------------------------------------------------
	void RZ_BlinkingLight(ResourceName prefab, vector pos, string message, float unitSeconds)
	{
		m_sPrefab = prefab;
		m_vPos = pos;
		m_fUnitSeconds = unitSeconds;
		RZ_Morse.ToSchedule(message, m_aUnits);

		m_Light = RZ_Util.SpawnPrefab(prefab, pos, 0);
		if (m_Light)
			m_Comp = RZ_BlinkLightComponent.Cast(m_Light.FindComponent(RZ_BlinkLightComponent));

		if (!m_Comp)
		{
			// Spawn mode: the light only exists while it's lit
			if (m_Light)
				SCR_EntityHelper.DeleteEntityAndChildren(m_Light);

			m_Light = null;
			m_fUnitSeconds = Math.Max(m_fUnitSeconds, SPAWN_MODE_UNIT);
		}
		else
		{
			m_Comp.SetOn(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ~RZ_BlinkingLight()
	{
		Stop();
		if (m_Light)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Light);

		m_Light = null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsRunning()
	{
		return m_bRunning;
	}

	//------------------------------------------------------------------------------------------------
	void Start()
	{
		if (m_bRunning || m_aUnits.IsEmpty())
			return;

		m_bRunning = true;
		m_iStep = 0;
		GetGame().GetCallqueue().CallLater(Step, m_fUnitSeconds * 1000, true);
	}

	//------------------------------------------------------------------------------------------------
	void Stop()
	{
		if (m_bRunning && GetGame() && GetGame().GetCallqueue())
			GetGame().GetCallqueue().Remove(Step);

		m_bRunning = false;
		SetOn(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void Step()
	{
		if (m_aUnits.IsEmpty())
			return;

		SetOn(m_aUnits[m_iStep]);
		m_iStep = (m_iStep + 1) % m_aUnits.Count();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetOn(bool on)
	{
		if (m_Comp)
		{
			m_Comp.SetOn(on);
			return;
		}

		if (on && !m_Flash)
			m_Flash = RZ_Util.SpawnPrefab(m_sPrefab, m_vPos, 0);
		else if (!on && m_Flash)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(m_Flash);
			m_Flash = null;
		}
	}
}
