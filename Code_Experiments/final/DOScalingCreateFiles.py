import sys

if (len(sys.argv)!=2):
	print("Usage: python3 DOScalingCreateFiles.py <filename>", file=sys.stderr)
	exit(1)

filename = sys.argv[1]
with open(filename, 'r') as ff:
	lines = ff.readlines()

lines = list(map(lambda x : x.rstrip(), lines))
line1Li = list(map(lambda x : int(x), lines[0].split()))
numQ = line1Li[0]
searchableAttr = line1Li[1]

assert(len(lines) == numQ+1)
for ii in range(numQ):
	if (ii % 100 == 0):
		print("Reached ii = ", ii)
	with open('./data/split/' + str(ii) + '.txt', 'w') as ff:
		ff.write("1\t52\t2\n")
		ff.write(lines[ii+1])
