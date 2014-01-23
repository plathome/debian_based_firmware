/*	$ssdlinux: flashcfg.c,v 1.26 2014/01/07 07:20:55 yamagata Exp $	*/
/*
 * Copyright (c) 2008-2014 Plat'Home CO., LTD.
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

//#define DEBUG

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/stat.h>
//#include <linux/compiler.h>
#include <mtd/mtd-user.h>
//#ifdef DEBIAN
//#include <linux/zlib.h>
//#else
#include <zlib.h>
#include <errno.h>
//#endif
#include <sys/wait.h>
#include <sys/types.h>

extern char **environ;
/* some variables used in getopt (3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

#define ENV_AREA	0x40000		/* u-boot environment area size */
#define ENV_SIZE	0x4000		/* size of u-boot environment size */
#define VERIFY
#if 1	/* return code */
#define NORMAL_END	0
#define ERROR_END	1
#endif

#if 1	/* support for /dev/mtd[0-6] */
#define MTD_KERNEL 0
#define MTD_USRCONF 1
#define MTD_USRDATA 2
#define MTD_UBOOTENV 5

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
	NULL,
	NULL,
	"/dev/mtd/5",
	NULL
};

const char *mtdname_new[] = {
	"/dev/mtd0",
	"/dev/mtd1",
	"/dev/mtd2",
	NULL,
	NULL,
	"/dev/mtd5",
	NULL
};

static char **mtdname;

char * set_tarpath(void);
const char* tarssd = "/usr/bin/tar";
const char* tarother = "/bin/tar";
static char* tarpath;
#endif

/*
 *      /proc/mtd
 *      dev:     size       erasesize     name
 *      mtd0: KERNEL_SIZE   SECT_SIZE  "OpenBlockS-OBS600 Kernel Image"
 *      mtd1: PARAM_SIZE               "OpenBlockS-OBS600 Configuration Data"
 *	mtd2: USER_SIZE	    SECT_SIZE  "OpenBlockS-OBS600 User Flash"
 *	mtd3: DTB           SECT_SIZE  "OpenBlockS-OBS600 DTB"
 *      mtd4: TSTPRG_SIZE              "OpenBlockS-OBS600 Diag program Image"
 *      mtd5: MPARAM_SIZE              "OpenBlockS-OBS600 Monitor Configuration Data"
 *      mtd6: MONITOR_SIZE             "OpenBlockS-OBS600 Uboot"
 */

#define VERSION "$Revision: 1.26 $"

int SECT_SIZE = 0;
int MONITOR_SIZE = 0;
int PARAM_SIZE = 0;
int MPARAM_SIZE = 0;
int USER_SIZE = 0;
int KERNEL_SIZE = 0;
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

unsigned short *membase;
FILE *fp_log = NULL;

int flash_set_bootdev(int, char *);
int flash_prog_kern(char *);
int flash_save_param(int i, char *);
int flash_delete_param(int i);
int flash_extract_param(int i);
int read_proc_mtd(void);
int flash_restore_ubootenv(char*);
#if 1	/* write log */
void flash_open_log(void);
void flash_close_log(void);
void flash_write_log(char*);
#define LOGNAME		"flashcfg.log"
#define LOGPATH_CF	"/usr/pkg/var/log/"
#define LOGPATH_RAM	"/var/log/"
#endif

/*#define DEBUG*/

