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
 * from: @(#)cmds.c	5.26 (Berkeley) 3/5/91
char cmds_rcsid[] = 
   "$Id: cmds.c,v 1.33 2000/07/23 01:36:59 dholland Exp $";
*/

/*
 * FTP User Program -- Command Routines.
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/ftp.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <ctype.h>
#include <limits.h>	/* for PATH_MAX */
#include <time.h>
#include <string.h>
#include <unistd.h>


#include "ftp_var.h"
#include "pathnames.h"
#include "cmds.h"
#include "glob.h"

#include "libftp.h"


static char *remglob(char *argv[], int doswitch);
static int checkglob(int fd, const char *pattern);
static char *dotrans(char *name);
static char *domap(char *name);
static char *globulize(char *str);
static int confirm(const char *cmd, const char *file);
static int getit(int argc, char *argv[], int restartit, const char *modestr);
static void quote1(const char *initial, int argc, char **argv);
extern g_ftp * get_ftp_local_variable(void);


/*
 * pipeprotect: protect against "special" local filenames by prepending
 * "./". Special local filenames are "-" and "|..." AND "/...".
 */
static char *pipeprotect(char *name) 
{
	char *nu;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};
	if (strcmp(name, "-") && *name!='|' && *name!='/') {
		return name;
	}

	/* We're going to leak this memory. XXX. */
	nu = malloc(strlen(name)+3);
	if (nu==NULL) {
		PRINTF("malloc error\n");
		lv_ftp->code = 610;
		return NULL;
	}
	strcpy(nu, ".");
	if (*name != '/') strcat(nu, "/");
	strcat(nu, name);
	return nu;
}

/*
 * Look for embedded ".." in a pathname and change it to "!!", printing
 * a warning.
 */
static char *pathprotect(char *name)
{
	int gotdots=0, i, len;
	
	/* Convert null terminator to trailing / to catch a trailing ".." */
	len = strlen(name)+1;
	name[len-1] = '/';

	/*
	 * State machine loop. gotdots is < 0 if not looking at dots,
	 * 0 if we just saw a / and thus might start getting dots,
	 * and the count of dots seen so far if we have seen some.
	 */
	for (i=0; i<len; i++) {
		if (name[i]=='.' && gotdots>=0) gotdots++;
		else if (name[i]=='/' && gotdots<0) gotdots=0;
		else if (name[i]=='/' && gotdots==2) {
		    PRINTF("Warning: embedded .. in %.*s (changing to !!)\n",
			   len-1, name);
		    name[i-1] = '!';
		    name[i-2] = '!';
		    gotdots = 0;
		}
		else if (name[i]=='/') gotdots = 0;
		else gotdots = -1;
	}
	name[len-1] = 0;
	return name;
}

/*
 * Connect to peer server and
 * auto-login, if possible.
 */
