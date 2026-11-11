//! Talking to people (docs/DESIGN.md, "Talking to people"). Server side.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Dialogue: the talk screen and the screen guard that closes
//! the talk screen and the reader when danger comes (UI/RZ_ScreenGuard.c). Read on every machine.
[BaseContainerProps()]
class RZ_DialogueSettings
{
	[Attribute("6", desc: "Talk: you stay in a conversation while you're this close to the person (m)")]
	float m_fRange;

	[Attribute("120", desc: "Talk: seconds with no reply picked before the conversation ends by itself")]
	float m_fIdleSeconds;

	[Attribute("6", desc: "Danger: an infected this close to you (or to the person you talk to) closes the talk screen and the reader (m)")]
	float m_fDangerNear;

	[Attribute("30", desc: "Danger: an infected within this distance that came 2 m closer since the last check closes the screen (m)")]
	float m_fDangerClosing;

	[Attribute("32", desc: "Talk screens and readers open at once on the server, at most")]
	int m_iMaxOpen;

	protected static ref RZ_DialogueSettings s_Defaults;

	//------------------------------------------------------------------------------------------------
	//! The game mode's settings, else the defaults (always non-null).
	static RZ_DialogueSettings Get()
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (gm && gm.GetDialogueSettings())
			return gm.GetDialogueSettings();

		if (!s_Defaults)
			s_Defaults = RZ_DialogueSettings.Cast(RZ_Util.NewWithDefaults("RZ_DialogueSettings"));

		return s_Defaults;
	}
}
