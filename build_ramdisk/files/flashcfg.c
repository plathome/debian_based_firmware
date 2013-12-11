/*	$ssdlinux: flashcfg.c,v 1.37 2013/12/11 07:13:19 yamagata Exp $	*/
/*
 * Copyright (c) 2012-2013 Plat'Home CO., LTD.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Plat'Home CO., LTD. nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#undef DEBUG

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <linux/compiler.h>
#include <mtd/mtd-user.h>
#if defined(CONFIG_OBSA6)
#include <mtd/mtd-abi.h>
#endif
#if defined(CONFIG_OBSA6)
#include <zlib.h>
#else
#include <linux/zlib.h>
#endif
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>

extern char **environ;
/* some variables used in getopt (3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

#if defined(CONFIG_OBSA6)
#define ENV_AREA	0x20000		/* u-boot environment area size */
#define ENV_SIZE	0x10000		/* size of u-boot environment size */
#else
#define ENV_AREA	0x40000		/* u-boot environment area size */
#define ENV_SIZE	0x20000		/* size of u-boot environment size */
#endif
#define VERIFY
#define NORMAL_END	0
#define ERROR_END	1
#define READWRITE	"0xC00"
#define READONLY	"0x800"

#define MTD_UBOOT	0
#define MTD_UBOOTENV	1
#define MTD_TSTPRG	2
#define MTD_USRCONF	3
#define MTD_KERNEL	4
#define MTD_USRDATA	5
#define MTD_JAVA	6

#define SWAP_ENDIAN(a) {				\
	unsigned long b = a;				\
	((char*)&b)[0] = ((char*)&a)[3];	\
	((char*)&b)[1] = ((char*)&a)[2];	\
	((char*)&b)[2] = ((char*)&a)[1];	\
	((char*)&b)[3] = ((char*)&a)[0];	\
	a = b;								\
}
#ifdef CONFIG_OBSA6
#define TMPFS "/tmp/.flashcfg"
#define FNAME "flashcfg.XXXX"
#endif

#ifdef DEBIAN
#define EXTRACTPATH "/.rw"
#else
#define EXTRACTPATH "/"
#endif

char** set_mtdtype(void);

const char *mtdname_old[] = {
	"/dev/mtd/0",
	"/dev/mtd/1",
	"/dev/mtd/2",
	"/dev/mtd/3",
	"/dev/mtd/4",
	"/dev/mtd/5",
	"/dev/mtd/6"
};

const char *mtdname_new[] = {
	"/dev/mtd0",
	"/dev/mtd1",
	"/dev/mtd2",
	"/dev/mtd3",
	"/dev/mtd4",
	"/dev/mtd5",
	"/dev/mtd6"
};

static char **mtdname;

char * set_tarpath(void);
const char* tarssd = "/usr/bin/tar";
const char* tarother = "/bin/tar";
static char* tarpath;

/*
 *      /proc/mtd
 *      dev:     size       erasesize     name
 *      mtd0: MONITOR_SIZE             "OpenBlocks A series Uboot"
 *      mtd1: MPARAM_SIZE              "OpenBlocks A series Uboot Configuration Data"
 *      mtd2: TSTPRG_SIZE              "OpenBlocks A series Diag program Image"
 *      mtd3: PARAM_SIZE               "OpenBlocks A series Configuration Data"
 *      mtd4: KERNEL_SIZE   SECT_SIZE  "OpenBlocks A series Kernel Image"
 *      mtd5: USER_SIZE	    SECT_SIZE  "OpenBlocks A series User Flash"
 *      mtd6: JAVA_SIZE	    SECT_SIZE  "OpenBlocks A series Java Image"
 */

#define VERSION "$Revision: 1.37 $"

int SECT_SIZE = 0;
int MONITOR_SIZE = 0;
int PARAM_SIZE = 0;
int MPARAM_SIZE = 0;
int USER_SIZE = 0;
int KERNEL_SIZE = 0;
int TSTPRG_SIZE = 0;
int JAVA_SIZE = 0;
#define MAGIC 0x27051956

#define STATUS_COL	50

#define SYSENV_RECORD_IS_FREE  0xff
#define SYSENV_RECORD_IN_USE   0x42

typedef struct boot_block_t {
	unsigned long	magic;
	unsigned long	crc;
	unsigned long	time;
	unsigned long	size;
	unsigned long	loadp;
	unsigned long	entryp;
	unsigned long	imgcrc;
	unsigned char	os;
	unsigned char	arch;
	unsigned char	image;
	unsigned char	compression;
	unsigned char	name[32];
} boot_block_t;

boot_block_t * pboot_block;

unsigned short *membase = NULL;
unsigned char *spare = NULL;
FILE *fp_log = NULL;

int flash_set_bootdev(int, char *);
int flash_set_pmlevel(int, char *);
int flash_prog_kern(char *);
int flash_save_param(int i, char *);
int flash_delete_param(int i);
int flash_extract_param(int i);
int read_proc_mtd(void);
int flash_restore_ubootenv(char*);
void flash_open_log(void);
void flash_close_log(void);
void flash_write_log(char*);
int flash_prog_java(char *);
int flash_extract_java(int);
int mtd_protect(int, int);
int mount_mtddev(int);
int get_mtdfile(int, char*);
int umount_mtddev(void);
int read_core_area(void);
int test_core_area(void);
int read_magic(char *);
void dump(unsigned char *, int);
#define LOGNAME		"flashcfg.log"
#define LOGPATH_CF	"/usr/pkg/var/log/"
#define LOGPATH_RAM	"/var/log/"