void
setpeer(int argc, char **argv)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->connected) {
		PRINTF("Already connected to %s, use close first.\n",
			lv_ftp->hostname);
		lv_ftp->code = 622;
		return;
	}
	
	if (argc < 4) {
		PRINTF("usage: %s host username password [account]\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}


	char *host = hookup(argv[1], lv_ftp->ftp_port);
	if (host) {
		int overbose;

		lv_ftp->connected = 1;
		/*
		 * Set up defaults for FTP.
		 */
		(void) strcpy(lv_ftp->typename, "ascii"), lv_ftp->type = TYPE_A;
		lv_ftp->curtype = TYPE_A;
		(void) strcpy(lv_ftp->formname, "non-print"), lv_ftp->form = FORM_N;
		(void) strcpy(lv_ftp->modename, "stream"), lv_ftp->mode = MODE_S;
		(void) strcpy(lv_ftp->structname, "file"), lv_ftp->stru = STRU_F;
		(void) strcpy(lv_ftp->bytename, "8"), lv_ftp->bytesize = 8;

		if( dologin(argv[2], argv[3], NULL)){
			lv_ftp->code = 641;
			return;
		}

#if defined(__unix__) && CHAR_BIT == 8
/*
 * this ifdef is to keep someone form "porting" this to an incompatible
 * system and not checking this out. This way they have to think about it.
 */
		overbose = lv_ftp->verbose;
		if (lv_ftp->debug == 0)
			lv_ftp->verbose = -1;
		if (command("SYST") == COMPLETE && overbose) {
			register char *cp, c = 0;
			cp = index(lv_ftp->reply_string+4, ' ');
			if (cp == NULL)
				cp = index(lv_ftp->reply_string+4, '\r');
			if (cp) {
				if (cp[-1] == '.')
					cp--;
				c = *cp;
				*cp = '\0';
			}

			PRINTF("Remote system type is %s.\n",
				lv_ftp->reply_string+4);
			if (cp)
				*cp = c;
		}
		if (!strncmp(lv_ftp->reply_string, "215 UNIX Type: L8", 17)) {
			if (lv_ftp->proxy)
				lv_ftp->unix_proxy = 1;
			else
				lv_ftp->unix_server = 1;
			/*
			 * Set type to 0 (not specified by user),
			 * meaning binary by default, but don't bother
			 * telling server.  We can use binary
			 * for text files unless changed by the user.
			 */
			lv_ftp->type = 0;
			(void) strcpy(lv_ftp->typename, "binary");
			if (overbose)
			    PRINTF("Using %s mode to transfer files.\n",
				lv_ftp->typename);
		} else {
			if (lv_ftp->proxy)
				lv_ftp->unix_proxy = 0;
			else
				lv_ftp->unix_server = 0;
			if (overbose && 
			    !strncmp(lv_ftp->reply_string, "215 TOPS20", 10))
				PRINTF(
"Remember to set tenex mode when transfering binary files from this machine.\n");
		}
		lv_ftp->verbose = overbose;
#else
#warning "Unix auto-mode code skipped"
#endif /* unix */
	}
}



/*
 * Set transfer type.
 */
static 
void
do_settype(const char *thetype) 
{
	struct types *p;
	int comret;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	for (p = lv_ftp->local_mcds.types; p->t_name; p++)
		if (strcmp(thetype, p->t_name) == 0)
			break;
	if (p->t_name == 0) {
		PRINTF("%s: unknown mode\n", thetype);
		lv_ftp->code = 622;
		return;
	}
	if ((p->t_arg != NULL) && (*(p->t_arg) != '\0'))
		comret = command("TYPE %s %s", p->t_mode, p->t_arg);
	else
		comret = command("TYPE %s", p->t_mode);
	if (comret == COMPLETE) {
		(void) strcpy(lv_ftp->typename, p->t_name);
		lv_ftp->curtype = lv_ftp->type = p->t_type;
	}
}

void
settype(int argc, char *argv[])
{
	struct types *p;
	
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	if (argc > 2) {
		const char *sep;

		PRINTF("usage: %s [", argv[0]);
		sep = " ";
		for (p = lv_ftp->local_mcds.types; p->t_name; p++) {
			PRINTF("%s%s", sep, p->t_name);
			sep = " | ";
		}
		PRINTF(" ]\n");
		lv_ftp->code = 622;
		return;
	}
	if (argc < 2) {
		PRINTF("Using %s mode to transfer files.\n", lv_ftp->typename);
		lv_ftp->code = 0;
		return;
	}
	do_settype(argv[1]);
}

/*
 * Internal form of settype; changes current type in use with server
 * without changing our notion of the type for data transfers.
 * Used to change to and from ascii for listings.
 */
void
changetype(int newtype, int show)
{
	register struct types *p;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	int comret, oldverbose = lv_ftp->verbose;
	int oldtick = lv_ftp->tick;

	if (newtype == 0)
		newtype = TYPE_I;
	if (newtype == lv_ftp->curtype)
		return;
	if (lv_ftp->debug == 0 && show == 0)
		lv_ftp->verbose = 0;
	lv_ftp->tick = 0;
	for (p = lv_ftp->local_mcds.types; p->t_name; p++)
		if (newtype == p->t_type)
			break;
	if (p->t_name == 0) {
		PRINTF("ftp: internal error: unknown type %d\n", newtype);
		return;
	}
	if (newtype == TYPE_L && lv_ftp->bytename[0] != '\0')
		comret = command("TYPE %s %s", p->t_mode, lv_ftp->bytename);
	else
		comret = command("TYPE %s", p->t_mode);
	if (comret == COMPLETE)
		lv_ftp->curtype = newtype;
	lv_ftp->verbose = oldverbose;
	lv_ftp->tick = oldtick;
}

/*
 * Set binary transfer type.
 */
/*VARARGS*/
void
setbinary(void)
{
	do_settype("binary");
}

/*
 * Set ascii transfer type.
 */
/*VARARGS*/
void
setascii(void)
{
	do_settype("ascii");
}

/*
 * Set tenex transfer type.
 */
/*VARARGS*/
void
settenex(void)
{
	do_settype("tenex");
}

/*
 * Set file transfer mode.
 */
/*ARGSUSED*/
void
setmode(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	PRINTF("We only support %s mode, sorry.\n", lv_ftp->modename);
	lv_ftp->code = 622;
}

/*
 * Set file transfer format.
 */
/*ARGSUSED*/
void
setform(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	PRINTF("We only support %s format, sorry.\n", lv_ftp->formname);
	lv_ftp->code = 622;
}

/*
 * Set file transfer structure.
 */
void
setstruct(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	PRINTF("We only support %s structure, sorry.\n", lv_ftp->structname);
	lv_ftp->code = 622;
}

/*
 * Send a single file.
 */
void
put(int argc, char *argv[])
{
	const char *cmd;
	int loc = 0;
	char *oldargv1, *oldargv2;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc == 2) {
		argc++;
		argv[2] = argv[1];
		loc++;
	}

	if (argc < 3 ) {
		PRINTF("usage: %s local-file remote-file\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	oldargv1 = argv[1];
	oldargv2 = argv[2];
	argv[1] = globulize(argv[1]);
	if (!argv[1]) {
		lv_ftp->code = 622;
		return;
	}
	/*
	 * If "globulize" modifies argv[1], and argv[2] is a copy of
	 * the old argv[1], make it a copy of the new argv[1].
	 */
	if (argv[1] != oldargv1 && argv[2] == oldargv1) {
		argv[2] = argv[1];
	}
	cmd = (argv[0][0] == 'a') ? "APPE" : ((lv_ftp->sunique) ? "STOU" : "STOR");
	if (loc && lv_ftp->ntflag) {
		argv[2] = dotrans(argv[2]);
	}
	if (loc && lv_ftp->mapflag) {
		argv[2] = domap(argv[2]);
	}
	sendrequest(cmd, argv[1], argv[2],
	    argv[1] != oldargv1 || argv[2] != oldargv2);
}

void mabort(int);

/*
 * Send multiple files.
 */
void
mput(int argc, char *argv[])
{
	register int i;
	void (*oldintr)(int);
	int ointer;
	char *tp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s local-files\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	lv_ftp->local_mcds.mname = argv[0];
	lv_ftp->mflag = 1;
	oldintr = signal(SIGINT, mabort);
	(void) sigsetjmp(lv_ftp->local_mcds.jabort, 1);
	if (lv_ftp->proxy) {
		char *cp, *tp2, tmpbuf[PATH_MAX];

		while ((cp = remglob(argv,0)) != NULL) {
			if (*cp == 0) {
				lv_ftp->mflag = 0;
				continue;
			}
			if (lv_ftp->mflag && confirm(argv[0], cp)) {
				tp = cp;
				if (lv_ftp->mcase) {
					while (*tp && !islower(*tp)) {
						tp++;
					}
					if (!*tp) {
						tp = cp;
						tp2 = tmpbuf;
						while ((*tp2 = *tp) != '\0') {
						     if (isupper(*tp2)) {
						        *tp2 = 'a' + *tp2 - 'A';
						     }
						     tp++;
						     tp2++;
						}
					}
					tp = tmpbuf;
				}
				if (lv_ftp->ntflag) {
					tp = dotrans(tp);
				}
				if (lv_ftp->mapflag) {
					tp = domap(tp);
				}
				sendrequest((lv_ftp->sunique) ? "STOU" : "STOR",
				    cp, tp, cp != tp || !lv_ftp->interactive);
				if (!lv_ftp->mflag) {
					ointer = lv_ftp->interactive;
					lv_ftp->interactive = 1;
					if (confirm("Continue with","mput")) {
						lv_ftp->mflag++;
					}
					lv_ftp->interactive = ointer;
				}
			}
		}
		(void) signal(SIGINT, oldintr);
		lv_ftp->mflag = 0;
		return;
	}
	for (i = 1; i < argc; i++) {
		register char **cpp, **gargs;

		if (!lv_ftp->doglob) {
			if (lv_ftp->mflag && confirm(argv[0], argv[i])) {
				tp = (lv_ftp->ntflag) ? dotrans(argv[i]) : argv[i];
				tp = (lv_ftp->mapflag) ? domap(tp) : tp;
				sendrequest((lv_ftp->sunique) ? "STOU" : "STOR",
				    argv[i], tp, tp != argv[i] || !lv_ftp->interactive);
				if (!lv_ftp->mflag) {
					ointer = lv_ftp->interactive;
					lv_ftp->interactive = 1;
					if (confirm("Continue with","mput")) {
						lv_ftp->mflag++;
					}
					lv_ftp->interactive = ointer;
				}
			}
			continue;
		}
		gargs = ftpglob(argv[i]);
		if (lv_ftp->globerr != NULL) {
			PRINTF("%s\n", lv_ftp->globerr);
			if (gargs) {
				blkfree(gargs);
				free((char *)gargs);
			}
			continue;
		}
		for (cpp = gargs; cpp && *cpp != NULL; cpp++) {
			if (lv_ftp->mflag && confirm(argv[0], *cpp)) {
				tp = (lv_ftp->ntflag) ? dotrans(*cpp) : *cpp;
				tp = (lv_ftp->mapflag) ? domap(tp) : tp;
				sendrequest((lv_ftp->sunique) ? "STOU" : "STOR",
				    *cpp, tp, *cpp != tp || !lv_ftp->interactive);
				if (!lv_ftp->mflag) {
					ointer = lv_ftp->interactive;
					lv_ftp->interactive = 1;
					if (confirm("Continue with","mput")) {
						lv_ftp->mflag++;
					}
					lv_ftp->interactive = ointer;
				}
			}
		}
		if (gargs != NULL) {
			blkfree(gargs);
			free((char *)gargs);
		}
	}
	(void) signal(SIGINT, oldintr);
	lv_ftp->mflag = 0;
}

void
reget(int argc, char *argv[])
{
	(void) getit(argc, argv, 1, "r+w");
}

void
get(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	(void) getit(argc, argv, 0, lv_ftp->restart_point ? "r+w" : "w" );
}

/*
 * Receive one file.
 */
static int
getit(int argc, char *argv[], int restartit, const char *modestr)
{
	int loc = 0;
	char *oldargv1, *oldargv2;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	if (argc == 2) {
		argc++;
		/* 
		 * Protect the user from accidentally retrieving special
		 * local names.
		 */
		argv[2] = pipeprotect(argv[1]);
		if (!argv[2]) {
			lv_ftp->code = 622;
			return 0;
		}
		loc++;
	}
	if (argc < 3 ) {
		PRINTF("usage: %s remote-file [ local-file ]\n", argv[0]);
		lv_ftp->code = 622;
		return (0);
	}
	oldargv1 = argv[1];
	oldargv2 = argv[2];
	argv[2] = globulize(argv[2]);
	if (!argv[2]) {
		lv_ftp->code = 622;
		return (0);
	}
	if (loc && lv_ftp->mcase) {
		char *tp = argv[1], *tp2, tmpbuf[PATH_MAX];

		while (*tp && !islower(*tp)) {
			tp++;
		}
		if (!*tp) {
			tp = argv[2];
			tp2 = tmpbuf;
			while ((*tp2 = *tp) != '\0') {
				if (isupper(*tp2)) {
					*tp2 = 'a' + *tp2 - 'A';
				}
				tp++;
				tp2++;
			}
			argv[2] = tmpbuf;
		}
	}
	if (loc && lv_ftp->ntflag)
		argv[2] = dotrans(argv[2]);
	if (loc && lv_ftp->mapflag)
		argv[2] = domap(argv[2]);
	if (restartit) {
		struct stat stbuf;
		int ret;

		ret = stat(argv[2], &stbuf);
		if (restartit == 1) {
			if (ret < 0) {
				FPRINTF(stderr, "local: %s: %s\n", argv[2],
					strerror(errno));
				return (0);
			}
			lv_ftp->restart_point = stbuf.st_size;
		} else {
			if (ret == 0) {
				int overbose;

				overbose = lv_ftp->verbose;
				if (lv_ftp->debug == 0)
					lv_ftp->verbose = -1;
				if (command("MDTM %s", argv[1]) == COMPLETE) {
					int yy, mo, day, hour, min, sec;
					struct tm *tm;
					lv_ftp->verbose = overbose;
					sscanf(lv_ftp->reply_string,
					    "%*s %04d%02d%02d%02d%02d%02d",
					    &yy, &mo, &day, &hour, &min, &sec);
					tm = gmtime(&stbuf.st_mtime);
					tm->tm_mon++;
/* Indentation is misleading, but changes keep small. */
/* 
 * I think the indentation and braces are now correct. Whoever put this
 * in the way it was originally should be prohibited by law.
 */
					if (tm->tm_year+1900 > yy)
					    	return (1);
					if (tm->tm_year+1900 == yy) {
					   if (tm->tm_mon > mo)
					      return (1);
					   if (tm->tm_mon == mo) {
					      if (tm->tm_mday > day)
						 return (1);
					      if (tm->tm_mday == day) {
						 if (tm->tm_hour > hour)
							return (1);
						 if (tm->tm_hour == hour) {
						    if (tm->tm_min > min)
						       return (1);
						    if (tm->tm_min == min) {
						       if (tm->tm_sec > sec)
							  return (1);
						    }
						 }
					      }
					   }
					}
				} else {
					PRINTF("%s\n", lv_ftp->reply_string);
					lv_ftp->verbose = overbose;
					return (0);
				}
			}
		}
	}

	recvrequest("RETR", argv[2], argv[1], modestr,
		    argv[1] != oldargv1 || argv[2] != oldargv2);
	lv_ftp->restart_point = 0;
	return (0);
}

void
mabort(int ignore)
{
	int ointer;

	(void)ignore;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	PRINTF("\n");
	if (lv_ftp->mflag) {
		ointer = lv_ftp->interactive;
		lv_ftp->interactive = 1;
		if (confirm("Continue with", lv_ftp->local_mcds.mname)) {
			lv_ftp->interactive = ointer;
			siglongjmp(lv_ftp->local_mcds.jabort,0);
		}
		lv_ftp->interactive = ointer;
	}
	lv_ftp->mflag = 0;
	siglongjmp(lv_ftp->local_mcds.jabort,0);
}

/*
 * Get multiple files.
 */
void
mget(int argc, char **argv)
{
	void (*oldintr)(int);
	int ointer;
	char *cp, *tp, *tp2, tmpbuf[PATH_MAX];
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s remote-files\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	lv_ftp->local_mcds.mname = argv[0];
	lv_ftp->mflag = 1;
	oldintr = signal(SIGINT,mabort);
	(void) sigsetjmp(lv_ftp->local_mcds.jabort, 1);
	while ((cp = remglob(argv,lv_ftp->proxy)) != NULL) {
		if (*cp == '\0') {
			lv_ftp->mflag = 0;
			continue;
		}
		if (lv_ftp->mflag && confirm(argv[0], cp)) {
			tp = cp;
			if (lv_ftp->mcase) {
				while (*tp && !islower(*tp)) {
					tp++;
				}
				if (!*tp) {
					tp = cp;
					tp2 = tmpbuf;
					while ((*tp2 = *tp) != '\0') {
						if (isupper(*tp2)) {
							*tp2 = 'a' + *tp2 - 'A';
						}
						tp++;
						tp2++;
					}
				}
				tp = tmpbuf;
			}
			if (lv_ftp->ntflag) {
				tp = dotrans(tp);
			}
			if (lv_ftp->mapflag) {
				tp = domap(tp);
			}
			/* Reject embedded ".." */
			tp = pathprotect(tp);

			/* Prepend ./ to "-" or "!*" or leading "/" */
			tp = pipeprotect(tp);
			if (tp == NULL) {
				/* hmm... how best to handle this? */
				lv_ftp->mflag = 0;
			}
			else {
				recvrequest("RETR", tp, cp, "w",
					    tp != cp || !lv_ftp->interactive);
			}
			if (!lv_ftp->mflag) {
				ointer = lv_ftp->interactive;
				lv_ftp->interactive = 1;
				if (confirm("Continue with","mget")) {
					lv_ftp->mflag++;
				}
				lv_ftp->interactive = ointer;
			}
		}
	}
	(void) signal(SIGINT,oldintr);
	lv_ftp->mflag = 0;
}

char *
remglob(char *argv[], int doswitch)
{
	char temp[16];
	static char buf[PATH_MAX];
	static FILE *ftemp = NULL;
	static char **args;
	int oldverbose, oldhash, badglob = 0;
	char *cp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

	if (!lv_ftp->mflag) {
		if (!lv_ftp->doglob) {
			args = NULL;
		}
		else {
			if (ftemp) {
				(void) fclose(ftemp);
				ftemp = NULL;
			}
		}
		return(NULL);
	}
	if (!lv_ftp->doglob) {
		if (args == NULL)
			args = argv;
		if ((cp = *++args) == NULL)
			args = NULL;
		return (cp);
	}
	if (ftemp == NULL) {
		int oldumask, fd;
		(void) strcpy(temp, _PATH_TMP);

		/* libc 5.2.18 creates with mode 0666, which is dumb */
		oldumask = umask(077);
		fd = mkstemp(temp);
		umask(oldumask);

		if (fd<0) {
			PRINTF("Error creating temporary file, oops\n");
			return NULL;
		}
		
		oldverbose = lv_ftp->verbose, lv_ftp->verbose = 0;
		oldhash = lv_ftp->hash, lv_ftp->hash = 0;
		if (doswitch) {
			pswitch(!lv_ftp->proxy);
		}
		while (*++argv != NULL) {
			int	dupfd = dup(fd);

			recvrequest ("NLST", temp, *argv, "a", 0);
			if (!checkglob(dupfd, *argv)) {
				badglob = 1;
				break;
			}
		}
		unlink(temp);

		if (doswitch) {
			pswitch(!lv_ftp->proxy);
		}
		lv_ftp->verbose = oldverbose; lv_ftp->hash = oldhash;
		if (badglob) {
			PRINTF("Refusing to handle insecure file list\n");
			close(fd);
			return NULL;
		}
		ftemp = fdopen(fd, "r");
		if (ftemp == NULL) {
			PRINTF("fdopen failed, oops\n");
			return NULL;
		}
		rewind(ftemp);
	}
	if (fgets(buf, sizeof (buf), ftemp) == NULL) {
		(void) fclose(ftemp), ftemp = NULL;
		return (NULL);
	}
	if ((cp = index(buf, '\n')) != NULL)
		*cp = '\0';
	return (buf);
}

/*
 * Check whether given pattern matches `..'
 * We assume only a glob pattern starting with a dot will match
 * dot entries on the server.
 */
static int
isdotdotglob(const char *pattern)
{
	int	havedot = 0;
	char	c;

	if (*pattern++ != '.')
		return 0;
	while ((c = *pattern++) != '\0' && c != '/') {
		if (c == '*' || c == '?')
			continue;
		if (c == '.' && havedot++)
			return 0;
	}
	return 1;
}

/*
 * This function makes sure the list of globbed files returned from
 * the server doesn't contain anything dangerous such as
 * /home/<yourname>/.forward, or ../.forward,
 * or |mail foe@doe </etc/passwd, etc.
 * Covered areas:
 *  -	returned name starts with / but glob pattern doesn't
 *  -	glob pattern starts with / but returned name doesn't
 *  -	returned name starts with |
 *  -	returned name contains .. in a position where glob
 *	pattern doesn't match ..
 *	I.e. foo/.* allows foo/../bar but not foo/.bar/../fly
 *
 * Note that globbed names starting with / should really be stored
 * under the current working directory; this is handled in mget above.
 *						--okir
 */
static int
checkglob(int fd, const char *pattern)
{
	const char	*sp;
	char		buffer[MAXPATHLEN], dotdot[MAXPATHLEN];
	int		okay = 1, nrslash, initial, nr;
	FILE		*fp;

	/* Find slashes in glob pattern, and verify whether component
	 * matches `..'
	 */
	initial = (pattern[0] == '/');
	for (sp = pattern, nrslash = 0; sp != 0; sp = strchr(sp, '/')) {
		while (*sp == '/')
			sp++;
		if (nrslash >= MAXPATHLEN) {
			PRINTF("Incredible pattern: %s\n", pattern);
			return 0;
		}
		dotdot[nrslash++] = isdotdotglob(sp);
	}

	fp = fdopen(fd, "r");
	while (okay && fgets(buffer, sizeof(buffer), fp) != NULL) {
		char	*sp;

		if ((sp = strchr(buffer, '\n')) != 0) {
			*sp = '\0';
		} else {
			PRINTF("Extremely long filename from server: %s",
				buffer);
			okay = 0;
			break;
		}
		if (buffer[0] == '|'
		 || (buffer[0] != '/' && initial)
		 || (buffer[0] == '/' && !initial))
			okay = 0;
		for (sp = buffer, nr = 0; sp; sp = strchr(sp, '/'), nr++) {
			while (*sp == '/')
				sp++;
			if (sp[0] == '.' && !strncmp(sp, "../", 3)
			 && (nr >= nrslash || !dotdot[nr]))
				okay = 0;
		}
	}

	if (!okay)
		PRINTF("Filename provided by server "
		       "doesn't match pattern `%s': %s\n", pattern, buffer);

	fclose(fp);
	return okay;
}

static const char *
onoff(int bool)
{
	return (bool ? "on" : "off");
}

/*
 * Show status.
 */
void
status(void)
{
	int i;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->connected)
		PRINTF("Connected to %s.\n", lv_ftp->hostname);
	else
		PRINTF("Not connected.\n");
	if (!lv_ftp->proxy) {
		pswitch(1);
		if (lv_ftp->connected) {
			PRINTF("Connected for proxy commands to %s.\n", lv_ftp->hostname);
		}
		else {
			PRINTF("No proxy connection.\n");
		}
		pswitch(0);
	}
	PRINTF("Mode: %s; Type: %s; Form: %s; Structure: %s\n",
		lv_ftp->modename, lv_ftp->typename, lv_ftp->formname, lv_ftp->structname);
	PRINTF("Verbose: %s; Bell: %s; Prompting: %s; Globbing: %s\n", 
		onoff(lv_ftp->verbose), onoff(lv_ftp->bell), onoff(lv_ftp->interactive),
		onoff(lv_ftp->doglob));
	PRINTF("Store unique: %s; Receive unique: %s\n", onoff(lv_ftp->sunique),
		onoff(lv_ftp->runique));
	PRINTF("Case: %s; CR stripping: %s\n",onoff(lv_ftp->mcase),onoff(lv_ftp->crflag));
	if (lv_ftp->ntflag) {
		PRINTF("Ntrans: (in) %s (out) %s\n", lv_ftp->ntin,lv_ftp->ntout);
	}
	else {
		PRINTF("Ntrans: off\n");
	}
	if (lv_ftp->mapflag) {
		PRINTF("Nmap: (in) %s (out) %s\n", lv_ftp->mapin, lv_ftp->mapout);
	}
	else {
		PRINTF("Nmap: off\n");
	}
	PRINTF("Hash mark printing: %s; Use of PORT cmds: %s\n",
		onoff(lv_ftp->hash), onoff(lv_ftp->sendport));
	PRINTF("Tick counter printing: %s\n", onoff(lv_ftp->tick));
	if (lv_ftp->macnum > 0) {
		PRINTF("Macros:\n");
		for (i=0; i<lv_ftp->macnum; i++) {
			PRINTF("\t%s\n",lv_ftp->macros[i].mac_name);
		}
	}
	lv_ftp->code = 0;
}

/*
 * Set beep on cmd completed mode.
 */
void
setbell(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->bell = !lv_ftp->bell;
	PRINTF("Bell mode %s.\n", onoff(lv_ftp->bell));
	lv_ftp->code = lv_ftp->bell;
}

/*
 * Turn on packet tracing.
 */
void
settrace(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->traceflag = !lv_ftp->traceflag;
	PRINTF("Packet tracing %s.\n", onoff(lv_ftp->traceflag));
	lv_ftp->code = lv_ftp->traceflag;
}

/*
 * Toggle hash mark printing during transfers.
 */
void
sethash(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->hash = !lv_ftp->hash;
	if (lv_ftp->hash && lv_ftp->tick)
		settick();
 
	PRINTF("Hash mark printing %s", onoff(lv_ftp->hash));
	lv_ftp->code = lv_ftp->hash;
	if (lv_ftp->hash)
		PRINTF(" (%d bytes/hash mark)", 1024);
	PRINTF(".\n");
}

/*
 * Toggle tick counter printing during transfers.
 */
void
settick(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->tick = !lv_ftp->tick;
	if (lv_ftp->hash && lv_ftp->tick)
		sethash();
	PRINTF("Tick counter printing %s", onoff(lv_ftp->tick));
	lv_ftp->code = lv_ftp->tick;
	if (lv_ftp->tick)
		PRINTF(" (%d bytes/tick increment)", TICKBYTES);
	PRINTF(".\n");
}

/*
 * Turn on printing of server echos.
 */
void
setverbose(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->verbose = !lv_ftp->verbose;
	PRINTF("Verbose mode %s.\n", onoff(lv_ftp->verbose));
	lv_ftp->code = lv_ftp->verbose;
}

/*
 * Toggle PORT cmd use before each data connection.
 */
void
setport(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->sendport = !lv_ftp->sendport;
	PRINTF("Use of PORT cmds %s.\n", onoff(lv_ftp->sendport));
	lv_ftp->code = lv_ftp->sendport;
}

/*
 * Turn on interactive prompting
 * during mget, mput, and mdelete.
 */
void
setprompt(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	lv_ftp->interactive = !lv_ftp->interactive;
	PRINTF("Interactive mode %s.\n", onoff(lv_ftp->interactive));
	lv_ftp->code = lv_ftp->interactive;
}

/*
 * Toggle metacharacter interpretation
 * on local file names.
 */
void
setglob(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	lv_ftp->doglob = !lv_ftp->doglob;
	PRINTF("Globbing %s.\n", onoff(lv_ftp->doglob));
	lv_ftp->code = lv_ftp->doglob;
}

/*
 * Set debugging mode on/off and/or
 * set level of debugging.
 */
void
setdebug(int argc, char *argv[])
{
	int val;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc > 1) {
		val = atoi(argv[1]);
		if (val < 0) {
			PRINTF("%s: bad debugging value.\n", argv[1]);
			lv_ftp->code = 622;
			return;
		}
	} else
		val = !lv_ftp->debug;
	lv_ftp->debug = val;
	if (lv_ftp->debug)
		lv_ftp->options |= SO_DEBUG;
	else
		lv_ftp->options &= ~SO_DEBUG;
	PRINTF("Debugging %s (debug=%d).\n", onoff(lv_ftp->debug), lv_ftp->debug);
	lv_ftp->code = lv_ftp->debug > 0;
}

