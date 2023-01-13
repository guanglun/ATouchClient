CC=gcc 

ifeq ($(ARCH),arm)
CC=/home/guanglun/workspace/Tina-t113-pro/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-gcc
export STAGING_DIR=/home/guanglun/workspace/Tina-t113-pro/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain

INCLUDES += -I/home/guanglun/workspace/Tina-t113-pro/out/t113-pro/staging_dir/target/usr/include
LDFLAGS+= -L/home/guanglun/workspace/Tina-t113-pro/out/t113-pro/staging_dir/target/usr/lib

endif

SRCDIR = 				\
		../src				\
		../src/log			\

OBJDIR = output/build
BINDIR = output/bin
BINAME = ATouchClient

INCLUDES += $(foreach n,$(SRCDIR),-I $(n))

TARGET = $(BINDIR)/$(BINAME)
SRCS = $(foreach n,$(SRCDIR),$(n)/*.c)
SRCS := $(wildcard $(SRCS)) 
OBJS = $(SRCS:.c=.o)
OBJS :=$(notdir $(OBJS))  
OBJS := $(foreach n,$(OBJS),$(OBJDIR)/$(n))
CCFLAGS = -pthread

$(TARGET):$(OBJS)
	$(CC) $(CCFLAGS) -o $@ $(OBJS)

# @echo $(filter %$(notdir $(@:.o=.c)),$(SRCS))
# @echo LIBFILE: $(LIBFILE)
# @echo SRCS: $(SRCS)
# @echo OBJS: $(OBJS)
$(OBJS):$(SRCS)
	$(CC) $(CCFLAGS) -o $@ -c $(filter %/$(notdir $(@:.o=.c)),$(SRCS)) $(INCLUDES)

clean:
	rm $(OBJDIR)/*