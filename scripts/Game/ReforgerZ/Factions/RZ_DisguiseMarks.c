//! Borrowed kit, disguise phase 3 (docs/REPUTATION.md, "Whose kit it was"): kit that remembers whose
//! it was, the nail list and the season's counts.
//!
//! Marks (runtime, weak references, at most 200; a restart forgets them):
//! - a dead man's: worn by one of their people when he died (retaliation sees every death of a
//!   watched unit). Fails at their look. The Red Hand's passes only if it was bought fair at the
//!   fence (a Red Hand quartermaster's sale is marked fair);
//! - off the turned: worn by an infected whose kill the spawner counts. Fails at their look;
//! - signed for: sold by that faction's own quartermaster, to the buyer's identity. Someone
//!   else's fails at their look and is taken at a Bastion search. The buyer's own is issued kit:
//!   it still counts as their own colours for a real day after their standing slips below Known;
//! - given: the Red Hand's armband and the Haven's ribbon, to the identity they were given to. A red
//!   armband the Red Hand didn't give you is a tell at its look; a ribbon that isn't yours closes
//!   Haven doors.
//! Saved: the nail list (the last 30 dead per faction, with a seeded name each), Bastion's board of
//! impostors (the last 20) and the season's counts. Server only.

//------------------------------------------------------------------------------------------------
class RZ_KitMark
{
	IEntity m_Item;		// weak
	int m_iKind;
	int m_iFaction;
	string m_sName;		// the dead man's name or coat number; the buyer's name (signed); "" none
	string m_sOwner;	// signed or given: the identity it belongs to; dead: the killer's identity
	int m_iUnix;
	int m_iNail = -1;	// a dead man's entry in the nail list (its id)
}

//------------------------------------------------------------------------------------------------
//! One name on the nail (Bastion), in the book (the Red Hand) or on the authors' list (Halcyon).
class RZ_NailSave
{
	int id;
	int f;
	string name;
	int day;
	float x;
	float z;
	string place;
	int holding = -1;
	int how;		// RZ_DisguiseMarks.HOW_*
	string killer;	// the identity of the player who killed him, "" none
	bool read;		// read on the nightly call (or the change of shift)
}

//------------------------------------------------------------------------------------------------
//! One line on Bastion's board (the season's impostors): the kit, the tells, the name given. Never
//! the player's.
class RZ_ImpostorSave
{
	int f;
	int day;
	string place;
	string kit;
	string named;
}

