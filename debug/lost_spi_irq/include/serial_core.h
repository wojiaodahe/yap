#ifndef __SERIAL_CORE_H__
#define __SERIAL_CORE_H__

struct uart_port
{
	unsigned long iobase;
	unsigned char *membase;
	unsigned int (*serial_in)(struct uart_port *, int);
	void (*serial_out)(struct uart_port *, int, int);
	struct tty *tty;
};

struct uart_ops
{
	void (*stop_tx)(struct uart_port *);
	void (*stop_rx)(struct uart_port *);
	void (*start_tx)(struct uart_port *);
	void (*start_rx)(struct uart_port *);
	void (*startup)(struct uart_port *);
	void (*send_xchar)(struct uart_port *, char ch);
	unsigned char unused[2];
	int irq_rx;
	int irq_tx;
};

struct uart_driver
{
	const char *driver_name;
	const char *dev_name;
	int major;
	int minor;
	int nr;
	struct tty *tty;
};

#endif

