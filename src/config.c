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
*                      coded by Igor van den Hoven 2004                       *
******************************************************************************/


#include "tintin.h"


DO_COMMAND(do_configure)
{
	struct listnode *node;
	int index;

	arg = sub_arg_in_braces(ses, arg, arg1, GET_ONE, SUB_VAR|SUB_FUN);
	arg = sub_arg_in_braces(ses, arg, arg2, GET_ALL, SUB_VAR|SUB_FUN);

	if (*arg1 == 0)
	{
		tintin_header(ses, 80, " CONFIGURATIONS ");

		for (index = 0 ; *config_table[index].name != 0 ; index++)
		{
			node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

			if (node)
			{
				strcpy(arg2, "");

				config_table[index].config(ses, arg1, arg2, index);

				tintin_printf2(ses, "[%-14s] [%12s] %s",
					node->arg1,
					arg2,
					strcmp(node->arg2, "OFF") ? config_table[index].msg_on : config_table[index].msg_off);
			}
		}

		tintin_header(ses, 80, "");
	}
	else
	{
		// fixing silly config name

		if (is_abbrev(arg1, "MOUSE TRACKING"))
		{
			str_cpy(&arg1, "MOUSE");
		}

		for (index = 0 ; *config_table[index].name != 0 ; index++)
		{
			if (is_abbrev(arg1, config_table[index].name))
			{
				if (*arg2)
				{
					if (config_table[index].config(ses, arg1, arg2, index) != NULL)
					{
						update_node_list(ses->list[LIST_CONFIG], config_table[index].name, arg2, "", "");

						node = search_node_list(ses->list[LIST_CONFIG], config_table[index].name);

						if (node)
						{
							show_message(ses, LIST_CONFIG, "#CONFIG {%s} HAS BEEN SET TO {%s}.", config_table[index].name, node->arg2);
						}
					}
				}
				else
				{
					config_table[index].config(ses, arg1, arg2, index);

					show_message(ses, LIST_CONFIG, "#CONFIG {%s} IS SET TO {%s}.", config_table[index].name, arg2);
				}
				return ses;
			}
		}
		show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG {%s} IS NOT A VALID OPTION.", capitalize(arg1));
	}
	return ses;
}

DO_CONFIG(config_autotab)
{
	if (*arg2)
	{
		if (!is_number(arg2))
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {AUTO TAB} <NUMBER>");

			return NULL;
		}

		if (atoi(arg2) < 1 || atoi(arg2) > 999999)
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG BUFFER: PROVIDE A NUMBER BETWEEN 1 and 999999");

			return NULL;
		}

		ses->scrollback_tab = atoi(arg2);
	}
	sprintf(arg2, "%d", ses->scrollback_tab);

	return ses;
}


DO_CONFIG(config_buffersize)
{
	if (*arg2)
	{
		if (!is_number(arg2))
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {BUFFER SIZE} <NUMBER>");

			return NULL;
		}

		switch (atoi(arg2))
		{
			case 100:
			case 1000:
			case 10000:
			case 100000:
			case 1000000:
				break;

			default:
				show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG BUFFER: SIZE MUST BE 100, 1000, 10000, 100000, or 1000000.");
				return NULL;
		}
		init_buffer(ses, atoi(arg2));
	}
	sprintf(arg2, "%d", ses->scroll->size);

	return ses;
}

