CC=gcc 

SRCDIR = 				\
		../src				\

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

# $(TARGET):$(OBJS)
# 	@echo $(filter %$(notdir $(@:.o=.c)),$(SRCS))
# 	$(CC) $(CCFLAGS) -o $@ $(OBJS)

$(OBJS):$(SRCS)
	@echo LIBFILE: $(LIBFILE)
	@echo SRCS: $(SRCS)
	@echo OBJS: $(OBJS)
	@echo $(filter %$(notdir $(@:.o=.c)),$(SRCS))

	$(CC) $(CCFLAGS) -o $@ -c $(filter %/$(notdir $(@:.o=.c)),$(SRCS)) $(INCLUDES)

clean:
	rm $(OBJDIR)/*