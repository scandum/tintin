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

struct advertisement_type
{
	time_t                  start;
	time_t                  end;
	int                     value;
	char                  * desc;
	char                  * wrap;
};

struct advertisement_type advertisement_table[] =
{
	{
		1400000000, /* 2014 */
		1800000000, /* 2027 */
		100,

		"\n"
		"<138>                     Lost Souls  -  http://lostsouls.org\n"
		"\n"
		"<078>\"Lost Souls is not for the faint of heart.\" -- Net Games\n"
		"\n"
		"<078>\"The depth of Lost Souls can be amazing.\" -- Playing MUDs On The Internet\n"
		"\n"
		"<078>\"Have you ever come upon a place on the Net that's so incredible that you\n"
		"<078>can't believe such entertainment is free? This MUD will blow your mind with\n"
		"<078>its marvelous attention to detail and incredible role-playing atmosphere!\"\n"
		"\n"
		"<078>  -- Yahoo! Wild Web Rides\n"
		"\n"
                "<178>To connect to Lost Souls enter: #session ls lostsouls.org 23\n"
                "\n",

		"\n"
		"<138>Lost Souls\n"
		"<168>http://lostsouls.org\n"
		"\n"
		"<078>\"Lost Souls is not for the faint of heart.\" -- Net Games\n"
		"\n"
		"<078>\"The depth of Lost Souls can be amazing.\" -- Playing MUDs On The Internet\n"
		"\n"
		"<078>\"Have you ever come upon a place on the Net that's so incredible that you can't believe such entertainment is free? This MUD will blow your mind with its marvelous attention to detail and incredible role-playing atmosphere!\" -- Yahoo! Wild Web Rides\n"
		"\n"
		"<178>To connect to Lost Souls enter: #session ls lostsouls.org 23\n"
		"\n"
	},

	{
		1400000000, /* 2014 */
		1800000000, /* 2027 */
		100,
		"\n"
		"<138>               Legends of Kallisti  -  http://www.KallistiMUD.com\n"
		"\n"
		"<078>One of the longest running, most feature rich MUDs in the world with decades of\n"
		"<078>development. Kallisti boasts a massive original world, great atmosphere of long\n"
		"<078>time players, excellent combat system including group formations, ranged combat,\n"
		"<078>optional PK and arena PvP, extensive character customization options, player\n"
		"<078>lineages, clans, customizable player houses, item crafting, extensively\n"
		"<078>customizable UI, Mud Sound Protocol (MSP), MSDP, and so much more.\n"
		"\n"
		"<078>This is an a amazing game that you could literally play for a decade and still\n"
		"<078>discover more - you won't be disappointed!\n"
		"\n"
		"<178>To connect to Kallisti MUD enter: #session LoK kallistimud.com 4000\n"
		"\n",

		"\n"
		"<138>Legends of Kallisti\n"
		"<168>http://www.KallistiMUD.com\n"
		"\n"
		"<078>One of the longest running, most feature rich MUDs in the world with decades of development. Kallisti boasts a massive original world, great atmosphere of long time players, excellent combat system including group formations, ranged combat, optional PK and arena PvP, extensive character customization options, player lineages, clans, customizable player houses, item crafting, extensively customizable UI, Mud Sound Protocol (MSP), and so much more.\n"
		"<078>This is an a amazing game that you could literally play for a decade and still discover more - you won't be disappointed!\n"
		"\n"
		"<178>To connect to Kallisti MUD enter: #session LoK kallistimud.com 4000\n"
		"\n",
	},
/*
	{
		1400000000, // 2014
 		1800000000, // 2027
		100,

		"\n"
                "<138>                    Lowlands  -  http://lolamud.net\n"
		"\n"
		"<078>Lowlands is based on the Storms of Time codebase which in turn is based on the\n"
		"<078>Mortal Realms codebase which was established in 1993. Lowlands has many unique\n"
		"<078>systems and features. Its main strength is the inclusion of over 100 detailed\n"
		"<078>quests which have been custom coded using a powerful mob prog engine.\n"
		"<078>\n"
		"<078>Lowlands has a generic medieval fantasy theme with a mature playerbase that has\n"
		"<078>little interest in roleplaying, but make up for that with their enthusiasm when\n"
		"<078>it comes to grinding, raiding, questing and exploring.\n"
		"\n"
		"<178>To connect to Lowlands enter: #session lol lolamud.net 6969\n"
		"\n",

		"\n"
		"<138>Lowlands\n"
		"<168>http://lolamud.net\n"
		"\n"
		"<078>Lowlands is based on the Storms of Time codebase which in turn is based on the Mortal Realms codebase. It's a very polished MUD with many unique systems and features. Its main strength is the inclusion of over 100 detailed area quests which have been custom coded using a powerful mob prog engine.\n"
		"<078>\n"
		"<078>Lowlands has a generic medieval fantasy theme and the codebase is open source so anyone wanting to build or code can easily set up their own test mud.\n"
		"\n"
		"<178>To connect to Lowlands enter: #session lol lolamud.net 6969\n"
		"\n"
	},
*/
	{
		1400000000,
		1800000000,
		100,
		"\n"
                "<138>                    3Kingdoms  -  http://3k.org\n"
                "\n"
                "<078>Based around the mighty town of Pinnacle, three main realms beckon the player\n"
                "<078>to explore.  These kingdoms are: Fantasy, a vast medieval realm of orcs, elves,\n"
                "<078>dragons, and a myriad of other creatures; Science, a post-apocalyptic, war-torn\n"
                "<078>world set in the not-so-distant future; and Chaos, a transient realm where the\n"
                "<078>realities of Fantasy and Science collide to produce unimaginable results.\n"
                "\n"
                "<078>3Kingdoms combines all these features, and so much more, to give the player an\n"
                "<078>experience that will stay with them for the rest of their lives.  Come live the\n"
                "<078>adventure and find out for yourself why 3K is the best there is!\n"
                "\n"
                "<178>To connect to 3Kingdoms enter:  #session 3K 3k.org 3000\n"
                "\n",
 
                "\n"
                "<138>3Kingdoms\n"
                "<168>http://3k.org\n"
                "\n"
                "<078>Based around the mighty town of Pinnacle, three main realms beckon the player to explore.  These kingdoms are: Fantasy, a vast medieval realm of orcs, elves, dragons, and a myriad of other creatures; Science, a post-apocalyptic, war-torn world set in the not-so-distant future; and Chaos, a transient realm where the realities of Fantasy and Science collide to produce unimaginable results.  3Kingdoms combines all these features, and so much more, to give the player an experience that will stay with them for the rest of their lives.  Come live the adventure and find out for yourself why 3K is the best there is!\n"
                "\n"
                "<178>To connect to 3Kingdoms enter:  #session 3K 3k.org 3000\n"
                "\n",
	},

