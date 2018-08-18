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

char copyright[] =
  "@(#) Copyright (c) 1985, 1989 Regents of the University of California.\n"
  "All rights reserved.\n";

/*
 * from: @(#)main.c	5.18 (Berkeley) 3/1/91
 */
char main_rcsid[] = 
  "$Id: main.c,v 1.15 1999/10/02 13:25:23 netbug Exp $";


/*
 * FTP User Program -- Command Interface.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

/* #include <arpa/ftp.h>	<--- unused? */

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <pwd.h>


#define Extern
#include "ftp_var.h"
#include "libftp.h"

#include <stdio.h>
#include <pthread.h>
#include <string.h>

extern struct cmd cmdtab[];
extern int NCMDS;

void lostpeer(int);
void help(int argc, char *argv[]);

static int cmdscanner(char *cmds);
static char *slurpstring(void);

typedef struct  response_code_
{
	int code;
	char *C_instructions;
	char *E_instructions;
}response_code;

response_code g_resp_code[]={
	{110,	"新文件指示器上的重启标记", 		"The restart of the mark in the new file pointer"},
	{120,	"服务器准备就绪的时间（分钟数）",	"The server is ready time (minutes)"},
	{125,	"打开数据连接，开始传输",			"Transfer of began to open data connection"},
	{150,	"打开连接", 						"Open the connection"},
	{200,	"成功", 							"successful"},
	{202,	"命令没有执行", 					"Command not implemented"},
	{211,	"系统状态回复", 					"The system state to reply"},
	{212,	"目录状态回复", 					"Directory status reply"},
	{213,	"文件状态回复", 					"File status reply"},
	{214,	"帮助信息回复", 					"Help information reply"},
	{215,	"系统类型回复", 					"System type reply"},
	{220,	"服务就绪", 						"Service ready"},
	{221,	"退出网络", 						"Out of the network"},
	{225,	"打开数据连接", 					"To open data connection"},
	{226,	"结束数据连接", 					"The end of the data connection"},
	{227,	"进入被动模式（IP 地址、ID 端口）", "Entering passive mode (IP address, ID port)"},
	{230,	"登录因特网",						"Log on to the Internet"},
	{250,	"文件行为完成", 					"Complete file behavior"},
	{257,	"路径名建立",						"Path name to establish"},
	{331,	"要求密码", 						"Request a password"},
	{332,	"要求帐号", 						"Request account"},
	{350,	"文件行为暂停", 					"File behavior to suspend"},
	{421,	"服务关闭", 						"Service closing"},
	{425,	"无法打开数据连接", 				"Unable to open data connection"},
	{426,	"结束连接", 						"End connections"},
	{450,	"文件不可用",						"The file is not available"},
	{451,	"遇到本地错误", 					"Meet local error"},
	{452,	"磁盘空间不足", 					"There is insufficient space on the disk"},
	{500,	"无效命令", 						"Invalid command"},
	{501,	"错误参数", 						"Error parameters"},
	{502,	"命令没有执行", 					"Command not implemented"},
	{503,	"错误指令序列", 					"The wrong instruction sequence"},
	{504,	"无效命令参数", 					"Invalid command parameters"},
	{530,	"未登录网络",						"Not to log in"},
	{532,	"存储文件需要帐号", 				"Need account for storing files"},
	{550,	"文件不可用",						"The file is not available"},
	{551,	"不知道的页类型",					"The types of pages don't know"},
	{552,	"超过存储分配", 					"More than storage allocation"},
	{553,	"文件名不允许", 					"File name not allowed"},
	//自定义错误
	{600,	"ftp: ftp/tcp: 未知的服务", 		"ftp: ftp/tcp: unknown service"},
	{610,	"内存申请失败", 					"malloc error"},
	{620,	"参数错误", 						"unknown option"},
	{621,	"参数太长", 						"sorry, input option too long"},
	{622,	"参数错误: 无参数", 				"usage: cmd remote-directory"},
	{630,	"命令错误: 不确定", 				"Ambiguous command"},
	{631,	"命令错误: 无效",					"Invalid command"},
	{640,	"连接未找开",						"Not connected."},
	{641,	"连接失败",						    "connected fail."},
	{-1,    "未知错误",                         "unknown error"},
	{0,0,0},
};


static
void
usage(void)
{
	PRINTF("\n\tUsage: { ftp | pftp } [-pinegvtd] [hostname]\n");
	PRINTF("\t   -p: enable passive mode (default for pftp)\n");
	PRINTF("\t   -i: turn off prompting during mget\n");
	PRINTF("\t   -n: inhibit auto-login\n");
	PRINTF("\t   -e: disable readline support, if present\n");
	PRINTF("\t   -g: disable filename globbing\n");
	PRINTF("\t   -v: verbose mode\n");
	PRINTF("\t   -t: enable packet tracing [nonfunctional]\n");
	PRINTF("\t   -d: enable debugging\n");
	PRINTF("\n");
}
pthread_key_t g_ftp_key;

