//! Talk that writes into the world's stories (docs/DIALOGUE.md, "What a choice does"), through the
//! systems that own them. Server only.
//!
//! - `set:<key>=N`: a world flag of the scene's (`talk.<key>`, read back as `talk:<key>`). Only in a
//!   conversation with a scene, so it runs once for everyone.
//! - `qset:<questline>.<flag>=N`: a running questline's own flag. Scene only.
//! - `begin:<questline>`: the questline's own TryBegin, with its own rules. Scene only.
//! - `note:<questline>:"<text>"`: a line in that run's journal notes (what you know so far).
//! - `approach:<questline>:<way>:"<what>"`: this player took part that way (talk, persuade, sneak,
//!   force, bribe, betray, side, press, trade, trick, leave), as the run remembers its people.
//! - `test:<claim>:<hour>:<metres>`: a claim the world tests (the owner's own check, not a rumour
//!   round): at the next game <hour> (20.5 is 20:30), if the player is within <metres> of where they
//!   told it, it was false, and this speaker remembers that (`nmem:<claim>.false`). Not saved: a
//!   restart forgets a test still waiting.
//! Anything physical, and a stage's outcome, stays the stage's own: a conversation at a stage's person
//! picks its built choice with call:slot:N.

//------------------------------------------------------------------------------------------------
class RZ_DClaimTest
{
	int m_iPlayer;
	string m_sIdent;
	string m_sNote;			// "speaker|claim.false"
	vector m_vAt;
	float m_fHour;
	float m_fMetres;
	float m_fWaited;
	float m_fLastHour;
}

//------------------------------------------------------------------------------------------------
class RZ_DialogueStory
{
	protected static ref array<ref RZ_DClaimTest> s_aTests = {};
	protected static const float TEST_MAX_SECONDS = 43200;
	//------------------------------------------------------------------------------------------------
	static void Effect(RZ_TalkSession s, string name, string arg)
	{
		RZ_DCtx ctx = s.m_Tree.m_Ctx;
		RZ_GameModeComponent gm = ctx.m_GM;
		switch (name)
		{
			case "set":
			{
				RZ_WorldFacts facts = gm.GetFacts();
				int eq = arg.IndexOf("=");
				if (!facts || eq <= 0)
					return;

				string key = RZ_DialogueTrees.Scoped(ctx, arg.Substring(0, eq));
				facts.SetFlag("talk." + key, arg.Substring(eq + 1, arg.Length() - eq - 1).ToInt());
				return;
			}
			case "qset":
			{
				int qeq = arg.IndexOf("=");
				int dot = arg.IndexOf(".");
				if (qeq <= 0 || dot <= 0 || dot > qeq)
					return;

				RZ_QuestRun qrun = Run(gm, arg.Substring(0, dot));
				if (qrun)
					qrun.SetFlag(arg.Substring(dot + 1, qeq - dot - 1), arg.Substring(qeq + 1, arg.Length() - qeq - 1).ToInt());

				return;
			}
			case "begin":
			{
				if (gm.GetQuestlines() && gm.GetQuestlines().TryBegin(arg))
					RZ_Log.Info(string.Format("Dialogue: %1 began %2", RZ_Util.PlayerName(s.m_iPlayer), arg));

				return;
			}
			case "note":
			{
				int colon = arg.IndexOf(":");
				if (colon <= 0)
					return;

				RZ_QuestRun nrun = Run(gm, arg.Substring(0, colon));
				string text = arg.Substring(colon + 1, arg.Length() - colon - 1);
				text.Replace("\"", "");
				if (nrun && !text.IsEmpty())
					nrun.Note(RZ_DialogueText.Fill(ctx, text));

				return;
			}
			case "test":
			{
				array<string> bits = {};
				arg.Split(":", bits, false);
				if (bits.Count() < 3)
					return;

				RZ_DClaimTest t = new RZ_DClaimTest();
				t.m_iPlayer = s.m_iPlayer;
				t.m_sIdent = gm.GetProfile(s.m_iPlayer).m_sIdentity;
				t.m_sNote = ctx.m_sKey + "|" + bits[0] + ".false";
				t.m_vAt = ctx.Pos();
				t.m_fHour = bits[1].ToFloat();
				t.m_fMetres = bits[2].ToFloat();
				t.m_fLastHour = RZ_GameTime.GetHour();
				s_aTests.Insert(t);
				return;
			}
			case "approach":
			{
				array<string> parts = {};
				arg.Split(":", parts, false);
				if (parts.Count() < 2)
					return;

				RZ_QuestRun arun = Run(gm, parts[0]);
				string what = "";
				if (parts.Count() >= 3)
					what = parts[2];

				what.Replace("\"", "");
				int way = Way(parts[1]);
				if (arun && way >= 0)
					arun.Did(gm, s.m_iPlayer, way, what);

				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! On the dialogue tick: tests whose hour has come.
	static void Tick(RZ_GameModeComponent gm, float dt)
	{
		if (s_aTests.IsEmpty())
			return;

		float hour = RZ_GameTime.GetHour();
		RZ_DialogueTrees trees = RZ_DialogueTrees.Get();
		for (int i = s_aTests.Count() - 1; i >= 0; i--)
		{
			RZ_DClaimTest t = s_aTests[i];
			t.m_fWaited += dt;
			bool due = (t.m_fLastHour < t.m_fHour && hour >= t.m_fHour) || (t.m_fLastHour > hour && hour >= t.m_fHour);
			t.m_fLastHour = hour;
			if (!due)
			{
				if (t.m_fWaited > TEST_MAX_SECONDS)
					s_aTests.Remove(i);

				continue;
			}

			IEntity me = GetGame().GetPlayerManager().GetPlayerControlledEntity(t.m_iPlayer);
			if (me && trees && gm.GetProfile(t.m_iPlayer).m_sIdentity == t.m_sIdent && RZ_Util.DistXZ(me.GetOrigin(), t.m_vAt) <= t.m_fMetres)
			{
				trees.GetMemory().Add(t.m_iPlayer, RZ_DialogueMemory.NOTES, t.m_sNote);
				RZ_Log.Info(string.Format("Dialogue: claim %1 tested false for %2", t.m_sNote, RZ_Util.PlayerName(t.m_iPlayer)));
			}

			s_aTests.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static RZ_QuestRun Run(RZ_GameModeComponent gm, string key)
	{
		if (!gm.GetQuestlines())
			return null;

		return gm.GetQuestlines().FindRunByKey(key);
	}

	//------------------------------------------------------------------------------------------------
	//! A way's word as the questlines' approach (RZ_EQuestApproach), -1 unknown.
	static int Way(string word)
	{
		switch (word)
		{
			case "talk": return RZ_EQuestApproach.TALK;
			case "persuade": return RZ_EQuestApproach.PERSUADE;
			case "sneak": return RZ_EQuestApproach.SNEAK;
			case "force": return RZ_EQuestApproach.FORCE;
			case "bribe": return RZ_EQuestApproach.BRIBE;
			case "betray": return RZ_EQuestApproach.BETRAY;
			case "side": return RZ_EQuestApproach.SIDE;
			case "press": return RZ_EQuestApproach.PRESS;
			case "trade": return RZ_EQuestApproach.TRADE;
			case "trick": return RZ_EQuestApproach.TRICK;
			case "leave": return RZ_EQuestApproach.LEAVE;
		}

		return -1;
	}
}
