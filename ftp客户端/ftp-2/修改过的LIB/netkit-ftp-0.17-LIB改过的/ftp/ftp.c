/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* 
 * From: @(#)ftp.c	5.38 (Berkeley) 4/22/91
char ftp_rcsid[] = 
  "$Id: ftp.c,v 1.25 1999/12/13 20:33:20 dholland Exp $";
*/

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <stdarg.h>

#include "ftp_var.h"
#include "cmds.h"

#include "../version.h"


//l_ftp local_ftp={{0},{0},{0},0};



void lostpeer(int);


static char *gunique(char *);
static void proxtrans(const char *cmd, char *local, char *remote);
static int initconn(void);
static void ptransfer(const char *direction, long bytes, 
		      const struct timeval *t0, 
		      const struct timeval *t1);
static void tvsub(struct timeval *tdiff, 
		  const struct timeval *t1, 
		  const struct timeval *t0);
static void abort_remote(FILE *din);

static FILE *dataconn(const char *);
extern g_ftp * get_ftp_local_variable(void);

char *
hookup(char *host, int port)
{
	register struct hostent *hp = 0;
	int s, tos;
	socklen_t len;
	static char hostnamebuf[256];
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

	memset(&lv_ftp->local_ftp.hisctladdr, 0, sizeof(lv_ftp->local_ftp.hisctladdr));
	if (inet_aton(host, &lv_ftp->local_ftp.hisctladdr.sin_addr)) {
		lv_ftp->local_ftp.hisctladdr.sin_family = AF_INET;
		strncpy(hostnamebuf, host, sizeof(hostnamebuf));
		hostnamebuf[sizeof(hostnamebuf)-1]=0;
	} 
	else {
		hp = gethostbyname(host);
		if (hp == NULL) {
			FPRINTF(stderr, "ftp: %s: ", host);
			herror((char *)NULL);
			lv_ftp->code = -1;
			return((char *) 0);
		}
		lv_ftp->local_ftp.hisctladdr.sin_family = hp->h_addrtype;
		if (hp->h_length > (int)sizeof(lv_ftp->local_ftp.hisctladdr.sin_addr)) {
			hp->h_length = sizeof(lv_ftp->local_ftp.hisctladdr.sin_addr);
		}
		memcpy(&lv_ftp->local_ftp.hisctladdr.sin_addr, hp->h_addr_list[0], hp->h_length);
		(void) strncpy(hostnamebuf, hp->h_name, sizeof(hostnamebuf));
		hostnamebuf[sizeof(hostnamebuf)-1] = 0;
	}
	lv_ftp->hostname = hostnamebuf;
	s = socket(lv_ftp->local_ftp.hisctladdr.sin_family, SOCK_STREAM, 0);
	if (s < 0) {
		perror("ftp: socket");
		lv_ftp->code = -1;
		return (0);
	}
	lv_ftp->local_ftp.hisctladdr.sin_port = port;
	while (connect(s, (struct sockaddr *)&lv_ftp->local_ftp.hisctladdr, sizeof (lv_ftp->local_ftp.hisctladdr)) < 0) {
		if (hp && hp->h_addr_list[1]) {
			int oerrno = errno;

			FPRINTF(stderr, "ftp: connect to address %s: ",
				inet_ntoa(lv_ftp->local_ftp.hisctladdr.sin_addr));
			errno = oerrno;
			perror((char *) 0);
			hp->h_addr_list++;
			memcpy(&lv_ftp->local_ftp.hisctladdr.sin_addr, hp->h_addr_list[0], 
			       hp->h_length);
			PRINTF("Trying %s...\n",
				inet_ntoa(lv_ftp->local_ftp.hisctladdr.sin_addr));
			(void) close(s);
			s = socket(lv_ftp->local_ftp.hisctladdr.sin_family, SOCK_STREAM, 0);
			if (s < 0) {
				perror("ftp: socket");
				lv_ftp->code = -1;
				return (0);
			}
			continue;
		}
		perror("ftp: connect");
		lv_ftp->code = -1;
		goto bad;
	}
	len = sizeof (lv_ftp->local_ftp.myctladdr);
	if (getsockname(s, (struct sockaddr *)&lv_ftp->local_ftp.myctladdr, &len) < 0) {
		perror("ftp: getsockname");
		lv_ftp->code = -1;
		goto bad;
	}
#ifdef IP_TOS
	tos = IPTOS_LOWDELAY;
	if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
		perror("ftp: setsockopt TOS (ignored)");
#endif
	lv_ftp->cin = fdopen(s, "r");
	lv_ftp->cout = fdopen(s, "w");
	if (lv_ftp->cin == NULL || lv_ftp->cout == NULL) {
		FPRINTF(stderr, "ftp: fdopen failed.\n");
		if (lv_ftp->cin)
			(void) fclose(lv_ftp->cin);
		if (lv_ftp->cout)
			(void) fclose(lv_ftp->cout);
		lv_ftp->code = -1;
		goto bad;
	}
	if (lv_ftp->verbose)
		PRINTF("Connected to %s.\n", lv_ftp->hostname);
	if (getreply(0) > 2) { 	/* read startup message from server */
		if (lv_ftp->cin)
			(void) fclose(lv_ftp->cin);
		if (lv_ftp->cout)
			(void) fclose(lv_ftp->cout);
		lv_ftp->code = -1;
		goto bad;
	}
#ifdef SO_OOBINLINE
	{
	int on = 1;

	if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on))
		< 0 && lv_ftp->debug) {
			perror("ftp: setsockopt");
		}
	}
#endif /* SO_OOBINLINE */

	return (lv_ftp->hostname);
bad:
	(void) close(s);
	return ((char *)0);
}