int g_ftp_debug = 0;
/*
	如果参数为debug则开启调试信息
*/
int global_ftp_client_init(int option)
{
	int ret = pthread_key_create(&g_ftp_key, NULL);
	if( ret != 0 ) {
		PRINTF("Failed to set threadlocal storage key for threadvar\n");
		return -1;
	}
	if( option == GLOBAL_FTP_INIT_DEBUG)
		g_ftp_debug++;
	return 0;
}
int global_ftp_client_exit(void)
{
	pthread_key_delete(g_ftp_key);
	return 0;
}
extern g_ftp * set_ftp_local_variable(g_ftp *lv_ftp);

g_ftp * set_ftp_local_variable(g_ftp *lv_ftp)
{
	int ret;
	g_ftp *lv_ftp_tmp=NULL;
	if(lv_ftp == NULL){
		lv_ftp_tmp = (g_ftp *)malloc(sizeof(g_ftp));
		if(lv_ftp_tmp == NULL)
			return NULL;
	}else{
		lv_ftp_tmp = lv_ftp;
	}
	ret = pthread_setspecific(g_ftp_key, lv_ftp_tmp);
	if(ret == 0 )
		return lv_ftp_tmp;
	else{
		if(lv_ftp == NULL)
			free(lv_ftp_tmp);
		return NULL;
	}
}
extern g_ftp * get_ftp_local_variable(void);

g_ftp * get_ftp_local_variable(void )
{
//g_ftp *lv_ftp = get_ftp_local_variable();
	g_ftp *lv_ftp = (g_ftp *)pthread_getspecific(g_ftp_key);
	return lv_ftp;
}

int ftp_client_exit(void)
{
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	if(lv_ftp->STDOUT)
		fclose(lv_ftp->STDOUT);
	if(lv_ftp->fromatty == 0){
		
	}
	
	free(lv_ftp);

	return 0;

}
int ftp_client_init(char *argv)
{
	register char *cp;
	struct servent *sp;
	struct passwd *pw = NULL;
	char homedir[MAXPATHLEN];

	g_ftp *lv_ftp = set_ftp_local_variable(NULL);
	if(!lv_ftp)
		return 610;

	lv_ftp->tick = 0;

	sp = getservbyname("ftp", "tcp");
	if (sp == 0) {
		return 600;
	}
	/*main.c*/
	
	lv_ftp->home = "/";
	lv_ftp->traceflag = 0;
	
	lv_ftp->ftp_port = sp->s_port;
	lv_ftp->doglob = 1;
	lv_ftp->interactive = 1;
	lv_ftp->passivemode = 0;

	lv_ftp->verbose = 0;
	lv_ftp->debug = 0;

	//fromatty = isatty(fileno(stdin));
	lv_ftp->fromatty = 0;
	lv_ftp->STDIN = stdin;
	lv_ftp->STDOUT = stdout;


	for (cp = argv; argv!=NULL && *cp != '\0'; cp++)
		switch (*cp) {

		case 'd':
			lv_ftp->options |= SO_DEBUG;
			if(g_ftp_debug == GLOBAL_FTP_INIT_DEBUG)
				lv_ftp->debug++;
			break;
		
		case 'v':
			lv_ftp->verbose++;
			break;

		case 't':
			lv_ftp->traceflag++;
			break;

		case 'i':
			lv_ftp->interactive = 0;
			break;

		case 'p':
			lv_ftp->passivemode = 1;
			break;

		case 'g':
			lv_ftp->doglob = 0;
			break;
			
		case 'e':
			lv_ftp->rl_inhibit = 1;
			break;
			
		case 'h':
			usage();
			break;

		default:
			return 620;
		}

	lv_ftp->cpend = 0;	/* no pending replies */
	lv_ftp->proxy = 0;	/* proxy not active */
	lv_ftp->crflag = 1;	/* strip c.r. on ascii gets */
	lv_ftp->sendport = -1;	/* not using ports */
	/*
	 * Set up the home directory in case we're globbing.
	 */
	cp = getlogin();
	if (cp != NULL) {
		pw = getpwnam(cp);
	}
	if (pw == NULL)
		pw = getpwuid(getuid());
	if (pw != NULL) {
		strncpy(homedir, pw->pw_dir, sizeof(homedir));
		homedir[sizeof(homedir)-1] = 0;
		lv_ftp->home = homedir;
	}
/*ftp.c*/
	
	lv_ftp->data = -1;
	lv_ftp->restart_point = 0;

	lv_ftp->local_ftp.abrtflag = 0;
	lv_ftp->local_ftp.ptabflg = 0;
	lv_ftp->local_ftp.ptflag = 0;

/*glob.c*/
	lv_ftp->local_glob.globchars ="`{[*?";

/*cmds.c*/
	lv_ftp->local_mcds.types[0].t_name = "ascii";
	lv_ftp->local_mcds.types[0].t_mode = "A";
	lv_ftp->local_mcds.types[0].t_type = TYPE_A;
	lv_ftp->local_mcds.types[0].t_arg = NULL;

	lv_ftp->local_mcds.types[1].t_name = "binary";
	lv_ftp->local_mcds.types[1].t_mode = "I";
	lv_ftp->local_mcds.types[1].t_type = TYPE_I;
	lv_ftp->local_mcds.types[1].t_arg = NULL;

	lv_ftp->local_mcds.types[2].t_name = "image";
	lv_ftp->local_mcds.types[2].t_mode = "I";
	lv_ftp->local_mcds.types[2].t_type = TYPE_I;
	lv_ftp->local_mcds.types[2].t_arg = NULL;

	lv_ftp->local_mcds.types[3].t_name = "ebcdic";
	lv_ftp->local_mcds.types[3].t_mode = "E";
	lv_ftp->local_mcds.types[3].t_type = TYPE_E;
	lv_ftp->local_mcds.types[3].t_arg = NULL;

	lv_ftp->local_mcds.types[4].t_name = "tenex";
	lv_ftp->local_mcds.types[4].t_mode = "L";
	lv_ftp->local_mcds.types[4].t_type = TYPE_L;
	lv_ftp->local_mcds.types[4].t_arg = lv_ftp->bytename;

	lv_ftp->local_mcds.types[5].t_name = NULL;
	lv_ftp->local_mcds.types[5].t_mode = NULL;
	lv_ftp->local_mcds.types[5].t_type = 0;
	lv_ftp->local_mcds.types[5].t_arg = NULL;

	(void) signal(SIGPIPE, lostpeer);
	return 0;

}