/*
 * Set current working directory
 * on remote machine.
 */
void
cd(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: cd remote-directory");
		lv_ftp->code = 622;
		return;
	}
	if (command("CWD %s", argv[1]) == ERROR && lv_ftp->code == 500) {
		if (lv_ftp->verbose)
			PRINTF("CWD command not recognized, trying XCWD\n");
		(void) command("XCWD %s", argv[1]);
	}
}

/*
 * Set current working directory
 * on local machine.
 */
void
lcd(int argc, char *argv[])
{
	char buf[PATH_MAX];
	const char *dir = NULL;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc == 1) {
	    /*dir = home;*/
	    dir = ".";
	}
	else if (argc != 2) {
		PRINTF("usage: %s local-directory\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	else {
	    dir = globulize(argv[1]);
	}
	if (!dir) {
		lv_ftp->code = 622;
		return;
	}
	if (chdir(dir) < 0) {
		FPRINTF(stderr, "local: %s: %s\n", dir, strerror(errno));
		lv_ftp->code = 622;
		return;
	}
	if (!getcwd(buf, sizeof(buf))) {
	    if (errno==ERANGE) strcpy(buf, "<too long>");
	    else strcpy(buf, "???");
	}
	PRINTF("Local directory now %s\n", buf);
	lv_ftp->code = 0;
}

