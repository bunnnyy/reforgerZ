//! The kinds' attacks that need the player's own machine (docs/INFECTED.md, "Attacks"): a slow
//! one's ankle grab and hold, the young's stagger after a leap, and the big one's knockdown at the
//! end of a charge. The server decides every one (here, once a second and on each hit the bite
//! detector sees); the player's client plays it (RZ_PlayerEffectsComponent on the player
//! controller), because a player's movement is predicted on their own machine. Effects are status,
//! never extra damage, and all of them share the behaviour layer's one special move per player per
//! Special move gap, never in the gentle start. Without the component the effects stay off; the
//! lines and the built grab chance stay.

//------------------------------------------------------------------------------------------------
//! One hold on a player: who holds them, and until when.
class RZ_Hold
{
	int m_iPlayer;
	IEntity m_Body;		// weak: the one holding
	float m_fUntil;
}

//------------------------------------------------------------------------------------------------
class RZ_AttackEffects
{
	protected RZ_GameModeComponent m_GM;
	protected RZ_Behaviour m_BH;
	protected ref array<ref RZ_Hold> m_aHolds = {};
	protected ref map<int, float> m_mHoldEndedAt = new map<int, float>();	// per player: the last hold ended
	protected ref map<int, float> m_mStaggerAt = new map<int, float>();
	protected ref map<int, float> m_mDownAt = new map<int, float>();
	protected ref map<int, float> m_mToldAt = new map<int, float>();		// per player: the last line
	protected ref array<IEntity> m_aGrabbers = {};		// weak: slow ones that grabbed lately
	protected ref array<float> m_aGrabReady = {};
	protected int m_iHolds;
	protected int m_iKickedFree;
	protected int m_iStaggers;
	protected int m_iKnockdowns;

	static const float GRAB_REACH = 2;			// m: the slow one this close to the player
	static const float GRAB_COOLDOWN = 20;		// s per slow one
	static const float GRAB_GAP = 5;			// s: never two holds in a row from different ones
	static const float HOLD_CAP = 0.25;			// the player's speed while held
	static const float HOLD_LEAVE = 3;			// m: the one holding is gone this far (or dead): the hold ends
	static const float STAGGER_SECONDS = 0.8;
	static const float STAGGER_CAP = 0.6;
	static const float STAGGER_GAP = 6;			// s per player
	static const float DOWN_SECONDS = 1.2;
	static const float DOWN_GAP = 30;			// s per player
	static const float LINE_GAP = 45;			// s: a player hears one line of these at most this often

	//------------------------------------------------------------------------------------------------
	void RZ_AttackEffects(RZ_GameModeComponent gm, RZ_Behaviour bh)
	{
		m_GM = gm;
		m_BH = bh;
	}

	//------------------------------------------------------------------------------------------------
	//! Zombies > Behaviour > Own attacks and Client effects both on.
	protected bool IsOn()
	{
		RZ_BehaviourSettings s = m_BH.GetSettings();
		return s && s.m_bOwnAttacks && s.m_bClientEffects;
	}

