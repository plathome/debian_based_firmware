/*	$ssdlinux: runled.c,v 1.12 2013/04/12 08:48:43 shimura Exp $	*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <errno.h>
#include <linux/version.h>
#include <syslog.h>

extern int errno;

#define PID_FILE "/var/run/segled.pid"

#define LED_0_33	(500*1000)	/* CPU  0% -  33% */
#define LED_34_66	(250*1000)	/* CPU 34% -  66% */
#define LED_67_100	(125*1000)	/* CPU 67% - 100% */

#define PM_INTVL		10	/* check temperature intervall sec */
#ifdef CONFIG_OBSAX3
#ifdef CONFIG_LINUX_3_11_X
#define TEMP_INPUT		"/sys/devices/virtual/thermal/thermal_zone0/hwmon0/temp1_input"
#define SEGLED_DEV_G	"/sys/class/leds/green_led/brightness"
#define SEGLED_DEV_Y	"/sys/class/leds/yellow_led/brightness"
#define SEGLED_DEV_R	"/sys/class/leds/red_led/brightness"
#else
#define TEMP_INPUT		"/sys/devices/platform/axp-temp.0/temp1_input"
#endif
#define CPU_ONLINE		"/sys/devices/system/cpu/cpu%d/online"
#define PM_TEMP_MAX	105 * 1000
#define PM_TEMP_MIN	10  * 1000
#define PM_TEMP_UP	90  * 1000
#define PM_CPU_UP	1
#define PM_CPU_DONW 0

static int PM_CTRL_CPU = 0;
static int PM_DOWN_CPU = PM_TEMP_MAX;
static int PM_UP_CPU = PM_TEMP_UP;
#endif

#ifdef DEBUG
#define LED_DEBUG(str...)       printf ("runled: " str)
#else
#define LED_DEBUG(str...)       /* nothing */
#endif

void donothing(int i){}
void die(int i){exit(0);}

static int led_speed = LED_0_33;
static int spdctl= 1;
static int prevuse[2];

/* some variables used in getopt (3) */
extern char *optarg;
extern int optind;
extern int optopt;
extern int opterr;
extern int optreset;

/*
	Read /proc/stat and get CPU used time
*/
int get_usetime(int* use){
	FILE *fp;
	char str[128];
	char *strend;
	char *sta, *end;
	char buf[32];

	if((fp = fopen("/proc/stat", "r")) == NULL){
		return 0;
	}
	if(fgets(str, sizeof(str) - 1, fp) == NULL){
		fclose(fp);
		return 0;
	}
	fclose(fp);

	/* get end of string */
	strend = str + strlen(str);

	/*********************************************
		      arg1 arg2 arg3 arg4 arg5
		str = cpu  xxxx xxxx xxxx xxxx xxxx ...
	**********************************************/
	sta = str;
	/* search top */
	while(*sta != ' ' && *sta != '\t'){
		sta++;
		if(sta >= strend)
			return 0;
	}

	/* search arg2 start */
	while(*sta == ' ' || *sta == '\t'){
		sta++;
		if(sta >= strend)
			return 0;
	}
	/* search arg2 end */
	end = sta;
	while(*end != ' ' && *end != '\t'){
		end++;
		if(sta >= strend)
			return 0;
	}
	/* get arg2 */
	strncpy(buf, sta, end-sta);
	buf[end-sta] = 0x0;
	use[0] = atoi(buf);

	sta = end;
	/* search arg3 start */
	while(*sta == ' ' || *sta == '\t'){
		sta++;
		if(sta >= strend)
			return 0;
	}
	/* search arg3 end */
	end = sta;
	while(*end != ' ' && *end != '\t'){
		end++;
		if(sta >= strend)
			return 0;
	}
	/* get arg3 */
	strncpy(buf, sta, end-sta);
	buf[end-sta] = 0x0;
	use[0] += atoi(buf);

	sta = end;
	/* search arg4 start */
	while(*sta == ' ' || *sta == '\t'){
		sta++;
		if(sta >= strend)
			return 0;
	}
	/* search arg4 end */
	end = sta;
	while(*end != ' ' && *end != '\t'){
		end++;
		if(sta >= strend)
			return 0;
	}
	/* get arg4 */
	strncpy(buf, sta, end-sta);
	buf[end-sta] = 0x0;
	use[0] += atoi(buf);

	sta = end;
	/* search arg5 start */
	while(*sta == ' ' || *sta == '\t'){
		sta++;
		if(sta >= strend)
			return 0;
	}
	/* search arg5 end */
	end = sta;
	while(*end != ' ' && *end != '\t'){
		end++;
		if(sta >= strend)
			return 0;
	}
	/* get arg5 */
	strncpy(buf, sta, end-sta);
	buf[end-sta] = 0x0;
	use[1] = use[0] + atoi(buf);

	return 1;
}

