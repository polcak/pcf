#
# Copyright (C) 2012 Jakub Jirasek <xjiras02@stud.fit.vutbr.cz>
# 
# This file is part of pcf - PC fingerprinter.
#
# Pcf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Pcf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with pcf. If not, see <http://www.gnu.org/licenses/>.
#

program = pcf

OBJ = capture.o main.o list.o parse_config.o computations.o check_computers.o computer_identity.o
HEAD = capture.h list.h parse_config.h computations.h check_computers.h computer_identity.h
OPT = -lpcap -lm `xml2-config --cflags --libs`
CC = gcc
CFLAGS ?= -O2 -Wall -std=gnu99

.PHONY: debug uninstall clean

all: $(program)

debug:
	make clean
	make CFLAGS="-DDEBUG -g -Wall -std=gnu99"

install: all
	cp -r pcf gen_pics.sh config ../bin
	cp -r http_traffic www ../bin
	mkdir -p ../bin/graph ../bin/log

uninstall:
	rm -f ../bin/pcf ../bin/gen_pics.sh

clean:
	rm -f *.o *~ $(program)

$(program): $(OBJ)
	$(CC) $(OBJ) -o $(program) $(OPT)

$(OBJ): $(HEAD)

check_computers.o: check_computers.c check_computers.h
	$(CC) $(CFLAGS) -c `xml2-config --cflags --libs` check_computers.c
