import sys, os, re, operator
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

if (len(sys.argv) < 3):
	print "Usage : python analyse.py <savepdf/savepng/showfig> <output folder for saving graphs>"
	exit(-1)

graphSaveFolder = sys.argv[2]
displayOption = -1
if (sys.argv[1] == "savepdf"):
	displayOption = 0
elif (sys.argv[1] == "savepng"):
	displayOption = 1
elif (sys.argv[1] == "showfig"):
	displayOption = 2
else:
	print "Usage : python analyse.py <savepdf/savepng/showfig> <output folder for saving graphs>"
	exit(-1)

CommBandwidthKBps = 12.5*(1<<10) #100Mbps WAN
# CommBandwidthKBps = 620*(1<<10)  #620MBps LAN
						  #KBps #Used to approximate time taken to transfer GC from auth to server in Hamming protocol
						  #	Since GC is currently written to disk by auth and then read from disk by server, to get total time
						  # approximate the time it would take to transfer this.
datafolder = '/mnt/c/Users/t-niskum/Documents/SrchComp/esorics/mergedlogs'
# datafolder = '/mnt/c/Users/t-niskum/Documents/SrchComp/USENIX_Submission/Logs_MAF_ChiSq_ValRet_Hamming/merged_relevant_logs'

# applications = ["MAF", "ChiSq", "ValueRet"] # WARNING : Don't change relative order : in plotSingleDO() dependence is taken on order in which appAreas appear
applications = ["MAF", "ChiSq", "ValueRet", "Hamming"] # WARNING : Don't change relative order : in plotSingleDO() dependence is taken on order in which appAreas appear
applicationsPlotStrings = ["MAF", "ChiSq", "GenomeRet", "Av. Hamming"]
# applications = ["MAF", "ChiSq"] # WARNING : Don't change relative order : in plotSingleDO() dependence is taken on order in which appAreas appear
# applications = ["ValueRet", "Hamming"]
# applications = ["Hamming"]
# applications = ["ChiSq"]
# applications = ["MAF","ValueRet"]
# applications = ["MAF"]
# applications = ["ValueRet"]
# applicationsPlotStrings = ["ValueRet"]

#Following 3 variables should remain in sync
numdb=5
fileSizes = {
	"MAF": ["20000", "40000", "60000", "80000", "100000"],
	"ChiSq": ["20000", "40000", "60000", "80000", "100000"],
	"ValueRet": ["20000", "40000", "60000", "80000", "100000"],
	# "ValueRet": ["20000", "40000", "60000", "80000", "100000", "2000000"],
	"Hamming": ["5000", "10000", "15000", "20000", "25000"]
}
snpSizes = ["50", "50", "50", "50", "50"]
# snpSizes = ["50", "50", "50", "50", "50", "50"]

entityNameFileSuffices = ["Client", "Server", "AuxServer", "setup"]
# entityNameFileSuffices = ["Client", "Server", "AuxServer"]
entityRegexName = ["Client", "Server", "Authority", "setup"] #BEWARE : Don't change the order of entities in this list or bad things will happen 
# entityRegexName = ["Client", "Server", "Authority"] #BEWARE : Don't change the order of entities in this list or bad things will happen 

numQueriesPerFile=10
debugmode=False

applicationColors = {"MAF":"r","ChiSq":"b","ValueRet":"g","Hamming":"m","AddHom":"k"}

'''
Structure of goddata:
(MAF,10k) -> (clientd, serverd, auxserverd, setupd)
client/auxserverd = [se time, se comm, dcfe time, dcfe comm,-,-,-]
serverd = [se time, se comm, dcfe time, dcfe comm, num of docs matching least freq keyword, num of filtered docs, num of gates (in case of ChiSq/Hamming) or for others -]
setupd = [unencrypted size, xset gen time, xset size, tset gen time, tset size, IP/ValueRet gen time, IP/ValueRet size]

For mutliDO,
setupd = [DOData, ServData, AuxServData]
either of the above 3 = [se time, se comm, dcfe time, dcfe comm]

***********************************************************************************

Analysis of total time taken
- For query phase
	- For MAF,ChiSq,ValueRet,AddHome: SE time taken: total SE time taken by server
							  		  QE time taken: total time client SE + DCFE
	- For hamming: SE time taken: total SE time taken by server
				   DCFE time taken: computation time of (A+S) + (total comm b/w A and S/bandw)
NOTE: For all the above except DCFE hamming, total time is required to be parsed. For DCFE hamming,
		actual computation time is required and not the total time.
- For setup phase
	- Single DO: DO computation time + (DO communication/bandwidth)
	- Multi DO: 
		- For collect phase: DO computaton time + (DO comm/bandw)
		- For merge phase: (S + A) computation time + ((S-A) comm/bandw)
		- For setup phase: computation time + comm/bandw
		- Overall, total computation time of (DO + S + A) + (total comm/bandw)
'''
goddata = {}
goddataForMultiDO = [{},{}] #EqCheck, OPRF

def mapTime(x):
	#return no. of sec
	return x/(1.0*1000000)

def mapSize(x):
	#return no. of KB
	return x/((1<<10))

###################################
#		Parsing Functions
###################################

