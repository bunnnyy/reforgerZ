//! Camp sites in Workbench (docs/SITES.md): measuring the camp kit's props, and flying the camera
//! round the sites the audit laid out. Workbench only.

//! Spawns every prop in tools/camp_kit/props.txt in the open world for a moment, reads its box (the
//! prop and its children, leaving out lights, flames and decals), and writes
//! tools/camp_kit/footprints.json: width, depth, height, the box's middle from the prop's origin, and
//! whether the origin is at its feet. Then run tools/camp_kit/build.py. Nothing is saved in the world.
[WorkbenchPluginAttribute(name: "ReforgerZ: Measure prop footprints", description: "Measure every camp-kit prop's box and write tools/camp_kit/footprints.json", wbModules: {"WorldEditor"}, awesomeFontCode: 0xF545)]
class RZ_MeasureFootprintsPlugin : WorkbenchPlugin
{
	[Attribute("$ReforgerZ:tools/camp_kit/props.txt", desc: "The props: one per line, \"Class ResourceName [hier]\" (build.py writes it)")]
	protected string m_sPropsPath;

	[Attribute("$ReforgerZ:tools/camp_kit/footprints.json", desc: "Where to write the boxes")]
	protected string m_sOutPath;

	override void Run()
	{
		WorldEditorAPI api = RZ_WB.Api();
		FileHandle list = FileIO.OpenFile(m_sPropsPath, FileMode.READ);
		if (!api || !list)
		{
			Print("[ReforgerZ] Footprints: no World Editor or no list at " + m_sPropsPath, LogLevel.WARNING);
			return;
		}

		array<string> prefabs = {};
		string line;
		while (list.ReadLine(line) >= 0)
		{
			line.Trim();
			if (line.IsEmpty() || line.IndexOf("#") == 0)
				continue;

			array<string> bits = {};
			line.Split(" ", bits, true);
			if (bits.Count() >= 2)
				prefabs.Insert(bits[1]);
		}

		list.Close();

		// Far from anything, high up, no turn
		vector cam[4];
		api.GetWorld().GetCurrentCamera(cam);
		vector at = cam[3] + Vector(0, 200, 0);
		array<string> lines = {"{", " \"version\": 1,", " \"source\": \"the Workbench plugin ReforgerZ: Measure prop footprints\",", " \"props\": {"};
		array<string> rows = {};
		api.BeginEntityAction("ReforgerZ: Measure footprints");
		foreach (string prefab : prefabs)
		{
			IEntitySource src = api.CreateEntity(prefab, "", api.GetCurrentEntityLayerId(), null, at, vector.Zero);
			if (!src)
				continue;

			IEntity e = api.SourceToEntity(src); // VERIFY: the World Editor's entity for a source just created, with its mesh loaded so GetWorldBounds is its box
			vector lo = Vector(float.MAX, float.MAX, float.MAX);
			vector hi = Vector(-float.MAX, -float.MAX, -float.MAX);
			Bounds(e, 0, lo, hi);
			vector origin = at;
			if (e)
				origin = e.GetOrigin();

			api.DeleteEntity(src);
			if (lo[0] > hi[0])
				continue;

			string name = FilePath.StripPath(prefab);
			float base = lo[1] - origin[1];
			string feet = "false";
			if (Math.AbsFloat(base) < 0.1)
				feet = "true";

			rows.Insert(string.Format("  \"%1\": {\"w\": %2, \"d\": %3, \"h\": %4, \"ox\": %5, \"oz\": %6, \"feet\": %7}", name, R(hi[0] - lo[0]), R(hi[2] - lo[2]), R(hi[1] - lo[1]), R((lo[0] + hi[0]) * 0.5 - origin[0]), R((lo[2] + hi[2]) * 0.5 - origin[2]), feet));
		}

		api.EndEntityAction();
		rows.Sort();
		foreach (int i, string row : rows)
		{
			if (i < rows.Count() - 1)
				row = row + ",";

			lines.Insert(row);
		}

		lines.Insert(" }");
		lines.Insert("}");
		if (RZ_WB.WriteLines(m_sOutPath, lines))
			Print("[ReforgerZ] Footprints: " + rows.Count().ToString() + " of " + prefabs.Count().ToString() + " props measured, written to " + m_sOutPath + " (now run tools/camp_kit/build.py)");
		else
			Print("[ReforgerZ] Footprints: couldn't write " + m_sOutPath, LogLevel.WARNING);
	}

