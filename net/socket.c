#include "socket.h"
#include "error.h"
#include "fs.h"
#include "common.h"
#include "proc.h"
#include "pcb.h"
#include "printk.h"
#include "syslib.h"
#include "kmalloc.h"

extern int inet_family_init(void);
extern struct file *get_empty_filp(void);
extern struct inode *get_empty_inode(void);



#define SO_ACCEPTCON	(1<<16)		/* performed a listen		*/

extern struct pcb *current;

static struct socket sockets[MAX_SOCKETS];
static struct family_ops *family_ops[MAX_PROTO];

#define last_socket	(sockets + MAX_SOCKETS - 1)


#if SKB_USE_SKB_POOL
struct sk_buff SKB[MAX_SKB_NUM];
#endif

void print_unused_skb()
{
	int i;
	int num = 0;

	for (i = 0; i < MAX_SKB_NUM; i++)
	{
		if (SKB[i].use_flag != SKB_USED)
			num++;
	}
	printk("unused skb: %d\n", num);
}

struct sk_buff *alloc_skbuff(unsigned short len)
{
#if SKB_USE_SKB_POOL
	int i;
//	print_unused_skb();
	for (i = 0; i < MAX_SKB_NUM; i++)
	{
		if (SKB[i].use_flag == SKB_NO_USE)
		{
            enter_critical();
			SKB[i].data_buf = kmalloc(len);
			if (!SKB[i].data_buf)
            {
                exit_critical();
                return NULL;
            }
			INIT_LIST_HEAD(&SKB[i].list);
			SKB[i].use_flag = SKB_USED;
            exit_critical();
			return &SKB[i];
		}
	}
	return NULL;
#else
	struct sk_buff *skb;

	sbk = kmalloc(sizeof (struct sk_buff));
	if (!skb)
		return NULL;

	skb->data_buf = kmalloc(data_len);
	if (!skb->data_buf)
	{
		kfree(skb);
		return NULL;
	}
	INIT_LIST_HEAD(&skb->list);
	
    return skb;
#endif
}

void free_skbuff(struct sk_buff *skb)
{
	if (!skb)
		return;

#if SKB_USE_SKB_POOL
	kfree(skb->data_buf);
	list_del(&skb->list);
	memset(skb, 0, sizeof (struct sk_buff));
#else
	list_del(&skb->list);
	kfree(skb->data_buf);
	kfree(skb);
#endif
}

void free_all_skb(struct list_head *head)
{
	struct sk_buff *tmp;
	struct list_head *list;

	list = head->next;
	while (list != head)
	{
		tmp = list_entry(list, struct sk_buff, list);
		list = list->next;
		free_skbuff(tmp);
	}
}

struct socket * socki_lookup(struct inode *inode)
{
	struct socket *sock;

	for (sock = sockets; sock <= last_socket; ++sock)
	{
		if (sock->state != SS_FREE && SOCK_INODE(sock) == inode) 
			return(sock);
	}
	
	return(NULL);
}

static struct socket * sockfd_lookup(int fd, struct file **pfile)
{
    struct file *file;

    if (fd < 0 || fd >= NR_OPEN || !(file = current->filp[fd]))
        return(NULL);

    if (pfile)
        *pfile = file;

    return socki_lookup(file->f_inode);
}

static int sock_read(struct inode *inode, struct file *file, char *ubuf, int size)
{
	struct socket *sock;

	if (!(sock = socki_lookup(inode))) 
	{
		printk("NET: sock_read: can't find socket for inode!\n");
		return -EBADF;
	}

	return(sock->ops->read(sock, ubuf, size, (file->f_flags & O_NONBLOCK)));
}

static int sock_write(struct inode *inode, struct file *file, char *ubuf, int size)
{
	struct socket *sock;

	if (!(sock = socki_lookup(inode))) 
	{
		printk("NET: sock_write: can't find socket for inode!\n");
		return -EBADF;
	}

	return(sock->ops->write(sock, ubuf, size,(file->f_flags & O_NONBLOCK)));
}

static int sock_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	struct socket *sock;

	if (!(sock = socki_lookup(inode))) 
	{
		printk("NET: sock_ioctl: can't find socket for inode!\n");
		return -EBADF;
	}
	
	return(sock->ops->ioctl(sock, cmd, arg));
}


static void sok_release(struct socket *sock)
{

}

void sock_close(struct inode *inode, struct file *file)
{
	struct socket *sock;
	struct socket *peersock;

	/* It's possible the inode is NULL if we're closing an unfinished socket. */
	if (!inode) 
		return;
	
	if (!(sock = socki_lookup(inode))) 
	{
		printk("NET: sock_close: can't find socket for inode!\n");
		return;
	}
	
	if (sock->ops) 
		sock->ops->close(sock, peersock);
	
	sock->state = SS_FREE;
	//  kn_release(sock);
}


