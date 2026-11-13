//! The photo tour (docs/PC_SETUP.md, "Photos of the sites"): Workbench play only, and only while
//! $profile:RZ_PhotoTour.txt exists. It takes the local player round every holding, Haven safehouse
//! and a few road posts, waits 110 m off until camp life has laid the place out, then photographs
//! it from a camera of its own (from the south, the east, the west and straight down, then from
//! the ground at its two clearest sides) into $profile:RZ_Photos/. It can also lay out the composition groups of $profile:RZ_ShowList.txt and
//! photograph each. Nothing here is part of the game: the server never compiles it.
#ifdef WORKBENCH
//------------------------------------------------------------------------------------------------
class RZ_PhotoStop
{
	string m_sName;
	vector m_vPos;
	bool m_bSite;					// wait for camp life's pieces before the photos
	ref array<string> m_aPrefabs;	// a showcase group: laid out, photographed and removed
	bool m_bClose;					// people and small things: 3 m apart, the camera close
}

//------------------------------------------------------------------------------------------------
class RZ_PhotoShot
{
	string m_sName;
	vector m_vEye;
	vector m_vAt;
}

//------------------------------------------------------------------------------------------------
class RZ_PhotoTour
{
	static const string LIST_PATH = "$profile:RZ_PhotoTour.txt";
	static const string SHOW_PATH = "$profile:RZ_ShowList.txt";
	static const string FOLDER = "$profile:RZ_Photos/";
	static const float STAND_OFF = 110;		// the player waits this far from a site, out of the picture
	static const float PIECE_RADIUS = 120;
	static const int SETTLE_SECONDS = 20;	// the piece count unchanged this long: it's laid out
	static const int SITE_TIMEOUT = 420;
	static const int EMPTY_TIMEOUT = 240;	// nothing at all has spawned: photograph the bare place
	static const int PLACE_TIMEOUT = 30;

	protected static const int S_WAIT = 0;
	protected static const int S_NEXT = 1;
	protected static const int S_SETTLE = 2;
	protected static const int S_SHOOT = 3;
	protected static const int S_DONE = 4;

	protected static ref RZ_PhotoTour s_Instance;

	protected RZ_GameModeComponent m_GM;
	protected ref array<string> m_aOptions = {};
	protected ref array<ref RZ_PhotoStop> m_aStops = {};
	protected ref array<ref RZ_PhotoShot> m_aShots = {};
	protected ref array<IEntity> m_aShown = {};
	protected ref array<string> m_aShownNames = {};
	protected CameraBase m_Camera;
	protected CameraBase m_PlayerCamera;
	protected int m_iState;
	protected int m_iStop = -1;
	protected int m_iShot;
	protected int m_iWait;
	protected int m_iStable;
	protected int m_iLastCount = -1;
	protected int m_iPhotos;
	protected float m_fHour = -1;
	protected string m_sOnly;		// "only <text>": just the stops whose name holds it
	protected bool m_bAround;		// "around": the ground photos from all eight compass points

	//------------------------------------------------------------------------------------------------
	//! From the game mode's start. Does nothing without the list file.
	static void Start(RZ_GameModeComponent gm)
	{
		s_Instance = null;
		if (!gm || !FileIO.FileExists(LIST_PATH))
			return;

		FileHandle file = FileIO.OpenFile(LIST_PATH, FileMode.READ);
		if (!file)
			return;

		array<string> lines = {};
		string line;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (!line.IsEmpty() && line.IndexOf("#") != 0)
				lines.Insert(line);
		}

