CC = arm-elf-gcc
LD = arm-elf-ld
OBJCOPY = arm-elf-objcopy
OBJDUMP = arm-elf-objdump

CFLAGS= 
ASFLAGS=
LDFLAGS=-Tuser.lds -Ttext 80000000 

OBJS=test_user.c syscall.c user_syscall.s printf.c lib.c

.c.o:
	$(CC) $(CFLAGS) -c $<
.s.o:
	$(CC) $(ASFLAGS) -c $<

prg:$(OBJS)
	$(CC) -static -nostdinc -nostartfiles -nostdlib $(LDFLAGS) $? -o $@
	$(OBJCOPY) -O binary $@ prg.bin
	$(OBJDUMP) prg -D -S > prg.dis
	./mkofs $@.bin
	cp ofs.img /mnt/hgfs/swap

clean:
	rm *.o prg prg.bin -f *.dis  ofs.img