int
dologin(const char *loginname, const char *pw, const char *acct)
{
	int n;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	if(loginname == NULL || pw == NULL){
		FPRINTF(stderr, "%s 1 Login failed.\n", __func__);
		return -3;
	}
	n = command("USER %s", loginname);
	/*
		USER getrecv 331 Please specify the password.

	*/
	if (n == CONTINUE) {
		n = command("PASS %s", pw);
	}
	/*
		PASS getrecv 230.but CONTINUE == 3
	*/
	if (n+1 != CONTINUE) {
		FPRINTF(stderr, "%s %d %d Login failed.\n", __func__, CONTINUE, n);
		return -3;
	}

	if (acct != NULL)
		(void) command("ACCT %s", acct);
	if (lv_ftp->proxy)
		return(0);
	for (n = 0; n < lv_ftp->macnum; ++n) {
		if (!strcmp("init", lv_ftp->macros[n].mac_name)) {
			int margc;
			char **margv;
			strcpy(lv_ftp->line, "$init");
			margv = makeargv(&margc, NULL);
			domacro(margc, margv);
			break;
		}
	}
	return (0);
}


static void
cmdabort(int ignore)
{
	(void)ignore;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	PRINTF("\n");
	lv_ftp->local_ftp.abrtflag++;
	if (lv_ftp->local_ftp.ptflag) siglongjmp(lv_ftp->local_ftp.ptabort,1);
}

int
command(const char *fmt, ...)
{
	va_list ap;
	int r;
	//void (*oldintr)(int);
	char cmd[1024]={0};
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	lv_ftp->local_ftp.abrtflag = 0;
	if (lv_ftp->debug) {
		PRINTF("---> ");
		va_start(ap, fmt);
		if (strncmp("PASS ", fmt, 5) == 0)
			PRINTF("PASS XXXX");
		else 
			vfprintf(lv_ftp->STDOUT, fmt, ap);
		va_end(ap);
		PRINTF("\n");
	}
	if (lv_ftp->cout == NULL) {
		perror ("No control connection for command");
		lv_ftp->code = -1;
		return (0);
	}
	//oldintr = signal(SIGINT, cmdabort);
	va_start(ap, fmt);
	vsprintf(cmd, fmt, ap);
	//vfprintf(lv_ftp->cout, fmt, ap);
	va_end(ap);

	fprintf(lv_ftp->cout, cmd);
	fprintf(lv_ftp->cout, "\r\n");
	fflush(lv_ftp->cout);

	lv_ftp->cpend = 1;
	r = getreply(!strcmp(fmt, "QUIT"));

	//if (lv_ftp->local_ftp.abrtflag && oldintr != SIG_IGN)
		//(*oldintr)(SIGINT);
	//(void) signal(SIGINT, oldintr);
	return(r);
}


#include <ctype.h>

int
getreply(int expecteof)
{
	register int c, n;
	register int dig;
	register char *cp;
	int originalcode = 0, continuation = 0;
	void (*oldintr)(int);
	int pflag = 0;
	size_t px = 0;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};
	size_t psize = sizeof(lv_ftp->pasv);

	oldintr = signal(SIGINT, cmdabort);
	for (;;) {
		dig = n = lv_ftp->code = 0;
		cp = lv_ftp->reply_string;
		while ((c = getc(lv_ftp->cin)) != '\n') {
			if (c == IAC) {     /* handle telnet commands */
				switch (c = getc(lv_ftp->cin)) {
				case WILL:
				case WONT:
					c = getc(lv_ftp->cin);
					fprintf(lv_ftp->cout, "%c%c%c", IAC, DONT, c);
					fflush(lv_ftp->cout);
					break;
				case DO:
				case DONT:
					c = getc(lv_ftp->cin);
					fprintf(lv_ftp->cout, "%c%c%c", IAC, WONT, c);
					fflush(lv_ftp->cout);
					break;
				default:
					break;
				}
				continue;
			}
			dig++;
			if (c == EOF) {
				if (expecteof) {
					(void) signal(SIGINT,oldintr);
					lv_ftp->code = 221;
					return (0);
				}
				lostpeer(0);
				if (lv_ftp->verbose) {
					PRINTF("421 Service not available, remote server has closed connection\n");
				}
				lv_ftp->code = 421;
				return(4);
			}
			if (c != '\r' && (lv_ftp->verbose > 0 ||
			    (lv_ftp->verbose > -1 && n == '5' && dig > 4))) {
				if (lv_ftp->proxflag &&
				   (dig == 1 || (dig == 5 && lv_ftp->verbose == 0)))
					PRINTF("%s:",lv_ftp->hostname);
				PUTCHAR(c);
			}
			if (dig < 4 && isdigit(c))
				lv_ftp->code = lv_ftp->code * 10 + (c - '0');
			if (!pflag && lv_ftp->code == 227)
				pflag = 1;
			if (dig > 4 && pflag == 1 && isdigit(c))
				pflag = 2;
			if (pflag == 2) {
				if (c != '\r' && c != ')') {
					if (px < psize-1) lv_ftp->pasv[px++] = c;
				}
				else {
					lv_ftp->pasv[px] = '\0';
					pflag = 3;
				}
			}
			if (dig == 4 && c == '-') {
				if (continuation)
					lv_ftp->code = 0;
				continuation++;
			}
			if (n == 0)
				n = c;
			if (cp < &lv_ftp->reply_string[sizeof(lv_ftp->reply_string) - 1])
				*cp++ = c;
		}

		if (lv_ftp->verbose > 0 || (lv_ftp->verbose > -1 && n == '5')) {
			PUTCHAR(c);
		}
		if (continuation && lv_ftp->code != originalcode) {
			if (originalcode == 0)
				originalcode = lv_ftp->code;
			continue;
		}
		*cp = '\0';
		if (n != '1')
			lv_ftp->cpend = 0;
		(void) signal(SIGINT,oldintr);
		if (lv_ftp->code == 421 || originalcode == 421)
			lostpeer(0);
		//printf("lostpeer\n");
//printf("%d \n",lv_ftp->local_ftp.abrtflag);
//printf("%p != %p \n",oldintr, cmdabort);
//printf("%p != %p \n",oldintr, SIG_IGN);
		if (lv_ftp->local_ftp.abrtflag && oldintr != cmdabort && oldintr != SIG_IGN)
			(*oldintr)(SIGINT);
		return (n - '0');
	}
}

