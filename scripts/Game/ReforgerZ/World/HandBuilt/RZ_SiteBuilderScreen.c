//! The site builder's own screen and its build HUD (docs/SITES.md, "The site builder"; UI/LAYOUTS.md):
//! the server's half. The screen (UI/RZ_SiteBuilderMenu.c) lists every site with faction tabs and
//! filters, and shows the picked site's details and role-spot checklist beside the list; browsing
//! stays on the client, and only a button that changes something comes here (ClientAct), as the same
//! action codes the object-menu version uses (RZ_SiteBuilder.Act). While a site is built the HUD
//! (UI/RZ_SiteHud.c) shows it, its object count and its checklist, refreshed every two seconds.
//!
//! Payload (RZ_UIFeed.PAYLOAD_SITES):
//!   H|world|sites|hand-built|building id|unsaved|view|ask text|armed|spot type
//!   S|id|name|kind|faction|state|metres|direction|grid|pinned|building (0, 1 me, 2 someone else)|
//!     have|want|objects|saved|backups|road post|automated pieces up
//!   C|label|ok|required   (the selected site's checklist, then the HUD's)
//! "X|reason" closes the screen; F from the client means it couldn't open one (the object menu then).
//! HUD payload (RZ_UIFeed.PAYLOAD_SITEHUD): B|name|head|objects|unsaved|outside|have|want|spot type|
//! problems, then C records; "" hides it.
class RZ_SiteBuilderScreen
{
	protected static bool s_bTicking;
	protected static const int HUD_MS = 2000;