//------------------------------------------------------------------------------------------------
class RZ_DisguiseSave
{
	ref array<ref RZ_NailSave> nails = {};
	ref array<ref RZ_ImpostorSave> board = {};
	ref array<int> catches = {};		// per faction, this season
	int nailId;
	int jacketsHome;
	int coatsOut;
	int coatsBack;
	ref array<string> lookIdents = {};	// looks passed this season, by identity (the "Never caught" line)
	ref array<int> lookCounts = {};
	ref array<string> caughtIdents = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DisguiseMarks
{
	static const int DEAD = 1;
	static const int TURNED = 2;
	static const int SIGNED = 3;
	static const int FAIR = 4;
	static const int GIVEN = 5;

	static const int HOW_INFECTED = 0;	// died to the infected or nobody saw who
	static const int HOW_KILLED = 1;	// a player's shot
	static const int HOW_HOME = 2;		// his tag came home
	static const int HOW_LOST = 3;		// his kit went to the fence

	// What a player wears, by kind (KindOf)
	static const int KIT_NONE = 0;
	static const int KIT_CLEAN = 1;			// in their kit, nothing remembered
	static const int KIT_ISSUED = 2;		// signed for to you
	static const int KIT_SIGNED_OTHER = 3;
	static const int KIT_DEAD = 4;
	static const int KIT_TURNED = 5;

	protected static const int MAX_MARKS = 200;
	protected static const int MAX_NAILS = 30;
	protected static const int MAX_BOARD = 20;
	protected static const int MAX_LOOKERS = 200;

	protected RZ_GameModeComponent m_GM;
	protected RZ_Disguise m_Disguise;
	protected ref array<ref RZ_KitMark> m_aMarks = {};
	protected ref RZ_DisguiseSave m_Data = new RZ_DisguiseSave();

	//------------------------------------------------------------------------------------------------
	void RZ_DisguiseMarks(RZ_GameModeComponent gm, RZ_Disguise disguise)
	{
		m_GM = gm;
		m_Disguise = disguise;
		EnsureCounts();
	}

	//------------------------------------------------------------------------------------------------
	protected void EnsureCounts()
	{
		if (!m_Data.catches)
			m_Data.catches = {};

		while (m_Data.catches.Count() < RZ_Factions.COUNT)
		{
			m_Data.catches.Insert(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_DisguiseSave Save()
	{
		return m_Data;
	}

	//------------------------------------------------------------------------------------------------
	void Restore(RZ_DisguiseSave saved)
	{
		if (!saved)
			return;

		m_Data = saved;
		if (!m_Data.nails)
			m_Data.nails = {};

		if (!m_Data.board)
			m_Data.board = {};

		if (!m_Data.lookIdents || !m_Data.lookCounts || m_Data.lookIdents.Count() != m_Data.lookCounts.Count())
		{
			m_Data.lookIdents = {};
			m_Data.lookCounts = {};
		}

		if (!m_Data.caughtIdents)
			m_Data.caughtIdents = {};

		EnsureCounts();
	}

	// ================================================================ Marks
	//------------------------------------------------------------------------------------------------
	protected void Prune()
	{
		for (int i = m_aMarks.Count() - 1; i >= 0; i--)
		{
			if (!m_aMarks[i].m_Item)
				m_aMarks.Remove(i);
		}

		while (m_aMarks.Count() >= MAX_MARKS)
		{
			m_aMarks.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_KitMark Find(IEntity item)
	{
		if (!item)
			return null;

		foreach (RZ_KitMark m : m_aMarks)
		{
			if (m.m_Item == item)
				return m;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Marks one item (a new mark replaces an old one: a dead man's jacket bought fair is fair).
	protected RZ_KitMark Mark(IEntity item, int kind, int f, string name, string owner)
	{
		if (!item)
			return null;

		RZ_KitMark m = Find(item);
		if (!m)
		{
			Prune();
			m = new RZ_KitMark();
			m.m_Item = item;
			m_aMarks.Insert(m);
		}

		m.m_iKind = kind;
		m.m_iFaction = f;
		m.m_sName = name;
		m.m_sOwner = owner;
		m.m_iUnix = RZ_SeasonClock.Now();
		return m;
	}

	//------------------------------------------------------------------------------------------------
	//! The pieces of `f`'s kit a character wears (VERIFY: worn items stay on a corpse long enough to
	//! be read when the death is seen).
	protected void WornOf(IEntity character, int f, notnull array<IEntity> result)
	{
		if (!character || !RZ_Factions.IsValid(f))
			return;

		m_Disguise.WornPieces(character, f, result);
	}

	//------------------------------------------------------------------------------------------------
	//! From retaliation: one of `f`'s people died (any cause). His name goes on the nail (or in the
	//! book, or on the authors' list), and what he wore is a dead man's kit. killerId: the player whose
	//! shot it was (no infected within 3 m), else -1.
	void OnPersonDied(IEntity body, int f, vector at, int killerId)
	{
		if (f != RZ_EFaction.MILITARY && f != RZ_EFaction.BANDITS && f != RZ_EFaction.INSTITUTE)
			return;

		string killer = "";
		int how = HOW_INFECTED;
		if (killerId > 0)
		{
			killer = m_GM.GetProfile(killerId).m_sIdentity;
			how = HOW_KILLED;
		}

		RZ_NailSave nail = AddNail(f, at, how, killer);
		if (!m_Disguise.MarksOn())
			return;

		array<IEntity> pieces = {};
		WornOf(body, f, pieces);
		foreach (IEntity piece : pieces)
		{
			RZ_KitMark m = Mark(piece, DEAD, f, nail.name, killer);
			if (m)
				m.m_iNail = nail.id;
		}

		if (f == RZ_EFaction.INSTITUTE && !pieces.IsEmpty())
			m_Data.coatsOut = m_Data.coatsOut + 1;

		if (!pieces.IsEmpty())
			RZ_Log.Info(string.Format("Disguise: %1 pieces of %2's kit marked as %3's (a dead man's)", pieces.Count(), RZ_Factions.GetName(f), nail.name));
	}

	//------------------------------------------------------------------------------------------------
	//! From the spawner's kill count: an infected died at `pos`. Kit of a faction still on it is off
	//! the turned. VERIFY: an infected's body keeps its clothes in its loadout slots after death.
	void OnInfectedKilled(vector pos)
	{
		if (!m_Disguise.MarksOn())
			return;

		array<IEntity> bodies = {};
		RZ_CorpseQuery.FindDead(pos, 3, bodies);
		int marked = 0;
		foreach (IEntity body : bodies)
		{
			if (RZ_Util.IsCharacterAlive(body) || GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(body) > 0)
				continue;

			for (int f = 0; f < RZ_Factions.COUNT; f++)
			{
				array<IEntity> pieces = {};
				WornOf(body, f, pieces);
				foreach (IEntity piece : pieces)
				{
					RZ_KitMark old = Find(piece);
					if (old && old.m_iKind == DEAD)
						continue;

					Mark(piece, TURNED, f, "", "");
					marked++;
				}
			}
		}

		if (marked > 0)
			RZ_Log.Info(string.Format("Disguise: %1 pieces of kit off the turned marked", marked));
	}

	//------------------------------------------------------------------------------------------------
	//! From a quartermaster's sale: kit sold by its own faction is signed for to the buyer; anything
	//! the Red Hand sells is bought fair. Walks the item and what it holds (a set in a bag).
	void OnSold(IEntity item, int buyerId, int shopFaction)
	{
		if (!item || buyerId <= 0 || !m_Disguise.MarksOn())
			return;

		RZ_PlayerProfile p = m_GM.GetProfile(buyerId);
		array<IEntity> items = {item};
		IEntity child = item.GetChildren();
		while (child)
		{
			items.Insert(child);
			child = child.GetSibling();
		}

		int marked = 0;
		foreach (IEntity e : items)
		{
			if (shopFaction == RZ_EFaction.BANDITS)
			{
				int kitOf = m_Disguise.KitFactionOf(e);
				if (kitOf >= 0 && Mark(e, FAIR, kitOf, "", p.m_sIdentity))
					marked++;

				continue;
			}

			if (m_Disguise.IsKitOf(e, shopFaction) && Mark(e, SIGNED, shopFaction, RZ_Util.PlayerName(buyerId), p.m_sIdentity))
				marked++;
		}

		if (marked > 0)
			RZ_Log.Info(string.Format("Disguise: %1 pieces sold by %2 to %3 (%4)", marked, RZ_Factions.GetName(shopFaction), RZ_Util.PlayerName(buyerId), SoldWord(shopFaction)));
	}

	//------------------------------------------------------------------------------------------------
	protected static string SoldWord(int shopFaction)
	{
		if (shopFaction == RZ_EFaction.BANDITS)
			return "bought fair";

		return "signed out to them";
	}

	//------------------------------------------------------------------------------------------------
	//! The Red Hand's armband or the Haven's ribbon, given to this player at Trusted.
	void OnGiven(IEntity item, int playerId, int f)
	{
		if (!item || playerId <= 0)
			return;

		Mark(item, GIVEN, f, RZ_Util.PlayerName(playerId), m_GM.GetProfile(playerId).m_sIdentity);
	}

	//------------------------------------------------------------------------------------------------
	//! What kind of `f`'s kit this character wears, the worst piece first: off the turned, a dead
	//! man's (not bought fair), signed out to someone else, issued to them, else clean. `mark` is the
	//! piece that decided it (null for clean).
	int KindOf(IEntity character, int playerId, int f, out RZ_KitMark mark)
	{
		mark = null;
		array<IEntity> pieces = {};
		WornOf(character, f, pieces);
		if (pieces.IsEmpty())
			return KIT_NONE;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		int worst = KIT_CLEAN;
		foreach (IEntity piece : pieces)
		{
			RZ_KitMark m = Find(piece);
			if (!m)
				continue;

			int kind = KIT_CLEAN;
			if (m.m_iKind == TURNED)
				kind = KIT_TURNED;
			else if (m.m_iKind == DEAD)
				kind = KIT_DEAD;
			else if (m.m_iKind == SIGNED && m.m_sOwner != ident)
				kind = KIT_SIGNED_OTHER;
			else if (m.m_iKind == SIGNED)
				kind = KIT_ISSUED;

			if (kind > worst)
			{
				worst = kind;
				mark = m;
			}
		}

		return worst;
	}

	//------------------------------------------------------------------------------------------------
	//! A worn red armband the Red Hand gave someone else (or nobody): a tell at the Red Hand's look.
	bool WearsGivenOther(IEntity character, int playerId, int f)
	{
		array<IEntity> pieces = {};
		WornOf(character, f, pieces);
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		foreach (IEntity piece : pieces)
		{
			string name = RZ_Util.GetPrefabName(piece);
			name.ToLower();
			if (!name.Contains("armband"))
				continue;

			RZ_KitMark m = Find(piece);
			if (!m || m.m_iKind != GIVEN || m.m_sOwner != ident)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A worn amber ribbon the Haven gave someone else: the Haven won't talk to you in it. Worn means
	//! attached to the character itself (VERIFY: worn clothing's parent is the character, and a pack's
	//! contents' parent is the pack).
	bool WearsOthersRibbon(IEntity character, int playerId)
	{
		if (!character)
			return false;

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		foreach (RZ_KitMark m : m_aMarks)
		{
			if (m.m_Item && m.m_iKind == GIVEN && m.m_iFaction == RZ_EFaction.SURVIVORS && m.m_sOwner != ident && m.m_Item.GetParent() == character)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! A worn piece, put on: what the wearer notices about it ("" nothing). Owner-only.
	static string PutOnLine(RZ_KitMark m)
	{
		if (!m)
			return "";

		switch (m.m_iKind)
		{
			case TURNED:
			{
				if (m.m_iFaction == RZ_EFaction.MILITARY)
					return "Bastion green, burnt at the cuff. Off one of the turned.";

				if (m.m_iFaction == RZ_EFaction.INSTITUTE)
					return "A numbered coat, stiff with something. Off one of the turned.";

				return "Red cloth gone brown at the edges. Off one of the turned.";
			}
			case DEAD:
			{
				string surname = Surname(m.m_sName);
				surname.ToUpper();
				if (m.m_iFaction == RZ_EFaction.MILITARY)
					return string.Format("The name tape says %1.", surname);

				if (m.m_iFaction == RZ_EFaction.INSTITUTE)
					return string.Format("It's numbered, and the number is signed out to %1.", m.m_sName);

				return "It's still warm in places. It hasn't been through the market.";
			}
			case SIGNED:
				return string.Format("A stamp inside the collar: signed out to %1.", m.m_sName);
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The last word of a name ("Private Emil Kral" -> "Kral").
	static string Surname(string name)
	{
		array<string> parts = {};
		name.Split(" ", parts, true);
		if (parts.IsEmpty())
			return name;

		return parts[parts.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! Their line at a look, for kit that fails it.
	static string FailLine(int kind, RZ_KitMark m, int f)
	{
		if (kind == KIT_TURNED)
		{
			if (f == RZ_EFaction.INSTITUTE)
				return "\"That coat was on one of the turned. We counted it lost.\"";

			return "\"That's off one of ours. Where is he?\"";
		}

		if (kind == KIT_DEAD && m)
		{
			if (f == RZ_EFaction.MILITARY)
				return string.Format("\"%1? %1 went on the nail on Day %2. Hands on the barrier.\"", Surname(m.m_sName), DayOf(m));

			if (f == RZ_EFaction.INSTITUTE)
				return string.Format("\"That coat is signed out to %1, and %1 isn't coming back.\"", m.m_sName);

			return "\"That's his jacket. It hasn't been through the market.\"";
		}

		if (kind == KIT_SIGNED_OTHER && m)
			return string.Format("\"Signed out to %1. You're not %1.\"", m.m_sName);

		return "";
	}

	//------------------------------------------------------------------------------------------------
	protected static int DayOf(RZ_KitMark m)
	{
		RZ_GameModeComponent gm = RZ_GameModeComponent.GetInstance();
		if (!gm || !gm.GetHostility())
			return RZ_SeasonClock.Day();

		RZ_NailSave n = gm.GetHostility().GetDisguise().GetMarks().FindNail(m.m_iNail);
		if (!n)
			return RZ_SeasonClock.Day();

		return n.day;
	}

	//------------------------------------------------------------------------------------------------
	//! Bastion's search: a dead man's kit and kit signed out to someone else go in the crate. The
	//! buyer of signed kit hears it came in. The line said, "" if nothing was taken.
	string TakeAtSearch(IEntity character, int playerId)
	{
		if (!character || !m_Disguise.MarksOn())
			return "";

		array<IEntity> pieces = {};
		WornOf(character, RZ_EFaction.MILITARY, pieces);
		SCR_InventoryStorageManagerComponent inv = SCR_InventoryStorageManagerComponent.Cast(character.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inv)
			return "";

		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		string line = "";
		int taken = 0;
		foreach (IEntity piece : pieces)
		{
			RZ_KitMark m = Find(piece);
			if (!m || (m.m_iKind != DEAD && !(m.m_iKind == SIGNED && m.m_sOwner != ident)))
				continue;

			int kind = m.m_iKind;
			string name = m.m_sName;
			string owner = m.m_sOwner;
			if (!inv.TryDeleteItem(piece)) // VERIFY: deleting a worn item (the same call as confiscation)
				continue;

			taken++;
			if (kind == DEAD)
			{
				line = string.Format(" \"That's %1's.\" It goes in a box for when the gates open.", Surname(name));
				continue;
			}

			line = string.Format(" \"Army property, signed out to %1. They'll be hearing from us.\"", name);
			int buyer = m_GM.FindPlayerByIdentity(owner);
			if (buyer > 0)
				m_GM.NotifyPlayer(buyer, RZ_Factions.GetName(RZ_EFaction.MILITARY), "Kit signed out to you turned up on somebody else at a checkpoint. It's back in the stores.");
		}

		if (taken > 0)
			RZ_Log.Info(string.Format("Disguise: Bastion's search took %1 marked pieces from %2", taken, RZ_Util.PlayerName(playerId)));

		return line;
	}

	// ================================================================ The nail list
	//------------------------------------------------------------------------------------------------
	//! A seeded name for the dead (never LORE's or the finale's canon people).
	protected string SeededName(int f, int n)
	{
		RZ_Rng rng = new RZ_Rng(RZ_WorldFacts.Get().GetSeed() + n * 977 + f * 31 + 5);
		for (int i = 0; i < 4; i++)
		{
			string name = RZ_FactionVoice.PersonName(rng, f);
			if (!name.Contains("Fiala"))
				return name;
		}

		return RZ_FactionVoice.PersonName(rng, f) + " the younger";
	}

	//------------------------------------------------------------------------------------------------
	protected RZ_NailSave AddNail(int f, vector at, int how, string killer)
	{
		m_Data.nailId = m_Data.nailId + 1;
		RZ_NailSave n = new RZ_NailSave();
		n.id = m_Data.nailId;
		n.f = f;
		n.name = SeededName(f, n.id);
		if (f == RZ_EFaction.INSTITUTE)
			n.name = string.Format("%1, coat %2", n.name, 10 + n.id % 40);

		n.day = RZ_SeasonClock.Day();
		n.x = at[0];
		n.z = at[2];
		n.how = how;
		n.killer = killer;
		RZ_Territory territory = m_GM.GetTerritory();
		if (territory && territory.IsActive())
		{
			RZ_Holding h = territory.FindHolding(f, at, 2500);
			if (h)
			{
				n.holding = h.m_iId;
				n.place = h.m_sName;
			}
		}

		if (n.place.IsEmpty())
		{
			RZ_Settlement s = m_GM.GetSettlement(m_GM.FindNearestSettlement(at));
			if (s)
				n.place = "near " + s.m_sName;
		}

		m_Data.nails.Insert(n);
		int count = 0;
		for (int i = m_Data.nails.Count() - 1; i >= 0; i--)
		{
			if (m_Data.nails[i].f != f)
				continue;

			count++;
			if (count > MAX_NAILS)
				m_Data.nails.Remove(i);
		}

		RZ_Log.Info(string.Format("Disguise: %1 on %2's list (%3, day %4)", n.name, RZ_Factions.GetName(f), n.place, n.day));
		return n;
	}

	//------------------------------------------------------------------------------------------------
	RZ_NailSave FindNail(int id)
	{
		foreach (RZ_NailSave n : m_Data.nails)
		{
			if (n.id == id)
				return n;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! `f`'s dead still lying out there: died within `hours` of play... as season days: today or
	//! `days` back, within `radius` of pos, his kit not yet home or sold. Newest first.
	void OutThere(int f, vector pos, float radius, int days, notnull array<RZ_NailSave> result)
	{
		int today = RZ_SeasonClock.Day();
		for (int i = m_Data.nails.Count() - 1; i >= 0; i--)
		{
			RZ_NailSave n = m_Data.nails[i];
			if (n.f != f || n.how >= HOW_HOME || today - n.day > days)
				continue;

			if (RZ_Util.DistXZ(Vector(n.x, 0, n.z), pos) <= radius)
				result.Insert(n);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! His tag came home (HOW_HOME) or his kit went to the fence (HOW_LOST).
	void SetFate(int nailId, int how)
	{
		RZ_NailSave n = FindNail(nailId);
		if (!n)
			return;

		n.how = how;
		if (how == HOW_HOME)
		{
			n.read = false; // read again on the call: the jacket came back
			if (n.f == RZ_EFaction.INSTITUTE)
				m_Data.coatsBack = m_Data.coatsBack + 1;
			else
				m_Data.jacketsHome = m_Data.jacketsHome + 1;
		}

		RZ_Log.Info(string.Format("Disguise: %1 marked %2", n.name, FateWord(how)));
	}

	//------------------------------------------------------------------------------------------------
	static string FateWord(int how)
	{
		if (how == HOW_HOME)
			return "home";

		if (how == HOW_LOST)
			return "kit lost";

		return "out there";
	}

	//------------------------------------------------------------------------------------------------
	//! A coat came back into the record (Masks Off, Notebook 31).
	void CoatBack()
	{
		m_Data.coatsBack = m_Data.coatsBack + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! A coat went out of the record (Masks Off's coat taken).
	void CoatOut()
	{
		m_Data.coatsOut = m_Data.coatsOut + 1;
	}

	//------------------------------------------------------------------------------------------------
	//! The names not read yet on `f`'s list (the nightly call; the change of shift), marked read.
	void TakeUnread(int f, notnull array<string> result)
	{
		foreach (RZ_NailSave n : m_Data.nails)
		{
			if (n.f != f || n.read)
				continue;

			n.read = true;
			string fate = "";
			if (n.how == HOW_HOME)
				fate = ", jacket home";
			else if (n.how == HOW_LOST)
				fate = ", kit lost";

			result.Insert(string.Format("%1, %2%3", n.name, n.place, fate));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "Coats returned: 7 of 9." "" before any coat went out.
	string CoatCount()
	{
		if (m_Data.coatsOut <= 0)
			return "";

		return string.Format("\"Coats returned: %1 of %2.\"", Math.Min(m_Data.coatsBack, m_Data.coatsOut), m_Data.coatsOut);
	}

	//------------------------------------------------------------------------------------------------
	//! A name of `f`'s dead, read on the air (for a cover detail), "" none.
	string LatestName(int f)
	{
		for (int i = m_Data.nails.Count() - 1; i >= 0; i--)
		{
			if (m_Data.nails[i].f == f && m_Data.nails[i].read)
				return m_Data.nails[i].name;
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! The holding a name on the nail served at, -1 none (his own post: a dead man's name fails there).
	int HoldingOfName(int f, string name)
	{
		foreach (RZ_NailSave n : m_Data.nails)
		{
			if (n.f == f && n.name == name)
				return n.holding;
		}

		return -1;
	}

	// ================================================================ The board and the counts
	//------------------------------------------------------------------------------------------------
	//! A catch goes on the season's counts, and a Bastion catch on the board at the wire.
	void OnCaught(int playerId, int f, string place, string kit, string named)
	{
		if (!RZ_Factions.IsValid(f))
			return;

		m_Data.catches[f] = m_Data.catches[f] + 1;
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		if (!m_Data.caughtIdents.Contains(ident) && m_Data.caughtIdents.Count() < MAX_LOOKERS)
			m_Data.caughtIdents.Insert(ident);

		if (f != RZ_EFaction.MILITARY)
			return;

		RZ_ImpostorSave b = new RZ_ImpostorSave();
		b.f = f;
		b.day = RZ_SeasonClock.Day();
		b.place = place;
		b.kit = kit;
		b.named = named;
		m_Data.board.Insert(b);
		while (m_Data.board.Count() > MAX_BOARD)
		{
			m_Data.board.RemoveOrdered(0);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! A look passed (the season's count of looks passed by identity).
	void OnPassed(int playerId)
	{
		string ident = m_GM.GetProfile(playerId).m_sIdentity;
		int i = m_Data.lookIdents.Find(ident);
		if (i >= 0)
		{
			m_Data.lookCounts[i] = m_Data.lookCounts[i] + 1;
			return;
		}

		if (m_Data.lookIdents.Count() >= MAX_LOOKERS)
			return;

		m_Data.lookIdents.Insert(ident);
		m_Data.lookCounts.Insert(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Catches by `f` this season.
	int Catches(int f)
	{
		if (!RZ_Factions.IsValid(f))
			return 0;

		return m_Data.catches[f];
	}

	//------------------------------------------------------------------------------------------------
	//! The newest line on Bastion's board, read at the quartermaster or the sentry; "" none.
	string BoardLine()
	{
		if (m_Data.board.IsEmpty())
			return "";

		RZ_ImpostorSave b = m_Data.board[m_Data.board.Count() - 1];
		string named = "no name";
		if (!b.named.IsEmpty())
			named = "the name " + b.named;

		return string.Format("The board at the wire: \"Day %1, %2. Civilian in our kit: %3. Gave %4. Impostors this season: %5.\"", b.day, b.place, b.kit, named, m_Data.board.Count());
	}
}
