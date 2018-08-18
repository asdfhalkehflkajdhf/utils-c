/*
 * Copyright (c) 1980 Regents of the University of California.
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
 * From: @(#)glob.c	5.9 (Berkeley) 2/25/91
 */
char glob_rcsid[] = 
  "$Id: glob.c,v 1.9 1999/10/02 13:25:23 netbug Exp $";

/*
 * C-shell glob for random programs.
 */

#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>

#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ftp_var.h"  /* for protos only */
#include "glob.h"


#define ARG_MAX 4096
#define	QUOTE 0200
#define	TRIM 0177
#define	eq(a,b)		(strcmp(a, b)==0)
#define	GAVSIZ		(ARG_MAX/6)
#define	isdir(d)	((d.st_mode & S_IFMT) == S_IFDIR)




//l_glob local_glob={0,0,0,0,0,0,"`{[*?", 0, 0, 0, 0, 0};

//local_glob.globchars ="`{[*?";


static int tglob(char c);
static char *strspl(const char *, const char *);
static char *strend(char *);
static char **copyblk(entry *);
static char **cloneblk(const centry *);

static void addpath(char c);
static void acollect(const char *);
static void collect(const char *);
static void expand(const char *);
static void Gcat(const char *, const char *);
static void ginit(entry *);
static void matchdir(const char *);
static void rscan(centry *, int (*f)(char));
static void sort(void);
static void efree(entry *);
static int amatch(const char *, const char *);
static int execbrc(const char *, const char *);
static int match(const char *, const char *);

static int gethdir(char *homedir);
static int letter(char c);
static int digit(char c);
static int any(int c, const char *s);
extern g_ftp * get_ftp_local_variable(void);

char **
ftpglob(const char *v)
{
	char agpath[BUFSIZ];
	entry agargv[GAVSIZ];
	centry vv[2];
	vv[0].text = v;
	vv[1].text = NULL;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};
	lv_ftp->local_glob.gflag = 0;
	rscan(vv, tglob);
	if (lv_ftp->local_glob.gflag == 0) {
		return cloneblk(vv);
	}

	lv_ftp->globerr = 0;

	lv_ftp->local_glob.gpath = agpath; 
	lv_ftp->local_glob.gpathp = lv_ftp->local_glob.gpath; 
	*lv_ftp->local_glob.gpathp = 0;
	/* added ()'s to sizeof, (ambigious math for the compiler) */
	lv_ftp->local_glob.lastgpathp = agpath + (sizeof(agpath)- 2);

	ginit(agargv); 
	lv_ftp->local_glob.globcnt = 0;
	collect(v);
	if (lv_ftp->local_glob.globcnt == 0 && (lv_ftp->local_glob.gflag&1)) {
		efree(lv_ftp->local_glob.gargv);
		lv_ftp->local_glob.gargv = NULL;
		return NULL;
	} 
	else {
		char **rv = copyblk(lv_ftp->local_glob.gargv);
		lv_ftp->local_glob.gargv = NULL;
		return rv;
	}
}

static 
void
ginit(entry *agargv)
{
	agargv[0].text = NULL; 
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};
	lv_ftp->local_glob.gargv = agargv; 
	lv_ftp->local_glob.sortbas = agargv; 
	lv_ftp->local_glob.gargc = 0;
	lv_ftp->local_glob.gnleft = ARG_MAX - 4;
}

static 
void
collect(const char *as)
{
	if (eq(as, "{") || eq(as, "{}")) {
		Gcat(as, "");
		sort();
	} 
	else {
		acollect(as);
	}
}

static 
void
acollect(const char *as)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	int ogargc = lv_ftp->local_glob.gargc;

	lv_ftp->local_glob.gpathp = lv_ftp->local_glob.gpath; *lv_ftp->local_glob.gpathp = 0; lv_ftp->local_glob.globbed = 0;
	expand(as);
	if (lv_ftp->local_glob.gargc != ogargc)
		sort();
}

static 
void
sort(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	entry *p1, *p2, c;
	entry *Gvp = &lv_ftp->local_glob.gargv[lv_ftp->local_glob.gargc];

	p1 = lv_ftp->local_glob.sortbas;
	while (p1 < Gvp-1) {
		p2 = p1;
		while (++p2 < Gvp)
			if (strcmp(p1->text, p2->text) > 0)
				c = *p1, *p1 = *p2, *p2 = c;
		p1++;
	}
	lv_ftp->local_glob.sortbas = Gvp;
}