static int
empty(fd_set *mask, int hifd, int sec)
{
	struct timeval t;

	t.tv_sec = (long) sec;
	t.tv_usec = 0;
	return(select(hifd+1, mask, (fd_set *) 0, (fd_set *) 0, &t));
}

static void
abortsend(int ignore)
{
	(void)ignore;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->mflag = 0;
	lv_ftp->local_ftp.abrtflag = 0;
	PRINTF("\nsend aborted\nwaiting for remote to finish abort\n");
	siglongjmp(lv_ftp->local_ftp.sendabort, 1);
}

#define HASHBYTES 1024

void
sendrequest(const char *cmd, char *local, char *remote, int printnames)
{
	struct stat st;
	struct timeval start, stop;
	register int c, d;
	FILE *volatile fin, *volatile dout = 0;
	int (*volatile closefunc)(FILE *);
	void (*volatile oldintr)(int);
	void (*volatile oldintp)(int);
	volatile long bytes = 0, hashbytes = HASHBYTES;
	char buf[BUFSIZ], *bufp;
	const char *volatile lmode;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->verbose && printnames) {
		if (local && *local != '-')
			PRINTF("local: %s ", local);
		if (remote)
			PRINTF("remote: %s\n", remote);
	}
	if (lv_ftp->proxy) {
		proxtrans(cmd, local, remote);
		return;
	}
	if (lv_ftp->curtype != lv_ftp->type)
		changetype(lv_ftp->type, 0);
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	lmode = "w";
	if (sigsetjmp(lv_ftp->local_ftp.sendabort, 1)) {
		while (lv_ftp->cpend) {
			(void) getreply(0);
		}
		if (lv_ftp->data >= 0) {
			(void) close(lv_ftp->data);
			lv_ftp->data = -1;
		}
		if (oldintr)
			(void) signal(SIGINT,oldintr);
		if (oldintp)
			(void) signal(SIGPIPE,oldintp);
		lv_ftp->code = -1;
		return;
	}
	oldintr = signal(SIGINT, abortsend);
	if (strcmp(local, "-") == 0)
		fin = lv_ftp->STDIN;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE,SIG_IGN);
		fin = popen(local + 1, "r");
		if (fin == NULL) {
			perror(local + 1);
			(void) signal(SIGINT, oldintr);
			(void) signal(SIGPIPE, oldintp);
			lv_ftp->code = -1;
			return;
		}
		closefunc = pclose;
	} else {
		fin = fopen(local, "r");
		if (fin == NULL) {
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
			(void) signal(SIGINT, oldintr);
			lv_ftp->code = -1;
			return;
		}
		closefunc = fclose;
		if (fstat(fileno(fin), &st) < 0 ||
		    (st.st_mode&S_IFMT) != S_IFREG) {
			FPRINTF(lv_ftp->STDOUT, "%s: not a plain file.\n", local);
			(void) signal(SIGINT, oldintr);
			fclose(fin);
			lv_ftp->code = -1;
			return;
		}
	}
	if (initconn()) {
		(void) signal(SIGINT, oldintr);
		if (oldintp)
			(void) signal(SIGPIPE, oldintp);
		lv_ftp->code = -1;
		if (closefunc != NULL)
			(*closefunc)(fin);
		return;
	}
	if (sigsetjmp(lv_ftp->local_ftp.sendabort, 1))
		goto abort;

	if (lv_ftp->restart_point &&
	    (strcmp(cmd, "STOR") == 0 || strcmp(cmd, "APPE") == 0)) {
		if (fseek(fin, (long) lv_ftp->restart_point, 0) < 0) {
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
			lv_ftp->restart_point = 0;
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
		if (command("REST %ld", (long) lv_ftp->restart_point)
			!= CONTINUE) {
			lv_ftp->restart_point = 0;
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
		lv_ftp->restart_point = 0;
		lmode = "r+w";
	}
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if (oldintp)
				(void) signal(SIGPIPE, oldintp);
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
	} else
		if (command("%s", cmd) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if (oldintp)
				(void) signal(SIGPIPE, oldintp);
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
	dout = dataconn(lmode);
	if (dout == NULL)
		goto abort;
	(void) gettimeofday(&start, (struct timezone *)0);
	oldintp = signal(SIGPIPE, SIG_IGN);
	switch (lv_ftp->curtype) {

	case TYPE_I:
	case TYPE_L:
		errno = d = 0;
		while ((c = read(fileno(fin), buf, sizeof (buf))) > 0) {
			bytes += c;
			for (bufp = buf; c > 0; c -= d, bufp += d)
				if ((d = write(fileno(dout), bufp, c)) <= 0)
					break;
			if (lv_ftp->hash) {
				while (bytes >= hashbytes) {
					PUTCHAR('#');
					hashbytes += HASHBYTES;
				}
			}
			if (lv_ftp->tick && (bytes >= hashbytes)) {
				PRINTF("\rBytes transferred: %ld", bytes);
				while (bytes >= hashbytes)
					hashbytes += TICKBYTES;
			}
		}
		if (lv_ftp->hash && (bytes > 0)) {
			if (bytes < HASHBYTES)
				PUTCHAR('#');
			PUTCHAR('\n');
		}
		if (lv_ftp->tick) {
			PRINTF("\rBytes transferred: %ld\n", bytes);
		}
		if (c < 0)
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (d < 0) {
			if (errno != EPIPE) 
				perror("netout");
			bytes = -1;
		}
		break;

	case TYPE_A:
		while ((c = getc(fin)) != EOF) {
			if (c == '\n') {
				while (lv_ftp->hash && (bytes >= hashbytes)) {
					PUTCHAR('#');
					hashbytes += HASHBYTES;
				}
				if (lv_ftp->tick && (bytes >= hashbytes)) {
					PRINTF("\rBytes transferred: %ld",
						bytes);
					while (bytes >= hashbytes)
						hashbytes += TICKBYTES;
				}
				if (ferror(dout))
					break;
				PUTC('\r', dout);
				bytes++;
			}
			PUTC(c, dout);
			bytes++;
	/*		if (c == '\r') {			  	*/
	/*		(void)	PUTC('\0', dout);  (* this violates rfc */
	/*			bytes++;				*/
	/*		}                          			*/     
		}
		if (lv_ftp->hash) {
			if (bytes < hashbytes)
				PUTCHAR('#');
			PUTCHAR('\n');
		}
		if (lv_ftp->tick) {
			PRINTF("\rBytes transferred: %ld\n", bytes);
		}
		if (ferror(fin))
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (ferror(dout)) {
			if (errno != EPIPE)
				perror("netout");
			bytes = -1;
		}
		break;
	}
	(void) gettimeofday(&stop, (struct timezone *)0);
	if (closefunc != NULL)
		(*closefunc)(fin);
	(void) fclose(dout);
	/* closes data as well, so discard it */
	lv_ftp->data = -1;
	(void) getreply(0);
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
	return;
abort:
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	if (!lv_ftp->cpend) {
		lv_ftp->code = -1;
		return;
	}
	if (dout) {
		(void) fclose(dout);
	}
	if (lv_ftp->data >= 0) {
		/* if it just got closed with dout, again won't hurt */
		(void) close(lv_ftp->data);
		lv_ftp->data = -1;
	}
	(void) getreply(0);
	lv_ftp->code = -1;
	if (closefunc != NULL && fin != NULL)
		(*closefunc)(fin);
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
}

static void
abortrecv(int ignore)
{
	(void)ignore;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->mflag = 0;
	lv_ftp->local_ftp.abrtflag = 0;
	PRINTF("\nreceive aborted\nwaiting for remote to finish abort\n");
	siglongjmp(lv_ftp->local_ftp.recvabort, 1);
}

void
recvrequest(const char *cmd, 
	    char *volatile local, char *remote, 
	    const char *lmode, int printnames)
{
	FILE *volatile fout, *volatile din = 0;
	int (*volatile closefunc)(FILE *);
	void (*volatile oldintp)(int);
	void (*volatile oldintr)(int);
	volatile int is_retr, tcrflag, bare_lfs = 0;
	static unsigned bufsize;
	static char *buf;
	volatile long bytes = 0, hashbytes = HASHBYTES;
	register int c, d;
	struct timeval start, stop;
	struct stat st;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	is_retr = strcmp(cmd, "RETR") == 0;
	if (is_retr && lv_ftp->verbose && printnames) {
		if (local && *local != '-')
			PRINTF("local: %s ", local);
		if (remote)
			PRINTF("remote: %s\n", remote);
	}
	if (lv_ftp->proxy && is_retr) {
		proxtrans(cmd, local, remote);
		return;
	}
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	tcrflag = !lv_ftp->crflag && is_retr;
	if (sigsetjmp(lv_ftp->local_ftp.recvabort, 1)) {
		while (lv_ftp->cpend) {
			(void) getreply(0);
		}
		if (lv_ftp->data >= 0) {
			(void) close(lv_ftp->data);
			lv_ftp->data = -1;
		}
		if (oldintr)
			(void) signal(SIGINT, oldintr);
		lv_ftp->code = -1;
		return;
	}
	oldintr = signal(SIGINT, abortrecv);
	if (strcmp(local, "-") && *local != '|') {
		if (access(local, W_OK) < 0) {
			char *dir = rindex(local, '/');

			if (errno != ENOENT && errno != EACCES) {
				FPRINTF(stderr, "local: %s: %s\n", local,
					strerror(errno));
				(void) signal(SIGINT, oldintr);
				lv_ftp->code = -1;
				return;
			}
			if (dir != NULL)
				*dir = 0;
			d = access(dir ? local : ".", W_OK);
			if (dir != NULL)
				*dir = '/';
			if (d < 0) {
				FPRINTF(stderr, "local: %s: %s\n", local,
					strerror(errno));
				(void) signal(SIGINT, oldintr);
				lv_ftp->code = -1;
				return;
			}
			if (!lv_ftp->runique && errno == EACCES &&
			    chmod(local, 0600) < 0) {
				FPRINTF(stderr, "local: %s: %s\n", local,
					strerror(errno));
				/*
				 * Believe it or not, this was actually
				 * repeated in the original source.
				 */
				(void) signal(SIGINT, oldintr);
				/*(void) signal(SIGINT, oldintr);*/
				lv_ftp->code = -1;
				return;
			}
			if (lv_ftp->runique && errno == EACCES &&
			   (local = gunique(local)) == NULL) {
				(void) signal(SIGINT, oldintr);
				lv_ftp->code = -1;
				return;
			}
		}
		else if (lv_ftp->runique && (local = gunique(local)) == NULL) {
			(void) signal(SIGINT, oldintr);
			lv_ftp->code = -1;
			return;
		}
	}
	if (!is_retr) {
		if (lv_ftp->curtype != TYPE_A)
			changetype(TYPE_A, 0);
	} 
	else if (lv_ftp->curtype != lv_ftp->type) {
		changetype(lv_ftp->type, 0);
	}
	if (initconn()) {
		(void) signal(SIGINT, oldintr);
		lv_ftp->code = -1;
		return;
	}
	if (sigsetjmp(lv_ftp->local_ftp.recvabort, 1))
		goto abort;
	if (is_retr && lv_ftp->restart_point &&
	    command("REST %ld", (long) lv_ftp->restart_point) != CONTINUE)
		return;
	if (remote) {
		if (command("%s %s", cmd, remote) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			return;
		}
	} 
	else {
		if (command("%s", cmd) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			return;
		}
	}
	din = dataconn("r");
	if (din == NULL)
		goto abort;
	if (strcmp(local, "-") == 0)
		fout = lv_ftp->STDOUT;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE, SIG_IGN);
		fout = popen(local + 1, "w");
		if (fout == NULL) {
			perror(local+1);
			goto abort;
		}
		closefunc = pclose;
	} 
	else {
		fout = fopen(local, lmode);
		if (fout == NULL) {
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
			goto abort;
		}
		closefunc = fclose;
	}
	if (fstat(fileno(fout), &st) < 0 || st.st_blksize == 0)
		st.st_blksize = BUFSIZ;
	if (st.st_blksize > bufsize) {
		if (buf)
			(void) free(buf);
		buf = malloc((unsigned)st.st_blksize);
		if (buf == NULL) {
			perror("malloc");
			bufsize = 0;
			goto abort;
		}
		bufsize = st.st_blksize;
	}
	(void) gettimeofday(&start, (struct timezone *)0);
	switch (lv_ftp->curtype) {

	case TYPE_I:
	case TYPE_L:
		if (lv_ftp->restart_point &&
		    lseek(fileno(fout), (long) lv_ftp->restart_point, L_SET) < 0) {
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
			if (closefunc != NULL)
				(*closefunc)(fout);
			return;
		}
		errno = d = 0;
		while ((c = read(fileno(din), buf, bufsize)) > 0) {
			if ((d = write(fileno(fout), buf, c)) != c)
				break;
			bytes += c;
			if (lv_ftp->hash && is_retr) {
				while (bytes >= hashbytes) {
					PUTCHAR('#');
					hashbytes += HASHBYTES;
				}
			}
			if (lv_ftp->tick && (bytes >= hashbytes) && is_retr) {
				PRINTF("\rBytes transferred: %ld",
					bytes);
				while (bytes >= hashbytes)
					hashbytes += TICKBYTES;
			}
		}
		if (lv_ftp->hash && bytes > 0) {
			if (bytes < HASHBYTES)
				PUTCHAR('#');
			PUTCHAR('\n');
		}
		if (lv_ftp->tick && is_retr) {
			PRINTF("\rBytes transferred: %ld\n", bytes);
		}
		if (c < 0) {
			if (errno != EPIPE)
				perror("netin");
			bytes = -1;
		}
		if (d < c) {
			if (d < 0)
				FPRINTF(stderr, "local: %s: %s\n", local,
					strerror(errno));
			else
				FPRINTF(stderr, "%s: short write\n", local);
		}
		break;

	case TYPE_A:
		if (lv_ftp->restart_point) {
			register int i, n, ch;

			if (fseek(fout, 0L, L_SET) < 0)
				goto done;
			n = lv_ftp->restart_point;
			for (i = 0; i++ < n;) {
				if ((ch = getc(fout)) == EOF)
					goto done;
				if (ch == '\n')
					i++;
			}
			if (fseek(fout, 0L, L_INCR) < 0) {
done:
				FPRINTF(stderr, "local: %s: %s\n", local,
					strerror(errno));
				if (closefunc != NULL)
					(*closefunc)(fout);
				return;
			}
		}
		while ((c = getc(din)) != EOF) {
			if (c == '\n')
				bare_lfs++;
			while (c == '\r') {
				while (lv_ftp->hash && (bytes >= hashbytes)
					&& is_retr) {
					PUTCHAR('#');
					hashbytes += HASHBYTES;
				}
				if (lv_ftp->tick && (bytes >= hashbytes) && is_retr) {
					PRINTF("\rBytes transferred: %ld",
						bytes);
					while (bytes >= hashbytes)
						hashbytes += TICKBYTES;
				}
				bytes++;
				if ((c = getc(din)) != '\n' || tcrflag) {
					if (ferror(fout))
						goto break2;
					PUTC('\r', fout);
					if (c == '\0') {
						bytes++;
						goto contin2;
					}
					if (c == EOF)
						goto contin2;
				}
			}
			PUTC(c, fout);
			bytes++;
	contin2:	;
		}
break2:
		if (lv_ftp->hash && is_retr) {
			if (bytes < hashbytes)
				PUTCHAR('#');
			PUTCHAR('\n');
		}
		if (lv_ftp->tick && is_retr) {
			PRINTF("\rBytes transferred: %ld\n", bytes);
		}
		if (bare_lfs) {
			PRINTF("WARNING! %d bare linefeeds received in ASCII mode\n", bare_lfs);
			PRINTF("File may not have transferred correctly.\n");
		}
		if (ferror(din)) {
			if (errno != EPIPE)
				perror("netin");
			bytes = -1;
		}
		if (ferror(fout))
			FPRINTF(stderr, "local: %s: %s\n", local,
				strerror(errno));
		break;
	}
	if (closefunc != NULL)
		(*closefunc)(fout);
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) fclose(din);
	/* closes data as well, so discard it */
	lv_ftp->data = -1;
	(void) getreply(0);
	if (bytes > 0 && is_retr)
		ptransfer("received", bytes, &start, &stop);
	return;