DO_CONFIG(config_charset)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "AUTO"))
		{
			if (strcasestr(gtd->system->lang, "UTF-8"))
			{
				DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
				SET_BIT(ses->charset, CHARSET_FLAG_UTF8);
			}
			else if (strcasestr(gtd->system->lang, "BIG-5"))
			{
				DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
				SET_BIT(ses->charset, CHARSET_FLAG_BIG5);
			}
			else if (strcasestr(gtd->system->term, "XTERM"))
			{
				DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
				SET_BIT(ses->charset, CHARSET_FLAG_UTF8);
			}
			else
			{
				DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			}
		}
		else if (is_abbrev(arg2, "ASCII"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
		}
		else if (is_abbrev(arg2, "BIG-5"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_BIG5);
		}
		else if (is_abbrev(arg2, "GBK-1"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_GBK1);
		}
		else if (is_abbrev(arg2, "UTF-8"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8);
		}
		else if (is_abbrev(arg2, "BIG5TOUTF8") || is_abbrev(arg2, "BIG2UTF"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_BIG5TOUTF8);
		}
		else if (is_abbrev(arg2, "CP1251TOUTF8"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_CP1251TOUTF8);
		}
		else if (is_abbrev(arg2, "FANSI"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_FANSITOUTF8);
		}
		else if (is_abbrev(arg2, "GBK1TOUTF8"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_GBK1TOUTF8);
		}
		else if (is_abbrev(arg2, "ISO1TOUTF8"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_ISO1TOUTF8);
		}
		else if (is_abbrev(arg2, "ISO2TOUTF8"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_ISO2TOUTF8);
		}
		else if (is_abbrev(arg2, "KOI8TOUTF8"))
		{
			DEL_BIT(ses->charset, CHARSET_FLAG_ALL);
			SET_BIT(ses->charset, CHARSET_FLAG_UTF8|CHARSET_FLAG_KOI8TOUTF8);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <AUTO|ASCII|BIG-5|BIG5TOUTF8|CP1251TOUTF8|FANSI|GBK-1|GBK1TOUTF8|ISO1TOUTF8|ISO2TOUTF8|KOI8TOUTF8|UTF-8>", config_table[index].name);

			return NULL;
		}
	}

	switch (HAS_BIT(ses->charset, CHARSET_FLAG_ALL))
	{
		case CHARSET_FLAG_BIG5:
			strcpy(arg2, "BIG-5");
			break;
		case CHARSET_FLAG_GBK1:
			strcpy(arg2, "GBK-1");
			break;
		case CHARSET_FLAG_UTF8:
			strcpy(arg2, "UTF-8");
			break;
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_BIG5TOUTF8:
			strcpy(arg2, "BIG5TOUTF8");
			break;
	        case CHARSET_FLAG_UTF8|CHARSET_FLAG_CP1251TOUTF8:
	        	strcpy(arg2, "CP1251TOUTF8");
	        	break;
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_FANSITOUTF8:
			strcpy(arg2, "FANSI");
			break;
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_GBK1TOUTF8:
			strcpy(arg2, "GBK1TOUTF8");
			break;
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_KOI8TOUTF8:
			strcpy(arg2, "KOI8TOUTF8");
			break;
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_ISO1TOUTF8:
			strcpy(arg2, "ISO1TOUTF8");
			break;
		case CHARSET_FLAG_UTF8|CHARSET_FLAG_ISO2TOUTF8:
			strcpy(arg2, "ISO2TOUTF8");
			break;
		default:
			strcpy(arg2, "ASCII");
			break;
	}

	return ses;
}

DO_CONFIG(config_childlock)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(gtd->flags, TINTIN_FLAG_CHILDLOCK) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_colormode)
{
	if (*arg2)
	{	
		if (is_abbrev(arg2, "NONE") || is_abbrev(arg2, "OFF"))
		{
			ses->color = 0;
		}
		else if (is_abbrev(arg2, "ANSI"))
		{
			ses->color = 16;
		}
		else if (is_abbrev(arg2, "256"))
		{
			ses->color = 256;
		}
		else if (is_abbrev(arg2, "TRUE") || is_abbrev(arg2, "ON") || is_abbrev(arg2, "AUTO"))
		{
			ses->color = 4096;
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF|ANSI|256|TRUE>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, ses->color == 0 ? "OFF" : ses->color == 16 ? "ANSI" : ses->color == 256 ? "256" : "TRUE");

	return ses;
}

DO_CONFIG(config_colorpatch)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_COLORPATCH);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_COLORPATCH);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_COLORPATCH) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_commandcolor)
{
	if (*arg2)
	{
		get_color_names(ses, arg2, arg1);

		RESTRING(ses->cmd_color, arg1);
	}
	convert_meta(ses->cmd_color, arg2, SUB_EOL);

	return ses;
}