int ftp_client_cmd(char *cmds)
{
	int code;
	if(cmds == NULL)
		return 631;
	code = cmdscanner(cmds);
	if(code > 400)
		return code;
	return 0;
}
/*通过code码获取英文说明*/
char *ftp_get_E_errors(int code)
{
	int i;
	for(i=0;g_resp_code[i].code!=0; i++){
		if(g_resp_code[i].code == code)
			return g_resp_code[i].E_instructions;
	};
	return NULL;
}
/*通过code码获取中文说明*/
char *ftp_get_C_errors(int code)
{
	int i;
	for(i=0;g_resp_code[i].code!=0; i++){
		if(g_resp_code[i].code == code)
			return g_resp_code[i].C_instructions;
	};
	return NULL;
}
#if 0
int
main(volatile int argc, char **volatile argv)
{
	(void)argv;
	(void)argc;
	int ret;

	global_ftp_client_init(GLOBAL_FTP_INIT_DEBUG);

	ret = ftp_client_init("d");
	if(ret !=0){
		return ret;
	}
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};


	//setpeer(argc, &argv[0]);
	ret = ftp_client_cmd("open 192.168.200.242 liuzc lzc123");
	printf("ftp_test %d\n",ret);
	ret = ftp_client_cmd("ls");
	printf("ftp_test %d\n",ret);

	ret = ftp_client_cmd("mkdir test");
	printf("ftp_test %d\n",ret);
	ret = ftp_client_cmd("help");
	printf("ftp_test %d\n",ret);
	ret = ftp_client_cmd("quit");
	printf("ftp_test %d\n",ret);
	
	global_ftp_client_exit();
	return 0;
}
#endif

void
lostpeer(int ignore)
{
	(void)ignore;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (lv_ftp->connected) {
		if (lv_ftp->cout != NULL) {
			shutdown(fileno(lv_ftp->cout), 1+1);
			fclose(lv_ftp->cout);
			lv_ftp->cout = NULL;
		}
		if (lv_ftp->data >= 0) {
			shutdown(lv_ftp->data, 1+1);
			close(lv_ftp->data);
			lv_ftp->data = -1;
		}
		lv_ftp->connected = 0;
	}
	pswitch(1);
	if (lv_ftp->connected) {
		if (lv_ftp->cout != NULL) {
			shutdown(fileno(lv_ftp->cout), 1+1);
			fclose(lv_ftp->cout);
			lv_ftp->cout = NULL;
		}
		lv_ftp->connected = 0;
	}
	lv_ftp->proxflag = 0;
	pswitch(0);
}


/*
 * Command parser.
 */
