//! Radios, open and secure (docs/DESIGN.md, "World interactions").
//!
//! - Open channels are already on the air (faction chatter in RZ_FactionLife, the Haven's open
//!   line and Bastion's and the Red Hand's in new-player guidance). A radio found in a house can
//!   be tuned through them, and a radio next to a running generator hears all of them.
//! - Secure channels (Bastion's command net, Halcyon's lab channel, the Red Hand boss's own
//!   frequency) can only be cracked by a Radio Operator: stand still with a radio for about a
//!   minute (noise, an infected close or a hit stops it; a charged radio is faster), then listen
//!   for 20 minutes to what the faction really says (RZ_FactionLife.GetSecureTraffic): patrol and
//!   shift times, truck departures and routes, samples wanted. Starting it gives the "channel" tip.
//!   Crack it on a radio item lying on the ground (RZ_TuneRadioAction) or on a house radio.

//------------------------------------------------------------------------------------------------
//! Settings on RZ_GameModeComponent > Interactions > Secure radio.
[BaseContainerProps()]
class RZ_RadioCrackSettings
{
	[Attribute("1", desc: "Radio Operators can crack secure faction channels; house radios can be tuned")]
	bool m_bEnabled;

	[Attribute("60", desc: "Seconds standing still with a radio to crack a channel")]
	float m_fCrackSeconds;

	[Attribute("0.75", desc: "Crack time multiplier with a charged radio (a generator)")]
	float m_fChargedMult;

	[Attribute("20", desc: "Real minutes you can listen once it's cracked")]
	float m_fListenMinutes;

	[Attribute("4", desc: "Real minutes between lines of secure traffic")]
	float m_fLineMinutes;

	[Attribute("3000", desc: "The secure channel of the nearest faction place within this distance (m) is the one you crack")]
	float m_fRange;

	[Attribute("40", desc: "Heat where you stand that stops the cracking")]
	float m_fNoiseInterrupt;

	[Attribute("10", desc: "An infected this close (m) stops the cracking")]
	float m_fZombieInterrupt;

	[Attribute("radio,transistor,receiver,wireless,shortwave", desc: "House radio prefab keywords (masts and stations are left out)")]
	string m_sHouseRadioWords;
}

//------------------------------------------------------------------------------------------------
//! "Work on a scrambled channel": on a radio item's ActionsManagerComponent (the wind-up radio,
//! any radio you want crackable). Used from the ground.
class RZ_TuneRadioAction : RZ_GroupedAction
{
	//------------------------------------------------------------------------------------------------
	override string MenuTitle(IEntity owner)
	{
		return "Use the radio";
	}

	//------------------------------------------------------------------------------------------------
	override bool Label(out string outName)
	{
		outName = "Work on a scrambled channel";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void Run(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RZ_Interactions hub = RZ_Interactions.Get();
		int playerId = RZ_PuzzleActionHelper.GetPlayerId(pUserEntity);
		if (!hub || playerId <= 0)
			return;

		hub.GetRadio().StartCrack(playerId, pUserEntity, -1);
	}
}

//------------------------------------------------------------------------------------------------
//! Someone listening to a cracked channel.
class RZ_Listener
{
	int m_iPlayerId;
	int m_iFaction;
	float m_fUntil;
	float m_fNext;
	vector m_vAnchor;
	bool m_bAnchored;		// cracked on a house radio: stay near it unless you carry a radio
}

//------------------------------------------------------------------------------------------------
class RZ_SecureRadio
{
	protected static const float ANCHOR_RADIUS = 30;

	// Why a crack can't start (StartCrack, and the object menu's locked row: CrackLock)
	protected static const string NOT_OPERATOR = "Under the static there's a voice, chopped into pieces. Someone who knew radios could put it back together. You can't.";
	protected static const string LISTENING = "You're already on a cracked channel. Keep listening.";
	protected static const string NOTHING_NEAR = "Nothing scrambled in range. Get closer to somewhere a faction holds.";
	protected static const string TOO_LOUD = "Too loud here to hear a scrambled channel. Find somewhere quiet.";

