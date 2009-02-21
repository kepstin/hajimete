/*
 * hajimete - A simple Linux system starter.
 * Helper functions for executing other programs
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

#include "exec.h"

#include <stdarg.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAXARGS 31

int exec_run_wait(const char *path, char *arg, ...)
{
	va_list ap;
	char *array[MAXARGS +1];
	int argno = 0;
	pid_t pid;
	int err, status;
	
	va_start(ap, arg);
	while (arg && argno < MAXARGS) {
		array[argno++] = arg;
		arg = va_arg(ap, char *);
	}
	array[argno] = (char *) 0;
	va_end(ap);
	
	pid = fork();
	if (!pid) {
		err = execv(path, array);
		return err;
	} else if (pid == -1) {
		return -1;
	}
	
	err = waitpid(pid, &status, 0);
	if (err == -1)
		return err;
	return WEXITSTATUS(status);
}
