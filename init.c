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
#include "udev.h"

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
static struct kernel_param param_ls = { "ls", "/dev" };
static struct kernel_param param_init = { "init", "/sbin/init" };

static struct kernel_param *params[] = {
	&param_root,
	&param_ls,
	&param_init,
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
	char root[128];
	
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
	
	err = udev_init();
	if (err) goto die;

	printf("Detected root as '%s'\n", param_root.value);
	
	strncpy(root, param_root.value, 128);
	root[127] = '\0';
	while (1) {
		printf("I'm now going to attempt to mount your root device.\n");
		pid = fork();
		if (!pid) {
			err = execl("/bin/mount", "/bin/mount", "-o", "ro", root, "/newroot", (char *) NULL);
			if (err)
				printf("Failed to start mount: %s\n", strerror(errno));
			exit(1);
		} else if (pid == -1) {
			printf("Could not fork: %s\n", strerror(errno));
			goto mount_fail;
		}
		err = waitpid(pid, &status, 0);
		if (err == -1) {
			printf("Failed to wait for child: %s\n", strerror(errno));
			goto mount_fail;
		}
		err = WEXITSTATUS(status);
		if (err) {
			printf("Failed to mount root (%d)\n", err);
			goto mount_fail;
		}
		break;
mount_fail:
		printf("Hmm. That didn't work.\n"
			"Try entering another root device, or 'shell' for a rescue shell\n"
			"root> ");
		fgets(root, 128, stdin);
		root[strlen(root) - 1] = '\0';
		if (strcmp(root, "shell") == 0)
			goto die;
	}
	
	printf("Looks good, switching to it.\n");

	err = chroot("/newroot");
	if (err) {
		printf("Failed to chroot: %s\n", strerror(errno));
		goto die;
	}

	udev_kill();

	err = chdir("/");
	if (err) {
		printf("Failed to chdir to /: %s\n", strerror(errno));
		goto die_hard;
	}
	printf("And now, running your real init. From now on, it's all you.\n");
	err = execv(param_init.value, argv);
	if (err) {
		printf("Failed to exec %s: %s", param_init.value, strerror(errno));
		goto die_hard;
	}
	
die:
	printf("An error has occured, but you may be able to recover.\n");
	pid = fork();
	if (!pid) {
		err = execl("/bin/bash", "/bin/bash", (char *) NULL);
		if (err)
			printf("Failed to start shell: %s\n", strerror(errno));
		return 1;
	} else if (pid == -1) {
		printf("Could not fork: %s\n", strerror(errno));
		goto die_hard;
	}
	err = waitpid(pid, &status, 0);
	if (err == -1) {
		printf("Failed to wait for child: %s\n", strerror(errno));
		goto die_hard;
	}

die_hard:
	printf("An unrecoverable error has occurred. Please contact your distribution support\n"
		"In your report, include as much of the above text as you can see.\n");
	sleep(5);
	return 1;
}