	protected RZ_GameModeComponent m_GM;
	protected RZ_Interactions m_Hub;
	protected RZ_RadioCrackSettings m_Settings;
	protected ref array<ref RZ_Listener> m_aListeners = {};
	protected ref array<string> m_aHouseWords = {};
	protected ref map<int, int> m_mHeardOn = new map<int, int>();	// player -> the open channel their last Listen caught (-1: only the broadcast)

	//------------------------------------------------------------------------------------------------
	void RZ_SecureRadio(RZ_GameModeComponent gm, RZ_Interactions hub, RZ_RadioCrackSettings settings)
	{
		m_GM = gm;
		m_Hub = hub;
		m_Settings = settings;
		RZ_ItemHelper.Words(m_Settings.m_sHouseRadioWords, m_aHouseWords);
	}

	//------------------------------------------------------------------------------------------------
	bool IsHouseRadioName(string name)
	{
		if (!m_Settings.m_bEnabled || !RZ_ItemHelper.NameHas(name, m_aHouseWords))
			return false;

		return !name.Contains("mast") && !name.Contains("antenna") && !name.Contains("tower") && !name.Contains("station") && !name.Contains("relay");
	}

	//------------------------------------------------------------------------------------------------
	static string ChannelName(int f)
	{
		switch (f)
		{
			case RZ_EFaction.MILITARY: return "Bastion command net";
			case RZ_EFaction.BANDITS: return "the Red Hand boss's channel";
			case RZ_EFaction.INSTITUTE: return "Halcyon's lab channel";
		}

		return "a scrambled channel";
	}

	//------------------------------------------------------------------------------------------------
	//! Noise or an infected close stops a crack.
	bool IsTooNoisy(vector pos)
	{
		if (m_GM.GetHeat().GetHeat(pos) >= m_Settings.m_fNoiseInterrupt)
			return true;

		return m_GM.GetSpawner().IsZombieNear(pos, m_Settings.m_fZombieInterrupt);
	}

	//------------------------------------------------------------------------------------------------
	void OnPlayerSpawned(int playerId)
	{
		for (int i = m_aListeners.Count() - 1; i >= 0; i--)
		{
			if (m_aListeners[i].m_iPlayerId == playerId)
				m_aListeners.Remove(i);
		}
	}

	// ================================================================ House radios
	//------------------------------------------------------------------------------------------------
	//! "Listen to the radio": one thing on the air right now.
	void ListenHouseRadio(int playerId, RZ_Spot s, IEntity user)
	{
		int heardOn = -1;
		string line = AnyOpenLine(heardOn);
		if (line.IsEmpty())
			line = "Static, and under it the same government recording, over and over: remain indoors, do not approach the perimeter.";

		m_mHeardOn.Set(playerId, heardOn);
		m_GM.NotifyPlayer(playerId, "Radio", line);
	}

	//------------------------------------------------------------------------------------------------
	//! The object menu's status strip on a house radio (RZ_ObjectStatus): POWER, RECEIVE, TRANSMIT
	//! and SIGNAL, what power would change, and which channel the read caught.
	void HouseRadioStatus(int playerId, RZ_Spot s, notnull RZ_ObjectStatus st)
	{
		bool powered = RZ_PowerGrid.IsPoweredAt(s.m_vPos);
		if (powered)
			st.Add("POWER", "ON", RZ_ObjectStatus.STATE_OK);
		else
			st.Add("POWER", "OFF", RZ_ObjectStatus.STATE_OFF);

		// Batteries hear the channels; only power carries a voice (RZ_DialogueNets.CanCall)
		st.Add("RECEIVE", "ON", RZ_ObjectStatus.STATE_OK);
		IEntity houseSet;
		if (s.m_Comp)
			houseSet = s.m_Comp.GetOwner();

		bool calls = RZ_Dialogue.IsOn() && RZ_DialogueSettings.Get().m_bTrees;
		if (calls && RZ_DialogueNets.CanCall(houseSet))
			st.Add("TRANSMIT", "YES", RZ_ObjectStatus.STATE_OK);
		else if (calls)
			st.Add("TRANSMIT", "NO", RZ_ObjectStatus.STATE_OFF);
		else
			st.Add("TRANSMIT", "NO", RZ_ObjectStatus.STATE_DIM);

		// The open channels it pulls in now: all with power, two on batteries (as Tune)
		int onAir = 0;
		array<int> order = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		foreach (int f : order)
		{
			if (!OpenLine(f).IsEmpty())
				onAir++;
		}

		int heard = onAir;
		if (!powered)
			heard = Math.Min(onAir, 2);

		if (heard >= 3)
			st.Add("SIGNAL", "STRONG", RZ_ObjectStatus.STATE_OK);
		else if (heard > 0)
			st.Add("SIGNAL", "WEAK", RZ_ObjectStatus.STATE_PART);
		else
			st.Add("SIGNAL", "NONE", RZ_ObjectStatus.STATE_OFF);

		if (!powered)
			st.m_sHint = string.Format("A running generator within %1 m would power it.", RZ_Util.RoundToInt(m_Hub.GetTraps().PowerRadius()));

		int last;
		if (!m_mHeardOn.Find(playerId, last))
			return;

		string channel = "THE BROADCAST";
		if (last >= 0)
			channel = RZ_Factions.GetShortName(last);

		channel.ToUpper();
		st.m_sReadLabel = "LAST HEARD · " + channel;
	}

