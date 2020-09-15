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
