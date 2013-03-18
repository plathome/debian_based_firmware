/*	$ssdlinux: obs600_pshd.c,v 1.6 2012/10/16 07:00:27 shimura Exp $	*/
/*
 * Push SW deamon
 */

#include <sys/types.h>
#include <asm/ioctls.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#if defined(HAVE_PUSHSW_OBS600_H)
#include <asm/pushsw_obs600.h>
#elif defined(HAVE_PUSHSW_OBSAXX_H)
#include <linux/obspushsw.h>
#endif

#define PID_FILE "/var/run/pshd.pid"

void donothing(int i);
void die(int i);

#ifdef DEBUG
#define PSW_DEBUG(str...)       printf ("pshd: " str)
#else
#define PSW_DEBUG(str...)       /* nothing */
#endif

#define MIN_SEC		1
#define MAX_SEC		3600
#define INTERVAL	200 * 1000	// 200ms(5 times a second)
#define SEGLED_PID	"/var/run/segled.pid"
#define SEGLED_DEV	"/dev/segled"

static int flag		= 1;		// exit() flag
static int reboot	= 1 * 5;	// reboot time(default = 1 sec)
static int halt		= 5 * 5;	// shutdown time(default = 5 sec)
static int wait		= 0;		// wait time

/* some variables used in getopt (3) */
extern char *optarg;

void usage(void)
{
	fprintf(stderr, "pshd [-r sec] [-h sec] [-t sec]\n");
	fprintf(stderr, "-r sec\tsecond to reboot\n");
	fprintf(stderr, "-h sec\tsecond to shutdown\n");
	fprintf(stderr, "-t sec\twait time before execute\n");
	fprintf(stderr, "default:\t-r 1\n");
	fprintf(stderr, "\t\t-h 5\n");
	fprintf(stderr, "\t\t-t 0(=now)\n");
	fprintf(stderr, "%d <= sec <= %d & reboot < halt\n", MIN_SEC, MAX_SEC);
	fprintf(stderr, "\n");
	fprintf(stderr, "* Orange LED (Upper) turns on when time up for reboot\n");
	fprintf(stderr, "* Green LED (Middle) turns on in the middle of reboot and halt\n");
	fprintf(stderr, "* Red LED (Lower) turns on when time up for halt\n");
}

static inline void flash_led(int fd, char* num)
{
	write(fd, num, 1);
}

void watch_pushsw(void)
{
	FILE *fp;
	int fd, ledfd=0, rv;
	int count = 0;		// time of push INIT switch
	char buf[16];

	if ((fd = open("/dev/pushsw", O_RDONLY | O_NONBLOCK)) < 0) {
		perror("open");
		exit(-1);
	}
	while(flag){
		rv = ioctl(fd, PSWIOC_GETSTATUS, NULL);
#ifdef DEBUG
printf("stat=%08x\n", rv);
#endif
		if (rv < 0) {
			perror("blocked");
			exit(-1);
		}
		else if(rv){	/* INIT switch pushed */
			count++;
			if(count >= reboot){
				/* kill runled daemon */
				if((fp = fopen(SEGLED_PID, "r")) != NULL){
					if(fgets(buf, sizeof(buf)-1, fp) != NULL){
						PSW_DEBUG("runled pid=%d\n", atoi(buf));
						kill(atoi(buf), SIGTERM);
					}
				}
				else{
					perror(SEGLED_PID);
					fprintf(stderr, "pshd can't control LED\n");
				}
				if ((ledfd = open(SEGLED_DEV, O_RDWR)) < 0){
					perror(SEGLED_DEV);
					fprintf(stderr, "pshd can't control LED\n");
				}
			}
			//count++;
			if(count >= reboot && count < halt){
				if(ledfd > 0)
#if defined(HAVE_PUSHSW_OBS600_H)
					flash_led(ledfd, "1");
#elif defined(HAVE_PUSHSW_OBSAXX_H)
					flash_led(ledfd, "4");
#endif

				if(count >= ((halt - reboot) / 2)){
					if(ledfd > 0)
						flash_led(ledfd, "2");
				}
			}
			else if(count >= halt){
				if(ledfd > 0)
#if defined(HAVE_PUSHSW_OBS600_H)
					flash_led(ledfd, "4");
#elif defined(HAVE_PUSHSW_OBSAXX_H)
					flash_led(ledfd, "1");
#endif
			}
		}
		else{		/* INIT switch release */
			if(wait)
				sprintf(buf, "%d", wait);
			else
				strcpy(buf, "now");
			if(count >= reboot && count < halt){
				PSW_DEBUG("reboot ON\n");
				execl("/sbin/shutdown", "shutdown", "-r", buf, NULL);
			}
			else if(count >= halt){
				PSW_DEBUG("halt ON\n");
				execl("/sbin/shutdown", "shutdown", "-h", buf, NULL);
			}

			if(count){
				if(ledfd > 0)
					close(ledfd);
			}
			count=0;
		}
		usleep(INTERVAL);
	}
	close(fd);
}

