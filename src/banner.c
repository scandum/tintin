/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2020 Igor van den Hoven                                    *
*                                                                             *
*   TinTin++ is free software; you can redistribute it and/or modify          *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*                               T I N T I N + +                               *
*                                                                             *
*                      coded by Igor van den Hoven 2009                       *
******************************************************************************/

#include "tintin.h"

#include "gui.h"

void banner_create(struct session *ses, char *name, char *arg)
{
	update_node_list(gtd->banner_list, name, "", "", "");
}


void banner_desc(struct session *ses, char *name, char *arg, char *arg1)
{
	struct listnode *node;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	node = search_node_list(gtd->banner_list, name);

	if (node)
	{
		update_node_list(gtd->banner_list, name, arg1, node->arg3, node->arg4);
	}
}

void banner_website(struct session *ses, char *name, char *arg, char *arg1)
{
	struct listnode *node;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	node = search_node_list(gtd->banner_list, name);

	if (node)
	{
		update_node_list(gtd->banner_list, name, node->arg2, arg1, node->arg4);
	}
}

void banner_address(struct session *ses, char *name, char *arg, char *arg1)
{
	struct listnode *node;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	node = search_node_list(gtd->banner_list, name);

	if (node)
	{
		update_node_list(gtd->banner_list, name, node->arg2, node->arg3, arg1);
	}
}

void banner_expires(struct session *ses, char *name, char *arg, char *arg1)
{
	struct listnode *node;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ALL);

	node = search_node_list(gtd->banner_list, name);

	if (node)
	{
		node->val64 = UMAX(0, (atoi(arg1) - 1970) * 31556926);
	}
}

