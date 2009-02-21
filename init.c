/*
 * hajimete - A simple Linux system starter.
 * Main init program
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

#include "kernel_param.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_LENGTH 2048

extern char **environ;

static char cmdline[CMDLINE_LENGTH];

static struct kernel_param param_root = { "root" };
static struct kernel_param param_ls = { "ls" };

static struct kernel_param *params[] = {
	&param_root,
	&param_ls,
	NULL
};

static int read_cmdline(char *cmdline)
{
	int err;
	int cmdline_fd;
	
	cmdline_fd = open("/proc/cmdline", O_RDONLY);
	if (cmdline_fd == -1) {
		printf("Could not open /proc/cmdline: %s\n", strerror(errno));
		return 1;
	}
	err = read(cmdline_fd, cmdline, CMDLINE_LENGTH);
	if (err == -1) {
		printf("Could not read cmdline: %s\n", strerror(errno));
		return 1;
	}
	if (err > 0)
		cmdline[err-1] = '\0';
	else
		cmdline[0] = '\0';
	
	return 0;
}

int main(int argc, char *argv[])
{
	int err, status;
	pid_t pid;
	
	printf("\thajimemasu\n");
		
	err = mount("proc", "/proc", "proc", 0, NULL);
	if (err)
		printf("Failed to mount /proc: %s\n", strerror(errno));

	err = mount("sysfs", "/sys", "sysfs", 0, NULL);
	if (err)
		printf("Failed to mount /sys: %s\n", strerror(errno));
	
	err = read_cmdline(cmdline);
	if (err)
		goto die;
	
	kernel_param_parse(cmdline, params);
	
	printf("Detected root as '%s'\n", param_root.value);
	
	printf("Please wait while I initialize your devices...\n");
	
	printf("Starting udevd.\n");
	pid = fork();
	if (!pid) {
		err = execl("/sbin/udevd", "/sbin/udevd",
			"--daemon", (char *) NULL);
		if (err)
			printf("Failed to start udevd: %s\n", strerror(errno));
		return 1;
	} else if (pid == -1) {
		printf("Could not fork: %s\n", strerror(errno));
		goto die;
	}
	err = waitpid(pid, &status, 0);
	if (err == -1) {
		printf("Failed to wait for child: %s\n", strerror(errno));
		goto die;
	}
	printf("udevd status %d. Triggering events.\n", WEXITSTATUS(status));
	pid = fork();
	if (!pid) {
		err = execl("/sbin/udevadm", "/sbin/udevadm",
			"trigger", (char *) NULL);
		if (err)
			printf("Failed to start udevadm: %s\n", strerror(errno));
		return 1;
	} else if (pid == -1) {
		printf("Could not fork: %s\n", strerror(errno));
		goto die;
	}
	err = waitpid(pid, &status, 0);
	if (err == -1) {
		printf("Failed to wait for child: %s\n", strerror(errno));
		goto die;
	}
	printf("udevadm status %d. Waiting for the events to propagate.\n",
		WEXITSTATUS(status));
	pid = fork();
	if (!pid) {
		err = execl("/sbin/udevadm", "/sbin/udevadm",
			"settle", (char *) NULL);
		if (err)
			printf("Failed to start udevadm: %s\n", strerror(errno));
		return 1;
	} else if (pid == -1) {
		printf("Could not fork: %s\n", strerror(errno));
		goto die;
	}
	err = waitpid(pid, &status, 0);
	if (err == -1) {
		printf("Failed to wait for child: %s\n", strerror(errno));
		goto die;
	}
	printf("udevadm status %d. You should now have all your device nodes.\n",
		WEXITSTATUS(status));
	
die:
	printf("Your kernel is about to panic, have a good day.\n");
	execl("/bin/ls", "/bin/ls", "-l",
		param_ls.value ?: "/dev", (char *) NULL);
	return 1;
}
