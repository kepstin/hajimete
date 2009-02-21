/*
 * hajimete - A simple Linux system starter.
 * Functions for kernel cmdline parsing
 * Copyright © 2009 Calvin Walton <calvin.walton@gmail.com>
 * 
 * Based on Linux kernel/params.c
 * Helpers for initial module or kernel cmdline parsing
 * Copyright (C) 2001 Rusty Russell.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "kernel_param.h"

#include <stdio.h>

static inline char dash2underscore(char c)
{
	if (c == '-')
		return '_';
	return c;
}

static inline int parameq(const char *input, const char *paramname)
{
	unsigned int i;
	for (i = 0; dash2underscore(input[i]) == paramname[i]; i++)
		if (input[i] == '\0')
			return 1;
	return 0;
}

static void parse_one(char *param,
		     char *val,
		     struct kernel_param *params[])
{
	unsigned int i;
	
	/* Find parameter */
	for (i = 0; params[i]; i++) {
		if (parameq(param, params[i]->name)) {
			params[i]->value = val;
		}
	}
}

/* You can use " around spaces, but can't escape ". */
/* Hyphens and underscores equivalent in parameter names. */
static char *next_arg(char *args, char **param, char **val)
{
	unsigned int i, equals = 0;
	int in_quote = 0, quoted = 0;
	char *next;
	
	if (*args == '"') {
		args++;
		in_quote = 1;
		quoted = 1;
	}
	
	for (i = 0; args[i]; i++) {
		if (args[i] == ' ' && !in_quote)
			break;
		if (equals == 0) {
			if (args[i] == '=')
				equals = i;
		}
		if (args[i] == '"')
			in_quote = !in_quote;
	}
	
	*param = args;
	if (!equals)
		*val = NULL;
	else {
		args[equals] = '\0';
		*val = args + equals + 1;
		
		/* Don't invlude quotes in value. */
		if (**val == '"') {
			(*val)++;
			if (args[i-1] == '"')
				args[i-1] = '\0';
		}
		if (quoted && args[i-1] == '"')
			args[i-1] = '\0';
	}
	
	if (args[i]) {
		args[i] = '\0';
		next = args + i + 1;
	} else
		next = args + i;
	
	/* Chew up trailing spaces. */
	while (*next == ' ')
		next++;
	
	return next;
}

/* Args looks like "foo=bar,bar2 baz=fuz wiz". */
int kernel_param_parse(char *args,
			struct kernel_param *params[])
{
	char *param, *val;
	
	if (!params)
		return 0;
	
	printf("Kernel Parameters: %s\n", args);
	
	/* Chew leading spaces */
	while (*args == ' ')
		args++;
	
	while (*args) {
		args = next_arg(args, &param, &val);
		parse_one(param, val, params);
	}
	
	/* All parsed OK. */
	return 0;
}