	//! The box of an entity and its children, in the world.
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

	protected static string R(float v)
	{
		return (Math.Round(v * 1000) / 1000).ToString();
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Measure prop footprints", "", this);
	}
}

//! Flies the World Editor camera round the sites the audit laid out (tools/camp_kit/sites_list.txt,
//! written by render.py --placed): the first run above the next site, looking down on it; the next
//! at its gate at eye height, looking in; then on to the next site. For a screenshot of every base to
//! hold against sites_placed.html.
[WorkbenchPluginAttribute(name: "ReforgerZ: Fly to the next site", description: "Move the camera above the next audited site, then to its gate", wbModules: {"WorldEditor"}, awesomeFontCode: 0xF072)]
class RZ_FlyToSitePlugin : WorkbenchPlugin
{
	[Attribute("$ReforgerZ:tools/camp_kit/sites_list.txt", desc: "The list: name x z gate_x gate_z, one site per line")]
	protected string m_sListPath;

	[Attribute("70", desc: "Metres above the site for the overview")]
	protected float m_fHeight;

	protected static int s_iNext;
	protected static bool s_bAtGate;

	override void Run()
	{
		WorldEditorAPI api = RZ_WB.Api();
		FileHandle file = FileIO.OpenFile(m_sListPath, FileMode.READ);
		if (!api || !file)
		{
			Print("[ReforgerZ] Fly to site: no list at " + m_sListPath + " (run render.py --placed on a site audit's log)", LogLevel.WARNING);
			return;
		}

		array<string> lines = {};
		string line;
		while (file.ReadLine(line) >= 0)
		{
			line.Trim();
			if (!line.IsEmpty() && line.IndexOf("#") != 0)
				lines.Insert(line);
		}

		file.Close();
		if (lines.IsEmpty())
			return;

		if (s_iNext >= lines.Count())
			s_iNext = 0;

		array<string> bits = {};
		lines[s_iNext].Split(" ", bits, true);
		if (bits.Count() < 5)
		{
			s_iNext++;
			return;
		}

		float x = bits[1].ToFloat();
		float z = bits[2].ToFloat();
		float gx = bits[3].ToFloat();
		float gz = bits[4].ToFloat();
		vector site = Vector(x, api.GetTerrainSurfaceY(x, z), z);
		if (!s_bAtGate)
		{
			// Above it, a little to the gate's side, looking down at it
			vector side = Vector(gx - x, 0, gz - z);
			if (side.Length() < 1)
				side = Vector(0, 0, -1);

			side.Normalize();
			vector eye = site + side * (m_fHeight * 0.35) + Vector(0, m_fHeight, 0);
			api.SetCamera(eye, (site - eye).Normalized());
			s_bAtGate = true;
			Print(string.Format("[ReforgerZ] Fly to site %1 of %2: %3, from above", s_iNext + 1, lines.Count(), bits[0]));
			return;
		}

		vector gate = Vector(gx, api.GetTerrainSurfaceY(gx, gz) + 1.7, gz);
		vector look = site + Vector(0, 1.5, 0);
		api.SetCamera(gate, (look - gate).Normalized());
		s_bAtGate = false;
		Print(string.Format("[ReforgerZ] Fly to site %1 of %2: %3, at the gate", s_iNext + 1, lines.Count(), bits[0]));
		s_iNext++;
	}

	override void Configure()
	{
		Workbench.ScriptDialog("ReforgerZ: Fly to the next site", "", this);
	}
}