void
usage()
{
	fprintf(stderr, "Flash Memory Configuration Tool (%s)\n\n", VERSION);
	fprintf(stderr, "usage: flashcfg                 Show this.\n");
	fprintf(stderr, "       flashcfg -b              Show current boot setting\n");
	fprintf(stderr, "       flashcfg -c rootdev      Change root file system [initrd|cf|sda-sdd[1-8]]\n");
	fprintf(stderr, "                                initrd       - Boot from FlashROM, / on Ramdisk\n");
	fprintf(stderr, "                                sda-sdd[1-8] - Boot from FlashROM, / on CF\n");
	fprintf(stderr, "                                cf           - Boot from CF,       / on CF\n");
	fprintf(stderr, "       flashcfg -f boot_image   Load boot image to flash\n");
#ifndef DEBIAN
	fprintf(stderr, "       flashcfg -s list_file    Save files to Parameter area\n");
	fprintf(stderr, "       flashcfg -S list_file    Save files to User area\n");
	fprintf(stderr, "       flashcfg -x              Restore files from Parameter area\n");
	fprintf(stderr, "       flashcfg -X              Restore files from User area\n");
#else
	fprintf(stderr, "       flashcfg -s list_file    Save files to Parameter area\n");
	fprintf(stderr, "       flashcfg -S list_file    Save files to User area\n");
#endif
	fprintf(stderr, "       flashcfg -d              Delete saved files from Parameter area\n");
	fprintf(stderr, "       flashcfg -D              Delete saved files from User area\n");
#ifndef DEBIAN
	fprintf(stderr, "       flashcfg -E env_file     Save params to U-Boot Environment area\n");
	fprintf(stderr, "       flashcfg -L env_file     Load params from U-Boot Environment area\n");
//	fprintf(stderr, "       flashcfg -R              Remove saved params from U-Boot Environment area\n");
#endif
	fprintf(stderr, "       flashcfg -h              Show this.\n");
}

