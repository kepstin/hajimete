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
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int udev_init(void)
{
	int err;
	
	printf("Please wait while I initialize your devices...\n");
	
	printf("Starting udevd.\n");
	err = exec_run_wait("/sbin/start-stop-daemon", "/sbin/start-stop-daemon", "--start", "--exec", "/sbin/udevd", "--", "--daemon", (char *) NULL);
	if (err) {
		printf("Failed to start udevd\n");
		return -1;
	}

	printf("Triggering events.\n");
	err = exec_run_wait("/sbin/udevadm", "/sbin/udevadm", "--debug", "trigger", (char *) NULL);
	if (err) {
		printf("Failed to trigger udev events\n");
		return -1;
	}
	printf("Waiting for the events to propagate.\n");
	exec_run_wait("/sbin/udevadm", "/sbin/udevadm", "--debug", "settle", "--timeout=0", (char *) NULL);
	
	printf("You should now have all your device nodes.\n");
	return 0;
}

int udev_kill(void)
{
	int err;
	
	printf("Stopping udevd.\n");
	err = exec_run_wait("/sbin/start-stop-daemon", "/sbin/start-stop-daemon", "--stop", "--exec", "/sbin/udevd", (char *) NULL);
	if (err) {
		printf("Failed to stop udevd\n");
		return -1;
	}
	return 0;
}
