
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
import os, sys, glob

if (len(sys.argv)!=2):
	print("Usage wrong: pass the directory as parameter. Exiting.", file=sys.stderr)
	exit(1)

errDirectory = sys.argv[1]

os.chdir(errDirectory)
allPassed = True
for filename in glob.glob("*"):
	fullFileName = filename
	with open(fullFileName, 'r') as ff:
		lines = ff.readlines()
		if not(len(lines)==0 or (len(lines)==1 and lines[0]=="read: End of file\n")):
			print(filename, "**********FAILED**********")
			allPassed = False
			break

if (allPassed):
	print("ALL PASSED")
else:
	print("SOME FAILURE")
