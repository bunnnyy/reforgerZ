//! Conversation trees (docs/DIALOGUE.md): the data the generated packs fill (Dialogue/Pack/, written
//! by tools/dialogue_pack.py from dialogue/**/*.rzd), compiled once at start into typed records, and
//! indexed by speaker kind. Conditions and effects are parsed here once and never again. Server only.

//------------------------------------------------------------------------------------------------
//! One condition term: [!]key[:arg][op value].
class RZ_DTerm
{
	bool m_bNeg;
	string m_sKey;
	string m_sArg;
	ref array<string> m_aArgs = {};	// the argument split on commas (bg:doctor,nurse)
	string m_sOp;					// "", "=", "!=", ">=", "<=", ">", "<"
	string m_sVal;
	int m_iVal;
	int m_iFaction = -1;			// the faction an argument names, -1 none
}

//------------------------------------------------------------------------------------------------
//! Terms joined with &. Empty: always true.
class RZ_DCond
{
	ref array<ref RZ_DTerm> m_aTerms = {};

	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_aTerms.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	//! The first term with this key, or null.
	RZ_DTerm Find(string key)
	{
		foreach (RZ_DTerm t : m_aTerms)
		{
			if (t.m_sKey == key && !t.m_bNeg)
				return t;
		}

		return null;
	}
}

//------------------------------------------------------------------------------------------------
//! One effect: name:arg.
class RZ_DEffect
{
	string m_sName;
	string m_sArg;
}

//------------------------------------------------------------------------------------------------
class RZ_DSay
{
	ref RZ_DCond m_Cond;	// a variant (say?) when not empty: replaces the next plain line
	string m_sText;
	bool m_bJump;			// a jump: m_sText is the target node
	bool m_bDo;				// node entry effects: m_aDo
	ref array<ref RZ_DEffect> m_aDo;
	string m_sLie;			// ~lie:<key>: a lie the player is told (heard_lie:<key> once shown)
	string m_sTruth;		// ~truth:<key>: the truth (knows:<key> once shown)
}

//------------------------------------------------------------------------------------------------
class RZ_DChoice
{
	string m_sId;
	string m_sLabel;
	string m_sPass;
	string m_sShort;
	string m_sRefused;
	string m_sCheck;			// "", "persuade", "threaten", "lie", "look"
	int m_iCheckFaction = -1;
	int m_iDifficulty;
	string m_sEasier;			// a trait that makes it one easier
	string m_sHarder;			// ...one harder
	string m_sFavoured;			// backgrounds that add 1 (comma-separated ids)
	string m_sCostKind;			// "", "favor", "food"
	int m_iCostFaction = -1;
	int m_iCost;
	ref RZ_DCond m_If;
	ref RZ_DCond m_Truth;		// a [Lie] that's true while this holds: no lie, no check
	string m_sCallout;			// [Evidence]: shown only to a player who heard this lie and knows the truth
	bool m_bTellAlways;			// `tell always`: a lie that's always caught (a joke lie)
	ref array<ref RZ_DEffect> m_aFound = {};	// `found`: run when this lie is found out later
	ref array<ref RZ_DEffect> m_aDo = {};
	ref array<ref RZ_DEffect> m_aShort = {};
	ref array<ref RZ_DEffect> m_aElse = {};

	//------------------------------------------------------------------------------------------------
	//! Where a result sends the player: Short and Refused fall back to the next target named.
	string TargetFor(RZ_ECheck result)
	{
		if (result == RZ_ECheck.PASS || m_sShort.IsEmpty())
			return m_sPass;

		if (result == RZ_ECheck.SHORT || m_sRefused.IsEmpty())
			return m_sShort;

		return m_sRefused;
	}
}

//------------------------------------------------------------------------------------------------
class RZ_DNode
{
	string m_sId;
	bool m_bPublic;			// overheard: its lines and the chosen label go to others within 8 m
	bool m_bConfirm;		// a [Point of no return] node: the finale's lock card follows its lines
	ref array<ref RZ_DSay> m_aItems = {};
	ref array<ref RZ_DChoice> m_aChoices = {};
}

