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
testName=MAF
CONFIGFILE=../../Code/paths.config
CILPATH=$(shell cat $(CONFIGFILE) | grep "pathoblivcdownload" | awk -F "=" '{print $$2}')
REMOTE_HOST=localhost
CFLAGS += -DREMOTE_HOST=$(REMOTE_HOST) -O3
./a.out: $(testName).oc $(testName).c util.c $(CILPATH)/_build/libobliv.a
	$(CILPATH)/bin/oblivcc $(CFLAGS) -I . $(testName).oc $(testName).c util.c
clean:
	rm -f a.out
