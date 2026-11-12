import sys
D=sys.argv[1]
def sub(p,a,b,n=1):
    s=open(p).read(); assert s.count(a)==n,(p,a); open(p,'w').write(s.replace(a,b))
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
# The stand reads who stands in the clean room (and at the bay) from their entity: the fake player
# has no engine id
sub(D+'/Cure/RZ_CureFinale.c','''			int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			string ident = Ident(pid);''','''			int pid = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(p);
			if (p == RZT_Fake.s_Ent)
				pid = RZT_Fake.ID;
			string ident = Ident(pid);''', 2)
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
