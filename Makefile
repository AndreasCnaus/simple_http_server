# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 -fPIC
LDFLAGS = -lsqlite3

# Directories
BUILD_DIR = build
VPATH = .

# Targets
EXEC = http_server
LIB = libgps_pack.so
EXEC_PATH = $(BUILD_DIR)/$(EXEC)
LIB_PATH = $(BUILD_DIR)/$(LIB)

# Source files
SRCS = http_server.c gps_db.c gps_pack.c pack.c
LIB_SRCS = gps_pack.c pack.c

# Object files (with build directory prefix)
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
LIB_OBJS = $(addprefix $(BUILD_DIR)/, $(LIB_SRCS:.c=.o))

# ----------------------------------------------------------------------
# Main Targets
# ----------------------------------------------------------------------

all: $(BUILD_DIR) $(EXEC_PATH) $(LIB_PATH)

# Create the build directory if it doesn't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build executable
$(EXEC_PATH): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

# Build shared library
$(LIB_PATH): $(LIB_OBJS)
	$(CC) -shared -o $@ $(LIB_OBJS)

# ----------------------------------------------------------------------
# Generic Object Compilation Rule (Pattern Rule)
# ----------------------------------------------------------------------

# Rule to compile a .c file into a .o file within the build directory
$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------------------------------------------------
# Cleanup
# ----------------------------------------------------------------------

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean $(BUILD_DIR)