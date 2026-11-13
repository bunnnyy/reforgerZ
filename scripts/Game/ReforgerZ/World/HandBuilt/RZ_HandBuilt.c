//! Hand-built sites (docs/SITES.md, "Hand-built sites"): a camp site (a holding, a Haven safehouse,
//! a road post) the author built by hand in Game Master, saved in world coordinates. A site with a
//! hand-built save in use is laid out from it by camp life instead of its automated layout; a site
//! without one keeps the automated layout. The site builder (RZ_SiteBuilder) makes and edits them.
//!
//! Where saves live:
//! - shipped: compiled into the mod (RZ_HandBuiltPack, written by tools/hand_built.py from the
//!   author's saves), so they ship with it and work on any server;
//! - on this machine: $profile:ReforgerZ/hand_built/<world>/<site id>.txt, written by the site
//!   builder. A save here takes the place of the shipped one with the same id (the author edits
//!   here, then the tool ships it). A save with "deleted 1" hides a shipped one.
//!
//! A save's lines (one per line, by their first word; any other first word is skipped, so later
//! kinds of lines can be added): site <id>, title <text>, kind <hold|out|safe|post>, centre x y z,
//! radius <m>, use <1|0>, saved <text>, deleted <1|0>, obj <prefab> x y z yaw pitch roll scale.
//!
//! A site's id is its kind and its place rounded to the metre ("out_4810_6620"): the same site gets
//! the same id every session, whatever order it's listed in. A place matches a save with its id, or
//! the nearest of its kind within MATCH_REACH (a holding that moved a few metres).

//------------------------------------------------------------------------------------------------
//! One object of a hand-built site, as data.
class RZ_HandBuiltObject
{
	ResourceName m_sPrefab;
	vector m_vPos;
	vector m_vAngles;		// yaw, pitch, roll (Math3D.AnglesToMatrix's order)
	float m_fScale = 1;
}

//------------------------------------------------------------------------------------------------
//! One hand-built site's save.
class RZ_HandBuiltSite
{
	string m_sId;
	string m_sTitle;
	string m_sKind;				// hold, out, safe, post
	vector m_vCentre;
	float m_fRadius;
	bool m_bUse = true;			// the game lays it out from this save (else the automated layout; the save stays)
	string m_sSaved;			// when it was saved, as text
	bool m_bDeleted;			// a save on this machine that hides a shipped one
	bool m_bShipped;			// from the mod, not this machine
	ref array<ref RZ_HandBuiltObject> m_aObjects = {};
}

//------------------------------------------------------------------------------------------------
//! The hand-built sites of this map: the shipped ones, then this machine's saves over them.
class RZ_HandBuilt
{
	static const string DIR = "$profile:ReforgerZ/hand_built";
	static const int BACKUPS = 3;				// a save keeps this many earlier versions (<id>.bak1.txt newest)
	static const float MATCH_REACH = 25;		// a place matches a save of its kind this close, when the ids differ

	protected static ref RZ_HandBuilt s_Instance;

	protected string m_sWorld;
	protected ref map<string, ref RZ_HandBuiltSite> m_mSites = new map<string, ref RZ_HandBuiltSite>();
	protected RZ_HandBuiltSite m_Filling;		// the shipped site RZ_HandBuiltPack is adding objects to
	protected string m_sFillingWorld;

