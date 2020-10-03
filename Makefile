
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
# This makefile combines makes everything in the code base given that prequisite libararies are installed.
# See README.md for info on libraries. Ensure that the paths in paths.config are correct before running everything.

all: FED Garble OblivC Experiments

FED:
	cd Code && make

Garble:
	cd Codegarble && make

OblivC: MAF ChiSq

MAF:
	cd CodeOblivC/MAF && make

ChiSq:
	cd CodeOblivC/ChiSq && make

Experiments:
	cd Code_Experiments/final && make
