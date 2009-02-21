/*
 * hajimete - A simple Linux system starter.
 * udev helper functions
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

#include "udev.h"

#include "exec.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

static pid_t udevd_pid;

int udev_init(void)
{
	pid_t pid;
	int err, status;
	unsigned long data;
	
	printf("Please wait while I initialize your devices...\n");
	
	printf("Starting udevd.\n");
	pid = fork();
	if (!pid) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		err = execl("/sbin/udevd", "/sbin/udevd", "--daemon", (char *) NULL);
		if (err)
			printf("Failed to start udevd: %s\n", strerror(errno));
		return 1;
	} else if (pid == -1) {
		printf("Could not fork: %s\n", strerror(errno));
		return -1;
	}
	printf("Waiting for signal from ptrace.\n");
	waitpid(pid, &status, 0);
	printf("Got it, setting up options.\n");
	ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACEFORK);
	ptrace(PTRACE_CONT, pid, NULL, NULL);
	
	printf("Waiting for fork.\n");
	waitpid(pid, &status, 0);
	ptrace(PTRACE_GETEVENTMSG, pid, NULL, &data);
	udevd_pid = data;
	printf("udevd forked, new pid is %d\n", udevd_pid);
	ptrace(PTRACE_DETACH, pid, NULL, NULL);
	ptrace(PTRACE_DETACH, udevd_pid, NULL, NULL);
	
	printf("Triggering events.\n");
	err = exec_run_wait("/sbin/udevadm", "/sbin/udevadm", "trigger", (char *) NULL);
	if (err) {
		printf("Failed to trigger udev events\n");
		return -1;
	}
	printf("Waiting for the events to propagate.\n");
	exec_run_wait("/sbin/udevadm", "/sbin/udevadm", "-d", "settle", (char *) NULL);
	
	printf("You should now have all your device nodes.\n");
	return 0;
}

int udev_kill(void)
{
	int err, status;
	
	printf("Stopping udevd.\n");
	
	err = kill(udevd_pid, SIGTERM);
	if (err) {
		printf("Failed to kill udevd: %s\n", strerror(errno));
		return err;
	}
	err = waitpid(udevd_pid, &status, 0);
	if (err == -1)
		return err;
	return 0;
}
