/*
 * hajimete - A simple Linux system starter.
 * Functions for kernel cmdline parsing
 * Copyright © 2009 Calvin Walton <calvin.walton@gmail.com>
 * 
 * Based in part on Linux include/linux/moduleparam.h
 * (C) Copyright 2001, 2002 Rusty Russell IBM Corporation
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

#ifndef HAJIMETE_KERNEL_PARAM_H
#define HAJIMETE_KERNEL_PARAM_H

struct kernel_param {
	const char *name;
	char *value;
};

int kernel_param_parse(char *args,
			struct kernel_param *params[]);

#endif /* HAJIMETE_KERNEL_PARAM_H */