/*
	Calc LED Sleep Time
*/
void calc_ledspeed(void){
	int tempuse[2];
	int total;

	if(!get_usetime(tempuse)){
		led_speed = LED_0_33;
		return;
	}

	/* calc used */
	total = tempuse[0] - prevuse[0];
	if(total)
		total = total / (tempuse[1]-prevuse[1]) * 100;

	if(total < 34){
		if(led_speed != LED_0_33){
			led_speed = LED_0_33;
			LED_DEBUG("change led speed 500ms(total=%f)\n", total);
		}
	}
	else if(total >= 34 && total < 67){
		if(led_speed != LED_34_66){
			led_speed = LED_34_66;
			LED_DEBUG("change led speed 250ms(total=%f)\n", total);
		}
	}
	else{
		if(led_speed != LED_67_100){
			led_speed = LED_67_100;
			LED_DEBUG("change led speed 125ms(total=%f)\n", total);
		}
	}
	memcpy(prevuse, tempuse, sizeof(prevuse));
}

#ifdef CONFIG_OBSAX3
int get_temp(void)
{
	FILE *fp;
	char buf[128];
	int now;

	if((fp = fopen(TEMP_INPUT, "r")) == NULL){
		printf("runled%d: %s\n", __LINE__, strerror(errno));
		return -1;
	}
	if(fgets(buf, sizeof(buf), fp) == NULL){
		printf("runled%d: %s\n", __LINE__, strerror(errno));
		fclose(fp);
		return -1;
	}
	fclose(fp);

	now = atoi(buf);
	return now;
}

void ctrl_cpu(int temp)
{
	static int prev=PM_TEMP_MIN;
	FILE *fp;
	char buf[128], val[8];
	int i;

	if(temp > PM_DOWN_CPU && prev <= PM_DOWN_CPU){
		strcpy(val, "0");
#if 0
		openlog("runled", LOG_CONS | LOG_PID, LOG_USER);
		syslog(LOG_INFO, "temperature: %d degrees, shutdown CPU core other than CPU0\n", temp);
		closelog();
#endif
	}
#ifdef CONFIG_LINUX_3_2_X
	else if(temp < PM_UP_CPU && prev >= PM_UP_CPU){
		strcpy(val, "1");
#if 0
		openlog("runled", LOG_CONS | LOG_PID, LOG_USER);
		syslog(LOG_INFO, "temperature: %d degrees, bootup CPU core other than CPU0\n", temp);
		closelog();
#endif
	}
#endif
	else{
		prev = temp;
		return;
	}

	for(i=1; i<4; i++){	// start i = 1
		sprintf(buf, CPU_ONLINE, i);
		if((fp = fopen(buf, "w")) == NULL)
			break;
		fputs(val, fp);
		fclose(fp);
	}
	prev = temp;
}
#endif

