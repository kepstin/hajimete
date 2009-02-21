/*
 * hajimete - A simple Linux system starter.
 * Run a rescue system
 * Copyright © 2009 Calvin Walton <calvin.walton@gmail.com>
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "rescue.h"

#include "exec.h"

#include <stdio.h>

int rescue(void)
{
	printf("\nStarting a rescue shell.\n"
		"To resume booting, exit the shell.\n");
	return exec_run_wait("/bin/bash", "/bin/bash", (char *) NULL);
}
