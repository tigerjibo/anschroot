/*
 * anschroot - chroot on steroids
 *
 * Copyright (C) 2015   Aaron M D Jones   <aaronmdjones@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE     1
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

extern int  anschroot_drop_caps(void);
extern int  anschroot_mount_paths_inroot(const char* const vm_root_path);
extern void anschroot_umount_paths_outroot(const char* const vm_root_path);

int main(int argc, char* argv[])
{

	// Check arguments were given
	if (argc < 3)
	{
		(void) fprintf(stderr, "Usage: %s <directory> <executable>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// Remove a trailing forward slash from the directory argument (if present)
	size_t argv_1_len = strlen(argv[1]);
	while (argv_1_len && argv[1][--argv_1_len] == '/')
		argv[1][argv_1_len] = '\0';



	/* Put the process into a new set of all namespaces (except user & net)
	 *
	 * Note that CLONE_NEWPID does not put the calling process (this) into a new PID
	 * namespace, as that would break a lot of libraries and programs that expect
	 * getpid(2) to always return the same value over the lifetime of their execution,
	 * but it does mean the next child we create (fork(2)) will be PID 1 in the new
	 * PID namespace, acting as its init. For more information see pid_namespaces(7).
	 */
	if (unshare(CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS) != 0)
	{
		(void) fprintf(stderr, "nschroot[parent]: unshare(2): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Fork a child
	pid_t pid = fork();
	if (pid < 0)
	{
		(void) fprintf(stderr, "nschroot[parent]: fork(2): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Parent
	if (pid > 0)
	{
		// Wait for child to terminate
		int status = 0;
		if (waitpid(pid, &status, 0) == -1)
		{
			(void) fprintf(stderr, "nschroot[parent]: waitpid(3): %s\n", strerror(errno));
			return EXIT_FAILURE;
		}

		// If the child exited normally, exit with the same return code
		if (WIFEXITED(status))
			return WEXITSTATUS(status);

		// If the child died due to a signal, kill ourselves with the same signal
		if (WIFSIGNALED(status))
			(void) kill(getpid(), WTERMSIG(status));

		return EXIT_FAILURE;
	}

	/**********************************/
	/* Child continues execution here */
	/**********************************/

	// Unmount as many unnecessary filesystems as we can (avoid polluting /proc/mounts in the child)
	(void) anschroot_umount_paths_outroot(argv[1]);

	// Mount filesystems that the child will need
	if (anschroot_mount_paths_inroot(argv[1]) != 0)
	{
		(void) fprintf(stderr, "nschroot[child]: mount(2): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Change root filesystem
	if (chroot(argv[1]) != 0)
	{
		(void) fprintf(stderr, "nschroot[child]: chroot(2): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	if (chdir("/") != 0)
	{
		(void) fprintf(stderr, "nschroot[child]: chdir(2): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Drop privilege
	if (anschroot_drop_caps() != 0)
	{
		(void) fprintf(stderr, "nschroot[child]: capng_apply(3): %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	// Execute a shell
	if (execv(argv[2], (char* const []) { argv[2], NULL }) != 0)
		(void) fprintf(stderr, "nschroot[child]: execv(3): %s\n", strerror(errno));

	return EXIT_FAILURE;
}