void banner_init(struct session *ses, char *arg1)
{
	banner_create(ses, "Lost Souls", arg1);

	banner_desc(ses, "Lost Souls",
		"\"Lost Souls is not for the faint of heart.\" -- Net Games\n"
		"\n"
		"\"The depth of Lost Souls can be amazing.\" -- Playing MUDs On The Internet\n"
		"\n"
		"\"Have you ever come upon a place on the Net that's so incredible that you\n"
		"can't believe such entertainment is free? This MUD will blow your mind with\n"
		"its marvelous attention to detail and incredible role-playing atmosphere!\"\n"
		"\n"
		"  -- Yahoo! Wild Web Rides", arg1);

	banner_website(ses, "Lost Souls", "http://lostsouls.org", arg1);
	banner_address(ses, "Lost Souls", "ls lostsouls.org 23", arg1);
	banner_expires(ses, "Lost Souls", "2026", arg1);


	banner_create(ses, "Legends of Kallisti", arg1);

	banner_desc(ses, "Legends of Kallisti",
		"One of the longest running, most feature rich MUDs in the world with decades\n"
		"of development. Kallisti boasts a massive original world, great atmosphere of\n"
		"players, excellent combat system including group formations, ranged combat,\n"
		"optional PK and arena PvP, extensive character customization, player lineages,\n"
		"clans, customizable player houses, item crafting, extensively customizable UI,\n"
		"Mud Sound Protocol, extensive blind player support, MSDP, and so much more.\n"
		"\n"
		"This is an amazing game that you could literally play for a decade and still\n"
		"discover more - you won't be disappointed!", arg1);

	banner_website(ses, "Legends of Kallisti", "http://www.KallistiMUD.com", arg1);
	banner_address(ses, "Legends of Kallisti", "LoK kallistimud.com 4000", arg1);
	banner_expires(ses, "Legends of Kallisti", "2026", arg1);


	banner_create(ses, "3Kingdoms", arg1);

	banner_desc(ses, "3Kingdoms",
		"{Based around the mighty town of Pinnacle, three main realms beckon the player\n"
		"to explore.  These kingdoms are: Fantasy, a vast medieval realm of orcs, elves,\n"
		"dragons, and a myriad of other creatures; Science, a post-apocalyptic, war-torn\n"
		"world set in the not-so-distant future; and Chaos, a transient realm where the\n"
		"realities of Fantasy and Science collide to produce unimaginable results.\n"
		"\n"
		"3Kingdoms combines all these features, and so much more, to give the player an\n"
		"experience that will stay with them for the rest of their lives.  Come live the\n"
		"adventure and find out for yourself why 3K is the best there is!}", arg1);

	banner_website(ses, "3Kingdoms", "http://3k.org", arg1);
	banner_address(ses, "3Kingdoms", "3K 3k.org 3000", arg1);
	banner_expires(ses, "3Kingdoms", "2026", arg1);


	banner_create(ses, "New World Ateraan", arg1);

	banner_desc(ses, "New World Ateraan",
		"Ateraan is a world of Intensive Roleplaying offering many unique and powerful\n"
		"guilds, races, politics, religion, justice, economy, and a storyline that is\n"
		"dominantly player controlled and based on a novel. The game is based on a\n"
		"Kingdom with knights, merchants, mages, and thieves, and a fierce southern\n"
		"state that has warriors, shaman, slaves, and servants. Ships rule the seas and\n"
		"caravans travel the lands. With 100's of players and features like invasions,\n"
		"ship creation, house building, clans, theaters, leatherball fields, and massive\n"
		"events, the game is incredibly robust and diverse.", arg1);

	banner_website(ses, "New World Ateraan", "http://www.ateraan.com", arg1);
	banner_address(ses, "New World Ateraan", "nwa ateraan.com 4002", arg1);
	banner_expires(ses, "New World Ateraan", "2026", arg1);


	banner_create(ses, "Realm of Utopian Dreams (RUD)", arg1);

	banner_desc(ses, "Realm of Utopian Dreams (RUD)",
		"RUD is a unique ROM-based high-fantasy MUD with character choices for many play\n"
		"styles from hack-n-slash to roleplay. Each of the races and classes offer\n"
		"unique spells and skills, unlocked through a tiered remort system with\n"
		"customization through religion memberships and epic advancements. RUD started\n"
		"in 1996, always seeking new adventurers to become the next hero, build a home,\n"
		"start a shop, and eventually become Nobles of the Realm! New players to RUD can\n"
		"be just as successful as 20+ year veterans. We run quests, plots, and annual\n"
		"festivals in an ever evolving world. Come build Lantarea with us!", arg1);

	banner_website(ses, "Realm of Utopian Dreams (RUD)", "http://rudmud.com", arg1);
	banner_address(ses, "Realm of Utopian Dreams (RUD)", "rud rudmud.com 1701", arg1);
	banner_expires(ses, "Realm of Utopian Dreams (RUD)", "2026", arg1);

	banner_create(ses, "Carrion Fields", arg1);

	banner_desc(ses, "Carrion Fields",
		"Carrion Fields blends high-caliber roleplay with complex, hardcore\n"
		"player-vs-player combat and has been running continuously, 100% free, for \n"
		"over 25 years.  Choose from among 21 races, 17 highly customizable classes\n"
		"and several cabals and religions to suit your playstyle and the story you\n"
		"want to tell.  With a massive, original world and coveted limited objects,\n"
		"we've been described as \"the 'Dark Souls' of MUDs\".  Come join our vibrant\n"
		"community for a real challenge and very real rewards: adrenaline-pumping\n"
		"battles, memorable quests run by our volunteer immortal staff, and stories\n"
		"that will stick with you for a lifetime.", arg1);

	banner_website(ses, "Carrion Fields", "http://carrionfields.net", arg1);
	banner_address(ses, "Carrion Fields", "cf carrionfields.net 4449", arg1);
	banner_expires(ses, "Carrion Fields", "2026", arg1);
}

int total_banners()
{
	int index, count;

	count = 0;

	for (index = 0 ; index < gtd->banner_list->used ; index++)
	{
		if (gtd->time < gtd->banner_list->list[index]->val64)
		{
			count++;
		}
	}
	return count;
}

struct listnode *random_pick()
{
	struct listnode *node;
	int index, max;

	max = total_banners();

