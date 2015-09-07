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

#include <cap-ng.h>

int anschroot_drop_caps(void)
{
	// Get the current process capabilities
	capng_get_caps_process();

	// Clear all capability flags in all capability sets
	capng_clear(CAPNG_SELECT_BOTH);

	// Then,
	capng_updatev(

		// Add,
		CAPNG_ADD,

		// To these capability sets,
		CAPNG_BOUNDING_SET | CAPNG_EFFECTIVE | CAPNG_PERMITTED,

		// The following capabilities:

		//   Networking:
		CAP_NET_BIND_SERVICE,

		//   Filesystem:
		CAP_CHOWN, CAP_DAC_OVERRIDE, CAP_DAC_READ_SEARCH, CAP_FOWNER,
		CAP_FSETID,

		//   Processes:
		CAP_KILL, CAP_SETGID, CAP_SETUID, CAP_SETPCAP, CAP_IPC_LOCK,
		CAP_IPC_OWNER, CAP_LEASE, CAP_SYS_PTRACE, CAP_SYS_NICE,
		CAP_SYS_RESOURCE,

	-1);

	// Apply the new capability sets
	return capng_apply(CAPNG_SELECT_BOTH);
}