	//------------------------------------------------------------------------------------------------
	//! The screen for the player, with the session's state (Open falls back to the object menu when
	//! the client says it couldn't open it).
	static void Send(RZ_SiteBuilderSession s)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		if (ui)
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SITES, RZ_SiteBuilder.ScreenPayload(s));

		SendHud(s);
	}

	//------------------------------------------------------------------------------------------------
	//! The build HUD while the session builds a site, else hidden.
	static void SendHud(RZ_SiteBuilderSession s)
	{
		RZ_PlayerUIComponent ui = RZ_PlayerUIComponent.GetFor(s.m_iPlayer);
		if (!ui)
			return;

		if (!s.m_Building)
		{
			ui.SendPayload(RZ_UIFeed.PAYLOAD_SITEHUD, "");
			return;
		}

		ui.SendPayload(RZ_UIFeed.PAYLOAD_SITEHUD, RZ_SiteBuilder.HudPayload(s));
		if (!s_bTicking)
		{
			s_bTicking = true;
			GetGame().GetCallqueue().CallLater(Tick, HUD_MS, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Every two seconds while anyone builds: their HUD again (objects placed in Game Master change it).
	protected static void Tick()
	{
		bool any = RZ_SiteBuilder.HudTick();
		if (any)
			return;

		GetGame().GetCallqueue().Remove(Tick);
		s_bTicking = false;
	}
}

//------------------------------------------------------------------------------------------------
modded class RZ_SiteBuilder
{
	//------------------------------------------------------------------------------------------------
	//! The screen's payload: the head, every site, and the selected site's checklist.
	static string ScreenPayload(RZ_SiteBuilderSession s)
	{
		array<ref RZ_SiteEntry> all = {};
		Sites(all);
		s_aKeep = all;
		int hand = 0;
		foreach (RZ_SiteEntry counted : all)
		{
			if (StateOf(counted) != 0)
				hand++;
		}

		string buildingId = "";
		bool unsaved = false;
		if (s.m_Building)
		{
			buildingId = s.m_Building.m_sId;
			unsaved = Unsaved(s);
		}

		int view = 0;
		string ask = "";
		if (s.m_iView == VIEW_LEAVE && s.m_Building)
		{
			view = 2;
			ask = string.Format("%1 has unsaved changes. Save them, discard them (it goes back to its last save), or stay and keep building.", Name(s.m_Building));
		}

		array<string> types = {};
		RZ_HandSpots.Types(types);
		string type = RZ_HandSpots.Label(types[s.m_iSpotType % types.Count()]);
		RZ_UILine headLine = new RZ_UILine("H");
		string pay = headLine.Add(RZ_HandBuilt.Get().GetWorld()).AddInt(all.Count()).AddInt(hand).Add(buildingId).AddInt(Bit(unsaved)).AddInt(view).Add(ask).Add(s.m_sArmed).Add(type).Add(s.m_sSaid).End();
		s.m_sSaid = "";
		vector me = PlayerPos(s.m_iPlayer);
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		foreach (RZ_SiteEntry e : all)
		{
			RZ_HandBuiltSite saved = RZ_HandBuilt.Get().ForPlace(e.m_eKind, e.m_vPos);
			int building = 0;
			if (s.m_Building && s.m_Building.m_sId == e.m_sId)
				building = 1;
			else if (!OtherBuilder(s, e.m_sId).IsEmpty())
				building = 2;

			int have = 0;
			int want = 0;
			Spots(s, e, saved, have, want, "");
			int objects = 0;
			string when = "";
			bool pinned = false;
			if (saved)
			{
				objects = saved.m_aObjects.Count();
				when = saved.m_sSaved;
				pinned = saved.m_bPinned;
			}

			if (building == 1)
			{
				array<IEntity> now = {};
				int outside;
				Capture(s, now, outside);
				objects = now.Count();
			}

			int up = 0;
			if (gm && gm.GetCampLife())
				up = gm.GetCampLife().PiecesAt(e.m_sId, e.m_vPos, e.m_fRadius);

			RZ_UILine line = new RZ_UILine("S");
			line.Add(e.m_sId).Add(Name(e)).Add(KindText(e.m_eKind)).AddInt(e.m_eFaction).AddInt(StateOf(e));
			line.AddInt(RZ_Util.RoundToInt(RZ_Util.DistXZ(me, e.m_vPos))).Add(RZ_Util.DirectionText(me, e.m_vPos)).Add(RZ_SettlementScanner.GridReference(e.m_vPos));
			line.AddInt(Bit(pinned)).AddInt(building).AddInt(have).AddInt(want).AddInt(objects).Add(when);
			string house = "";
			if (gm && gm.GetCampLife())
				house = gm.GetCampLife().BuildingNear(e.m_eKind, e.m_vPos);

			line.AddInt(RZ_HandBuilt.Get().BackupCount(e.m_sId)).AddInt(Bit(e.m_eKind == RZ_ECampPlace.ROAD_POST)).AddInt(up).Add(house);
			pay += line.End();
		}

		// The selected site's checklist
		RZ_SiteEntry picked = null;
		foreach (RZ_SiteEntry p : all)
		{
			if (p.m_sId == s.m_sSelected)
				picked = p;
		}

		if (picked)
		{
			RZ_HandBuiltSite pickedSave = RZ_HandBuilt.Get().ForPlace(picked.m_eKind, picked.m_vPos);
			int h;
			int w;
			pay += Spots(s, picked, pickedSave, h, w, "C");
		}

		return pay;
	}

	//------------------------------------------------------------------------------------------------
	//! A site's role spots against what it needs: what's being built there, else its save's, else
	//! none. `have`/`want`: the needs met and all of them. With a tag, the checklist as records.
	protected static string Spots(RZ_SiteBuilderSession s, RZ_SiteEntry site, RZ_HandBuiltSite saved, out int have, out int want, string tag)
	{
		array<ref RZ_HandSpot> spots = null;
		if (s.m_Building && s.m_Building.m_sId == site.m_sId)
		{
			SyncSpots(s);
			spots = s.m_aSpots;
		}
		else if (saved)
		{
			spots = saved.m_aSpots;
		}

		array<ref RZ_HandNeed> needs = {};
		int missing;
		RZ_HandSpots.Check(site.m_eFaction, site.m_eKind, spots, needs, missing);
		have = 0;
		want = needs.Count();
		string pay = "";
		foreach (RZ_HandNeed n : needs)
		{
			bool ok = n.m_iHave >= n.m_iWant;
			if (ok)
				have++;

			if (tag.IsEmpty())
				continue;

			string label = RZ_HandSpots.Label(n.m_sType);
			if (n.m_iWant > 1)
				label = string.Format("%1 %2 of %3", label, Math.Min(n.m_iHave, 99), n.m_iWant);

			RZ_UILine checkLine = new RZ_UILine(tag);
			pay += checkLine.Add(label).AddInt(Bit(ok)).AddInt(Bit(n.m_bRequired)).End();
		}

		return pay;
	}

	//------------------------------------------------------------------------------------------------
	protected static int Bit(bool on)
	{
		if (on)
			return 1;

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected static string KindText(RZ_ECampPlace kind)
	{
		switch (kind)
		{
			case RZ_ECampPlace.STRONGHOLD: return "stronghold";
			case RZ_ECampPlace.OUTPOST: return "outpost";
			case RZ_ECampPlace.SAFEHOUSE: return "safehouse";
			case RZ_ECampPlace.ROAD_POST: return "road post";
		}

		return "site";
	}

	//------------------------------------------------------------------------------------------------
	//! The HUD's payload while the session builds a site.
	static string HudPayload(RZ_SiteBuilderSession s)
	{
		RZ_SiteEntry site = s.m_Building;
		array<IEntity> now = {};
		int outside;
		Capture(s, now, outside);
		RZ_HandBuiltSite saved = RZ_HandBuilt.Get().ForPlace(site.m_eKind, site.m_vPos);
		int have;
		int want;
		string checklist = Spots(s, site, saved, have, want, "C");
		array<string> types = {};
		RZ_HandSpots.Types(types);
		string type = RZ_HandSpots.Label(types[s.m_iSpotType % types.Count()]);
		string head = string.Format("BUILDING · %1 %2", RZ_Factions.GetName(site.m_eFaction), KindText(site.m_eKind));
		head.ToUpper();
		string problem = "";
		if (Problems(s) > 0)
			problem = FirstProblem(s);

		RZ_UILine hudLine = new RZ_UILine("B");
		string pay = hudLine.Add(Name(site)).Add(head).AddInt(now.Count()).AddInt(Bit(Unsaved(s))).AddInt(outside).AddInt(have).AddInt(want).Add(type).Add(problem).End();
		return pay + checklist;
	}

	//------------------------------------------------------------------------------------------------
	//! Every building session's HUD again; false when nobody builds (the tick stops).
	static bool HudTick()
	{
		if (!s_mSessions)
			return false;

		bool any = false;
		foreach (int pid, RZ_SiteBuilderSession s : s_mSessions)
		{
			if (!s.m_Building)
				continue;

			any = true;
			RZ_SiteBuilderScreen.SendHud(s);
		}

		return any;
	}

	//------------------------------------------------------------------------------------------------
	//! A button on the screen (the client sends the action and the site it's for). "select" only
	//! changes the picked site (its checklist comes back); "closed" the screen went; "fallback" it
	//! couldn't open there (the object menu instead).
	static void ClientAct(int playerId, string act, string id)
	{
		if (!MayBuild(playerId))
			return;

		RZ_SiteBuilderSession s = SessionOf(playerId);
		if (act == "closed")
		{
			OnClosed(s);
			return;
		}

		if (act == "fallback")
		{
			OpenRowsMenu(playerId);
			return;
		}

		if (!id.IsEmpty() && s.m_iView != VIEW_LEAVE)
		{
			s.m_sSelected = id;
			s.m_iView = VIEW_SITE;
		}

		if (act != "select")
		{
			RZ_Log.Info(string.Format("Site builder: %1 pressed %2 on %3.", RZ_Util.PlayerName(playerId), act, s.m_sSelected));
			Act(s, act);
		}

		RZ_SiteBuilderScreen.Send(s);
	}
}
