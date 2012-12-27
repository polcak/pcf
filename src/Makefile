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

OBJ = capture.o main.o computer_info_list.o parse_config.o computations.o check_computers.o clock_skew_guard.o computer_info.o
HEAD = capture.h computer_info_list.h parse_config.h computations.h check_computers.h clock_skew_guard.h computer_info.h packet_time_info.h point2d.h observer.h clock_skew.h
OPT = -lpcap -lm `xml2-config --cflags --libs`
CC = g++
DEFINE ?= 
CXXFLAGS ?= -O2 -Wall -std=c++0x $(DEFINE)
LDFLAGS ?= 
LD = g++

.PHONY: debug profiling uninstall clean donotreduce

all: $(program)

debug:
	make clean
	make CXXFLAGS="-DDEBUG -g -Wall -std=c++0x $(DEFINE)"

profiling:
	make clean
	make CXXFLAGS="-pg -Wall -std=c++0x $(DEFINE)" LDFLAGS="-pg"

donotreduce:
	make DEFINE="-DDONOTREDUCE"

install: all
	cp -r pcf config ../bin
	cp -r http_traffic www ../bin
	mkdir -p ../bin/graph ../bin/log

uninstall:
	rm -f ../bin/pcf

clean:
	rm -f *.o *~ $(program)

$(program): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $(program) $(OPT)

$(OBJ): $(HEAD)

check_computers.o: check_computers.cc check_computers.h
	$(CC) $(CXXFLAGS) -c `xml2-config --cflags --libs` check_computers.cc