# Single DO Parsing functions
def parseQueryFileForValueRetOrAddHom(filename, entity, appArea, includeNetworkTime=False):
	datafound = [] #has similar structere as clientd in above comment
	actualSETimePattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	totalSETimePattern = '\[' + entity + ' SE Time\]: Total time = (\d+), .*'
	actualDCFETimePattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	totalDCFETimePattern = '\[' + entity + ' DCFE Time\]: Total time = (\d+), .*'

	setimepattern = totalSETimePattern if includeNetworkTime else actualSETimePattern
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	dcfetimepattern = totalDCFETimePattern if includeNetworkTime else actualDCFETimePattern
	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	indvTimeIgnorePattern = ('\[' + entity + ' (SE|DCFE) Time\] ?: Actual time = (\d+)') if includeNetworkTime else ('\[' + entity + ' (SE|DCFE) Time\]: Total time = \d+, .*')
	leastFreqKwDocsSizePattern = '\[Final Data\]: T.size\(\) is (\d+)'
	filteredDocsSizePattern = '\[Final Data\]: Size of documents retrieved is (\d+)'

	curtuple = [0,0,0,0,0,0,0]
	ct = 0
	numLinesPerQuery = 8
	if (entity == "Server"):
		numLinesPerQuery = 10
	with open(filename) as f:
		for line in f:
			if (line == ""):
				continue
			# print "^", line
			if (re.search(indvTimeIgnorePattern, line, re.IGNORECASE)):
				pass
			elif (re.search(setimepattern, line, re.IGNORECASE)):
				ff = re.search(setimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[0] += long(ff.group(1))
			elif (re.search(secommpattern, line, re.IGNORECASE)):
				ff = re.search(secommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[1] += (long(ff.group(1)) + long(ff.group(2)))
			elif (re.search(dcfetimepattern, line, re.IGNORECASE)):
				ff = re.search(dcfetimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] += long(ff.group(1))
			elif (re.search(dcfecommpattern, line, re.IGNORECASE)):
				ff = re.search(dcfecommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[3] += (long(ff.group(1)) + long(ff.group(2)))
			elif (re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[4] += long(ff.group(1))
			elif (re.search(filteredDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(filteredDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[5] += long(ff.group(1))
			else:
				assert False
			ct+=1
			# print "%", curtuple
			if (ct%numLinesPerQuery == 0):
				datafound.append(curtuple)
				curtuple = [0,0,0,0,0,0,0]
	
	# print ct,numLinesPerQuery,numQueriesPerFile
	assert ct == numLinesPerQuery*numQueriesPerFile
	if debugmode:
		print "DataFound : ", filename, entity, datafound

	avDataFound = [0,0,0,0,0,0,0]
	for xx in datafound:
		for i in xrange(7):
			avDataFound[i] += xx[i] 

	# NOTE : For ChiSq case, i am not individually adding case and control group values. 
	#		 Rather its getting implicitly accommodated when i find the sum of all entries and divide by 10.
	for i in xrange(7):
		avDataFound[i] /= (1.0*numQueriesPerFile)

	# NOTE : When finding each entity's individual comm sizes, don't divide by two. When finding the whole thing, then divide by 2.
	# avDataFound[1] /= 2.0
	# avDataFound[3] /= 2.0

	#get apt scales
	avDataFound[0] = mapTime(avDataFound[0])
	avDataFound[2] = mapTime(avDataFound[2])
	avDataFound[1] = mapSize(avDataFound[1])
	avDataFound[3] = mapSize(avDataFound[3])

	# print "$$", entity, avDataFound

	return avDataFound

def parseQueryFileForMAF(filename, entity, appArea, includeNetworkTime=False):
	datafound = [] #has similar structere as clientd in above comment
	actualSETimePattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	totalSETimePattern = '\[' + entity + ' SE Time\]: Total time = (\d+), .*'
	actualDCFETimePattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	totalDCFETimePattern = '\[' + entity + ' DCFE Time\]: Total time = (\d+), .*'

	setimepattern = totalSETimePattern if includeNetworkTime else actualSETimePattern
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	dcfetimepattern = totalDCFETimePattern if includeNetworkTime else actualDCFETimePattern
	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	indvTimeIgnorePattern = ('\[' + entity + ' (SE|DCFE) Time\] ?: Actual time = (\d+)') if includeNetworkTime else ('\[' + entity + ' (SE|DCFE) Time\]: Total time = \d+, .*')
	leastFreqKwDocsSizePattern = '\[Final Data\]: T.size\(\) is (\d+)'
	filteredDocsSizePattern = '\[Final Data\]: Size of documents retrieved is (\d+)'
	OblivCGateCtPattern = '\[Final OblivC\] : Gate count : (\d+)'
	OblivCBytesSentPattern = '\[Final OblivC\] : Bytes sent = (\d+)'

	otherPartyCommToAddTotal = 0
	curtuple = [0,0,0,0,0,0,0]
	ct = 0
	if (entity == entityRegexName[1]):
		numLinesPerQuery = 12
	elif (entity == entityRegexName[2]):
		numLinesPerQuery = 10
	else:
		numLinesPerQuery = 8
	with open(filename) as f:
		for line in f:
			if (line == ""):
				continue
			# print "^", line
			if (re.search(indvTimeIgnorePattern, line, re.IGNORECASE)):
				pass
			elif (re.search(setimepattern, line, re.IGNORECASE)):
				ff = re.search(setimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[0] += long(ff.group(1))
			elif (re.search(secommpattern, line, re.IGNORECASE)):
				ff = re.search(secommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[1] += (long(ff.group(1)) + long(ff.group(2)))
			elif (re.search(dcfetimepattern, line, re.IGNORECASE)):
				ff = re.search(dcfetimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] += long(ff.group(1))
			elif (re.search(dcfecommpattern, line, re.IGNORECASE)):
				ff = re.search(dcfecommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[3] += (long(ff.group(1)) + long(ff.group(2)))
			elif (re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[4] += long(ff.group(1))
			elif (re.search(filteredDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(filteredDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[5] += long(ff.group(1))
			elif (re.search(OblivCGateCtPattern, line, re.IGNORECASE)):
				ff = re.search(OblivCGateCtPattern, line, re.IGNORECASE)
				assert ff
				curtuple[6] += long(ff.group(1))
			elif (re.search(OblivCBytesSentPattern, line, re.IGNORECASE)):
				ff = re.search(OblivCBytesSentPattern, line, re.IGNORECASE)
				assert ff
				curtuple[3] += long(ff.group(1))
				otherPartyCommToAddTotal += long(ff.group(1))
			else:
				assert False
			ct+=1
			# print "%", curtuple
			if (ct%numLinesPerQuery == 0):
				datafound.append(curtuple)
				curtuple = [0,0,0,0,0,0,0]
	
	# print ct,numLinesPerQuery,numQueriesPerFile
	assert ct == numLinesPerQuery*numQueriesPerFile
	if debugmode:
		print "DataFound : ", filename, entity, datafound

	avDataFound = [0,0,0,0,0,0,0]
	for xx in datafound:
		for i in xrange(7):
			avDataFound[i] += xx[i] 

	# NOTE : For ChiSq case, i am not individually adding case and control group values. 
	#		 Rather its getting implicitly accommodated when i find the sum of all entries and divide by 10.
	for i in xrange(7):
		avDataFound[i] /= (1.0*numQueriesPerFile)

	avOtherPartyCommToAdd = otherPartyCommToAddTotal/(1.0*numQueriesPerFile)

	# NOTE : When finding each entity's individual comm sizes, don't divide by two. When finding the whole thing, then divide by 2.
	# avDataFound[1] /= 2.0
	# avDataFound[3] /= 2.0

	#get apt scales
	avOtherPartyCommToAdd = mapSize(avOtherPartyCommToAdd)
	avDataFound[0] = mapTime(avDataFound[0])
	avDataFound[2] = mapTime(avDataFound[2])
	avDataFound[1] = mapSize(avDataFound[1])
	avDataFound[3] = mapSize(avDataFound[3])

	# print "$$", entity, avDataFound

	return (avDataFound, avOtherPartyCommToAdd)

def parseQueryFileForChiSq(filename, entity, appArea, includeNetworkTime=False):
	datafound = [] #has similar structere as clientd in above comment
	actualSETimePattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	totalSETimePattern = '\[' + entity + ' SE Time\]: Total time = (\d+), .*'
	actualDCFETimePattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	totalDCFETimePattern = '\[' + entity + ' DCFE Time\]: Total time = (\d+), .*'

	setimepattern = totalSETimePattern if includeNetworkTime else actualSETimePattern
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	dcfetimepattern = totalDCFETimePattern if includeNetworkTime else actualDCFETimePattern
	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	OblivCGateCtPattern = '\[Final OblivC\] : Gate count : (\d+)'
	OblivCBytesSentPattern = '\[Final OblivC\] : Bytes sent = (\d+)'
	indvTimeIgnorePattern = ('\[' + entity + ' (SE|DCFE) Time\] ?: Actual time = (\d+)') if includeNetworkTime else ('\[' + entity + ' (SE|DCFE) Time\]: Total time = \d+, .*')
	leastFreqKwDocsSizePattern = '\[Final Data\]: T.size\(\) is (\d+)'
	filteredDocsSizePattern = '\[Final Data\]: Size of documents retrieved is (\d+)'
	curtuple = [0,0,0,0,0,0,0]
	ct = 0
	numLinesPerQuery = 0
	otherPartyCommToAddTotal = 0
	if (entity == entityRegexName[0]):# for client
		numLinesPerQuery = 16
	elif (entity == entityRegexName[1]):
		numLinesPerQuery = 22
	else:
		numLinesPerQuery = 18
	with open(filename) as f:
		for line in f:
			if (line == ""):
				continue
			if (re.search(indvTimeIgnorePattern, line, re.IGNORECASE)):
				pass 
			elif (re.search(setimepattern, line, re.IGNORECASE)):
				ff = re.search(setimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[0] += long(ff.group(1))
			elif (re.search(secommpattern, line, re.IGNORECASE)):
				ff = re.search(secommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[1] += (long(ff.group(1)) + long(ff.group(2)))
			elif (re.search(dcfetimepattern, line, re.IGNORECASE)):
				ff = re.search(dcfetimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] += long(ff.group(1))
			elif (re.search(dcfecommpattern, line, re.IGNORECASE)):
				ff = re.search(dcfecommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[3] += (long(ff.group(1)) + long(ff.group(2)))
			elif (re.search(OblivCGateCtPattern, line, re.IGNORECASE)):
				ff = re.search(OblivCGateCtPattern, line, re.IGNORECASE)
				assert ff
				curtuple[6] += long(ff.group(1))
			elif (re.search(OblivCBytesSentPattern, line, re.IGNORECASE)):
				ff = re.search(OblivCBytesSentPattern, line, re.IGNORECASE)
				assert ff
				curtuple[3] += long(ff.group(1))
				otherPartyCommToAddTotal += long(ff.group(1))
			elif (re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[4] += long(ff.group(1)) #This will only occur for server -- also divide by 2 for case/control
			elif (re.search(filteredDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(filteredDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[5] += long(ff.group(1)) #Divide by 2 for case/control.
			else:
				assert False
			# print curtuple
			ct+=1
			if (ct%numLinesPerQuery == 0):
				#one query is finished
				datafound.append(curtuple)
				curtuple = [0,0,0,0,0,0,0]

	assert ct == numLinesPerQuery*numQueriesPerFile
	if debugmode:
		print "DataFound : ", filename, entity, datafound

	avDataFound = [0,0,0,0,0,0,0]
	for xx in datafound:
		for i in xrange(7):
			avDataFound[i] += xx[i] 

	for i in xrange(7):
		avDataFound[i] /= (1.0*numQueriesPerFile)

	avOtherPartyCommToAdd = otherPartyCommToAddTotal/(1.0*numQueriesPerFile)

	# NOTE : When finding each entity's individual comm sizes, don't divide by two. When finding the whole thing, then divide by 2.
	# avDataFound[1] /= 2.0
	# avDataFound[3] /= 2.0

	#get apt scales
	avOtherPartyCommToAdd = mapSize(avOtherPartyCommToAdd)
	avDataFound[0] = mapTime(avDataFound[0])
	avDataFound[2] = mapTime(avDataFound[2])
	avDataFound[1] = mapSize(avDataFound[1])
	avDataFound[3] = mapSize(avDataFound[3])

	return (avDataFound, avOtherPartyCommToAdd)

def parseQueryFileForHamming(filename, entity, appArea, includeNetworkTime=False):
	datafound = [] #has similar structere as clientd in above comment
	actualSETimePattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	totalSETimePattern = '\[' + entity + ' SE Time\]: Total time = (\d+), .*'
	actualDCFETimePattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	totalDCFETimePattern = '\[' + entity + ' DCFE Time\]: Total time = (\d+), .*'

	setimepattern = totalSETimePattern if includeNetworkTime else actualSETimePattern
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	
	# Intentionally the below statement is independent of includeNetworkTime.
	#	Since, the diff in total time and actual time is network costs and in hamming, we already 
	#	need to add a lot of comm from outside, account for comm through sockets in the total comm only.
	dcfetimepattern = actualDCFETimePattern 

	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	indvTimeIgnorePatternSE = ('\[' + entity + ' SE Time\] ?: Actual time = (\d+)') if includeNetworkTime else ('\[' + entity + ' SE Time\]: Total time = \d+, .*')
	indvTimeIgnorePatternDCFE = '\[' + entity + ' DCFE Time\]: Total time = \d+, .*'

	excludeTimePattern = '\[Final Hamm\]: Exclude time:.*= (\d+) microseconds'
	circuitGenTimePattern = '\[Final Hamm\]: Circuit Generation time is (\d+.?\d*)'
	garbledSizePattern = '\[Final Hamm\] : Size of garbled table is (\d+), Number of gates =(\d+)'
	circuitEvalTimePattern = '\[Final Hamm\]: Circuit evaluation time is (\d+.?\d*)'
	labelSizeFromAuthPattern = '\[Final Hamm\]: Label size sent from authority to server : (\d+)'
	clientEvalTimePattern = '\[Final Hamm\]: Client takes to calculate (\d+.?\d*)'
	leastFreqKwDocsSizePattern = '\[Final Data\]: T.size\(\) is (\d+)'
	filteredDocsSizePattern = '\[Final Data\]: Size of documents retrieved is (\d+)'

	numLinesPerQuery = 0
	if (entity == entityRegexName[2]): #for auxserver
		numLinesPerQuery = 13
	elif (entity == entityRegexName[1]): #for server
		numLinesPerQuery = 15
	else:
		numLinesPerQuery = 8

	curtuple = [0,0,0,0,0,0,0]
	ct = 0
	commCostToAddToOtherParty = 0 # In auth log parsing will find something which needs to be added to server comm cost and 
								  # and in server log parsing will find something which needs to be added to auth comm cost.
	avCommCostToAddToOtherParty = 0
	allPatternsForCurrentQFound = [False]*10
	with open(filename) as f:
		for line in f:
			if (line == ""):
				continue
			# print line
			if (re.search(indvTimeIgnorePatternSE, line, re.IGNORECASE) or re.search(indvTimeIgnorePatternDCFE, line, re.IGNORECASE)):
				allPatternsForCurrentQFound[0] = True
				pass 
			elif (re.search(setimepattern, line, re.IGNORECASE)):
				ff = re.search(setimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[0] += long(ff.group(1))
				allPatternsForCurrentQFound[1] = True
			elif (re.search(secommpattern, line, re.IGNORECASE)):
				ff = re.search(secommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[1] += (long(ff.group(1)) + long(ff.group(2)))
				allPatternsForCurrentQFound[2] = True
			elif (re.search(dcfetimepattern, line, re.IGNORECASE)):
				ff = re.search(dcfetimepattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] += long(ff.group(1))
				allPatternsForCurrentQFound[3] = True
			elif (re.search(dcfecommpattern, line, re.IGNORECASE)):
				ff = re.search(dcfecommpattern, line, re.IGNORECASE)
				assert ff
				curtuple[3] += (long(ff.group(1)) + long(ff.group(2)))
				allPatternsForCurrentQFound[4] = True
			elif (re.search(excludeTimePattern, line, re.IGNORECASE)):
				ff = re.search(excludeTimePattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] -= long(ff.group(1)) #subtract the exclude time
				allPatternsForCurrentQFound[5] = True
			elif (re.search(circuitGenTimePattern, line, re.IGNORECASE)):
				ff = re.search(circuitGenTimePattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] += long(float(ff.group(1))*1000*1000) # input is time in seconds -- convert to microsec and add
				allPatternsForCurrentQFound[6] = True
			elif (re.search(circuitEvalTimePattern, line, re.IGNORECASE)):
				ff = re.search(circuitEvalTimePattern, line, re.IGNORECASE)
				assert ff
				curtuple[2] += long(float(ff.group(1))*1000*1000) # input is time in seconds -- convert to microsec and add
				allPatternsForCurrentQFound[6] = True
			elif (re.search(clientEvalTimePattern, line, re.IGNORECASE)):
				pass # for now since client evaluation time is practically 0, ignore it.
			elif (re.search(garbledSizePattern, line, re.IGNORECASE)):
				ff = re.search(garbledSizePattern, line, re.IGNORECASE)
				assert ff
				sizeGCT = long(ff.group(1))
				numGates = long(ff.group(2))
				
				curtuple[6] += numGates
				curtuple[3] += sizeGCT #This is authority and so add the gc table size to the comm cost
				assert(commCostToAddToOtherParty == 0) #Assert that this variable is not changed yet

				commCostToAddToOtherParty = sizeGCT #Save this so that it can be added to server's comm cost
				allPatternsForCurrentQFound[7] = True
			elif (re.search(labelSizeFromAuthPattern, line, re.IGNORECASE)):
				ff = re.search(labelSizeFromAuthPattern, line, re.IGNORECASE)
				assert ff
				sizeLabels = long(ff.group(1)) #Size in bytes of labels sent from auth to server
											   #This log comes only in server log -- save this so that it can be added to auth comm cost also.	

				#This is server log -- add to server comm cost and save to be added to auth's comm cost
				curtuple[3] += sizeLabels
				assert(commCostToAddToOtherParty == 0) #Assert this is unchanged -- this holds because garbledSizePattern occurs in auth and this pattern occurs in server

				commCostToAddToOtherParty = sizeLabels
				allPatternsForCurrentQFound[7] = True
			elif (re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(leastFreqKwDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[4] += long(ff.group(1))
				allPatternsForCurrentQFound[8] = True
			elif (re.search(filteredDocsSizePattern, line, re.IGNORECASE)):
				ff = re.search(filteredDocsSizePattern, line, re.IGNORECASE)
				assert ff
				curtuple[5] += long(ff.group(1))
				allPatternsForCurrentQFound[9] = True
			else:
				print line
				assert False
			ct+=1
			# print curtuple
			if (ct%numLinesPerQuery == 0):
				#end of one query
				if (entity == "Client"):
					for ii in xrange(5): assert(allPatternsForCurrentQFound[ii] == True)
				elif (entity == "Authority"):
					for ii in xrange(8): assert(allPatternsForCurrentQFound[ii] == True)
				elif (entity == "Server"):
					for ii in xrange(10): assert(allPatternsForCurrentQFound[ii] == True)
				datafound.append(curtuple)
				curtuple = [0,0,0,0,0,0,0]
				allPatternsForCurrentQFound = [False]*10
				avCommCostToAddToOtherParty += commCostToAddToOtherParty
				commCostToAddToOtherParty = 0
	
	assert ct == numLinesPerQuery*numQueriesPerFile

	if debugmode:
		print "DataFound : ", filename, entity, datafound

	avDataFound = [0,0,0,0,0,0,0]
	for xx in datafound:
		for i in xrange(7):
			avDataFound[i] += xx[i] 

	for i in xrange(7):
		avDataFound[i] /= (1.0*numQueriesPerFile)

	avCommCostToAddToOtherParty /= (1.0*numQueriesPerFile)

	# NOTE : When finding each entity's individual comm sizes, don't divide by two. When finding the whole thing, then divide by 2.
	# avDataFound[1] /= 2.0
	# avDataFound[3] /= 2.0

	#get apt scales
	avDataFound[0] = mapTime(avDataFound[0])
	avDataFound[2] = mapTime(avDataFound[2])
	avDataFound[1] = mapSize(avDataFound[1])
	avDataFound[3] = mapSize(avDataFound[3])
	avCommCostToAddToOtherParty = mapSize(avCommCostToAddToOtherParty)

	return (avDataFound, avCommCostToAddToOtherParty)

def parseSetupFileForSingleDO(filename):
	allPatterns = []
	allPatterns.append("\[Final SE\]: Size of unencrypted input file is filelen = \((\d+)\) bytes")
	allPatterns.append("\[Final SE\]: XSet generation takes (\d+) microseconds")
	allPatterns.append("\[Final SE\]: Size of XSet data produced is (\d+) bytes.")
	allPatterns.append("\[Final SE\]: TSet generation takes (\d+) microseconds")
	allPatterns.append("\[Final SE\]: Size of TSet data produced is (\d+) bytes.")
	allPatterns.append("\[Final IP\]: XSetIP generation takes (\d+) microsecond")
	allPatterns.append("\[Final IP\]: XSetIP size = (\d+)")
	allPatterns.append("\[Final ValueRet\]: Dataset generation takes (\d+) microseconds")
	allPatterns.append("\[Final ValueRet\]: Size of encrypted data is (\d+) bytes")
	allPatterns.append("\[Final Hamm\]: XfuncGC generation takes (\d+) microseconds")
	allPatterns.append("\[Final Hamm\]: XfuncGC size = (\d+) bytes.")
	datafound = [0,0,0,0,0,0,0]
	with open(filename) as f:
		for line in f:
			if line == "":
				continue
			# print "Matching line :::", line
			matchedPattIdx = -1
			for i,patt in enumerate(allPatterns):
				ff = re.search(patt, line, re.IGNORECASE)
				if ff:
					matchedPattIdx = i
					break
			assert matchedPattIdx != -1
			ff = re.search(allPatterns[matchedPattIdx], line, re.IGNORECASE)
			assert ff
			curData = long(ff.group(1))
			if (matchedPattIdx <= 4): 
				datafound[matchedPattIdx] = curData
			elif (matchedPattIdx == 5 or matchedPattIdx == 7 or matchedPattIdx == 9): 
				datafound[5] = curData
			elif (matchedPattIdx == 6 or matchedPattIdx == 8 or matchedPattIdx == 10):
				datafound[6] = curData

	#convert to apt scales
	for i in [0,2,4,6]:
		datafound[i] = mapSize(datafound[i])

	for i in [1,3,5]:
		datafound[i] = mapTime(datafound[i])

	print "Parsed file : ", filename
	print "Data found : ", datafound
	return datafound

def parseAndAddToGodData(appArea, singleDO, skipSetup, isProtocolOPRFOne, fileNamePrefixPrefix=None, includeNetworkTime=False):
	global goddata, goddataForMultiDO
	dirpath = datafolder
	for i in xrange(numdb):
		curFileSize = fileSizes[appArea][i]
		curSnpSize = snpSizes[i]
		if fileNamePrefixPrefix is not None:
			fileNamePrefix = fileNamePrefixPrefix+"log_" + curFileSize + "_" + curSnpSize + "_" + appArea +"0_"
		else:
			fileNamePrefix = "log_" + curFileSize + "_" + curSnpSize + "_" + appArea +"0_"
		curFileDataPointData = []
		hammAuthReportedCommCostToAddToServer = 0
		hammServerReportedCommCostToAddToAuth = 0
		chiSqAuxSrvrReportedCommCostToAddToServer = 0
		chiSqServerReportedCommCostToAddToAuxSrvr = 0
		for j in xrange(len(entityNameFileSuffices)):
			curEntity = entityNameFileSuffices[j]
			fileNameFull = os.path.join(dirpath, fileNamePrefix + curEntity)
			print "Parsing file :", fileNameFull
			if (curEntity == "setup"):
				if (not(skipSetup)):
					dataForFile = parseSetupFileForSingleDO(fileNameFull)
				else:
					dataForFile = []
			else:
				dataForFile = None
				if (appArea == "ValueRet" or appArea == "AddHom"):
					dataForFile = parseQueryFileForValueRetOrAddHom(fileNameFull, entityRegexName[j], appArea, includeNetworkTime=includeNetworkTime)
				elif (appArea == "ChiSq" or appArea == "MAF"):
					if appArea == "ChiSq":
						(dataForFile, avOtherPartyCommToAdd) = parseQueryFileForChiSq(fileNameFull, entityRegexName[j], appArea, includeNetworkTime=includeNetworkTime)
					else:
						(dataForFile, avOtherPartyCommToAdd) = parseQueryFileForMAF(fileNameFull, entityRegexName[j], appArea, includeNetworkTime=includeNetworkTime)
					if (entityRegexName[j] == "Authority"):
						chiSqAuxSrvrReportedCommCostToAddToServer = avOtherPartyCommToAdd
					elif (entityRegexName[j] == "Server"):
						chiSqServerReportedCommCostToAddToAuxSrvr = avOtherPartyCommToAdd
				elif (appArea == "Hamming"):
					(dataForFile, avCommCostToAddToOtherParty) = parseQueryFileForHamming(fileNameFull, entityRegexName[j], appArea, includeNetworkTime=includeNetworkTime)
					if (entityRegexName[j] == "Authority"):
						hammAuthReportedCommCostToAddToServer = avCommCostToAddToOtherParty
					elif (entityRegexName[j] == "Server"):
						hammServerReportedCommCostToAddToAuth = avCommCostToAddToOtherParty
				else:
					assert False
			if debugmode:
				print "(Apparea, Datapoint) =", appArea, curFileSize
				print "dataForFile =", dataForFile
			curFileDataPointData.append(dataForFile)

		if (appArea == "Hamming"):
			print curFileDataPointData
			#Add the auth reported and server reported comm cost to other party
			curFileDataPointData[1][3] += hammAuthReportedCommCostToAddToServer # Add auth reported comm cost to server's data point's dcfe comm cost 
			curFileDataPointData[2][3] += hammServerReportedCommCostToAddToAuth # Add server reported comm cost to auth's data point's dcfe comm cost
			if includeNetworkTime:
				# If want to get total time, add the time it would take to transfer GC and labels over network
				totalTimeToTranferGCAndLabels = (hammAuthReportedCommCostToAddToServer + hammServerReportedCommCostToAddToAuth)/(1.0*CommBandwidthKBps)
				curFileDataPointData[1][2] += totalTimeToTranferGCAndLabels #Add to server
				curFileDataPointData[2][2] += totalTimeToTranferGCAndLabels	#Add to auth

			print "(AppArea, curFileSize, authReportedCommCostToAddToServer, serverReportedCommCostToAddToAuth) = (", appArea, ",", curFileSize, ",", hammAuthReportedCommCostToAddToServer, ",", hammServerReportedCommCostToAddToAuth, ")"

		if (appArea == "ChiSq" or appArea=="MAF"):
			print curFileDataPointData
			curFileDataPointData[2][3] += chiSqServerReportedCommCostToAddToAuxSrvr #Server reported comm cost addint to AuxSrvr's dcfe comm cost
			curFileDataPointData[1][3] += chiSqAuxSrvrReportedCommCostToAddToServer #AuxSrvr reported comm cost adding to server's dcfe comm cost
			if includeNetworkTime:
				# If want to get total time, add the time it would take to transfer GC and labels over network
				totalTimeToTransferOblivC = (chiSqServerReportedCommCostToAddToAuxSrvr + chiSqAuxSrvrReportedCommCostToAddToServer)/(1.0*CommBandwidthKBps)
				curFileDataPointData[1][2] += totalTimeToTransferOblivC #Add to server
				curFileDataPointData[2][2] += totalTimeToTransferOblivC	#Add to auth

			print "(AppArea, curFileSize, chiSqAuxSrvrReportedCommCostToAddToServer, chiSqServerReportedCommCostToAddToAuxSrvr) = (", appArea, ",", curFileSize, ",", chiSqAuxSrvrReportedCommCostToAddToServer, ",", chiSqServerReportedCommCostToAddToAuxSrvr, ")"
		if (singleDO): 
			goddata[(appArea, curFileSize)] = curFileDataPointData
		else:
			#MultiDO
			goddataForMultiDO[isProtocolOPRFOne][(appArea, curFileSize)] = curFileDataPointData

def getSetupGraphsDataListFromGodData(curAppArea, idx):
	global goddata, fileSizes
	datali = []
	for curFileSize in fileSizes[curAppArea]:
		datali.append(goddata[(curAppArea, curFileSize)][3][idx])
	return datali

# Multi DO Parsing functions
def helper_parseMultiDO_CollectMerge_Files(filename, entity):
	##return for that entity (se time, se comm, dcfe time, dcfe comm)

	print "Parsing file :", filename

	qeCommPattern = '\[Output Comm:\] ' + entity + ' has send/recieve bytes (\d+)'
	qeTimePattern = '\[Output Time:\] ' + entity + ' takes (\d+) microseconds'
	seCommPattern = '\[Output Comm:\] ' + entity + ' SE has send/recieve bytes (\d+)'
	seTimePattern = '\[Output Time:\] ' + entity + ' SE takes (\d+) microseconds' 
	data = [0,0,0,0] #se time, se commm, dcfe time, dcfe comm
	ct = 0
	with open(filename) as f:
		for line in f:
			if (line == ""):
				continue
			# print "^", line
			if (re.search(qeCommPattern, line, re.IGNORECASE)):
				ff = re.search(qeCommPattern, line, re.IGNORECASE)
				assert ff
				data[3] = long(ff.group(1))
			elif (re.search(qeTimePattern, line, re.IGNORECASE)):
				ff = re.search(qeTimePattern, line, re.IGNORECASE)
				assert ff
				data[2] = long(ff.group(1))
			elif (re.search(seCommPattern, line, re.IGNORECASE)):
				ff = re.search(seCommPattern, line, re.IGNORECASE)
				assert ff
				data[1] = long(ff.group(1))
			elif (re.search(seTimePattern, line, re.IGNORECASE)):
				ff = re.search(seTimePattern, line, re.IGNORECASE)
				assert ff
				data[0] = long(ff.group(1))
			else:
				assert False
			# print "$", secomm, setiming
			ct+=1
	assert ct==4
	# subtract qe - se to get data for dcfe
	data[2] -= data[0]
	data[3] -= data[1]

	for i in xrange(4): assert data[i]>=-50

	data[0] = mapTime(data[0])
	data[2] = mapTime(data[2])

	data[1] = mapSize(data[1])
	data[3] = mapSize(data[3])

	return data

def helper_parseMultiDOSetupFiles(filename):
	print "Parsing file :", filename

	xsetGenTimePattern = '\[Final SE\]: XSet generation takes (\d+) microseconds'
	xsetGenSizePattern = '\[Final SE\]: Size of XSet data produced is (\d+) bytes.'
	tsetGenTimePattern = '\[Final SE\]: TSet generation takes (\d+) microseconds'
	tsetGenSizePattern = '\[Final SE\]: Size of TSet data produced is (\d+) bytes.'
	data = [0,0,0,0]
	ct = 0
	with open(filename) as f:
		for line in f:
			if (line == ""):
				continue
			if (re.search(xsetGenTimePattern, line, re.IGNORECASE)):
				ff = re.search(xsetGenTimePattern, line, re.IGNORECASE)
				assert ff
				data[0] = long(ff.group(1))
			elif (re.search(xsetGenSizePattern, line,  re.IGNORECASE)):
				ff = re.search(xsetGenSizePattern, line, re.IGNORECASE)
				assert ff
				data[1] = long(ff.group(1))
			elif (re.search(tsetGenTimePattern, line,  re.IGNORECASE)):
				ff = re.search(tsetGenTimePattern, line, re.IGNORECASE)
				assert ff
				data[2] = long(ff.group(1))
			elif (re.search(tsetGenSizePattern, line,  re.IGNORECASE)):
				ff = re.search(tsetGenSizePattern, line, re.IGNORECASE)
				assert ff
				data[3] = long(ff.group(1))
			else:
				assert False
			ct+=1
	assert ct==4
	data[0] = mapTime(data[0])
	data[2] = mapTime(data[2])
	data[1] = mapSize(data[1])
	data[3] = mapSize(data[3])
	return data

def parseMultiDOSetupPhase(appArea, isProtocolOPRFOne, includeNetworkTime=False):
	global goddataForMultiDO
	dirpath = datafolder
	for i in xrange(numdb):
		curFileSize = fileSizes[appArea][i]
		curSnpSize = snpSizes[i]
		fileNamePrefix = ("mDOOPRF_" if isProtocolOPRFOne else "mDOEq_") + "log_" + curFileSize + "_" + curSnpSize + "_" + appArea +"0_"

		#Collect phase
		collect_DOData = helper_parseMultiDO_CollectMerge_Files(os.path.join(dirpath,fileNamePrefix + 'collect_DO'), 'DO')
		collect_serverData = None
		if isProtocolOPRFOne:
			collect_serverData = helper_parseMultiDO_CollectMerge_Files(os.path.join(dirpath,fileNamePrefix + 'collect_Server'), 'Server')

		#Merge phase
		merge_AuxData = helper_parseMultiDO_CollectMerge_Files(os.path.join(dirpath, fileNamePrefix + 'merge_AuxServer'), 'AuxServer')
		merge_ServData = helper_parseMultiDO_CollectMerge_Files(os.path.join(dirpath, fileNamePrefix + 'merge_Server'), 'Server')

		#Setup phase
		setupPhaseData = helper_parseMultiDOSetupFiles(os.path.join(dirpath, fileNamePrefix + 'setup'))

		# print collect_DOData, collect_serverData, merge_AuxData, merge_ServData, setupPhaseData
		DOFinalData = collect_DOData
		ServFinalData = None
		if isProtocolOPRFOne:
			ServFinalData = list(map(operator.add, collect_serverData, merge_ServData))
		else:
			ServFinalData = merge_ServData
		AuxServFinalData = merge_AuxData

		#Right now all the 3 above variables have been updated with comm and time till merge-map stage
		#After this, adding setup side data to get final values

		#setup phase xset/tset gen time to auxserv and xset/tset comm size to auxserv/serv -->since the xset/tset is communicatedd from 
		#	auxserv to serv
		AuxServFinalData[0] += (setupPhaseData[0] + setupPhaseData[2]) #adding xset/tset gen time to se timme of aux
		AuxServFinalData[1] += (setupPhaseData[1] + setupPhaseData[3]) #adding xset/tset comm size to A and then S
		ServFinalData[1] += (setupPhaseData[1] + setupPhaseData[3])

		goddataForMultiDO[isProtocolOPRFOne][(appArea, curFileSize)][3] = [DOFinalData, ServFinalData, AuxServFinalData]

def parseAndAddToGodDataForMultiDO(curAppArea, isProtocolOPRFOne, includeNetworkTime=False):
	parseAndAddToGodData(curAppArea, 
						False, 
						True, 
						isProtocolOPRFOne, 
						"mDOOPRF_" if isProtocolOPRFOne else "mDOEq_", 
						includeNetworkTime=includeNetworkTime) #Use this to parse query files for multiDO in the same way as for single DO
	parseMultiDOSetupPhase(curAppArea, isProtocolOPRFOne, includeNetworkTime=includeNetworkTime)

### IMP parsing functions
def parseDataForSingleDO(includeNetworkTime=False):
	global applications
	for curAppArea in applications:
		parseAndAddToGodData(curAppArea, True, False, isProtocolOPRFOne=False, includeNetworkTime=includeNetworkTime)
	print "**************Single DO***************"
	print goddata

def parseDataForMultiDO(isProtocolOPRFOne, includeNetworkTime=False):
	global applications
	for curAppArea in applications:
		parseAndAddToGodDataForMultiDO(curAppArea, isProtocolOPRFOne, includeNetworkTime=includeNetworkTime)

	print "**************Mutli DO***************"
	print goddataForMultiDO[isProtocolOPRFOne]

###################################
#		GetData Functions
###################################

## For SingleDO/MultiDO
def getQueryGraphsListFromGodData(appArea, entityRegexNameIdx, ifSE, ifTime, singleDO, isProtocolOPRFOne=None):
	global goddata, fileSizes
	ans=[]
	assert entityRegexNameIdx<3
	ifSEInt = 0 if ifSE else 1
	ifTimeInt = 0 if ifTime else 1
	for curFileSize in fileSizes[appArea]:
		if singleDO:
			ans.append(goddata[(appArea, curFileSize)][entityRegexNameIdx][ifSEInt*2+ifTimeInt])
		else:
			assert(isProtocolOPRFOne is not None)
			ans.append(goddataForMultiDO[isProtocolOPRFOne][(appArea, curFileSize)][entityRegexNameIdx][ifSEInt*2+ifTimeInt])
	return ans

def getFilteredOrLeastFreqKwDocsSize(curAppArea, forFinalFilteredRecords, singleDO, isProtocolOPRFOne=None):
	global goddata, fileSizes
	idx = 5 if forFinalFilteredRecords else 4
	if singleDO:
		filteredRecordsLi = list(map(lambda curFileSize: goddata[(curAppArea, curFileSize)][1][idx], fileSizes[curAppArea]))
	else:
		assert(isProtocolOPRFOne is not None)
		filteredRecordsLi = list(map(lambda curFileSize: goddataForMultiDO[isProtocolOPRFOne][(curAppArea, curFileSize)][1][idx], fileSizes[curAppArea]))
	return filteredRecordsLi

def getDataForQueryPhase(isComm, isQEData, curAppArea, isSingleDO, isProtocolOPRFOne=None):
	totalSEData = [0]*numdb
	totalDCFEData = [0]*numdb
	for entityRegexNameIdx in xrange(3):
		curEntitySEData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, True, not(isComm), isSingleDO, isProtocolOPRFOne)
		curEntityDCFEData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, False, not(isComm), isSingleDO, isProtocolOPRFOne)
		totalSEData = list(map(operator.add, totalSEData, curEntitySEData))
		totalDCFEData = list(map(operator.add, totalDCFEData, curEntityDCFEData))
	if isComm:
		totalSEData = map(lambda x: x/2.0, totalSEData)
		totalDCFEData = map(lambda x: x/2.0, totalDCFEData)
	totalQEData = list(map(operator.add, totalSEData, totalDCFEData))
	if isQEData: return totalQEData
	else: return totalSEData

def getDataForSingleDOSetupPhase(isComm, isQEData, curAppArea):
	totalSEData = list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][1+isComm] + goddata[(curAppArea, curFileSize)][3][3+isComm], fileSizes[curAppArea]))
	totalDCFEData = list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][5+isComm], fileSizes[curAppArea]))
	if isComm:
		totalSEData = map(lambda x: x/2.0, totalSEData)
		totalDCFEData = map(lambda x: x/2.0, totalDCFEData)
	totalQEData = list(map(operator.add, totalSEData, totalDCFEData))
	if isQEData: return totalQEData
	else: return totalSEData

def getDataForMultiDOSetupPhase(isComm, isQEData, curAppArea, isProtocolOPRFOne):
	global goddataForMultiDO
	datali = []
	for curFileSize in fileSizes[curAppArea]:
		temp = 0
		for entityIdx in xrange(3):
			if (isComm):
				temp += goddataForMultiDO[isProtocolOPRFOne][(curAppArea, curFileSize)][3][entityIdx][1] 
				if (isQEData):
					temp += goddataForMultiDO[isProtocolOPRFOne][(curAppArea, curFileSize)][3][entityIdx][3]
			else:
				temp += goddataForMultiDO[isProtocolOPRFOne][(curAppArea, curFileSize)][3][entityIdx][0]
				if (isQEData):
					temp += goddataForMultiDO[isProtocolOPRFOne][(curAppArea, curFileSize)][3][entityIdx][2]
		if isComm:
			temp = temp/2.0
		datali.append(temp)
	return datali

#### New GetData functions for submission to USENIX
def getNewQueryPhaseData_TotalTime(curAppArea, ifSE, isSingleDO, isProtocolOPRFOne=None):
	clientRegexIdx = 0
	serverRegexIdx = 1
	auxServerRegexIdx = 2
	if ifSE:
		serverSEData = getQueryGraphsListFromGodData(curAppArea, serverRegexIdx, True, True, isSingleDO, isProtocolOPRFOne)
		return serverSEData
	else:
		if (curAppArea!="Hamming"):
			# As explained at the top, if the appArea is MAF/ChiSq/ValueRet/AddHom, total QE time is obtained from
			#	adding client SE and client DCFE.
			clientSEData = getQueryGraphsListFromGodData(curAppArea, clientRegexIdx, True, True, isSingleDO, isProtocolOPRFOne)
			clientDCFEData = getQueryGraphsListFromGodData(curAppArea, clientRegexIdx, False, True, isSingleDO, isProtocolOPRFOne)
			totalQEData = list(map(operator.add, clientSEData, clientDCFEData))
			return totalQEData
		else:
			# If appArea is Hamming, then total QE time is Server SE time + (DCFE computation time of A and S + comm/bandw)
			#	And by the flag includeNetworkTime, for hamming parsing, only computation time is parsed for DCFE and not the total time.
			serverSEData = getQueryGraphsListFromGodData(curAppArea, serverRegexIdx, True, True, isSingleDO, isProtocolOPRFOne)
			serverDCFEComputationTimeData = getQueryGraphsListFromGodData(curAppArea, serverRegexIdx, False, True, isSingleDO, isProtocolOPRFOne)
			serverDCFECommData = getQueryGraphsListFromGodData(curAppArea, serverRegexIdx, False, False, isSingleDO, isProtocolOPRFOne)
			auxServerDCFEComputationTimeData = getQueryGraphsListFromGodData(curAppArea, auxServerRegexIdx, False, True, isSingleDO, isProtocolOPRFOne)
			auxServerDCFECommData = getQueryGraphsListFromGodData(curAppArea, auxServerRegexIdx, False, False, isSingleDO, isProtocolOPRFOne)
			totalQEData = list(map(operator.add, serverDCFEComputationTimeData, auxServerDCFEComputationTimeData))
			totalQEData = list(map(operator.add, totalQEData, serverSEData))
			dcfeTotalComm = list(map(operator.add, serverDCFECommData, auxServerDCFECommData))
			dcfeTotalComm = list(map(lambda x : x/2, dcfeTotalComm))
			dcfeTotalCommTime = list(map(lambda x : x/CommBandwidthKBps, dcfeTotalComm))
			totalQEData = list(map(operator.add, totalQEData, dcfeTotalCommTime))
			return totalQEData

def getNewSetupPhaseData_TotalTime(curAppArea, ifSE, isSingleDO, isProtocolOPRFOne=None):
	if isSingleDO:
		return getDataForSingleDOSetupPhase(False, not(ifSE), curAppArea)
	else:
		# As explained at the top, add the total computation time of DO, Server, AuxServer and add to that time for total comm
		finalData = []
		for curFileSize in fileSizes[curAppArea]:
			totalData = [0.,0.,0.,0.]
			for entityIdx in range(3):
				totalData = list(map(operator.add, totalData, goddataForMultiDO[isProtocolOPRFOne][(curAppArea, curFileSize)][3][entityIdx]))
			totalData[1] = totalData[1]/2 #Divide comm by 2 to avoid double counting
			totalData[3] = totalData[3]/2 #Divide comm by 2 to avoid double counting
			if ifSE:
				finalData.append(totalData[0] + (totalData[1]/CommBandwidthKBps))
			else:
				finalData.append(totalData[0] + totalData[2] + ((totalData[1]+totalData[3])/CommBandwidthKBps))
		return finalData


###################################
#		Plot Functions
###################################

# Helper functions
def showfigure(figureObj, saveFileName):
	if (displayOption == 0):
		figureObj.savefig(os.path.join(graphSaveFolder, saveFileName+'.pdf'), bbox_inches='tight')
	elif (displayOption == 1):
		figureObj.savefig(os.path.join(graphSaveFolder, saveFileName+'.png'))
	else:
		plt.show()

# Plot functions
def plotSingleDO(figid):
	# first plot query
	# plt.figure(figid)
	singleDO = True
	# figid+=1
	curMarkerSize=4
	fig,axes = plt.subplots(2,2,num=figid)
	plt.subplots_adjust(right=0.8,hspace=0.3)
	figid+=1
	ifTimeArr = [False, True]
	curRowSubplot = 0
	curColSubplot = 1
	legendHandles = []
	convertToMB = lambda x : x/(1.0*(1<<10)) if not(ifTime) else x
	for ifTime in ifTimeArr:
		if ifTime:
			curRowSubplot = 1
		else:
			curRowSubplot = 0

		axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))
		for curAppArea in applications:
			if (curAppArea == "Hamming"): continue ##TODO : Not plotting hamming on this right now
			totalQEData = getDataForQueryPhase(not(ifTime), True, curAppArea, singleDO)
			# if (curAppArea == "ChiSq"):
			# 	print "$$$$$$$$$$", map(convertToMB, totalQEData), getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO)
			handle, = axes[curRowSubplot,curColSubplot].plot(getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO),
				map(convertToMB, totalQEData), 
				applicationColors[curAppArea]+"o-", 
				label=curAppArea, 
				alpha=0.7, 
				markersize=curMarkerSize+1, 
				markeredgecolor='k', 
				markerfacecolor='w'
				)
			legendHandles.append(handle)
		
		handle, = axes[curRowSubplot,curColSubplot].plot(getFilteredOrLeastFreqKwDocsSize("ChiSq", True, singleDO), 
			map(convertToMB, getDataForQueryPhase(not(ifTime), False, "ChiSq", singleDO)), 
			applicationColors["ChiSq"]+"^--", 
			label='SE comp (ChiSq)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)
		# print "########", getFilteredOrLeastFreqKwDocsSize("ChiSq", True, singleDO), map(convertToMB, getDataForQueryPhase(not(ifTime), False, "ChiSq", singleDO))
		legendHandles.append(handle)

		handle, = axes[curRowSubplot,curColSubplot].plot(getFilteredOrLeastFreqKwDocsSize("MAF", True, singleDO), 
			map(convertToMB, getDataForQueryPhase(not(ifTime), False, "MAF", singleDO)), 
			applicationColors["MAF"]+"^--", 
			label='SE comp (Others)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)
		legendHandles.append(handle)
		# plt.legend(handles=plotHandles, loc='upper left')

		axes[curRowSubplot, curColSubplot].set_title('Query Gross Computation Time' if ifTime else 'Query Communication size',
													 fontdict={'fontsize':9,'weight':'bold', 'fontstyle':'italic'})
		axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)

	#then plot setup
	# plt.figure(figid)
	# figid += 1
	curColSubplot = 0
	for ifTime in ifTimeArr:
		if ifTime:
			curRowSubplot = 1
		else:
			curRowSubplot = 0

		# For every axis, set the x and y major locator
		axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
		# legendHandles = []
		for curAppArea in applications:
			handle,=axes[curRowSubplot,curColSubplot].plot(fileSizes[curAppArea], 
				map(convertToMB, getDataForSingleDOSetupPhase(not(ifTime), True, curAppArea)), 
				applicationColors[curAppArea]+"o-", 
				label=curAppArea, 
				alpha=0.7, 
				markersize=curMarkerSize+1, 
				markeredgecolor='k', 
				markerfacecolor='w'
				)
			# legendHandles.append(handle)

		handle,=axes[curRowSubplot,curColSubplot].plot(fileSizes[curAppArea], 
			map(convertToMB, getDataForSingleDOSetupPhase(not(ifTime), False, "ChiSq")), 
			applicationColors["ChiSq"]+"^--", 
			label='SE comp (ChiSq)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)
		# legendHandles.append(handle)
		handle,=axes[curRowSubplot,curColSubplot].plot(fileSizes[curAppArea], 
			map(convertToMB, getDataForSingleDOSetupPhase(not(ifTime), False, "MAF")), 
			applicationColors["MAF"]+"^--", 
			label='SE comp (Others)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)

		# legendHandles.append(handle)
		# plt.legend(handles=legendHandles, loc='upper left')
		axes[curRowSubplot, curColSubplot].set_title('Setup Gross Computation Time' if ifTime else 'Setup Communication Size',
													 fontdict={'fontsize':9,'weight':'bold', 'fontstyle':'italic'})
		axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)
	
	# Shrink current axis by 20%
	# box = plt.get_position()
	# plt.set_position([box.x0, box.y0, box.width * 0.8, box.height])

	# Put a legend to the right of the current axis
	# plt.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	fig.legend((legendHandles[i] for i in xrange(len(legendHandles))),
				# (legendHandles[i].get_label() for i in xrange(len(legendHandles))), 
				('MAF','ChiSq','ValueRet','SE Comp\n(ChiSq)', 'SE Comp\n(Others)'),
				loc='right',
				ncol=1)
	# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))
	# add label
	axes[1,0].set_xlabel('#(input records)', fontdict={'fontsize':8})
	axes[1,1].set_xlabel('#(filtered records)', fontdict={'fontsize':8})
	axes[0,0].set_ylabel('Comm (MB)', fontdict={'fontsize':8})
	axes[1,0].set_ylabel('Time (sec)', fontdict={'fontsize':8})
	
	showfigure(fig, 'SingleDO')
	return figid

def plotAddHomVsMAF(figid):
	curMarkerSize=4
	fig,axes = plt.subplots(2,2,num=figid)
	plt.subplots_adjust(right=0.8,hspace=0.3)
	figid+=1
	curRowSubplot = 0
	curColSubplot = 0
	legendHandles = []
	plotLabels = []
	convertToMB = lambda x : x/(1.0*(1<<10)) if not(ifTime) else x
	plotColors = [['r','g','c'],['b','k','m']]
	dataFile = open('data_AddHomVsMAF.txt', 'w')
	for isSetupPhase in [True, False]:
		if isSetupPhase : curColSubplot = 0
		else: curColSubplot = 1
		for ifTime in [True, False]:
			dataFile.write('IsSetupPhase = ' + str(isSetupPhase) + ', IsTime = ' + str(ifTime) + ' ::::::::::\n\n')

			if ifTime: curRowSubplot = 1
			else: curRowSubplot = 0

			if isSetupPhase: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
			else: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))

			tempLegend = []
			tempPlotLabels = []
			for innerLoopVar in xrange(2):
				# if curRowSubplot == 0 and curColSubplot == 1:
				# 	if innerLoopVar == 2 or innerLoopVar == 1:
				# 		continue
				singleDO = None
				isProtocolOPRFOne = None
				if (innerLoopVar == 0):
					singleDO = True
					isProtocolOPRFOne = False
				elif (innerLoopVar == 1):
					singleDO = False
					isProtocolOPRFOne= True
				else:
					singleDO = False
					isProtocolOPRFOne = False
				xAxisData = None
				yAxisData = [None,None] #SE, QE

				for curAppArea in applications:
					if isSetupPhase: xAxisData = fileSizes[curAppArea]
					else: xAxisData = getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO, isProtocolOPRFOne)
					
					for isThisQE in [True]:					
						if isSetupPhase:
							if singleDO: yAxisData[isThisQE] = getDataForSingleDOSetupPhase(not(ifTime), isThisQE, curAppArea)
							else: yAxisData[isThisQE] = getDataForMultiDOSetupPhase(not(ifTime), isThisQE, curAppArea, isProtocolOPRFOne)
						else: yAxisData[isThisQE] = getDataForQueryPhase(not(ifTime), isThisQE, curAppArea, singleDO, isProtocolOPRFOne)

						dataFile.write('AppArea = ' + curAppArea + ', SingleDO = ' + str(singleDO) + ', isOPRF = ' + str(isProtocolOPRFOne) + ', isThisQE = ' + str(isThisQE))
						dataFile.write('\n')
						dataFile.write('xAxisData = ' + (','.join(map(str, xAxisData))))
						dataFile.write('\n')
						dataFile.write('yAxisData (MB/sec) = ' + (','.join(map(str, yAxisData[isThisQE]))))
						dataFile.write('\n')

						handle, = axes[curRowSubplot,curColSubplot].plot(xAxisData,
							map(convertToMB, yAxisData[isThisQE]), 
							# plotColors[0 if curAppArea=="MAF" else 1][innerLoopVar]+('o-' if isThisQE else '^--'), 
							plotColors[0 if curAppArea=="MAF" else 1][innerLoopVar]+('o-' if curAppArea=="MAF" else '^--'), 
							alpha=(0.7 if isThisQE else 1), 
							markersize=(curMarkerSize+2 if isThisQE else curMarkerSize-1), 
							markeredgecolor='k', 
							markerfacecolor=('w' if curAppArea=="MAF" else 'k'),
							label= str(innerLoopVar) + '_' + (curAppArea) + '_' + ('QE' if isThisQE else 'SE')
							)
						# legendHandles.append(handle)
						# plotLabels.append(curAppArea)
						tempLegend.append(handle)
						tempPlotLabels.append(('s' if innerLoopVar==0 else '') + ('FED' if isThisQE else 'SED')
											+ ('' if innerLoopVar==0 else ('\nOPRF' if innerLoopVar==1 else '\nSFE'))
											+ ('\nAddSS' if curAppArea=='MAF' else '\nAddHom')
											)
						# tempPlotLabels.append(('SingleDO' if innerLoopVar==0 else ('MultiDO\nOPRF' if innerLoopVar==1 else 'MultiDO\nEqCheck')) 
						# 					+ '\n' 
						# 					+ (curAppArea) 
						# 					+ ('' if isThisQE else '\nSE'))
			
			axes[curRowSubplot, curColSubplot].set_title(('Setup' if isSetupPhase else 'Query') + ' ' + ('Gross Computation Time' if ifTime else 'Communication Size'),
														 fontdict={'fontsize':9,'weight':'bold','fontname':'sans-serif'})
			axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)
			# axes[curRowSubplot, curColSubplot].legend(tempLegend, loc='upper left')
			legendHandles = tempLegend
			plotLabels = tempPlotLabels

			dataFile.write('\n*********************\n')

	fig.legend((legendHandles[i] for i in xrange(len(legendHandles))),
				# (legendHandles[i].get_label() for i in xrange(len(legendHandles))), 
				(plotLabels[i] for i in xrange(len(plotLabels))),
				loc='right',
				ncol=1)
	# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))
	
	axes[1,0].set_xlabel('#(input records)', fontdict={'fontsize':8})
	axes[1,1].set_xlabel('#(filtered records)', fontdict={'fontsize':8})
	axes[0,0].set_ylabel('Comm (MB)', fontdict={'fontsize':8})
	axes[1,0].set_ylabel('Time (sec)', fontdict={'fontsize':8})

	dataFile.close()

	showfigure(fig,'AddHomVsMAF')
	return figid

def plotMultiDO(figid, isProtocolOPRFOne):
	# first plot query
	# plt.figure(figid)
	singleDO = False
	# figid+=1
	curMarkerSize=4
	fig,axes = plt.subplots(2,2,num=figid)
	plt.subplots_adjust(right=0.8,hspace=0.3)
	figid+=1
	ifTimeArr = [False, True]
	curRowSubplot = 0
	curColSubplot = 1
	legendHandles = []
	convertToMB = lambda x : x/(1.0*(1<<10)) if not(ifTime) else x
	for ifTime in ifTimeArr:
		if ifTime:
			curRowSubplot = 1
		else:
			curRowSubplot = 0

		axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))
		for curAppArea in applications:
			if (curAppArea == "Hamming"): continue ##TODO : Not plotting hamming on this right now
			totalQEData = getDataForQueryPhase(not(ifTime), True, curAppArea, singleDO, isProtocolOPRFOne)
			# if (curAppArea == "ChiSq"):
			# 	print "$$$$$$$$$$", map(convertToMB, totalQEData), getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO)
			handle, = axes[curRowSubplot,curColSubplot].plot(getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO,isProtocolOPRFOne),
				map(convertToMB, totalQEData), 
				applicationColors[curAppArea]+"o-", 
				label=curAppArea, 
				alpha=0.7, 
				markersize=curMarkerSize+1, 
				markeredgecolor='k', 
				markerfacecolor='w'
				)
			legendHandles.append(handle)
		
		handle, = axes[curRowSubplot,curColSubplot].plot(getFilteredOrLeastFreqKwDocsSize("ChiSq", True, singleDO,isProtocolOPRFOne), 
			map(convertToMB, getDataForQueryPhase(not(ifTime), False, "ChiSq", singleDO,isProtocolOPRFOne)), 
			applicationColors["ChiSq"]+"^--", 
			label='SE comp (ChiSq)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)
		# print "########", getFilteredOrLeastFreqKwDocsSize("ChiSq", True, singleDO), map(convertToMB, getDataForQueryPhase(not(ifTime), False, "ChiSq", singleDO))
		legendHandles.append(handle)

		handle, = axes[curRowSubplot,curColSubplot].plot(getFilteredOrLeastFreqKwDocsSize("MAF", True, singleDO,isProtocolOPRFOne), 
			map(convertToMB, getDataForQueryPhase(not(ifTime), False, "MAF", singleDO,isProtocolOPRFOne)), 
			applicationColors["MAF"]+"^--", 
			label='SE comp (Others)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)
		legendHandles.append(handle)
		# plt.legend(handles=plotHandles, loc='upper left')

		axes[curRowSubplot, curColSubplot].set_title('Query Gross Computation Time' if ifTime else 'Query Communication Size',
													 fontdict={'fontsize':9,'weight':'bold', 'fontstyle':'italic'})
		axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)

	#then plot setup
	# plt.figure(figid)
	# figid += 1
	curColSubplot = 0
	for ifTime in ifTimeArr:
		if ifTime:
			curRowSubplot = 1
		else:
			curRowSubplot = 0

		# For every axis, set the x and y major locator
		axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
		# legendHandles = []
		for curAppArea in applications:
			handle,=axes[curRowSubplot,curColSubplot].plot(fileSizes[curAppArea], 
				map(convertToMB, getDataForMultiDOSetupPhase(not(ifTime), True, curAppArea, isProtocolOPRFOne)), 
				applicationColors[curAppArea]+"o-", 
				label=curAppArea, 
				alpha=0.7, 
				markersize=curMarkerSize+1, 
				markeredgecolor='k', 
				markerfacecolor='w'
				)
			# legendHandles.append(handle)

		handle,=axes[curRowSubplot,curColSubplot].plot(fileSizes[curAppArea], 
			map(convertToMB, getDataForMultiDOSetupPhase(not(ifTime), False, "ChiSq", isProtocolOPRFOne)), 
			applicationColors["ChiSq"]+"^--", 
			label='SE comp (ChiSq)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)
		# legendHandles.append(handle)
		handle,=axes[curRowSubplot,curColSubplot].plot(fileSizes[curAppArea], 
			map(convertToMB, getDataForMultiDOSetupPhase(not(ifTime), False, "MAF", isProtocolOPRFOne)), 
			applicationColors["MAF"]+"^--", 
			label='SE comp (Others)',
			markersize=curMarkerSize, 
			markeredgecolor='k', 
			markerfacecolor='k'
			)

		# legendHandles.append(handle)
		# plt.legend(handles=legendHandles, loc='upper left')
		axes[curRowSubplot, curColSubplot].set_title('Setup Gross Computation Time' if ifTime else 'Setup Communication Size',
													 fontdict={'fontsize':9,'weight':'bold', 'fontstyle':'italic'})
		axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)
	
	# Shrink current axis by 20%
	# box = plt.get_position()
	# plt.set_position([box.x0, box.y0, box.width * 0.8, box.height])

	# Put a legend to the right of the current axis
	# plt.legend(loc='center left', bbox_to_anchor=(1, 0.5))
	fig.legend((legendHandles[i] for i in xrange(len(legendHandles))),
				# (legendHandles[i].get_label() for i in xrange(len(legendHandles))), 
				('MAF','ChiSq','ValueRet','SE Comp\n(ChiSq)', 'SE Comp\n(Others)'),
				loc='right',
				ncol=1)
	# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))

	axes[1,0].set_xlabel('#(input records)', fontdict={'fontsize':8})
	axes[1,1].set_xlabel('#(filtered records)', fontdict={'fontsize':8})
	axes[0,0].set_ylabel('Comm (MB)', fontdict={'fontsize':8})
	axes[1,0].set_ylabel('Time (sec)', fontdict={'fontsize':8})

	showfigure(fig,'MultiDO_'+('OPRF' if isProtocolOPRFOne else 'EqCheck'))
	return figid

def plotHamming(figid):
	# first plot query
	singleDO = True
	curMarkerSize=4
	fig,axes = plt.subplots(2,2,num=figid)
	plt.subplots_adjust(right=0.8,hspace=0.3)
	figid+=1
	curRowSubplot = 0
	curColSubplot = 1
	legendHandles = []
	plotLabels = []
	convertToGB = lambda x : x/(1.0*(1<<20)) if not(ifTime) else x
	curAppArea = "Hamming"
	plotColors = ['r','g','b']
	for isSetupPhase in [True, False]:
		if isSetupPhase: curColSubplot = 0
		else: curColSubplot = 1
		for ifTime in [True,False]:
			if ifTime:
				curRowSubplot = 1
			else:
				curRowSubplot = 0

			if isSetupPhase: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
			else: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))
			for innerLoopVar in xrange(3):
				singleDO = None
				isProtocolOPRFOne = None
				if (innerLoopVar == 0):
					singleDO = True
					isProtocolOPRFOne = False
				elif (innerLoopVar == 1):
					singleDO = False
					isProtocolOPRFOne= True
				else:
					singleDO = False
					isProtocolOPRFOne = False
				xAxisData = None
				yAxisData = None
				
				if isSetupPhase: xAxisData = fileSizes[curAppArea]
				else: xAxisData = getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO, isProtocolOPRFOne)

				if isSetupPhase: 
					if singleDO: yAxisData = getDataForSingleDOSetupPhase(not(ifTime), True, curAppArea)
					else: yAxisData = getDataForMultiDOSetupPhase(not(ifTime), True, curAppArea, isProtocolOPRFOne)
				else: yAxisData = getDataForQueryPhase(not(ifTime), True, curAppArea, singleDO, isProtocolOPRFOne)
				print isSetupPhase, ifTime, singleDO, xAxisData, yAxisData
				handle, = axes[curRowSubplot,curColSubplot].plot(xAxisData,
					map(convertToGB, yAxisData), 
					plotColors[innerLoopVar]+"o-", 
					alpha=0.7, 
					markersize=curMarkerSize+1, 
					markeredgecolor='k', 
					markerfacecolor='w'
					)
				legendHandles.append(handle)
				plotLabels.append('SingleDO' if singleDO else ('MultiDO\nOPRF' if isProtocolOPRFOne else 'MultiDO\nEqCheck'))

				if isSetupPhase: 
					if singleDO: yAxisData = getDataForSingleDOSetupPhase(not(ifTime), False, curAppArea)
					else: yAxisData = getDataForMultiDOSetupPhase(not(ifTime), False, curAppArea, isProtocolOPRFOne)
				else: yAxisData = getDataForQueryPhase(not(ifTime), False, curAppArea, singleDO, isProtocolOPRFOne)
				print isSetupPhase, ifTime, singleDO, xAxisData, yAxisData
				handle, = axes[curRowSubplot,curColSubplot].plot(xAxisData, 
					map(convertToGB, yAxisData), 
					plotColors[innerLoopVar]+"^"+('--' if singleDO else (':' if isProtocolOPRFOne else '-.')), 
					markersize=curMarkerSize, 
					markeredgecolor='k', 
					markerfacecolor='k'
					)
				legendHandles.append(handle)
				plotLabels.append('SingleDO\nSE' if singleDO else ('MultiDO\nOPRF\nSE' if isProtocolOPRFOne else 'MultiDO\nEqCheck\nSE'))

			axes[curRowSubplot, curColSubplot].set_title(('Setup' if isSetupPhase else 'Query') + ' ' + ('Gross Computation Time' if ifTime else 'Communication Size'),
														 fontdict={'fontsize':9,'weight':'bold', 'fontstyle':'italic'})
			axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)
	
	fig.legend((legendHandles[i] for i in xrange(6)),
				(plotLabels[i] for i in xrange(6)),
				loc='right',
				ncol=1)
	# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))

	axes[1,0].set_xlabel('#(input records)', fontdict={'fontsize':8})
	axes[1,1].set_xlabel('#(filtered records)', fontdict={'fontsize':8})
	axes[0,0].set_ylabel('Comm (GB)', fontdict={'fontsize':8})
	axes[1,0].set_ylabel('Time (sec)', fontdict={'fontsize':8})

	showfigure(fig, 'Hamming')
	return figid

def plotSingleDOvsMultiDO(figid):
	assert(len(applications) == 1)
	curMarkerSize=6
	fig,axes = plt.subplots(2,2,num=figid)
	plt.subplots_adjust(right=0.84,hspace=0.3)
	figid+=1
	curRowSubplot = 0
	curColSubplot = 0
	legendHandles = []
	plotLabels = []
	convertToMB = lambda x : x/(1.0*(1<<10)) if not(ifTime) else x
	plotColors = [['g','r','b'],['c','k','m']]
	dataFile = open('data_'+applications[0]+'.txt', 'w')
	for isSetupPhase in [True, False]:
		if isSetupPhase : curColSubplot = 0
		else: curColSubplot = 1
		for ifTime in [True, False]:
			dataFile.write('IsSetupPhase = ' + str(isSetupPhase) + ', IsTime = ' + str(ifTime) + ' ::::::::::\n\n')

			if ifTime: curRowSubplot = 1
			else: curRowSubplot = 0

			if isSetupPhase: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
			else: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))

			tempLegend = []
			tempPlotLabels = []
			for innerLoopVar in xrange(3):
				# if curRowSubplot == 0 and curColSubplot == 1:
				# 	if innerLoopVar == 2 or innerLoopVar == 1:
				# 		continue
				singleDO = None
				isProtocolOPRFOne = None
				if (innerLoopVar == 0):
					singleDO = True
					isProtocolOPRFOne = False
				elif (innerLoopVar == 1):
					singleDO = False
					isProtocolOPRFOne= True
				else:
					singleDO = False
					isProtocolOPRFOne = False
				xAxisData = None
				yAxisData = [None,None] #SE, QE

				for curAppArea in applications:
					if isSetupPhase: xAxisData = fileSizes[curAppArea]
					else: xAxisData = getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO, isProtocolOPRFOne)
					
					for isThisQE in [True,False]:
						if isSetupPhase:
							if singleDO: yAxisData[isThisQE] = getDataForSingleDOSetupPhase(not(ifTime), isThisQE, curAppArea)
							else: yAxisData[isThisQE] = getDataForMultiDOSetupPhase(not(ifTime), isThisQE, curAppArea, isProtocolOPRFOne)
						else: yAxisData[isThisQE] = getDataForQueryPhase(not(ifTime), isThisQE, curAppArea, singleDO, isProtocolOPRFOne)

						if not(isSetupPhase) and isThisQE:
							print "**********************", ifTime, innerLoopVar, yAxisData[isThisQE]

						dataFile.write('SingleDO = ' + str(singleDO) + ', isOPRF = ' + str(isProtocolOPRFOne) + ', isThisQE = ' + str(isThisQE))
						dataFile.write('\n')
						dataFile.write('xAxisData = ' + (','.join(map(str, xAxisData))))
						dataFile.write('\n')
						dataFile.write('yAxisData (MB/sec) = ' + (','.join(map(str, yAxisData[isThisQE]))))
						dataFile.write('\n')

						handle, = axes[curRowSubplot,curColSubplot].plot(xAxisData,
							map(convertToMB, yAxisData[isThisQE]), 
							plotColors[0 if curAppArea==applications[0] else 1][innerLoopVar]
							+('o' if innerLoopVar==0 else ('^' if innerLoopVar==1 else 'v'))
							# +('-' if isThisQE else '--'), 
							+('-' if isThisQE else ('-.' if innerLoopVar==1 else (':' if innerLoopVar==2 else '--'))), 
							# plotColors[0 if curAppArea=="MAF" else 1][innerLoopVar]+('o-' if curAppArea=="MAF" else '^--'), 
							alpha=(0.7 if isThisQE else 1), 
							# markersize=(curMarkerSize+2 if isThisQE else curMarkerSize-1), 
							markersize=(curMarkerSize+2 if innerLoopVar==1 else (curMarkerSize if innerLoopVar==2 else curMarkerSize-2)), 
							markeredgecolor='k', 
							markerfacecolor=('w'),
							# markerfacecolor=('w' if curAppArea==applications[0] else 'k'),
							# markerfacecolor=plotColors[0 if curAppArea==applications[0] else 1][innerLoopVar],
							label= str(innerLoopVar) + '_' + (curAppArea) + '_' + ('QE' if isThisQE else 'SE')
							)
						# legendHandles.append(handle)
						# plotLabels.append(curAppArea)
						tempLegend.append(handle)
						tempPlotLabels.append(('s' if innerLoopVar==0 else '') + ('FED' if isThisQE else 'SED') 
											+ ('' if innerLoopVar==0 else ('\nOPRF' if innerLoopVar==1 else '\nSFE'))
											)

						# tempPlotLabels.append(('SingleDO' if innerLoopVar==0 else ('MultiDO\nOPRF' if innerLoopVar==1 else 'MultiDO\nEqCheck'))
						# 					+ ('' if (len(applications)==1) else ('\n'+curAppArea))
						# 					+ ('' if isThisQE else '\nSE'))
			
			axes[curRowSubplot, curColSubplot].set_title(('Init' if isSetupPhase else 'Query') + ' ' + ('Gross Computation Time' if ifTime else 'Communication Size'),
														 fontdict={'fontsize':9,'weight':'bold','fontname':'sans-serif'})
			axes[curRowSubplot, curColSubplot].tick_params(labelsize=7)
			# axes[curRowSubplot, curColSubplot].legend(tempLegend, loc='upper left')
			legendHandles = tempLegend
			plotLabels = tempPlotLabels

			dataFile.write('\n*********************\n')

	fig.legend((legendHandles[i] for i in xrange(len(legendHandles))),
				# (legendHandles[i].get_label() for i in xrange(len(legendHandles))), 
				(plotLabels[i] for i in xrange(len(plotLabels))),
				loc='right',
				ncol=1)
	# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))
	
	axes[1,0].set_xlabel('#(input records)', fontdict={'fontsize':8})
	axes[1,1].set_xlabel('#(filtered records)', fontdict={'fontsize':8})
	axes[0,0].set_ylabel('Comm (MB)', fontdict={'fontsize':8})
	axes[1,0].set_ylabel('Time (sec)', fontdict={'fontsize':8})

	dataFile.close()
	showfigure(fig, 'SingleDoVsMultiDo'+('_'+applications[0] if len(applications)==1 else ''))
	return figid


#### New Plot functions for USENIX submission
def plotNewTotalTimeSingleDOVsMultiDO(figid):
	curMarkerSize=6
	curApplications = applications[:]
	curApplicationsPlotStrings = applicationsPlotStrings[:]
	# fig,axes = plt.subplots(2, len(curApplications),num=figid,figsize=(7,7))
	# plt.subplots_adjust(hspace=0.3,wspace=0.15)
	
	fig,axes = plt.subplots(2, len(applications),num=figid,figsize=(15,7))
	plt.subplots_adjust(hspace=0.4,top=0.8)

	figid+=1
	curRowSubplot = 0
	curColSubplot = 0
	legendHandles = []
	plotLabels = []
	convertToMB = lambda x : x/(1.0*(1<<10)) if not(ifTime) else x
	# plotColors = [['g','r','b'],['c','k','m']]
	plotColors = ['g','r','b']
	dataFile = open('./data_time.txt', 'w')
	for ii, curAppArea in enumerate(curApplications):
		dataFile.write('curAppArea = {0}------>>>>>>\n\n'.format(curAppArea))
		curColSubplot = ii
		for isSetupPhase in [True, False]:
			dataFile.write('IsSetupPhase = ' + str(isSetupPhase) + ' ::::::::::\n\n')
			if isSetupPhase: curRowSubplot = 0
			else: curRowSubplot = 1
			for ifTime in [True]:

				if isSetupPhase: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
				else: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))

				tempLegend = []
				tempPlotLabels = []
				for innerLoopVar in xrange(3):
					singleDO = None
					isProtocolOPRFOne = None
					if (innerLoopVar == 0):
						singleDO = True
						isProtocolOPRFOne = False
					elif (innerLoopVar == 1):
						singleDO = False
						isProtocolOPRFOne= True
					else:
						singleDO = False
						isProtocolOPRFOne = False
					xAxisData = None
					yAxisData = [None,None] #SE, QE

					if isSetupPhase: xAxisData = fileSizes[curAppArea]
					else: xAxisData = getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO, isProtocolOPRFOne)
					
					for isThisQE in [True,False]:
						if isSetupPhase: yAxisData[isThisQE] = getNewSetupPhaseData_TotalTime(curAppArea, not(isThisQE), singleDO, isProtocolOPRFOne)
						else: yAxisData[isThisQE] = getNewQueryPhaseData_TotalTime(curAppArea, not(isThisQE), singleDO, isProtocolOPRFOne)

						# if not(isSetupPhase) and isThisQE:
						# 	print "**********************", ifTime, innerLoopVar, yAxisData[isThisQE]

						dataFile.write('SingleDO = ' + str(singleDO) + ', isOPRF = ' + str(isProtocolOPRFOne) + ', isThisQE = ' + str(isThisQE))
						dataFile.write('\n')
						dataFile.write('xAxisData = ' + (','.join(map(str, xAxisData))))
						dataFile.write('\n')
						dataFile.write('yAxisData (sec) = ' + (','.join(map(str, yAxisData[isThisQE]))))
						dataFile.write('\n')

						handle, = axes[curRowSubplot, curColSubplot].plot(xAxisData,
							map(convertToMB, yAxisData[isThisQE]), 
							plotColors[innerLoopVar]
							+('o' if innerLoopVar==0 else ('^' if innerLoopVar==1 else 'v'))
							+('-' if isThisQE else '--'), 
							# +('-' if isThisQE else ('-.' if innerLoopVar==1 else (':' if innerLoopVar==2 else '--'))), 
							# plotColors[0 if curAppArea=="MAF" else 1][innerLoopVar]+('o-' if curAppArea=="MAF" else '^--'), 
							alpha=(0.7 if isThisQE else 1), 
							# markersize=(curMarkerSize+2 if isThisQE else curMarkerSize-1), 
							markersize=(curMarkerSize+2 if innerLoopVar==1 else (curMarkerSize if innerLoopVar==2 else curMarkerSize-2)), 
							markeredgecolor='k', 
							markerfacecolor=('w'),
							# markerfacecolor=('w' if curAppArea==curApplications[0] else 'k'),
							# markerfacecolor=plotColors[0 if curAppArea==curApplications[0] else 1][innerLoopVar],
							label= str(innerLoopVar) + '_' + (curAppArea) + '_' + ('QE' if isThisQE else 'SE')
							)
						# legendHandles.append(handle)
						# plotLabels.append(curAppArea)
						tempLegend.append(handle)
						tempPlotLabels.append(('s' if innerLoopVar==0 else '') + ('FED' if isThisQE else 'SED') 
											+ ('' if innerLoopVar==0 else ('\nOPRF' if innerLoopVar==1 else '\nSFE'))
											)
				
				axes[curRowSubplot, curColSubplot].set_title(curApplicationsPlotStrings[ii] + ' ' + ('Init' if isSetupPhase else 'Query') + ' Time',
															 fontdict={'fontsize':13,'weight':'bold','fontname':'sans-serif'})
				axes[curRowSubplot, curColSubplot].tick_params(labelsize=10)
				if (len(legendHandles)==0 and len(plotLabels)==0):
					legendHandles = tempLegend
					plotLabels = tempPlotLabels

				dataFile.write('\n*********************\n')

		fig.legend((legendHandles[i] for i in xrange(len(legendHandles))),
					(plotLabels[i] for i in xrange(len(plotLabels))),
					loc='upper center',
					ncol=6,
					prop={'size':12}
					,markerscale=1.2
					# ,mode="expand"
					,columnspacing=4.0
					,borderaxespad=2.0
					# ,frameon=False
					)
		# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))
	
	for jj in xrange(len(curApplications)):
		axes[0,jj].set_xlabel('#(input records)', fontdict={'fontsize':12})
		axes[1,jj].set_xlabel('#(filtered records)', fontdict={'fontsize':12})
	axes[0,0].set_ylabel('Time (sec)', fontdict={'fontsize':12})
	axes[1,0].set_ylabel('Time (sec)', fontdict={'fontsize':12})

	dataFile.close()
	showfigure(fig, 'TotalTimeSingleDOVsMultiDO')
	return figid