	for (index = 0 ; index < gtd->banner_list->used ; index++)
	{
		node = gtd->banner_list->list[index];

		if (node->val64 < gtd->time)
		{
			continue;
		}

		if (generate_rand(gts) % max == 0)
		{
			return node;
		}
		max--;
	}
	return NULL;
}

void banner_random(struct session *ses)
{
	struct listnode *node;

	node = random_pick();

	if (node)
	{
		tintin_printf2(ses, "");

		if (get_scroll_cols(ses) < 80 || HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER))
		{
			command(ses, do_draw, "scroll scaled calign tile 1 1 1 80 <138>%s", node->arg1);
			command(ses, do_draw, "scroll scaled calign tile 1 1 1 80 <178>%s", node->arg3);
		}
		else
		{
//			command(ses, do_draw, "scroll scaled calign tile 1 1 1 80 {<138>%s <178>- <138>\e]8;;%s\a\e[58:5:2;4m%s\e[59;24m\e]8;;\a}", node->arg1, node->arg3, node->arg3);
			command(ses, do_draw, "scroll scaled calign tile 1 1 1 80 {<138>%s <178>- <138>\e]8;;%s\a\e[04m%s\e[24m\e]8;;\a}", node->arg1, node->arg3, node->arg3);
		}
		tintin_printf2(ses, "");

		if (get_scroll_cols(ses) < 80 || HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER))
		{
			command(ses, do_draw, "scroll ualign scaled tile 1 1 1 -1 {<278>%s}", node->arg2);
		}
		else
		{
			command(ses, do_draw, "scroll scaled tile 1 1 1 80 {<278>%s}", node->arg2);
		}

		tintin_printf2(ses, "");

		command(ses, do_draw, "scroll scaled tile 1 1 1 80 <178>To connect to %s enter: #session %s", node->arg1, node->arg4);

		tintin_printf2(ses, "");
	}
//	tintin_printf2(ses, "#NO SESSION ACTIVE. USE: %csession {name} {host} {port} TO START ONE.", gtd->tintin_char);
}

void banner_list(struct session *ses, char *arg1)
{
	int index;
	struct listnode *node;

	tintin_header(ses, 80, " BANNERS ");

	for (index = 0 ; index < gtd->banner_list->used ; index++)
	{
		node = gtd->banner_list->list[index];

		arg1 = strstr(node->arg4, " ");

		tintin_printf2(ses, "[%-30s] %s", node->arg1, arg1);
	}
	tintin_header(ses, 80, "");

}

void banner_save(struct session *ses, char *arg1)
{
	int index;
	struct listnode *node;
	char *name, *arg2;
	
	name = str_alloc_stack(0);
	arg2 = str_alloc_stack(0);

	set_nest_node_ses(ses, "info[BANNERS]", "");

	for (index = 0 ; index < gtd->banner_list->used ; index++)
	{
		node = gtd->banner_list->list[index];

		sprintf(name, "info[BANNERS][%s]", node->arg1);

		arg1 = node->arg4;

		arg1 = get_arg_in_braces(ses, arg1, arg2, GET_ONE);
		add_nest_node_ses(ses, name, "{ALIAS}{%s}", arg2);

		add_nest_node_ses(ses, name, "{DESC}{%s}", node->arg2);

		arg1 = get_arg_in_braces(ses, arg1, arg2, GET_ONE);
		add_nest_node_ses(ses, name, "{HOST}{%s}", arg2);

		add_nest_node_ses(ses, name, "{NAME}{%s}", node->arg1);

		arg1 = get_arg_in_braces(ses, arg1, arg2, GET_ONE);
		add_nest_node_ses(ses, name, "{PORT}{%s}", arg2);

		add_nest_node_ses(ses, name, "{WEBSITE}{%s}", node->arg3);
	}

	show_message(ses, LIST_COMMAND, "#INFO: DATA WRITTEN TO {info[BANNERS]}");
}

