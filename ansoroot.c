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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <unistd.h>

#include "utlist.h"

struct vm_umount
{
	struct vm_umount*       next;
	char                    mountpoint[PATH_MAX];
};

void anschroot_umount_paths_outroot(const char* const vm_root_path)
{
	// Open the list of mountpoints
	FILE* fh = NULL;
	if (! (fh = fopen("/proc/self/mounts", "r")))
		return;

	/* To hold a list of mountpoints.
	 *
	 * We could unmount them as we discover them, but that would prevent
	 * us unmounting /proc due to the file we just opened above.
	 */
	struct vm_umount* vm_umounts = NULL;

	// Take the string length of the vm root path
	size_t vm_root_path_len = strlen(vm_root_path);

	/* Parse each entry for ones that don't share the same prefix, add
	 * them to the list of filesystems to unmount
	 */
	char* line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fh) != -1)
	{
		char mountpoint[PATH_MAX];
		if (sscanf(line, "%*s %s %*s %*s %*u %*u", mountpoint) < 1)
			continue;

		if (strlen(mountpoint) >= vm_root_path_len)
			if (memcmp(mountpoint, vm_root_path, vm_root_path_len) == 0)
				continue;

		struct vm_umount* vm_umounts_new = calloc(1, sizeof(*vm_umounts_new));
		if (! vm_umounts_new)
			continue;

		snprintf(vm_umounts_new->mountpoint, PATH_MAX, "%s", mountpoint);
		LL_PREPEND(vm_umounts, vm_umounts_new);
		vm_umounts_new = NULL;
	}
	fclose(fh);
	free(line);

	// Unmount the filesystems in the list
	// They were added in reverse order (e.g. you can't unmount /foo without first unmounting /foo/bar)
	struct vm_umount* vm_umounts_cur = NULL;
	struct vm_umount* vm_umounts_tmp = NULL;
	LL_FOREACH_SAFE(vm_umounts, vm_umounts_cur, vm_umounts_tmp)
	{
		/* We don't care if the unmount fails because it could be for any number of good reasons.
		 *
		 * This includes the fact that the VM root could be e.g. /foo/bar/baz, which is located on
		 * a filesystem mounted at /foo/bar, while /foo is also a mountpoint. We won't unmount
		 * /foo/bar because we need it, but we would try to unmount /foo, which will obviously fail
		 * with -EBUSY while bar/ is mounted under it.
		 */
		(void) umount(vm_umounts_cur->mountpoint);

		LL_DELETE(vm_umounts, vm_umounts_cur);
		free(vm_umounts_cur);
	}
}
