#ifndef __LIBFTP_H__
#define __LIBFTP_H__

/*
每个程序只能使用一次 global_ftp_client_init

*/
enum {
	GLOBAL_FTP_INIT_DEFAULT,
	GLOBAL_FTP_INIT_DEBUG,
};

extern int global_ftp_client_init(int option);
extern int global_ftp_client_exit(void);

extern int ftp_client_exit(void);

/*
           -p: enable passive mode
           -i: turn off prompting during mget
           -n: inhibit auto-login
           -g: disable filename globbing
           -m: don't force data channel interface to the same as control channel
           -v: verbose mode ,need on -d
           -t: enable packet tracing [nonfunctional]
           -d: enable debugging, 只有在GLOBAL_FTP_INIT_DEBUG 配置才可能有用

	每个线程只能执行一次
	返回值:-1未知错误; >400可以ftp_get_E_errors 错误信息
*/
extern int ftp_client_init(char *argv);

/*
!               debug           mdir            sendport        site
$               dir             mget            put             size
account         disconnect      mkdir           pwd             status
append          exit            mls             quit            struct
ascii           form            mode            quote           system
bell            get             modtime         recv            sunique
binary          glob            mput            reget           tenex
bye             hash            newer           rstatus         tick
case            help            nmap            rhelp           trace
cd              idle            nlist           rename          type
cdup            image           ntrans          reset           user
chmod           lcd             open            restart         umask
close           ls              prompt          rmdir           verbose
cr              macdef          passive         runique         ?
delete          mdelete         proxy           send
*/
extern int ftp_client_cmd(char *cmds);
/*通过code码获取英文说明*/
extern char *ftp_get_E_errors(int code);
/*通过code码获取中文说明*/
extern char *ftp_get_C_errors(int code);


#endif
