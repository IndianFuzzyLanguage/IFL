SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj
STATIC_LIB=$(BIN_DIR)/libifl.a
TARGET=$(STATIC_LIB)
DEPENDENCY_DIR=dependency
EXPAT=libexpat
EXPAT_DIR=$(DEPENDENCY_DIR)/$(EXPAT)
LIB_EXPAT=$(EXPAT_DIR)/expat/lib/.libs/libexpat.a
DEPENDENCY=$(LIB_EXPAT)

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(addprefix $(OBJ_DIR)/,$(SRCS:.c=.o))

CC = gcc
AR = ar
RM = rm

ifeq ($(NOSAN),1)
	SAN_CFLAGS=
else
	SAN_CFLAGS= -fsanitize=address
endif

CFLAGS = -g -ggdb -O0 -Wall -Werror $(SAN_CFLAGS) -fstack-protector-all
LFLAGS = 

INC = -I ./src -I ./include -I $(EXPAT_DIR)/expat/lib
CFLAGS += $(INC)

.PHONY: all clean init_setup

all: init_setup $(TARGET)

$(LIB_EXPAT):
	echo "expatlib is $(LIB_EXPAT)"
	cd $(DEPENDENCY_DIR) && tar -zxvf $(EXPAT).tgz > /dev/null
	cd $(EXPAT_DIR)/expat && ./buildconf.sh && ./configure > /dev/null
	cd $(EXPAT_DIR)/expat/lib && make > /dev/null

init_setup: $(DEPENDENCY)
	@mkdir -p $(OBJ_DIR)/$(SRC_DIR)
	@mkdir -p $(BIN_DIR)
	@cp $(LIB_EXPAT) $(BIN_DIR)/.

$(OBJ_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(STATIC_LIB): $(OBJS)
	$(AR) r $@ $^

clean:
	@$(RM) -rf $(OBJS)
	@$(RM) -rf $(TARGET)
	@$(RM) -rf $(OBJ_DIR) $(BIN_DIR)

clobber:clean
	cd $(EXPAT_DIR)/expat/lib && make clean > /dev/null
