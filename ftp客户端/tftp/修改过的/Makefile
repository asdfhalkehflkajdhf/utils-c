#---------------------------------------------------------------------
#   author: cenlizhong
#     date: 2012-9-29 
# describe: a versatile Makefile
#
#       ps: language type (c/cpp)
#       cc: compiler (gcc/g++)
#   CFLAGS: c/c++ language flags (for example: -g -wall -o2)
#     DEST: output file name 
#     LIBS: library name (for example: pthread)
# INCLUDES: header file directory (for example: ./include)
#  SUB_DIR: subdirectory (for example: dir1 dir1/aaa dir/bbb dir2) 
# DEST_DIR: prefix install directory   
#  INSTALL: install directory (actually install to $DEST_DIR/$INSTALL)  
#----------------------------------------------------------------------

#----------------------configure in this part
PS = c
CC = gcc 
CFLAGS = -g -Wall
DEST := tftp
LIBS := 
INCLUDES :=  
SUB_DIR := 
DEST_DIR := 
INSTALL := /usr/local/bin

#----------------------do nothing in this part
RM := rm -f
CFLAGS  += -MMD -MF $(patsubst ./%, %, $(patsubst %.o, %.d, $(dir $@).$(notdir $@))) $(addprefix -I, $(INCLUDES))
SRCS := $(wildcard *.$(PS) $(addsuffix /*.$(PS), $(SUB_DIR)))
OBJS := $(patsubst %.$(PS), %.o, $(SRCS))
DEPS := $(patsubst %.$(PS), %.d, $(foreach n,$(SRCS),$(patsubst ./%, %, $(dir $n).$(notdir $n))))
MISS := $(filter-out $(wildcard DEPS), $(DEPS))

all: $(DEST)

clean :
	@$(RM) $(OBJS) 
	@$(RM) $(DEPS) 
	@$(RM) $(DEST)
	@$(RM) *.rpm *.tar.gz
install:
	@if [ ! -d $(DEST_DIR)$(INSTALL) ]; then mkdir -p $(DEST_DIR)$(INSTALL); fi
	cp -f $(DEST) $(DEST_DIR)$(INSTALL) 
	cp -f ./ext/bin/* $(DEST_DIR)$(INSTALL) 
	@if [ ! -d $(DEST_DIR)/etc/$(DEST) ]; then mkdir -p $(DEST_DIR)/etc/$(DEST); fi
	cp -f cron.sh $(DEST_DIR)/etc/$(DEST)/$(DEST)-cron.sh
	cp -f conf.yaml $(DEST_DIR)/etc/$(DEST)/$(DEST)-conf.yaml.template
	cp -f newiup2.0-install-reference.doc $(DEST_DIR)/etc/$(DEST)/$(DEST)-reference.doc
	@if [ ! -d $(DEST_DIR)/usr/local/lib/ ]; then mkdir -p $(DEST_DIR)/usr/local/lib/; fi
	cp -f -d ./ext/lib/* $(DEST_DIR)/usr/local/lib/

build:
	cp -f -d ./ext/lib/* /usr/local/lib/
	rm -rf /usr/lib64/libarchive.so.13
	ln -s /usr/local/lib/libarchive.so.13 /usr/lib64/libarchive.so.13

ifneq ($(MISS),)
$(MISS):
	@$(RM) $(patsubst %.d, %.o, $(dir $@)$(patsubst .%,%, $(notdir $@)))
endif

-include $(DEPS)

$(DEST): $(OBJS)
	$(CC) -o $(DEST) $(OBJS) $(addprefix -l,$(LIBS))

rpm:
	./buildrpm.sh $(DEST) 2.4.2