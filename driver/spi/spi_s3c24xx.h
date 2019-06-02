#ifndef __SPI_S3C24XX_H__ 
#define __SPI_S3C24XX_H__

#define S3C2440_SPCON               (0x00)
#define S3C2440_SPSTA               (0x04)
#define S3C2440_SPPIN               (0x08)
#define S3C2440_SPPRE               (0x0c)
#define S3C2440_SPDATA              (0x10)
#define S3C2440_SPRDAT              (0x14)

#define S3C2440_SPCON_SMOD_DMA      (2 << 5)
#define S3C2440_SPCON_SMOD_IRQ      (1 << 5)
#define S3C2440_SPCON_SMOD_POLL     (0 << 5)
#define S3C2440_SPCON_ENSCK         (1 << 4)
#define S3C2440_SPCON_MSTR          (1 << 3)
#define S3C2440_SPCON_CPOL_HIGH     (1 << 2)
#define S3C2440_SPCON_CPOL_LOW      (0 << 2)
#define S3C24XX_SPCON_CPHA_FMTB     (1 << 1)
#define S3C24XX_SPCON_CPHA_FMTA     (0 << 1)

#define S3C24XX_SPSTA_READY_ORG     (1 << 3)
#define S3C24XX_SPSTA_DCOL          (1 << 2)
#define S3C24XX_SPSTA_MULD          (1 << 1)
#define S3C24XX_SPSTA_READY         (1 << 0)

#define S3C24XX_SPPIN_ENMUL         (1 << 2)
#define S3C24XX_SPPIN_KEEP          (1 << 0)

#endif