abort:

/* abort using RFC959 recommended IP,SYNC sequence  */

	(void) gettimeofday(&stop, (struct timezone *)0);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	(void) signal(SIGINT, SIG_IGN);
	if (!lv_ftp->cpend) {
		lv_ftp->code = -1;
		(void) signal(SIGINT, oldintr);
		return;
	}

	abort_remote(din);
	lv_ftp->code = -1;
	if (closefunc != NULL && fout != NULL)
		(*closefunc)(fout);
	if (din) {
		(void) fclose(din);
	}
	if (lv_ftp->data >= 0) {
		/* if it just got closed with din, again won't hurt */
		(void) close(lv_ftp->data);
		lv_ftp->data = -1;
	}
	if (bytes > 0)
		ptransfer("received", bytes, &start, &stop);
	(void) signal(SIGINT, oldintr);
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
static int
initconn(void)
{
	register char *p, *a;
	int result, tmpno = 0;
	socklen_t len;
	int on = 1;
	int tos;
	u_long a1,a2,a3,a4,p1,p2;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	if (lv_ftp->passivemode) {
		lv_ftp->data = socket(AF_INET, SOCK_STREAM, 0);
		if (lv_ftp->data < 0) {
			perror("ftp: socket");
			return(1);
		}
		if (lv_ftp->options & SO_DEBUG &&
		    setsockopt(lv_ftp->data, SOL_SOCKET, SO_DEBUG, (char *)&on,
			       sizeof (on)) < 0)
			perror("ftp: setsockopt (ignored)");
		if (command("PASV") != COMPLETE) {
			PRINTF("Passive mode refused.\n");
			return(1);
		}

		/*
		 * What we've got at this point is a string of comma separated
		 * one-byte unsigned integer values, separated by commas.
		 * The first four are the an IP address. The fifth is the MSB
		 * of the port number, the sixth is the LSB. From that we'll
		 * prepare a sockaddr_in.
		 */

		if (sscanf(lv_ftp->pasv,"%ld,%ld,%ld,%ld,%ld,%ld",
			   &a1,&a2,&a3,&a4,&p1,&p2)
		    != 6) 
		{
			PRINTF("Passive mode address scan failure. Shouldn't happen!\n");
			return(1);
		}

		lv_ftp->local_ftp.data_addr.sin_family = AF_INET;
		lv_ftp->local_ftp.data_addr.sin_addr.s_addr = htonl((a1 << 24) | (a2 << 16) |
						  (a3 << 8) | a4);
		lv_ftp->local_ftp.data_addr.sin_port = htons((p1 << 8) | p2);

		if (connect(lv_ftp->data, (struct sockaddr *) &lv_ftp->local_ftp.data_addr,
		    sizeof(lv_ftp->local_ftp.data_addr))<0) {
			perror("ftp: connect");
			return(1);
		}
#ifdef IP_TOS
		tos = IPTOS_THROUGHPUT;
		if (setsockopt(lv_ftp->data, IPPROTO_IP, IP_TOS, (char *)&tos,
		    sizeof(tos)) < 0)
			perror("ftp: setsockopt TOS (ignored)");
#endif
		return(0);
	}
noport:
	lv_ftp->local_ftp.data_addr = lv_ftp->local_ftp.myctladdr;
	if (lv_ftp->sendport)
		lv_ftp->local_ftp.data_addr.sin_port = 0;	/* let system pick one */ 
	if (lv_ftp->data != -1)
		(void) close(lv_ftp->data);
	lv_ftp->data = socket(AF_INET, SOCK_STREAM, 0);
	if (lv_ftp->data < 0) {
		perror("ftp: socket");
		if (tmpno)
			lv_ftp->sendport = 1;
		return (1);
	}
	if (!lv_ftp->sendport)
		if (setsockopt(lv_ftp->data, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof (on)) < 0) {
			perror("ftp: setsockopt (reuse address)");
			goto bad;
		}
	if (bind(lv_ftp->data, (struct sockaddr *)&lv_ftp->local_ftp.data_addr, sizeof (lv_ftp->local_ftp.data_addr)) < 0) {
		perror("ftp: bind");
		goto bad;
	}
	if (lv_ftp->options & SO_DEBUG &&
	    setsockopt(lv_ftp->data, SOL_SOCKET, SO_DEBUG, (char *)&on, sizeof (on)) < 0)
		perror("ftp: setsockopt (ignored)");
	len = sizeof (lv_ftp->local_ftp.data_addr);
	if (getsockname(lv_ftp->data, (struct sockaddr *)&lv_ftp->local_ftp.data_addr, &len) < 0) {
		perror("ftp: getsockname");
		goto bad;
	}
	if (listen(lv_ftp->data, 1) < 0)
		perror("ftp: listen");
	if (lv_ftp->sendport) {
		a = (char *)&lv_ftp->local_ftp.data_addr.sin_addr;
		p = (char *)&lv_ftp->local_ftp.data_addr.sin_port;
#define	UC(b)	(((int)b)&0xff)
		result =
		    command("PORT %d,%d,%d,%d,%d,%d",
		      UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]),
		      UC(p[0]), UC(p[1]));
		if (result == ERROR && lv_ftp->sendport == -1) {
			lv_ftp->sendport = 0;
			tmpno = 1;
			goto noport;
		}
		return (result != COMPLETE);
	}
	if (tmpno)
		lv_ftp->sendport = 1;