//------------------------------------------------------------------------------------------------
class RZ_DConv
{
	static const int FLAG_ONCE = 1;
	static const int FLAG_DAILY = 2;
	static const int FLAG_GREET = 4;
	static const int FLAG_ALWAYS = 8;	// shows even when the speaker refuses you (the finale's route topics)
	static const int FLAG_BARK = 16;

	string m_sId;
	ref array<string> m_aFor = {};
	ref RZ_DCond m_When;
	string m_sLabel;
	int m_iPrio;			// 0 quest, 1 ask, 2 world, 3 reaction, 4 flavour
	int m_iFlags;
	string m_sVoice;
	string m_sScene;		// a scene its decisions settle for the world ("" none)
	string m_sSceneScope;	// "season" (one for the server), "speaker" (one per speaker), "house" (one per safehouse), "run" (one per mission run)
	ref array<string> m_aBindNames = {};	// `bind <name> = <picker>`, picked once when the topic opens
	ref array<string> m_aBindPickers = {};
	ref RZ_DCond m_SceneWho;	// who may decide (`scene <id> <scope> who <conditions>`); null: whoever may take the choice
	float m_fOpens;			// a greet that opens the talk screen by itself within this many metres (0 never)
	ref array<ref RZ_DNode> m_aNodes = {};

