//! Sound and light (docs/TRACES.md, "Sound and light"): what the Zone sounds and looks like after
//! dark, near players only, all from the kit (World > Traces > Kit), all optional.
//!
//! | S1 | Lanterns where people live: a lit lantern at the 4 lived-in places nearest players, 20:30-05:00 |
//! |    | (from 18:30 in rain or fog); never in a town nobody holds                                        |
//! | S2 | Lit windows (World > Traces > Lit windows, off): client lights inside lived-in doors             |
//! | S3 | A radio in an empty house: the hiss loops (2 at most); the broadcast's line said within 12 m     |
//! | S4 | Dogs, then nothing: 5 minutes before a pack's hunt sets out, at a town by its lair             |
//! | S5 | A generator's hum at the lab and Halcyon's outposts                                              |
//! | S6 | One stroke of the bell at the Day 4 church, 18:30 on an evening a Remembered name is read        |
//! | S7 | The town loudspeaker at Bastion's holdings, 05:30 and 17:30; silent after the cure              |
//! | S8 | Candle jars lit on the church step on the bell's evening, and at a walk-out's chair             |
//!
//! Lights: 12 on the server within 200 m of a player. Loops: 4, within 80 m, gone past 120 m.
//! One-shots: 4 at once, removed after 40 s. Every prop through the dressing guard. The lines come
//! without any prefab (the Hunter's dogs, the bell, the loudspeaker). Server only, but for
//! RZ_AmbientLoopComponent, which plays on clients.

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "ReforgerZ", description: "A looping sound on a look-only prop (Traces: the radio's hiss, a generator's hum): plays its sound event when the entity appears on a client and stops it when the entity goes (docs/WORKBENCH_SETUP.md).")]
class RZ_AmbientLoopComponentClass : ScriptComponentClass
{
}

//! On the loop prefabs: the server spawns the prefab near players and deletes it when nobody is
//! near; each client that streams it in starts the looped event, and stops it on delete.
class RZ_AmbientLoopComponent : ScriptComponent
{
	[Attribute("SOUND_LOOP", desc: "The SoundComponent's looped event to start when the entity appears")]
	string m_sEvent;

	protected AudioHandle m_Handle = AudioHandle.Invalid;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		// Only where someone can hear it (not on a dedicated server)
		if (System.IsConsoleApp())
			return;

		GetGame().GetCallqueue().CallLater(Play, 100, false);
	}

	//------------------------------------------------------------------------------------------------
	//! VERIFY: a looped event started with SoundEvent on a client-side copy plays until Terminate.
	protected void Play()
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		SoundComponent sound = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		if (sound)
			m_Handle = sound.SoundEvent(m_sEvent);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		GetGame().GetCallqueue().Remove(Play);
		if (owner && m_Handle != AudioHandle.Invalid)
		{
			SoundComponent sound = SoundComponent.Cast(owner.FindComponent(SoundComponent));
			if (sound)
				sound.Terminate(m_Handle);
		}

		super.OnDelete(owner);
	}
}

//------------------------------------------------------------------------------------------------
//! One lit or looping thing up near players.
class RZ_TraceLook
{
	string m_sKey;
	bool m_bLoop;
	IEntity m_Entity;
}

//------------------------------------------------------------------------------------------------
class RZ_TraceAmbience
{
	protected static const int MAX_LIGHTS = 12;
	protected static const int MAX_LOOPS = 4;
	protected static const int MAX_RADIOS = 2;
	protected static const int MAX_SHOTS = 4;
	protected static const int LANTERN_PLACES = 4;
	protected static const float LIGHT_NEAR = 200;
	protected static const float LIGHT_FAR = 250;
	protected static const float LOOP_NEAR = 80;
	protected static const float LOOP_FAR = 120;
	protected static const float SHOT_SECONDS = 40;
	protected static const float DOGS_REACH = 800;
	protected static const float BELL_REACH = 1200;
	protected static const float SPEAKER_REACH = 300;
	protected static const float AGAIN_SECONDS = 1800;	// a place's one-shot fires once a window

	protected RZ_GameModeComponent m_GM;
	protected RZ_Traces m_Traces;
	protected ref array<ref RZ_TraceLook> m_aUp = {};
	protected ref map<string, vector> m_mSpot = new map<string, vector>();	// a place's lantern or hum, once a run
	protected ref array<IEntity> m_aShots = {};
	protected ref array<float> m_aShotUntil = {};
	protected ref map<string, float> m_mFired = new map<string, float>();	// "dogs|tribe", "speaker|holding" -> clock
	protected float m_fClock;
	protected bool m_bBellPending;		// a Remembered name was read: the bell at the next 18:30
	protected bool m_bJarsTonight;		// the bell rang: the church's jars until 22:00
	protected int m_iWindows;

