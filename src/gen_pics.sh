#!/bin/bash

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

find graph/ -maxdepth 1 -name *.ps -type f -mmin +60 -delete
find www/graph/ -maxdepth 1 -name *.jpg -type f -mmin +60 -delete

i=0
for image in graph/*.ps; do
	gs -sDEVICE=jpeg -r65 -dBATCH -dNOPAUSE -sOutputFile="www/${image%.*}.jpg" -c "<</Orientation 3>> setpagedevice" -f "$image"
	echo "image: $image"
	i=$[$i+1];
done
