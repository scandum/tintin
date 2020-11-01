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
*                      coded by Igor van den Hoven 2008                       *
******************************************************************************/

#include "tintin.h"

extern struct command_type command_table[];

struct scriptnode
{
	struct scriptnode    * next;
	struct scriptnode    * prev;
	union
	{
		struct scriptdata   * data;
		struct script_regex * regex;
	};
	char                 * str;
	short                  lvl;
	short                  type;
	short                  cmd;
};

struct scriptdata
{
	long long              min;
	long long              max;
	long long              cnt;
	int                    inc;
	char                 * cpy;
	char                 * hlt;
	char                 * str;
	char                 * arg;
};

struct script_regex
{
	char                 * str;
	char                 * bod;
	char                 * buf;
	int                    val;
};

void debugtoken(struct session *ses, struct scriptroot *root, struct scriptnode *token)
{
	push_call("debugtoken(%p,%d,%p,%d)",ses,root->list,token,token->type);

	if (gtd->level->debug || HAS_BIT(root->ses->list[root->list]->flags, LIST_FLAG_DEBUG) || HAS_BIT(root->ses->list[root->list]->flags, LIST_FLAG_LOG))
	{
		switch (token->type)
		{
			case TOKEN_TYPE_REPEAT:
				show_debug(ses, root->list, "%s" COLOR_REPEAT "!\e[0m%s", indent(token->lvl + 1), token->str);
				break;

			case TOKEN_TYPE_STRING:
			case TOKEN_TYPE_SESSION:
				show_debug(ses, root->list, "%s%s", indent(token->lvl + 1), token->str);
				break;

			case TOKEN_TYPE_ELSE:
			case TOKEN_TYPE_END:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s\e[0m", indent(token->lvl + 1), gtd->tintin_char, token->str);
				break;

			case TOKEN_TYPE_DEFAULT:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name);
				break;

			case TOKEN_TYPE_BREAK:
			case TOKEN_TYPE_CONTINUE:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name);
				break;

			case TOKEN_TYPE_COMMAND:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_COMMAND   "%s " COLOR_STRING "%s\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name, token->str);
				break;

			case TOKEN_TYPE_RETURN:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_STRING "%s\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name, token->str);
				break;

			case TOKEN_TYPE_CASE:
			case TOKEN_TYPE_ELSEIF:
			case TOKEN_TYPE_IF:
			case TOKEN_TYPE_WHILE:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name, token->str);
				break;

			case TOKEN_TYPE_FOREACH:
			case TOKEN_TYPE_LOOP:
			case TOKEN_TYPE_PARSE:
			case TOKEN_TYPE_SWITCH:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_STRING "%s\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name, token->data->hlt);
				break;

			case TOKEN_TYPE_REGEX:
				show_debug(ses, root->list, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}\e[0m", indent(token->lvl + 1), gtd->tintin_char, command_table[token->cmd].name, token->str, token->regex->str);
				break;

			default:
				if (token == (struct scriptnode *) ses)
				{
					show_debug(ses, root->list, "[--] (error) token == ses");
				}
				else
				{
					show_debug(ses, root->list, "[%02d] %s\e[1;33m%d {\e[0m%s\e[1;32m}\e[0m", token->type, indent(token->lvl + 1), token->cmd, token->str);
				}
				break;
		}
	}
	pop_call();
	return;
}


void addtoken(struct scriptroot *root, int lvl, int opr, int cmd, char *str)
{
	struct scriptnode *token;

	token = (struct scriptnode *) calloc(1, sizeof(struct scriptnode));

	token->lvl = lvl;
	token->type = opr;
	token->cmd = cmd;
	token->str = strdup(str);

	LINK(token, root->next, root->prev);
}