void
dancer()
{
	int fd;
#if defined(CONFIG_OBSAX3)
	int	temp;
	time_t t;
	time_t prev = time(NULL) + PM_INTVL;
#endif

	for (;;) {
#ifdef CONFIG_LINUX_3_11_X
		if ((fd = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "1", 1);
		close(fd);
#else
		if ((fd = open("/dev/segled", O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "1", 1);
		close(fd);
#endif
		usleep(led_speed);
#ifdef CONFIG_LINUX_3_11_X
		if ((fd = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "1", 1);
		close(fd);
#else
		if ((fd = open("/dev/segled", O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "2", 1);
		close(fd);
#endif
		usleep(led_speed);
#ifdef CONFIG_LINUX_3_11_X
		if ((fd = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "1", 1);
		close(fd);
#else
		if ((fd = open("/dev/segled", O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "4", 1);
		close(fd);
#endif
		usleep(led_speed);
#ifdef CONFIG_LINUX_3_11_X
		if ((fd = open(SEGLED_DEV_Y, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_R, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "0", 1);
		close(fd);
		if ((fd = open(SEGLED_DEV_G, O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "1", 1);
		close(fd);
#else
		if ((fd = open("/dev/segled", O_RDWR)) < 0) {
			perror("open");
			exit(-1);
		}
		write(fd, "2", 1);
		close(fd);
#endif
		usleep(led_speed);
		if(spdctl)
			calc_ledspeed();	/* control LED speed */

#if defined(CONFIG_OBSAX3)
		t = time(NULL);
		if(!prev || t >= prev){
			prev = t + PM_INTVL;
			if((temp = get_temp()) == -1)
				exit(-1);
			if(PM_CTRL_CPU)
				ctrl_cpu(temp);
		}
#endif
	}
}

void usage(void)
{
#ifdef CONFIG_OBSAX3
#ifdef CONFIG_LINUX_3_2_X
	fprintf(stderr, "runled [-ds] [-l downtemp] [-m uptemp]\n");
#else
	fprintf(stderr, "runled [-ds] [-l downtemp]\n");
#endif
#else
	fprintf(stderr, "runled [-s]\n");
#endif

	fprintf(stderr, "option:\n");

#ifdef CONFIG_OBSAX3
	fprintf(stderr, "\t-d : control cpu core\n");
	fprintf(stderr, "\t-l : cpu core DOWN temperature\n");
#ifdef CONFIG_LINUX_3_2_X
	fprintf(stderr, "\t-m : cpu core UP temperature\n");
#endif
#endif
	fprintf(stderr, "\t-s : cancel LED speed control\n");
}

int
main(int argc, char *argv[])
{
	int fd;
	int pid;
	int i;

	if (getuid()) {
		fprintf(stderr, "must be super user\n");
		return 1;
	}

#ifdef CONFIG_OBSAX3
	if(argc == 1){
		usage();
		return (0);
	}
#endif

#ifdef CONFIG_OBSAX3
#ifdef CONFIG_LINUX_3_2_X
	while ((i = getopt(argc, argv, "l:m:ds")) != -1) {
#else
	while ((i = getopt(argc, argv, "l:ds")) != -1) {
#endif
#else
	while ((i = getopt(argc, argv, "s")) != -1) {
#endif
		switch (i) {
#ifdef CONFIG_OBSAX3
		case 'd':	// control CPU up/down
			PM_CTRL_CPU = 1;
			break;
		case 'l':	// CPU down temp
			PM_DOWN_CPU = atoi(optarg) * 1000;
			break;
#ifdef CONFIG_LINUX_3_2_X
		case 'm':	// CPU up temp
			PM_UP_CPU = atoi(optarg) * 1000;
			break;
#endif
#endif
		case 's':	// cancel LED speed control
			spdctl = 0;
			break;
		default:
			usage();
			return (0);
		}
	}

#ifdef CONFIG_OBSAX3
#ifdef CONFIG_LINUX_3_2_X
	if(PM_DOWN_CPU > PM_TEMP_MAX || PM_UP_CPU < PM_TEMP_MIN || PM_DOWN_CPU <= PM_UP_CPU){
#else
	if(PM_DOWN_CPU > PM_TEMP_MAX){
#endif
		printf("runled%d: Invalid CPU control temperature\n", __LINE__);
		return -1;
	}
#endif

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
		/* daemon */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		if(setsid() == -1)
			exit(4);

		/* child */
		signal( SIGHUP,donothing);
		signal( SIGINT,die);
		signal( SIGQUIT,die);
		signal( SIGILL,die);
		signal( SIGTRAP,die);
		signal( SIGABRT,die);
		signal( SIGIOT,die);
		signal( SIGBUS,die);
		signal( SIGFPE,die);
		signal( SIGKILL,die);
		signal( SIGUSR1,die);
		signal( SIGSEGV,die);
		signal( SIGUSR2,die);
		signal( SIGPIPE,die);
		signal( SIGALRM,die);
		signal( SIGTERM,die);
		signal( SIGSTKFLT,die);
		signal( SIGCHLD,die);
		signal( SIGCONT,die);
		signal( SIGSTOP,die);
		signal( SIGTSTP,die);
		signal( SIGTTIN,die);
		signal( SIGTTOU,die);
		signal( SIGURG,die);
		signal( SIGXCPU,die);
		signal( SIGXFSZ,die);
		signal( SIGVTALRM,die);
		signal( SIGPROF,die);
		signal( SIGWINCH,die);
		signal( SIGIO,die);
		signal( SIGPWR,die);
		signal( SIGSYS,die);
		dancer();
	}
	return 0;
}
