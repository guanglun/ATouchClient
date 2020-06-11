CC=gcc 

ifeq ($(ARCH),arm)
CC=arm-linux-gnueabihf-gcc
INCLUDES += /home/guanglun/workspace/lichee_zero/linux
endif

SRCDIR = 				\
		../src				\
		../src/log			\

OBJDIR = ../output/build
BINDIR = ../output/bin
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