	//------------------------------------------------------------------------------------------------
	//! A house radio's option `option` can't be done now: what HandleHouseRadio would say instead,
	//! and the tag (RZ_ObjectStatus.SpotLock asks after work under way).
	bool HouseRadioLock(int playerId, RZ_Spot s, int option, IEntity user, out string reason, out string tag)
	{
		if (option == 1)
			return CrackLock(playerId, user, reason, tag);

		if (option != 2)
			return false;

		IEntity houseSet;
		if (s.m_Comp)
			houseSet = s.m_Comp.GetOwner();

		if (RZ_DialogueNets.CanCall(houseSet))
			return false;

		reason = RZ_Dialogue.NO_POWER;
		tag = "NEEDS POWER";
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Cracking a channel here can't start now: StartCrack's own refusals, in its order.
	protected bool CrackLock(int playerId, IEntity user, out string reason, out string tag)
	{
		if (!RZ_Radio.IsRadioOperator(m_GM, playerId))
		{
			reason = NOT_OPERATOR;
			tag = "OPERATOR ONLY";
			return true;
		}

		if (IsListening(playerId))
		{
			reason = LISTENING;
			tag = "LISTENING";
			return true;
		}

		if (PickFaction(user.GetOrigin()) < 0)
		{
			reason = NOTHING_NEAR;
			tag = "NO CHANNEL";
			return true;
		}

		if (IsTooNoisy(user.GetOrigin()))
		{
			reason = TOO_LOUD;
			tag = "TOO LOUD";
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! 0 tune through the open channels, 1 crack a scrambled channel (Radio Operators), 2 call on
	//! the open channel, 3 the radio screen (UI/RZ_RadioMenu.c).
	void HandleHouseRadio(int playerId, RZ_Spot s, int option, IEntity user)
	{
		if (option == 3)
		{
			if (s.m_Comp)
				RZ_RadioScreen.Open(m_GM, playerId, s.m_Comp.GetOwner(), m_Hub.Title(s), RZ_PowerGrid.IsPoweredAt(s.m_vPos));

			return;
		}

		if (!m_Settings.m_bEnabled)
			return;

		if (option == 1)
		{
			StartCrack(playerId, user, s.m_iId);
			return;
		}

		// A house set calls the open channel with power (docs/DIALOGUE.md, "The open channel")
		if (option == 2)
		{
			IEntity houseSet;
			if (s.m_Comp)
				houseSet = s.m_Comp.GetOwner();

			if (houseSet && m_GM.GetDialogue())
				m_GM.GetDialogue().OpenRadio(playerId, houseSet, user);

			return;
		}

		if (option != 0)
			return;

		// Every open channel with something to say; clearer with power
		array<int> order = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		bool powered = RZ_PowerGrid.IsPoweredAt(s.m_vPos);
		string text = "";
		int heard = 0;
		foreach (int f : order)
		{
			if (!powered && heard >= 2)
				break;

			string line = OpenLine(f);
			if (line.IsEmpty())
				continue;

			if (heard > 0)
				text += "\n";

			text += string.Format("[%1] %2", RZ_FactionVoice.ChannelTitle(f), line);
			heard++;
		}

		if (heard == 0)
			text = "Nothing but static and the government recording. Whatever else is on the air is too far off for this set.";
		else if (!powered)
			text += "\n(The batteries are weak; with power it would pull in more.)";

		m_GM.NotifyPlayer(playerId, "Radio", text);
	}

	//------------------------------------------------------------------------------------------------
	//! A line on an open channel: faction f's chatter, or any faction's (-1).
	protected string OpenLine(int f)
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (!life || !life.IsActive())
			return "";

		if (f >= 0)
			return life.ChatterFor(f);

		int any;
		return AnyOpenLine(any);
	}

	//------------------------------------------------------------------------------------------------
	//! A line on any open channel, from a random one on; `faction` the channel it came from (-1: none).
	protected string AnyOpenLine(out int faction)
	{
		faction = -1;
		RZ_FactionLife life = m_GM.GetFactionLife();
		if (!life || !life.IsActive())
			return "";

		array<int> order = {RZ_EFaction.SURVIVORS, RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		int start = Math.RandomInt(0, order.Count());
		for (int i = 0; i < order.Count(); i++)
		{
			int f = order[(start + i) % order.Count()];
			string line = life.ChatterFor(f);
			if (!line.IsEmpty())
			{
				faction = f;
				return line;
			}
		}

		return "";
	}

	// ================================================================ Cracking
	//------------------------------------------------------------------------------------------------
	//! The faction whose secure channel is nearest: its closest place within range, else -1.
	protected int PickFaction(vector pos)
	{
		RZ_Territory territory = m_GM.GetTerritory();
		int best = -1;
		float bestDist = m_Settings.m_fRange;
		array<int> factions = {RZ_EFaction.MILITARY, RZ_EFaction.BANDITS, RZ_EFaction.INSTITUTE};
		foreach (int f : factions)
		{
			if (!territory || !territory.IsActive())
				break;

			RZ_Holding h = territory.FindHolding(f, pos, m_Settings.m_fRange);
			if (h && RZ_Util.DistXZ(h.m_vPos, pos) < bestDist)
			{
				bestDist = RZ_Util.DistXZ(h.m_vPos, pos);
				best = f;
			}
		}

		// Without territory: Halcyon by its lab
		if (best < 0 && m_GM.HasLab() && RZ_Util.DistXZ(m_GM.GetLabPosition(), pos) <= m_Settings.m_fRange)
			best = RZ_EFaction.INSTITUTE;

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! Radio Operators only, with a radio (or at a house radio). spotId -1 = a radio item.
	void StartCrack(int playerId, IEntity user, int spotId)
	{
		if (!m_Settings.m_bEnabled || !user)
			return;

		if (!RZ_Radio.IsRadioOperator(m_GM, playerId))
		{
			m_GM.NotifyPlayer(playerId, "Radio", NOT_OPERATOR);
			return;
		}

		if (spotId < 0 && !RZ_Radio.HasRadio(m_GM, playerId, user))
			return;

		if (IsListening(playerId))
		{
			m_GM.NotifyPlayer(playerId, "Radio", LISTENING);
			return;
		}

		if (m_Hub.HasTask(playerId))
			return;

		int f = PickFaction(user.GetOrigin());
		if (f < 0)
		{
			m_GM.NotifyPlayer(playerId, "Radio", NOTHING_NEAR);
			return;
		}

		if (IsTooNoisy(user.GetOrigin()))
		{
			m_GM.NotifyPlayer(playerId, "Radio", TOO_LOUD);
			return;
		}

		float seconds = m_Settings.m_fCrackSeconds;
		if (m_Hub.GetTraps().IsCharged(playerId))
			seconds = seconds * m_Settings.m_fChargedMult;

		m_Hub.StartTask(playerId, user, RZ_Interactions.TASK_CRACK, spotId, f, seconds, "the channel", true);
		RZ_Guidance.Report(playerId, "channel");
		m_GM.NotifyPlayer(playerId, "Radio", string.Format("There it is: %1, chopped and shuffled. Stand still and keep quiet for about %2 seconds while you work it out. Noise, a hit or one of them close and you lose it.",
			ChannelName(f), RZ_Util.RoundToInt(seconds)));
	}

	//------------------------------------------------------------------------------------------------
	void CompleteCrack(int playerId, int f, IEntity user, int spotId)
	{
		RZ_Listener l = new RZ_Listener();
		l.m_iPlayerId = playerId;
		l.m_iFaction = f;
		l.m_fUntil = m_Hub.GetClock() + m_Settings.m_fListenMinutes * 60;
		l.m_fNext = m_Hub.GetClock() + m_Settings.m_fLineMinutes * 60;
		RZ_Spot s = m_Hub.FindSpot(spotId);
		if (spotId >= 0 && s)
		{
			l.m_vAnchor = s.m_vPos;
			l.m_bAnchored = true;
		}

		m_aListeners.Insert(l);
		string traffic = Traffic(f);
		// Bastion's net carries the word at the wire (a cover detail at its looks, RZ_Disguise)
		RZ_Hostility hostility = m_GM.GetHostility();
		if (f == RZ_EFaction.MILITARY && hostility && hostility.GetDisguise().IsOn())
		{
			hostility.GetDisguise().LearnWord(playerId);
			traffic += string.Format("\n...word at the wire is %1. Until the shift change. Out.", hostility.GetDisguise().WordToday());
		}

		m_GM.NotifyPlayer(playerId, "Cracked: " + ChannelName(f), string.Format("The voices come clear. You've got about %1 minutes before they change codes.\n%2", RZ_Util.RoundToInt(m_Settings.m_fListenMinutes), traffic));
	}

	//------------------------------------------------------------------------------------------------
	protected string Traffic(int f)
	{
		RZ_FactionLife life = m_GM.GetFactionLife();
		string line;
		if (life)
			line = life.GetSecureTraffic(f);

		if (line.IsEmpty())
			return "...no contact, nothing moving on the road. Maintain. Out.";

		return line;
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose secure channel this player is listening to, or -1 (the HUD indicator).
	int GetListening(int playerId, out float secondsLeft)
	{
		secondsLeft = 0;
		foreach (RZ_Listener l : m_aListeners)
		{
			if (l.m_iPlayerId == playerId)
			{
				secondsLeft = Math.Max(0, l.m_fUntil - m_Hub.GetClock());
				return l.m_iFaction;
			}
		}

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsListening(int playerId)
	{
		foreach (RZ_Listener l : m_aListeners)
		{
			if (l.m_iPlayerId == playerId)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		float now = m_Hub.GetClock();
		PlayerManager pm = GetGame().GetPlayerManager();
		for (int i = m_aListeners.Count() - 1; i >= 0; i--)
		{
			RZ_Listener l = m_aListeners[i];
			IEntity p = pm.GetPlayerControlledEntity(l.m_iPlayerId);
			if (!p || !RZ_Util.IsCharacterAlive(p))
			{
				m_aListeners.Remove(i);
				continue;
			}

			if (now >= l.m_fUntil)
			{
				m_GM.NotifyPlayer(l.m_iPlayerId, ChannelName(l.m_iFaction), "A burst of tones, and the voices turn to mush again. They've changed codes.");
				m_aListeners.Remove(i);
				continue;
			}

			bool carries = RZ_Radio.CarriesRadio(p);
			if (l.m_bAnchored && !carries && RZ_Util.DistXZ(p.GetOrigin(), l.m_vAnchor) > ANCHOR_RADIUS)
			{
				m_GM.NotifyPlayer(l.m_iPlayerId, ChannelName(l.m_iFaction), "You've left the radio behind, and the channel with it.");
				m_aListeners.Remove(i);
				continue;
			}

			if (!l.m_bAnchored && !carries)
				continue;

			if (now < l.m_fNext)
				continue;

			l.m_fNext = now + m_Settings.m_fLineMinutes * 60;
			m_GM.NotifyPlayer(l.m_iPlayerId, ChannelName(l.m_iFaction), Traffic(l.m_iFaction));
		}
	}
}
