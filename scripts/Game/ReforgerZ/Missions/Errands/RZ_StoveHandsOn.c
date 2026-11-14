//! Minding the pot, up close (docs/ERRANDS.md, "The kitchen"; the view: Interactions/RZ_HandsOn.c).
//! The kitchen duty's "Stir the pot" opens the camera on the real cook pot. The fire under it is
//! yours: more wood or rake it down (the panel's buttons), and the pot follows it slowly. Kept at a
//! simmer and stirred, it cooks; left too long without a stir it catches on the bottom, too hot it
//! boils over, too cool it stops cooking. Twenty seconds at a simmer and it's done: that's the
//! chore. Burnt through, it's scraped out and started again (nothing is lost but the time). Tasting
//! says what it wants. Nothing here pays more than the chore: it's the same 3 Favor, done by hand.
//! Server only.
class RZ_StoveHandsOn : RZ_HandsOnGame
{
	protected static const float COOK_FOR = 20;		// seconds at a simmer, stirred
	protected static const float SIMMER_LOW = 55;
	protected static const float SIMMER_HIGH = 78;
	protected static const float BOIL_OVER = 90;
	protected static const float CATCH_AFTER = 9;		// seconds without a stir, hot, before it catches
	protected static const float BURN_AFTER = 16;		// and burns through
	protected static const int FIRE_MAX = 4;

	protected string m_sKey;
	protected int m_iPlayer;
	protected float m_fHeat = 35;
	protected int m_iFire = 2;
	protected float m_fCooked;
	protected float m_fSinceStir;
	protected bool m_bCaught;
	protected bool m_bOver;
	protected int m_iSpills;
	protected bool m_bDone;
	protected int m_iLastBand = -1;
	protected ref array<string> m_aLog = {};
	protected ref array<int> m_aTones = {};

	//------------------------------------------------------------------------------------------------
	void RZ_StoveHandsOn(string key, int player)
	{
		m_sKey = key;
		m_iPlayer = player;
		Say("The pot's on, the fire's low. Keep it at a simmer and keep it moving.", RZ_HandsOn.TONE_PLAIN);
	}

	//------------------------------------------------------------------------------------------------
	protected void Say(string line, int tone)
	{
		m_aLog.Insert(line);
		m_aTones.Insert(tone);
	}

	//------------------------------------------------------------------------------------------------
	override IEntity Subject()
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return null;