#ifdef IP_TOS
	on = IPTOS_THROUGHPUT;
	if (setsockopt(lv_ftp->data, IPPROTO_IP, IP_TOS, (char *)&on, sizeof(int)) < 0)
		perror("ftp: setsockopt TOS (ignored)");
#endif
	return (0);
bad:
	(void) close(lv_ftp->data), lv_ftp->data = -1;
	if (tmpno)
		lv_ftp->sendport = 1;
	return (1);
}

static FILE *
dataconn(const char *lmode)
{
	struct sockaddr_in from;
	int s, tos;
	socklen_t fromlen = sizeof(from);
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

        if (lv_ftp->passivemode)
            return (fdopen(lv_ftp->data, lmode));

	s = accept(lv_ftp->data, (struct sockaddr *) &from, &fromlen);
	if (s < 0) {
		perror("ftp: accept");
		(void) close(lv_ftp->data), lv_ftp->data = -1;
		return (NULL);
	}
	(void) close(lv_ftp->data);
	lv_ftp->data = s;
#ifdef IP_TOS
	tos = IPTOS_THROUGHPUT;
	if (setsockopt(s, IPPROTO_IP, IP_TOS, (char *)&tos, sizeof(int)) < 0)
		perror("ftp: setsockopt TOS (ignored)");
#endif
	return (fdopen(lv_ftp->data, lmode));
}