/*
 * Delete a single file.
 */
void
delete_cmd(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s remote-file\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	(void) command("DELE %s", argv[1]);
}

/*
 * Delete multiple files.
 */
void
mdelete(int argc, char *argv[])
{
	void (*oldintr)(int);
	int ointer;
	char *cp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2 ) {
		PRINTF("usage: %s remote-files\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	lv_ftp->local_mcds.mname = argv[0];
	lv_ftp->mflag = 1;
	oldintr = signal(SIGINT, mabort);
	(void) sigsetjmp(lv_ftp->local_mcds.jabort, 1);
	while ((cp = remglob(argv,0)) != NULL) {
		if (*cp == '\0') {
			lv_ftp->mflag = 0;
			continue;
		}
		if (lv_ftp->mflag && confirm(argv[0], cp)) {
			(void) command("DELE %s", cp);
			if (!lv_ftp->mflag) {
				ointer = lv_ftp->interactive;
				lv_ftp->interactive = 1;
				if (confirm("Continue with", "mdelete")) {
					lv_ftp->mflag++;
				}
				lv_ftp->interactive = ointer;
			}
		}
	}
	(void) signal(SIGINT, oldintr);
	lv_ftp->mflag = 0;
}

/*
 * Rename a remote file.
 */
void
renamefile(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 3) {
		PRINTF("%s from-name to-name\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	if (command("RNFR %s", argv[1]) == CONTINUE)
		(void) command("RNTO %s", argv[2]);
}

/*
 * Get a directory listing
 * of remote files.
 */
void
ls(int argc, char *argv[])
{
	static char foo[2] = "-";
	const char *cmd;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		argc++, argv[1] = NULL;
	}
	if (argc < 3) {
		argc++, argv[2] = foo;
	}
	if (argc > 3) {
		PRINTF("usage: %s remote-directory local-file\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	cmd = argv[0][0] == 'n' ? "NLST" : "LIST";
	if (strcmp(argv[2], "-") && (argv[2] = globulize(argv[2]))==NULL) {
		lv_ftp->code = 622;
		return;
	}
	if (strcmp(argv[2], "-") && *argv[2] != '|')
		if ((argv[2] = globulize(argv[2]))==NULL || 
		    !confirm("output to local-file:", argv[2])) {
			lv_ftp->code = 622;
			return;
	}
	recvrequest(cmd, argv[2], argv[1], "w", 0);
}

/*
 * Get a directory listing
 * of multiple remote files.
 */
void
mls(int argc, char *argv[])
{
	void (*oldintr)(int);
	int ointer, i;
	const char *volatile cmd;
	char *volatile dest;
	const char *modestr;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 3 ) {
		PRINTF("usage: %s remote-files local-file\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	dest = argv[argc - 1];
	argv[argc - 1] = NULL;
	if (strcmp(dest, "-") && *dest != '|')
		if ((dest = globulize(dest))==NULL ||
		    !confirm("output to local-file:", dest)) {
			lv_ftp->code = 622;
			return;
	}
	cmd = argv[0][1] == 'l' ? "NLST" : "LIST";
	lv_ftp->local_mcds.mname = argv[0];
	lv_ftp->mflag = 1;
	oldintr = signal(SIGINT, mabort);

	/*
	 * This just plain seems wrong.
	 */
	(void) sigsetjmp(lv_ftp->local_mcds.jabort, 1);

	for (i = 1; lv_ftp->mflag && i < argc-1; ++i) {
		modestr = (i == 1) ? "w" : "a";
		recvrequest(cmd, dest, argv[i], modestr, 0);
		if (!lv_ftp->mflag) {
			ointer = lv_ftp->interactive;
			lv_ftp->interactive = 1;
			if (confirm("Continue with", argv[0])) {
				lv_ftp->mflag ++;
			}
			lv_ftp->interactive = ointer;
		}
	}
	(void) signal(SIGINT, oldintr);
	lv_ftp->mflag = 0;
}

/*
 * Do a shell escape
 */
void
shell(const char *arg)
{
	int pid;
	void (*old1)(int);
	void (*old2)(int);
	char shellnam[40];
	const char *theshell, *namep; 
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	old1 = signal (SIGINT, SIG_IGN);
	old2 = signal (SIGQUIT, SIG_IGN);
	if ((pid = fork()) == 0) {
		for (pid = 3; pid < 20; pid++)
			(void) close(pid);
		(void) signal(SIGINT, SIG_DFL);
		(void) signal(SIGQUIT, SIG_DFL);
		theshell = getenv("SHELL");
		if (theshell == NULL)
			theshell = _PATH_BSHELL;
		namep = strrchr(theshell, '/');
		if (namep == NULL)
			namep = theshell;
		else 
			namep++;
		(void) strcpy(shellnam,"-");
		(void) strcat(shellnam, namep);
		if (strcmp(namep, "sh") != 0)
			shellnam[0] = '+';
		if (lv_ftp->debug) {
			PRINTF("%s\n", theshell);
		}
		if (arg) {
			execl(theshell, shellnam, "-c", arg, NULL);
		}
		else {
			execl(theshell, shellnam, NULL);
		}
		perror(theshell);
		lv_ftp->code = 622;
		return;
	}
	if (pid > 0) while (wait(NULL) != pid);

	(void) signal(SIGINT, old1);
	(void) signal(SIGQUIT, old2);
	if (pid == -1) {
		perror("Try again later");
		lv_ftp->code = -1;
	}
	else {
		lv_ftp->code = 0;
	}
}

/*
 * Send new user information (re-login)
 */
void
user(int argc, char *argv[])
{
	char theacct[80];
	int n, aflag = 0;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc != 3) {
		PRINTF("usage: %s username [password] [account]\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	n = command("USER %s", argv[1]);
	if (n == CONTINUE) {
		if (argc < 3 )
			argv[2] = getpass("Password: "), argc++;
		n = command("PASS %s", argv[2]);
	}
	if (n == CONTINUE) {
		if (argc < 4) {
			PRINTF("Account: "); 
			fgets(theacct, sizeof(theacct), lv_ftp->STDIN);
			argv[3] = theacct; argc++;
		}
		n = command("ACCT %s", argv[3]);
		aflag++;
	}
	if (n != COMPLETE) {
		PRINTF("%s Login failed.\n", __func__);
		return;
	}
	if (!aflag && argc == 4) {
		(void) command("ACCT %s", argv[3]);
	}
}

/*
 * Print working directory.
 */
void
pwd(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	int oldverbose = lv_ftp->verbose;

	/*
	 * If we aren't verbose, this doesn't do anything!
	 */
	lv_ftp->verbose = 1;
	if (command("PWD") == ERROR && lv_ftp->code == 500) {
		PRINTF("PWD command not recognized, trying XPWD\n");
		(void) command("XPWD");
	}
	lv_ftp->verbose = oldverbose;
}

/*
 * Make a directory.
 */
void
makedir(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2 ) {
		PRINTF("usage: %s directory-name\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	if (command("MKD %s", argv[1]) == ERROR && lv_ftp->code == 500) {
		if (lv_ftp->verbose)
			PRINTF("MKD command not recognized, trying XMKD\n");
		(void) command("XMKD %s", argv[1]);
	}
}

/*
 * Remove a directory.
 */
void
removedir(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s directory-name\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	if (command("RMD %s", argv[1]) == ERROR && lv_ftp->code == 500) {
		if (lv_ftp->verbose)
			PRINTF("RMD command not recognized, trying XRMD\n");
		(void) command("XRMD %s", argv[1]);
	}
}

/*
 * Send a line, verbatim, to the remote machine.
 */
void
quote(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s line-to-send\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	quote1("", argc, argv);
}

/*
 * Send a SITE command to the remote machine.  The line
 * is sent verbatim to the remote machine, except that the
 * word "SITE" is added at the front.
 */
void
site(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s line-to-send\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	quote1("SITE ", argc, argv);
}

/*
 * Turn argv[1..argc) into a space-separated string, then prepend initial text.
 * Send the result as a one-line command and get response.
 */
static void
quote1(const char *initial, int argc, char **argv)
{
	register int i, len;
	char buf[BUFSIZ];		/* must be >= sizeof(line) */

	(void) strcpy(buf, initial);
	if (argc > 1) {
		len = strlen(buf);
		len += strlen(strcpy(&buf[len], argv[1]));
		for (i = 2; i < argc; i++) {
			buf[len++] = ' ';
			len += strlen(strcpy(&buf[len], argv[i]));
		}
	}
	if (command("%s", buf) == PRELIM) {
		while (getreply(0) == PRELIM);
	}
}

void
do_chmod(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 3) {
		PRINTF("usage: %s mode file-name\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	(void) command("SITE CHMOD %s %s", argv[1], argv[2]);
}

void
do_umask(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	int oldverbose = lv_ftp->verbose;

	lv_ftp->verbose = 1;
	(void) command(argc == 1 ? "SITE UMASK" : "SITE UMASK %s", argv[1]);
	lv_ftp->verbose = oldverbose;
}

void
idle_cmd(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	int oldverbose = lv_ftp->verbose;

	lv_ftp->verbose = 1;
	(void) command(argc == 1 ? "SITE IDLE" : "SITE IDLE %s", argv[1]);
	lv_ftp->verbose = oldverbose;
}

/*
 * Ask the other side for help.
 */
void
rmthelp(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	int oldverbose = lv_ftp->verbose;

	lv_ftp->verbose = 1;
	(void) command(argc == 1 ? "HELP" : "HELP %s", argv[1]);
	lv_ftp->verbose = oldverbose;
}

/*
 * Terminate session and exit.
 */
void
quit(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->connected)
		disconnect();
	pswitch(1);
	if (lv_ftp->connected) {
		disconnect();
	}
	if (lv_ftp->fromatty){
		exit(0);
	}
}

/*
 * Terminate session, but don't exit.
 */
void
disconnect(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (!lv_ftp->connected)
		return;
	(void) command("QUIT");
	if (lv_ftp->cout) {
		(void) fclose(lv_ftp->cout);
	}
	lv_ftp->cout = NULL;
	lv_ftp->connected = 0;
	lv_ftp->data = -1;
	if (!lv_ftp->proxy) {
		lv_ftp->macnum = 0;
	}
}

static int
confirm(const char *cmd, const char *file)
{
	char lyne[BUFSIZ];
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return 1;
	};

	if (!lv_ftp->interactive)
		return (1);
		if(!lv_ftp->fromatty)
			return (1);

		PRINTF("%s %s? ", cmd, file);
		if (fgets(lyne, sizeof(lyne), lv_ftp->STDIN) == NULL) {
		    return 0;
		}

	return (*lyne != 'n' && *lyne != 'N');
}



/*
 * Glob a local file name specification with
 * the expectation of a single return value.
 * Can't control multiple values being expanded
 * from the expression, we return only the first.
 */
static 
char *
globulize(char *cpp)
{
	char **globbed;
	char *rv = cpp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

	if (!lv_ftp->doglob) return cpp;

	globbed = ftpglob(cpp);
	if (lv_ftp->globerr != NULL) {
		PRINTF("%s: %s\n", cpp, lv_ftp->globerr);
		if (globbed) {
			blkfree(globbed);
			free(globbed);
		}
		return NULL;
	}
	if (globbed) {
		rv = globbed[0];
		/* don't waste too much memory */
		if (globbed[0]) {
			blkfree(globbed+1);
		}
		free(globbed);
	}
	return rv;
}

void
account(int argc, char *argv[])
{
	char buf[128], *ap;

	if (argc > 1) {
		*buf = 0;
		while (argc > 1) {
			--argc;
			++argv;
			strncat(buf, *argv, sizeof(buf)-strlen(buf));
			buf[sizeof(buf)-1] = 0;
		}
		ap = buf;
	}
	else {
		ap = getpass("Account:");
	}
	command("ACCT %s", ap);
}

static 
void
proxabort(int ignore)
{
	(void)ignore;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (!lv_ftp->proxy) {
		pswitch(1);
	}
	if (lv_ftp->connected) {
		lv_ftp->proxflag = 1;
	}
	else {
		lv_ftp->proxflag = 0;
	}
	pswitch(0);
	siglongjmp(lv_ftp->local_mcds.abortprox,1);
}

void
doproxy(int argc, char *argv[])
{
	register struct cmd *c;
	void (*oldintr)(int);
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s command\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	c = getcmd(argv[1]);
	if (c == (struct cmd *) -1) {
		PRINTF("?Ambiguous command\n");
		lv_ftp->code = 630;
		return;
	}
	if (c == 0) {
		PRINTF("?Invalid command\n");
		lv_ftp->code = 630;
		return;
	}
	if (!c->c_proxy) {
		PRINTF("?Invalid proxy command\n");
		lv_ftp->code = 631;
		return;
	}
	if (sigsetjmp(lv_ftp->local_mcds.abortprox, 1)) {
		lv_ftp->code = 630;
		return;
	}
	oldintr = signal(SIGINT, proxabort);
	pswitch(1);
	if (c->c_conn && !lv_ftp->connected) {
		PRINTF("Not connected\n");
		pswitch(0);
		(void) signal(SIGINT, oldintr);
		lv_ftp->code = 640;
		return;
	}

	if (c->c_handler_v) c->c_handler_v(argc-1, argv+1);
	else if (c->c_handler_0) c->c_handler_0();
	else c->c_handler_1(NULL);  /* should not reach this */

	if (lv_ftp->connected) {
		lv_ftp->proxflag = 1;
	}
	else {
		lv_ftp->proxflag = 0;
	}
	pswitch(0);
	(void) signal(SIGINT, oldintr);
}

void
setcase(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->mcase = !lv_ftp->mcase;
	PRINTF("Case mapping %s.\n", onoff(lv_ftp->mcase));
	lv_ftp->code = lv_ftp->mcase;
}

void
setcr(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->crflag = !lv_ftp->crflag;
	PRINTF("Carriage Return stripping %s.\n", onoff(lv_ftp->crflag));
	lv_ftp->code = lv_ftp->crflag;
}

void
setntrans(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc == 1) {
		lv_ftp->ntflag = 0;
		PRINTF("Ntrans off.\n");
		lv_ftp->code = lv_ftp->ntflag;
		return;
	}
	lv_ftp->ntflag++;
	lv_ftp->code = lv_ftp->ntflag;
	(void) strncpy(lv_ftp->ntin, argv[1], 16);
	lv_ftp->ntin[16] = '\0';
	if (argc == 2) {
		lv_ftp->ntout[0] = '\0';
		return;
	}
	(void) strncpy(lv_ftp->ntout, argv[2], 16);
	lv_ftp->ntout[16] = '\0';
}

static char *
dotrans(char *name)
{
	static char new[PATH_MAX];
	char *cp1, *cp2 = new;
	register int i, ostop, found;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

	for (ostop = 0; *(lv_ftp->ntout + ostop) && ostop < 16; ostop++);
	for (cp1 = name; *cp1; cp1++) {
		found = 0;
		for (i = 0; *(lv_ftp->ntin + i) && i < 16; i++) {
			if (*cp1 == *(lv_ftp->ntin + i)) {
				found++;
				if (i < ostop) {
					*cp2++ = *(lv_ftp->ntout + i);
				}
				break;
			}
		}
		if (!found) {
			*cp2++ = *cp1;
		}
	}
	*cp2 = '\0';
	return(new);
}

void
setnmap(int argc, char *argv[])
{
	char *cp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc == 1) {
		lv_ftp->mapflag = 0;
		PRINTF("Nmap off.\n");
		lv_ftp->code = lv_ftp->mapflag;
		return;
	}
	if (argc < 3) {
		PRINTF("Usage: %s [mapin mapout]\n",argv[0]);
		lv_ftp->code = 622;
		return;
	}
	lv_ftp->mapflag = 1;
	lv_ftp->code = 1;
	cp = index(lv_ftp->altarg, ' ');
	if (lv_ftp->proxy) {
		while(*++cp == ' ');
		lv_ftp->altarg = cp;
		cp = index(lv_ftp->altarg, ' ');
	}
	*cp = '\0';
	(void) strncpy(lv_ftp->mapin, lv_ftp->altarg, PATH_MAX - 1);
	lv_ftp->mapin[PATH_MAX-1] = 0;
	while (*++cp == ' ');
	(void) strncpy(lv_ftp->mapout, cp, PATH_MAX - 1);
	lv_ftp->mapout[PATH_MAX-1] = 0;
}

static
char *
domap(char *name)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};static char new[PATH_MAX];
	register char *cp1 = name, *cp2 =lv_ftp->mapin;
	char *tp[9], *te[9];
	int i, toks[9], toknum = 0, match = 1;


	for (i=0; i < 9; ++i) {
		toks[i] = 0;
	}
	while (match && *cp1 && *cp2) {
		switch (*cp2) {
			case '\\':
				if (*++cp2 != *cp1) {
					match = 0;
				}
				break;
			case '$':
				if (*(cp2+1) >= '1' && *(cp2+1) <= '9') {
					if (*cp1 != *(++cp2+1)) {
						toknum = *cp2 - '1';
						toks[toknum]++;
						tp[toknum] = cp1;
						while (*++cp1 && *(cp2+1)
							!= *cp1);
						te[toknum] = cp1;
					}
					cp2++;
					break;
				}
				/* FALLTHROUGH */
			default:
				if (*cp2 != *cp1) {
					match = 0;
				}
				break;
		}
		if (match && *cp1) {
			cp1++;
		}
		if (match && *cp2) {
			cp2++;
		}
	}
	if (!match && *cp1) /* last token mismatch */
	{
		toks[toknum] = 0;
	}
	cp1 = new;
	*cp1 = '\0';
	cp2 = lv_ftp->mapout;
	while (*cp2) {
		match = 0;
		switch (*cp2) {
			case '\\':
				if (*(cp2 + 1)) {
					*cp1++ = *++cp2;
				}
				break;
			case '[':
LOOP:
				if (*++cp2 == '$' && isdigit(*(cp2+1))) { 
					if (*++cp2 == '0') {
						char *cp3 = name;

						while (*cp3) {
							*cp1++ = *cp3++;
						}
						match = 1;
					}
					else if (toks[toknum = *cp2 - '1']) {
						char *cp3 = tp[toknum];

						while (cp3 != te[toknum]) {
							*cp1++ = *cp3++;
						}
						match = 1;
					}
				}
				else {
					while (*cp2 && *cp2 != ',' && 
					    *cp2 != ']') {
						if (*cp2 == '\\') {
							cp2++;
						}
						else if (*cp2 == '$' &&
   						        isdigit(*(cp2+1))) {
							if (*++cp2 == '0') {
							   char *cp3 = name;

							   while (*cp3) {
								*cp1++ = *cp3++;
							   }
							}
							else if (toks[toknum =
							    *cp2 - '1']) {
							   char *cp3=tp[toknum];

							   while (cp3 !=
								  te[toknum]) {
								*cp1++ = *cp3++;
							   }
							}
						}
						else if (*cp2) {
							*cp1++ = *cp2++;
						}
					}
					if (!*cp2) {
						PRINTF("nmap: unbalanced brackets\n");
						return(name);
					}
					match = 1;
					cp2--;
				}
				if (match) {
					while (*++cp2 && *cp2 != ']') {
					      if (*cp2 == '\\' && *(cp2 + 1)) {
							cp2++;
					      }
					}
					if (!*cp2) {
						PRINTF("nmap: unbalanced brackets\n");
						return(name);
					}
					break;
				}
				switch (*++cp2) {
					case ',':
						goto LOOP;
					case ']':
						break;
					default:
						cp2--;
						goto LOOP;
				}
				break;
			case '$':
				if (isdigit(*(cp2 + 1))) {
					if (*++cp2 == '0') {
						char *cp3 = name;

						while (*cp3) {
							*cp1++ = *cp3++;
						}
					}
					else if (toks[toknum = *cp2 - '1']) {
						char *cp3 = tp[toknum];

						while (cp3 != te[toknum]) {
							*cp1++ = *cp3++;
						}
					}
					break;
				}
				/* intentional drop through */
			default:
				*cp1++ = *cp2;
				break;
		}
		cp2++;
	}
	*cp1 = '\0';
	if (!*new) {
		return(name);
	}
	return(new);
}