		return sites.BoxOf(m_sKey);
	}

	//------------------------------------------------------------------------------------------------
	override vector Where()
	{
		RZ_WorkSites sites = RZ_WorkSites.Get();
		if (!sites)
			return vector.Zero;

		return sites.PosOf(m_sKey);
	}

	//------------------------------------------------------------------------------------------------
	override string Title()
	{
		return "The pot";
	}

	//------------------------------------------------------------------------------------------------
	//! The heat in words, 0 cold to 4 boiling over.
	protected int Band()
	{
		if (m_fHeat >= BOIL_OVER)
			return 4;

		if (m_fHeat > SIMMER_HIGH)
			return 3;

		if (m_fHeat >= SIMMER_LOW)
			return 2;

		if (m_fHeat >= 35)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	override string Status()
	{
		array<string> words = {"Cold", "Warming", "Simmering", "Boiling hard", "Boiling over"};
		array<string> fire = {"no fire", "embers", "a low fire", "a good fire", "a roaring fire"};
		if (m_bDone)
			return "Done, so go and tell the cook";

		return string.Format("%1 on %2 · cooked %3 of %4 s", words[Band()], fire[m_iFire], RZ_Util.RoundToInt(m_fCooked), RZ_Util.RoundToInt(COOK_FOR));
	}

	//------------------------------------------------------------------------------------------------
	override string Text()
	{
		return "Keep it at a simmer: more wood to bring it up, rake the fire down to calm it. The pot follows the fire slowly.\n\nStir it every few seconds or it catches on the bottom. Twenty seconds at a simmer, stirred, and it's done. Taste it if you're not sure.";
	}

	//------------------------------------------------------------------------------------------------
	override void Spots(notnull array<ref RZ_HandsOnSpot> spots)
	{
		RZ_HandsOnSpot stir = new RZ_HandsOnSpot();
		stir.m_sLabel = "Stir it";
		stir.m_sBone = "@view";
		stir.m_vOffset = Vector(-0.12, 0.12, 0.05);
		if (m_bDone)
			stir.m_iState = RZ_HandsOn.SPOT_SHUT;

		spots.Insert(stir);
		RZ_HandsOnSpot taste = new RZ_HandsOnSpot();
		taste.m_sLabel = "Taste it";
		taste.m_sBone = "@view";
		taste.m_vOffset = Vector(0.3, 0.2, 0.1);
		if (m_bDone)
			taste.m_iState = RZ_HandsOn.SPOT_SHUT;

		spots.Insert(taste);
	}

	//------------------------------------------------------------------------------------------------
	override void Log(notnull array<string> lines, notnull array<int> tones)
	{
		lines.Copy(m_aLog);
		tones.Copy(m_aTones);
	}

	//------------------------------------------------------------------------------------------------
	override void Actions(notnull array<string> labels)
	{
		if (m_bDone)
			return;

		labels.Insert("More wood");
		labels.Insert("Rake the fire down");
	}

	//------------------------------------------------------------------------------------------------
	//! The places: stir (the spoon round the bottom), taste (what it wants, in a word).
	override void Look(int spot, int playerId)
	{
		if (m_bDone || playerId != m_iPlayer)
			return;

		if (spot == 0)
		{
			m_fSinceStir = 0;
			if (m_bCaught)
			{
				m_bCaught = false;
				Say("You scrape it off the bottom, most of it.", RZ_HandsOn.TONE_ODD);
			}
			else if (Band() >= 2)
			{
				Say("Stirred. It moves the way it should.", RZ_HandsOn.TONE_CLEAN);
			}
			else
			{
				Say("Stirred. It's not doing much yet.", RZ_HandsOn.TONE_PLAIN);
			}

			return;
		}

		switch (Band())
		{
			case 0: Say("Cold. It wants fire under it.", RZ_HandsOn.TONE_ODD); break;
			case 1: Say("Warm, and raw. Give it more heat.", RZ_HandsOn.TONE_ODD); break;
			case 2: Say("That's it. Keep it like this.", RZ_HandsOn.TONE_CLEAN); break;
			default: Say("Scalding, and it tastes of the pan. Calm the fire.", RZ_HandsOn.TONE_BAD); break;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool Act(int index, int playerId)
	{
		if (m_bDone || playerId != m_iPlayer)
			return false;

		if (index == 0)
		{
			if (m_iFire >= FIRE_MAX)
			{
				Say("There's no room for more wood under it.", RZ_HandsOn.TONE_PLAIN);
				return false;
			}

			m_iFire++;
			Say("A split log under it. The fire takes.", RZ_HandsOn.TONE_PLAIN);
		}
		else if (index == 1)
		{
			if (m_iFire <= 0)
				return false;

			m_iFire--;
			Say("You rake the fire down.", RZ_HandsOn.TONE_PLAIN);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second while it's open: the pot follows the fire, and cooks, catches or boils over.
	override bool Tick(float dt)
	{
		if (m_bDone)
			return false;

		float want = m_iFire * 25;
		m_fHeat = m_fHeat + (want - m_fHeat) * 0.2 * dt;
		m_fSinceStir += dt;
		int band = Band();
		if (band != m_iLastBand)
		{
			if (band == 2 && m_iLastBand >= 0)
				Say("It's come to a simmer.", RZ_HandsOn.TONE_CLEAN);
			else if (band == 3)
				Say("It's boiling hard. Too much fire.", RZ_HandsOn.TONE_ODD);
			else if (band == 1 && m_iLastBand >= 2)
				Say("It's gone off the boil.", RZ_HandsOn.TONE_ODD);

			m_iLastBand = band;
		}

		// Too hot: it boils over, and a third time puts the fire half out
		if (band == 4)
		{
			if (!m_bOver)
			{
				m_bOver = true;
				m_iSpills++;
				Say("It boils over. Hissing on the fire.", RZ_HandsOn.TONE_BAD);
				if (m_iSpills >= 3)
				{
					m_iFire = 1;
					m_fHeat = 40;
					Say("The fire's half drowned. Build it up again.", RZ_HandsOn.TONE_BAD);
				}
			}
		}
		else
		{
			m_bOver = false;
		}

		// Hot and not stirred: it catches, then burns through and is started again
		if (band >= 2 && m_fSinceStir > CATCH_AFTER && !m_bCaught)
		{
			m_bCaught = true;
			Say("It's catching on the bottom. Stir it.", RZ_HandsOn.TONE_ODD);
		}

		if (band >= 2 && m_fSinceStir > BURN_AFTER)
		{
			m_fCooked = 0;
			m_fSinceStir = 0;
			m_bCaught = false;
			Say("Burnt through. You scrape it out and start it again.", RZ_HandsOn.TONE_BAD);
			return true;
		}

		// A simmer, stirred lately, and nothing stuck: it cooks
		if (band == 2 && !m_bCaught)
			m_fCooked += dt;

		if (m_fCooked >= COOK_FOR)
		{
			m_bDone = true;
			Say("It's done. Thick, and nothing stuck. Go and tell the cook.", RZ_HandsOn.TONE_CLEAN);
			RZ_WorkSites sites = RZ_WorkSites.Get();
			if (sites)
				sites.Complete(m_sKey, m_iPlayer);

			RZ_Log.Info(string.Format("Stove: %1 cooked the pot (%2 boil-overs).", RZ_Util.PlayerName(m_iPlayer), m_iSpills));
		}

		return true;
	}
}
