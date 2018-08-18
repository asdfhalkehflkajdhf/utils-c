/*
 * Copyright (c) 1985 Regents of the University of California.
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
 * from: @(#)domacro.c	1.8 (Berkeley) 9/28/90
char domacro_rcsid[] = 
  "$Id: domacro.c,v 1.4 1996/08/14 23:27:28 dholland Exp $";
*/

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "ftp_var.h"
extern g_ftp * get_ftp_local_variable(void);

void
domacro(int argc, char *argv[])
{
	int margc;
	char *marg;
	char **margv;
	register int i, j;
	register char *cp1, *cp2;
	int count = 2, loopflg = 0;
	char line2[200];
	struct cmd *c;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc < 2) {
		PRINTF("Usage: %s macro_name.\n", argv[0]);
		lv_ftp->code = 622;
		return;
	}
	for (i = 0; i < lv_ftp->macnum; ++i) {
		if (!strncmp(argv[1], lv_ftp->macros[i].mac_name, 9)) {
			break;
		}
	}
	if (i == lv_ftp->macnum) {
		PRINTF("'%s' macro not found.\n", argv[1]);
		lv_ftp->code = 622;
		return;
	}
	(void) strcpy(line2, lv_ftp->line);
TOP:
	cp1 = lv_ftp->macros[i].mac_start;
	while (cp1 != lv_ftp->macros[i].mac_end) {
		while (isspace(*cp1)) {
			cp1++;
		}
		cp2 = lv_ftp->line;
		while (*cp1 != '\0') {
		      switch(*cp1) {
		   	    case '\\':
				 *cp2++ = *++cp1;
				 break;
			    case '$':
				 if (isdigit(*(cp1+1))) {
				    j = 0;
				    while (isdigit(*++cp1)) {
					  j = 10*j +  *cp1 - '0';
				    }
				    cp1--;
				    if (argc - 2 >= j) {
					(void) strcpy(cp2, argv[j+1]);
					cp2 += strlen(argv[j+1]);
				    }
				    break;
				 }
				 if (*(cp1+1) == 'i') {
					loopflg = 1;
					cp1++;
					if (count < argc) {
					   (void) strcpy(cp2, argv[count]);
					   cp2 += strlen(argv[count]);
					}
					break;
				}
				/* intentional drop through */
			    default:
				*cp2++ = *cp1;
				break;
		      }
		      if (*cp1 != '\0') {
			 cp1++;
		      }
		}
		*cp2 = '\0';
		margv = makeargv(&margc, &marg);
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			PRINTF("?Ambiguous command\n");
			lv_ftp->code = 630;
		}
		else if (c == NULL) {
			PRINTF("?Invalid command\n");
			lv_ftp->code = 631;
		}
		else if (c->c_conn && !lv_ftp->connected) {
			PRINTF("Not connected.\n");
			lv_ftp->code = 640;
		}
		else {
			if (lv_ftp->verbose) {
				PRINTF("%s\n",lv_ftp->line);
			}
			if (c->c_handler_v) c->c_handler_v(margc, margv);
			else if (c->c_handler_0) c->c_handler_0();
			else c->c_handler_1(marg);

			if (lv_ftp->bell && c->c_bell) {
				PUTCHAR('\007');
			}
			(void) strcpy(lv_ftp->line, line2);
			margv = makeargv(&margc, &marg);
			argc = margc;
			argv = margv;
		}
		if (cp1 != lv_ftp->macros[i].mac_end) {
			cp1++;
		}
	}
	if (loopflg && ++count < argc) {
		goto TOP;
	}
}
