import sys
D=sys.argv[1]
def sub(p,a,b):
    s=open(p).read(); assert s.count(a)==1,(p,a); open(p,'w').write(s.replace(a,b,1))
sub(D+'/Core/RZ_Util.c','''			outIds.Insert(id);
			outEntities.Insert(ent);
		}
	}''','''			outIds.Insert(id);
			outEntities.Insert(ent);
		}
		RZT_Fake.Add(outIds, outEntities);
	}''')
sub(D+'/Core/RZ_Util.c','''	static string PlayerName(int playerId)
	{''','''	static string PlayerName(int playerId)
	{
		if (playerId == 901)
			return "Tester";''')
sub(D+'/Core/RZ_GameModeComponent.c','''		string ident;
		if (m_mIdentities.Find(playerId, ident))''','''		if (playerId == 901)
			return "rzt_901";
		string ident;
		if (m_mIdentities.Find(playerId, ident))''')
# The fake player counts as online for questlines that look a player up by identity (A Place at the Table)
sub(D+'/Director/RZ_Questlines.c','''		GetGame().GetPlayerManager().GetPlayers(all);
		foreach (int playerId : all)
		{
			if (m_GM.GetProfile(playerId).m_sIdentity == ident)''','''		GetGame().GetPlayerManager().GetPlayers(all);
		if (RZT_Fake.s_Ent)
			all.Insert(RZT_Fake.ID);
		foreach (int playerId : all)
		{
			if (m_GM.GetProfile(playerId).m_sIdentity == ident)''')