void
setsunique(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->sunique = !lv_ftp->sunique;
	PRINTF("Store unique %s.\n", onoff(lv_ftp->sunique));
	lv_ftp->code = lv_ftp->sunique;
}

void
setrunique(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	lv_ftp->runique = !lv_ftp->runique;
	PRINTF("Receive unique %s.\n", onoff(lv_ftp->runique));
	lv_ftp->code = lv_ftp->runique;
}

/* change directory to parent directory */
void
cdup(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (command("CDUP") == ERROR && lv_ftp->code == 500) {
		if (lv_ftp->verbose)
			PRINTF("CDUP command not recognized, trying XCUP\n");
		(void) command("XCUP");
	}
}

/* restart transfer at specific point */
void
restart(int argc, char *argv[])
{

	if (argc != 2)
		PRINTF("restart: offset not specified\n");
	else {
		g_ftp *lv_ftp = get_ftp_local_variable();
		if(!lv_ftp){
			return ;
		};
		lv_ftp->restart_point = atol(argv[1]);
		PRINTF("restarting at %ld. %s\n", lv_ftp->restart_point,
		    "execute get, put or append to initiate transfer");
	}
}

/* show remote system type */
void
syst(void)
{
	command("SYST");
}

void
macdef(int argc, char *argv[])
{
	char *tmp;
	int c;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->macnum == 16) {
		PRINTF("Limit of 16 macros have already been defined\n");
		lv_ftp->code = 622;
		return;
	}
	if (argc < 2) {
		PRINTF("Usage: %s macro_name\n",argv[0]);
		lv_ftp->code = 622;
		return;
	}
	if (lv_ftp->interactive) {
		PRINTF("Enter macro line by line, terminating it with a null line\n");
	}
	(void) strncpy(lv_ftp->macros[lv_ftp->macnum].mac_name, argv[1], 8);
	lv_ftp->macros[lv_ftp->macnum].mac_name[8] = 0;
	if (lv_ftp->macnum == 0) {
		lv_ftp->macros[lv_ftp->macnum].mac_start = lv_ftp->macbuf;
	}
	else {
		lv_ftp->macros[lv_ftp->macnum].mac_start = lv_ftp->macros[lv_ftp->macnum - 1].mac_end + 1;
	}
	tmp = lv_ftp->macros[lv_ftp->macnum].mac_start;
	/* stepping over the end of the array, remember to take away 1! */
	while (tmp != lv_ftp->macbuf+MACBUF_SIZE) {
		if ((c = getchar()) == EOF) {
			PRINTF("macdef:end of file encountered\n");
			lv_ftp->code = 622;
			return;
		}
		if ((*tmp = c) == '\n') {
			if (tmp == lv_ftp->macros[lv_ftp->macnum].mac_start) {
				lv_ftp->macros[lv_ftp->macnum++].mac_end = tmp;
				lv_ftp->code = 0;
				return;
			}
			if (*(tmp-1) == '\0') {
				lv_ftp->macros[lv_ftp->macnum++].mac_end = tmp - 1;
				lv_ftp->code = 0;
				return;
			}
			*tmp = '\0';
		}
		tmp++;
	}
	while (1) {
		while ((c = getchar()) != '\n' && c != EOF)
			/* LOOP */;
		if (c == EOF || getchar() == '\n') {
			PRINTF("Macro not defined - 4k buffer exceeded\n");
			lv_ftp->code = 622;
			return;
		}
	}
}

