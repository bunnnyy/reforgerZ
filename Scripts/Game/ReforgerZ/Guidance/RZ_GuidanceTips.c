//! Contextual tips (docs/DESIGN.md, "New-player guidance"): when something happens to a
//! player, the nearest plausible person says one line about it, in their own voice.
//!
//! The table is data: RZ_GameModeComponent > Guidance > Tips. Leave it empty for the
//! built-in defaults below, or copy and edit them. A line may use %1 and %2, which the
//! trigger fills in (see the id list on m_sId). An empty line means "this kind of
//! speaker has nothing to say about it": the next one in line is tried.

[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sId")]
class RZ_GuidanceTip
{
	[Attribute("", desc: "Trigger id. Built in: bitten (%1 nearest safehouse town), special and samples (%1 the lab, place and direction), tier (%1 faction, %2 tier), watched and hunted (%1 faction), perimeter, loud, night, safehouse and shortage (%1 town), clue. Hooks for other systems: hungry, car, channel (RZ_Guidance.Report)")]
	string m_sId;

	[Attribute("0", desc: "Critical: past its level, a veteran still gets the short line")]
	bool m_bCritical;

	[Attribute("2", UIWidgets.Slider, "Said in full up to this guidance level (0 brand new, 1 learning, 2 familiar, 3 veteran)", "0 3 1")]
	int m_iMaxLevel = 2;

	[Attribute("0", desc: "Once per player (until a refresher brings guidance back), not once per life")]
	bool m_bOnce;

	[Attribute("20", desc: "Minutes before the same player can hear it again in a later life")]
	float m_fCooldownMinutes = 20;

	[Attribute("", UIWidgets.EditBoxMultiline, "A Haven leader or safehouse occupant says")]
	string m_sHaven;

	[Attribute("", UIWidgets.EditBoxMultiline, "A Bastion leader says")]
	string m_sBastion;

	[Attribute("", UIWidgets.EditBoxMultiline, "A Red Hand leader says")]
	string m_sRedHand;

	[Attribute("", UIWidgets.EditBoxMultiline, "A Halcyon leader says")]
	string m_sHalcyon;

	[Attribute("", UIWidgets.EditBoxMultiline, "A wanderer says")]
	string m_sWanderer;

	[Attribute("", UIWidgets.EditBoxMultiline, "Heard on the radio (only if the player carries one)")]
	string m_sRadio;

	[Attribute("", UIWidgets.EditBoxMultiline, "Plain hint when nobody is around")]
	string m_sPlain;

	[Attribute("", UIWidgets.EditBoxMultiline, "Short line for veterans (critical tips only)")]
	string m_sShort;

	//------------------------------------------------------------------------------------------------
	string GetFactionLine(int faction)
	{
		switch (faction)
		{
			case RZ_EFaction.SURVIVORS: return m_sHaven;
			case RZ_EFaction.MILITARY: return m_sBastion;
			case RZ_EFaction.BANDITS: return m_sRedHand;
			case RZ_EFaction.INSTITUTE: return m_sHalcyon;
		}

		return "";
	}
}

//! The built-in tip table, written to the voices in docs/LORE.md.
class RZ_GuidanceTips
{
	//------------------------------------------------------------------------------------------------
	protected static RZ_GuidanceTip Add(notnull array<ref RZ_GuidanceTip> list, string id, bool critical, int maxLevel, bool once, float cooldown)
	{
		RZ_GuidanceTip t = new RZ_GuidanceTip();
		t.m_sId = id;
		t.m_bCritical = critical;
		t.m_iMaxLevel = maxLevel;
		t.m_bOnce = once;
		t.m_fCooldownMinutes = cooldown;
		list.Insert(t);
		return t;
	}

	//------------------------------------------------------------------------------------------------
	static void CreateDefaults(notnull array<ref RZ_GuidanceTip> list)
	{
		RZ_GuidanceTip t;

		t = Add(list, "bitten", true, 2, false, 0);
		t.m_sHaven = "\"Let me see. Oh, love. Go to our medic, now, and tell her straight. She can slow it. Hiding a bite is the one thing we don't forgive.\"";
		t.m_sBastion = "\"You're bitten. Procedure: an antidote if you can get one, Halcyon makes them. Otherwise report to a Haven medic. Don't make me use the other procedure.\"";
		t.m_sRedHand = "\"Bitten? Shame. The eggheads' antidotes fall off the back of trucks all the time. Ours have hardly been used. Bring Favor.\"";
		t.m_sHalcyon = "\"That's a bite. You have minutes, not hours. An antidote stops it, and the lab makes them when the research allows. A Haven medic can slow it. I'm sorry.\"";
		t.m_sWanderer = "\"That's a bite. Don't hide it. The safehouse in %1 has a medic, or find the Doctor on the roads. Go now.\"";
		t.m_sRadio = "\"...Haven, open channel. Anyone bitten: come to %1 and say so at the door. The medic can slow it. We can tell when people lie.\"";
		t.m_sPlain = "That's a bite. An antidote cures it. A safehouse medic (%1) slows it, if you tell her.";
		t.m_sShort = "Bitten: find an antidote, or tell a safehouse medic.";

		t = Add(list, "special", false, 1, true, 0);
		t.m_sHaven = "\"Was that one of the changed ones? Leave the rest, but take the sample. The scientists in %1 will want it, and so will everyone else.\"";
		t.m_sBastion = "\"Confirmed kill on a special. Secure the sample. Halcyon's lab is in %1. Do not sell it to the Red Hand.\"";
		t.m_sRedHand = "\"Nice. Don't waste that sample on the eggheads in %1. We pay faster.\"";
		t.m_sHalcyon = "\"A special. Its tissue is what the research runs on. Bring the sample to the lab in %1, please. Every one counts.\"";
		t.m_sWanderer = "\"Big one, that. Grab whatever dropped. The lab in %1 pays in hope, the Red Hand in tins.\"";
		t.m_sRadio = "\"...samples from the large ones are wanted at the lab in %1. Halcyon thanks you. Halcyon out.\"";
		t.m_sPlain = "The changed ones drop samples. The lab in %1 turns them into cure research.";

		t = Add(list, "samples", false, 2, false, 30);
		t.m_sHaven = "\"You're carrying samples? Take them to the lab in %1. And don't let the Red Hand see them.\"";
		t.m_sBastion = "\"Biological material goes to the Halcyon lab in %1. That's an order. Well. Advice.\"";
		t.m_sRedHand = "\"Those samples are worth more at our dead drop than at the lab in %1. Just saying.\"";
		t.m_sHalcyon = "\"Those go to the lab in %1. Keep them cold if you can. You can't. Just hurry.\"";
		t.m_sWanderer = "\"Is that a sample? The lab's in %1. Stay off the main road with it.\"";
		t.m_sRadio = "\"...Halcyon to anyone carrying samples: the lab in %1 is receiving. Halcyon out.\"";
		t.m_sPlain = "Samples in your pack. The lab is in %1.";

		t = Add(list, "hungry", false, 2, false, 30);
		t.m_sHaven = "\"You look hungry. Sit. There's soup, and the quartermaster takes tins for the store if you find any.\"";
		t.m_sBastion = "\"Rations are for personnel. The Haven feeds civilians. Try %1.\"";
		t.m_sRedHand = "\"Hungry? Everything's for sale. Even soup.\"";
		t.m_sHalcyon = "\"Eat something. Low blood sugar makes people careless, and careless people get bitten.\"";
		t.m_sWanderer = "\"When did you last eat? Cellars and kitchen cupboards. Tins keep.\"";
		t.m_sRadio = "\"...there's soup at %1 tonight. Bring a tin for the next person.\"";
		t.m_sPlain = "You're hungry. Tins turn up in cellars and kitchens, and the Haven feeds people at %1.";

		t = Add(list, "tier", false, 2, true, 0);
		t.m_sHaven = "\"People are starting to say your name nicely. %1 calls you %2 now. The quartermaster will open the better shelves.\"";
		t.m_sBastion = "\"Your file's been updated. %1: %2. Some doors open.\"";
		t.m_sRedHand = "\"Look at you, %2 with %1. Don't let it go to your head. The shops notice, though.\"";
		t.m_sHalcyon = "\"Your standing with %1 is now %2. It changes what their quartermaster will give you.\"";
		t.m_sWanderer = "\"Word gets round. %1 calls you %2 now. Their shop will treat you better.\"";
		t.m_sPlain = "%1 calls you %2 now. Their quartermaster has more for you.";

		t = Add(list, "watched", false, 2, false, 30);
		t.m_sHaven = "\"%1 has your name on a list. It happens. Do something for them, the amends jobs, and it comes off. Don't make it worse.\"";
		t.m_sBastion = "\"You've been flagged by %1. Watched status. Amends contracts work it off. I'd take them.\"";
		t.m_sRedHand = "\"%1 is watching you. Join the club. Or do them a favour or two, if you want your name off the list.\"";
		t.m_sHalcyon = "\"%1 has noted you. Amends contracts exist for exactly this. They're the sensible option.\"";
		t.m_sWanderer = "\"%1 have been asking about you. Do them a good turn, or keep your head down.\"";
		t.m_sPlain = "Watched by %1. Amends contracts work Infamy off.";

		t = Add(list, "hunted", true, 2, false, 0);
		t.m_sHaven = "\"%1 wants you dead. I mean it. Their people will shoot on sight. Do the amends jobs, or stay well away from them.\"";
		t.m_sBastion = "\"%1 has you marked hostile. Patrols will engage. Amends contracts, or leave the area.\"";
		t.m_sRedHand = "\"%1 is hunting you. Ha. Welcome. Amends if you're soft, our protection if you're not.\"";
		t.m_sHalcyon = "\"%1 has you as a target. Their people will shoot. Amends is the only way back.\"";
		t.m_sWanderer = "\"They're out for you, %1 are. Squads. Don't go near their towns.\"";
		t.m_sPlain = "Hunted by %1: their people shoot on sight. Amends contracts work it off.";
		t.m_sShort = "Hunted by %1. Amends work it off.";

		t = Add(list, "perimeter", false, 2, true, 0);
		t.m_sHaven = "\"Don't go further that way. The line's out there. Nobody gets through. My brother tried at the bridge.\"";
		t.m_sBastion = "\"The quarantine line isn't ours. We can't get you through it. Nobody can. Turn around.\"";
		t.m_sRedHand = "\"The edge? Drones, then a loudspeaker, then a bullet. We'd sell you a way out if there was one.\"";
		t.m_sHalcyon = "\"The line won't open for anyone. Only proof that it's over will open it. That's what the research is for.\"";
		t.m_sWanderer = "\"That way's the line. You hear a drone, you turn round. You don't argue with it.\"";
		t.m_sRadio = "\"...the perimeter will be maintained. Do not approach the perimeter.\" The same words, over and over.";
		t.m_sPlain = "The quarantine line is out that way: drones, a loudspeaker, then snipers you never see. Nobody gets through.";

		t = Add(list, "loud", false, 2, false, 20);
		t.m_sHaven = "\"Shh. They'll hear you from the next village. A bottle thrown the other way does more than a rifle.\"";
		t.m_sBastion = "\"Noise discipline. You're drawing them in. Throw something to pull them off. A bottle, a firecracker.\"";
		t.m_sRedHand = "\"Loud, aren't you. Every dead thing for a kilometre is coming to say hello. Throw a bottle and walk the other way.\"";
		t.m_sHalcyon = "\"They hunt by sound, and you've made a great deal of it. A noisemaker thrown away from you will draw them off.\"";
		t.m_sWanderer = "\"Too much noise round here. They're coming. Throw something loud the other way and go quiet.\"";
		t.m_sPlain = "Too loud: they're coming. A thrown bottle or a firecracker pulls them away from you.";

		t = Add(list, "car", false, 2, true, 0);
		t.m_sHaven = "\"That car might run. A jerrycan and some patience. Mind, an engine is a dinner bell.\"";
		t.m_sBastion = "\"Vehicle looks serviceable. Fuel's scarce, and the noise carries.\"";
		t.m_sRedHand = "\"Nice wheels. Toll's on the bridge road. Ha.\"";
		t.m_sWanderer = "\"That one could run with a bit of work. They'll hear you coming, mind.\"";
		t.m_sPlain = "That car could run with fuel and a little work. Engines are loud.";

		t = Add(list, "night", false, 2, true, 0);
		t.m_sHaven = "\"It's getting dark. Stay in tonight if you can. They hear better at night, and the big ones come out.\"";
		t.m_sBastion = "\"Curfew's in effect. Patrols thin out after dark. So should you.\"";
		t.m_sRedHand = "\"Night's when the good work gets done. Quiet roads, sleepy soldiers. Watch out for the big ones.\"";
		t.m_sHalcyon = "\"At night their hearing improves and the specials hunt. It's also when the patrols are thinnest. Choose.\"";
		t.m_sWanderer = "\"Dark soon. They hear further at night, and the changed ones come out. Find a door that locks.\"";
		t.m_sRadio = "\"...Bastion, all stations: curfew is now in effect. Bastion out.\"";
		t.m_sPlain = "Night: the infected hear further and the specials come out. Patrols are thinner too.";

		t = Add(list, "channel", false, 2, true, 0);
		t.m_sRadio = "Under the static there's something else: a scrambled channel, close by. Someone is talking where they think nobody can hear.";
		t.m_sPlain = "Your radio picks up a scrambled channel nearby. Someone is talking where they think nobody can hear.";

		t = Add(list, "safehouse", false, 2, true, 0);
		t.m_sHaven = "\"Welcome to %1. House rules: guns are fine, firing them isn't. If you're bitten, tell us at the door and the medic will help. If you hide it, we'll find out.\"";
		t.m_sWanderer = "\"%1's a good house. Keep your gun slung, don't fire it, and tell them if you're bitten. They're fair if you're honest.\"";
		t.m_sPlain = "A Haven safehouse. Weapons are fine here, firing them isn't. Declare a bite at the door.";

		t = Add(list, "shortage", false, 2, false, 60);
		t.m_sHaven = "\"We're short. The store's nearly empty and people will start leaving. Anything in a tin, give it to the quartermaster.\"";
		t.m_sBastion = "\"Supply shortage. Requisitions are limited until the next delivery.\"";
		t.m_sRedHand = "\"Stock's low, so prices aren't. Supply and demand.\"";
		t.m_sHalcyon = "\"We're short of supplies. Some items won't be available until the next delivery.\"";
		t.m_sWanderer = "\"%1 is running out of food. They'd take anything you can carry.\"";
		t.m_sRadio = "\"...%1 is short of food. Anything helps. Anything.\"";
		t.m_sPlain = "%1 is short of food. Tins handed to the quartermaster keep people from leaving.";

		t = Add(list, "clue", false, 2, true, 0);
		t.m_sHaven = "\"Notes like that usually point somewhere. Have a look at your map, love, and your compass.\"";
		t.m_sBastion = "\"Treat it as intelligence. Map, compass, bearing. Then move.\"";
		t.m_sRedHand = "\"Somebody wrote that for a reason. Map and compass, genius.\"";
		t.m_sHalcyon = "\"Notes like that usually point somewhere. Check your map and compass.\"";
		t.m_sWanderer = "\"Notes like that usually point somewhere. Check your map and compass.\"";
		t.m_sPlain = "Notes like that usually point somewhere. Check your map and compass.";
	}
}