	//------------------------------------------------------------------------------------------------
	RZ_DNode GetNode(string id)
	{
		foreach (RZ_DNode n : m_aNodes)
		{
			if (n.m_sId == id)
				return n;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	bool IsGreet()
	{
		return (m_iFlags & FLAG_GREET) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! One line said to a player passing by, no choices (RZ_DialogueBarks).
	bool IsBark()
	{
		return (m_iFlags & FLAG_BARK) != 0;
	}

	//------------------------------------------------------------------------------------------------
	//! The faction whose voice it is, -1 for plain and the Long Table.
	int VoiceFaction()
	{
		return RZ_DialogueDB.FactionKey(m_sVoice);
	}
}

//------------------------------------------------------------------------------------------------
//! Every conversation, compiled, indexed by speaker selector.
class RZ_DialogueDB
{
	protected ref array<ref RZ_DConv> m_aConvs = {};
	protected ref map<string, ref array<RZ_DConv>> m_mBySpeaker = new map<string, ref array<RZ_DConv>>();
	protected RZ_DConv m_Conv;		// filling
	protected RZ_DNode m_Node;
	protected int m_iNodes;
	protected int m_iBad;

	//------------------------------------------------------------------------------------------------
	void Load()
	{
		RZ_DialoguePacks.AddAll(this);
		RZ_Log.Info(string.Format("Dialogue: loaded %1 conversations, %2 nodes", m_aConvs.Count(), m_iNodes));
		if (m_iBad > 0)
			RZ_Log.Warn(string.Format("Dialogue: %1 lines in the packs couldn't be read (run tools/dialogue_pack.py)", m_iBad));
	}

	//------------------------------------------------------------------------------------------------
	int Count()
	{
		return m_aConvs.Count();
	}

	//------------------------------------------------------------------------------------------------
	int CountBarks()
	{
		int n = 0;
		foreach (RZ_DConv c : m_aConvs)
		{
			if (c.IsBark())
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	void All(notnull array<RZ_DConv> result)
	{
		foreach (RZ_DConv c : m_aConvs)
		{
			result.Insert(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! The line that tells the lie `key` (a say with ~lie:<key>), "" if none.
	string LieText(string key)
	{
		foreach (RZ_DConv c : m_aConvs)
		{
			foreach (RZ_DNode n : c.m_aNodes)
			{
				foreach (RZ_DSay item : n.m_aItems)
				{
					if (item.m_sLie == key)
						return item.m_sText;
				}
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	//! Greets that open the talk screen by themselves (RZ_DialogueOpens).
	int CountOpens()
	{
		int n = 0;
		foreach (RZ_DConv c : m_aConvs)
		{
			if (c.m_fOpens > 0)
				n++;
		}

		return n;
	}

	//------------------------------------------------------------------------------------------------
	//! Every conversation for any of these speaker selectors, once each.
	void ForSpeaker(notnull array<string> kinds, notnull array<RZ_DConv> result)
	{
		foreach (string kind : kinds)
		{
			array<RZ_DConv> list;
			if (!m_mBySpeaker.Find(kind, list))
				continue;

			foreach (RZ_DConv c : list)
			{
				if (result.Find(c) < 0)
					result.Insert(c);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	RZ_DConv Find(string id)
	{
		foreach (RZ_DConv c : m_aConvs)
		{
			if (c.m_sId == id)
				return c;
		}

		return null;
	}

	// ================================================================ Filled by the packs
	//------------------------------------------------------------------------------------------------
	void Conv(string id, string speakers, string when, string label, int prio, int flags, string voice)
	{
		RZ_DConv c = new RZ_DConv();
		c.m_sId = id;
		speakers.Split(",", c.m_aFor, true);
		c.m_When = Compile(when);
		c.m_sLabel = label;
		c.m_iPrio = prio;
		c.m_iFlags = flags;
		c.m_sVoice = voice;
		m_aConvs.Insert(c);
		m_Conv = c;
		m_Node = null;
		foreach (string kind : c.m_aFor)
		{
			array<RZ_DConv> list;
			if (!m_mBySpeaker.Find(kind, list))
			{
				list = {};
				m_mBySpeaker.Insert(kind, list);
			}

			list.Insert(c);
		}
	}

	//------------------------------------------------------------------------------------------------
	void Node(string id)
	{
		if (!m_Conv)
		{
			m_iBad++;
			return;
		}

		RZ_DNode n = new RZ_DNode();
		n.m_sId = id;
		m_Conv.m_aNodes.Insert(n);
		m_Node = n;
		m_iNodes++;
	}

	//------------------------------------------------------------------------------------------------
	void Public()
	{
		if (m_Node)
			m_Node.m_bPublic = true;
	}

	//------------------------------------------------------------------------------------------------
	void Confirm()
	{
		if (m_Node)
			m_Node.m_bConfirm = true;
	}

	//------------------------------------------------------------------------------------------------
	//! `tell always` and `found <effects>` on the last choice.
	void ChoiceMore(bool tellAlways, string found)
	{
		if (!m_Node || m_Node.m_aChoices.IsEmpty())
		{
			m_iBad++;
			return;
		}

		RZ_DChoice c = m_Node.m_aChoices[m_Node.m_aChoices.Count() - 1];
		c.m_bTellAlways = tellAlways;
		c.m_aFound = Effects(found);
	}

	//------------------------------------------------------------------------------------------------
	//! `bind <name> = <picker>` on the conversation.
	void Bind(string name, string picker)
	{
		if (!m_Conv)
		{
			m_iBad++;
			return;
		}

		m_Conv.m_aBindNames.Insert(name);
		m_Conv.m_aBindPickers.Insert(picker);
	}

	//------------------------------------------------------------------------------------------------
	//! The choice `@id` of conversation `convId`, or null.
	RZ_DChoice FindChoice(string convId, string id)
	{
		RZ_DConv c = Find(convId);
		if (!c)
			return null;

		foreach (RZ_DNode n : c.m_aNodes)
		{
			foreach (RZ_DChoice ch : n.m_aChoices)
			{
				if (ch.m_sId == id)
					return ch;
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! `opens approach <metres>`: the greet opens the talk screen by itself within that distance.
	void Opens(float metres)
	{
		if (m_Conv)
			m_Conv.m_fOpens = metres;
	}

	//------------------------------------------------------------------------------------------------
	void Scene(string id, string scope, string who = "")
	{
		if (!m_Conv)
		{
			m_iBad++;
			return;
		}

		m_Conv.m_sScene = id;
		m_Conv.m_sSceneScope = scope;
		if (!who.IsEmpty())
			m_Conv.m_SceneWho = Compile(who);
	}

	//------------------------------------------------------------------------------------------------
	void Say(string cond, string text)
	{
		if (!m_Node)
		{
			m_iBad++;
			return;
		}

		RZ_DSay s = new RZ_DSay();
		s.m_Cond = Compile(cond);
		s.m_sText = text;
		m_Node.m_aItems.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	//! The last line said is a lie the player is told (kind "lie") or the truth (kind "truth").
	void Mark(string kind, string key)
	{
		if (!m_Node || m_Node.m_aItems.IsEmpty())
		{
			m_iBad++;
			return;
		}

		RZ_DSay s = m_Node.m_aItems[m_Node.m_aItems.Count() - 1];
		if (kind == "lie")
			s.m_sLie = key;
		else
			s.m_sTruth = key;
	}

	//------------------------------------------------------------------------------------------------
	void Jump(string cond, string target)
	{
		if (!m_Node)
		{
			m_iBad++;
			return;
		}

		RZ_DSay s = new RZ_DSay();
		s.m_Cond = Compile(cond);
		s.m_sText = target;
		s.m_bJump = true;
		m_Node.m_aItems.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	void Do(string effects)
	{
		if (!m_Node)
		{
			m_iBad++;
			return;
		}

		RZ_DSay s = new RZ_DSay();
		s.m_Cond = Compile("");
		s.m_bDo = true;
		s.m_aDo = Effects(effects);
		m_Node.m_aItems.Insert(s);
	}

	//------------------------------------------------------------------------------------------------
	void Choice(string id, string label, string pass, string shortTarget, string refused, string check, string cost,
		string cond, string doFx, string shortFx, string elseFx)
	{
		if (!m_Node)
		{
			m_iBad++;
			return;
		}

		RZ_DChoice c = new RZ_DChoice();
		c.m_sId = id;
		c.m_sLabel = label;
		c.m_sPass = pass;
		c.m_sShort = shortTarget;
		c.m_sRefused = refused;
		c.m_If = Compile(cond);
		c.m_aDo = Effects(doFx);
		c.m_aShort = Effects(shortFx);
		c.m_aElse = Effects(elseFx);
		ReadCheck(c, check);
		ReadCost(c, cost);
		m_Node.m_aChoices.Insert(c);
	}

	//------------------------------------------------------------------------------------------------
	//! The last choice's options beyond the check and cost: `truth <conditions>` (a [Lie] that's true
	//! while they hold) and `callout <lie key>` ([Evidence]).
	void ChoiceExtra(string truth, string callout)
	{
		if (!m_Node || m_Node.m_aChoices.IsEmpty())
		{
			m_iBad++;
			return;
		}

		RZ_DChoice c = m_Node.m_aChoices[m_Node.m_aChoices.Count() - 1];
		if (!truth.IsEmpty())
			c.m_Truth = Compile(truth);

		c.m_sCallout = callout;
	}

	// ================================================================ Compiling
	//------------------------------------------------------------------------------------------------
	//! "persuade haven 2 easier:merciful bg:doctor,nurse"
	protected void ReadCheck(RZ_DChoice c, string check)
	{
		array<string> parts = {};
		check.Split(" ", parts, true);
		if (parts.Count() < 3)
			return;

		c.m_sCheck = parts[0];
		c.m_iCheckFaction = FactionKey(parts[1]);
		c.m_iDifficulty = parts[2].ToInt();
		for (int i = 3; i < parts.Count(); i++)
		{
			string part = parts[i];
			int colon = part.IndexOf(":");
			if (colon <= 0)
				continue;

			string key = part.Substring(0, colon);
			string val = part.Substring(colon + 1, part.Length() - colon - 1);
			if (key == "easier")
				c.m_sEasier = val;
			else if (key == "harder")
				c.m_sHarder = val;
			else if (key == "bg")
				c.m_sFavoured = val;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! "favor:haven 20" or "item:food*2"
	protected void ReadCost(RZ_DChoice c, string cost)
	{
		if (cost.IsEmpty())
			return;

		if (cost.StartsWith("favor:"))
		{
			int space = cost.IndexOf(" ");
			if (space < 0)
				return;

			c.m_sCostKind = "favor";
			c.m_iCostFaction = FactionKey(cost.Substring(6, space - 6));
			c.m_iCost = cost.Substring(space + 1, cost.Length() - space - 1).ToInt();
			return;
		}

		int star = cost.IndexOf("*");
		if (cost.StartsWith("item:food") && star > 0)
		{
			c.m_sCostKind = "food";
			c.m_iCost = cost.Substring(star + 1, cost.Length() - star - 1).ToInt();
		}
	}

	//------------------------------------------------------------------------------------------------
	static array<ref RZ_DEffect> Effects(string text)
	{
		array<ref RZ_DEffect> result = {};
		if (text.IsEmpty())
			return result;

		array<string> parts = {};
		text.Split(";", parts, true);
		foreach (string raw : parts)
		{
			string part = raw.Trim();
			if (part.IsEmpty())
				continue;

			RZ_DEffect e = new RZ_DEffect();
			int colon = part.IndexOf(":");
			if (colon < 0)
			{
				e.m_sName = part;
			}
			else
			{
				e.m_sName = part.Substring(0, colon);
				e.m_sArg = part.Substring(colon + 1, part.Length() - colon - 1);
			}

			result.Insert(e);
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	static RZ_DCond Compile(string text)
	{
		RZ_DCond cond = new RZ_DCond();
		if (text.IsEmpty())
			return cond;

		array<string> parts = {};
		text.Split("&", parts, true);
		foreach (string raw : parts)
		{
			string term = raw.Trim();
			if (term.IsEmpty())
				continue;

			cond.m_aTerms.Insert(Term(term));
		}

		return cond;
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_DTerm Term(string text)
	{
		RZ_DTerm t = new RZ_DTerm();
		string s = text;
		if (s.StartsWith("!"))
		{
			t.m_bNeg = true;
			s = s.Substring(1, s.Length() - 1);
		}

		// The operator: the first of >= <= != = > <
		int opAt = -1;
		int opLen = 0;
		for (int i = 0; i < s.Length(); i++)
		{
			string ch = s.Substring(i, 1);
			if (ch == ">" || ch == "<" || ch == "=" || ch == "!")
			{
				opAt = i;
				opLen = 1;
				if (i + 1 < s.Length() && s.Substring(i + 1, 1) == "=")
					opLen = 2;

				break;
			}
		}

		string head = s;
		if (opAt >= 0)
		{
			head = s.Substring(0, opAt);
			t.m_sOp = s.Substring(opAt, opLen);
			t.m_sVal = s.Substring(opAt + opLen, s.Length() - opAt - opLen);
			t.m_iVal = t.m_sVal.ToInt();
		}

		int colon = head.IndexOf(":");
		if (colon < 0)
		{
			t.m_sKey = head;
		}
		else
		{
			t.m_sKey = head.Substring(0, colon);
			t.m_sArg = head.Substring(colon + 1, head.Length() - colon - 1);
			t.m_sArg.Split(",", t.m_aArgs, true);
			if (!t.m_aArgs.IsEmpty())
				t.m_iFaction = FactionKey(t.m_aArgs[0]);
		}

		return t;
	}

	//------------------------------------------------------------------------------------------------
	//! haven, bastion, redhand, halcyon; anything else -1.
	static int FactionKey(string key)
	{
		if (key == "haven")
			return RZ_EFaction.SURVIVORS;
		if (key == "bastion")
			return RZ_EFaction.MILITARY;
		if (key == "redhand")
			return RZ_EFaction.BANDITS;
		if (key == "halcyon")
			return RZ_EFaction.INSTITUTE;

		return -1;
	}

	//------------------------------------------------------------------------------------------------
	static string FactionWord(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return "haven";
			case RZ_EFaction.MILITARY: return "bastion";
			case RZ_EFaction.BANDITS: return "redhand";
			case RZ_EFaction.INSTITUTE: return "halcyon";
		}

		return "none";
	}
}