int
main(int argc, char *argv[])
{
	FILE *fp;
	int i;
	char boot_dev[256];
	char buf[32];
	char *p;

	if (getuid()) {
		fprintf(stderr, "must be super user\n");
		return ERROR_END;
	}

#if 1	/* support for /dev/mtd[0-6] */
	if((mtdname = set_mtdtype()) == NULL){
		fprintf(stderr, "mtd device not found!\n");
		return ERROR_END;
	}
	if((tarpath = set_tarpath()) == NULL){
		fprintf(stderr, "tar command not found!\n");
		return ERROR_END;
	}
#endif
#ifdef DEBUG
	printf("mtd0=%s\n", mtdname[0]);
	printf("tar=%s\n", tarpath);
#endif

#ifdef DEBIAN
	while ((i = getopt(argc, argv, "c:f:s:S:j:x::X::bdDJr")) != -1) {
#else
	while ((i = getopt(argc, argv, "c:f:t:s:S:E:L:r:x::X::bdDhRrJ")) != -1) {
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
					perror("popen");
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
			else if (strcmp(optarg, "cf") == 0){
				buf[0] = 0x0;
				strcpy(boot_dev, optarg);
				/* get cf label */
				if((fp = popen("/sbin/blkid -l -t LABEL=/", "r")) == NULL){
					perror("popen");
					return ERROR_END;
				}
				fgets(buf, sizeof(buf)-1, fp);
				pclose(fp);
				if(buf[0] == 0){
					printf("ERROR: disk label is not '/' on CF\n");
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
			if(flash_set_bootdev(i, boot_dev) < 0){
				fprintf(stderr, "Fail to Boot device change\n");
				return ERROR_END;
			}
			fprintf(stderr, "Boot device change to %s\n", optarg);
			return NORMAL_END;

		case 'f':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(flash_prog_kern(optarg) < 0){
				fprintf(stderr, "Fail to kernel image change\n");
				return ERROR_END;
			}
			fprintf(stderr, "done\n");
			return NORMAL_END;

		case 's':
		case 'S':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(flash_save_param(i, optarg) < 0){
				fprintf(stderr, "Fail to environment change\n");
				return ERROR_END;
			}
			fprintf(stderr, "done\n");
			return NORMAL_END;

		case 'x':
		case 'X':
#ifdef DEBIAN
			return 0;
#else
			if(flash_extract_param(i) < 0){
				fprintf(stderr, "Fail to environment extract\n");
				return ERROR_END;
			}
			return NORMAL_END;
#endif

		case 'd':
		case 'D':
			if(flash_delete_param(i) < 0){
				fprintf(stderr, "Fail to environment delete\n");
				return ERROR_END;
			}
			fprintf(stderr, "done\n");
			return NORMAL_END;

		case 'b':
		case 'E':
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(flash_set_bootdev(i, optarg) < 0){
				fprintf(stderr, "Fail to Save params\n");
				return ERROR_END;
			}
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
#ifdef DEBUG
			fprintf(stderr, "option %c: arg = %s\n", i, optarg);
#endif
			if(flash_delete_param(i) < 0){
				fprintf(stderr, "Fail to Delete params\n");
				return ERROR_END;
			}
			fprintf(stderr, "done\n");
			return NORMAL_END;

		case 'h':
		default:
	//		usage();
			break;
		}
	}
#if 0
	if (argc != 1) {
		usage();
		return (0);
	}
#endif
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
	erase_info_t erase;
	int env_size = ENV_SIZE;
	unsigned short *verify;

	read_proc_mtd();

	if ((ofd = open(mtdname[MTD_UBOOTENV], O_RDWR)) < 0) {
		perror(mtdname[MTD_UBOOTENV]);
		return -1;
	}
	if(offset){
		if((st = lseek(ofd, offset, SEEK_SET)) != offset){
			close(ofd);
			perror("lseek");
#ifdef DEBUG
			printf("seek=%x\n", st);
#endif
			return -1;
		}
	}

	if ((vfd = open(mtdname[MTD_UBOOTENV], O_RDONLY)) < 0) {
		perror(mtdname[MTD_UBOOTENV]);
		close(ofd);
		return -1;
	}
	if(offset){
		if((st = lseek(vfd, offset, SEEK_SET)) != offset){
			close(ofd);
			close(vfd);
			perror("lseek");
#ifdef DEBUG
			printf("seek=%x\n", st);
#endif
			return -1;
		}
	}

	if ((verify = malloc(SECT_SIZE)) == NULL) {
		perror("malloc");
		close(ofd);
		close(vfd);
		return -1;
	}

	erase.start = offset;
	erase.length = SECT_SIZE;
	i=0;
//	fprintf(stderr, "Load u-boot environment to FlashROM\n");
	while (env_size > 0) {
		if(env_size < SECT_SIZE)
			nread = env_size;
		else if(env_size >= SECT_SIZE)
			nread = SECT_SIZE;

		retry = 2;
RETRY:
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
			perror("Erase failure");
			ret = -1;
			break;
		}
		if (write(ofd, env, nread) < 0) {
			perror("mtd");
			ret = -1;
			break;
		}
		if(read(vfd, verify, nread) != nread){
			perror("verify");
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
					perror("lseek");
					ret = -1;
					break;
				}
				if(lseek(vfd, -nread, SEEK_CUR) < 0){
					perror("lseek");
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
		env_size -= nread;
		if(i>=STATUS_COL) {
			fprintf(stderr, "\n");
			i=0;
		}
	}
//	fprintf(stderr, "\n");

	free(verify);
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

	if((fp = fopen(mtdname[MTD_UBOOTENV], "r")) == NULL){
		perror(mtdname[MTD_UBOOTENV]);
		return -1;
	}
	/* read area1 */
	if(fread(env_area1, sizeof(env_area1), 1, fp) != 1){
		perror("fread");
		fclose(fp);
		return -1;
	}
	if(fseek(fp, ENV_AREA >> 1, SEEK_SET) != 0){
		perror("fseek");
		fclose(fp);
		return -1;
	}
	/* read area2 */
	if(fread(env_area2, sizeof(env_area2), 1, fp) != 1){
		perror("fread");
		fclose(fp);
		return -1;
	}
	fclose(fp);

	crc1 = crc32(0, (unsigned char*)&env_area1[OFFSET], ENV_SIZE-OFFSET);
	crc2 = crc32(0, (unsigned char*)&env_area2[OFFSET], ENV_SIZE-OFFSET);

	if(!crc1 && !crc2){			/* Invalid both */
		fprintf(stderr, "ERROR: environment area is empty\n");
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
		fprintf(stderr, "ERROR: environment area is empty\n");
		return -1;
	}

	wk_prev = p_prev;
	wk_now = p_now;
	if(target == 'c'){	/* change boot device */
		/* previous environment -> now environment */
		while(wk_prev < p_prev + (ENV_SIZE-OFFSET)){
			if((p=strstr(wk_prev, "_std_script="))){		/* found "_std_script=" */
				sprintf(buf, "_std_script=%s",
					(strcmp("cf", arg) ? "_fl_boot" : "_cf_boot"));
				if(wk_now - p_now > strlen(buf)){
					strcpy(wk_now, buf);
					wk_now += strlen(buf) + 1;
				}
				else{
					fprintf(stderr, "ERROR: bootargs string is too long\n");
					return -1;
				}
			}
			else if((p=strstr(wk_prev, "rootdev="))){		/* found "rootdev=" */
				if(strcmp("ram", arg) == 0)
					strcpy(buf, "rootdev=/dev/ram");
				else if(strncmp("sd", arg, 2) == 0)
					strcpy(buf, "rootdev=LABEL=/");
				else	/* arg = "cf" */
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
			perror("fopen");
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
	else if(target == 'b'){	/* print current boot setting */
		char std_script[32], rootdev[32];

		std_script[0] = 0x0; rootdev[0] = 0x0;
		while(wk_prev < p_prev + (ENV_SIZE-OFFSET)){
			char *p;

			if((p=strstr(wk_prev, "_std_script="))){		/* found "_std_script=" */
				memcpy(std_script, p, sizeof(std_script));
			}
			else if((p=strstr(wk_prev, "rootdev="))){		/* found "rootdev=" */
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
						perror("popen");
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
			else if(strstr(std_script, "_cf_boot")){
				printf("current boot device: cf\n");
			}
		}
		return 0;
	}
	crc1 = crc32(0, (unsigned char*)&p_now[OFFSET], ENV_SIZE-OFFSET);
	memcpy(p_now, &crc1, sizeof(crc1));

	/* set flag */
	p_now[4] = 0x1;		/* active */
	p_prev[4] = 0x0;	/* obsolete */
#if 1
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
#endif
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
	erase_info_t erase;
	struct stat sb;
	int sector_len;
	unsigned short *verify;
	unsigned long	crc;
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
#if 1	/* Add CRC check */
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
	if(crc != pboot_block->imgcrc){
		sprintf(buf, "ERROR%d: kernel image is CRC error(%08lx)\n", __LINE__, crc);
		flash_write_log(buf);
		free(membase);
		close(ifd);
		ret = -1;
		goto err_exit;
	}
	free(membase);
	lseek(ifd, 0, SEEK_SET);
#endif

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
	
	if (pboot_block->magic != MAGIC) {
		sprintf(buf, "ERROR%d: file isn't kernel image(%08lx)\n", __LINE__, pboot_block->magic);
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
	i=0;
	lseek(ifd, 0, SEEK_SET);
	fprintf(stderr, "Load boot image to FlashROM\n");
	while ((nread = read(ifd, membase, sector_len)) > 0) {
		total += nread;
		retry = 2;
RETRY:
		fprintf(stderr, ".");
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
			sprintf(buf, "ERROR%d: erase error(%lx)\n", __LINE__, total);
			flash_write_log(buf);
			ret = -1;
			break;
		}
		fprintf(stderr, "%c#",0x08);
		if (write(ofd, membase, nread) < 0) {
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
	erase_info_t erase;
	int maxsize;
	unsigned short *verify;
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
		execl(tarpath, "tar", "cvpTzf", list, localbuf, NULL);
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
		perror(localbuf);
		unlink(localbuf);
		return -1;
	}

	if (fstat(ifd, &sb) < 0) {
		perror("stat");
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
		perror("malloc");
		close(ifd);
		unlink(localbuf);
		return -1;
	}

	if ((ofd = open(targetdevice , O_RDWR)) < 0) {
		perror(targetdevice);
		free(membase);
		close(ifd);
		unlink(localbuf);
		return -1;
	}

	if ((vfd = open(targetdevice, O_RDONLY)) < 0) {
		perror(targetdevice);
		free(membase);
		close(ifd);
		close(ofd);
		unlink(localbuf);
		return -1;
	}

	if ((verify = malloc(SECT_SIZE)) == NULL) {
		perror("malloc");
		free(membase);
		close(ifd);
		close(ofd);
		close(vfd);
		unlink(localbuf);
		return -1;
	}

	erase.start = 0;
	erase.length = SECT_SIZE;
	i=0;
	fprintf(stderr, "Save files to FlashROM\n");
	while ((nread = read(ifd, membase, SECT_SIZE)) > 0) {
		retry = 2;
RETRY:
		fprintf(stderr, ".");
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
			perror("Erase failure");
			ret = -1;
			break;
		}
		fprintf(stderr, "%c#",0x08);
		if (write(ofd, membase, nread) < 0) {
			perror("mtd");
			ret = -1;
			break;
		}

		fprintf(stderr, "%c*",0x08);
		if(read(vfd, verify, nread) != nread){
			perror("verify");
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
					perror("lseek");
					ret = -1;
					break;
				}
				if(lseek(vfd, -nread, SEEK_CUR) < 0){
					perror("lseek");
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

int
flash_extract_param(int target)
{
	int pid, st;
	char targetdevice[256], buf[256];
	int ret=NORMAL_END;

	if (target == 'x') {
		strcpy(targetdevice, mtdname[MTD_USRCONF]);
	} else if (target == 'X') {
		strcpy(targetdevice, mtdname[MTD_USRDATA]);
	} else {
		return -1;
	}

	if ((pid = fork()) == 0) {
		/* in child */
		if(strcmp(tarpath, tarssd) == 0){
			ret = execl(tarpath, "tar", "--warning=no-timestamp",
					"-xvpzf", targetdevice, "-C", "/", NULL);
		}
		else{
			sprintf(buf, "gunzip < %s | tar xvpf - -C /", targetdevice);
			ret = execl("/bin/sh", "sh", "-c", buf, NULL);
		}

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
	return ret;
}

int flash_delete_param(int target)
{
	int ofd, i;
	erase_info_t erase;
	char targetdevice[256];
	int maxsize;

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
	} else {
		return -1;
	}


	if ((ofd = open(targetdevice, O_RDWR)) < 0) {
		perror(targetdevice);
		close(ofd);
		return -1;
	}

	erase.start = 0;
	erase.length = SECT_SIZE;
	fprintf(stderr, "Delete saved files from FlashROM\n");
	for (i = 0; i < maxsize/SECT_SIZE; i++) {
		fprintf(stderr, ".");
		fflush(stderr);
		if (ioctl(ofd, MEMERASE, &erase) != 0) {
			perror("Erase failure");
			close(ofd);
			return -1;
		}
		erase.start += SECT_SIZE;
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
		perror("fopen mtd");
		return -1;
	}
	memset(buf, 0x0, sizeof(buf));
	if(fread(buf, sizeof(buf), 1, fp) != 1){
		perror("fread");
		return -1;
	}
	if(buf[4] == 0x0){			/* area1 is absolute */
		char buf2[ENV_SIZE];

		if(fseek(fp, ENV_AREA >> 1, SEEK_SET) != 0){
			perror("fseek");
			fclose(fp);
			return -1;
		}
		if(fread(buf2, sizeof(buf2), 1, fp) != 1){
			perror("fread");
			return -1;
		}
		if(buf2[4] == 0x1){		/* area2 is active */
			memcpy(buf, buf2, sizeof(buf));
		}
	}
	fclose(fp);

	/* Write local file */
	if((fp = fopen(fname, "w")) == NULL){
		perror("fopen");
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
		perror("/proc/mtd");
		return (1);
	}

	while (fscanf(ifp, "%s", buf) != EOF) {
		if (strncmp(buf, "mtd2:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			USER_SIZE = strtol(buf, NULL, 16);
			fscanf(ifp, "%s", buf);
			SECT_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd0:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			KERNEL_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd1:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			PARAM_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd5:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			MPARAM_SIZE = strtol(buf, NULL, 16);
		} else if (strncmp(buf, "mtd6:", sizeof(char) * 5) == 0) {
			fscanf(ifp, "%s", buf);
			MONITOR_SIZE = strtol(buf, NULL, 16);
		}
	}
	fclose(ifp);
#if 0
	printf("SECT_SIZE:    %d\n", SECT_SIZE);
	printf("MONITOR_SIZE: %d\n", MONITOR_SIZE);
	printf("PARAM_SIZE:   %d\n", PARAM_SIZE);
	printf("USER_SIZE:    %d\n", USER_SIZE);
	printf("KERNEL_SIZE:  %d\n", KERNEL_SIZE);
	printf("MPARAM_SIZE:  %d\n", MPARAM_SIZE);
#endif
	return (0);
}

#if 1
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

	fprintf(stderr, "%s%s", buf, s);
	if(fp_log){
		time(&tm);
		strftime(buf, sizeof(buf)-1, "%c", localtime(&tm));
		fprintf(fp_log, "%s %s", buf, s);
	}
}
#endif

#if 1	/* support for /dev/mtd[0-6] */
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
#endif
