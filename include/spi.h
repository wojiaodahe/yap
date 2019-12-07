#ifndef __SPI_H__
#define __SPI_H__

#include "device.h"

#define SPI_NAME_SIZE  20

#define SPI_MASTER_HALF_DUPLEX	(1 << 0)		/* can't do full duplex */
#define SPI_MASTER_NO_RX	    (1 << 1)		/* can't do buffer read */
#define SPI_MASTER_NO_TX	    (1 << 2)		/* can't do buffer write */

#define SPI_CPHA                (0x01)
#define SPI_CHOL                (0x02)
#define SPI_MODE_0              (0 | 0)
#define SPI_MODE_1              (0 | SPI_CPHA)
#define SPI_MODE_2              (SPI_CPOL | 0)
#define SPI_MODE_3              (SPI_CPOL | SPI_CPHA)


struct spi_board_info 
{
	int		        irq;
	void	        *platform_data;
	void		    *controller_data;
	unsigned int	max_speed_hz;

    unsigned char 	mode;
	unsigned short  bus_num;
	unsigned short  chip_select;

    struct list_head list;
};

struct spi_message 
{
	struct list_head	transfers;
	struct spi_device	*spi;
    void			    (*complete)(void *context);
	void			    *context;
	int			        status;
	void			    *state;
    struct list_head	queue;
    struct spi_master   *master;
};

struct spi_transfer
{
	void	            *tx_buf;
	void		        *rx_buf;
	unsigned	        len;
    unsigned short		delay_usecs;
    unsigned short		speed_hz;

	struct list_head    list;
};


struct spi_master 
{
	struct device	    dev;

	struct list_head    list;

    unsigned char       bus_lock_flag;

    unsigned int        irq;
    unsigned short      bus_num;
    unsigned short      num_chipselect;
	unsigned short 	    mode_bits;
    unsigned short      flags;

	int			        (*setup)(struct spi_device *spi);
	int			        (*transfer)(struct spi_device *spi, struct spi_message *mesg);
	void			    (*cleanup)(struct spi_device *spi);
	unsigned char		queued;
	struct list_head	queue;
	struct spi_message	*cur_msg;
};

struct spi_device
{
    char                    *name;
    int                     id;
    struct device           dev;
	int		                irq;
    unsigned int            num_resource;
    unsigned char 	        mode;
    unsigned short          chip_select;
	unsigned int	        max_speed_hz;
    struct resource         *resource;
    struct spi_master       *master;
    struct spi_device_id    *id_entry;
};

struct spi_driver
{
    int (*probe)(struct spi_device *);
    int (*remove)(struct spi_device *);
    void (*shutdown)(struct spi_device *);
    struct device_driver driver;
    struct spi_device_id *id_table;
};

extern int spi_register_master(struct spi_master *master);
extern struct spi_master *spi_alloc_master(struct device *dev, unsigned int size);
extern void spi_master_set_devdata(struct spi_master *master, void *data);
extern void *spi_master_get_devdata(struct spi_master *master);
extern int spi_register_driver(struct spi_driver *spi_drv);
extern void spi_unregister_driver(struct spi_driver *spi_drv);
extern int spi_register_board_info(struct spi_board_info *info, unsigned int n);


extern int spi_sync(struct spi_device *spi, struct spi_message *message);
extern int spi_write(struct spi_device *dev, void *buf, int len);

#endif

