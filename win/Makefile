CC=gcc 

SRCDIR = 				\
		../src				\

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
CCFLAGS = -lws2_32 -pthread -fexec-charset=GBK # -Wall

#@echo $(filter %$(notdir $(@:.o=.c)),$(SRCS))

$(TARGET):$(OBJS)
	$(CC) -o $@ $(OBJS) $(CCFLAGS)

# @echo LIBFILE: $(LIBFILE)
# @echo SRCS: $(SRCS)
# @echo OBJS: $(OBJS)
# @echo $(filter %$(notdir $(@:.o=.c)),$(SRCS))

$(OBJS):$(SRCS)
	$(CC) -o $@ -c $(filter %/$(notdir $(@:.o=.c)),$(SRCS)) $(INCLUDES) $(CCFLAGS)

clean:
	del ..\output\build\*.o