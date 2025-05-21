# /*
#  * airdump - a lightweight yet powerful tool for monitoring WiFi networks
#  *
#  * Copyright (c) 2025 Antonio Mancuso <profmancusoa@gmail.com>
#  * 
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
#  *
#  *
#  * This tools is eavily inspired by wavemon (https://github.com/uoaerg/wavemon) and reuse part of its code
#  *
#  */

# compiler flags
CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Wpedantic -Werror -I/usr/include/libnl3

# linking flags
LDFLAGS := -L.
LIBS    := -lm  -lnl-cli-3 -lnl-genl-3  -lnl-route-3 -lnl-3
DEFS	:= -DTOOL_NAME=\"airdump\" -DTOOL_VERSION=\"0.1.0\" -D_DEFAULT_SOURCE -D_XOPEN_SOURCE=600 -D_XOPEN_SOURCE_EXTENDED


# source
SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)

# target app name
TARGET  := airdump

.PHONY: all debug release clean

all: release

debug: CFLAGS  += -g -O0
debug: LDFLAGS +=
debug: DEFS += -DDEBUG
debug: $(TARGET)

release: CFLAGS  += -O2
release: $(TARGET)

# linking
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# .c → .o
%.o: %.c
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