int main(int argc, char *argv[])
{
	int fd;
	int pid;
	int i;

	if (getuid()) {
		fprintf(stderr, "must run super user\n");
		return 1;
	}

	while ((i = getopt(argc, argv, "r:h:t")) != -1) {
		switch (i) {
		case 'r':
			reboot = atoi(optarg) < MAX_SEC ? atoi(optarg) * 5 : MAX_SEC * 5;
			if(!reboot) reboot++;	// 0 is no use
			break;
		case 'h':
			halt = atoi(optarg) < MAX_SEC ? atoi(optarg) * 5 : MAX_SEC * 5;
			if(!halt) halt++;	// 0 is no use
			break;
		case 't':
			wait = atoi(optarg) < MAX_SEC ? atoi(optarg) * 5 : MAX_SEC * 5;
			if(!wait) wait++;	// 0 is no use
			break;
		default:
			usage();
			return (0);
		}
	}

	if (halt < reboot) {
		fprintf(stderr, "Please set it so that halt bigger than reboot.\n");
		return(1);
	}

	if ((halt - reboot) < (2 * 5)) {
		// for Green LED
		fprintf(stderr, "Please add differences more than two seconds to reboot and halt.\n");
		return(1);
	}

	if ((pid = fork())) {
		/* parent */
		char tmp[100];
		if ((fd = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) < 0) {
			perror("open");
			exit(-1);
		}
		sprintf(tmp, "%d\n", pid);
		if (write(fd, tmp, strlen(tmp)) != strlen(tmp)) {
			perror("write");
			close(fd);
			exit(-2);
		}
		close(fd);
		return 0;
	} else {
#ifndef DEBUG
		/* daemon */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		if(setsid() == -1) {
			exit(4);
		}

		/* child */
		signal(SIGHUP,donothing);
		signal(SIGINT,die);
		signal(SIGQUIT,die);
		signal(SIGILL,die);
		signal(SIGTRAP,die);
		signal(SIGABRT,die);
		signal(SIGIOT,die);
		signal(SIGBUS,die);
		signal(SIGFPE,die);
		signal(SIGUSR1,die);
		signal(SIGSEGV,die);
		signal(SIGUSR2,die);
		signal(SIGPIPE,die);
		signal(SIGALRM,die);
		signal(SIGTERM,die);
		signal(SIGCHLD,die);
		signal(SIGCONT,die);
		signal(SIGSTOP,die);
		signal(SIGTSTP,die);
		signal(SIGTTIN,die);
		signal(SIGTTOU,die);
		signal(SIGURG,die);
		signal(SIGXCPU,die);
		signal(SIGXFSZ,die);
		signal(SIGVTALRM,die);
		signal(SIGPROF,die);
		signal(SIGWINCH,die);
		signal(SIGIO,die);
		signal(SIGPWR,die);
		signal(SIGSYS,die);

		watch_pushsw();

		exit(0);
	}
}

void donothing(int i) {
}
void die(int i) {
	flag = 0;
}