	//------------------------------------------------------------------------------------------------
	//! The player is held right now (a bite in a hold counts as grabbed: RZ_BiteDetector).
	bool IsHeld(int playerId)
	{
		foreach (RZ_Hold h : m_aHolds)
		{
			if (h.m_iPlayer == playerId)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Once a second: a hold ends early when the one holding is dead or gone, or the player is.
	void Tick(array<int> ids, array<IEntity> players)
	{
		float now = RZ_BaconBody.Now();
		for (int i = m_aHolds.Count() - 1; i >= 0; i--)
		{
			RZ_Hold h = m_aHolds[i];
			if (now >= h.m_fUntil)
			{
				EndHold(i, false);
				continue;
			}

			int at = ids.Find(h.m_iPlayer);
			if (at < 0)
			{
				EndHold(i, false);
				continue;
			}

			if (!h.m_Body || !RZ_Util.IsCharacterAlive(h.m_Body) || RZ_Util.DistXZ(h.m_Body.GetOrigin(), players[at].GetOrigin()) > HOLD_LEAVE)
				EndHold(i, true);
		}

		for (int g = m_aGrabbers.Count() - 1; g >= 0; g--)
		{
			if (!m_aGrabbers[g] || now >= m_aGrabReady[g])
			{
				m_aGrabbers.Remove(g);
				m_aGrabReady.Remove(g);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void EndHold(int i, bool tellClient)
	{
		RZ_Hold h = m_aHolds[i];
		m_mHoldEndedAt.Set(h.m_iPlayer, RZ_BaconBody.Now());
		m_aHolds.Remove(i);
		if (!tellClient)
			return;

		RZ_PlayerEffectsComponent fx = RZ_PlayerEffectsComponent.GetFor(h.m_iPlayer);
		if (fx)
			fx.SendClear();
	}

	//------------------------------------------------------------------------------------------------
	//! The client says its player kicked free (RZ_PlayerEffectsComponent): the hold ends here too.
	void OnKickedFree(int playerId)
	{
		for (int i = m_aHolds.Count() - 1; i >= 0; i--)
		{
			if (m_aHolds[i].m_iPlayer != playerId)
				continue;

			// Kicked free: the one holding lets go, and can't grab again at once
			IEntity body = m_aHolds[i].m_Body;
			EndHold(i, false);
			m_iKickedFree++;
			if (body)
				Cooldown(body, RZ_BaconBody.Now() + GRAB_COOLDOWN);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A player was hit (the bite detector, through RZ_Behaviour.OnPlayerHit), with no special move
	//! landed this gap: a slow one's ankle grab, or a young's stagger after its leap. True if one landed
	//! (the caller marks the special move).
	bool OnHit(int playerId, IEntity player, float now)
	{
		if (!IsOn() || !player || RZ_Util.IsInVehicle(player) || Gentle(player))
			return false;

		if (TryGrab(playerId, player, now))
			return true;

		return TryStagger(playerId, player, now);
	}

	//------------------------------------------------------------------------------------------------
	//! In a player's gentle start: no special moves (RZ_Guidance).
	protected bool Gentle(IEntity player)
	{
		RZ_Guidance guide = m_GM.GetGuidance();
		return guide && guide.GetDensityMult(player.GetOrigin(), 5) < 1;
	}

	// ================================================================ The ankle grab
	//------------------------------------------------------------------------------------------------
	//! A slow one within 2 m that's low (hunched, stance 0.7 or less) or the player crouched or prone:
	//! held, the speed capped at 0.25 for Hold seconds, until they kick free. 20 s per slow one, one
	//! hold on a player at a time, never two in a row from different ones within 5 s.
	protected bool TryGrab(int playerId, IEntity player, float now)
	{
		if (IsHeld(playerId))
			return false;

		float ended;
		if (m_mHoldEndedAt.Find(playerId, ended) && now - ended < GRAB_GAP)
			return false;

		IEntity body = FindSlowOne(player.GetOrigin(), now);
		if (!body)
			return false;

		if (RZ_Senses.GetStance(player) < 1 && !IsLow(body))
			return false;

		RZ_BehaviourSettings s = m_BH.GetSettings();
		float seconds = Math.Max(0.5, s.m_fHoldSeconds);
		Cooldown(body, now + GRAB_COOLDOWN);
		RZ_PlayerEffectsComponent fx = RZ_PlayerEffectsComponent.GetFor(playerId);
		if (fx)
		{
			RZ_Hold h = new RZ_Hold();
			h.m_iPlayer = playerId;
			h.m_Body = body;
			h.m_fUntil = now + seconds;
			m_aHolds.Insert(h);
			fx.SendHold(seconds, HOLD_CAP, Math.Max(1, s.m_iHoldKicks));
		}

		m_iHolds++;
		Tell(playerId, now, "Something has your ankle.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The nearest slow one (a town's own) within reach of the player that isn't cooling down.
	protected IEntity FindSlowOne(vector at, float now)
	{
		IEntity best;
		float bestDist = GRAB_REACH;
		array<ref RZ_TrackedZombie> tracked = m_GM.GetSpawner().GetTracked();
		foreach (RZ_TrackedZombie z : tracked)
		{
			if (z.m_eRole != RZ_EInfectedRole.CRAWLER || !z.m_aMembers || RZ_Util.DistXZ(z.m_vLastPos, at) > 80)
				continue;

			foreach (IEntity m : z.m_aMembers)
			{
				if (!m || !RZ_Util.IsCharacterAlive(m) || CoolingDown(m, now))
					continue;

				float d = RZ_Util.DistXZ(m.GetOrigin(), at);
				if (d <= bestDist)
				{
					bestDist = d;
					best = m;
				}
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A body hunched low (its dynamic stance 0.7 or less: crawler life's 0.6 when lying up).
	protected static bool IsLow(IEntity body)
	{
		ChimeraCharacter c = ChimeraCharacter.Cast(body);
		if (!c)
			return false;

		CharacterControllerComponent ctrl = c.GetCharacterController();
		return ctrl && ctrl.GetDynamicStance() <= 0.7;
	}

	//------------------------------------------------------------------------------------------------
	protected bool CoolingDown(IEntity body, float now)
	{
		int at = m_aGrabbers.Find(body);
		return at >= 0 && now < m_aGrabReady[at];
	}

	//------------------------------------------------------------------------------------------------
	protected void Cooldown(IEntity body, float until)
	{
		int at = m_aGrabbers.Find(body);
		if (at >= 0)
		{
			m_aGrabReady[at] = until;
			return;
		}

		m_aGrabbers.Insert(body);
		m_aGrabReady.Insert(until);
	}

	// ================================================================ The stagger
	//------------------------------------------------------------------------------------------------
	//! A young whose leap ended within 3 s and 2.5 m of the player hit them: staggered, the speed
	//! capped at 0.6 for 0.8 s. At most once per player per 6 s.
	protected bool TryStagger(int playerId, IEntity player, float now)
	{
		float last;
		if (m_mStaggerAt.Find(playerId, last) && now - last < STAGGER_GAP)
			return false;

		if (!m_BH.FindLeaper(player.GetOrigin(), now))
			return false;

		m_mStaggerAt.Set(playerId, now);
		m_iStaggers++;
		RZ_PlayerEffectsComponent fx = RZ_PlayerEffectsComponent.GetFor(playerId);
		if (fx)
			fx.SendStagger(STAGGER_SECONDS, STAGGER_CAP);

		Tell(playerId, now, "It came off the run and hit you full on. You nearly went over.");
		return true;
	}

	// ================================================================ The knockdown
	//------------------------------------------------------------------------------------------------
	//! A big one's charge reached its target (within 1.6 m; RZ_Behaviour.TryCharge): forced prone, no
	//! movement for 1.2 s, then Bacon's swing. Never with 3 or more other infected within 10 m of the
	//! player, at most once per player per 30 s, and inside the one special move per gap.
	bool TryKnockdown(int playerId, IEntity player)
	{
		if (!IsOn() || !player || RZ_Util.IsInVehicle(player) || Gentle(player))
			return false;

		float now = RZ_BaconBody.Now();
		float last;
		if (m_mDownAt.Find(playerId, last) && now - last < DOWN_GAP)
			return false;

		if (!m_BH.SpecialFree(playerId, now))
			return false;

		// The charger itself is within 10 m: 4 or more counted means 3 others
		if (m_GM.GetSpawner().CountZombiesNear(player.GetOrigin(), 10, 4) >= 4)
			return false;

		m_mDownAt.Set(playerId, now);
		m_BH.MarkSpecial(playerId, now);
		m_iKnockdowns++;
		RZ_PlayerEffectsComponent fx = RZ_PlayerEffectsComponent.GetFor(playerId);
		if (fx)
			fx.SendKnockdown(DOWN_SECONDS);

		Tell(playerId, now, "It hit you like a car door and you were on the ground before you knew it. Get up.");
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! One line to the player, at most every 45 s.
	protected void Tell(int playerId, float now, string text)
	{
		float last;
		if (m_mToldAt.Find(playerId, last) && now - last < LINE_GAP)
			return;

		m_mToldAt.Set(playerId, now);
		m_GM.NotifyPlayer(playerId, "Close", text);
	}

	//------------------------------------------------------------------------------------------------
	//! The player left: drop what's kept for them.
	void Forget(int playerId)
	{
		for (int i = m_aHolds.Count() - 1; i >= 0; i--)
		{
			if (m_aHolds[i].m_iPlayer == playerId)
				m_aHolds.Remove(i);
		}

		m_mHoldEndedAt.Remove(playerId);
		m_mStaggerAt.Remove(playerId);
		m_mDownAt.Remove(playerId);
		m_mToldAt.Remove(playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! "holds 2 (kicked free 1), staggers 1, knockdowns 0"
	string StatusText()
	{
		return string.Format("holds %1 (kicked free %2), staggers %3, knockdowns %4", m_iHolds, m_iKickedFree, m_iStaggers, m_iKnockdowns);
	}
}