	{
		1400000000,  /* 2014 */ 
		1800000000,  /* 2027 */ 
		100,

		"\n"
		"<138>                New Worlds Ateraan  -  http://www.ateraan.com\n"
		"\n"
		"<078>Ateraan is a world of Intensive Roleplaying offering many unique and powerful\n"
		"<078>guilds, races, politics, religion, justice, economy, and a storyline that is\n"
		"<078>dominantly player controlled and based on a novel. The game is based on a\n"
		"<078>Kingdom with knights, merchants, mages, and thieves, and a fierce southern\n"
		"<078>state that has warriors, shaman, slaves, and servants. Ships rule the seas and\n"
		"<078>caravans travel the lands. With 100's of players and features like invasions,\n"
		"<078>ship creation, house building, clans, theaters, leatherball fields, and massive\n"
		"<078>events, the game is incredibly robust and diverse.\n"
		"\n"
                "<178>To connect to New Worlds Ateraan enter: #session nwa ateraan.com 4002\n"
                "\n",

                "\n"
                "<138>New Worlds Ateraan\n"
                "<168>http://www.ateraan.com\n"
                "\n"
                "<078>Ateraan is a world of Intensive Roleplaying offering many unique and powerful guilds, races, politics, religion, justice, economy, and a storyline that is dominantly player controlled and based on a novel. The game is based on a Kingdom with fighters, merchants, mages, and thieves, and a fierce southern state that has warriors, shaman, slaves, and servants. Ships rule the seas and caravans travel the lands. With 100's of players and features like invasions, ship creation, house building, clans, theaters, leatherball fields, and massive events, the game is incredibly robust and diverse.\n"
                "\n"
                "<178>To connect to New Worlds Ateraan enter: #session nwa ateraan.com 4002\n"
                "\n"
	},

