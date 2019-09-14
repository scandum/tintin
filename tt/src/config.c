/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2004-2019 Igor van den Hoven                                    *
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
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*   file: config.c - funtions related tintin++ configuration                  *
*              (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                   *
*                     coded by Igor van den Hoven 2004                        *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_configure)
{
	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE];
	struct listnode *node;
	int index;

	arg = get_arg_in_braces(ses, arg, arg1,  GET_ONE);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ONE, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, " CONFIGURATIONS ");

		for (index = 0 ; *config_table[index].name != 0 ; index++)
		{
			node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

			if (node)
			{
				tintin_printf2(ses, "[%-14s] [%8s] %s", 
					node->arg1,
					node->arg2,
					strcmp(node->arg2, "ON") == 0 ? config_table[index].msg_on : config_table[index].msg_off);
			}
		}

		tintin_header(ses, "");
	}
	else
	{
		for (index = 0 ; *config_table[index].name != 0 ; index++)
		{
			if (is_abbrev(arg1, config_table[index].name))
			{
				if (config_table[index].config(ses, arg2, index) != NULL)
				{
					node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

					if (node)
					{
						show_message(ses, LIST_CONFIG, "#CONFIG {%s} HAS BEEN SET TO {%s}.", config_table[index].name, node->arg2);
					}
				}
				return ses;
			}
		}
		show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG {%s} IS NOT A VALID OPTION.", capitalize(arg1));
	}
	return ses;
}


DO_CONFIG(config_speedwalk)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_SPEEDWALK);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_SPEEDWALK);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}


DO_CONFIG(config_verbatim)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_VERBATIM);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_VERBATIM);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_repeatenter)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_REPEATENTER);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_REPEATENTER);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_commandcolor)
{
	char buf[BUFFER_SIZE];

	substitute(ses, arg, buf, SUB_COL);

	RESTRING(ses->cmd_color, buf);

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "", "");

	return ses;
}

DO_CONFIG(config_commandecho)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_ECHOCOMMAND);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_ECHOCOMMAND);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_verbose)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_VERBOSE);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_VERBOSE);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_wordwrap)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_WORDWRAP);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_WORDWRAP);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	SET_BIT(gtd->flags, TINTIN_FLAG_RESETBUFFER);

	return ses;
}

DO_CONFIG(config_log)
{
	if (!strcasecmp(arg, "HTML"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOGPLAIN);
		SET_BIT(ses->flags, SES_FLAG_LOGHTML);
	}
	else if (!strcasecmp(arg, "PLAIN"))
	{
		SET_BIT(ses->flags, SES_FLAG_LOGPLAIN);
		DEL_BIT(ses->flags, SES_FLAG_LOGHTML);
	}
	else if (!strcasecmp(arg, "RAW"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOGPLAIN);
		DEL_BIT(ses->flags, SES_FLAG_LOGHTML);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG LOG <HTML|PLAIN|RAW>");

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_buffersize)
{
	if (!is_number(arg))
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {BUFFER SIZE} <NUMBER>");

		return NULL;
	}

	if (atoi(arg) < 100 || atoi(arg) > 999999)
	{
		show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG BUFFER: PROVIDE A NUMBER BETWEEN 100 and 999999");

		return NULL;
	}

	init_buffer(ses, atoi(arg));

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_scrolllock)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_SCROLLLOCK);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCROLLLOCK);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_connectretry)
{
	if (!is_number(arg))
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {CONNECT RETRY} <NUMBER>");

		return NULL;
	}

	gts->connect_retry = atoll(arg) * 1000000LL;

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_packetpatch)
{
	if (is_abbrev("AUTO", arg))
	{
		gts->check_output = 0;

		update_node_list(ses->list[LIST_CONFIG], config_table[index].name, "AUTO", "", "");

		SET_BIT(ses->flags, SES_FLAG_AUTOPATCH);

		return ses;
	}

	if (!is_number(arg))
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {PACKET PATCH} <NUMBER>");

		return NULL;
	}

	if (atof(arg) < 0 || atof(arg) > 10)
	{
		show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG PACKET PATCH: PROVIDE A NUMBER BETWEEN 0.00 and 10.00");

		return NULL;
	}

	DEL_BIT(ses->flags, SES_FLAG_AUTOPATCH);

	gts->check_output = (unsigned long long) (tintoi(arg) * 1000000ULL);

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}


DO_CONFIG(config_historysize)
{
	if (!is_number(arg))
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {HISTORY SIZE} <NUMBER>");
	}

	if (atoi(arg) < 0 || atoi(arg) > 9999)
	{
		show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG HISTORY: PROVIDE A NUMBER BETWEEN 0 and 9999");

		return NULL;
	}

	gtd->history_size = atoi(arg);

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_tintinchar)
{
	gtd->tintin_char = arg[0];

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "", "");

	return ses;
}

DO_CONFIG(config_verbatimchar)
{
	gtd->verbatim_char = arg[0];

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "", "");

	return ses;
}

DO_CONFIG(config_repeatchar)
{
	gtd->repeat_char = arg[0];

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "", "");

	return ses;
}

