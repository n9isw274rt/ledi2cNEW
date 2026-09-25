#ifndef SOCKET_H
#define SOCKET_H

int  socket_init(const char *path);
int  socket_get_listen_fd(void);
int  socket_accept(void);
int  socket_is_client(int fd);
int  socket_recv_line(int fd, char *buf, int size);
void socket_close_client(int fd);
int  socket_send(int fd, const char *buf, int len);
void socket_close(void);

#endif /* SOCKET_H */