static struct file_operations socket_file_ops = 
{
	NULL,
	sock_close,
	sock_read,
	sock_write,
	sock_ioctl,
	NULL,		
	NULL,			
	sock_close
};

static struct socket *sock_alloc(void)
{
	struct socket *sock;

	for (sock = sockets; sock <= last_socket; ++sock)
	{
		if (sock->state == SS_FREE) 
		{
			sock->state = SS_UNCONNECTED;

			sock->flags = 0;
			sock->ops = NULL;
			sock->data = NULL;
			sock->conn = NULL;
			sock->iconn = NULL;

			/*
			* This really shouldn't be necessary, but everything
			* else depends on inodes, so we grab it.
			* Sleeps are also done on the i_wait member of this
			* inode.  The close system call will iput this inode
			* for us.
			*/
			if (!(SOCK_INODE(sock) = get_empty_inode())) 
			{
				printk("NET: sock_alloc: no more inodes\n");
				sock->state = SS_FREE;
				return NULL;
			}
			SOCK_INODE(sock)->i_mode = S_IFSOCK;

			return(sock);
		}
	}
	
	return NULL;
}

void sock_free(struct socket *sock)
{

}

static int sock_alloc_fd(struct socket *sock)
{
	int fd;
	struct file *file;
	struct inode *inode;

	inode = sock->inode;

	/* Find a file descriptor suitable for return to the user. */
	file = get_empty_filp();
	if (!file) 
		return -EAGAIN;
	
	for (fd = 0; fd < NR_OPEN; ++fd)
	{
		if (!current->filp[fd]) 
			break;
	}
	if (fd == NR_OPEN) 
	{
		file->f_count = 0;
		return -EAGAIN;
	}
	
	current->filp[fd] = file;
	file->f_op = &socket_file_ops;
	file->f_mode = 3;
	file->f_flags = 0;
	file->f_count = 1;
	file->f_inode = inode;
	if (inode) 
		inode->i_count++;
	file->f_pos = 0;
	
	return fd;
}

void sock_free_fd(struct socket *sock)
{
	int fd;
	struct file *file;
	
	fd = sock->fd;

	file = current->filp[fd];
	if (file)
		memset(file, 0, sizeof (struct file));

	current->filp[fd] = NULL;

}

int sys_socket(int family, int type, int protocol)
{
	int i, fd, ret;
	struct socket *sock;
	struct family_ops *ops;

	/* Locate the correct protocol family. */
	for (i = 0; i < MAX_PROTO; ++i)
	{
		if (family_ops[i] == NULL)
			continue;

		if (family_ops[i]->family == family)
			break;
	}
	if (i == MAX_PROTO)
	{
		printk(("NET: sock_socket: family not found\n"));
		return -EINVAL;
	}
	ops = family_ops[i];
	
	if ((type != SOCK_STREAM && type != SOCK_DGRAM &&
		 type != SOCK_SEQPACKET && type != SOCK_RAW &&
		 type != SOCK_PACKET) || protocol < 0)
		return -EINVAL;

	if (!(sock = sock_alloc()))
	{
		printk("sock_socket: no more sockets\n");
		return -EAGAIN;
	}

	sock->type = type;
    sock->ops = ops;

	if ((fd = sock_alloc_fd(sock)) < 0)
	{
		sock_free(sock);
		return -EINVAL;
	}

	sock->fd = fd;

	ret = ops->socket(sock, protocol);
	if (ret < 0)
	{
		sock_free_fd(sock);
		return ret;
	}

	return fd;
}

int sys_bind(int fd, struct sockaddr *umyaddr, int addrlen)
{
	struct socket *sock;

	if (fd < 0 || fd > NR_OPEN || current->filp[fd] == NULL)
		return -EBADF;

	if (!(sock = sockfd_lookup(fd, NULL)))
		return -ENOTSOCK;

	 return sock->ops->bind(sock, umyaddr, addrlen);
}

int sys_listen(int fd, int backlog)
{
	struct socket *sock;

	if (fd < 0 || fd >= NR_OPEN || current->filp[fd] == NULL)
		return -EBADF;

	if (!(sock = sockfd_lookup(fd, NULL)))
		return(-ENOTSOCK);

	if (sock->state != SS_UNCONNECTED)
		return(-EINVAL);

	sock->flags |= SO_ACCEPTCON;

	return sock->ops->listen(sock, backlog);
}