static void
ptransfer(const char *direction, long bytes, 
	  const struct timeval *t0, 
	  const struct timeval *t1)
{
	struct timeval td;
	float s, bs;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->verbose) {
		tvsub(&td, t1, t0);
		s = td.tv_sec + (td.tv_usec / 1000000.);
#define	nz(x)	((x) == 0 ? 1 : (x))
		bs = bytes / nz(s);
		PRINTF("%ld bytes %s in %.3g secs (%.2g Kbytes/sec)\n",
		    bytes, direction, s, bs / 1024.0);
	}
}

#if 0
tvadd(tsum, t0)
	struct timeval *tsum, *t0;
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
}
#endif

static void
tvsub(struct timeval *tdiff, 
      const struct timeval *t1, 
      const struct timeval *t0)
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

static 
void
psabort(int ignore)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	(void)ignore;
	lv_ftp->local_ftp.abrtflag++;
}

void
pswitch(int flag)
{
	void (*oldintr)(int);
	static struct comvars {
		int connect;
		char name[MAXHOSTNAMELEN];
		struct sockaddr_in mctl;
		struct sockaddr_in hctl;
		FILE *in;
		FILE *out;
		int tpe;
		int curtpe;
		int cpnd;
		int sunqe;
		int runqe;
		int mcse;
		int ntflg;
		char nti[17];
		char nto[17];
		int mapflg;
		char mi[MAXPATHLEN];
		char mo[MAXPATHLEN];
	} proxstruct, tmpstruct;
	struct comvars *ip, *op;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->local_ftp.abrtflag = 0;
	oldintr = signal(SIGINT, psabort);
	if (flag) {
		if (lv_ftp->proxy)
			return;
		ip = &tmpstruct;
		op = &proxstruct;
		lv_ftp->proxy++;
	} 
	else {
		if (!lv_ftp->proxy)
			return;
		ip = &proxstruct;
		op = &tmpstruct;
		lv_ftp->proxy = 0;
	}
	ip->connect = lv_ftp->connected;
	lv_ftp->connected = op->connect;
	if (lv_ftp->hostname) {
		(void) strncpy(ip->name, lv_ftp->hostname, sizeof(ip->name) - 1);
		ip->name[strlen(ip->name)] = '\0';
	} 
	else {
		ip->name[0] = 0;
	}
	lv_ftp->hostname = op->name;
	ip->hctl = lv_ftp->local_ftp.hisctladdr;
	lv_ftp->local_ftp.hisctladdr = op->hctl;
	ip->mctl = lv_ftp->local_ftp.myctladdr;
	lv_ftp->local_ftp.myctladdr = op->mctl;
	ip->in = lv_ftp->cin;
	lv_ftp->cin = op->in;
	ip->out = lv_ftp->cout;
	lv_ftp->cout = op->out;
	ip->tpe = lv_ftp->type;
	lv_ftp->type = op->tpe;
	ip->curtpe = lv_ftp->curtype;
	lv_ftp->curtype = op->curtpe;
	ip->cpnd = lv_ftp->cpend;
	lv_ftp->cpend = op->cpnd;
	ip->sunqe = lv_ftp->sunique;
	lv_ftp->sunique = op->sunqe;
	ip->runqe = lv_ftp->runique;
	lv_ftp->runique = op->runqe;
	ip->mcse = lv_ftp->mcase;
	lv_ftp->mcase = op->mcse;
	ip->ntflg = lv_ftp->ntflag;
	lv_ftp->ntflag = op->ntflg;
	(void) strncpy(ip->nti, lv_ftp->ntin, 16);
	(ip->nti)[strlen(ip->nti)] = '\0';
	(void) strcpy(lv_ftp->ntin, op->nti);
	(void) strncpy(ip->nto, lv_ftp->ntout, 16);
	(ip->nto)[strlen(ip->nto)] = '\0';
	(void) strcpy(lv_ftp->ntout, op->nto);
	ip->mapflg = lv_ftp->mapflag;
	lv_ftp->mapflag = op->mapflg;
	(void) strncpy(ip->mi, lv_ftp->mapin, MAXPATHLEN - 1);
	(ip->mi)[strlen(ip->mi)] = '\0';
	(void) strcpy(lv_ftp->mapin, op->mi);
	(void) strncpy(ip->mo, lv_ftp->mapout, MAXPATHLEN - 1);
	(ip->mo)[strlen(ip->mo)] = '\0';
	(void) strcpy(lv_ftp->mapout, op->mo);
	(void) signal(SIGINT, oldintr);
	if (lv_ftp->local_ftp.abrtflag) {
		lv_ftp->local_ftp.abrtflag = 0;
		(*oldintr)(SIGINT);
	}
}

