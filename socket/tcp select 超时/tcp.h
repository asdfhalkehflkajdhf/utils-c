

#define INVALID_SOCKET -1




int select_fd_init(fd_set *fdset);
int select_fd_del(fd_set *fdset, int fd);
int select_fd_add(fd_set *fdset, int fd);
int select_fd_check_r(fd_set *fdset_r, int fd, int sec, int usec);
int select_fd_check_w(fd_set *fdset_w, int fd, int sec, int usec);
int select_fd_check_rw(fd_set *fdset_r, fd_set *fdset_w,int fd, int sec, int usec);

int select_fd_check2(fd_set *fdset, int fd);
int select_fd_timeout_r(int fd, int sec, int usec);
int select_fd_timeout_w(int fd, int sec, int usec);
int select_fd_timeout_rw(int fd, int sec, int usec);

int CloseSocket(int sockfd);

int Writen(int sckid, unsigned char *buf, int len, int sec, int usec);
int Readn(int sckid, char *buf, int len, int sec, int usec);
int ConnectRemote(char *ip, int port, int sec, int usec);
int ListenRemote(int port);
int AcceptRemote(int sockfd);
void setOpetinNoBlock(int fd);