char *addlooptoken(struct scriptroot *root, int lvl, int opr, int cmd, char *str)
{
	struct scriptdata *data;

	char min[BUFFER_SIZE], max[BUFFER_SIZE], var[BUFFER_SIZE];

	data = (struct scriptdata *) calloc(1, sizeof(struct scriptdata));

	str = get_arg_in_braces(root->ses, str, min, GET_ONE);
	str = get_arg_in_braces(root->ses, str, max, GET_ONE);
	str = get_arg_in_braces(root->ses, str, var, GET_ONE);

	data->cpy = restringf(NULL, "{%s} {%s} {%s}", min, max, var);
	data->hlt = restringf(NULL, COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", min, max, var);

	addtoken(root, lvl, opr, cmd, var);

	data->str = strdup("");

	root->prev->data = data;

	return str;
}

char *addswitchtoken(struct scriptroot *root, int lvl, int opr, int cmd, char *str)
{
	struct scriptdata *data;

	char arg[BUFFER_SIZE];

	data = (struct scriptdata *) calloc(1, sizeof(struct scriptdata));

	str = get_arg_in_braces(root->ses, str, arg, GET_ONE);

	data->cpy = restringf(NULL, "{%s}", arg);
	data->hlt = restringf(NULL, COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}", arg);

	addtoken(root, lvl, opr, cmd, arg);

	data->str = strdup("");

	root->prev->data = data;

	return str;
}

void resetlooptoken(struct session *ses, struct scriptnode *token)
{
	char *str, min[BUFFER_SIZE], max[BUFFER_SIZE];

	str = token->data->cpy;

	str = get_arg_in_braces(ses, str, min, GET_ONE);
	str = get_arg_in_braces(ses, str, max, GET_ONE);

	token->data->min = (int) get_number(ses, min);
	token->data->max = (int) get_number(ses, max);

	token->data->inc = token->data->min <= token->data->max ? 1 : -1;
	token->data->cnt = token->data->min;
}

void breaklooptoken(struct scriptnode *token)
{
	token->data->min = token->data->max = token->data->cnt = token->data->inc = 0;
}

char *addforeachtoken(struct scriptroot *root, int lvl, int opr, int cmd, char *str)
{
	struct scriptdata *data;

	char *arg, var[BUFFER_SIZE];

	arg = str_dup(str);

	str = get_arg_in_braces(root->ses, str, arg, GET_ONE);
	str = get_arg_in_braces(root->ses, str, var, GET_ONE);

	addtoken(root, lvl, opr, cmd, var);

	data = (struct scriptdata *) calloc(1, sizeof(struct scriptdata));

	data->cpy = restringf(NULL, "{%s} {%s}", arg, var);
	data->hlt = restringf(NULL, COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", arg, var);
	data->str = strdup("");
	data->arg = data->str;

	root->prev->data = data;

	str_free(arg);

	return str;
}

void resetforeachtoken(struct session *ses, struct scriptnode *token)
{
	char *str, arg[BUFFER_SIZE];

	str = token->data->cpy;

	str = sub_arg_in_braces(ses, str, arg, GET_ONE, SUB_VAR|SUB_FUN);

	RESTRING(token->data->str, arg);

	token->data->arg = token->data->str;
}

void breakforeachtoken(struct scriptnode *token)
{
	RESTRING(token->data->str, "");

	token->data->arg = token->data->str;
}

void handlereturntoken(struct session *ses, struct scriptnode *token)
{
	char arg[BUFFER_SIZE];

	substitute(ses, token->str, arg, SUB_VAR|SUB_FUN);

	if (gtd->script_index == 0)
	{
		set_nest_node_ses(ses, "result", "%s", arg);
	}
	else
	{
		DEL_BIT(gtd->flags, TINTIN_FLAG_LOCAL);

		set_nest_node(gtd->script_stack[gtd->script_index - 1]->local, "result", "%s", arg);

		SET_BIT(gtd->flags, TINTIN_FLAG_LOCAL);
	}
}

void handleswitchtoken(struct session *ses, struct scriptnode *token)
{
	char arg[BUFFER_SIZE];

	mathexp(ses, token->str, arg, 0);

	RESTRING(token->data->str, arg);
}

char *get_arg_foreach(struct scriptroot *root, struct scriptnode *token)
{
	static char buf[BUFFER_SIZE];

	token->data->arg = get_arg_in_braces(root->ses, token->data->arg, buf, GET_ALL);

	if (*token->data->arg == COMMAND_SEPARATOR)
	{
		token->data->arg++;
	}
	return buf;
}

char *addparsetoken(struct scriptroot *root, int lvl, int opr, int cmd, char *str)
{
	struct scriptdata *data;

	char arg[BUFFER_SIZE], var[BUFFER_SIZE];

	str = get_arg_in_braces(root->ses, str, arg, GET_ONE);
	str = get_arg_in_braces(root->ses, str, var, GET_ONE);

	addtoken(root, lvl, opr, cmd, var);

	data = (struct scriptdata *) calloc(1, sizeof(struct scriptdata));

	data->cpy = restringf(NULL, "{%s} {%s}", arg, var);
	data->hlt = restringf(NULL, COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}", arg, var);

	data->str = strdup("");
	data->arg = data->str;

	root->prev->data = data;

	return str;
}

void resetparsetoken(struct session *ses, struct scriptnode *token)
{
	char *str, arg[BUFFER_SIZE];

	str = token->data->cpy;

	str = sub_arg_in_braces(ses, str, arg, GET_ONE, SUB_VAR|SUB_FUN);

	RESTRING(token->data->str, arg);

	token->data->arg = token->data->str;
}

void breakparsetoken(struct scriptnode *token)
{
	RESTRING(token->data->str, "");

	token->data->arg = token->data->str;
}

char *get_arg_parse(struct session *ses, struct scriptnode *token)
{
	static char buf[5];

	if (HAS_BIT(ses->charset, CHARSET_FLAG_EUC) && is_euc_head(ses, token->data->arg))
	{
		token->data->arg += sprintf(buf, "%.*s", get_euc_size(ses, token->data->arg), token->data->arg);
	}
	else if (HAS_BIT(ses->charset, CHARSET_FLAG_UTF8) && is_utf8_head(token->data->arg))
	{
		token->data->arg += sprintf(buf, "%.*s", get_utf8_size(token->data->arg), token->data->arg);
	}
	else
	{
		token->data->arg += sprintf(buf, "%c", token->data->arg[0]);
	}

	return buf;
}

char *addregextoken(struct scriptroot *root, int lvl, int type, int cmd, char *str)
{
	struct script_regex *regex;

	char arg1[BUFFER_SIZE], arg2[BUFFER_SIZE], arg3[BUFFER_SIZE];

	str = get_arg_in_braces(root->ses, str, arg1, GET_ONE);
	str = get_arg_in_braces(root->ses, str, arg2, GET_ONE);
	str = get_arg_in_braces(root->ses, str, arg3, GET_ALL);

	addtoken(root, lvl, type, cmd, arg1);

	regex = (struct script_regex *) calloc(1, sizeof(struct script_regex));

	regex->str = strdup(arg2);
	regex->bod = strdup(arg3);
	regex->buf = calloc(1, BUFFER_SIZE);

	root->prev->regex = regex;

	return str;
}

void deltoken(struct scriptroot *root, struct scriptnode *token)
{
	UNLINK(token, root->next, root->prev);

	free(token->str);

	switch (token->type)
	{
		case TOKEN_TYPE_REGEX:
			free(token->regex->str);
			free(token->regex->bod);
			free(token->regex->buf);
			free(token->regex);
			break;

		case TOKEN_TYPE_LOOP:
		case TOKEN_TYPE_FOREACH:
		case TOKEN_TYPE_PARSE:
		case TOKEN_TYPE_SWITCH:
			free(token->data->cpy);
			free(token->data->hlt);
			free(token->data->str);
			free(token->data);
			break;
	}

	free(token);
}


int find_command(char *command)
{
	int cmd;

	if (find_session(command))
	{
		return -1;
	}

	if (is_alpha(*command) && command[1] != 0)
	{
		for (cmd = gtd->command_ref[*command % 32] ; *command_table[cmd].name ; cmd++)
		{
			if (is_abbrev(command, command_table[cmd].name))
			{
				return cmd;
			}
		}
	}
	return -1;
}

void init_local(struct session *ses)
{
	struct scriptroot *root;

	root = (struct scriptroot *) calloc(1, sizeof(struct scriptroot));

	root->ses = ses;
	root->list = LIST_VARIABLE;
	root->local = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	gtd->script_stack[0] = root;

	return;
}

struct listroot *local_list(struct session *ses)
{
	struct listroot *root;

	root = gtd->script_stack[gtd->script_index]->local;

	return root;
}

struct scriptroot *push_script_stack(struct session *ses, int list)
{
	struct scriptroot *root;

	root = (struct scriptroot *) calloc(1, sizeof(struct scriptroot));

	root->ses = ses;
	root->list = list;
	root->local = init_list(ses, LIST_VARIABLE, LIST_SIZE);

	gtd->script_stack[++gtd->script_index] = root;

	return root;
}

void pop_script_stack(void)
{
	free_list(gtd->script_stack[gtd->script_index]->local);

	free(gtd->script_stack[gtd->script_index]);

	if (--gtd->script_index == 0)
	{
		if (HAS_BIT(gtd->flags, TINTIN_FLAG_LOCAL))
		{
			kill_list(gtd->script_stack[0]->local);

			DEL_BIT(gtd->flags, TINTIN_FLAG_LOCAL);
		}
	}
}


void tokenize_script(struct scriptroot *root, int lvl, char *str)
{
	char *arg, *line;
	int cmd;

	push_call("tokenize_script(%p,%d,%p)",root,lvl,str);

	if (*str == 0)
	{
		addtoken(root, lvl, TOKEN_TYPE_STRING, -1, "");

		pop_call();
		return;
	}

	line = str_alloc_stack(strlen(str));

	while (*str)
	{
		if (!VERBATIM(root->ses))
		{
			str = space_out(str);
		}

		if (*str != gtd->tintin_char)
		{
			if (*str == gtd->repeat_char && gtd->level->repeat == 0)
			{
				str = get_arg_all(root->ses, str+1, line, VERBATIM(root->ses));

				addtoken(root, lvl, TOKEN_TYPE_REPEAT, -1, line);
			}
			else
			{
				str = get_arg_all(root->ses, str, line, VERBATIM(root->ses));

				addtoken(root, lvl, TOKEN_TYPE_STRING, -1, line);
			}
		}
		else
		{
//			arg = get_arg_in_braces(root->ses, str+1, line, GET_ONE);
			arg = sub_arg_in_braces(root->ses, str+1, line, GET_ONE, SUB_VAR|SUB_FUN);

			cmd = find_command(line);

			if (cmd == -1)
			{
				str = get_arg_all(root->ses, str, line, 0);
				addtoken(root, lvl, TOKEN_TYPE_SESSION, -1, line+1);
			}
			else
			{
				switch (command_table[cmd].type)
				{
					case TOKEN_TYPE_BREAK:
						str = get_arg_with_spaces(root->ses, arg, line, 1);
						addtoken(root, lvl, TOKEN_TYPE_BREAK, cmd, line);
						break;

					case TOKEN_TYPE_CASE:
						str = get_arg_in_braces(root->ses, arg, line, GET_ONE);
						addtoken(root, lvl++, TOKEN_TYPE_CASE, cmd, line);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endcase");
						break;

					case TOKEN_TYPE_CONTINUE:
						str = get_arg_with_spaces(root->ses, arg, line, 1);
						addtoken(root, lvl, TOKEN_TYPE_CONTINUE, cmd, line);
						break;

					case TOKEN_TYPE_DEFAULT:
						addtoken(root, lvl++, TOKEN_TYPE_DEFAULT, cmd, "");

						str = get_arg_in_braces(root->ses, arg, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "enddefault");
						break;

					case TOKEN_TYPE_ELSE:
						addtoken(root, lvl++, TOKEN_TYPE_ELSE, cmd, "else");

						str = get_arg_in_braces(root->ses, arg, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endelse");
						break;

					case TOKEN_TYPE_ELSEIF:
						str = get_arg_in_braces(root->ses, arg, line, GET_ONE);
						addtoken(root, lvl++, TOKEN_TYPE_ELSEIF, cmd, line);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endif");
						break;

					case TOKEN_TYPE_FOREACH:
						str = addforeachtoken(root, lvl++, TOKEN_TYPE_FOREACH, cmd, arg);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endforeach");
						break;

					case TOKEN_TYPE_IF:
						str = get_arg_in_braces(root->ses, arg, line, GET_ONE);
						addtoken(root, lvl++, TOKEN_TYPE_IF, cmd, line);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endif");

						if (*str && *str != COMMAND_SEPARATOR)
						{
							addtoken(root, lvl++, TOKEN_TYPE_ELSE, -1, "else");

							str = get_arg_in_braces(root->ses, str, line, GET_ALL);
							tokenize_script(root, lvl--, line);

							addtoken(root, lvl, TOKEN_TYPE_END, -1, "endif");
						}
						break;

					case TOKEN_TYPE_LOOP:
						str = addlooptoken(root, lvl++, TOKEN_TYPE_LOOP, cmd, arg);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endloop");
						break;

					case TOKEN_TYPE_PARSE:
						str = addparsetoken(root, lvl++, TOKEN_TYPE_PARSE, cmd, arg);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endparse");
						break;

					case TOKEN_TYPE_REGEX:
						str = addregextoken(root, lvl, TOKEN_TYPE_REGEX, cmd, arg);

//						addtoken(root, --lvl, TOKEN_TYPE_END, -1, "endregex");

						if (*str && *str != COMMAND_SEPARATOR)
						{
							addtoken(root, lvl++, TOKEN_TYPE_ELSE, -1, "else");

							str = get_arg_in_braces(root->ses, str, line, GET_ALL);
							tokenize_script(root, lvl--, line);

							addtoken(root, lvl, TOKEN_TYPE_END, -1, "endregex");
						}
						break;

					case TOKEN_TYPE_RETURN:
						str = get_arg_in_braces(root->ses, arg, line, GET_ALL);
						addtoken(root, lvl, TOKEN_TYPE_RETURN, cmd, line);
						break;

					case TOKEN_TYPE_SWITCH:
						str = addswitchtoken(root, lvl++, TOKEN_TYPE_SWITCH, cmd, arg);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endswitch");
						break;

					case TOKEN_TYPE_WHILE:
						str = get_arg_in_braces(root->ses, arg, line, GET_ONE);
						addtoken(root, lvl++, TOKEN_TYPE_WHILE, cmd, line);

						str = get_arg_in_braces(root->ses, str, line, GET_ALL);
						tokenize_script(root, lvl--, line);

						addtoken(root, lvl, TOKEN_TYPE_END, -1, "endwhile");
						break;

					default:
						str = get_arg_with_spaces(root->ses, arg, line, GET_ALL);
						addtoken(root, lvl, TOKEN_TYPE_COMMAND, cmd, line);
						break;
				}
			}
		}
		str = space_out(str);

		if (*str == COMMAND_SEPARATOR)
		{
			str++;
		}
	}
	pop_call();
	return;
}


struct scriptnode *parse_script(struct scriptroot *root, int lvl, struct scriptnode *token, struct scriptnode *shift)
{
	struct scriptnode *split = NULL;

	while (token)
	{
		if (token->lvl < lvl)
		{
			if (shift->lvl + 1 == lvl)
			{
				switch (shift->type)
				{
					case TOKEN_TYPE_FOREACH:
					case TOKEN_TYPE_LOOP:
					case TOKEN_TYPE_PARSE:
					case TOKEN_TYPE_WHILE:
						debugtoken(root->ses, root, token);
						return shift;

					case TOKEN_TYPE_BROKEN_FOREACH:
					case TOKEN_TYPE_BROKEN_LOOP:
					case TOKEN_TYPE_BROKEN_PARSE:
					case TOKEN_TYPE_BROKEN_WHILE:
						shift->type--;
						return token;
				}
			}
			return token;
		}

		debugtoken(root->ses, root, token);

		switch (token->type)
		{
			case TOKEN_TYPE_BREAK:
				switch (shift->type)
				{
					case TOKEN_TYPE_FOREACH:
						breakforeachtoken(shift);
						shift->type++;
						break;
					case TOKEN_TYPE_LOOP:
						breaklooptoken(shift);
						shift->type++;
						break;
					case TOKEN_TYPE_PARSE:
						breakparsetoken(shift);
						shift->type++;
						break;
					case TOKEN_TYPE_WHILE:
						shift->type++;
						break;
				}

				do
				{
					token = token->next;
				}
				while (token && token->lvl > shift->lvl);

				continue;

			case TOKEN_TYPE_CASE:
				if (shift->data && mathswitch(root->ses, shift->data->str, token->str))
				{

					token = token->next;

					token = parse_script(root, lvl + 1, token, shift);

					while (token && token->lvl >= lvl)
					{
						token = token->next;
					}
				}
				else
				{
					do
					{
						token = token->next;
					}
					while (token && token->lvl > lvl);
				}
				continue;

			case TOKEN_TYPE_COMMAND:
				if (push_call_printf("do_%s(%s,%p)", command_table[token->cmd].name, root->ses->name, token->str))
				{
					switch (command_table[token->cmd].args)
					{
						case 0:
							root->ses = (*command_table[token->cmd].command) (root->ses, token->str, NULL, NULL, NULL, NULL);
							break;
						case 1:
							root->ses = (*command_table[token->cmd].command) (root->ses, token->str, str_alloc_stack(0), NULL, NULL, NULL);
							break;
						case 2:
							root->ses = (*command_table[token->cmd].command) (root->ses, token->str, str_alloc_stack(0), str_alloc_stack(0), NULL, NULL);
							break;
						case 3:
							root->ses = (*command_table[token->cmd].command) (root->ses, token->str, str_alloc_stack(0), str_alloc_stack(0), str_alloc_stack(0), NULL);
							break;
						case 4:
							tintin_printf2(gtd->ses, "error: parse_script: command_table[%d].command == 4", token->cmd);
							root->ses = (*command_table[token->cmd].command) (root->ses, token->str, str_alloc_stack(0), str_alloc_stack(0), str_alloc_stack(0), str_alloc_stack(0));
							break;
					}
				}
				pop_call();
/*
	return;
}
*/
				break;

			case TOKEN_TYPE_CONTINUE:

				do
				{
					token = token->next;
				}
				while (token && token->lvl > shift->lvl);

				continue;

			case TOKEN_TYPE_DEFAULT:
				token = token->next;

				token = parse_script(root, lvl + 1, token, shift);

				while (token && token->lvl >= lvl)
				{
					token = token->next;
				}
				continue;

			case TOKEN_TYPE_ELSE:
				if (split)
				{
					token = parse_script(root, lvl + 1, token->next, shift);

					split = NULL;
				}
				else
				{
					do
					{
						token = token->next;
					}
					while (token && token->lvl > lvl);
				}
				continue;

			case TOKEN_TYPE_ELSEIF:
				if (split && get_number(root->ses, token->str))
				{
					token = parse_script(root, lvl + 1, token->next, shift);

					split = NULL;
				}
				else
				{
					do
					{
						token = token->next;
					}
					while (token && token->lvl > lvl);
				}
				continue;

			case TOKEN_TYPE_END:
				break;

			case TOKEN_TYPE_FOREACH:
				if (*token->data->arg == 0)
				{
					resetforeachtoken(root->ses, token);
				}

				if (*token->data->arg == 0)
				{
//					token->type++;

					do
					{
						token = token->next;
					}
					while (token && token->lvl > lvl);
				}
				else
				{
					set_nest_node_ses(root->ses, token->str, "%s", get_arg_foreach(root, token));

					if (*token->data->arg == 0)
					{
						token->type++;
					}
					token = parse_script(root, lvl + 1, token->next, token);
				}
				continue;

			case TOKEN_TYPE_IF:
				split = NULL;

				if (get_number(root->ses, token->str))
				{
					token = parse_script(root, lvl + 1, token->next, shift);
				}
				else
				{
					split = token;

					do
					{
						token = token->next;
					}
					while (token && token->lvl > lvl);
				}
				continue;

			case TOKEN_TYPE_LOOP:
				if (token->data->cnt == token->data->max + token->data->inc)
				{
					resetlooptoken(root->ses, token);
				}

				set_nest_node_ses(root->ses, token->str, "%lld", token->data->cnt);

				token->data->cnt += token->data->inc;

				if (token->data->cnt == token->data->max + token->data->inc)
				{
					token->type++;
				}

				token = parse_script(root, lvl + 1, token->next, token);

				continue;

			case TOKEN_TYPE_PARSE:
				if (*token->data->arg == 0)
				{
					resetparsetoken(root->ses, token);

					if (*token->data->arg == 0)
					{
//						token->type++;

						do
						{
							token = token->next;
						}
						while (token && token->lvl > lvl);

						continue;
					}

				}

				set_nest_node_ses(root->ses, token->str, "%s", get_arg_parse(root->ses, token));

				if (*token->data->arg == 0)
				{
					token->type++;
				}
				token = parse_script(root, lvl + 1, token->next, token);

				continue;

			case TOKEN_TYPE_REGEX:
				split = NULL;

				token->regex->val = find(root->ses, token->str, token->regex->str, SUB_VAR|SUB_FUN, REGEX_FLAG_CMD);

				if (token->regex->val)
				{
					substitute(root->ses, token->regex->bod, token->regex->buf, SUB_CMD);

					root->ses = script_driver(root->ses, LIST_COMMAND, token->regex->buf);
				}
				else
				{
					split = token;
				}
				break;

			case TOKEN_TYPE_RETURN:
				handlereturntoken(root->ses, token);

				goto end;

			case TOKEN_TYPE_SESSION:
				root->ses = parse_tintin_command(root->ses, token->str);
				break;

			case TOKEN_TYPE_STRING:
				root->ses = parse_input(root->ses, token->str);
				break;

			case TOKEN_TYPE_REPEAT:
				root->ses = repeat_history(root->ses, token->str);
				break;

			case TOKEN_TYPE_SWITCH:
				handleswitchtoken(root->ses, token);

				token = parse_script(root, lvl + 1, token->next, token);
				continue;

			case TOKEN_TYPE_WHILE:
				if (get_number(root->ses, token->str))
				{
					token = parse_script(root, lvl + 1, token->next, token);
				}
				else
				{
//					token->type++;

					do
					{
						token = token->next;
					}
					while (token && token->lvl > lvl);
				}
				continue;
		}

		if (token)
		{
			token = token->next;
		}
	}

	end:

	if (lvl)
	{
		return NULL;
	}

	return (struct scriptnode *) root->ses;
}


char *write_script(struct session *ses, struct scriptroot *root)
{
	struct scriptnode *token;
	static char buf[STRING_SIZE];

	token = root->next;

	buf[0] = 0;

	while (token)
	{
		switch (token->type)
		{
			case TOKEN_TYPE_STRING:
				cat_sprintf(buf, "%s%s", indent(token->lvl), token->str);
				break;

			case TOKEN_TYPE_BREAK:
			case TOKEN_TYPE_CONTINUE:
				cat_sprintf(buf, "%s%c%s", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name);
				break;

			case TOKEN_TYPE_COMMAND:
			case TOKEN_TYPE_RETURN:
				cat_sprintf(buf, "%s%c%s%s%s", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, *token->str ? " " : "", token->str);
				break;

			case TOKEN_TYPE_ELSE:
				cat_sprintf(buf, "%s%c%s\n%s{\n", indent(token->lvl), gtd->tintin_char, token->str, indent(token->lvl));
				break;

			case TOKEN_TYPE_DEFAULT:
				cat_sprintf(buf, "%s%c%s\n%s{\n", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, indent(token->lvl));
				break;

			case TOKEN_TYPE_FOREACH:
			case TOKEN_TYPE_LOOP:
			case TOKEN_TYPE_PARSE:
			case TOKEN_TYPE_SWITCH:
				cat_sprintf(buf, "%s%c%s %s\n%s{\n", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, token->data->cpy, indent(token->lvl));
				break;

			case TOKEN_TYPE_CASE:
			case TOKEN_TYPE_ELSEIF:
			case TOKEN_TYPE_IF:
			case TOKEN_TYPE_WHILE:
				cat_sprintf(buf, "%s%c%s {%s}\n%s{\n", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, token->str, indent(token->lvl));
				break;

			case TOKEN_TYPE_REGEX:
				cat_sprintf(buf, "%s%c%s {%s} {%s}\n%s{\n%s%s\n%s}", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, token->str, token->regex->str, indent(token->lvl), indent(token->lvl + 1), token->regex->bod, indent(token->lvl));
				break;

			case TOKEN_TYPE_END:
				cat_sprintf(buf, "\n%s}", indent(token->lvl));
				break;

			case TOKEN_TYPE_SESSION:
				cat_sprintf(buf, "%s%c%s", indent(token->lvl), gtd->tintin_char, token->str);
				break;

			default:
				tintin_printf2(ses, "#WRITE: UNKNOWN TOKEN TYPE: %d", token->type);
				break;
		}

		if (token->next && token->lvl == token->next->lvl)
		{
			strcat(buf, ";\n");
		}

		token = token->next;
	}

	while (root->next)
	{
		deltoken(root, root->next);
	}

	free(root);

	return buf;
}

char *view_script(struct session *ses, struct scriptroot *root)
{
	struct scriptnode *token;
	static char buf[STRING_SIZE];

	token = root->next;

	buf[0] = 0;

	while (token)
	{
		switch (token->type)
		{
			case TOKEN_TYPE_STRING:
				cat_sprintf(buf, "%s" COLOR_STRING "%s", indent(token->lvl), token->str);
				break;

			case TOKEN_TYPE_BREAK:
			case TOKEN_TYPE_CONTINUE:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name);
				break;

			case TOKEN_TYPE_RETURN:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s" COLOR_STRING "%s%s", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, *token->str ? " " : "", token->str);
				break;

			case TOKEN_TYPE_COMMAND:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_COMMAND "%s" COLOR_STRING "%s%s", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, *token->str ? " " : "", token->str);
				break;

			case TOKEN_TYPE_ELSE:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s\n%s" COLOR_BRACE "{\n", indent(token->lvl), gtd->tintin_char, token->str, indent(token->lvl));
				break;

			case TOKEN_TYPE_DEFAULT:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s\n%s" COLOR_BRACE "{\n", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, indent(token->lvl));
				break;

			case TOKEN_TYPE_FOREACH:
			case TOKEN_TYPE_LOOP:
			case TOKEN_TYPE_PARSE:
			case TOKEN_TYPE_SWITCH:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_STRING "%s\n%s" COLOR_BRACE "{\n", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, token->data->hlt, indent(token->lvl));
				break;

			case TOKEN_TYPE_CASE:
			case TOKEN_TYPE_ELSEIF:
			case TOKEN_TYPE_IF:
			case TOKEN_TYPE_WHILE:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "}" COLOR_STRING "\n%s" COLOR_BRACE "{\n", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, token->str, indent(token->lvl));
				break;

			case TOKEN_TYPE_REGEX:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STATEMENT "%s " COLOR_BRACE "{" COLOR_STRING "%s" COLOR_BRACE "} {" COLOR_STRING "%s" COLOR_BRACE "}\n%s{\n%s" COLOR_STRING "%s\n" COLOR_BRACE "%s}", indent(token->lvl), gtd->tintin_char, command_table[token->cmd].name, token->str, token->regex->str, indent(token->lvl), indent(token->lvl + 1), token->regex->bod, indent(token->lvl));
				break;

			case TOKEN_TYPE_END:
				cat_sprintf(buf, "\n%s" COLOR_BRACE "}" COLOR_STRING, indent(token->lvl));
				break;

			case TOKEN_TYPE_SESSION:
				cat_sprintf(buf, "%s" COLOR_TINTIN "%c" COLOR_STRING "%s", indent(token->lvl), gtd->tintin_char, token->str);
				break;

			default:
				tintin_printf2(ses, "#ERROR: UNKNOWN TOKEN TYPE: %d", token->type);
				break;
		}

		if (token->next && token->lvl == token->next->lvl)
		{
			strcat(buf, COLOR_SEPARATOR ";\n");
		}

		token = token->next;
	}

	while (root->next)
	{
		deltoken(root, root->next);
	}

	free(root);

	return buf;
}

struct session *script_driver(struct session *ses, int list, char *str)
{
	struct scriptroot *root;

	push_call("script_driver(%p,%d,%p)",ses,list,str);

	root = push_script_stack(ses, list);

	gtd->level->input += list != LIST_COMMAND;

	tokenize_script(root, 0, str);

	ses = (struct session *) parse_script(root, 0, root->next, root->prev);

	gtd->level->input -= list != LIST_COMMAND;

	while (root->prev)
	{
		deltoken(root, root->prev);
	}

	pop_script_stack();

	if (HAS_BIT(ses->flags, SES_FLAG_CLOSED))
	{
		pop_call();
		return gtd->ses;
	}
	pop_call();
	return ses;
}


char *script_writer(struct session *ses, char *str)
{
	struct scriptroot *root;

	root = (struct scriptroot *) calloc(1, sizeof(struct scriptroot));

	root->ses = ses;

	tokenize_script(root, 1, str);

	return write_script(ses, root);
}

char *script_viewer(struct session *ses, char *str)
{
	struct scriptroot *root;

	root = (struct scriptroot *) calloc(1, sizeof(struct scriptroot));

	root->ses = ses;

	tokenize_script(root, 1, str);

	return view_script(ses, root);
}