	//------------------------------------------------------------------------------------------------
	void RZ_TraceAmbience(RZ_GameModeComponent gm, RZ_Traces traces)
	{
		m_GM = gm;
		m_Traces = traces;
	}

	//------------------------------------------------------------------------------------------------
	protected bool On()
	{
		return m_Traces.IsOn() && m_Traces.GetSettings().m_bSoundLight;
	}

	//------------------------------------------------------------------------------------------------
	int Lights()
	{
		return CountUp(false) + m_iWindows;
	}

	//------------------------------------------------------------------------------------------------
	int Loops()
	{
		return CountUp(true);
	}

	//------------------------------------------------------------------------------------------------
	protected int CountUp(bool loop)
	{
		int n = 0;
		foreach (RZ_TraceLook l : m_aUp)
		{
			if (l.m_bLoop == loop && l.m_Entity)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! P3b: a Remembered name is read tonight: one stroke of the bell at the next 18:30.
	void RingBell()
	{
		m_bBellPending = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Every 5 s (Traces' slot tick): what should be lit or looping near players, and the one-shots.
	void Tick(float dt, array<int> ids, array<IEntity> players)
	{
		m_fClock += dt;
		ShotsTick();
		if (!On())
		{
			Clear();
			return;
		}

		array<string> keys = {};
		array<vector> spots = {};
		array<ResourceName> prefabs = {};
		array<bool> loops = {};
		float hour = RZ_GameTime.GetHour();
		if (LanternHours(hour))
			Lanterns(players, keys, spots, prefabs, loops);

		Jars(hour, keys, spots, prefabs, loops);
		Radios(players, keys, spots, prefabs, loops);
		Hums(players, keys, spots, prefabs, loops);
		Sync(players, keys, spots, prefabs, loops);
		Windows(players, hour);
		Dogs(ids, players);
		Bell(ids, players, hour);
		Speakers(ids, players, hour);
	}

	//------------------------------------------------------------------------------------------------
	//! 20:30-05:00, from 18:30 in rain or fog.
	protected static bool LanternHours(float hour)
	{
		if (hour < 0)
			return false;

		if (RZ_GameTime.IsNightHour(hour))
			return true;

		return hour >= 18.5 && (RZ_Weather.Rain() >= 0.3 || RZ_Weather.Fog() >= 0.5);
	}

	//------------------------------------------------------------------------------------------------
	//! A place's spot, worked out once a run so it doesn't wander.
	protected vector SpotFor(string key, vector center, float minR, float maxR)
	{
		vector at;
		if (m_mSpot.Find(key, at))
			return at;

		at = RZ_Scenes.Ground(center, minR, maxR);
		m_mSpot.Set(key, at);
		return at;
	}

	//------------------------------------------------------------------------------------------------
	//! S1: the lived-in places nearest players (holdings, safehouses with people), 4 at most.
	protected void Lanterns(array<IEntity> players, array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<bool> loops)
	{
		ResourceName lantern = m_Traces.GetKit().m_sLantern;
		if (lantern.IsEmpty() || players.IsEmpty())
			return;

		array<string> placeKeys = {};
		array<vector> centers = {};
		array<float> dists = {};
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			foreach (RZ_Holding h : territory.GetHoldings())
			{
				Consider("lantern.h" + h.m_iId.ToString(), h.m_vPos, players, placeKeys, centers, dists);
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (houses)
		{
			for (int i = 0; i < houses.Count(); i++)
			{
				RZ_Safehouse sh = houses.Get(i);
				if (sh && sh.m_iPopulation > 0)
					Consider("lantern.s" + i.ToString(), sh.m_vPos, players, placeKeys, centers, dists);
			}
		}

		for (int n = 0; n < LANTERN_PLACES && !placeKeys.IsEmpty(); n++)
		{
			int best = 0;
			for (int j = 1; j < dists.Count(); j++)
			{
				if (dists[j] < dists[best])
					best = j;
			}

			keys.Insert(placeKeys[best]);
			spots.Insert(SpotFor(placeKeys[best], centers[best], 5, 12));
			prefabs.Insert(lantern);
			loops.Insert(false);
			placeKeys.Remove(best);
			centers.Remove(best);
			dists.Remove(best);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static void Consider(string key, vector center, array<IEntity> players, array<string> placeKeys, array<vector> centers, array<float> dists)
	{
		float d = RZ_Util.NearestDistance(center, players);
		if (d > LIGHT_NEAR)
			return;

		placeKeys.Insert(key);
		centers.Insert(center);
		dists.Insert(d);
	}

	//------------------------------------------------------------------------------------------------
	//! S8: the church's jars on the bell's evening, and a walk-out's chair its first two evenings,
	//! dusk to 22:00.
	protected void Jars(float hour, array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<bool> loops)
	{
		if (hour < 18.5 || hour >= 22)
		{
			if (hour >= 22 || (hour >= 0 && hour < 18.5))
				m_bJarsTonight = false;

			return;
		}

		ResourceName jars = m_Traces.GetKit().m_sCandleJars;
		if (jars.IsEmpty())
			return;

		float today = RZ_SeasonClock.Days();
		foreach (RZ_Scene s : m_Traces.GetScenes().GetScenes())
		{
			bool lit = (s.kind == "F4" && m_bJarsTonight) || (s.kind == "V3" && today - s.made < 2);
			if (!lit)
				continue;

			keys.Insert("jars." + s.key);
			spots.Insert(RZ_Util.GroundPos(RZ_Scenes.Pos(s) + Vector(1.2, 0, 0.6)));
			prefabs.Insert(jars);
			loops.Insert(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! S3: the hiss at the two radio scenes nearest a player, within 80 m.
	protected void Radios(array<IEntity> players, array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<bool> loops)
	{
		ResourceName hiss = m_Traces.GetKit().m_sRadioHiss;
		if (hiss.IsEmpty() || players.IsEmpty())
			return;

		int n = 0;
		foreach (RZ_Scene s : m_Traces.GetScenes().GetScenes())
		{
			if (n >= MAX_RADIOS)
				break;

			if (!RZ_Scenes.HasRadio(s) || RZ_Util.NearestDistance(RZ_Scenes.Pos(s), players) > LOOP_FAR)
				continue;

			keys.Insert("radio." + s.key);
			spots.Insert(RZ_Util.GroundPos(RZ_Scenes.Pos(s) + Vector(0.8, 0, -0.5)));
			prefabs.Insert(hiss);
			loops.Insert(true);
			n++;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! S5: a generator's hum at the lab and at Halcyon's outposts.
	protected void Hums(array<IEntity> players, array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<bool> loops)
	{
		ResourceName hum = m_Traces.GetKit().m_sGeneratorHum;
		if (hum.IsEmpty() || players.IsEmpty())
			return;

		if (m_GM.HasLab() && RZ_Util.NearestDistance(m_GM.GetLabPosition(), players) <= LOOP_FAR)
		{
			keys.Insert("hum.lab");
			spots.Insert(SpotFor("hum.lab", m_GM.GetLabPosition(), 12, 25));
			prefabs.Insert(hum);
			loops.Insert(true);
		}

		RZ_Territory territory = m_GM.GetTerritory();
		if (!territory || !territory.IsActive())
			return;

		array<RZ_Holding> stations = {};
		territory.GetOutposts(RZ_EFaction.INSTITUTE, stations);
		foreach (RZ_Holding h : stations)
		{
			if (RZ_Util.NearestDistance(h.m_vPos, players) > LOOP_FAR)
				continue;

			string key = "hum.h" + h.m_iId.ToString();
			keys.Insert(key);
			spots.Insert(SpotFor(key, h.m_vPos, 10, 20));
			prefabs.Insert(hum);
			loops.Insert(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Up what's wanted within reach (12 lights, 4 loops), down what isn't or nobody is near.
	protected void Sync(array<IEntity> players, array<string> keys, array<vector> spots, array<ResourceName> prefabs, array<bool> loops)
	{
		for (int i = m_aUp.Count() - 1; i >= 0; i--)
		{
			RZ_TraceLook l = m_aUp[i];
			int at = keys.Find(l.m_sKey);
			float far = LIGHT_FAR;
			if (l.m_bLoop)
				far = LOOP_FAR;

			if (l.m_Entity && at >= 0 && RZ_Util.NearestDistance(l.m_Entity.GetOrigin(), players) <= far)
				continue;

			Delete(l.m_Entity);
			m_aUp.Remove(i);
		}

		foreach (int k, string key : keys)
		{
			if (IsUp(key))
				continue;

			float near = LIGHT_NEAR;
			int cap = MAX_LIGHTS;
			if (loops[k])
			{
				near = LOOP_NEAR;
				cap = MAX_LOOPS;
			}

			if (CountUp(loops[k]) >= cap || RZ_Util.NearestDistance(spots[k], players) > near)
				continue;

			IEntity e = SpawnLook(prefabs[k], spots[k]);
			if (!e)
				continue;

			RZ_TraceLook look = new RZ_TraceLook();
			look.m_sKey = key;
			look.m_bLoop = loops[k];
			look.m_Entity = e;
			m_aUp.Insert(look);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsUp(string key)
	{
		foreach (RZ_TraceLook l : m_aUp)
		{
			if (l.m_sKey == key)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A look-only prop: through the dressing guard and marked as scenery. Null if the guard took it.
	protected IEntity SpawnLook(ResourceName prefab, vector pos)
	{
		IEntity e = RZ_Util.SpawnPrefab(prefab, RZ_Util.GroundPos(pos), Math.RandomFloat(0, 360));
		if (!e)
			return null;

		if (!RZ_DressingGuard.Strip(e, prefab))
			return null;

		RZ_CampLife.AddDressing(e);
		return e;
	}

	//------------------------------------------------------------------------------------------------
	protected static void Delete(IEntity e)
	{
		if (!e)
			return;

		RZ_CampLife.RemoveDressing(e);
		SCR_EntityHelper.DeleteEntityAndChildren(e);
	}

	//------------------------------------------------------------------------------------------------
	//! Traces off, or sound and light off: everything comes down.
	void Clear()
	{
		foreach (RZ_TraceLook l : m_aUp)
		{
			Delete(l.m_Entity);
		}

		m_aUp.Clear();
		m_iWindows = 0;
		m_GM.SetTraceLights("");
	}

	// ================================================================ S2: lit windows
	//------------------------------------------------------------------------------------------------
	//! With World > Traces > Lit windows on, in lantern hours: a point of light 1.5 m inside the
	//! outside doors of lived-in buildings near players, 12 at most, for the clients to draw
	//! (UI/RZ_TraceLightsClient.c).
	protected void Windows(array<IEntity> players, float hour)
	{
		if (!m_Traces.GetSettings().m_bLitWindows || !LanternHours(hour) || players.IsEmpty())
		{
			m_iWindows = 0;
			m_GM.SetTraceLights("");
			return;
		}

		string list = "";
		int n = 0;
		RZ_WorldFacts facts = RZ_WorldFacts.Get();
		foreach (int i, RZ_Settlement s : m_GM.GetSettlements())
		{
			if (n >= MAX_LIGHTS)
				break;

			if (!facts.IsLivedIn(i) || RZ_Util.NearestDistance(s.m_vCenter, players) > s.m_fRadius + LIGHT_NEAR)
				continue;

			RZ_TownSlots slots = m_Traces.GetSlots().Get(i);
			if (!slots)
				continue;

			foreach (RZ_DoorSlot d : slots.m_aSlots)
			{
				if (n >= MAX_LIGHTS)
					break;

				if (!d.m_bOff || !facts.IsNearLivedIn(d.m_vBuilding, 60) || RZ_Util.NearestDistance(d.m_vPos, players) > LIGHT_NEAR)
					continue;

				vector p = d.m_vPos - d.m_vNormal * 1.5;
				list += string.Format("%1 %2 %3;", RZ_Util.RoundToInt(p[0] * 10), RZ_Util.RoundToInt(p[1] * 10), RZ_Util.RoundToInt(p[2] * 10));
				n++;
			}
		}

		m_iWindows = n;
		m_GM.SetTraceLights(list);
	}

	// ================================================================ One-shots
	//------------------------------------------------------------------------------------------------
	protected void Shot(ResourceName prefab, vector pos)
	{
		if (prefab.IsEmpty() || m_aShots.Count() >= MAX_SHOTS)
			return;

		IEntity e = SpawnLook(prefab, RZ_Util.GroundPos(pos) + Vector(0, 2, 0));
		if (!e)
			return;

		m_aShots.Insert(e);
		m_aShotUntil.Insert(m_fClock + SHOT_SECONDS);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShotsTick()
	{
		for (int i = m_aShots.Count() - 1; i >= 0; i--)
		{
			if (m_aShots[i] && m_fClock < m_aShotUntil[i])
				continue;

			Delete(m_aShots[i]);
			m_aShots.Remove(i);
			m_aShotUntil.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Once a window: true the first time `key` asks after AGAIN_SECONDS.
	protected bool FireOnce(string key)
	{
		float last;
		if (m_mFired.Find(key, last) && m_fClock - last < AGAIN_SECONDS)
			return false;

		m_mFired.Set(key, m_fClock);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! S4: 5 real minutes before a pack's hunt sets out, the dogs at the town nearest its lair
	//! (within 1.5 km) bark and stop. A Hunter within 800 m hears them stop.
	protected void Dogs(array<int> ids, array<IEntity> players)
	{
		RZ_Tribes tribes = m_GM.GetTribes();
		if (!tribes || players.IsEmpty() || RZ_WorldFacts.Get().IsCured())
			return;

		foreach (RZ_Tribe t : tribes.GetTribeList())
		{
			if (!t || t.IsRover())
				continue;

			float until = RZ_GameTime.RealMinutesUntil(t.m_fSetOutHour);
			if (until < 0 || until > 5)
				continue;

			int town = m_GM.FindNearestSettlement(t.m_vLair);
			RZ_Settlement s = m_GM.GetSettlement(town);
			if (!s || RZ_Util.DistXZ(s.m_vCenter, t.m_vLair) > s.m_fRadius + 1500 || RZ_WorldFacts.Get().IsLivedIn(town))
				continue;

			if (RZ_Util.NearestDistance(s.m_vCenter, players) > DOGS_REACH || !FireOnce("dogs|" + t.m_iId.ToString()))
				continue;

			vector toward = t.m_vLair - s.m_vCenter;
			toward[1] = 0;
			toward.Normalize();
			Shot(m_Traces.GetKit().m_sDogs, s.m_vCenter + toward * s.m_fRadius);
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), s.m_vCenter) <= DOGS_REACH && RZ_TraceReader.For(m_GM, ids[i]).Is("hunter"))
					m_GM.NotifyPlayerAs(ids[i], "Dogs", RZ_SceneText.Dogs(s.m_sName), RZ_ENoticeTag.LOW);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! S6: at 18:30 after a Remembered name, one stroke at the Day 4 church: 10 heat there, the line
	//! to everyone within 1.2 km. Never at night.
	protected void Bell(array<int> ids, array<IEntity> players, float hour)
	{
		if (!m_bBellPending || hour < 18.5 || hour >= 18.75)
			return;

		m_bBellPending = false;
		m_bJarsTonight = true;
		vector church = RZ_WorldFacts.Get().CanonPos("day4.church");
		if (church == vector.Zero)
			return;

		if (m_GM.GetHeat())
			m_GM.GetHeat().AddHeat(church, 10);

		if (RZ_Util.NearestDistance(church, players) > BELL_REACH)
			return;

		Shot(m_Traces.GetKit().m_sBell, church);
		string line = RZ_SceneText.Bell();
		foreach (int i, IEntity p : players)
		{
			if (RZ_Util.DistXZ(p.GetOrigin(), church) <= BELL_REACH)
				m_GM.NotifyPlayerAs(ids[i], "A bell", line, RZ_ENoticeTag.LOW);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! S7: at 05:30 and 17:30, Bastion's holdings: a chime and the line to everyone within 300 m.
	//! Silent after the cure.
	protected void Speakers(array<int> ids, array<IEntity> players, float hour)
	{
		bool morning = hour >= 5.5 && hour < 5.75;
		bool evening = hour >= 17.5 && hour < 17.75;
		RZ_Territory territory = m_GM.GetTerritory();
		if ((!morning && !evening) || !territory || !territory.IsActive() || RZ_WorldFacts.Get().IsCured() || players.IsEmpty())
			return;

		string line = RZ_SceneText.Loudspeaker(RZ_WorldFacts.Get().BastionPresence() >= 0.25);
		foreach (RZ_Holding h : territory.GetHoldings())
		{
			if (h.m_eFaction != RZ_EFaction.MILITARY || RZ_Util.NearestDistance(h.m_vPos, players) > SPEAKER_REACH)
				continue;

			if (!FireOnce("speaker|" + h.m_iId.ToString()))
				continue;

			Shot(m_Traces.GetKit().m_sLoudspeaker, h.m_vPos);
			foreach (int i, IEntity p : players)
			{
				if (RZ_Util.DistXZ(p.GetOrigin(), h.m_vPos) > SPEAKER_REACH)
					continue;

				m_GM.Say(ids[i], "Loudspeaker", RZ_EFaction.MILITARY, line);
				if (RZ_TraceReader.For(m_GM, ids[i]).Is("soldier"))
					m_GM.NotifyPlayerAs(ids[i], "Loudspeaker", RZ_SceneText.LoudspeakerMore(), RZ_ENoticeTag.LOW);
			}
		}
	}
}