def plotNewCommSingleDOVsMultiDO(figid):
	curMarkerSize=6
	curApplications = applications[:]
	curApplicationsPlotStrings = applicationsPlotStrings[:]
	# fig,axes = plt.subplots(2, len(curApplications),num=figid,figsize=(7,7))
	# plt.subplots_adjust(hspace=0.3,wspace=0.15)
	
	fig,axes = plt.subplots(2, len(applications),num=figid,figsize=(15,7))
	plt.subplots_adjust(hspace=0.4,left=0.05,top=0.82)

	figid+=1
	curRowSubplot = 0
	curColSubplot = 0
	legendHandles = []
	plotLabels = []
	convertToMB = lambda x : x/(1.0*(1<<10)) if not(ifTime) else x
	# plotColors = [['g','r','b'],['c','k','m']]
	plotColors = ['g','r','b']
	# dataFile = open('data_'+applications[0]+'.txt', 'w')
	for ii,curAppArea in enumerate(curApplications):
		curColSubplot = ii
		for isSetupPhase in [True, False]:
			if isSetupPhase: curRowSubplot = 0
			else: curRowSubplot = 1
			for ifTime in [False]:
				# dataFile.write('IsSetupPhase = ' + str(isSetupPhase) + ', IsTime = ' + str(ifTime) + ' ::::::::::\n\n')

				if isSetupPhase: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MultipleLocator(2))
				else: axes[curRowSubplot,curColSubplot].xaxis.set_major_locator(ticker.MaxNLocator(4))

				tempLegend = []
				tempPlotLabels = []
				for innerLoopVar in xrange(3):
					singleDO = None
					isProtocolOPRFOne = None
					if (innerLoopVar == 0):
						singleDO = True
						isProtocolOPRFOne = False
					elif (innerLoopVar == 1):
						singleDO = False
						isProtocolOPRFOne= True
					else:
						singleDO = False
						isProtocolOPRFOne = False
					xAxisData = None
					yAxisData = [None,None] #SE, QE

					if isSetupPhase: xAxisData = fileSizes[curAppArea]
					else: xAxisData = getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO, isProtocolOPRFOne)
					
					for isThisQE in [True,False]:
						if isSetupPhase:
							if singleDO: yAxisData[isThisQE] = getDataForSingleDOSetupPhase(not(ifTime), isThisQE, curAppArea)
							else: yAxisData[isThisQE] = getDataForMultiDOSetupPhase(not(ifTime), isThisQE, curAppArea, isProtocolOPRFOne)
						else: yAxisData[isThisQE] = getDataForQueryPhase(not(ifTime), isThisQE, curAppArea, singleDO, isProtocolOPRFOne)
						# if not(isSetupPhase) and isThisQE:
						# 	print "**********************", ifTime, innerLoopVar, yAxisData[isThisQE]

						# dataFile.write('SingleDO = ' + str(singleDO) + ', isOPRF = ' + str(isProtocolOPRFOne) + ', isThisQE = ' + str(isThisQE))
						# dataFile.write('\n')
						# dataFile.write('xAxisData = ' + (','.join(map(str, xAxisData))))
						# dataFile.write('\n')
						# dataFile.write('yAxisData (MB/sec) = ' + (','.join(map(str, yAxisData[isThisQE]))))
						# dataFile.write('\n')

						handle, = axes[curRowSubplot, curColSubplot].plot(xAxisData,
							map(convertToMB, yAxisData[isThisQE]), 
							plotColors[innerLoopVar]
							+('o' if innerLoopVar==0 else ('^' if innerLoopVar==1 else 'v'))
							+('-' if isThisQE else '--'), 
							# +('-' if isThisQE else ('-.' if innerLoopVar==1 else (':' if innerLoopVar==2 else '--'))), 
							# plotColors[0 if curAppArea=="MAF" else 1][innerLoopVar]+('o-' if curAppArea=="MAF" else '^--'), 
							alpha=(0.7 if isThisQE else 1), 
							# markersize=(curMarkerSize+2 if isThisQE else curMarkerSize-1), 
							markersize=(curMarkerSize+2 if innerLoopVar==1 else (curMarkerSize if innerLoopVar==2 else curMarkerSize-2)), 
							markeredgecolor='k', 
							markerfacecolor=('w'),
							# markerfacecolor=('w' if curAppArea==curApplications[0] else 'k'),
							# markerfacecolor=plotColors[0 if curAppArea==curApplications[0] else 1][innerLoopVar],
							label= str(innerLoopVar) + '_' + (curAppArea) + '_' + ('QE' if isThisQE else 'SE')
							)
						# legendHandles.append(handle)
						# plotLabels.append(curAppArea)
						tempLegend.append(handle)
						tempPlotLabels.append(('s' if innerLoopVar==0 else '') + ('FED' if isThisQE else 'SED') 
											+ ('' if innerLoopVar==0 else ('\nOPRF' if innerLoopVar==1 else '\nSFE'))
											)
				
				axes[curRowSubplot, curColSubplot].set_title(curApplicationsPlotStrings[ii] + ' ' + ('Init' if isSetupPhase else 'Query') + ' Comm. Size',
															 fontdict={'fontsize':11,'weight':'bold','fontname':'sans-serif'})
				axes[curRowSubplot, curColSubplot].tick_params(labelsize=10)
				if (len(legendHandles)==0 and len(plotLabels)==0):
					legendHandles = tempLegend
					plotLabels = tempPlotLabels

				# dataFile.write('\n*********************\n')

		fig.legend((legendHandles[i] for i in xrange(len(legendHandles))),
					(plotLabels[i] for i in xrange(len(plotLabels))),
					loc='upper center',
					ncol=6,
					prop={'size':11}
					,markerscale=1.2
					# ,mode="expand"
					,columnspacing=4.0
					,borderaxespad=2.0
					# ,frameon=False
					)
		# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))
	
	for jj in xrange(len(curApplications)):
		axes[0,jj].set_xlabel('#(input records)', fontdict={'fontsize':11})
		axes[1,jj].set_xlabel('#(filtered records)', fontdict={'fontsize':11})
	axes[0,0].set_ylabel('Communication (MB)', fontdict={'fontsize':11})
	axes[1,0].set_ylabel('Communication (MB)', fontdict={'fontsize':11})

	# dataFile.close()
	showfigure(fig, 'TotalCommSingleDOVsMultiDO')
	return figid