void banner_test(struct session *ses, char *arg1)
{
	int cnt, max, index;
	struct listnode *node;

	max = total_banners() * 100000;

	for (cnt = 0 ; cnt < max ; cnt++)
	{
		node = random_pick();

		if (node)
		{
			node->shots++;
		}
	}

	tintin_header(ses, 80, " BANNER TEST ");

	for (index = 0 ; index < gtd->banner_list->used ; index++)
	{
		node = gtd->banner_list->list[index];

		arg1 = strstr(node->arg4, " ");

		tintin_printf2(ses, "[%-30s] [%6d] %s", node->arg1, node->shots, arg1);

		node->shots = 0;
	}
	tintin_header(ses, 80, "");
}

struct session *banner_gui(struct session *ses, char *arg1)
{
	char filename[PATH_SIZE];
	FILE *fp;

	sprintf(filename, "%s/gui.tin", gtd->system->tt_dir);

	if ((fp = fopen(filename, "w")) == NULL)
	{
		tintin_printf2(ses, "#BANNER GUI: FAILED TO CREATE {%s}.", filename);

		return ses;
	}

	fputs(tt_gui, fp);

	fclose(fp);

	return command(ses, do_line, "quiet #read %s", filename);
}

DO_COMMAND(do_banner)
{
	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (is_abbrev(arg1, "RANDOM"))
	{
		banner_random(ses);
	}
	else if (is_abbrev(arg1, "INIT"))
	{
		banner_init(ses, arg1);
	}
	else if (is_abbrev(arg1, "GUI"))
	{
		return banner_gui(ses, arg1);
	}
	else if (is_abbrev(arg1, "LIST"))
	{
		banner_list(ses, arg1);
	}
	else if (is_abbrev(arg1, "TEST"))
	{
		banner_test(ses, arg1);
	}
	else if (is_abbrev(arg1, "SAVE"))
	{
		banner_save(ses, arg1);
	}
	else
	{
		banner_list(ses, arg1);
//		tintin_printf2(ses, "#SYNTAX: #BANNER {INIT|LIST|RANDOM|TEST}");
	}
	return ses;
}

// archive