void
usage()
{
	fprintf(stderr, "Flash Memory Configuration Tool (%s)\n\n", VERSION);
	fprintf(stderr, "usage: flashcfg                 Show this.\n");
#if 0
	fprintf(stderr, "       flashcfg -b              Show current boot setting\n");
#endif
	fprintf(stderr, "       flashcfg -c rootdev      Change root file system [initrd|ext|sda-sdd[1-8]]\n");
	fprintf(stderr, "                                initrd       - Boot from FlashROM, rootfs on Ramdisk\n");
	fprintf(stderr, "                                sda-sdd[1-8] - Boot from FlashROM, rootfs on SATA\n");
	fprintf(stderr, "                                ext          - Boot from SATA,     rootfs on SATA\n");
	fprintf(stderr, "       flashcfg -f boot_image   Load boot image to flash\n");
#ifndef DEBIAN
	fprintf(stderr, "       flashcfg -s list_file    Save files to Parameter area\n");
	fprintf(stderr, "       flashcfg -S list_file    Save files to User area\n");
	fprintf(stderr, "       flashcfg -x              Restore files from Parameter area\n");
	fprintf(stderr, "       flashcfg -X              Restore files from User area\n");
#else
	fprintf(stderr, "       flashcfg -s conf_archive Save files to Parameter area\n");
	fprintf(stderr, "       flashcfg -S user_archive Save files to User area\n");
#endif
	fprintf(stderr, "       flashcfg -d              Delete saved files from Parameter area\n");
	fprintf(stderr, "       flashcfg -D              Delete saved files from User area\n");
#ifndef DEBIAN
	fprintf(stderr, "       flashcfg -E env_file     Save params to U-Boot Environment area\n");
	fprintf(stderr, "       flashcfg -L env_file     Load params from U-Boot Environment area\n");
//	fprintf(stderr, "       flashcfg -R              Remove saved params from U-Boot Environment area\n");
//	fprintf(stderr, "       flashcfg -r              Delete firmware area\n");
#endif
#if defined(CONFIG_OBSAX3) || defined(CONFIG_OBSA7)
	fprintf(stderr, "       flashcfg -j java_archive Save files from Java area\n");
	fprintf(stderr, "       flashcfg -J              Restore files from Java area\n");
#endif
	fprintf(stderr, "       flashcfg -t              Test read and write in coredump save area\n");
	fprintf(stderr, "       flashcfg -T              Read message in coredump save area\n");
#if defined(CONFIG_OBSAX3)
	fprintf(stderr, "       flashcfg -p (now|wfi|idle|snooze) Print or set Power Management Level\n");
#endif
	fprintf(stderr, "       flashcfg -h              Show this.\n");
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int i, ret;
	char boot_dev[256];
	char buf[32];
	char *p;

	if (getuid()) {
		fprintf(stderr, "must be super user\n");
		return ERROR_END;
	}

	if((mtdname = set_mtdtype()) == NULL){
		fprintf(stderr, "mtd device not found!\n");
		return ERROR_END;
	}
	if((tarpath = set_tarpath()) == NULL){
		fprintf(stderr, "tar command not found!\n");
		return ERROR_END;
	}
#ifdef DEBUG
	printf("mtd0=%s\n", mtdname[0]);
	printf("tar=%s\n", tarpath);
#endif

	ret = NORMAL_END;
#ifdef DEBIAN
#if defined(CONFIG_OBSAX3)
	while ((i = getopt(argc, argv, "c:f:p:s:S:j:dDxXJrtT")) != -1) {
#else
	while ((i = getopt(argc, argv, "c:f:s:S:j:dDxXJr")) != -1) {
#endif
#else
#if defined(CONFIG_OBSAX3)
	while ((i = getopt(argc, argv, "c:f:p:s:S:E:L:j:xXdDhRrJtT")) != -1) {
#else
	while ((i = getopt(argc, argv, "c:f:s:S:E:L:j:xXdDhRrJ")) != -1) {
#endif
#endif
		switch (i) {
		case 'c':
			if (strcmp(optarg, "initrd") == 0) {
				strcpy(boot_dev, "ram");
			}
			else if (strncmp(optarg, "sd", 2) == 0){
				if (strlen(optarg) != 4)
					goto invalid_arg;
				if ((optarg[2] < 'a') ||
					(optarg[2] > 'd'))
					goto invalid_arg;
				if ((optarg[3] < '1') ||
					(optarg[3] > '8'))
					goto invalid_arg;
				strcpy(boot_dev, optarg);
				/* get disk label */
				sprintf(buf, "/sbin/e2label /dev/%s", boot_dev);
				if((fp = popen(buf, "r")) == NULL){
					fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, buf, strerror(errno));
					return ERROR_END;
				}
				fgets(buf, sizeof(buf)-1, fp);
				pclose(fp);
				if((p = strchr(buf, '\n')) != NULL) /* delete CR */
					*p = 0x0;
				if(strcmp(buf, "/") != 0){
					printf("ERROR: disk label is not '/' on %s\n", boot_dev);
					printf("exp: `e2label /dev/%s /`\n", boot_dev);
					return ERROR_END;
				}
			}
			else if (strcmp(optarg, "ext") == 0){
				buf[0] = 0x0;
				strcpy(boot_dev, optarg);
				/* get cf label */
				if((fp = popen("/sbin/blkid -l -t LABEL=/", "r")) == NULL){
					fprintf(stderr, "ERROR%d: popen %s\n", __LINE__, strerror(errno));
					return ERROR_END;
				}
				fgets(buf, sizeof(buf)-1, fp);
				pclose(fp);
				if(buf[0] == 0){
					printf("ERROR: disk label is not '/' on SATA\n");
					printf("exp: `e2label /dev/sda1 /`\n");
					return ERROR_END;
				}
			}
			else {
invalid_arg:
				fprintf(stderr, "invalid option %s\n", optarg);
				return ERROR_END;
			}
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(mtd_protect(MTD_UBOOTENV, 0) != 0){
				fprintf(stderr, "Fail to kernel image protect off\n");
				ret = ERROR_END;
			}
			if(flash_set_bootdev(i, boot_dev) < 0){
				fprintf(stderr, "Fail to Boot device change\n");
				mtd_protect(MTD_UBOOTENV, 1);
				return ERROR_END;
			}
			if(mtd_protect(MTD_UBOOTENV, 1) != 0){
				fprintf(stderr, "Fail to kernel image protect off\n");
				ret = ERROR_END;
			}
			fprintf(stderr, "Boot device change to %s\n", optarg);
			return NORMAL_END;

		case 'f':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(mtd_protect(MTD_KERNEL, 0) != 0){
				fprintf(stderr, "Fail to kernel image protect off\n");
				ret = ERROR_END;
			}
			if(flash_prog_kern(optarg) < 0){
				fprintf(stderr, "Fail to kernel image change\n");
				ret = ERROR_END;
			}
			mtd_protect(MTD_KERNEL, 1);
			if(ret == NORMAL_END)
				fprintf(stderr, "done\n");
			return ret;

#if defined(CONFIG_OBSAX3) || defined(CONFIG_OBSA7)
		case 'j':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(mtd_protect(MTD_JAVA, 0) != 0){
				fprintf(stderr, "Fail to java image protect off\n");
				ret = ERROR_END;
			}
			if(flash_prog_java(optarg) < 0){
				fprintf(stderr, "Fail to Java image change\n");
				return ERROR_END;
			}
			mtd_protect(MTD_JAVA, 1);
			fprintf(stderr, "done\n");
			return NORMAL_END;

		case 'J':
			if(flash_extract_java(i) < 0){
				fprintf(stderr, "Fail to java extract\n");
				return ERROR_END;
			}
			return NORMAL_END;
#endif

		case 's':
		case 'S':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(i == 's')
				ret = MTD_USRCONF;
			else
				ret = MTD_USRDATA;
			if(mtd_protect(ret, 0) != 0){
				fprintf(stderr, "Fail to environmet protect off\n");
				ret = ERROR_END;
			}
			if(flash_save_param(i, optarg) < 0){
				fprintf(stderr, "Fail to environment change\n");
				return ERROR_END;
			}
			mtd_protect(ret, 1);
			fprintf(stderr, "done\n");
			return NORMAL_END;

		case 'x':
		case 'X':
			if(flash_extract_param(i) < 0){
				fprintf(stderr, "Fail to environment extract\n");
				return ERROR_END;
			}
			return NORMAL_END;

		case 'd':
		case 'D':
			if(i == 'd')
				ret = MTD_USRCONF;
			else
				ret = MTD_USRDATA;
			if(mtd_protect(ret, 0) != 0){
				fprintf(stderr, "Fail to environmet protect off\n");
				ret = ERROR_END;
			}
			if(flash_delete_param(i) < 0){
				fprintf(stderr, "Fail to environment delete\n");
				return ERROR_END;
			}
			fprintf(stderr, "done\n");
			mtd_protect(ret, 1);
			return NORMAL_END;

#if 0
		case 'b':
#endif
		case 'E':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(mtd_protect(MTD_UBOOTENV, 0) != 0){
				fprintf(stderr, "Fail to params protect off\n");
				ret = ERROR_END;
			}
			if(flash_set_bootdev(i, optarg) < 0){
				fprintf(stderr, "Fail to Save params\n");
				return ERROR_END;
			}
			mtd_protect(MTD_UBOOTENV, 1);
			return NORMAL_END;

		case 'L':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(flash_restore_ubootenv(optarg) < 0){
				fprintf(stderr, "Fail to Restore params\n");
				return ERROR_END;
			}
			return NORMAL_END;
		case 'R':
		case 'r':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(mtd_protect(MTD_KERNEL, 0) != 0){
				fprintf(stderr, "Fail to kernel image protect off\n");
				ret = ERROR_END;
			}
			if(flash_delete_param(i) < 0){
				fprintf(stderr, "Fail to Delete params\n");
				return ERROR_END;
			}
			mtd_protect(MTD_KERNEL, 1);
			fprintf(stderr, "done\n");
			return NORMAL_END;
#if defined(CONFIG_OBSAX3)
		case 't':
			read_proc_mtd();
			return test_core_area();
			break;
		case 'T':
			read_proc_mtd();
			return read_core_area();
			break;
		case 'p':
			if(strcmp(optarg, "now") && strcmp(optarg, "wfi") && strcmp(optarg, "idle") && strcmp(optarg, "snooze")){
				fprintf(stderr, "invalid option %s\n", optarg);
				return ERROR_END;
			}
			if(mtd_protect(MTD_UBOOTENV, 0) != 0){
				fprintf(stderr, "Fail to kernel image protect off\n");
				ret = ERROR_END;
			}
			if(flash_set_pmlevel(i, optarg) < 0){
				fprintf(stderr, "Fail to Boot device change\n");
				mtd_protect(MTD_UBOOTENV, 1);
				return ERROR_END;
			}
			if(mtd_protect(MTD_UBOOTENV, 1) != 0){
				fprintf(stderr, "Fail to kernel image protect off\n");
				ret = ERROR_END;
			}
			return NORMAL_END;
			break;
#endif
		case 'h':
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	usage();
	return ERROR_END;
}

int
flash_prog_bootdev(char *env, int offset)
{
	int i, st, ofd, vfd, nread, retry;
	int ret=NORMAL_END;
#if defined(CONFIG_OBSA6)
	struct erase_info_user64 erase64;
	erase_info_t erase;
	int rts;
#else
	erase_info_t erase;
#endif
	int env_size = ENV_SIZE;
	unsigned short *verify = NULL, *_verify=NULL;

	if ((ofd = open(mtdname[MTD_UBOOTENV], O_RDWR)) < 0) {
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, mtdname[MTD_UBOOTENV], strerror(errno));
		return -1;
	}
	if(offset){
		if((st = lseek(ofd, offset, SEEK_SET)) != offset){
			close(ofd);
			fprintf(stderr, "ERROR%d: lseek %s\n", __LINE__, strerror(errno));
#ifdef DEBUG
			printf("seek=%x\n", st);
#endif
			return -1;
		}
	}

	if ((vfd = open(mtdname[MTD_UBOOTENV], O_RDONLY)) < 0) {
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, mtdname[MTD_UBOOTENV], strerror(errno));
		close(ofd);
		return -1;
	}
	if(offset){
		if((st = lseek(vfd, offset, SEEK_SET)) != offset){
			close(ofd);
			close(vfd);
			fprintf(stderr, "ERROR%d: lseek %s\n", __LINE__, strerror(errno));
#ifdef DEBUG
			printf("seek=%x\n", st);
#endif
			return -1;
		}
	}

	if ((verify = malloc(SECT_SIZE)) == NULL) {
		fprintf(stderr, "ERROR%d: malloc %s\n", __LINE__, strerror(errno));
		close(ofd);
		close(vfd);
		return -1;
	}
	_verify = verify;	/* save pointer */

	erase.start = offset;
	erase.length = SECT_SIZE;
#if defined(CONFIG_OBSA6)
	erase64.start = 0;
	erase64.length = SECT_SIZE;
#endif
	i=0;
	while (env_size > 0) {
		if(env_size < SECT_SIZE)
			nread = env_size;
		else if(env_size >= SECT_SIZE)
			nread = SECT_SIZE;

		retry = 2;
RETRY:
#if defined(CONFIG_OBSA6)
		rts = ioctl(ofd, MEMGETBADBLOCK, &erase64.start);
		if(rts > 0){
			erase.start += SECT_SIZE;
			erase64.start += SECT_SIZE;
			lseek(ofd, SECT_SIZE, SEEK_CUR);
			lseek(vfd, SECT_SIZE, SEEK_CUR);
			fprintf(stderr, "%cS",0x08);
			continue;
		}
		else if(rts < 0){
//			sprintf(buf, "ERROR%d: %s(%lx)\n", __LINE__, strerror(errno), total);
//			flash_write_log(buf);
//			ret = -1;
			break;
		}
#endif
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
			fprintf(stderr, "ERROR%d: Erase failure %s\n", __LINE__, strerror(errno));
			ret = -1;
			break;
		}
#if defined(CONFIG_OBSAX3)
		if (write(ofd, env, nread) < 0) {
#else	// CONFIG_OBSA6
		if (write(ofd, env, SECT_SIZE) < 0) {
#endif
			fprintf(stderr, "ERROR%d: mtd %s\n", __LINE__, strerror(errno));
			ret = -1;
			break;
		}
		if(read(vfd, verify, nread) != nread){
			fprintf(stderr, "ERROR%d: verify %s\n", __LINE__, strerror(errno));
			ret = -1;
			break;
		}
#if 0	/* Force verify error */
if(i && !(i % 10) && retry == 2){
	verify[0] *= 2;
}
#endif
		if(memcmp(env, verify, nread) != 0){
			fprintf(stderr, "%c!",0x08);
			if(--retry){
				if(lseek(ofd, -nread, SEEK_CUR) < 0){
					fprintf(stderr, "ERROR%d: lseek %s\n", __LINE__, strerror(errno));
					ret = -1;
					break;
				}
				if(lseek(vfd, -nread, SEEK_CUR) < 0){
					fprintf(stderr, "ERROR%d: lseek %s\n", __LINE__, strerror(errno));
					ret = -1;
					break;
				}
				goto RETRY;
			}
			else{
				fprintf(stderr, " STOP.\nERROR: flash memory verify error\n");
				ret = -1;
				break;
			}
		}
		i++;
		erase.start += nread;
		env += SECT_SIZE;
		verify += SECT_SIZE;
#if defined(CONFIG_OBSA6)
		erase64.start += nread;
#endif
		env_size -= nread;
		if(i>=STATUS_COL) {
			fprintf(stderr, "\n");
			i=0;
		}
	}

	free(_verify);
	close(ofd);
	close(vfd);
	return ret;
}


/*
	u-boot environment area 0xfff60000 - 0xfff9ffff
	offset	0x0     - 0x1FFFF	now environment area	    use 0x4000 of 0x20000
		0x20000 - 0x3FFFF	previous environmet area    use 0x4000 of 0x20000
*/
int
flash_set_bootdev(int target, char *arg)
{
#define OFFSET		5
	FILE *fp;
	char env_area1[ENV_SIZE];		// u-boot environment area1
	char env_area2[ENV_SIZE];		// u-boot environment area2
	char *p_prev, *p_now, *wk_prev, *wk_now;
	unsigned long crc1, crc2;
	char buf[512];
	char *p;

	memset(env_area1, 0x0, ENV_SIZE);
	memset(env_area2, 0x0, ENV_SIZE);

	read_proc_mtd();

	if((fp = fopen(mtdname[MTD_UBOOTENV], "r")) == NULL){
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, mtdname[MTD_UBOOTENV], strerror(errno));
		return -1;
	}
	/* read area1 */
	if(fread(env_area1, ENV_SIZE, 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	if(fseek(fp, ENV_AREA >> 1, SEEK_SET) != 0){
		fprintf(stderr, "ERROR%d: fseek %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	/* read area2 */
	if(fread(env_area2, ENV_SIZE, 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	fclose(fp);

#if 0
dump(env_area1, ENV_SIZE);
printf("\n");
printf("\n");
dump(env_area2, ENV_SIZE);
#endif

	crc1 = crc32(0, (unsigned char*)&env_area1[OFFSET], ENV_SIZE-OFFSET);
	crc2 = crc32(0, (unsigned char*)&env_area2[OFFSET], ENV_SIZE-OFFSET);

	if(!crc1 && !crc2){			/* Invalid both */
		fprintf(stderr, "ERROR%d: environment area is empty\n", __LINE__);
		return -1;
	}
	else if(crc1 && !crc2){		/* Invalid area2 */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(!crc1 && crc2){		/* Invalid area1 */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
#if defined(CONFIG_OBSA6)
	else if(env_area1[4] == env_area2[4]){
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] > env_area2[4]){	/* area1 is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] < env_area2[4]){	/* area2 is active */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
#else
	else if(env_area1[4] == 1 && env_area2[4] == 1){	/* both is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 1 && env_area2[4] == 0){	/* area1 is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0 && env_area2[4] == 1){	/* area2 is active */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0 && env_area2[4] == 0){	/* both is obsolete */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
#endif
	else if(env_area1[4] == 0xff && env_area2[4] != 0xff){	/* area1 is empty */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] != 0xff && env_area2[4] == 0xff){	/* area2 is empty */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else{	/* other case */
		fprintf(stderr, "ERROR%d: environment area is empty\n", __LINE__);
		return -1;
	}

	wk_prev = p_prev;
	wk_now = p_now;
	if(target == 'c'){	/* change boot device */
		/* previous environment -> now environment */
		while(wk_prev < p_prev + (ENV_SIZE-OFFSET)){
			if((p=strstr(wk_prev, "_std_script=")) && p == wk_prev){	/* found "_std_script=" */
				sprintf(buf, "_std_script=%s",
					(strcmp("ext", arg) ? "_fl_boot" : "_sata_boot"));
				if(wk_now - p_now > strlen(buf)){
					strcpy(wk_now, buf);
					wk_now += strlen(buf) + 1;
				}
				else{
					fprintf(stderr, "ERROR: bootargs string is too long\n");
					return -1;
				}
			}
			else if((p=strstr(wk_prev, "root=")) && p == wk_prev){		/* found "rootdev=" */
				if(strcmp("ram", arg) == 0)
					strcpy(buf, "root=/dev/ram");
				else if(strncmp("sd", arg, 2) == 0)
					strcpy(buf, "root=LABEL=/");
				else	/* arg = "ext" */
					strcpy(buf, wk_prev);

				if(wk_now - p_now > strlen(buf)){
					strcpy(wk_now, buf);
					wk_now += strlen(buf) + 1;
				}
				else{
					fprintf(stderr, "ERROR: bootargs string is too long\n");
					return -1;
				}
			}
#if 0
			else if((p=strstr(wk_prev, "usbdev="))){		/* found "usbdev=" */
				if(strcmp("cf", arg) == 0)
					strcpy(buf, "usbdev=usb 0:1");
				else if(strncmp("sd", arg, 2) == 0)
					sprintf(buf, "usbdev=usb %d:%d", arg[2] - 'a', arg[3] - '0');
				else
					strcpy(buf, wk_prev);

				if(wk_now - p_now > strlen(buf)){
					strcpy(wk_now, buf);
					wk_now += strlen(buf) + 1;
				}
				else{
					fprintf(stderr, "ERROR: bootargs string is too long\n");
					return -1;
				}
			}
#endif
			else{
				/* no change */
				strcpy(wk_now, wk_prev);
				wk_now += strlen(wk_prev) + 1;
			}
			wk_prev += strlen(wk_prev) + 1;	/* +1 = '\0' */
		}
	}
	else if(target == 'E'){	/* save u-boot env to flash rom */
		wk_now += OFFSET;	/* skip header area */
		if((fp = fopen(arg, "r")) == NULL){
			fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, arg, strerror(errno));
			return -1;
		}
		while(fgets(buf, sizeof(buf)-1, fp) != NULL){
			if((p = strchr(buf, '\r')) != NULL)
				*p = 0x0;
			else if((p = strchr(buf, '\n')) != NULL)
				*p = 0x0;
			memcpy(wk_now, buf, strlen(buf)+1);
			wk_now += strlen(buf)+1;
		}
		fclose(fp);
	}
#if 0
	else if(target == 'b'){	/* print current boot setting */
		char std_script[32], rootdev[32];

		std_script[0] = 0x0; rootdev[0] = 0x0;
		while(wk_prev < p_prev + (ENV_SIZE-OFFSET)){
			char *p;

			if((p=strstr(wk_prev, "_std_script="))){		/* found "_std_script=" */
				memcpy(std_script, p, sizeof(std_script));
			}
			else if((p=strstr(wk_prev, "root="))){		/* found "rootdev=" */
				memcpy(rootdev, strchr(p, '=') + 1, sizeof(rootdev));
			}
			/* found both string */
			if(std_script[0] && rootdev[0])
				break;
			wk_prev += strlen(wk_prev) + 1;	/* +1 = '\0' */
		}
		if(!std_script[0] || !rootdev[0])
			printf("unknown current boot device.\n");
		else{
			if(strstr(std_script, "_fl_boot")){
				if(strstr(rootdev, "/dev/ram"))
					printf("current boot device: initrd\n");
				else if(strstr(rootdev, "LABEL=/")){
					/* get disk label */
					if((fp = popen("/sbin/blkid -l -t LABEL=/", "r")) == NULL){
						fprintf(stderr, "ERROR%d: popen %s\n", __LINE__, strerror(errno));
						printf("unknown current boot device.\n");
						return 0;
					}
					fgets(buf, sizeof(buf)-1, fp);
					pclose(fp);
					/* get device name in /sbin/blkid output */
					if(buf[0] == 0)
						printf("LABEL=/ is not found\n");
					else{
						if((p = strchr(buf, ':')) == NULL){
							printf("unknown current boot device.\n");
							return 0;
						}
						*p = 0x0;
						printf("current boot device: %s\n", &buf[5]);
					}
				}
			}
			else if(strstr(std_script, "_sata_boot")){
				printf("current boot device: sata\n");
			}
		}
		return 0;
	}
#endif
	crc1 = crc32(0, (unsigned char*)&p_now[OFFSET], ENV_SIZE-OFFSET);
	memcpy(p_now, &crc1, sizeof(crc1));

#if !defined(CONFIG_OBSA6)
	/* set flag */
	p_now[4] = 0x1;		/* active */
	p_prev[4] = 0x0;	/* obsolete */
#endif
	/* save 0xfff60000 */
	fprintf(stderr, "Change 1st U-Boot environment area\n");
	if(flash_prog_bootdev(env_area1, 0) < 0){
		fprintf(stderr, "ERROR: flash memory write error in area1\n");
		return -1;
	}
	/* save 0xfff80000 */
	fprintf(stderr, "Change 2nd U-Boot environment area\n");
	if(flash_prog_bootdev(env_area2, ENV_AREA >> 1) < 0){
		fprintf(stderr, "ERROR: flash memory write error in area2)\n");
		return -1;
	}
#ifdef DEBUG
	if((fp = fopen("/tmp/now.env", "w")) != NULL){
		fwrite(p_now, ENV_SIZE, 1, fp);
		fclose(fp);
	}
	if((fp = fopen("/tmp/prev.env", "w")) != NULL){
		fwrite(p_prev, ENV_SIZE, 1, fp);
		fclose(fp);
	}
#endif
	return NORMAL_END;
}

int
flash_prog_kern(char *kern)
{
	int i, ifd, ofd, vfd, nread, retry;
#if defined(CONFIG_OBSA6)
	struct erase_info_user64 erase64;
	erase_info_t erase;
	int rts=0;
#else
	erase_info_t erase;
#endif
	struct stat sb;
	int sector_len;
	unsigned short *verify = NULL;
	unsigned long	crc, magic;
	unsigned long	total=0;
	char buf[512];
	
	int ret=NORMAL_END;

	flash_open_log();
	read_proc_mtd();
	if ((ifd = open(kern, O_RDONLY)) < 0) {
		sprintf(buf, "ERROR%d: %s open error\n", __LINE__, kern);
		flash_write_log(buf);
		ret = -1;
		goto err_exit;
	}

	if (fstat(ifd, &sb) < 0) {
		sprintf(buf, "ERROR%d: %s is not exist\n", __LINE__, kern);
		flash_write_log(buf);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

#if 0
sb.st_size = KERNEL_SIZE + 1;
#endif
	if (sb.st_size > KERNEL_SIZE) {
		sprintf(buf, "ERROR%d: file size exceeds MAX kernel SIZE\n", __LINE__);
		flash_write_log(buf);
		close(ifd);
		ret = -1;
		goto err_exit;
	}
	if ((membase = malloc(sb.st_size)) == NULL) {
		sprintf(buf, "ERROR%d: memory alloc error(size=%ld)\n", __LINE__, sb.st_size);
		flash_write_log(buf);
		ret = -1;
		goto err_exit;
	}
	nread = read(ifd, membase, sb.st_size);
#if 0
	nread--;
#endif
	if (nread != sb.st_size) {
		sprintf(buf, "ERROR%d: kernel image read error\n", __LINE__);
		flash_write_log(buf);
		free(membase);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

	pboot_block = (boot_block_t *)membase;
	crc = crc32(0, (unsigned char*)pboot_block + sizeof(boot_block_t),
					sb.st_size - sizeof(boot_block_t));
#ifdef __LITTLE_ENDIAN
	SWAP_ENDIAN(crc);
#endif
	if(crc != pboot_block->imgcrc){
		sprintf(buf, "ERROR%d: kernel image is CRC error(%08lx != %08lx)\n", __LINE__, crc, pboot_block->imgcrc);
		flash_write_log(buf);
		free(membase);
		close(ifd);
		ret = -1;
		goto err_exit;
	}
	free(membase);
	lseek(ifd, 0, SEEK_SET);

	if ((membase = malloc(SECT_SIZE)) == NULL) {
		sprintf(buf, "ERROR%d: memory alloc error(size=%d)\n", __LINE__, SECT_SIZE);
		flash_write_log(buf);
		ret = -1;
		goto err_exit;
	}
	nread = read(ifd, membase, SECT_SIZE);

#if 0
nread = SECT_SIZE - 1;
#endif
	if (nread < SECT_SIZE) {
		sprintf(buf, "ERROR%d: file isn't kernel image\n", __LINE__);
		flash_write_log(buf);
		free(membase);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

	pboot_block = (boot_block_t *)membase;
	magic = pboot_block->magic;
	
#ifdef __LITTLE_ENDIAN
	SWAP_ENDIAN(magic);
#endif
	if (magic != MAGIC) {
		sprintf(buf, "ERROR%d: file isn't kernel image(%08lx)\n", __LINE__, magic);
		flash_write_log(buf);
		free(membase);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

	if ((ofd = open(mtdname[MTD_KERNEL], O_RDWR)) < 0) {
		sprintf(buf, "ERROR%d: %s open error\n", __LINE__, mtdname[MTD_KERNEL]);
		flash_write_log(buf);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

#ifdef VERIFY
	if ((vfd = open(mtdname[MTD_KERNEL], O_RDONLY)) < 0) {
		sprintf(buf, "ERROR%d: %s open error\n", __LINE__, mtdname[MTD_KERNEL]);
		flash_write_log(buf);
		close(ifd);
		close(ofd);
		ret = -1;
		goto err_exit;
	}

	if ((verify = malloc(SECT_SIZE)) == NULL) {
		sprintf(buf, "ERROR%d: memory alloc error(size=%d)\n", __LINE__, SECT_SIZE);
		flash_write_log(buf);
		close(ifd);
		close(ofd);
		close(vfd);
		ret = -1;
		goto err_exit;
	}
#endif

	erase.start = 0;
	sector_len = SECT_SIZE;
	erase.length = sector_len;
#if defined(CONFIG_OBSA6)
	erase64.start = 0;
	erase64.length = sector_len;
#endif
	i=0;
	lseek(ifd, 0, SEEK_SET);
	fprintf(stderr, "Load boot image to FlashROM\n");
	while ((nread = read(ifd, membase, sector_len)) > 0) {
		retry = 2;
RETRY:
		fprintf(stderr, ".");
#if defined(CONFIG_OBSA6)
		rts = ioctl(ofd, MEMGETBADBLOCK, &erase64.start);
		if(rts > 0){
			erase.start += SECT_SIZE;
			erase64.start += SECT_SIZE;
			lseek(ofd, SECT_SIZE, SEEK_CUR);
			lseek(vfd, SECT_SIZE, SEEK_CUR);
			fprintf(stderr, "%cS",0x08);
			goto RETRY;
		}
		else if(rts < 0){
//			sprintf(buf, "ERROR%d: %s(%lx)\n", __LINE__, strerror(errno), total);
//			flash_write_log(buf);
//			ret = -1;
			break;
		}
#endif
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
#if defined(CONFIG_OBSA6)
			/* found Bad Block, Mark bad */
			if((rts = ioctl(ofd, MEMSETBADBLOCK, &erase64.start)) < 0){
				sprintf(buf, "ERROR%d: %s(%lx)\n", __LINE__, strerror(errno), total);
				flash_write_log(buf);
				ret = -1;
				break;
			}
			erase.start += nread;
			erase64.start += nread;
			goto RETRY;
#else
			sprintf(buf, "ERROR%d: erase error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
#endif
		}
		fprintf(stderr, "%c#",0x08);
#if defined(CONFIG_OBSAX3)
		if (write(ofd, membase, nread) < 0) {
#else	// CONFIG_OBSA6
		if (write(ofd, membase, SECT_SIZE) < 0) {
#endif
			sprintf(buf, "ERROR%d: write error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
		}

#ifdef VERIFY
		fprintf(stderr, "%c*",0x08);
		if(read(vfd, verify, nread) != nread){
			sprintf(buf, "ERROR%d: verify read error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
		}
#if 0	/* Force verify error */
if(i && !(i % 10) && retry == 2){
	verify[0] *= 2;
}
#endif
		if(memcmp(membase, verify, nread) != 0){
			fprintf(stderr, "%c!",0x08);
			if(--retry){
				if(lseek(ofd, -nread, SEEK_CUR) < 0){
					sprintf(buf, "ERROR%d: seek error(%lx)\n", __LINE__, total);
					flash_write_log(buf);
					ret = -1;
					break;
				}
				if(lseek(vfd, -nread, SEEK_CUR) < 0){
					sprintf(buf, "ERROR%d: verify seek error(%lx)\n", __LINE__, total);
					flash_write_log(buf);
					ret = -1;
					break;
				}
				sprintf(buf, "%d: verify error at %08lx bytes\n", __LINE__, total);
				flash_write_log(buf);
				goto RETRY;
			}
			else{
				sprintf(buf, "STOP: verify error\n");
				flash_write_log(buf);
				ret = -1;
				break;
			}
		}
#endif
		i++;
		erase.start += nread;
#if defined(CONFIG_OBSA6)
		erase64.start += nread;
#endif
		total += nread;
		if(i>=STATUS_COL) {
			fprintf(stderr, "\n");
			i=0;
		}
	}
	fprintf(stderr, "\n");

	free(membase);
	free(spare);
#ifdef VERIFY
	free(verify);
	close(vfd);
#endif
	close(ifd);
	close(ofd);

err_exit:
	flash_close_log();
	return ret;
}

#if defined(CONFIG_OBSAX3) || defined(CONFIG_OBSA7)
int
flash_prog_java(char *java)
{
	int i, ifd, ofd, vfd, nread, retry;
#if defined(CONFIG_OBSA6)
	struct erase_info_user64 erase64;
	erase_info_t erase;
	int rts = 0;
#else
	erase_info_t erase;
#endif
	struct stat sb;
	int sector_len;
	unsigned short *verify = NULL;
	unsigned long	total=0;
	char buf[512];
	
	int ret=NORMAL_END;

	flash_open_log();
	read_proc_mtd();
	if ((ifd = open(java, O_RDONLY)) < 0) {
		sprintf(buf, "ERROR%d: %s open error\n", __LINE__, java);
		flash_write_log(buf);
		ret = -1;
		goto err_exit;
	}

	if (fstat(ifd, &sb) < 0) {
		sprintf(buf, "ERROR%d: %s is not exist\n", __LINE__, java);
		flash_write_log(buf);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

#if 0
sb.st_size = JAVA_SIZE + 1;
#endif
	if (sb.st_size > JAVA_SIZE) {
		sprintf(buf, "ERROR%d: file size exceeds MAX Java SIZE\n", __LINE__);
		flash_write_log(buf);
		close(ifd);
		ret = -1;
		goto err_exit;
	}
	if ((membase = malloc(sb.st_size)) == NULL) {
		sprintf(buf, "ERROR%d: memory alloc error(size=%ld)\n", __LINE__, sb.st_size);
		flash_write_log(buf);
		ret = -1;
		goto err_exit;
	}
	nread = read(ifd, membase, sb.st_size);
#if 0
	nread--;
#endif
	if (nread != sb.st_size) {
		sprintf(buf, "ERROR%d: java image read error\n", __LINE__);
		flash_write_log(buf);
		free(membase);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

	if ((ofd = open(mtdname[MTD_JAVA], O_RDWR)) < 0) {
		sprintf(buf, "ERROR%d: %s open error\n", __LINE__, mtdname[MTD_JAVA]);
		flash_write_log(buf);
		close(ifd);
		ret = -1;
		goto err_exit;
	}

#ifdef VERIFY
	if ((vfd = open(mtdname[MTD_JAVA], O_RDONLY)) < 0) {
		sprintf(buf, "ERROR%d: %s open error\n", __LINE__, mtdname[MTD_JAVA]);
		flash_write_log(buf);
		close(ifd);
		close(ofd);
		ret = -1;
		goto err_exit;
	}

	if ((verify = malloc(SECT_SIZE)) == NULL) {
		sprintf(buf, "ERROR%d: memory alloc error(size=%d)\n", __LINE__, SECT_SIZE);
		flash_write_log(buf);
		close(ifd);
		close(ofd);
		close(vfd);
		ret = -1;
		goto err_exit;
	}
#endif

	erase.start = 0;
	sector_len = SECT_SIZE;
	erase.length = sector_len;
#if defined(CONFIG_OBSA6)
	erase64.start = 0;
	erase64.length = sector_len;
#endif
	i=0;
	lseek(ifd, 0, SEEK_SET);
	fprintf(stderr, "Load boot image to FlashROM\n");
	while ((nread = read(ifd, membase, sector_len)) > 0) {
		total += nread;
		retry = 2;
RETRY:
		fprintf(stderr, ".");
#if defined(CONFIG_OBSA6)
		rts = ioctl(ofd, MEMGETBADBLOCK, &erase64.start);
		if(rts > 0){
			erase.start += SECT_SIZE;
			erase64.start += SECT_SIZE;
			lseek(ofd, SECT_SIZE, SEEK_CUR);
			lseek(vfd, SECT_SIZE, SEEK_CUR);
			fprintf(stderr, "%cS",0x08);
			goto RETRY;
		}
		else if(rts < 0)
			break;
#endif
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
#if defined(CONFIG_OBSA6)
			/* found Bad Block, Mark bad */
			if((rts = ioctl(ofd, MEMSETBADBLOCK, &erase64.start)) < 0){
				sprintf(buf, "ERROR%d: %s(%lx)\n", __LINE__, strerror(errno), total);
				flash_write_log(buf);
				ret = -1;
				break;
			}
			erase.start += nread;
			erase64.start += nread;
			goto RETRY;
#else
			sprintf(buf, "ERROR%d: erase error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
#endif
		}
		fprintf(stderr, "%c#",0x08);
#if defined(CONFIG_OBSAX3)
		if (write(ofd, membase, nread) < 0) {
#else
		// CONFIG_OBSA6
		if (write(ofd, membase, SECT_SIZE) < 0) {
#endif
			sprintf(buf, "ERROR%d: write error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
		}

#ifdef VERIFY
		fprintf(stderr, "%c*",0x08);
		if(read(vfd, verify, nread) != nread){
			sprintf(buf, "ERROR%d: verify read error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
		}
#if 0	/* Force verify error */
if(i && !(i % 10) && retry == 2){
	verify[0] *= 2;
}
#endif
		if(memcmp(membase, verify, nread) != 0){
			fprintf(stderr, "%c!",0x08);
			if(--retry){
				if(lseek(ofd, -nread, SEEK_CUR) < 0){
					sprintf(buf, "ERROR%d: seek error(%lx)\n", __LINE__, total);
					flash_write_log(buf);
					ret = -1;
					break;
				}
				if(lseek(vfd, -nread, SEEK_CUR) < 0){
					sprintf(buf, "ERROR%d: verify seek error(%lx)\n", __LINE__, total);
					flash_write_log(buf);
					ret = -1;
					break;
				}
				sprintf(buf, "%d: verify error at %08lx bytes\n", __LINE__, total);
				flash_write_log(buf);
				goto RETRY;
			}
			else{
				sprintf(buf, "STOP: verify error\n");
				flash_write_log(buf);
				ret = -1;
				break;
			}
		}
#endif
		i++;
		erase.start += nread;
#if defined(CONFIG_OBSA6)
		erase64.start += nread;
#endif
		if(i>=STATUS_COL) {
			fprintf(stderr, "\n");
			i=0;
		}
	}
	fprintf(stderr, "\n");

	free(membase);
#ifdef VERIFY
	free(verify);
	close(vfd);
#endif
	close(ifd);
	close(ofd);

err_exit:
	flash_close_log();
	return ret;
}
#endif

int
flash_save_param(int target, char *list)
{
	int i, ifd, ofd, vfd, nread, retry;
#ifndef DEBIAN
	int pid, st;
#endif
	struct stat sb;
	char localbuf[256];
	char targetdevice[256];
#if defined(CONFIG_OBSA6)
	struct erase_info_user64 erase64;
	int rts=0;
#endif
	erase_info_t erase;
	unsigned long	total=0;
	char buf[512];
	int maxsize;
	unsigned short *verify = NULL;
	int ret=0;

	read_proc_mtd();
	if (target == 's') {
		maxsize = PARAM_SIZE;
		strcpy(targetdevice, mtdname[MTD_USRCONF]);
	} else if (target == 'S') {
		maxsize = USER_SIZE;
		strcpy(targetdevice, mtdname[MTD_USRDATA]);
	} else {
		return -1;
	} 

#ifndef DEBIAN
	strcpy(localbuf, "/tmp/flashcfg.XXXXXX");

	if ((ifd = mkstemp(localbuf)) < 0) {
		fprintf(stderr, "ERROR%d: mkstemp %s\n", __LINE__, strerror(errno));
		return -1;
	}

	if ((pid = fork()) == 0) {
		/* in child */
//		execl(tarpath, "tar", "cvpTzf", list, localbuf, NULL);
//printf("list=%s buf=%s\n", list, localbuf);
		execl(tarpath, "tar", "cvz", "-T", list, "-f", localbuf, NULL);
		/* not reached here */
	} else {
		wait(&st);
		if(st)
			return -1;
	}
#else
	strcpy(localbuf, list);
#endif

#if defined(DEBUG) && !defined(DEBIAN)
	if (st)
		fprintf(stderr, "child returns %d\n", st);
#endif

	if ((ifd = open(localbuf, O_RDONLY)) < 0) {
		fprintf(stderr, "ERROR%d: localbuf %s\n", __LINE__, strerror(errno));
		unlink(localbuf);
		return -1;
	}

	if (fstat(ifd, &sb) < 0) {
		fprintf(stderr, "ERROR%d: stat %s\n", __LINE__, strerror(errno));
		close(ifd);
		unlink(localbuf);
		return -1;
	}

	if (sb.st_size > maxsize) {
		fprintf(stderr, "file size exceeds Area\n");
		close(ifd);
		unlink(localbuf);
		return -1;
	}

	if ((membase = malloc(SECT_SIZE)) == NULL) {
		fprintf(stderr, "ERROR%d: malloc %s\n", __LINE__, strerror(errno));
		close(ifd);
		unlink(localbuf);
		return -1;
	}

	if ((ofd = open(targetdevice , O_RDWR)) < 0) {
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, targetdevice, strerror(errno));
		free(membase);
		close(ifd);
		unlink(localbuf);
		return -1;
	}

	if ((vfd = open(targetdevice, O_RDONLY)) < 0) {
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, targetdevice, strerror(errno));
		free(membase);
		close(ifd);
		close(ofd);
		unlink(localbuf);
		return -1;
	}

	if ((verify = malloc(SECT_SIZE)) == NULL) {
		fprintf(stderr, "ERROR%d: malloc %s\n", __LINE__, strerror(errno));
		free(membase);
		close(ifd);
		close(ofd);
		close(vfd);
		unlink(localbuf);
		return -1;
	}

	erase.start = 0;
	erase.length = SECT_SIZE;
#if defined(CONFIG_OBSA6)
	erase64.start = 0;
	erase64.length = SECT_SIZE;
#endif
	i=0;
	fprintf(stderr, "Save files to FlashROM\n");
	while ((nread = read(ifd, membase, SECT_SIZE)) > 0) {
		retry = 2;
RETRY:
		fprintf(stderr, ".");
#if defined(CONFIG_OBSA6)
		rts = ioctl(ofd, MEMGETBADBLOCK, &erase64.start);
		if(rts > 0){
			erase.start += SECT_SIZE;
			erase64.start += SECT_SIZE;
			lseek(ofd, SECT_SIZE, SEEK_CUR);
			lseek(vfd, SECT_SIZE, SEEK_CUR);
			fprintf(stderr, "%cS",0x08);
			goto RETRY;
		}
		else if(rts < 0){
//			sprintf(buf, "ERROR%d: %s(%lx)\n", __LINE__, strerror(errno), total);
//			flash_write_log(buf);
//			ret = -1;
			break;
		}
#endif
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
#if defined(CONFIG_OBSA6)
			/* found Bad Block, Mark bad */
			if((rts = ioctl(ofd, MEMSETBADBLOCK, &erase64.start)) < 0){
				sprintf(buf, "ERROR%d: %s(%lx)\n", __LINE__, strerror(errno), total);
				flash_write_log(buf);
				ret = -1;
				break;
			}
			erase.start += nread;
			erase64.start += nread;
			goto RETRY;
#else
			sprintf(buf, "ERROR%d: erase error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
#endif
		}
		fprintf(stderr, "%c#",0x08);
#if defined(CONFIG_OBSAX3)
		if (write(ofd, membase, nread) < 0) {
#else	// CONFIG_OBSA6
		if (write(ofd, membase, SECT_SIZE) < 0) {
#endif
			fprintf(stderr, "ERROR%d: mtd %s\n", __LINE__, strerror(errno));
			ret = -1;
			break;
		}

		fprintf(stderr, "%c*",0x08);
		if(read(vfd, verify, nread) != nread){
			fprintf(stderr, "ERROR%d: verify %s\n", __LINE__, strerror(errno));
			ret = -1;
			break;
		}
#if 0	/* Force verify error */
if(i && !(i % 10) && retry == 2){
	verify[0] *= 2;
}
#endif
		if(memcmp(membase, verify, nread) != 0){
			fprintf(stderr, "%c!",0x08);
			if(--retry){
				if(lseek(ofd, -nread, SEEK_CUR) < 0){
					fprintf(stderr, "ERROR%d: lseek %s\n", __LINE__, strerror(errno));
					ret = -1;
					break;
				}
				if(lseek(vfd, -nread, SEEK_CUR) < 0){
					fprintf(stderr, "ERROR%d: lseek %s\n", __LINE__, strerror(errno));
					ret = -1;
					break;
				}
				goto RETRY;
			}
			else{
				fprintf(stderr, " STOP.\nflash memory verify error\n");
				ret = -1;
				break;
			}
		}
		i++;
		erase.start += nread;
#if defined(CONFIG_OBSA6)
		erase64.start += nread;
#endif
		if(i>=STATUS_COL) {
			fprintf(stderr, "\n");
			i=0;
		}
	}
	fprintf(stderr, "\n");

	free(membase);
	free(verify);
	close(ifd);
	close(ofd);
	close(vfd);
	unlink(localbuf);

	return ret;
}

#if defined(CONFIG_OBSAX3) || defined(CONFIG_OBSA7)
int
flash_extract_java(int target)
{
	int pid, st;
	char buf[256];
	char targetdevice[256];
	int ret=NORMAL_END;

#if defined(CONFIG_OBSA6)
	sprintf(targetdevice, "%s/%s", TMPFS, FNAME);
	if(mount_mtddev(MTD_JAVA) != 0)
		return -1;
	if(get_mtdfile(MTD_JAVA, targetdevice) != 0)
		goto error;
#endif

	if ((pid = fork()) == 0) {
		/* in child */
#ifdef EXTRACT_LZMA
		if(strcmp(tarpath, tarssd) == 0){
			ret = execl(tarpath, "tar", "--warning=no-timestamp",
					"-xvpf", targetdevice, "--lzma -C", "/", NULL);
		}
		else{
			sprintf(buf, "lzma -d < %s | tar xvpf - -C /", targetdevice);
			ret = execl("/bin/sh", "sh", "-c", buf, NULL);
		}
#else
		if(strcmp(tarpath, tarssd) == 0){
			ret = execl(tarpath, "tar", "--warning=no-timestamp",
					"-xvpjf", targetdevice, "-C", "/", NULL);
		}
		else{
			sprintf(buf, "bunzip2 < %s | tar xvpf - -C /", testdevice);
			ret = execl("/bin/sh", "sh", "-c", buf, NULL);
		}
#endif

		/* not reached here */
		return ret;
	} else {
		wait(&st);
		if(st){
#ifdef DEBUG
			fprintf(stderr, "child returns %d\n", st);
#endif
			return -1;
		}
	}
error:
#if defined(CONFIG_OBSA6)
	umount_mtddev();
#endif
	return ret;
}
#endif

int
flash_extract_param(int target)
{
	int pid, st;
	char targetdevice[256], buf[256], option[16];
	int ret=NORMAL_END;

#if defined(CONFIG_OBSA6)
	int mode = target == 'x' ? MTD_USRCONF : MTD_USRDATA;
	sprintf(targetdevice, "%s/%s", TMPFS, FNAME);
	if(mount_mtddev(mode) != 0)
		return -1;
	if(get_mtdfile(mode, targetdevice) != 0)
		goto error;
#else
	if (target == 'x') {
		strcpy(targetdevice, mtdname[MTD_USRCONF]);
	} else if (target == 'X') {
		strcpy(targetdevice, mtdname[MTD_USRDATA]);
	} else {
		return -1;
	}
#endif

	if ((pid = fork()) == 0) {
		/* in child */
		if(strcmp(tarpath, tarssd) == 0){
			if(read_magic(targetdevice))
				strcpy(option, "-xvpJf");
			else
				strcpy(option, "-xvpzf");
			ret = execl(tarpath, "tar", "--warning=no-timestamp",
					option, targetdevice, "-C", EXTRACTPATH, NULL);
		}
		else{
			if(read_magic(targetdevice))
				sprintf(buf, "unxz < %s | tar xvpf - -C %s", targetdevice, EXTRACTPATH);
			else
				sprintf(buf, "gunzip < %s | tar xvpf - -C %s", targetdevice, EXTRACTPATH);
			ret = execl("/bin/sh", "sh", "-c", buf, NULL);
		}

		/* not reached here */
		goto error;
	} else {
		wait(&st);
		if(st){
#ifdef DEBUG
			fprintf(stderr, "child returns %d\n", st);
#endif
			goto error;
		}
	}

error:
#if defined(CONFIG_OBSA6)
	umount_mtddev();
#endif
	return ret;
}

int flash_delete_param(int target)
{
	int ofd, i;
#if defined(CONFIG_OBSA6)
	struct erase_info_user64 erase64;
	erase_info_t erase;
	int rts;
#else
	erase_info_t erase;
#endif
	char targetdevice[256];
	int maxsize, cnt, col;

	read_proc_mtd();
	if (target == 'd') {
		maxsize = PARAM_SIZE;
		strcpy(targetdevice, mtdname[MTD_USRCONF]);
	} else if (target == 'D') {
		maxsize = USER_SIZE;
		strcpy(targetdevice, mtdname[MTD_USRDATA]);
	} else if (target ==  'R') {
		maxsize = MPARAM_SIZE;
		strcpy(targetdevice, mtdname[MTD_UBOOTENV]);
	} else if (target ==  'r') {
		maxsize = KERNEL_SIZE;
		strcpy(targetdevice, mtdname[MTD_KERNEL]);
	} else {
		return -1;
	}


	if ((ofd = open(targetdevice, O_RDWR)) < 0) {
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, targetdevice, strerror(errno));
		close(ofd);
		return -1;
	}

	erase.start = 0;
	erase.length = SECT_SIZE;
#if defined(CONFIG_OBSA6)
	erase64.start = 0;
	erase64.length = SECT_SIZE;
#endif
	fprintf(stderr, "Delete saved files from FlashROM\n");
	cnt = maxsize / SECT_SIZE - 1;
	for (i = 0,col=0; i < cnt; i++) {
		fprintf(stderr, ".");
#if defined(CONFIG_OBSA6)
		rts = ioctl(ofd, MEMGETBADBLOCK, &erase64.start);
#if 0
	if(i % 100 == 0) rts = 1;
#endif
		if(rts > 0){
			erase.start += SECT_SIZE;
#if defined(CONFIG_OBSA6)
			erase64.start += SECT_SIZE;
#endif
			lseek(ofd, SECT_SIZE, SEEK_CUR);
			fprintf(stderr, "%cS",0x08);
			continue;
		}
		else if(rts < 0){
			printf("ERROR%d: %s(%x)\n", __LINE__, strerror(errno), erase.start);
//			flash_write_log(buf);
//			ret = -1;
			break;
		}
#endif
		fflush(stderr);
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
			printf("ERROR%d: %s(%x)\n", __LINE__, strerror(errno), erase.start);
//			close(ofd);
//			return -1;
			break;
		}
		erase.start += SECT_SIZE;
#if defined(CONFIG_OBSA6)
		erase64.start += SECT_SIZE;
#endif
		col++;
		if(col >= STATUS_COL) {
			col = 0;
			fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "\n");
	close(ofd);

	return NORMAL_END;
}

int flash_restore_ubootenv(char *fname)
{
	FILE *fp;
	char buf[ENV_SIZE];
	char *p_buf = &buf[5];
	int cnt;

	/* Read U-BOOT Environment */
	if((fp = fopen(mtdname[MTD_UBOOTENV], "r")) == NULL){
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, mtdname[MTD_UBOOTENV], strerror(errno));
		return -1;
	}
	memset(buf, 0x0, sizeof(buf));
	if(fread(buf, sizeof(buf), 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(buf[4] == 0x0){			/* area1 is absolute */
		char buf2[ENV_SIZE];

		if(fseek(fp, ENV_AREA >> 1, SEEK_SET) != 0){
			fprintf(stderr, "ERROR%d: fseek %s\n", __LINE__, strerror(errno));
			fclose(fp);
			return -1;
		}
		if(fread(buf2, sizeof(buf2), 1, fp) != 1){
			fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
			return -1;
		}
		if(buf2[4] == 0x1){		/* area2 is active */
			memcpy(buf, buf2, sizeof(buf));
		}
	}
	fclose(fp);

	/* Write local file */
	if((fp = fopen(fname, "w")) == NULL){
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, fname, strerror(errno));
		return -1;
	}
	while(*p_buf || *(p_buf-1)){
		cnt = fprintf(fp, "%s\n", p_buf);
		p_buf += cnt;
	}
	fclose(fp);

	return NORMAL_END;
}

int
read_proc_mtd(void) {
	FILE *ifp;
	char buf[80];

	if ((ifp = fopen("/proc/mtd", "r")) == NULL) {
		fprintf(stderr, "ERROR%d: /proc/mtd %s\n", __LINE__, strerror(errno));
		return (1);
	}

	while (fscanf(ifp, "%s", buf) != EOF) {
		if (strncmp(buf, "mtd5:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			USER_SIZE = strtol(buf, NULL, 16);
			fscanf(ifp, "%s", buf);
			SECT_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd4:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			KERNEL_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd3:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			PARAM_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd1:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			MPARAM_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd0:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			MONITOR_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd2:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			TSTPRG_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd6:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			JAVA_SIZE = strtol(buf, NULL, 16);
		}
	}
	fclose(ifp);
#ifdef DEBUG
	printf("SECT_SIZE:    %d\n", SECT_SIZE);
	printf("MONITOR_SIZE: %d\n", MONITOR_SIZE);
	printf("PARAM_SIZE:   %d\n", PARAM_SIZE);
	printf("USER_SIZE:    %d\n", USER_SIZE);
	printf("KERNEL_SIZE:  %d\n", KERNEL_SIZE);
	printf("MPARAM_SIZE:  %d\n", MPARAM_SIZE);
	printf("TSTPRG_SIZE:  %d\n", TSTPRG_SIZE);
	printf("JAVA_SIZE:  %d\n", JAVA_SIZE);
#endif
	return (0);
}

void flash_open_log(void)
{
	struct stat st;

	if(stat(LOGPATH_CF, &st) == 0){
		if((fp_log = fopen(LOGPATH_CF LOGNAME, "a")) != NULL)
			return ;
	}
	fp_log = fopen(LOGPATH_RAM LOGNAME, "w");
}

void flash_close_log(void)
{
	if(fp_log){
		fclose(fp_log);
		fp_log = NULL;
	}
}

void flash_write_log(char *s)
{
	char buf[256];
	time_t tm;

	fprintf(stderr, "%s", s);
	if(fp_log){
		time(&tm);
		strftime(buf, sizeof(buf)-1, "%c", localtime(&tm));
		fprintf(fp_log, "%s %s", buf, s);
	}
}

char** set_mtdtype(void)
{
	struct stat buf;
	if(stat(mtdname_old[MTD_KERNEL], &buf) == 0)
		return (char**)mtdname_old;
	else if(stat(mtdname_new[MTD_KERNEL], &buf) == 0)
		return (char**)mtdname_new;
	
	return NULL;
}

char * set_tarpath(void)
{
	struct stat buf;
	if(stat(tarssd, &buf) == 0)
		return (char*)tarssd;
	else if(stat(tarother, &buf) == 0)
		return (char*)tarother;

	return NULL;
}


int mtd_protect(int mtd_num, int on)
{
	int fd;
	char buf[64], val[] = {0,0,0,0,0,0};

	sprintf(buf, "/sys/devices/virtual/mtd/mtd%d/flags", mtd_num);
	if((fd = open(buf, O_RDWR)) == -1){
		fprintf(stderr, "ERROR%d: mtd%d open error\n", __LINE__, mtd_num);
		return -1;
	}

	/* on=1: protect on, on=0: protect off */
	if(on)
		strcpy(val, READONLY);
	else
		strcpy(val, READWRITE);

	if(write(fd, val, sizeof(val)) != sizeof(val)){
		fprintf(stderr, "ERROR%d: mtd%d write error\n", __LINE__, mtd_num);
		close(fd);
		return -1;
	}
	close(fd);

	return 0;
}

#if defined(CONFIG_OBSA6)
int mount_mtddev(int mtd)
{
	int ret;
	char opt[16];

	/* make directory */
	if((ret = mkdir(TMPFS, S_IRUSR|S_IWUSR)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ret;
	}

	/* mount tmpfs */
	if(mtd == MTD_USRCONF)
		strcpy(opt, "size=10M");
	else if(mtd == MTD_JAVA)
		strcpy(opt, "size=60M");
	else	/* MTD_USRDATA */
#if defined(CONFIG_OBSA7)
		strcpy(opt, "size=190M");
#else
		strcpy(opt, "size=60M");
#endif

	if((ret = mount("none", TMPFS, "tmpfs", 0, opt)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ret;
	}
	return 0;
}

int get_mtdfile(int mtd, char* fname)
{
	struct erase_info_user64 erase64;
	int src, dst, ret=0;
	char *p;
	int i, len;
	
#if defined(CONFIG_OBSA7)
	SECT_SIZE = 131072;
#elif defined(CONFIG_OBSA6)
	SECT_SIZE = 16384;
#else
	SECT_SIZE = 0;
#endif

	erase64.start = 0;
	erase64.length = SECT_SIZE;

	if((src = open(mtdname[mtd], O_RDONLY)) < 0){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if((dst = open(fname, O_WRONLY|O_CREAT)) < 0){
		printf("%d: %s\n", __LINE__, strerror(errno));
		close(src);
		return -1;
	}
	if((p = malloc(SECT_SIZE)) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		ret = -1;
		goto error2;
	}

	for(i=0;;i++){
		ret = ioctl(src, MEMGETBADBLOCK, &erase64.start);
		if(ret > 0){
			lseek(src, SECT_SIZE, SEEK_CUR);
			erase64.start += SECT_SIZE;
			continue;
		}
		else if(ret < 0){
			if(errno == EINVAL)
				ret = 0;
			break;
		}

		if((len = read(src, p, SECT_SIZE)) < 0){
			printf("%d: %s\n", __LINE__, strerror(errno));
			ret = -1;
			goto error1;
		}
		else if(!len)	/* EOF */
			break;

#if 0
		/* check GZIP header */
		if((mtd == MTD_USRCONF || mtd == MTD_USRDATA) && i == 0){
			if(*p != 0x1f || *(p+1) != 0x8b){
				printf("%d: not in gzip format\n", __LINE__);
				ret = -1;
				goto error1;
			}
		}
#endif

		if(write(dst, p, SECT_SIZE) < 0){
			printf("%d: %s\n", __LINE__, strerror(errno));
			ret = -1;
			goto error1;
		}
		erase64.start += SECT_SIZE;
	}
#ifdef DEBUG
printf("size=%x ret=%d\n", i * SECT_SIZE, ret);
#endif
error1:
	free(p);
error2:
	close(src);
	close(dst);
	return ret;
}

int umount_mtddev(void)
{
	int ret=0;
	/* unmount tmpfs */
	if((ret = umount(TMPFS)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ret;
	}

	/* remove directory */
	if((ret = rmdir(TMPFS)) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ret;
	}
	return ret;
}
#endif

#if defined(CONFIG_OBSAX3)
#include <time.h>
#define MTDSYSDIR	"/sys/devices/virtual/mtd/%s/"
#define MTDDIR		"/dev/%s"
#define MTDFLAGS	MTDSYSDIR"flags"
struct core_msg{
	struct timeval flag;
	char msg[];
};
int test_core_area(void)
{
	int fd;
	erase_info_t erase;
	char *mtd;
	int i;
	char buf[256], part[8];

	if(JAVA_SIZE)
		strcpy(part, "mtd6");
	else
		strcpy(part, "mtd5");

	sprintf(buf, MTDFLAGS, part);
	fd = open(buf, O_WRONLY);
	if(fd == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(write(fd, "0xc00", 6) < 0){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	close(fd);

	printf("Erase core dump area...");
	sprintf(buf, MTDDIR, part);
	fd = open(buf, O_WRONLY);
	if(fd == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(JAVA_SIZE){
		erase.start = JAVA_SIZE - SECT_SIZE;
		erase.length = SECT_SIZE;
	}
	else{
		erase.start = USER_SIZE - SECT_SIZE;
		erase.length = SECT_SIZE;
	}
	if (ioctl(fd, MEMERASE, &erase) != 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	close(fd);
	printf("done\n");

	printf("Write core dump area...");
	if((mtd = malloc(SECT_SIZE)) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	for(i=0; i<SECT_SIZE; i++){
		mtd[i] = (char)i;
	}
	sprintf(buf, MTDDIR, part);
	fd = open(buf, O_WRONLY);
	if(fd == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(lseek(fd, SECT_SIZE * -1, SEEK_END) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(write(fd, mtd, SECT_SIZE) < 0){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	close(fd);
	free(mtd);
	printf("done\n");

	printf("Read and Compare core dump area...");
	if((mtd = malloc(SECT_SIZE)) == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	sprintf(buf, MTDDIR, part);
	fd = open(buf, O_RDONLY);
	if(fd == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(lseek(fd, SECT_SIZE * -1, SEEK_END) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(read(fd, mtd, SECT_SIZE) < 0){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	close(fd);
	for(i=0; i<SECT_SIZE; i++){
		if(mtd[i] != (char)i){
			printf("%d: core dump area verify error at 0x%08x(%02x)\n", __LINE__, i, mtd[i]);
			return ERROR_END;
		}
	}
	free(mtd);
	printf("done\n");

	printf("Erase core dump area...");
	sprintf(buf, MTDDIR, part);
	fd = open(buf, O_WRONLY);
	if(fd == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(JAVA_SIZE){
		erase.start = JAVA_SIZE - SECT_SIZE;
		erase.length = SECT_SIZE;
	}
	else{
		erase.start = USER_SIZE - SECT_SIZE;
		erase.length = SECT_SIZE;
	}
	if (ioctl(fd, MEMERASE, &erase) != 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	close(fd);
	printf("done\n");

	printf("OK\n");
	return NORMAL_END;
}

int read_core_area(void)
{
	int fd;
	int len;
	char buf[256], part[8];
	struct core_msg *mtd;
	int page = SECT_SIZE / 2;
	struct tm *tm;

	if(JAVA_SIZE)
		strcpy(part, "mtd6");
	else
		strcpy(part, "mtd5");

	mtd = malloc(SECT_SIZE);
	if(mtd == NULL){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	sprintf(buf, MTDDIR, part);
	fd = open(buf, O_RDONLY);
	if(fd == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	if(lseek(fd, SECT_SIZE * -1, SEEK_END) == -1){
		printf("%d: %s\n", __LINE__, strerror(errno));
		return ERROR_END;
	}
	len = read(fd, mtd, SECT_SIZE);
	if(len != SECT_SIZE){
		printf("%d: read error(%d)\n", __LINE__, len);
		return ERROR_END;
	}
	close(fd);

//	printf("\n");
	tm = localtime(&mtd->flag.tv_sec);
	if ((mtd->msg[0] >= 33) && (mtd->msg[0] <= 126)) {
		printf("%d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("%s", mtd->msg);
		printf("\n\n");
	}

	tm = localtime(&((struct core_msg*)((char*)mtd+page))->flag.tv_sec);
	if ((((struct core_msg*)((char*)mtd+page))->msg[0] >= 33) &&
	    (((struct core_msg*)((char*)mtd+page))->msg[0] <= 126)) {
		printf("%d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
		printf("%s", ((struct core_msg*)((char*)mtd+page))->msg);
	}

	free(mtd);
	return NORMAL_END;
}

/*
	u-boot environment area 0xfff60000 - 0xfff9ffff
	offset	0x0     - 0x1FFFF	now environment area	    use 0x4000 of 0x20000
		0x20000 - 0x3FFFF	previous environmet area    use 0x4000 of 0x20000
*/
#if 0
int
flash_set_pmlevel(int target, char *arg)
{
#define OFFSET		5
	FILE *fp;
	char env_area1[ENV_SIZE];		// u-boot environment area1
	char env_area2[ENV_SIZE];		// u-boot environment area2
	char *p_prev, *p_now, *wk_prev, *wk_now;
	unsigned long crc1, crc2;
	char buf[512];
	char *p;

	memset(env_area1, 0x0, ENV_SIZE);
	memset(env_area2, 0x0, ENV_SIZE);

	read_proc_mtd();

	if((fp = fopen(mtdname[MTD_UBOOTENV], "r")) == NULL){
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, mtdname[MTD_UBOOTENV], strerror(errno));
		return -1;
	}
	/* read area1 */
	if(fread(env_area1, ENV_SIZE, 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	if(fseek(fp, ENV_AREA >> 1, SEEK_SET) != 0){
		fprintf(stderr, "ERROR%d: fseek %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	/* read area2 */
	if(fread(env_area2, ENV_SIZE, 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	fclose(fp);

#if 0
dump(env_area1, ENV_SIZE);
printf("\n");
printf("\n");
dump(env_area2, ENV_SIZE);
#endif

	crc1 = crc32(0, (unsigned char*)&env_area1[OFFSET], ENV_SIZE-OFFSET);
	crc2 = crc32(0, (unsigned char*)&env_area2[OFFSET], ENV_SIZE-OFFSET);

	if(!crc1 && !crc2){			/* Invalid both */
		fprintf(stderr, "ERROR%d: environment area is empty\n", __LINE__);
		return -1;
	}
	else if(crc1 && !crc2){		/* Invalid area2 */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(!crc1 && crc2){		/* Invalid area1 */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 1 && env_area2[4] == 1){	/* both is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 1 && env_area2[4] == 0){	/* area1 is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0 && env_area2[4] == 1){	/* area2 is active */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0 && env_area2[4] == 0){	/* both is obsolete */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0xff && env_area2[4] != 0xff){	/* area1 is empty */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] != 0xff && env_area2[4] == 0xff){	/* area2 is empty */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else{	/* other case */
		fprintf(stderr, "ERROR%d: environment area is empty\n", __LINE__);
		return -1;
	}

	wk_prev = p_prev;
	wk_now = p_now;
	/* previous environment -> now environment */
	while(wk_prev < p_prev + (ENV_SIZE-OFFSET)){
		if((p=strstr(wk_prev, "pm_level=")) && p == wk_prev){		/* found "pm_level=" */
			if(strcmp(arg, "now") == 0){
				if((p = strchr(wk_prev, '=')))
					printf("The current state is %s\n", ++p);
				else
					printf("The current state is wfi\n");
				return NORMAL_END;
			}
			else
				sprintf(buf, "miscargs=pm_level=%s", arg);

			if(wk_now - p_now > strlen(buf)){
				strcpy(wk_now, buf);
				wk_now += strlen(buf) + 1;
			}
			else{
				fprintf(stderr, "ERROR%d: bootargs string is too long\n", __LINE__);
				return -1;
			}
		}
		else{
			/* no change */
			strcpy(wk_now, wk_prev);
			wk_now += strlen(wk_prev) + 1;
		}
		wk_prev += strlen(wk_prev) + 1;	/* +1 = '\0' */
	}
	crc1 = crc32(0, (unsigned char*)&p_now[OFFSET], ENV_SIZE-OFFSET);
	memcpy(p_now, &crc1, sizeof(crc1));

	/* set flag */
	p_now[4] = 0x1;		/* active */
	p_prev[4] = 0x0;	/* obsolete */

	fprintf(stderr, "Change 1st U-Boot environment area\n");
	if(flash_prog_bootdev(p_now, 0) < 0){
		fprintf(stderr, "ERROR%d: flash memory write error in area1\n", __LINE__);
		return -1;
	}
	fprintf(stderr, "Change 2nd U-Boot environment area\n");
	if(flash_prog_bootdev(p_prev, ENV_AREA >> 1) < 0){
		fprintf(stderr, "ERROR%d: flash memory write error in area2)\n", __LINE__);
		return -1;
	}
#ifdef DEBUG
	if((fp = fopen("/tmp/now.env", "w")) != NULL){
		fwrite(p_now, ENV_SIZE, 1, fp);
		fclose(fp);
	}
	if((fp = fopen("/tmp/prev.env", "w")) != NULL){
		fwrite(p_prev, ENV_SIZE, 1, fp);
		fclose(fp);
	}
#endif
	fprintf(stderr, "Power management level change to %s\n", arg);
	return NORMAL_END;
}
#else
/*
	u-boot environment area 0xfff60000 - 0xfff9ffff
	offset	0x0     - 0x1FFFF	now environment area	    use 0x4000 of 0x20000
		0x20000 - 0x3FFFF	previous environmet area    use 0x4000 of 0x20000
*/
int
flash_set_pmlevel(int target, char *arg)
{
#define OFFSET		5
	FILE *fp;
	char env_area1[ENV_SIZE];		// u-boot environment area1
	char env_area2[ENV_SIZE];		// u-boot environment area2
	char *p_prev, *p_now, *wk_prev, *wk_now;
	unsigned long crc1, crc2;
	char buf[512];
	char *p;
	int flag=0;

	memset(env_area1, 0x0, ENV_SIZE);
	memset(env_area2, 0x0, ENV_SIZE);

	read_proc_mtd();

	if((fp = fopen(mtdname[MTD_UBOOTENV], "r")) == NULL){
		fprintf(stderr, "ERROR%d: %s %s\n", __LINE__, mtdname[MTD_UBOOTENV], strerror(errno));
		return -1;
	}
	/* read area1 */
	if(fread(env_area1, ENV_SIZE, 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	if(fseek(fp, ENV_AREA >> 1, SEEK_SET) != 0){
		fprintf(stderr, "ERROR%d: fseek %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	/* read area2 */
	if(fread(env_area2, ENV_SIZE, 1, fp) != 1){
		fprintf(stderr, "ERROR%d: fread %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	fclose(fp);

	crc1 = crc32(0, (unsigned char*)&env_area1[OFFSET], ENV_SIZE-OFFSET);
	crc2 = crc32(0, (unsigned char*)&env_area2[OFFSET], ENV_SIZE-OFFSET);

	if(!crc1 && !crc2){			/* Invalid both */
		fprintf(stderr, "ERROR%d: environment area is empty\n", __LINE__);
		return -1;
	}
	else if(crc1 && !crc2){		/* Invalid area2 */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(!crc1 && crc2){		/* Invalid area1 */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 1 && env_area2[4] == 1){	/* both is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 1 && env_area2[4] == 0){	/* area1 is active */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0 && env_area2[4] == 1){	/* area2 is active */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0 && env_area2[4] == 0){	/* both is obsolete */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] == 0xff && env_area2[4] != 0xff){	/* area1 is empty */
		p_now = env_area1;
		p_prev = env_area2;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else if(env_area1[4] != 0xff && env_area2[4] == 0xff){	/* area2 is empty */
		p_now = env_area2;
		p_prev = env_area1;
		memset(p_now, 0x0, ENV_SIZE);
	}
	else{	/* other case */
		fprintf(stderr, "ERROR%d: environment area is empty\n", __LINE__);
		return -1;
	}

	wk_prev = p_prev;
	wk_now = p_now;
	/* previous environment -> now environment */
	while(wk_prev < p_prev + (ENV_SIZE-OFFSET)){
		if(!wk_prev[0])		/* last of environment */
			break;
		if((p=strstr(wk_prev, "miscargs=")) && p == wk_prev){	/* found "miscargs=" */
			flag=1;												/* miscargs exist */
			if(strcmp(arg, "now") == 0){
				if((p = strchr(wk_prev, '='))){
					if((p = strchr(++p, '=')))
						printf("The current setting is %s\n", ++p);
				}
				else
					printf("The current setting is unknown\n");
				return NORMAL_END;
			}
			else
				sprintf(buf, "miscargs=pm_level=%s", arg);

			if(wk_now - p_now > strlen(buf)){
				strcpy(wk_now, buf);
				wk_now += strlen(buf) + 1;
			}
			else{
				fprintf(stderr, "ERROR%d: bootargs string is too long\n", __LINE__);
				return -1;
			}
		}
		else{
			/* no change */
			strcpy(wk_now, wk_prev);
			wk_now += strlen(wk_prev) + 1;
		}
		wk_prev += strlen(wk_prev) + 1;	/* +1 = '\0' */
	}
	if(!flag){												/* not found "miscargs=" */
		if(strcmp(arg, "now") == 0){
			printf("The current setting is unknown\n");
			return NORMAL_END;
		}
		else{
			sprintf(buf, "miscargs=pm_level=%s", arg);
			if(wk_now - p_now > strlen(buf))
				strcpy(wk_now, buf);
		}
	}
	crc1 = crc32(0, (unsigned char*)&p_now[OFFSET], ENV_SIZE-OFFSET);
	memcpy(p_now, &crc1, sizeof(crc1));

	/* set flag */
	p_now[4] = 0x1;		/* active */
	p_prev[4] = 0x0;	/* obsolete */

	fprintf(stderr, "Change 1st U-Boot environment area\n");
	if(flash_prog_bootdev(p_now, 0) < 0){
		fprintf(stderr, "ERROR%d: flash memory write error in area1\n", __LINE__);
		return -1;
	}
	fprintf(stderr, "Change 2nd U-Boot environment area\n");
	if(flash_prog_bootdev(p_prev, ENV_AREA >> 1) < 0){
		fprintf(stderr, "ERROR%d: flash memory write error in area2)\n", __LINE__);
		return -1;
	}
	fprintf(stderr, "Power management level change to %s\n", arg);
	return NORMAL_END;
}
#endif
#endif


/*
 * retun	0 : gzip
 *			1 : lzma
 *		   -1 : ERROR
*/
int read_magic(char *mtd)
{
	int fd;
	char buf[6];

	if ((fd = open(mtd, O_RDONLY)) < 0)
		return -1;
	if(read(fd, buf, sizeof(buf)) != sizeof(buf))
		return -1;
	close(fd);

	if(buf[0] == 0x1f && buf[1] == 0x8b)
		return 0;
	else if(buf[0] == 0xfd && buf[1] == 0x37 && buf[2] == 0x7a
				&& buf[3] == 0x58 && buf[4] == 0x5a && buf[5] == 0x00)
		return 1;

	return 0;
}

#ifdef DEBUG
unsigned char chgchr(char c)
{
	if(c >= 0x20 && c < 0x7f)
		return (unsigned char)c;
	else
		return '.';
}

void dump(unsigned char *p, int size)
{
	int i, len = size / 16;

	for(i=0; i<len; i++, p+=16){
		printf("%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x "
			"%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
			*p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7),
			*(p+8), *(p+9), *(p+10), *(p+11), *(p+12), *(p+13), *(p+14), *(p+15),
			chgchr(*p), chgchr(*(p+1)), chgchr(*(p+2)), chgchr(*(p+3)),
			chgchr(*(p+4)), chgchr(*(p+5)), chgchr(*(p+6)), chgchr(*(p+7)),
			chgchr(*(p+8)), chgchr(*(p+9)), chgchr(*(p+10)), chgchr(*(p+11)),
			chgchr(*(p+12)), chgchr(*(p+13)), chgchr(*(p+14)), chgchr(*(p+15)));
	}
}
#endif

