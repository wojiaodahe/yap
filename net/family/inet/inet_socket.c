#include "error.h"
#include "fs.h"
#include "common.h"
#include "socket.h"
#include "inet_socket.h"
#include "tcp.h"

struct i_socket *alloc_isocket()
{
	struct i_socket *isk;
	isk = kmalloc(sizeof (struct i_socket));

	if (!isk)
		return NULL;

	INIT_LIST_HEAD(&isk->wq.task_list);
	INIT_LIST_HEAD(&isk->recv_data_head);
	INIT_LIST_HEAD(&isk->send_data_head);

	return isk;
}

void release_sock(struct i_socket *isk)
{
	kfree(isk);
}

extern struct i_proto_opt tcp_opt[];
static int inet_socket(struct socket *sock, int protocol)
{
	struct i_socket *isk;
	void *prot = 0;

	isk = alloc_isocket();
	if (!isk)
		return -EAGAIN;

	switch (sock->type)
	{
	case SOCK_DGRAM:
		prot = alloc_udp(isk);
		break;
	}
	if (!prot)
	{
		release_sock(isk);
		return -EAGAIN;
	}

	isk->prot = prot;
	sock->data = isk;
	isk->socket = sock;

	return 0;
}

void inet_close(struct socket *sock, struct socket *peer)
{

}

static int inet_dup(struct socket *newsock, struct socket *oldsock)
{
	return 0;
}

static int inet_bind(struct socket *sock, struct sockaddr *uaddr, int addr_len)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->bind(isk, (struct sockaddr_in *)uaddr, addr_len);
}

static int inet_listen(struct socket *sock, int backlog)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->listen(isk, backlog);
}

static int inet_connect(struct socket *sock, struct sockaddr *uservaddr, int addrlen)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->connect(isk, (struct sockaddr_in *)uservaddr, addrlen);
}

extern int HelloWorld_accept(void *arg, struct tcp_pcb *pcb, int err);
extern int i_tcp_accept_callback(void *arg, struct tcp_pcb *tcp, int err);
extern void i_tcp_conn_err_callback(void *arg, int err);
static int inet_accept(struct socket *sock, struct socket *newsock, int flags)
{
    int err;
    struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
//    err = isk->i_prot_opt->accept();
    if (err < 0)
        return err;

    wait_event(&newsock->wq, newsock->state == ESTABLISHED);
    
    return 0;
}

static int inet_read(struct socket *sock, char *ubuf, int size, int noblock)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->read(isk, ubuf, size, noblock, 0);
}

static int inet_write(struct socket *sock, char *ubuf, int size, int noblock)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->write(isk, ubuf, size, noblock, 0);
}

static int inet_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg)
{
	return -EINVAL;
}

static int inet_send(struct socket *sock, void *ubuf, int size, int noblock, unsigned flags)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
	return isk->i_prot_opt->send(isk, ubuf, size, noblock, flags);
}

static int inet_recv(struct socket *sock, void *ubuf, int size, int noblock, unsigned flags)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
	return isk->i_prot_opt->recv(isk, ubuf, size, noblock, flags);
}

static int inet_sendto(struct socket *sock, void *ubuf, int size, int noblock,
	    unsigned flags, struct sockaddr *sin, int addrlen)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
	isk->i_prot_opt->sendto(isk, ubuf, size, noblock, flags, (struct sockaddr_in *)sin, addrlen);
}

static int inet_recvfrom(struct socket *sock, void *ubuf, int size, int noblock,
		   unsigned flags, struct sockaddr *sin, int *addrlen)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
	isk->i_prot_opt->recvfrom(isk, ubuf, size, noblock, flags, (struct sockaddr_in *)sin, addrlen);
}

static struct family_ops inet_proto_ops =
{
  AF_INET,

  inet_socket,
  inet_dup,
  inet_close,
  inet_bind,
  inet_connect,
  inet_accept,
  inet_read,
  inet_write,
  inet_ioctl,
  inet_listen,
  inet_send,
  inet_recv,
  inet_sendto,
  inet_recvfrom,
};

void inet_family_init()
{
	socket_register(inet_proto_ops.family, &inet_proto_ops);
}