/*
	{
		1400000000,  // 2014
		1800000000,  // 2027
		100,

		"\n"
		"<138>                 Primal Darkness  -  http://www.primaldarkness.com\n"
		"\n"
		"<078>Primal Darkness boasts twenty three start races, three quest races, five start\n"
		"<078>classes and twenty one subclasses to suit your adventuring needs, and a custom\n"
		"<078>remort system allowing you to save all your hard work while trying out new\n"
		"<078>races and/or classes. The world has zoned player killing (pk) supported by a\n"
		"<078>justice system, player built and ran guilds, a fully sail-able ocean with\n"
		"<078>customizable ships and ship battle system, a fully flyable sky with mapping\n"
		"<078>system, mud-wide auction line, a colosseum in which to prove your battle\n"
		"<078>prowess (harmless/free pk), and 30+ areas on multiple continents to explore.\n"
		"\n"
		"<178>To connect to Primal Darkness enter: #session pd mud.primaldarkness.com 5000\n"
		"\n",
		
		"\n"
		"<138>Primal Darkness\n"
		"<168>http://www.primaldarkness.com\n"
		"\n"
		"<078>Primal Darkness boasts twenty three start races, three quest races, five start classes and twenty one subclasses to suit your adventuring needs, and a custom remort system allowing you to save all your hard work while trying out new races and/or classes. The world has zoned player killing (pk) supported by a justice system, player built and ran guilds, a fully sail-able ocean with customizable ships and ship battle system, a fully flyable sky with mapping system, mud-wide auction line, a colosseum in which to prove your battle prowess (harmless/free pk), and 30+ areas on multiple continents to explore.\n"
		"\n"
		"<178>To connect to Primal Darkness enter: #session pd mud.primaldarkness.com 5000\n"
		"\n"
	},

	{
		1400000000,
 		1800000000,
		100,

		"\n"
		"<138>                The Last Outpost  -  https://www.last-outpost.com\n"
		"\n"
		"<078>The Last Outpost has been serving up adventure since 1992.  Along with\n"
		"<078>exploring and advancing through the game world, the game offers players the\n"
		"<078>ability to lay claim to the zones that make up the land.  Once claimed, a zone\n"
		"<078>can be taxed, and the player making the claim gets to decide policy within the\n"
		"<078>zone.  Whoever claims the whole world is declared the Leader of the Last\n"
		"<078>Outpost!  Whether you enjoy hack 'n slash, following quests, PvP, NPK, playing\n"
		"<078>in clans, or soloing, the Last Outpost has it.\n"
		"\n"
		"<178>To connect to The Last Outpost enter: #session lo last-outpost.com 4000\n"
		"\n",

		"\n"
		"<138>The Last Outpost\n"
		"<168>https://www.last-outpost.com\n"
		"\n"
		"<078>The Last Outpost has been serving up adventure since 1992.  Along with exploring and advancing through the game world, the game offers players the ability to lay claim to the zones that make up the land.  Once claimed, a zone can be taxed, and the player making the claim gets to decide policy within the zone.  Whoever claims the whole world is declared the Leader of the Last Outpost!  Whether you enjoy hack 'n slash, following quests, PvP, NPK, playing in clans, or soloing, the Last Outpost has it.\n"
		"\n"
		"<178>To connect to The Last Outpost enter: #session lo last-outpost.com 4000\n"
		"\n"
	},

	{
		1400000000, 
		1700000000, 
		100,
		"\n"
		"<138>               Carrion Fields  -  http://carrionfields.net\n"
		"\n"
		"<078>Adventure, politics and bloody war await you in this life of swords, sorcery,\n"
		"<078>deception, and honor.  We have 17 customizable classes with which to explore a\n"
		"<078>massively rich world of over 270 areas.  RP is mandatory, but help is always\n"
		"<078>available on the newbie channel.  Intuitive game mechanics provide a fun and\n"
		"<078>fulfilling PK environment.  Carrion Fields is 100% free to play and free of\n"
		"<078>paid perks as well.  By what name do you wish to be mourned?\n"
		"\n"
                "<178>To connect to Carrion Fields enter: #session cf carrionfields.net 4449\n"
                "\n"
		                
	},

	{
		1400000000, 
		1700000000, 
		100,
		"\n"
		"<138>                 Alter Aeon  -  http://www.alteraeon.com\n"
		"\n"
		"<078>Alter Aeon is a custom multiclass MUD, where each of the character\n"
		"<078>classes can be combined to make very unique characters.  This huge\n"
		"<078>fantasy themed game has hundreds of areas and quests, spanning\n"
		"<078>several continents and outer planar regions.  There are custom spells,\n"
		"<078>skills, minions, player run shops, boats, PvP, and many other features\n"
		"<078>for nearly every kind of player.  The game is very friendly to new players\n"
		"<078>and has extensive support for the blind and visually impaired.\n"
		"\n"
		"<178>To connect to Alter Aeon enter: #session aa alteraeon.com 3000\n"
		"\n"
	},

	{
		1260469590, * 10 Dec 2009 *
		1323541590, * 10 Dec 2011 *
		"\n"
		"<138>                                  Lowlands\n"
		"\n"
		"<078>Lowlands is an old school severely customized Merc derived Hack and Slash MUD\n"
		"<078>with an emphasis on exploration and solving challenging and immersive area\n"
		"<078>based quests.\n"
		"\n"
		"<078>A voluntary two faction player killing system is in place for the followers of\n"
		"<078>the two gods, Chaos and Order, combined with easy corpse retrieval, no corpse\n"
		"<078>looting, and a large 15,000 room world.\n"
		"\n"
		"<178>To connect to Lowlands enter: #session lo slackhalla.org 6969\n"
		"\n"
	},

	{
		1260469590, * 10 Dec 2009 *
		1323541590, * 10 Dec 2011 *
		"\n"
		"<138>                Maiden Desmodus  -  http://maidendesmodus.com\n"
		"\n"
		"<078>Maiden Desmodus is an immersive world of high adventure where your actions, or\n"
		"<078>inaction, will determine the fate of The Isle. Choose to be born unto one of\n"
		"<078>two opposing factions, join one of the six powerful guilds, and carve your\n"
		"<078>place in history through your cunning, your strategy, and your skill with magic\n"
		"<078>or a blade. At every turn are players who may ally themselves to you, or work\n"
		"<078>to destroy you. Shall you form your own cabal and command your peers, control\n"
		"<078>the politics of your city, or lead an army against those who oppose you?\n"
		"<078>Maiden Desmodus features a completely original world and a custom game engine.\n"
		"\n"
		"<178>To connect to Maiden Desmodus enter: #session md maidendesmodus.com 4000\n"
		"\n"
	},

	{
		1260469590, * 10 Dec 2009 *
		1323541590, * 10 Dec 2011 *
		"\n"
		"<138>                     Lost Souls  -  http://lostsouls.org\n"
		"\n"
		"<078>\"Our world is fallen, boy.  Aedaris is a ruin.  My grandfather, he told me\n"
		"<078>of days, not so long gone, when everything you see was part of a great empire.\n"
		"<078>Peaceful, he said.  Full of wonders.  They called it eternal.  Funny, eh, boy?\n"
		"<078>They thought it'd last forever, and it went crazy and tore itself apart.  But\n"
		"<078>they left behind a few things for us, didn't they?  Ha!  Yes, lots for us.  Now\n"
		"<078>give that wizard-stick here before you blow your fool horns off, and get to\n"
		"<078>work.  Daylight's soon, and these faeries aren't going to skin themselves.\"\n"
		"<078>Lost Souls: chaos in the wreckage of empire.  Be clever if you want to live.\n"
		"\n"
                "<178>To connect to Lost Souls enter: #session ls lostsouls.org 23\n"
                "\n"
		                
	},

	{
		1291140000, * 30 Nov 2010 *
		1354280000, * 30 Nov 2012 *
		100,
		"\n"
		"<138>                Threshold RPG  -  http://www.thresholdrpg.com\n"
		"\n"
		"<078>Join us as Threshold RPG, one of the oldest RP enforced games on the\n"
		"<078>internet. Add to thirteen years of player created history and make your own\n"
		"<078>mark on the world today. Join a hundred other players who are vying for\n"
		"<078>political and religious power in complex systems that reward skill, effort,\n"
		"<078>and social interactions. Threshold RPG is a custom code-base written in\n"
		"<078>LPC and features a completely unique and original world.\n"
		"\n"
		"<178>To connect to Threshold RPG enter: #session thresh thresholdrpg.com 23\n"
		"\n"
	},

	{
		1291140000,
		1354280000,
		100,
		"\n"
		"<138>                   Primordiax - http://www.primordiax.com\n"
		"\n"
		"<078>Primordiax is an in-character enforced MUD where roleplaying is heavily\n"
		"<078>encouraged. The exclusive design of Primordiax makes it extremely\n"
		"<078>accessible to new players without losing the intrigue and complexity that\n"
		"<078>continues to attract veterans of the genre. Primordiax offers a classic\n"
		"<078>gaming experience with a highly unique class system and an open skill tree.\n"
		"\n"
		"<178>To connect to Primordiax enter: #session prim primordiax.com 3000\n"
		"\n"
	},

	{
		1291140000, * 30 Nov 2010 *
		1354280000, * 30 Nov 2012 *
		100,
		"\n"
		"<138>                Northern Crossroads - http://www.ncmud.org\n"
		"\n"
		"<078>Northern Crossroads is a diverse world of blade wielders, assassins and magic\n"
		"<078>users who use their powers together to seek out the darkest dungeons. Decide\n"
		"<078>between five classes, as allowed by your choice of ten races, and prove your\n"
		"<078>strength to ascend to an Advanced class. Venture to dangerous zones with other\n"
		"<078>mortals, claim the rarest of items, join one of several clubs and build your\n"
		"<078>character to challenge other mortals. NC has enthralled players with hundreds\n"
		"<078>of detailed areas of various themes since 1993 and is one of the oldest MUDs\n"
		"<078>in the world.\n"
		"\n"
		"<178>To connect to Northern Crossroads enter: #session NC ncmud.org 9000\n"
		"\n"
	},
*/