static int
cmdscanner(char *cmds)
{
	int margc;
	char *marg;
	char **margv;
	register struct cmd *c;
	register int l;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return -1;
	};

	lv_ftp->line[0]='\0';
	if(cmds == NULL)
		return -4;
	l = strlen(cmds);
	if (l == 0)
		return -4;
	if(l > (int)sizeof(lv_ftp->line)){
		return 621;
	}
	strcpy(lv_ftp->line, cmds);
	if (lv_ftp->line[--l] == '\n') {
		if (l == 0)
			return 620;
		lv_ftp->line[l] = '\0';
	} 

	margv = makeargv(&margc, &marg);
	if (margc == 0) {
		return 620;
	}
	c = getcmd(margv[0]);
	if (c == (struct cmd *)-1) {
		return 630;
	}
	if (c == NULL) {
		return 631;
	}
	if (c->c_conn && !lv_ftp->connected) {
		return 640;
	}
	if (c->c_handler_v) c->c_handler_v(margc, margv);
	else if (c->c_handler_0) c->c_handler_0();
	else c->c_handler_1(marg);

	if (lv_ftp->bell && c->c_bell) PUTCHAR('\007');


	(void) signal(SIGPIPE, lostpeer);
	return lv_ftp->code;
}

struct cmd *
getcmd(const char *name)
{
	const char *p, *q;
	struct cmd *c, *found;
	int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; (p = c->c_name) != NULL; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */


char **
makeargv(int *pargc, char **parg)
{
	static char *rargv[20];
	int rargc = 0;
	char **argp;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

	argp = rargv;
	lv_ftp->stringbase = lv_ftp->line;		/* scan from first of buffer */
	lv_ftp->argbase = lv_ftp->argbuf;		/* store from first of buffer */
	while ((*argp++ = slurpstring())!=NULL)
		rargc++;

	*pargc = rargc;
	if (parg) *parg = lv_ftp->altarg;
	return rargv;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
static
char *
slurpstring(void)
{
	int slrflag = 0;
	static char excl[] = "!", dols[] = "$";
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return NULL;
	};

	int got_one = 0;
	register char *sb = lv_ftp->stringbase;
	register char *ap = lv_ftp->argbase;
	char *tmp = lv_ftp->argbase;		/* will return this if token found */

	if (*sb == '!' || *sb == '$') {	/* recognize ! as a token for shell */
		switch (slrflag) {	/* and $ as token for macro invoke */
			case 0:
				slrflag++;
				lv_ftp->stringbase++;
				return ((*sb == '!') ? excl : dols);
				/* NOTREACHED */
			case 1:
				slrflag++;
				lv_ftp->altarg = lv_ftp->stringbase;
				break;
			default:
				break;
		}
	}

S0:
	switch (*sb) {

	case '\0':
		goto OUT;

	case ' ':
	case '\t':
		sb++; goto S0;

	default:
		switch (slrflag) {
			case 0:
				slrflag++;
				break;
			case 1:
				slrflag++;
				lv_ftp->altarg = sb;
				break;
			default:
				break;
		}
		goto S1;
	}

S1:
	switch (*sb) {

	case ' ':
	case '\t':
	case '\0':
		goto OUT;	/* end of token */

	case '\\':
		sb++; goto S2;	/* slurp next character */

	case '"':
		sb++; goto S3;	/* slurp quoted string */

	default:
		*ap++ = *sb++;	/* add character to token */
		got_one = 1;
		goto S1;
	}

S2:
	switch (*sb) {

	case '\0':
		goto OUT;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S1;
	}

S3:
	switch (*sb) {

	case '\0':
		goto OUT;

	case '"':
		sb++; goto S1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S3;
	}

OUT:
	if (got_one)
		*ap++ = '\0';
	lv_ftp->argbase = ap;			/* update storage pointer */
	lv_ftp->stringbase = sb;		/* update scan pointer */
	if (got_one) {
		return(tmp);
	}
	switch (slrflag) {
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			lv_ftp->altarg = NULL;
			break;
		default:
			break;
	}
	return NULL;
}

#define HELPINDENT ((int) sizeof ("directory"))

/*
 * Help command.
 * Call each command handler with argc == 0 and argv[0] == name.
 */
void
help(int argc, char *argv[])
{
	struct cmd *c;
	g_ftp *lv_ftp = get_ftp_local_variable();
	if(!lv_ftp){
		return ;
	};

	if (argc == 1) {
		int i, j, w;
		unsigned k;
		int columns, width = 0, lines;

		PRINTF("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if (c->c_name && (!lv_ftp->proxy || c->c_proxy)) {
					PRINTF("%s", c->c_name);
				}
				else if (c->c_name) {
					for (k=0; k < strlen(c->c_name); k++) {
						PUTCHAR(' ');
					}
				}
				if (c + lines >= &cmdtab[NCMDS]) {
					PRINTF("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					PUTCHAR('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			PRINTF("?Ambiguous help command %s\n", arg);
		else if (c == NULL)
			PRINTF("?Invalid help command %s\n", arg);
		else
			PRINTF("%-*s\t%s\n", HELPINDENT,
				c->c_name, c->c_help);
	}
}