	{
		1400000000,
 		1800000000,
		100,

		"\n"
		"<138>              Realm of Utopian Dreams (RUD)  -  http://rudmud.com\n"
		"\n"
		"<078>RUD is a unique ROM-based high-fantasy MUD with character choices for many play\n"
		"<078>styles from hack-n-slash to roleplay. Each of the races and classes offer\n"
		"<078>unique spells and skills, unlocked through a tiered remort system with\n"
		"<078>customization through religion memberships and epic advancements. RUD started\n"
		"<078>in 1996, always seeking new adventurers to become the next hero, build a home,\n"
		"<078>start a shop, and eventually become Nobles of the Realm! New players to RUD can\n"
		"<078>be just as successful as 20+ year veterans. We run quests, plots, and annual\n"
		"<078>festivals in an ever evolving world. Come build Lantarea with us!\n"
		"\n"
		"<178>To connect to Realm of Utopian Dreams enter: #session rud rudmud.com 1701\n"
		"\n",

		"\n"
		"<138>Realms of Utopian Dreams (RUD)\n"
		"<168>http://rudmud.com\n"
		"\n"
		"<078>RUD is a unique ROM-based high-fantasy MUD with character choices for many play styles from hack-n-slash to roleplay. Each of the races and classes offer unique spells and skills, unlocked through a tiered remort system with customization through religion memberships and epic advancements. RUD started in 1996, always seeking new adventurers to become the next hero, build a home, start a shop, and eventually become Nobles of the Realm! New players to RUD can be just as successful as 20+ year veterans. We run quests, plots, and annual festivals in an ever evolving world. Come build Lantarea with us!\n"
		"\n"
		"<178>To connect to Realm of Utopian Dreams enter: #session rud rudmud.com 1701\n"
		"\n"
	},

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
*/
/*	{
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
*/


/*
	{
		1400000000,
		1800000000,
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
		"\n",

		"\n"
		"<138>Threshold RPG\n"
		"<168>http://www.thresholdrpg.com\n"
		"\n"
		"<078>Join us as Threshold RPG, one of the oldest RP enforced games on the internet. Add to thirteen years of player created history and make your own mark on the world today. Join a hundred other players who are vying for political and religious power in complex systems that reward skill, effort, and social interactions. Threshold RPG is a custom code-base written in LPC and features a completely unique and original world.\n"
		"\n"
		"<178>To connect to Threshold RPG enter: #session thresh thresholdrpg.com 23\n"
		"\n"
	},
*/

/*
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
		1388166000, 
		1600000000,
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
*/
	{
		0,
		0,
		0,
		"",
		""
	}
};

int valid_advertisement(int i)
{
	if (advertisement_table[i].start > gtd->time)
	{
		return 0;
	}
	if (advertisement_table[i].end < gtd->time)
	{
		return 0;
	}
	return advertisement_table[i].value;
}

int total_advertisements()
{
	int i, count = 0;

	for (i = 0 ; *advertisement_table[i].desc ; i++)
	{
		count += valid_advertisement(i);
	}
	return count;
}

DO_COMMAND(do_advertise)
{
	int i, max, cnt;
	char buf[BUFFER_SIZE];

	max = total_advertisements();

	for (i = 0 ; *advertisement_table[i].desc ; i++)
	{
		if (!valid_advertisement(i))
		{
			continue;
		}

		cnt = advertisement_table[i].value;

		if (generate_rand(ses) % (unsigned long long) max < (unsigned long long) cnt)
		{
			char *pto, *ptf;

			if (gtd->screen->cols >= 80)
			{
				substitute(ses, advertisement_table[i].desc, buf, SUB_COL);
			}
			else
			{
				substitute(ses, advertisement_table[i].wrap, buf, SUB_COL);
			}

			pto = buf;

			while (*pto)
			{
				ptf = strchr(pto, '\n');

				if (ptf == NULL)
				{
					break;
				}
				*ptf++ = 0;

				tintin_puts(ses, pto);

				pto = ptf;
			}
			break;
		}
		max -= cnt;
	}

//	tintin_printf2(ses, "#NO SESSION ACTIVE. USE: %csession {name} {host} {port} TO START ONE.", gtd->tintin_char);

	return ses;
}





/*
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
		"<138>                   Alter Aeon  -  http://www.alteraeon.com\n"
		"\n"
		"<078>Alter Aeon is a custom MUD written entirely from scratch. The story setting\n"
		"<078>is reminiscent of Dungeons and Dragons, but has elements of fantasy.\n"
		"<078>There are unique spell, skill, and minion systems, player run shops, boats,\n"
		"<078>and many other features for nearly every kind of player.  In development\n"
		"<078>since 1995, the world of Alter Aeon has hundreds of areas to explore, quests\n"
		"<078>to complete, and puzzles to solve.\n"
		"\n"
		"<178>To connect to Alter Aeon enter: #session aa alteraeon.com 3000\n"
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