static 
void
expand(const char *as)
{
	const char *cs;
	const char *oldcs;
	char *sgpathp;
	struct stat stb;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	sgpathp = lv_ftp->local_glob.gpathp;
	cs = as;
	if (*cs == '~' && lv_ftp->local_glob.gpathp == lv_ftp->local_glob.gpath) {
		addpath('~');
		for (cs++; letter(*cs) || digit(*cs) || *cs == '-';)
			addpath(*cs++);
		if (!*cs || *cs == '/') {
			if (lv_ftp->local_glob.gpathp != lv_ftp->local_glob.gpath + 1) {
				*lv_ftp->local_glob.gpathp = 0;
				if (gethdir(lv_ftp->local_glob.gpath + 1))
					lv_ftp->globerr = "Unknown user name after ~";
				/*
				 * Was: strcpy(gpath, gpath + 1);
				 * but that's WRONG
				 */
				memmove(lv_ftp->local_glob.gpath, lv_ftp->local_glob.gpath+1, strlen(lv_ftp->local_glob.gpath+1)+1);
			} 
			else {
				(void) strcpy(lv_ftp->local_glob.gpath, lv_ftp->home);
			}
			lv_ftp->local_glob.gpathp = strend(lv_ftp->local_glob.gpath);
		}
	}
	while (!any(*cs, lv_ftp->local_glob.globchars)) {
		if (*cs == 0) {
			if (!lv_ftp->local_glob.globbed)
				Gcat(lv_ftp->local_glob.gpath, "");
			else if (stat(lv_ftp->local_glob.gpath, &stb) >= 0) {
				Gcat(lv_ftp->local_glob.gpath, "");
				lv_ftp->local_glob.globcnt++;
			}
			goto endit;
		}
		addpath(*cs++);
	}
	oldcs = cs;
	while (cs > as && *cs != '/')
		cs--, lv_ftp->local_glob.gpathp--;
	if (*cs == '/')
		cs++, lv_ftp->local_glob.gpathp++;
	*lv_ftp->local_glob.gpathp = 0;
	if (*oldcs == '{') {
		(void) execbrc(cs, ((char *)0));
		return;
	}
	matchdir(cs);
endit:
	lv_ftp->local_glob.gpathp = sgpathp;
	*lv_ftp->local_glob.gpathp = 0;
}

static 
void
matchdir(const char *pattern)
{
	struct stat stb;
	register struct dirent *dp;
	DIR *dirp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

#if 0
#ifdef	__linux__
	if (lv_ftp->local_glob.gpath == NULL || *lv_ftp->local_glob.gpath == '\0')
		lv_ftp->local_glob.gpath = "./";
#endif
#endif
	dirp = opendir((!lv_ftp->local_glob.gpath || !*lv_ftp->local_glob.gpath) ? "./" : lv_ftp->local_glob.gpath);
	if (dirp == NULL) {
		if (lv_ftp->local_glob.globbed)
			return;
		goto patherr2;
	}
	if (fstat(dirfd(dirp), &stb) < 0)
		goto patherr1;
	if (!isdir(stb)) {
		errno = ENOTDIR;
		goto patherr1;
	}
	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_ino == 0)
			continue;
		if (match(dp->d_name, pattern)) {
			Gcat(lv_ftp->local_glob.gpath, dp->d_name);
			lv_ftp->local_glob.globcnt++;
		}
	}
	closedir(dirp);
	return;

patherr1:
	closedir(dirp);
patherr2:
	lv_ftp->globerr = "Bad directory components";
}

