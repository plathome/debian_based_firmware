/*	$ssdlinux: pshd_bx1.c,v 0.01 2014/01/07 07:19:59 yamagata Exp $	*/
/*
 * Copyright (c) 2009-2018 Plat'Home CO., LTD.
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <sys/epoll.h>
#include <errno.h>
#include <linux/version.h>

#if 0
void donothing(int);
void die(int);
#endif

#define PID_FILE	"/var/run/pshd.pid"
#if defined(CONFIG_OBSVX1) || defined(CONFIG_OBSIX9)
#define INITSW		"/sys/class/gpio/gpio345/value"
#elif defined(CONFIG_OBSGEM1)
#define INITSW     "/sys/class/gpio/gpio466/value"
#else
#define INITSW		"/sys/class/gpio/gpio14/value"
#endif
#define READ_PATH	"/tmp/.pshd"
#define REBOOT		2 * 1000
#define HALT		3 * 1000	/* HALT 5 sec (REBOOT + 3) */
#define RED		1
#define YELLOW	3

#if 0
#define SEGLED_DEV_R	"/sys/class/gpio/gpio47/value"
#define SEGLED_DEV_G	"/sys/class/gpio/gpio48/value"
#define SEGLED_DEV_B	"/sys/class/gpio/gpio49/value"
#endif

#ifdef DEBUG
#define D(...) {printf("%d:", __LINE__); printf(__VA_ARGS__);}
#else
#define D(...)
#endif

static int flag = 1;	// exit() flag

#if 0
void setLED(char* b, char* g, char* r)
{
	int fd;

	if ((fd = open(SEGLED_DEV_R, O_RDWR)) < 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}
	write(fd, r, 1);
	close(fd);
	if ((fd = open(SEGLED_DEV_G, O_RDWR)) < 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}
	write(fd, g, 1);
	close(fd);
	if ((fd = open(SEGLED_DEV_B, O_RDWR)) < 0) {
		printf("%d: %s\n", __LINE__, strerror(errno));
		exit(-1);
	}
	write(fd, b, 1);
	close(fd);
}
#endif

void kill_runled(void)
{
	int fd, ret;
	char buf[16];

	if((fd = open("/var/run/segled.pid", O_RDONLY)) == -1){
		return;
	}
	if((ret = read(fd, buf, sizeof(buf)-1)) == -1){
		return;
	}
	buf[ret] = 0x0;
	
	kill(strtol(buf, NULL, 10), SIGTERM);
}

int getINITSW(int epfd, int fd, int tout, int *val)
{
	struct epoll_event events;
	int ret;
	char c;

	ret = epoll_wait(epfd, &events, 1, tout);
	if(ret == -1){
		return -1;
	}
	else if(ret == 0)
		return 0;

	if(lseek(fd, 0, SEEK_SET) == -1){
		return -1;
	}

	if(read(fd, &c, 1) == -1){
		return -1;
	}

	if(c != '0' && c != '1'){
		return -1;
	}

	*val = (int)c - 0x30;
	return 1;
}

void system_exec(void) {

	FILE *fp ;
	char buf[1024] ;

	memset(buf , 0x00 , sizeof(buf));

	if ( (fp = fopen(READ_PATH, "r"))  == NULL ) {
		return ;
	}

	if ( fgets(buf , sizeof(buf) , fp) == NULL ) {
		fclose(fp) ;
		return ;
	}

	fclose(fp) ;

	system(buf);

	return ;
}

void watch_pushsw(void)
{
	struct epoll_event ev;
	int fd, epfd, val, ret;

	if((epfd = epoll_create(1)) == -1){
//D("%d\n", __LINE__);
		return;
	}
	if((fd = open(INITSW, O_RDWR | O_NONBLOCK)) == -1){
//D("%d\n", __LINE__);
		close(epfd);
		return;
	}

	memset(&ev, 0x0, sizeof(struct epoll_event));
	ev.events = EPOLLPRI;
	ev.data.fd = fd;

	if(epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1){
//D("%d\n", __LINE__);
		return;
	}
	while(flag){
//D("%d\n", __LINE__);
		ret = getINITSW(epfd, fd, -1, &val);
//D("ret=%d val=%d\n", ret, val);
		if(ret == 0){
			continue;
		}
		else if(ret == 1 && val == 0){
			ret = getINITSW(epfd, fd, REBOOT, &val);
//D("ret=%d val=%d\n", ret, val);
			if(ret == 1 || ret == -1){
//D("ret=%d\n", ret);
				continue;
			}
			//kill_runled();
			//setLED("0", "1", "1");
//D("%d\n", __LINE__);
			ret = getINITSW(epfd, fd, HALT, &val);
//D("ret=%d val=%d\n", ret, val);
			if(ret == 1 && val == 1){
//D("REBOOT!!\n");
				system_exec();
			}
			else if(ret == 0){
				//setLED("0", "0", "1");
			}

			val = 0;
			do{
//D("%d\n", __LINE__);
				ret = getINITSW(epfd, fd, 1000, &val);
//D("ret=%d val=%d\n", ret, val);
				if(ret == -1){
					break;
				}
			} while(!ret);
//D("HALT!!\n");
			system_exec();
		}
	}
	close(fd);
	close(epfd);
}

int main(void)
{
	int pid, fd;

	if (getuid()) {
		fprintf(stderr, "must run super user\n");
		return 1;
	}

	if((pid = fork())){
		/* parent */
		char tmp[100];
		if ((fd = open(PID_FILE, O_CREAT|O_WRONLY|O_TRUNC)) < 0) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			exit(-1);
		}
		sprintf(tmp, "%d\n", pid);
		if (write(fd, tmp, strlen(tmp)) != strlen(tmp)) {
			printf("%d: %s\n", __LINE__, strerror(errno));
			close(fd);
			exit(-2);
		}
		close(fd);
		return 0;
	}
	else{
#if !defined(DEBUG)
		/* daemon */
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
#endif
		if(setsid() == -1) {
			exit(4);
		}

#if 0
		/* child */
		signal(SIGHUP,donothing);
		signal(SIGINT,donothing);
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
#endif

		watch_pushsw();

		exit(0);
	}

	return 0;
}

#if 0
void donothing(int i){
}
void die(int i){
	flag = 0;
}
#endif