DO_CONFIG(config_commandecho)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_ECHOCOMMAND);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_ECHOCOMMAND);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_ECHOCOMMAND) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_connectretry)
{
	if (*arg2)
	{
		if (!is_number(arg2))
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {CONNECT RETRY} <NUMBER>");

			return NULL;
		}
		else if (atof(arg2) < 0 || atof(arg2) > 10000)
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG CONNECT RETRY: PROVIDE A NUMBER BETWEEN 0.0 and 10000.0");

			return NULL;
		}
		gts->connect_retry = atoll(arg2) * 1000000LL;
	}

	sprintf(arg2, "%.1Lf", (long double) gts->connect_retry / 1000000);

	return ses;
}

DO_CONFIG(config_convertmeta)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_CONVERTMETA);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_CONVERTMETA);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_CONVERTMETA) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_debugtelnet)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_historysize)
{
	if (*arg2)
	{
		if (!is_number(arg2))
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {HISTORY SIZE} <NUMBER>");

			return NULL;
		}

		if (atoi(arg2) < 0 || atoi(arg2) > 9999)
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG HISTORY: PROVIDE A NUMBER BETWEEN 0 and 9999");

			return NULL;
		}

		gtd->history_size = atoi(arg2);
	}
	sprintf(arg2, "%d", gtd->history_size);

	return ses;
}

DO_CONFIG(config_inheritance)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(gtd->flags, TINTIN_FLAG_INHERITANCE);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(gtd->flags, TINTIN_FLAG_INHERITANCE);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(gtd->flags, TINTIN_FLAG_INHERITANCE) ? "ON" : "OFF");

	return ses;
}


DO_CONFIG(config_loglevel)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "LOW"))
		{
			SET_BIT(ses->logmode, LOG_FLAG_LOW);
		}
		else if (is_abbrev(arg2, "HIGH"))
		{
			DEL_BIT(ses->logmode, LOG_FLAG_LOW);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <LOW|HIGH>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->logmode, LOG_FLAG_LOW) ? "LOW" : "HIGH");

	return ses;
}


DO_CONFIG(config_logmode)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "HTML"))
		{
			SET_BIT(ses->logmode, LOG_FLAG_HTML);
			DEL_BIT(ses->logmode, LOG_FLAG_PLAIN);
			DEL_BIT(ses->logmode, LOG_FLAG_RAW);
		}
		else if (is_abbrev(arg2, "PLAIN"))
		{
			DEL_BIT(ses->logmode, LOG_FLAG_HTML);
			SET_BIT(ses->logmode, LOG_FLAG_PLAIN);
			DEL_BIT(ses->logmode, LOG_FLAG_RAW);
		}
		else if (is_abbrev(arg2, "RAW"))
		{
			DEL_BIT(ses->logmode, LOG_FLAG_HTML);
			DEL_BIT(ses->logmode, LOG_FLAG_PLAIN);
			SET_BIT(ses->logmode, LOG_FLAG_RAW);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG LOG <HTML|PLAIN|RAW>");

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->logmode, LOG_FLAG_HTML) ? "HTML" : HAS_BIT(ses->logmode, LOG_FLAG_PLAIN) ? "PLAIN" : "RAW");

	return ses;
}


DO_CONFIG(config_mccp)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_MCCP);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MCCP);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_MCCP) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_mousetracking)
{
	if (*arg2)
	{
		DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
		DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
		DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);	
		DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);
		DEL_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);

		if (is_member(arg2, "INFO"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		}
		if (is_member(arg2, "DEBUG"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		}
		if (is_member(arg2, "ON"))
		{
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		}
		if (is_member(arg2, "PIXELS"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		}

		if (is_abbrev(arg2, "OFF"))
		{
			if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
			{
				DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
				DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
				DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);
				DEL_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
				print_stdout(0, 0, "\e[?1000l\e[?1002l\e[?1004l\e[?1006l\e[?1016l");
			}
		}
		else
		{
			if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
			{
				if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS))
				{
					print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1016h");
				}
				else
				{
					print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1006h");
				}
			}
			else
			{
				show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF|DEBUG|INFO|PIXELS>", config_table[index].name);

				return NULL;
			}
		}