	//------------------------------------------------------------------------------------------------
	//! The store, loaded the first time it's asked for (server).
	static RZ_HandBuilt Get()
	{
		if (!s_Instance)
		{
			s_Instance = new RZ_HandBuilt();
			s_Instance.Load();
		}

		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! The map's short name: the world file's name ("RZ_Everon").
	static string WorldName()
	{
		string file = FilePath.StripPath(GetGame().GetWorldFile());
		return FilePath.StripExtension(file);
	}

	//------------------------------------------------------------------------------------------------
	//! A site kind's word in ids and saves.
	static string KindWord(RZ_ECampPlace kind)
	{
		switch (kind)
		{
			case RZ_ECampPlace.STRONGHOLD: return "hold";
			case RZ_ECampPlace.SAFEHOUSE: return "safe";
			case RZ_ECampPlace.ROAD_POST: return "post";
		}

		return "out";
	}

	//------------------------------------------------------------------------------------------------
	//! How far round its centre a site's hand-built objects belong to it (m).
	static float RadiusOf(RZ_ECampPlace kind)
	{
		switch (kind)
		{
			case RZ_ECampPlace.STRONGHOLD: return 72;
			case RZ_ECampPlace.SAFEHOUSE: return 40;
			case RZ_ECampPlace.ROAD_POST: return 22;
		}

		return 56;
	}

	//------------------------------------------------------------------------------------------------
	//! A site's id: its kind and its place rounded to the metre.
	static string SiteId(RZ_ECampPlace kind, vector pos)
	{
		return string.Format("%1_%2_%3", KindWord(kind), RZ_Util.RoundToInt(pos[0]), RZ_Util.RoundToInt(pos[2]));
	}

	//------------------------------------------------------------------------------------------------
	string GetWorld() { return m_sWorld; }

	//------------------------------------------------------------------------------------------------
	//! The save with this id (not one marked deleted), or null.
	RZ_HandBuiltSite Find(string id)
	{
		RZ_HandBuiltSite s;
		if (!m_mSites.Find(id, s) || !s || s.m_bDeleted)
			return null;

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! The save for a place of this kind here: its own id, else the nearest of its kind within
	//! MATCH_REACH. Null: none (the automated layout).
	RZ_HandBuiltSite ForPlace(RZ_ECampPlace kind, vector pos)
	{
		RZ_HandBuiltSite own = Find(SiteId(kind, pos));
		if (own)
			return own;

		string word = KindWord(kind);
		RZ_HandBuiltSite best;
		float bestDist = MATCH_REACH;
		foreach (string id, RZ_HandBuiltSite s : m_mSites)
		{
			if (!s || s.m_bDeleted || s.m_sKind != word)
				continue;

			float d = RZ_Util.DistXZ(s.m_vCentre, pos);
			if (d <= bestDist)
			{
				bestDist = d;
				best = s;
			}
		}

		return best;
	}

	//------------------------------------------------------------------------------------------------
	//! A place with no save while the map has saves of its kind: the nearest is named in the log, so a
	//! save left behind when the season put the site elsewhere is found (the site audit asks).
	void TellNoMatch(string title, RZ_ECampPlace kind, vector pos)
	{
		string word = KindWord(kind);
		RZ_HandBuiltSite best;
		float bestDist = float.MAX;
		foreach (string id, RZ_HandBuiltSite s : m_mSites)
		{
			if (!s || s.m_bDeleted || s.m_sKind != word)
				continue;

			float d = RZ_Util.DistXZ(s.m_vCentre, pos);
			if (d < bestDist)
			{
				bestDist = d;
				best = s;
			}
		}

		if (best)
			RZ_Log.Info(string.Format("Hand-built: %1 (%2) has no save; the nearest of its kind is %3, %4 m off.", title, SiteId(kind, pos), best.m_sId, RZ_Util.RoundToInt(bestDist)));
	}

	//------------------------------------------------------------------------------------------------
	//! Every save of this map (not those marked deleted).
	void All(notnull array<RZ_HandBuiltSite> result)
	{
		foreach (string id, RZ_HandBuiltSite s : m_mSites)
		{
			if (s && !s.m_bDeleted)
				result.Insert(s);
		}
	}

	// ================================================================ Loading
	//------------------------------------------------------------------------------------------------
	protected void Load()
	{
		m_sWorld = WorldName();
		RZ_HandBuiltPack.Add(this);
		m_Filling = null;
		int shipped = m_mSites.Count();

		int here = 0;
		array<string> files = {};
		string dir = DIR + "/" + m_sWorld;
		FileIO.FindFiles(files.Insert, dir, ".txt");
		foreach (string file : files)
		{
			if (file.Contains(".bak"))
				continue;

			RZ_HandBuiltSite s = ReadFile(file);
			if (!s)
				continue;

			m_mSites.Set(s.m_sId, s);
			here++;
		}

		int inUse = 0;
		int objects = 0;
		foreach (string id, RZ_HandBuiltSite site : m_mSites)
		{
			if (site && !site.m_bDeleted && site.m_bUse)
			{
				inUse++;
				objects += site.m_aObjects.Count();
			}
		}

		RZ_Log.Info(string.Format("Hand-built: %1 has %2 sites (%3 shipped, %4 saved on this machine), %5 in use, %6 objects.", m_sWorld, inUse + CountNotInUse(), shipped, here, inUse, objects));
	}

	//------------------------------------------------------------------------------------------------
	protected int CountNotInUse()
	{
		int n = 0;
		foreach (string id, RZ_HandBuiltSite s : m_mSites)
		{
			if (s && !s.m_bDeleted && !s.m_bUse)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_HandBuiltPack: a shipped site begins (only this map's are kept).
	void Site(string world, string id, string title, string kind, float x, float y, float z, float radius, bool use, string saved)
	{
		m_Filling = null;
		if (world != m_sWorld)
			return;

		RZ_HandBuiltSite s = new RZ_HandBuiltSite();
		s.m_sId = id;
		s.m_sTitle = title;
		s.m_sKind = kind;
		s.m_vCentre = Vector(x, y, z);
		s.m_fRadius = radius;
		s.m_bUse = use;
		s.m_sSaved = saved;
		s.m_bShipped = true;
		m_mSites.Set(id, s);
		m_Filling = s;
	}

	//------------------------------------------------------------------------------------------------
	//! From RZ_HandBuiltPack: one object of the shipped site begun last.
	void Obj(ResourceName prefab, float x, float y, float z, float yaw, float pitch, float roll, float scale)
	{
		if (!m_Filling)
			return;

		RZ_HandBuiltObject o = new RZ_HandBuiltObject();
		o.m_sPrefab = prefab;
		o.m_vPos = Vector(x, y, z);
		o.m_vAngles = Vector(yaw, pitch, roll);
		o.m_fScale = scale;
		m_Filling.m_aObjects.Insert(o);
	}

	//------------------------------------------------------------------------------------------------
	//! One save read from a file; null if it isn't one.
	static RZ_HandBuiltSite ReadFile(string path)
	{
		if (!FileIO.FileExists(path))
			return null;

		FileHandle file = FileIO.OpenFile(path, FileMode.READ);
		if (!file)
			return null;

		RZ_HandBuiltSite s = new RZ_HandBuiltSite();
		string line;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (line.IsEmpty() || line.IndexOf("#") == 0)
				continue;

			int gap = line.IndexOf(" ");
			if (gap <= 0)
				continue;

			string word = line.Substring(0, gap);
			string rest = line.Substring(gap + 1, line.Length() - gap - 1);
			rest.Trim();
			ReadLine(s, word, rest);
		}

		file.Close();
		if (s.m_sId.IsEmpty())
			return null;

		return s;
	}

	//------------------------------------------------------------------------------------------------
	//! One line of a save; a line kind it doesn't know is skipped.
	protected static void ReadLine(RZ_HandBuiltSite s, string word, string rest)
	{
		array<string> parts = {};
		rest.Split(" ", parts, true);
		if (word == "site")
			s.m_sId = rest;
		else if (word == "title")
			s.m_sTitle = rest;
		else if (word == "kind")
			s.m_sKind = rest;
		else if (word == "saved")
			s.m_sSaved = rest;
		else if (word == "use")
			s.m_bUse = rest != "0";
		else if (word == "deleted")
			s.m_bDeleted = rest == "1";
		else if (word == "radius")
			s.m_fRadius = rest.ToFloat();
		else if (word == "centre" && parts.Count() >= 3)
			s.m_vCentre = Vector(parts[0].ToFloat(), parts[1].ToFloat(), parts[2].ToFloat());
		else if (word == "obj" && parts.Count() >= 8)
		{
			RZ_HandBuiltObject o = new RZ_HandBuiltObject();
			o.m_sPrefab = parts[0];
			o.m_vPos = Vector(parts[1].ToFloat(), parts[2].ToFloat(), parts[3].ToFloat());
			o.m_vAngles = Vector(parts[4].ToFloat(), parts[5].ToFloat(), parts[6].ToFloat());
			o.m_fScale = parts[7].ToFloat();
			if (o.m_fScale <= 0)
				o.m_fScale = 1;

			s.m_aObjects.Insert(o);
		}

		// Any other kind of line (a later version's) is skipped
	}

	// ================================================================ Saving
	//------------------------------------------------------------------------------------------------
	protected string PathOf(string id)
	{
		return DIR + "/" + m_sWorld + "/" + id + ".txt";
	}

	//------------------------------------------------------------------------------------------------
	protected string BackupOf(string id, int n)
	{
		return DIR + "/" + m_sWorld + "/" + id + ".bak" + n.ToString() + ".txt";
	}

	//------------------------------------------------------------------------------------------------
	//! How many earlier versions of this site's save are kept on this machine.
	int BackupCount(string id)
	{
		int n = 0;
		for (int i = 1; i <= BACKUPS; i++)
		{
			if (FileIO.FileExists(BackupOf(id, i)))
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Writes the site's save on this machine, overriding the one before (kept as the newest backup).
	//! Keeps it in the store. False if the file couldn't be written.
	bool Save(RZ_HandBuiltSite s)
	{
		if (!s || s.m_sId.IsEmpty())
			return false;

		FileIO.MakeDirectory("$profile:ReforgerZ");
		FileIO.MakeDirectory(DIR);
		FileIO.MakeDirectory(DIR + "/" + m_sWorld);
		string path = PathOf(s.m_sId);
		if (FileIO.FileExists(path))
			Rotate(s.m_sId);

		if (!WriteFile(path, s))
			return false;

		s.m_bShipped = false;
		m_mSites.Set(s.m_sId, s);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! The newest backup becomes the save again (the save it replaces is dropped); the older ones move
	//! up. False: no backup.
	bool RestoreLast(string id)
	{
		string first = BackupOf(id, 1);
		if (!FileIO.FileExists(first))
			return false;

		string path = PathOf(id);
		FileIO.DeleteFile(path);
		FileIO.CopyFile(first, path);
		FileIO.DeleteFile(first);
		for (int i = 2; i <= BACKUPS; i++)
		{
			string older = BackupOf(id, i);
			if (!FileIO.FileExists(older))
				break;

			FileIO.CopyFile(older, BackupOf(id, i - 1));
			FileIO.DeleteFile(older);
		}

		RZ_HandBuiltSite s = ReadFile(path);
		if (s)
			m_mSites.Set(id, s);

		return s != null;
	}

	//------------------------------------------------------------------------------------------------
	//! The save is gone (its backups too): a shipped one with the id is hidden by a "deleted" save on
	//! this machine, so the tool drops it from the mod.
	void Delete(string id)
	{
		RZ_HandBuiltSite s = Find(id);
		for (int i = 1; i <= BACKUPS; i++)
		{
			FileIO.DeleteFile(BackupOf(id, i));
		}

		FileIO.DeleteFile(PathOf(id));
		if (!s)
			return;

		RZ_HandBuiltSite gone = new RZ_HandBuiltSite();
		gone.m_sId = id;
		gone.m_sTitle = s.m_sTitle;
		gone.m_sKind = s.m_sKind;
		gone.m_vCentre = s.m_vCentre;
		gone.m_bDeleted = true;
		FileIO.MakeDirectory(DIR);
		FileIO.MakeDirectory(DIR + "/" + m_sWorld);
		WriteFile(PathOf(id), gone);
		m_mSites.Set(id, gone);
	}

	//------------------------------------------------------------------------------------------------
	//! Switches whether the game uses the save (it stays either way). False: no save.
	bool SetUse(string id, bool use)
	{
		RZ_HandBuiltSite s = Find(id);
		if (!s)
			return false;

		s.m_bUse = use;
		FileIO.MakeDirectory("$profile:ReforgerZ");
		FileIO.MakeDirectory(DIR);
		FileIO.MakeDirectory(DIR + "/" + m_sWorld);
		WriteFile(PathOf(id), s);
		s.m_bShipped = false;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void Rotate(string id)
	{
		FileIO.DeleteFile(BackupOf(id, BACKUPS));
		for (int i = BACKUPS - 1; i >= 1; i--)
		{
			string from = BackupOf(id, i);
			if (FileIO.FileExists(from))
			{
				FileIO.CopyFile(from, BackupOf(id, i + 1));
				FileIO.DeleteFile(from);
			}
		}

		FileIO.CopyFile(PathOf(id), BackupOf(id, 1));
	}

	//------------------------------------------------------------------------------------------------
	protected static bool WriteFile(string path, RZ_HandBuiltSite s)
	{
		FileIO.DeleteFile(path);
		FileHandle file = FileIO.OpenFile(path, FileMode.WRITE);
		if (!file)
		{
			RZ_Log.Warn("Hand-built: couldn't write " + path);
			return false;
		}

		file.WriteLine("# ReforgerZ hand-built site (docs/SITES.md, \"Hand-built sites\"). Ship it with tools/hand_built.py.");
		file.WriteLine("site " + s.m_sId);
		file.WriteLine("title " + s.m_sTitle);
		file.WriteLine("kind " + s.m_sKind);
		file.WriteLine(string.Format("centre %1 %2 %3", Num(s.m_vCentre[0]), Num(s.m_vCentre[1]), Num(s.m_vCentre[2])));
		file.WriteLine("radius " + Num(s.m_fRadius));
		if (s.m_bDeleted)
		{
			file.WriteLine("deleted 1");
			file.Close();
			return true;
		}

		int use = 0;
		if (s.m_bUse)
			use = 1;

		file.WriteLine("use " + use.ToString());
		file.WriteLine("saved " + s.m_sSaved);
		foreach (RZ_HandBuiltObject o : s.m_aObjects)
		{
			file.WriteLine(string.Format("obj %1 %2 %3 %4 %5 %6 %7 %8", o.m_sPrefab, Num(o.m_vPos[0]), Num(o.m_vPos[1]), Num(o.m_vPos[2]), Num(o.m_vAngles[0]), Num(o.m_vAngles[1]), Num(o.m_vAngles[2]), Num(o.m_fScale)));
		}

		file.Close();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! A number with three decimals (millimetres, a tenth of a degree is plenty).
	static string Num(float v)
	{
		return v.ToString(-1, 3);
	}

	//------------------------------------------------------------------------------------------------
	//! Now, as a save's "saved" text: 2026-11-13 14:22.
	static string Now()
	{
		int y;
		int mo;
		int d;
		int h;
		int mi;
		int sec;
		System.GetYearMonthDay(y, mo, d);
		System.GetHourMinuteSecond(h, mi, sec);
		return string.Format("%1-%2-%3 %4:%5", y, Two(mo), Two(d), Two(h), Two(mi));
	}

	//------------------------------------------------------------------------------------------------
	protected static string Two(int v)
	{
		if (v < 10)
			return "0" + v.ToString();

		return v.ToString();
	}

	// ================================================================ Spawning
	//------------------------------------------------------------------------------------------------
	//! One object put into the world as it was saved: place, turn, tilt and scale.
	static IEntity Spawn(RZ_HandBuiltObject o)
	{
		Resource res = Resource.Load(o.m_sPrefab);
		if (!res || !res.IsValid())
		{
			RZ_Log.Warn("Hand-built: no such prefab " + o.m_sPrefab);
			return null;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		Math3D.AnglesToMatrix(o.m_vAngles, params.Transform);
		params.Transform[3] = o.m_vPos;
		params.Scale = o.m_fScale;
		return GetGame().SpawnEntityPrefab(res, GetGame().GetWorld(), params); // VERIFY: an object spawned with the angles MatrixToAngles read from it (yaw, pitch, roll) and its scale stands exactly as it did in Game Master
	}

	//------------------------------------------------------------------------------------------------
	//! An entity's place, turn and scale, as a save's object.
	static RZ_HandBuiltObject Read(IEntity e)
	{
		RZ_HandBuiltObject o = new RZ_HandBuiltObject();
		o.m_sPrefab = RZ_Util.GetPrefabName(e);
		vector t[4];
		e.GetWorldTransform(t);
		o.m_vPos = t[3];
		vector rot[3];
		rot[0] = t[0];
		rot[1] = t[1];
		rot[2] = t[2];
		o.m_vAngles = Math3D.MatrixToAngles(rot);
		o.m_fScale = e.GetScale();
		if (o.m_fScale <= 0)
			o.m_fScale = 1;

		return o;
	}
}