static 
int
execbrc(const char *p, const char *s)
{
	char restbuf[BUFSIZ + 2];
	const char *pe, *pm, *pl;
	int brclev = 0;
	char *lm, *sgpathp;

	for (lm = restbuf; *p != '{'; *lm++ = *p++)
		continue;
	for (pe = ++p; *pe; pe++)
	switch (*pe) {

	case '{':
		brclev++;
		continue;

	case '}':
		if (brclev == 0)
			goto pend;
		brclev--;
		continue;

	case '[':
		for (pe++; *pe && *pe != ']'; pe++)
			continue;
		continue;
	}
pend:
	brclev = 0;
	for (pl = pm = p; pm <= pe; pm++)
	switch (*pm & (QUOTE|TRIM)) {

	case '{':
		brclev++;
		continue;

	case '}':
		if (brclev) {
			brclev--;
			continue;
		}
		goto doit;

	case ','|QUOTE:
	case ',':
		if (brclev)
			continue;
doit:
#if 0
		savec = *pm;
		*pm = 0;
		strcpy(lm, pl);
		*pm = savec;
#else
		strncpy(lm, pl, pm-pl);
		lm[pm-pl] = 0;
#endif
		(void) strcat(restbuf, pe + 1);
		if (s == 0) {
			g_ftp *lv_ftp = get_ftp_local_variable();
			if(!lv_ftp){
				return -1;
			};
			sgpathp = lv_ftp->local_glob.gpathp;
			expand(restbuf);
			lv_ftp->local_glob.gpathp = sgpathp;
			*lv_ftp->local_glob.gpathp = 0;
		} 
		else if (amatch(s, restbuf)) {
			return (1);
		}
		sort();
		pl = pm + 1;
		if (brclev)
			return (0);
		continue;

	case '[':
		for (pm++; *pm && *pm != ']'; pm++)
			continue;
		if (!*pm)
			pm--;
		continue;
	}
	if (brclev)
		goto doit;
	return (0);
}

static 
int
match(const char *s, const char *p)
{
	int c;
	const char *sentp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};
	char sglobbed = lv_ftp->local_glob.globbed;

	if (*s == '.' && *p != '.')
		return (0);
	sentp = lv_ftp->local_glob.entp;
	lv_ftp->local_glob.entp = s;
	c = amatch(s, p);
	lv_ftp->local_glob.entp = sentp;
	lv_ftp->local_glob.globbed = sglobbed;
	return (c);
}

static 
int
amatch(const char *s, const char *p)
{
	register int scc;
	int ok, lc;
	char *sgpathp;
	struct stat stb;
	int c, cc;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	lv_ftp->local_glob.globbed = 1;
	for (;;) {
		scc = *s++ & TRIM;
		switch (c = *p++) {

		case '{':
			return (execbrc(p - 1, s - 1));

		case '[':
			ok = 0;
			lc = 077777;
			while ((cc = *p++) != 0) {
				if (cc == ']') {
					if (ok)
						break;
					return (0);
				}
				if (cc == '-') {
					if (lc <= scc && scc <= *p++)
						ok++;
				} else
					if (scc == (lc = cc))
						ok++;
			}
			if (cc == 0) {
				if (ok)
					p--;
				else
					return 0;
			}
			continue;

		case '*':
			if (!*p)
				return (1);
			if (*p == '/') {
				p++;
				goto slash;
			}
			s--;
			do {
				if (amatch(s, p))
					return (1);
			} while (*s++);
			return (0);

		case 0:
			return (scc == 0);

		default:
			if (c != scc)
				return (0);
			continue;

		case '?':
			if (scc == 0)
				return (0);
			continue;

		case '/':
			if (scc)
				return (0);
slash:
			s = lv_ftp->local_glob.entp;
			sgpathp = lv_ftp->local_glob.gpathp;
			while (*s)
				addpath(*s++);
			addpath('/');
			if (stat(lv_ftp->local_glob.gpath, &stb) == 0 && isdir(stb)) {
				if (*p == 0) {
					Gcat(lv_ftp->local_glob.gpath, "");
					lv_ftp->local_glob.globcnt++;
				} else {
					expand(p);
				}
			}
			lv_ftp->local_glob.gpathp = sgpathp;
			*lv_ftp->local_glob.gpathp = 0;
			return (0);
		}
	}
}

#if 0 /* dead code */
static 
int
Gmatch(const char *s, const char *p)
{
	register int scc;
	int ok, lc;
	int c, cc;

	for (;;) {
		scc = *s++ & TRIM;
		switch (c = *p++) {

		case '[':
			ok = 0;
			lc = 077777;
			while ((cc = *p++) != 0) {
				if (cc == ']') {
					if (ok)
						break;
					return (0);
				}
				if (cc == '-') {
					if (lc <= scc && scc <= *p++)
						ok++;
				} else
					if (scc == (lc = cc))
						ok++;
			}
			if (cc == 0)
				if (ok)
					p--;
				else
					return 0;
			continue;

		case '*':
			if (!*p)
				return (1);
			for (s--; *s; s++)
				if (Gmatch(s, p))
					return (1);
			return (0);

		case 0:
			return (scc == 0);

		default:
			if ((c & TRIM) != scc)
				return (0);
			continue;

		case '?':
			if (scc == 0)
				return (0);
			continue;

		}
	}
}
#endif

