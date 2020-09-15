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