def plotNewComparisonPrevWorks(figid):
	fig, axes = plt.subplots(1, num=figid, figsize=(5,2))
	figid+=1
	plt.subplots_adjust(left=0.18)
	curFileSizeMAFIdx = 0
	curFileSizeHammingIdx = 3
	SEData = []
	QEData = []
	xAxisData = []
	yAxisTicks = []
	for ii, curAppArea in enumerate(applications):
		if curAppArea == "Hamming":
			idx = curFileSizeHammingIdx
			# continue
		else:
			idx = curFileSizeMAFIdx
		SEData.append(getNewQueryPhaseData_TotalTime(curAppArea, True, True, False)[idx])
		QEData.append(getNewQueryPhaseData_TotalTime(curAppArea, False, True, False)[idx])
		xAxisData.append((len(applications)-ii)/2.0)
		if curAppArea == "Hamming":
			yAxisTicks.append("Av.\nHamming")
			print SEData, "^^^^^^^^^^^^^^6"
		elif curAppArea == "ValueRet":
			yAxisTicks.append("Genome\nRetrieval")
		else:
			yAxisTicks.append(applicationsPlotStrings[ii])
	
	width = 0.3
	# p1 = axes.bar(xAxisData, SEData, width, color = 'k', edgecolor='k')
	# p2 = axes.bar(xAxisData, QEData, width, bottom=SEData, color='w', edgecolor='k')
	# # print SEData
	# # print QEData
	# axes.set_ylabel('Total time (in sec)')
	# axes.set_title('sFED vs sSED query time comparison')
	# axes.set_xticks(xAxisData)
	# axes.set_xticklabels(yAxisTicks)
	# axes.legend((p1[0], p2[0]), ('sSED', 'sCED'))
	# showfigure(fig, 'prevWorksComparison')

	p1 = axes.barh(xAxisData, SEData, width, color = 'k', edgecolor='k')
	p2 = axes.barh(xAxisData, QEData, width, left=SEData, color='w', edgecolor='k')
	print '*** ', QEData
	print(SEData, QEData)
	# print SEData
	# print QEData
	axes.set_xlabel('Total time (in sec)')
	axes.set_title('sFED vs sSED query time comparison')
	axes.set_yticks(xAxisData)
	axes.set_yticklabels(yAxisTicks)
	axes.legend((p1[0], p2[0]), ('sSED', 'sCED'))
	showfigure(fig, 'prevWorksComparison')

	return figid

