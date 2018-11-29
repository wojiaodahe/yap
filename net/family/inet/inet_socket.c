#include "error.h"
#include "fs.h"
#include "common.h"
#include "socket.h"
#include "inet_socket.h"
#include "tcp.h"
#include "udp.h"
#include "kmalloc.h"

static struct i_proto_opt *inet_protos[MAX_INET_PROTO];

struct i_socket *alloc_isocket()
{
	struct i_socket *isk;
	isk = kmalloc(sizeof (struct i_socket));

	if (!isk)
		return NULL;

	INIT_LIST_HEAD(&isk->wq.task_list);
	INIT_LIST_HEAD(&isk->recv_data_head);
	INIT_LIST_HEAD(&isk->send_data_head);
	INIT_LIST_HEAD(&isk->unsend);
	INIT_LIST_HEAD(&isk->unacked);
	INIT_LIST_HEAD(&isk->list);

	return isk;
}

void free_isock(struct i_socket *isk)
{
	kfree(isk);
}

struct i_prot_opt *inet_get_opt(short int type)
{
    int i; 

    for (i = 0; i < MAX_INET_PROTO; i++)
    {
        if (inet_protos[i]->proto == type)
            return inet_protos[i];
    }
    
    return NULL;
}

static int inet_socket(struct socket *sock, int protocol)
{
    int i;
	struct i_socket   *isk;
    struct i_proto_opt *opt;

	isk = alloc_isocket();
	if (!isk)
		return -EAGAIN;

	sock->data = isk;
	isk->socket = sock;

    opt = inet_get_opt(sock->type);
    if (!opt)
    {
        return -EINVAL;
        free_isock(isk);
    }
    isk->i_prot_opt = opt;
    if (opt->proto_new_sock)
        opt->proto_new_sock(isk);

	return 0;
}

int inet_close(struct socket *sock, struct socket *peer)
{
    return 0;
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

static int inet_connect(struct socket *sock, struct sockaddr *uservaddr, int addrlen, int flags)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->connect(isk, (struct sockaddr_in *)uservaddr, addrlen);
}

static int inet_accept(struct socket *sock, struct socket *newsock, int flags)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
    isk = isk->i_prot_opt->accept(isk, flags);
    if (!isk)
        return -EBADF;

    newsock->data = isk;
    isk->socket   = newsock;

    wait_event(&isk->wq, isk->status == ESTABLISHED);
    
    return 0;
}

static int inet_listen(struct socket *sock, int backlog)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;

	return isk->i_prot_opt->listen(isk, backlog);
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
	return isk->i_prot_opt->sendto(isk, ubuf, size, noblock, flags, (struct sockaddr_in *)sin, addrlen);
}

static int inet_recvfrom(struct socket *sock, void *ubuf, int size, int noblock,
		   unsigned flags, struct sockaddr *sin, int *addrlen)
{
	struct i_socket *isk;

	if (!sock || !sock->data)
		return -EBADF;

	isk = (struct i_socket *)sock->data;
	return isk->i_prot_opt->recvfrom(isk, ubuf, size, noblock, flags, (struct sockaddr_in *)sin, addrlen);
}

int inet_register_proto(struct i_proto_opt *opt)
{
    int i;

    for (i = 0; i < MAX_INET_PROTO; i++)
    {
        if (!inet_protos[i])
        {
            inet_protos[i] = opt;
            return 0;
        }
    }

    return -1;
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
    int err;

    memset(&inet_protos, 0, sizeof (inet_protos));

    err = socket_register(inet_proto_ops.family, &inet_proto_ops);
    if (err < 0)
    {
        printk("socket_register() failed\n");
        panic();
    }

    err = tcp_init();
    if (err < 0)
    {
        printk("upd_init() failed\n");
        panic();
    }
    
    err = udp_init();
    if (err < 0)
    {
        printk("tcp_init() failed\n");
        panic();
    }
}