/*
 * Start up passive mode interaction
 */
void
setpassive(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

    lv_ftp->passivemode = !lv_ftp->passivemode;
    PRINTF("Passive mode %s.\n", onoff(lv_ftp->passivemode));
    lv_ftp->code = lv_ftp->passivemode;
}

/*
 * get size of file on remote machine
 */
void
sizecmd(int argc, char *argv[])
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s filename\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	(void) command("SIZE %s", argv[1]);
}

/*
 * get last modification time of file on remote machine
 */
void
modtime(int argc, char *argv[])
{
	int overbose;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("usage: %s filename\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	overbose = lv_ftp->verbose;
	if (lv_ftp->debug == 0)
		lv_ftp->verbose = -1;
	if (command("MDTM %s", argv[1]) == COMPLETE) {
		int yy, mo, day, hour, min, sec;
		sscanf(lv_ftp->reply_string, "%*s %04d%02d%02d%02d%02d%02d", &yy, &mo,
			&day, &hour, &min, &sec);
		/* might want to print this in local time */
		PRINTF("%s\t%02d/%02d/%04d %02d:%02d:%02d GMT\n", argv[1],
			mo, day, yy, hour, min, sec);
	} else
		PRINTF("%s\n", lv_ftp->reply_string);
	lv_ftp->verbose = overbose;
}

/*
 * show status on remote machine
 */
void
rmtstatus(int argc, char *argv[])
{
	(void) command(argc > 1 ? "STAT %s" : "STAT" , argv[1]);
}

/*
 * get file if modtime is more recent than current file
 */
void
newer(int argc, char *argv[])
{
	if (getit(argc, argv, -1, "w")) {
		/* This should be controlled by some verbose flag */
		PRINTF("Local file \"%s\" is newer than remote file \"%s\"\n",
			argv[2], argv[1]);
	}
}
