/*
 * Copyright (c) 2014 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 * Verify that,
 *  1) mknod(2) returns -1 and sets errno to EROFS if pathname refers to
 *     a file on a read-only file system.
 *  2) mknod(2) returns -1 and sets errno to ELOOP if Too many symbolic
 *     links were encountered in resolving pathname.
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <error.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/mount.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"
#include "lapi/fcntl.h"
#include "mknodat.h"

static void setup(void);
static void cleanup(void);
static void help(void);

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define MNT_POINT	"mntpoint"

#define FIFOMODE	(S_IFIFO | S_IRUSR | S_IRGRP | S_IROTH)
#define FREGMODE	(S_IFREG | S_IRUSR | S_IRGRP | S_IROTH)
#define SOCKMODE	(S_IFSOCK | S_IRUSR | S_IRGRP | S_IROTH)

static char *fstype = "ext2";
static char *device;
static int mount_flag;

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", NULL, &device},
	{NULL, NULL, NULL}
};

static int dirfd;
static int curfd = AT_FDCWD;

#define ELOPFILE	"/test_eloop"
static char elooppathname[sizeof(ELOPFILE) * 43] = ".";

static struct test_case_t {
	int *dirfd;
	char *pathname;
	mode_t mode;
	int exp_errno;
} test_cases[] = {
	{ &curfd, "tnode1", FIFOMODE, 0 },
	{ &curfd, "tnode2", FREGMODE, 0 },
	{ &curfd, "tnode3", SOCKMODE, 0 },
	{ &dirfd, "tnode4", FIFOMODE, EROFS },
	{ &dirfd, "tnode5", FREGMODE, EROFS },
	{ &dirfd, "tnode6", SOCKMODE, EROFS },
	{ &curfd, elooppathname, FIFOMODE, ELOOP },
	{ &curfd, elooppathname, FREGMODE, ELOOP },
	{ &curfd, elooppathname, SOCKMODE, ELOOP },
};

static void mknodat_verify(struct test_case_t *tc);

char *TCID = "mknodat";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static int exp_enos[] = { EROFS, ELOOP, 0 };

int main(int ac, char **av)
{
	int lc, i;
	const char *msg;

	msg = parse_opts(ac, av, options, help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	if (!device) {
		tst_brkm(TBROK, NULL, "you must specify the device "
			 "used for mounting with -D option");
	}

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			mknodat_verify(&test_cases[i]);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int i;

	if (tst_kvercmp(2, 6, 16) < 0) {
		tst_brkm(TCONF, NULL, "This test can only run on kernels "
			 "that are 2.6.16 and higher");
	}

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	TEST_EXP_ENOS(exp_enos);

	tst_mkfs(NULL, device, fstype, NULL);

	tst_tmpdir();

	TEST_PAUSE;

	/*
	 * mount a read-only file system for EROFS test
	 */
	SAFE_MKDIR(cleanup, MNT_POINT, DIR_MODE);
	if (mount(device, MNT_POINT, fstype, MS_RDONLY, NULL) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "mount device:%s failed", device);
	}
	mount_flag = 1;
	dirfd = SAFE_OPEN(cleanup, MNT_POINT, O_DIRECTORY);

	/*
	 * NOTE: the ELOOP test is written based on that the consecutive
	 * symlinks limits in kernel is hardwired to 40.
	 */
	SAFE_MKDIR(cleanup, "test_eloop", DIR_MODE);
	SAFE_SYMLINK(cleanup, "../test_eloop", "test_eloop/test_eloop");
	for (i = 0; i < 43; i++)
		strcat(elooppathname, ELOPFILE);
}

static void mknodat_verify(struct test_case_t *tc)
{
	int fd = *(tc->dirfd);
	char *pathname = tc->pathname;
	mode_t mode = tc->mode;

	TEST(mknodat(fd, pathname, mode, 0));

	if (TEST_ERRNO == tc->exp_errno) {
		tst_resm(TPASS | TTERRNO,
			 "mknodat() returned the expected value");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "mknodat() got unexpected return value; expected: "
			 "%d - %s", tc->exp_errno,
			 strerror(tc->exp_errno));
	}

	if (TEST_ERRNO == 0 &&
	    ltp_syscall(__NR_unlinkat, fd, pathname, 0) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup, "unlinkat(%d, %s) "
			 "failed.", fd, pathname);
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (dirfd > 0 && close(dirfd) < 0)
		tst_resm(TWARN | TERRNO, "close(%d) failed", dirfd);
	if (mount_flag && umount(MNT_POINT) < 0)
		tst_resm(TWARN | TERRNO, "umount device:%s failed", device);

	tst_rmdir();
}

static void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted. "
	       "Default ext2.\n");
	printf("-D device : device used for mounting.\n");
}