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
 *
 *	from: @(#)ftp_var.h	5.9 (Berkeley) 6/1/90
 *	$Id: ftp_var.h,v 1.12 1999/10/02 18:39:17 dholland Exp $
 */

/*
 * FTP global variables.
 */

#include <setjmp.h>
#include <sys/param.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <pthread.h>

/*
 * Tick counter step size.
 */
#define TICKBYTES     10240

#ifndef Extern
#define Extern extern
#endif
/*glob.c******************/
typedef struct {
    const char *text;
} centry;

typedef struct {
    char *text;
} entry;

typedef struct l_glob_
{
	entry *gargv;	/* Pointer to the (stack) arglist */
	entry *sortbas;
	int gargc;	/* Number args in gargv */
	int gnleft;		/* space left before we hit max args length */
	
	short gflag;
	
	int globcnt;
	char *globchars;
	
	char *gpath;
	char *gpathp;
	char *lastgpathp;
	int globbed;
	const char *entp;

}l_glob;

/*ftp.c**********/
typedef struct l_ftp_
{
	struct sockaddr_in hisctladdr;
	struct sockaddr_in data_addr;
	struct sockaddr_in myctladdr;
	int ptflag;//need init 0
	sigjmp_buf recvabort;
	sigjmp_buf sendabort;
	sigjmp_buf ptabort;
	int ptabflg;//need init 0
	int abrtflag;//need init 0
}l_ftp;
/********************/

/*cmds.c******************/
struct types {
	char *t_name;
	char *t_mode;
	int t_type;
	char *t_arg;
};

typedef struct {
	 char *mname;
	 sigjmp_buf jabort;
	 sigjmp_buf abortprox;
	struct types types[6];
}l_mcds;
/*************************/




/***************************************/




/*
 * Format of command table.
 */
struct cmd {
	
	const char *c_name;	/* name of command */
	const char *c_help;	/* help string */
	char c_bell;		/* give bell when command completes */
	char c_conn;		/* must be connected to use command */
	char c_proxy;		/* proxy server may execute */

        /* Exactly one of these should be non-NULL. */
	void (*c_handler_v)(int, char **); /* function to call */
	void (*c_handler_0)(void);
	void (*c_handler_1)(const char *);
};

struct macel {
	char mac_name[9];	/* macro name */
	char *mac_start;	/* start of macro in macbuf */
	char *mac_end;		/* end of macro in macbuf */
};


typedef struct g_ftp_
{
	l_mcds local_mcds;
	l_glob local_glob;
	l_ftp local_ftp;
	/*
	 * Options and other state info.
	 */
	int	rl_inhibit; /* disable readline support */
	int	traceflag;	/* trace packets exchanged */
	int	hash;		/* print # for each buffer transferred */
	int	tick;		/* print byte counter during transfers */
	int	sendport;	/* use PORT cmd for each data connection */
	int	verbose;	/* print messages coming back from server */
	int	connected;	/* connected to server */
	int	fromatty;	/* input is from a terminal */
	int	interactive;	/* interactively prompt on m* cmds ½»»¥Ä£Ê½*/

	int	debug;		/* debugging level */
	int	bell;		/* ring bell on cmd completion */
	int	doglob; 	/* glob local file names */
	int	proxy;		/* proxy server connection active */
	int	proxflag;	/* proxy connection exists */
	int	sunique;	/* store files on server with unique name */
	int	runique;	/* store local files with unique name */
	int	mcase;		/* map upper to lower case for mget names */
	int	ntflag; 	/* use ntin ntout tables for name xlation */
	int	mapflag;	/* use mapin mapout templates on file names */
	int	code;		/* return/reply code for ftp command */
	int	crflag; 	/* if 1, strip car. rets. on ascii gets */

	char 	pasv[64];		/* passive port for proxy data connection */
	int		passivemode;	/* passive mode enabled */
	char *altarg;	/* argv[1] with no shell-like preprocessing  */
	char ntin[17];	/* input translation table */
	char ntout[17];	/* output translation table */
	char mapin[MAXPATHLEN];	/* input map template */
	char mapout[MAXPATHLEN]; /* output map template */
	char typename[32];		/* name of file transfer type */
	int	type;			/* requested file transfer type */
	int	curtype;		/* current file transfer type */
	char structname[32]; 	/* name of file transfer structure */

	int	stru;			/* file transfer structure */
	char formname[32];		/* name of file transfer format */
	int	form;			/* file transfer format */
	char modename[32];		/* name of file transfer mode */
	int	mode;			/* file transfer mode */
	char bytename[32];		/* local byte size in ascii */
	int	bytesize;		/* local byte size in binary */
	
	char *hostname;	/* name of host connected to */
	int	unix_server;	/* server is unix, can use binary for ascii */
	int	unix_proxy; /* proxy is unix, can use binary for ascii */


	/*Extern struct servent *sp;*/	/* service spec for tcp/ftp */
	int	ftp_port;	/* htons'd port number for ftp service */
	
	char line[200];	/* input line buffer */
	char *stringbase;	/* current scan point in line buffer */
	char argbuf[200];	/* argument storage buffer */
	char *argbase;	/* current storage point in arg buffer */
	int	cpend;		/* flag: if != 0, then pending server reply */
	int	mflag;		/* flag: if != 0, then active multi command */
	
	int	options;	/* used during socket creation */


	/*main.c*/
	char *home;
	
	/*ftp.c*/
	char reply_string[BUFSIZ];		/* last line of previous reply */
	FILE *cin;
	FILE *cout;
	int data;
	off_t restart_point;

	FILE *STDIN;
	FILE *STDOUT;

	/*glob.c*/
	char *globerr;

	int macnum;			/* number of defined macros */
	struct macel macros[16];
	char macbuf[4096];
}g_ftp;


#define MACBUF_SIZE 4096

extern int g_ftp_debug;
#define PRINTF(str, args...)  do{\
	if (g_ftp_debug)\
	{\
		printf(str, ##args);\
	}\
}while(0)


#define FPRINTF(outfd, str, args...)  do{\
		if (g_ftp_debug)\
		{\
			fprintf(outfd, str, ##args);\
		}\
	}while(0)
		
#define PUTCHAR(char_s)  do{\
		if (g_ftp_debug)\
		{\
			putchar(char_s);\
		}\
	}while(0)
			
#define PUTC(char_s, outfd)  do{\
		if (g_ftp_debug)\
		{\
			putc(char_s, outfd);\
		}\
	}while(0)



char *hookup(char *host, int port);
struct cmd *getcmd(const char *);
char **makeargv(int *pargc, char **parg);
int dologin(const char *loginname, const char *pw, const char *acct);
int command(const char *fmt, ...);
void sendrequest(const char *cmd, char *local, char *remote, int printnames);
void recvrequest(const char *cmd, char *local, char *remote, 
		 const char *lmode, int printnames);

void blkfree(char **av0);
int getreply(int expecteof);
void domacro(int argc, char *argv[]);
void pswitch(int flag);
void setpeer(int argc, char *argv[]);
void quit(void);
void changetype(int newtype, int show);
