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

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/types.h>

#define VM_MOUNTS_COUNT 6

static const struct vm_mount {
	const char*     source;
	const char*     target;
	const char*     fstype;
	const char*     fsopts;
	unsigned long   mflags;
} vm_mounts[VM_MOUNTS_COUNT] = {
	{ "devpts", "/dev/pts", "devpts", "newinstance,ptmxmode=0666,mode=0600,gid=5", MS_NOSUID | MS_NOEXEC },
	{ "shm", "/dev/shm", "tmpfs", "size=256M,nr_inodes=16k,mode=1777", MS_NOSUID | MS_NOEXEC | MS_NODEV },
	{ "proc", "/proc", "proc", NULL, MS_NOSUID | MS_NOEXEC | MS_NODEV },
	{ "runfs", "/run", "tmpfs", "size=8M,nr_inodes=8k,mode=1775,gid=500", MS_NOSUID | MS_NOEXEC },
	{ "tmpfs", "/tmp", "tmpfs", "size=256M,nr_inodes=16k,mode=1777", MS_NOSUID },
	{ "tmpfs", "/var/tmp/portage", "tmpfs", "size=2G,nr_inodes=256k,mode=0755,uid=250,gid=250", MS_NOSUID },
};

int anschroot_mount_paths_inroot(const char* const vm_root_path)
{
	for (unsigned int i = 0; i < VM_MOUNTS_COUNT; i++)
	{
		char target[PATH_MAX];
		memset(target, 0x00, sizeof target);
		(void) snprintf(target, sizeof target, "%s%s", vm_root_path, vm_mounts[i].target);

		const char* source = vm_mounts[i].source;
		const char* fstype = vm_mounts[i].fstype;
		const char* fsopts = vm_mounts[i].fsopts;
		unsigned long mflags = vm_mounts[i].mflags;

		if (mount(source, target, fstype, mflags, fsopts) != 0)
			return -1;
	}

	return 0;
}