DO_CONFIG(config_debugtelnet)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_convertmeta)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_CONVERTMETA);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CONVERTMETA);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_loglevel)
{
	if (!strcasecmp(arg, "LOW"))
	{
		SET_BIT(ses->flags, SES_FLAG_LOGLEVEL);
	}
	else if (!strcasecmp(arg, "HIGH"))
	{
		DEL_BIT(ses->flags, SES_FLAG_LOGLEVEL);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <LOW|HIGH>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_colorpatch)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_COLORPATCH);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_COLORPATCH);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_mccp)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_MCCP);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_MCCP);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_autotab)
{
	if (!is_number(arg))
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {AUTO TAB} <NUMBER>");

		return NULL;
	}

	if (atoi(arg) < 1 || atoi(arg) > 999999)
	{
		show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG BUFFER: PROVIDE A NUMBER BETWEEN 1 and 999999");

		return NULL;
	}

	ses->auto_tab = atoi(arg);

	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_charset)
{
	if (!strcasecmp(arg, "AUTO"))
	{
		if (strcasestr(gtd->lang, "UTF-8"))
		{
			strcpy(arg, "UTF-8");
		}
		else if (strcasestr(gtd->lang, "BIG-5"))
		{
			strcpy(arg, "BIG-5");
		}
		else
		{
			strcpy(arg, "ASCII");
		}
	}

	if (!strcasecmp(arg, "BIG5"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
		SET_BIT(ses->flags, SES_FLAG_BIG5);
	}
	else if (!strcasecmp(arg, "BIG-5"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
		SET_BIT(ses->flags, SES_FLAG_BIG5);
	}
	else if (!strcasecmp(arg, "UTF-8"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
		SET_BIT(ses->flags, SES_FLAG_UTF8);
	}
	else if (!strcasecmp(arg, "ASCII"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
	}
	else if (!strcasecmp(arg, "BIG2UTF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
		SET_BIT(ses->flags, SES_FLAG_UTF8|SES_FLAG_BIG5TOUTF8);
	}
	else if (!strcasecmp(arg, "FANSI"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
		SET_BIT(ses->flags, SES_FLAG_UTF8|SES_FLAG_FANSITOUTF8);
	}
	else if (!strcasecmp(arg, "KOI2UTF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_CHARSETS);
		SET_BIT(ses->flags, SES_FLAG_UTF8|SES_FLAG_KOI8TOUTF8);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <AUTO|ASCII|BIG-5|FANSI|UTF-8|BIG2UTF|KOI2UTF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_colormode)
{
	if (!strcasecmp(arg, "AUTO"))
	{
		if (strcasestr(gtd->term, "truecolor"))
		{
			strcpy(arg, "TRUE");
		}
		else if (!strcasecmp(gtd->term, "xterm") || strcasestr(gtd->term, "256color"))
		{
			strcpy(arg, "256");
		}
		else
		{
			strcpy(arg, "ANSI");
		}
	}

	if (!strcasecmp(arg, "NONE"))
	{
		strcpy(arg, "NONE");
		DEL_BIT(ses->flags, SES_FLAG_ANSICOLOR|SES_FLAG_256COLOR|SES_FLAG_TRUECOLOR);
	}
	else if (!strcasecmp(arg, "ANSI"))
	{
		strcpy(arg, "ANSI");
		SET_BIT(ses->flags, SES_FLAG_ANSICOLOR);
		DEL_BIT(ses->flags, SES_FLAG_256COLOR|SES_FLAG_TRUECOLOR);
	}
	else if (!strcasecmp(arg, "256"))
	{
		SET_BIT(ses->flags, SES_FLAG_ANSICOLOR|SES_FLAG_256COLOR);
		DEL_BIT(ses->flags, SES_FLAG_TRUECOLOR);
	}
	else if (!strcasecmp(arg, "TRUE"))
	{
		strcpy(arg, "TRUE");
		SET_BIT(ses->flags, SES_FLAG_ANSICOLOR|SES_FLAG_256COLOR|SES_FLAG_TRUECOLOR);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <AUTO|NONE|ANSI|256|TRUE>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "", "");

	return ses;
}


DO_CONFIG(config_screenreader)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(ses->flags, SES_FLAG_SCREENREADER);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(ses->flags, SES_FLAG_SCREENREADER);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_inheritance)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_INHERITANCE);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_INHERITANCE);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_randomseed)
{
	if (!strcasecmp(arg, "AUTO"))
	{
		strcpy(arg, "AUTO");

		seed_rand(ses, utime());
	}
	else if (is_number(arg))
	{
		seed_rand(ses, get_number(ses, arg));
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <AUTO|NUMBER>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg, "", "");

	return ses;
}

DO_CONFIG(config_mousetracking)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		printf("\e[?1000h\e[?1002h\e[?1004h\e[?1006h");
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		printf("\e[?1000l\e[?1002l\e[?1004l\e[?1006l");
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}

DO_CONFIG(config_childlock)
{
	if (!strcasecmp(arg, "ON"))
	{
		SET_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK);
	}
	else if (!strcasecmp(arg, "OFF"))
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK);
	}
	else
	{
		show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

		return NULL;
	}
	update_node_list(ses->list[LIST_CONFIG], config_table[index].name, capitalize(arg), "", "");

	return ses;
}