		file.Close();
		s_Instance = new RZ_PhotoTour(gm, lines);
		GetGame().GetCallqueue().CallLater(s_Instance.Step, 1000, true);
		RZ_Log.Info("Photo tour: waiting for the player (" + LIST_PATH + " is there; delete it to play normally).");
	}

	//------------------------------------------------------------------------------------------------
	void RZ_PhotoTour(RZ_GameModeComponent gm, array<string> options)
	{
		m_GM = gm;
		foreach (string option : options)
		{
			m_aOptions.Insert(option);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Step()
	{
		if (m_iState == S_WAIT)
			WaitReady();
		else if (m_iState == S_NEXT)
			NextStop();
		else if (m_iState == S_SETTLE)
			Settle();
		else if (m_iState == S_SHOOT)
			Shoot();
	}

	//------------------------------------------------------------------------------------------------
	//! The player stands in the world and the holdings are rolled: build the list and set up.
	protected void WaitReady()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		m_iWait++;
		RZ_Territory territory = m_GM.GetTerritory();
		bool any = territory && !territory.GetHoldings().IsEmpty();
		if (m_iWait < 10 || (!any && m_iWait < 120))
			return;

		BuildStops(player.GetOrigin());
		if (m_fHour >= 0)
			SetHour(m_fHour, false);

		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (hud)
			hud.SetVisible(false);

		CameraManager cameras = GetGame().GetCameraManager();
		if (cameras)
			m_PlayerCamera = cameras.CurrentCamera();

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.MatrixIdentity4(params.Transform);
		params.Transform[3] = player.GetOrigin() + Vector(0, 30, 0);
		m_Camera = CameraBase.Cast(GetGame().SpawnEntity(CameraBase, GetGame().GetWorld(), params));
		if (m_Camera)
		{
			m_Camera.SetVerticalFOV(55);
			m_Camera.SetFarPlane(4000);
		}

		RZ_Log.Info(string.Format("Photo tour: %1 stops, photos go to %2", m_aStops.Count(), FOLDER));
		m_iState = S_NEXT;
	}

	//------------------------------------------------------------------------------------------------
	protected void BuildStops(vector start)
	{
		bool sites = m_aOptions.IsEmpty();
		int posts = 0;
		foreach (string option : m_aOptions)
		{
			array<string> bits = {};
			option.Split(" ", bits, true);
			if (bits.IsEmpty())
				continue;

			if (bits[0] == "sites")
				sites = true;
			else if (bits[0] == "posts" && bits.Count() >= 2)
				posts = bits[1].ToInt();
			else if (bits[0] == "noon")
				m_fHour = 12;
			else if (bits[0] == "only" && bits.Count() >= 2)
				m_sOnly = bits[1];
			else if (bits[0] == "around")
				m_bAround = true;
			else if (bits[0] == "hour" && bits.Count() >= 2)
				m_fHour = bits[1].ToFloat();
			else if (bits[0] == "show")
			{
				vector origin = start + Vector(0, 0, 60);
				if (bits.Count() >= 3)
					origin = Vector(bits[1].ToFloat(), 0, bits[2].ToFloat());

				AddShowGroups(RZ_Util.GroundPos(origin));
			}
			else if (bits[0] == "place" && bits.Count() >= 4)
			{
				RZ_PhotoStop place = new RZ_PhotoStop();
				place.m_sName = Clean(bits[1]);
				place.m_vPos = RZ_Util.GroundPos(Vector(bits[2].ToFloat(), 0, bits[3].ToFloat()));
				m_aStops.Insert(place);
			}
		}

		if (sites)
			AddSites();

		if (posts > 0)
			AddPosts(posts);

		if (!m_sOnly.IsEmpty())
		{
			for (int k = m_aStops.Count() - 1; k >= 0; k--)
			{
				// The sites named; a showcase group is always photographed
				if (m_aStops[k].m_bSite && !m_aStops[k].m_sName.Contains(m_sOnly))
					m_aStops.Remove(k);
			}
		}

		foreach (int i, RZ_PhotoStop stop : m_aStops)
		{
			int order = i + 1;
			string number = order.ToString();
			if (order < 10)
				number = "0" + number;

			stop.m_sName = number + "_" + stop.m_sName;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every holding (strongholds first) and Haven safehouse standing now.
	protected void AddSites()
	{
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory)
		{
			for (int pass = 0; pass < 2; pass++)
			{
				foreach (RZ_Holding h : territory.GetHoldings())
				{
					if (h.m_bStronghold != (pass == 0))
						continue;

					RZ_PhotoStop stop = new RZ_PhotoStop();
					stop.m_sName = Clean(h.GetTitle());
					stop.m_vPos = h.m_vPos;
					stop.m_bSite = true;
					m_aStops.Insert(stop);
				}
			}
		}

		RZ_Safehouses houses = m_GM.GetSafehouses();
		if (!houses)
			return;

		for (int i = 0; i < houses.Count(); i++)
		{
			RZ_Safehouse house = houses.Get(i);
			if (!house)
				continue;

			RZ_PhotoStop home = new RZ_PhotoStop();
			home.m_sName = Clean("Haven safehouse " + house.m_sName);
			home.m_vPos = house.m_vPos;
			home.m_bSite = true;
			m_aStops.Insert(home);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddPosts(int most)
	{
		RZ_Checkpoints checkpoints = m_GM.GetCheckpoints();
		if (!checkpoints)
			return;

		array<RZ_RoadPost> posts = {};
		checkpoints.PostsNear(-1, vector.Zero, 1000000, posts);
		foreach (int i, RZ_RoadPost post : posts)
		{
			if (i >= most)
				break;

			RZ_PhotoStop stop = new RZ_PhotoStop();
			stop.m_sName = Clean(post.GetTitle());
			stop.m_vPos = post.m_vPos;
			stop.m_bSite = true;
			m_aStops.Insert(stop);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The groups of $profile:RZ_ShowList.txt ("# Title" lines, ResourceNames under them; a title
	//! ending "@3" is people or small things), each laid out at origin in its turn.
	protected void AddShowGroups(vector origin)
	{
		FileHandle file = FileIO.OpenFile(SHOW_PATH, FileMode.READ);
		if (!file)
			return;

		RZ_PhotoStop group;
		string line;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (line.IsEmpty())
				continue;

			if (line.IndexOf("#") == 0)
			{
				string title = line.Substring(1, line.Length() - 1);
				group = new RZ_PhotoStop();
				group.m_aPrefabs = {};
				group.m_vPos = origin;
				int at = title.IndexOf("@");
				if (at > 0)
				{
					group.m_bClose = true;
					title = title.Substring(0, at);
				}

				title.Trim();
				group.m_sName = Clean(title);
				m_aStops.Insert(group);
				continue;
			}

			if (group)
				group.m_aPrefabs.Insert(line);
		}

		file.Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void NextStop()
	{
		ClearShown();
		m_iStop++;
		if (m_iStop >= m_aStops.Count())
		{
			Finish();
			return;
		}

		RZ_PhotoStop stop = m_aStops[m_iStop];
		m_iWait = 0;
		m_iStable = 0;
		m_iLastCount = -1;
		m_aShots.Clear();
		Protect();
		if (stop.m_aPrefabs)
		{
			MovePlayer(stop.m_vPos + Vector(0, 0, -90));
			LayOut(stop);
		}
		else
		{
			MovePlayer(StandSpot(stop.m_vPos));
		}

		RZ_Log.Info(string.Format("Photo tour: stop %1 of %2, %3 at %4 %5", m_iStop + 1, m_aStops.Count(), stop.m_sName, RZ_Util.RoundToInt(stop.m_vPos[0]), RZ_Util.RoundToInt(stop.m_vPos[2])));
		m_iState = S_SETTLE;
	}

	//------------------------------------------------------------------------------------------------
	//! Waits until the place is laid out (camp life's piece count stops changing), then plans the photos.
	protected void Settle()
	{
		RZ_PhotoStop stop = m_aStops[m_iStop];
		m_iWait++;
		if (stop.m_aPrefabs)
		{
			if (m_iWait < 6)
				return;

			PlanGroupShots(stop);
		}
		else
		{
			array<IEntity> pieces = {};
			RZ_CampLife.PiecesNear(stop.m_vPos, PIECE_RADIUS, pieces);
			int count = pieces.Count();
			if (count == m_iLastCount)
				m_iStable++;
			else
				m_iStable = 0;

			m_iLastCount = count;
			int limit = PLACE_TIMEOUT;
			if (stop.m_bSite)
			{
				limit = EMPTY_TIMEOUT;
				if (count > 0)
					limit = SITE_TIMEOUT;
			}

			// (a site still being read or solved has only its counters up: not ready, however still)
			bool ready = count > 0 && m_iStable >= SETTLE_SECONDS;
			RZ_CampLife camp = m_GM.GetCampLife();
			if (stop.m_bSite && camp && !camp.LaidOutNear(stop.m_vPos, PIECE_RADIUS))
				ready = false;

			if (!ready && m_iWait < limit)
				return;

			RZ_Log.Info(string.Format("Photo tour: %1 has %2 camp pieces after %3 s", stop.m_sName, count, m_iWait));
			PlanSiteShots(stop, pieces);
		}

		m_iShot = 0;
		m_iWait = 0;
		m_iState = S_SHOOT;
	}

	//------------------------------------------------------------------------------------------------
	//! Round the pieces' middle, as far out as they reach: from the south (north up, as on the map),
	//! low from the east and the west, and straight down; then from the ground, where trees hide a
	//! camp from above: the clearest of the eight compass points, and the clearest of those at least
	//! a quarter turn from it.
	protected void PlanSiteShots(RZ_PhotoStop stop, array<IEntity> pieces)
	{
		vector c = stop.m_vPos;
		float r = 25;
		if (!pieces.IsEmpty())
		{
			vector sum = vector.Zero;
			foreach (IEntity piece : pieces)
			{
				sum = sum + piece.GetOrigin();
			}

			c = sum * (1.0 / pieces.Count());
			r = 0;
			foreach (IEntity other : pieces)
			{
				r = Math.Max(r, RZ_Util.DistXZ(other.GetOrigin(), c));
			}
		}

		r = Math.Clamp(r, 18, 70);
		c = RZ_Util.GroundPos(c);
		AddShot(stop.m_sName + "_1_south", c + Vector(0, r * 0.9 + 14, -(r * 1.1 + 14)), c);
		AddShot(stop.m_sName + "_2_east", c + Vector(r * 0.9 + 16, r * 0.45 + 10, r * 0.5 + 8), c + Vector(0, 1.5, 0));
		AddShot(stop.m_sName + "_3_west", c + Vector(-(r * 0.9 + 16), r * 0.45 + 10, -(r * 0.3)), c + Vector(0, 1.5, 0));
		AddShot(stop.m_sName + "_4_top", c + Vector(0, r * 2.0 + 25, 0), c);

		float reach = Math.Clamp(r * 0.6 + 8, 12, 30);
		vector look = c + Vector(0, 1.5, 0);
		array<float> clear = {};
		int best = -1;
		for (int i = 0; i < 8; i++)
		{
			vector eye = GroundEye(c, i * 45, reach);
			float way = -1;
			if (!RZ_CampLife.WetAt(eye))
				way = Clear(eye, look);

			clear.Insert(way);
			if (way >= 0 && (best < 0 || way > clear[best]))
				best = i;
		}

		if (best < 0)
			return;

		if (m_bAround)
		{
			for (int all = 0; all < 8; all++)
			{
				if (clear[all] >= 0)
					AddShot(stop.m_sName + "_g" + all.ToString() + "_ground", GroundEye(c, all * 45, reach), look);
			}

			return;
		}

		AddShot(stop.m_sName + "_5_ground", GroundEye(c, best * 45, reach), look);
		int second = -1;
		for (int k = 0; k < 8; k++)
		{
			int turn = Math.AbsInt(k - best);
			if (turn > 4)
				turn = 8 - turn;

			if (turn < 2 || clear[k] < 0)
				continue;

			if (second < 0 || clear[k] > clear[second])
				second = k;
		}

		if (second >= 0)
			AddShot(stop.m_sName + "_6_ground", GroundEye(c, second * 45, reach), look);
	}

	//------------------------------------------------------------------------------------------------
	//! Standing height, reach metres from the middle on a bearing.
	protected vector GroundEye(vector middle, float bearing, float reach)
	{
		float rad = bearing * Math.DEG2RAD;
		vector eye = RZ_Util.GroundPos(middle + Vector(Math.Sin(rad) * reach, 0, Math.Cos(rad) * reach));
		return eye + Vector(0, 3, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! How much of the way from one point to another is open (1 = all of it).
	protected float Clear(vector from, vector to)
	{
		TraceParam param = new TraceParam();
		param.Start = from;
		param.End = to;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		return GetGame().GetWorld().TraceMove(param, null);
	}

	//------------------------------------------------------------------------------------------------
	//! Each piece close up from the south-east, as far off as it is big, then the whole group.
	protected void PlanGroupShots(RZ_PhotoStop stop)
	{
		vector lo = Vector(float.MAX, float.MAX, float.MAX);
		vector hi = Vector(-float.MAX, -float.MAX, -float.MAX);
		foreach (int i, IEntity e : m_aShown)
		{
			if (!e)
				continue;

			vector mins = Vector(float.MAX, float.MAX, float.MAX);
			vector maxs = Vector(-float.MAX, -float.MAX, -float.MAX);
			Bounds(e, 0, mins, maxs);
			if (mins[0] > maxs[0])
			{
				mins = e.GetOrigin() - Vector(1, 0, 1);
				maxs = e.GetOrigin() + Vector(1, 2, 1);
			}

			vector mid = (mins + maxs) * 0.5;
			float size = Math.Max(maxs[0] - mins[0], Math.Max(maxs[1] - mins[1], maxs[2] - mins[2]));
			size = Math.Clamp(size, 1.6, 30);
			int order = i + 1;
			string number = order.ToString();
			if (order < 10)
				number = "0" + number;

			AddShot(stop.m_sName + "_" + number + "_" + m_aShownNames[i], mid + Vector(size * 0.8, size * 0.45 + 0.6, -size * 1.2), mid);
			for (int k = 0; k < 3; k++)
			{
				lo[k] = Math.Min(lo[k], mins[k]);
				hi[k] = Math.Max(hi[k], maxs[k]);
			}
		}

		if (lo[0] > hi[0])
			return;

		vector c = (lo + hi) * 0.5;
		float span = Math.Max(hi[0] - lo[0], hi[2] - lo[2]);
		AddShot(stop.m_sName + "_all", c + Vector(span * 0.45, span * 0.5 + 4, -(span * 0.75 + 6)), c);
	}

	//------------------------------------------------------------------------------------------------
	//! The box of an entity and its children, in the world (lights, decals and effects left out).
	protected void Bounds(IEntity e, int depth, inout vector lo, inout vector hi)
	{
		if (!e || depth > 3)
			return;

		if (LightEntity.Cast(e) || DecalEntity.Cast(e) || ParticleEffectEntity.Cast(e))
			return;

		vector mins;
		vector maxs;
		e.GetWorldBounds(mins, maxs);
		if (maxs[0] - mins[0] > 0.001 || maxs[2] - mins[2] > 0.001)
		{
			for (int i = 0; i < 3; i++)
			{
				lo[i] = Math.Min(lo[i], mins[i]);
				hi[i] = Math.Max(hi[i], maxs[i]);
			}
		}

		IEntity child = e.GetChildren();
		while (child)
		{
			Bounds(child, depth + 1, lo, hi);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddShot(string name, vector eye, vector at)
	{
		// Never under the hill it looks from
		float ground = GetGame().GetWorld().GetSurfaceY(eye[0], eye[2]);
		if (eye[1] < ground + 3)
			eye[1] = ground + 3;

		RZ_PhotoShot shot = new RZ_PhotoShot();
		shot.m_sName = name;
		shot.m_vEye = eye;
		shot.m_vAt = at;
		m_aShots.Insert(shot);
	}

	//------------------------------------------------------------------------------------------------
	//! One photo every five seconds (three for a group's pieces, already in view): aim, give the
	//! picture time to load, take it. The engine adds ".png".
	protected void Shoot()
	{
		if (m_iShot >= m_aShots.Count() || !m_Camera)
		{
			m_iState = S_NEXT;
			return;
		}

		RZ_PhotoShot shot = m_aShots[m_iShot];
		if (m_iWait == 0)
			Aim(shot);

		int load = 4;
		if (m_aStops[m_iStop].m_aPrefabs && m_iShot > 0)
			load = 2;

		m_iWait++;
		if (m_iWait == load)
		{
			System.MakeScreenshot(FOLDER + shot.m_sName);
			m_iPhotos++;
		}

		if (m_iWait > load)
		{
			m_iShot++;
			m_iWait = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void Aim(RZ_PhotoShot shot)
	{
		vector dir = vector.Direction(shot.m_vEye, shot.m_vAt).Normalized();
		vector up = vector.Up;
		if (Math.AbsFloat(dir[1]) > 0.98)
			up = Vector(0, 0, 1);

		vector mat[4];
		Math3D.DirectionAndUpMatrix(dir, up, mat);
		mat[3] = shot.m_vEye;
		m_Camera.SetWorldTransform(mat);
		CameraManager cameras = GetGame().GetCameraManager();
		if (cameras)
			cameras.SetCamera(m_Camera);
	}

	//------------------------------------------------------------------------------------------------
	//! A group's compositions in rows, as the Workbench plugin Show the next composition group lays them.
	protected void LayOut(RZ_PhotoStop stop)
	{
		int cols = 4;
		float spaceX = 9;
		float spaceZ = 10;
		float yaw = 0;
		if (stop.m_bClose)
		{
			cols = 6;
			spaceX = 3;
			spaceZ = 4;
			yaw = 180;
		}

		foreach (int i, string prefab : stop.m_aPrefabs)
		{
			int row = i / cols;
			int col = i - row * cols;
			vector pos = RZ_Util.GroundPos(stop.m_vPos + Vector(col * spaceX, 0, row * spaceZ));
			IEntity e = RZ_Util.SpawnPrefab(prefab, pos, yaw);
			if (!e)
			{
				RZ_Log.Warn("Photo tour: couldn't spawn " + prefab);
				continue;
			}

			m_aShown.Insert(e);
			m_aShownNames.Insert(Clean(FilePath.StripExtension(FilePath.StripPath(prefab))));
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearShown()
	{
		foreach (IEntity e : m_aShown)
		{
			if (e)
				SCR_EntityHelper.DeleteEntityAndChildren(e);
		}

		m_aShown.Clear();
		m_aShownNames.Clear();
	}

	//------------------------------------------------------------------------------------------------
	//! The first dry spot STAND_OFF from the site, trying the eight compass points from the south.
	protected vector StandSpot(vector site)
	{
		for (int i = 0; i < 8; i++)
		{
			float bearing = (180 + i * 45) * Math.DEG2RAD;
			vector spot = site + Vector(Math.Sin(bearing) * STAND_OFF, 0, Math.Cos(bearing) * STAND_OFF);
			if (!RZ_CampLife.WetAt(spot))
				return RZ_Util.GroundPos(spot);
		}

		return RZ_Util.GroundPos(site);
	}

	//------------------------------------------------------------------------------------------------
	protected void MovePlayer(vector pos)
	{
		SCR_Global.TeleportLocalPlayer(RZ_Util.GroundPos(pos) + Vector(0, 0.3, 0));
	}

	//------------------------------------------------------------------------------------------------
	//! Nothing kills the photographer on the way.
	protected void Protect()
	{
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (!player)
			return;

		DamageManagerComponent damage = DamageManagerComponent.Cast(player.FindComponent(DamageManagerComponent));
		if (damage)
			damage.EnableDamageHandling(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetHour(float hour, bool advance)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return;

		TimeAndWeatherManagerEntity clock = world.GetTimeAndWeatherManager();
		if (!clock)
			return;

		if (hour >= 0)
			clock.SetTimeOfTheDay(hour, true);

		clock.SetIsDayAutoAdvanced(advance);
	}

	//------------------------------------------------------------------------------------------------
	protected void Finish()
	{
		m_iState = S_DONE;
		GetGame().GetCallqueue().Remove(Step);
		CameraManager cameras = GetGame().GetCameraManager();
		if (cameras && m_PlayerCamera)
			cameras.SetCamera(m_PlayerCamera);

		if (m_Camera)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Camera);

		SCR_HUDManagerComponent hud = GetGame().GetHUDManager();
		if (hud)
			hud.SetVisible(true);

		if (m_fHour >= 0)
			SetHour(-1, true);

		RZ_Log.Info(string.Format("Photo tour: done, %1 photos of %2 stops in %3", m_iPhotos, m_aStops.Count(), FOLDER));
	}

	//------------------------------------------------------------------------------------------------
	//! A title as a file name: letters and digits, everything else an underscore.
	protected static string Clean(string title)
	{
		string keep = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		string name = "";
		for (int i = 0; i < title.Length(); i++)
		{
			string ch = title.Get(i);
			if (keep.Contains(ch))
				name = name + ch;
			else if (!name.IsEmpty() && !name.EndsWith("_"))
				name = name + "_";
		}

		return name;
	}
}
#endif