static
void
abortpt(int ignore)
{
	(void)ignore;
	PRINTF("\n");
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	lv_ftp->local_ftp.ptabflg++;
	lv_ftp->mflag = 0;
	lv_ftp->local_ftp.abrtflag = 0;
	siglongjmp(lv_ftp->local_ftp.ptabort, 1);
}

static void
proxtrans(const char *cmd, char *local, char *remote)
{
	void (*volatile oldintr)(int);
	volatile int secndflag = 0, prox_type, nfnd;
	const char *volatile cmd2;
	fd_set mask;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (strcmp(cmd, "RETR"))
		cmd2 = "RETR";
	else
		cmd2 = lv_ftp->runique ? "STOU" : "STOR";
	if ((prox_type = lv_ftp->type) == 0) {
		if (lv_ftp->unix_server && lv_ftp->unix_proxy)
			prox_type = TYPE_I;
		else
			prox_type = TYPE_A;
	}
	if (lv_ftp->curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PASV") != COMPLETE) {
		PRINTF("proxy server does not support third party transfers.\n");
		return;
	}
	pswitch(0);
	if (!lv_ftp->connected) {
		PRINTF("No primary connection\n");
		pswitch(1);
		lv_ftp->code = -1;
		return;
	}
	if (lv_ftp->curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PORT %s", lv_ftp->pasv) != COMPLETE) {
		pswitch(1);
		return;
	}
	if (sigsetjmp(lv_ftp->local_ftp.ptabort, 1))
		goto abort;
	oldintr = signal(SIGINT, abortpt);
	if (command("%s %s", cmd, remote) != PRELIM) {
		(void) signal(SIGINT, oldintr);
		pswitch(1);
		return;
	}
	sleep(2);
	pswitch(1);
	secndflag++;
	if (command("%s %s", cmd2, local) != PRELIM)
		goto abort;
	lv_ftp->local_ftp.ptflag++;
	(void) getreply(0);
	pswitch(0);
	(void) getreply(0);
	(void) signal(SIGINT, oldintr);
	pswitch(1);
	lv_ftp->local_ftp.ptflag = 0;
	PRINTF("local: %s remote: %s\n", local, remote);
	return;
abort:
	(void) signal(SIGINT, SIG_IGN);
	lv_ftp->local_ftp.ptflag = 0;
	if (strcmp(cmd, "RETR") && !lv_ftp->proxy)
		pswitch(1);
	else if (!strcmp(cmd, "RETR") && lv_ftp->proxy)
		pswitch(0);
	if (!lv_ftp->cpend && !secndflag) {  /* only here if cmd = "STOR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (lv_ftp->cpend)
				abort_remote((FILE *) NULL);
		}
		pswitch(1);
		if (lv_ftp->local_ftp.ptabflg)
			lv_ftp->code = -1;
		(void) signal(SIGINT, oldintr);
		return;
	}
	if (lv_ftp->cpend)
		abort_remote((FILE *) NULL);
	pswitch(!lv_ftp->proxy);
	if (!lv_ftp->cpend && !secndflag) {  /* only if cmd = "RETR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (lv_ftp->cpend)
				abort_remote((FILE *) NULL);
			pswitch(1);
			if (lv_ftp->local_ftp.ptabflg)
				lv_ftp->code = -1;
			(void) signal(SIGINT, oldintr);
			return;
		}
	}
	if (lv_ftp->cpend)
		abort_remote((FILE *) NULL);
	pswitch(!lv_ftp->proxy);
	if (lv_ftp->cpend) {
		FD_ZERO(&mask);
		FD_SET(fileno(lv_ftp->cin), &mask);
		if ((nfnd = empty(&mask, fileno(lv_ftp->cin), 10)) <= 0) {
			if (nfnd < 0) {
				perror("abort");
			}
			if (lv_ftp->local_ftp.ptabflg)
				lv_ftp->code = -1;
			lostpeer(0);
		}
		(void) getreply(0);
		(void) getreply(0);
	}
	if (lv_ftp->proxy)
		pswitch(0);
	pswitch(1);
	if (lv_ftp->local_ftp.ptabflg)
		lv_ftp->code = -1;
	(void) signal(SIGINT, oldintr);
}

void
reset(void)
{
	fd_set mask;
	int nfnd = 1;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	FD_ZERO(&mask);
	while (nfnd > 0) {
		FD_SET(fileno(lv_ftp->cin), &mask);
		if ((nfnd = empty(&mask, fileno(lv_ftp->cin), 0)) < 0) {
			perror("reset");
			lv_ftp->code = 622;
			lostpeer(0);
		}
		else if (nfnd) {
			(void) getreply(0);
		}
	}
}

static char *
gunique(char *local)
{
	static char new[MAXPATHLEN];
	char *cp = rindex(local, '/');
	int d, count=0;
	char ext = '1';

	if (cp)
		*cp = '\0';
	d = access(cp ? local : ".", W_OK);
	if (cp)
		*cp = '/';
	if (d < 0) {
		FPRINTF(stderr, "local: %s: %s\n", local, strerror(errno));
		return((char *) 0);
	}
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	while (!d) {
		if (++count == 100) {
			PRINTF("runique: can't find unique file name.\n");
			return((char *) 0);
		}
		*cp++ = ext;
		*cp = '\0';
		if (ext == '9')
			ext = '0';
		else
			ext++;
		if ((d = access(new, F_OK)) < 0)
			break;
		if (ext != '0')
			cp--;
		else if (*(cp - 2) == '.')
			*(cp - 1) = '1';
		else {
			*(cp - 2) = *(cp - 2) + 1;
			cp--;
		}
	}
	return(new);
}

static void
abort_remote(FILE *din)
{
	char buf[BUFSIZ];
	int nfnd, hifd;
	fd_set mask;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	/*
	 * send IAC in urgent mode instead of DM because 4.3BSD places oob mark
	 * after urgent byte rather than before as is protocol now
	 */
	snprintf(buf, sizeof(buf), "%c%c%c", IAC, IP, IAC);
	if (send(fileno(lv_ftp->cout), buf, 3, MSG_OOB) != 3)
		perror("abort");
	fprintf(lv_ftp->cout, "%cABOR\r\n", DM);
	fflush(lv_ftp->cout);

	FD_ZERO(&mask);
	FD_SET(fileno(lv_ftp->cin), &mask);
	hifd = fileno(lv_ftp->cin);
	if (din) { 
		FD_SET(fileno(din), &mask);
		if (hifd < fileno(din)) hifd = fileno(din);
	}
	if ((nfnd = empty(&mask, hifd, 10)) <= 0) {
		if (nfnd < 0) {
			perror("abort");
		}
		if (lv_ftp->local_ftp.ptabflg)
			lv_ftp->code = -1;
		lostpeer(0);
	}
	if (din && FD_ISSET(fileno(din), &mask)) {
		while (read(fileno(din), buf, BUFSIZ) > 0)
			/* LOOP */;
	}
	if (getreply(0) == ERROR && lv_ftp->code == 552) {
		/* 552 needed for nic style abort */
		(void) getreply(0);
	}
	(void) getreply(0);
}
