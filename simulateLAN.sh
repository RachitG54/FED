
# ====================================================================
# Functionally Encrypted Datatores - Implementation for project eprint.iacr.org/2019/1262
# Copyright (C) 2019  Rachit Garg Nishant Kumar

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# ====================================================================
#
#
#!/bin/bash

sudo tc qdisc del dev lo root
sudo tc qdisc add dev lo root handle 1: htb default 12
sudo tc class add dev lo parent 1:1 classid 1:12 htb rate 620mbps ceil 620mbps
sudo tc qdisc add dev lo parent 1:12 netem delay 0.1ms 0.01ms distribution normal
sudo tc -s qdisc ls dev lo 
