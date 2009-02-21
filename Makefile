CFLAGS += -Wall

init_OBJECTS = exec.o \
	       init.o \
	       kernel_param.o \
	       rescue.o \
	       udev.o

all: initramfs_data.cpio

init: $(init_OBJECTS)
	$(CC) -o init $(init_OBJECTS)

initramfs_data.cpio: gen_init_cpio init initramfs_contents
	./gen_init_cpio initramfs_contents >initramfs_data.cpio


kernel_param.c: kernel_param.h
exec.c: exec.h
udev.c: udev.h
init.c: kernel_param.h udev.h exec.h