static 
void
Gcat(const char *s1, const char *s2)
{
	int len = strlen(s1) + strlen(s2) + 1;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (len >= lv_ftp->local_glob.gnleft || lv_ftp->local_glob.gargc >= GAVSIZ - 1) {
		lv_ftp->globerr = "Arguments too long";
	}
	else {
		lv_ftp->local_glob.gargc++;
		lv_ftp->local_glob.gnleft -= len;
		lv_ftp->local_glob.gargv[lv_ftp->local_glob.gargc].text = NULL;
		lv_ftp->local_glob.gargv[lv_ftp->local_glob.gargc - 1].text = strspl(s1, s2);
	}
}

static 
void
addpath(char c)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->local_glob.gpathp >= lv_ftp->local_glob.lastgpathp)
		lv_ftp->globerr = "Pathname too long";
	else {
		*lv_ftp->local_glob.gpathp++ = c;
		*lv_ftp->local_glob.gpathp = 0;
	}
}

static void
rscan(centry *t, int (*f)(char))
{
	const char *p;
	char c;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	while ((p = (t++)->text) != NULL) {
		if (f == tglob) {
			if (*p == '~')
				lv_ftp->local_glob.gflag |= 2;
			else if (eq(p, "{") || eq(p, "{}"))
				continue;
		}
		while ((c = *p++) != 0)
			(*f)(c);
	}
}

static 
int
tglob(char c)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	if (any(c, lv_ftp->local_glob.globchars))
		lv_ftp->local_glob.gflag |= c == '{' ? 2 : 1;
	return (c);
}

static int
letter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int
digit(char c)
{
	return (c >= '0' && c <= '9');
}

static 
int
any(int c, const char *s)
{
	while (*s) if (*s++ == c) return 1;
	return 0;
}

static 
int
cblklen(const centry *av)
{
	int i = 0;
	while ((av++)->text) i++;
	return i;
}

static 
int
blklen(const entry *av)
{
	int i = 0;
	while ((av++)->text) i++;
	return i;
}

void
blkfree(char **av)
{
	int i;
	for (i=0; av[i]; i++) free(av[i]);
}

static
void 
efree(entry *av) 
{
    int i;
    for (i=0; av[i].text; i++) free(av[i].text);
}
void fatal(const char *msg);

void
fatal(const char *msg)
{

	FPRINTF(stderr, "ftp: %s\n", msg);
	exit(1);
}

static
char *
strspl(const char *cp, const char *dp)
{
	char *ep = malloc(strlen(cp) + strlen(dp) + 1);
	if (ep == NULL)	fatal("Out of memory");

	strcpy(ep, cp);
	strcat(ep, dp);
	return ep;
}

static 
char **
copyblk(entry *v)
{
	int i;
	char **nv = malloc((blklen(v) + 1) * sizeof(char **));
	if (nv == NULL) fatal("Out of memory");

	for (i=0; v[i].text; i++) {
	    nv[i] = v[i].text;
	    v[i].text = NULL;
	}
	nv[i] = NULL;

	return nv;
}

static
char **
cloneblk(const centry *v)
{
	int i;
	char **nv = malloc((cblklen(v) + 1) * sizeof(char **));
	if (nv == NULL) fatal("Out of memory");

	for (i=0; v[i].text; i++) {
	    nv[i] = strdup(v[i].text);
	}
	nv[i] = NULL;

	return nv;
}

static
char *
strend(char *cp)
{
	while (*cp)
		cp++;
	return (cp);
}
/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 *
 * XXX, this needs buffer length checking and stuff.
 */
static 
int
gethdir(char *homedir)
{
	register struct passwd *pp = getpwnam(homedir);
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return 1;
	};

	if (!pp || homedir + strlen(pp->pw_dir) >= lv_ftp->local_glob.lastgpathp)
		return 1;
	strcpy(homedir, pp->pw_dir);
	return 0;
}
