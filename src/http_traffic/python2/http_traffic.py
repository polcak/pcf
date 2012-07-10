#!/usr/bin/python

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

import sys
import random
import time
import httplib2

if len(sys.argv) < 2:
  address = "http://localhost/"
else:
  address = sys.argv[1]

if address.find("http://") == -1:
  address = "http://" + address

h = httplib2.Http()
count = 1400
total = 0

for i in range(count):
  
  wait = random.randint(10, 30)
  time.sleep(wait)
  
  number = random.randint(1, 4)
  total = total + number
  
  for i in range(number):
    print "\r", total, "packets sent",
    sys.stdout.flush()
    resp, content = h.request(address, "GET")
