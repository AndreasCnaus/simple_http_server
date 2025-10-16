CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
LDFLAGS = -lsqlite3

# Targets
EXEC = http_server
LIB  = libgps_pack.so

# Source files
SRCS = http_server.c gps_db.c gps_pack.c pack.c  
OBJS = $(SRCS:.c=.o)

# Object files for shared library (only gps_pack.c and pack.c)
LIB_SRCS = gps_pack.c pack.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

all: $(EXEC) $(LIB)

# Build executable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Build shared library
$(LIB): $(LIB_OBJS)
	$(CC) -shared -o $@ $(LIB_OBJS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(LIB_OBJS) $(EXEC) $(LIB)

.PHONY: all clean