/*
		if (is_abbrev(arg2, "ON"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
			print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1006h");
		}
		else if (is_abbrev(arg2, "PIXELS"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
		
			print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1016h");
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
			{
				DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
				DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
				DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS);
				DEL_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
				print_stdout(0, 0, "\e[?1000l\e[?1002l\e[?1004l\e[?1006l");
			}
		}
		else if (is_abbrev(arg2, "DEBUG"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
			print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1006h");
		}
		else if (is_abbrev(arg2, "DEBUG INFO"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
			print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1006h");
		}
		else if (is_abbrev(arg2, "INFO"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG);
			SET_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO);
			SET_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING);
			print_stdout(0, 0, "\e[?1000h\e[?1002h\e[?1004h\e[?1006h");
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF|DEBUG|INFO|DEBUG INFO>", config_table[index].name);

			return NULL;
		}
*/
	}

	if (HAS_BIT(gtd->flags, TINTIN_FLAG_MOUSETRACKING))
	{
		if (!HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG|CONFIG_FLAG_MOUSEINFO|CONFIG_FLAG_MOUSEPIXELS))
		{
			strcpy(arg2, "ON");
		}
		else
		{
			strcpy(arg2, "");

			if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEPIXELS))
			{
				strcpy(arg2, "PIXELS");
			}

			if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEDEBUG))
			{
				strcat(arg2, *arg2 ? " DEBUG" : "DEBUG");
			}

			if (HAS_BIT(ses->config_flags, CONFIG_FLAG_MOUSEINFO))
			{
				strcat(arg2, *arg2 ? " INFO" : "INFO");
			}
		}
	}
	else
	{
		strcpy(arg2, "OFF");
	}
	return ses;
}


DO_CONFIG(config_packetpatch)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "AUTO PROMPT") || is_abbrev(arg2, "AUTO TELNET") || is_abbrev(arg2, "AUTO OFF"))
		{
			ses->packet_patch = 0;

			SET_BIT(ses->config_flags, CONFIG_FLAG_AUTOPATCH);
		}
		else if (!is_number(arg2))
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {PACKET PATCH} <NUMBER>");

			return NULL;
		}
		else if (atof(arg2) < 0 || atof(arg2) > 10)
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG PACKET PATCH: PROVIDE A NUMBER BETWEEN 0.00 and 10.00");

			return NULL;
		}
		else
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_AUTOPATCH);

			ses->packet_patch = (unsigned long long) (tintoi(arg2) * 1000000ULL);
		}
	}

	if (HAS_BIT(ses->config_flags, CONFIG_FLAG_AUTOPATCH))
	{
		if (ses->list[LIST_PROMPT]->list[0])
		{
			strcpy(arg2, "AUTO PROMPT");
		}
		else if (HAS_BIT(ses->config_flags, CONFIG_FLAG_AUTOPROMPT))
		{
			strcpy(arg2, "AUTO TELNET");
		}
		else
		{
			strcpy(arg2, "AUTO OFF");
		}
	}
	else
	{
		sprintf(arg2, "%4.2Lf", (long double) ses->packet_patch / 1000000);
	}
	return ses;
}

DO_CONFIG(config_randomseed)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "AUTO"))
		{
			seed_rand(ses, utime());
		}
		else if (is_number(arg2))
		{
			seed_rand(ses, get_number(ses, arg2));
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <AUTO|NUMBER>", config_table[index].name);

			return NULL;
		}
	}
	sprintf(arg2, "%u", (unsigned int) ses->rand);

	return ses;
}

DO_CONFIG(config_repeatchar)
{
	if (*arg2)
	{
		if (!ispunct((int) arg2[0]))
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG REPEAT CHAR: INVALID CHARACTER {%c}", arg2[0]);

			return NULL;
		}

		gtd->repeat_char = arg2[0];

		arg2[1] = 0;
	}

	sprintf(arg2, "%c", gtd->repeat_char);

	return ses;
}

