#include "spi.h"

struct spi_board_info spi_info_jz2440[] = 
{
    {
        .max_speed_hz   = 10000000,
        .bus_num        = 1,
        .mode           = SPI_MODE_0,
        .chip_select    = 1,
        .platform_data  = (void *)4,
    },
};

void spi_info_jz2440_init(void)
{
    spi_register_board_info(spi_info_jz2440, 1);
}
