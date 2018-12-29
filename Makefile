SRC_DIR=src
BIN_DIR=bin
OBJ_DIR=obj
STATIC_LIB=$(BIN_DIR)/libifl.a
TARGET=$(STATIC_LIB)

SRCS=$(wildcard $(SRC_DIR)/*.c)
OBJS=$(addprefix $(OBJ_DIR)/,$(SRCS:.c=.o))

CC = gcc
AR = ar
RM = rm

EXPAT_DIR=dependency/libexpat
CFLAGS = -g -ggdb -O0 -Wall -Werror
LFLAGS = 

INC = -I ./src -I ./include -I $(EXPAT_DIR)/expat/lib
CFLAGS += $(INC)

.PHONY: all clean init_setup

all: init_setup $(TARGET)

init_setup:
	@mkdir -p $(OBJ_DIR)/$(SRC_DIR)
	@mkdir -p $(BIN_DIR)
	@cp $(EXPAT_DIR)/expat/lib/.libs/libexpat.a $(BIN_DIR)/.

$(OBJ_DIR)/%.o:%.c
	$(CC) $(CFLAGS) -o $@ -c $^

$(STATIC_LIB): $(OBJS)
	$(AR) r $@ $^

clean:
	@$(RM) -rf $(OBJS)
	@$(RM) -rf $(TARGET)
	@$(RM) -rf $(OBJ_DIR) $(BIN_DIR)