int sys_accept(int fd, struct sockaddr *upeer_sockaddr, int *upeer_addrlen)
{
	struct file *file;
	struct socket *sock, *newsock;
	int ret;

	if (fd < 0 || fd >= NR_OPEN || ((file = current->filp[fd]) == NULL))
		return(-EBADF);

	if (!(sock = sockfd_lookup(fd, &file)))
	  return -ENOTSOCK;

	if (sock->state != SS_UNCONNECTED)
		return -EINVAL;

	if (!(sock->flags & SO_ACCEPTCON))
		return -EINVAL;

	if (!(newsock = sock_alloc()))
		return(-EAGAIN);

	newsock->type = sock->type;
	newsock->ops  = sock->ops;
    newsock->data = sock->data; 

	//if ((ret = sock->ops->dup(newsock, sock)) < 0)
	//{
	//	sock_free(newsock);
	//	return ret;
	//}

	ret = newsock->ops->accept(sock, newsock, file->f_flags);
	if (ret < 0)
	{
		sock_free(newsock);
		return ret;
	}

	if ((fd = sock_alloc_fd(newsock)) < 0)
	{
		sock_free(newsock);
		return -EINVAL;
	}

	//if (upeer_sockaddr)
	//	newsock->ops->getname(newsock, upeer_sockaddr, upeer_addrlen, 1);

	return fd;
}

int sys_connect(int fd, struct sockaddr *uservaddr, int addrlen)
{
	struct socket *sock;
	struct file *file;

	if (fd < 0 || fd >= NR_OPEN || (file=current->filp[fd]) == NULL)
		return -EBADF;

	if (!(sock = sockfd_lookup(fd, &file)))
		return -ENOTSOCK;
	switch(sock->state)
	{
	case SS_UNCONNECTED:
		/* This is ok... continue with connect */
		break;
	case SS_CONNECTED:
		/* Socket is already connected */
		return -EISCONN;
	case SS_CONNECTING:
		/* Not yet connected... we will check this. */
		return(sock->ops->connect(sock, uservaddr, addrlen, file->f_flags));
	default:
		printk("NET: sock_connect: socket not unconnected\n");
		return -EINVAL;
	}

	return sock->ops->connect(sock, uservaddr, addrlen, file->f_flags);
}

int sys_send(int fd, void * buff, int len, unsigned flags)
{
	struct socket *sock;
	struct file *file;


	if (fd < 0 || fd >= NR_OPEN || ((file = current->filp[fd]) == NULL))
		return(-EBADF);

	if (!(sock = sockfd_lookup(fd, NULL)))
		return(-ENOTSOCK);

	return(sock->ops->send(sock, buff, len, (file->f_flags & O_NONBLOCK), flags));
}

int sys_sendto(int fd, void * buff, int len, unsigned flags, struct sockaddr *addr, int addr_len)
{
	struct socket *sock;
	struct file *file;

	if (fd < 0 || fd >= NR_OPEN || ((file = current->filp[fd]) == NULL))
		return -EBADF;
	if (!(sock = sockfd_lookup(fd, NULL))) 
        return -ENOTSOCK;

	return(sock->ops->sendto(sock, buff, len, (file->f_flags & O_NONBLOCK),
							  flags, addr, addr_len));
}

int sys_recv(int fd, void * buff, int len, unsigned flags)
{
	struct socket *sock;
	struct file *file;

	if (fd < 0 || fd >= NR_OPEN || ((file = current->filp[fd]) == NULL))
		return -EBADF;

	if (!(sock = sockfd_lookup(fd, NULL)))
		return -ENOTSOCK;

	return(sock->ops->recv(sock, buff, len,(file->f_flags & O_NONBLOCK), flags));

}

int sys_recvfrom(int fd, void * buff, int len, unsigned flags, struct sockaddr *addr, int *addr_len)
{
	struct socket *sock;
	struct file *file;

	if (fd < 0 || fd >= NR_OPEN || ((file = current->filp[fd]) == NULL))
		return -EBADF;

	if (!(sock = sockfd_lookup(fd, NULL)))
		return -ENOTSOCK;

	return(sock->ops->recvfrom(sock, buff, len, (file->f_flags & O_NONBLOCK), flags, addr, addr_len));
}

int socket_register(int family, struct family_ops *ops)
{
	int i;
	//disable_irq

	for (i = 0; i < MAX_PROTO; i++)
	{
		if (family_ops[i] != NULL)
			continue;
		family_ops[i] = ops;
		family_ops[i]->family = family;

		//enable_irq
		return i;
	}

	//enable_irq
	return -ENOMEM;
}

int socket_init(void)
{
	struct socket *sock;
	int i;


	/* Release all sockets. */
	for (sock = sockets; sock <= last_socket; ++sock)
		sock->state = SS_FREE;

	/* Initialize all address (protocol) families. */
	for (i = 0; i < MAX_PROTO; ++i)
		family_ops[i] = NULL;

    netif_init();

	return inet_family_init();
}