figid = 0

# For SingleDO
# parseDataForSingleDO()
# figid = plotSingleDO(figid)

# For MultiDO
# isProtocolOPRFOne = False
# parseDataForMultiDO(isProtocolOPRFOne)
# figid = plotMultiDO(figid, isProtocolOPRFOne)

# For hamming
# parseDataForSingleDO()
# for isProtocolOPRFOne in [True, False]:
# 	if not(isProtocolOPRFOne): numQueriesPerFile = 1
# 	parseDataForMultiDO(isProtocolOPRFOne)
# figid = plotHamming(figid)

# For AddHom
# parseDataForSingleDO()
# for isProtocolOPRFOne in [True, False]:
# 	parseDataForMultiDO(isProtocolOPRFOne)
# figid = plotAddHomVsMAF(figid)

# For SingleDOvsMultiDO of MAF,ChiSq,ValuRet
# parseDataForSingleDO()
# for isProtocolOPRFOne in [True, False]:
# 	parseDataForMultiDO(isProtocolOPRFOne)
# figid = plotSingleDOvsMultiDO(figid)

parseDataForSingleDO(includeNetworkTime=True)
for isProtocolOPRFOne in [True, False]:
	parseDataForMultiDO(isProtocolOPRFOne, includeNetworkTime=True)
# figid = plotNewTotalTimeSingleDOVsMultiDO(figid)
# figid = plotNewCommSingleDOVsMultiDO(figid)
figid = plotNewComparisonPrevWorks(figid)

# print getDataForQueryPhase(True, True, "ValueRet", True, False)