DO_CONFIG(config_repeatenter)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_REPEATENTER);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_REPEATENTER);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_REPEATENTER) ? "ON" : "OFF");

	return ses;
}


DO_CONFIG(config_screenreader)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_SCREENREADER) ? "ON" : "OFF");

	return ses;
}




DO_CONFIG(config_scrolllock)
{
	if (*arg2)
	{	
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_SCROLLLOCK);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_SCROLLLOCK);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_SCROLLLOCK) ? "ON" : "OFF");	

	return ses;
}

DO_CONFIG(config_speedwalk)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_SPEEDWALK);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_SPEEDWALK);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_SPEEDWALK) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_tabwidth)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "AUTO"))
		{
			ses->tab_width = 8;
		}
		else if (!is_number(arg2))
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {TAB WIDTH} <NUMBER>");

			return NULL;
		}
		else if (atof(arg2) < 1 || atof(arg2) > 16)
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG TAB WIDTH: PROVIDE A NUMBER BETWEEN 1 and 16");

			return NULL;
		}
		else
		{
			ses->tab_width = (int) tintoi(arg2);
		}
	}
	sprintf(arg2, "%d", ses->tab_width);

	return ses;
}
	
DO_CONFIG(config_telnet)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			DEL_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
			SET_BIT(ses->config_flags, CONFIG_FLAG_TELNET);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
			DEL_BIT(ses->config_flags, CONFIG_FLAG_TELNET);
		}
		else if (is_abbrev(arg2, "DEBUG"))
		{
			SET_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
			SET_BIT(ses->config_flags, CONFIG_FLAG_TELNET);
		}
		else if (is_abbrev(arg2, "INFO"))
		{
			SET_BIT(ses->telopts, TELOPT_FLAG_DEBUG);
			SET_BIT(ses->config_flags, CONFIG_FLAG_TELNET);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF|DEBUG|INFO>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->telopts, TELOPT_FLAG_DEBUG) ? "DEBUG" : HAS_BIT(ses->config_flags, CONFIG_FLAG_TELNET) ? "ON" : "OFF");

	return ses;
}


DO_CONFIG(config_tintinchar)
{
	if (*arg2)
	{
		if (!ispunct((int) arg2[0]))
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG TINTIN CHAR: INVALID CHARACTER {%c}", arg2[0]);

			return NULL;
		}

		gtd->tintin_char = arg2[0];
	}
	sprintf(arg2, "%c", gtd->tintin_char);

	return ses;
}

DO_CONFIG(config_verbatim)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_VERBATIM);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_VERBATIM);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_VERBATIM) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_verbatimchar)
{
	if (*arg2)
	{
		if (!ispunct((int) arg2[0]))
		{
			show_error(ses, LIST_CONFIG, "#ERROR: #CONFIG VERBATIM CHAR: INVALID CHARACTER {%c}", arg2[0]);

			return NULL;
		}
		gtd->verbatim_char = arg2[0];
	}
	sprintf(arg2, "%c", gtd->verbatim_char);

	return ses;
}


DO_CONFIG(config_verbose)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_VERBOSE);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_VERBOSE);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_VERBOSE) ? "ON" : "OFF");

	return ses;
}

DO_CONFIG(config_wordwrap)
{
	if (*arg2)
	{
		if (is_abbrev(arg2, "ON"))
		{
			SET_BIT(ses->config_flags, CONFIG_FLAG_WORDWRAP);
		}
		else if (is_abbrev(arg2, "OFF"))
		{
			DEL_BIT(ses->config_flags, CONFIG_FLAG_WORDWRAP);
		}
		else
		{
			show_error(ses, LIST_CONFIG, "#SYNTAX: #CONFIG {%s} <ON|OFF>", config_table[index].name);

			return NULL;
		}
	}
	strcpy(arg2, HAS_BIT(ses->config_flags, CONFIG_FLAG_WORDWRAP) ? "ON" : "OFF");

	return ses;
}
