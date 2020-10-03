
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
import sys, os, re, operator
import matplotlib.pyplot as plt

if (len(sys.argv) < 2):
	print "Usage : python analyse.py <output folder for saving graphs>"
	exit(-1)

graphSaveFolder = sys.argv[1]
# datafolder = sys.argv[1]
# datafolder = "/mnt/c/Users/t-niskum/OneDrive/Documents/FurtherLife/Projects/Search-compute/FinalExp/correctedResults/SingleDOSorted/"
# datafolder = "/mnt/c/Users/t-niskum/OneDrive/Documents/FurtherLife/Projects/Search-compute/SeptSubmission/AllLogsDump/Final/MultiDO/EqChecks/mDOrelevantlogs/"
datafolder = "/mnt/c/Users/t-niskum/Documents/SrchComp/SeptSubmissionLogs/MultiDO_Eq/logs_mdo_EQcheck_ValMAFChiSq/mDOrelevantlogs"
applications = ["MAF", "ChiSq", "ValueRet"]
# applications = ["MAF", "ChiSq", "ValueRet", "Hamming"]
# applications = ["MAF", "AddHom"]
# applications = ["Hamming"]
# applications = ["ChiSq"]
# applications = ["MAF"]

#Following 3 variables should remain in sync
numdb=5
# fileSizes = ["25000", "50000", "75000", "100000", "125000"]
# fileSizes = ["10000", "20000", "50000", "75000", "100000"]
# fileSizes = ["5000", "10000", "15000", "20000", "25000"]
fileSizes = ["20000", "40000", "60000", "80000", "100000"]
# fileSizes = ["5000"]
# fileSizes = ["50", "100", "200", "300", "400"]
# snpSizes = ["1000", "1000", "1000", "1000", "1000"]
# snpSizes = ["10", "10", "10", "10", "10"]
snpSizes = ["200", "200", "200", "200", "200"]
# snpSizes = ["50", "50", "50", "50", "50"]

entityNameFileSuffices = ["Client", "Server", "AuxServer", "setup"]
# entityNameFileSuffices = ["Client", "Server", "AuxServer"]
entityRegexName = ["Client", "Server", "Authority", "setup"] #BEWARE : Don't change the order of entities in this list or bad things will happen 
# entityRegexName = ["Client", "Server", "Authority"] #BEWARE : Don't change the order of entities in this list or bad things will happen 

numQueriesPerFile=1
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
right now since only QE data is comming, se data = qe data and dcfe data = 0 --> Change this tomorrow
'''
goddata = {}
goddataForMultiDO = {}

def mapTime(x):
	#return no. of sec
	return x/(1.0*1000000)

def mapSize(x):
	#return no. of KB
	return x/((1<<10))

# Single DO parsing functions

def parseQueryFileForMAFOrValueRetOrAddHom(filename, entity, appArea):
	datafound = [] #has similar structere as clientd in above comment
	setimepattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	dcfetimepattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	indvTimeIgnorePattern = '\[' + entity + ' (SE|DCFE) Time\]: Total time = \d+, .*'
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

def parseQueryFileForChiSq(filename, entity, appArea):
	datafound = [] #has similar structere as clientd in above comment
	setimepattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	dcfetimepattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	OblivCGateCtPattern = '\[Final OblivC\] : Gate count : (\d+)'
	OblivCBytesSentPattern = '\[Final OblivC\] : Bytes sent = (\d+)'
	indvTimeIgnorePattern = '\[' + entity + ' (SE|DCFE) Time\]: Total time = \d+, .*'
	leastFreqKwDocsSizePattern = '\[Final Data\]: T.size\(\) is (\d+)'
	filteredDocsSizePattern = '\[Final Data\]: Size of documents retrieved is (\d+)'
	curtuple = [0,0,0,0,0,0,0]
	ct = 0
	numLinesPerQuery = 0
	otherPartyCommToAddTotal = 0
	if (entity == entityRegexName[0]):# for client
		numLinesPerQuery = 18
	elif (entity == entityRegexName[1]):
		numLinesPerQuery = 22
	else:
		numLinesPerQuery = 16
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

def parseQueryFileForHamming(filename, entity, appArea):
	datafound = [] #has similar structere as clientd in above comment
	setimepattern = '\[' + entity + ' SE Time\] ?: Actual time = (\d+)'
	secommpattern = '\[' + entity + ' SE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	dcfetimepattern = '\[' + entity + ' DCFE Time\] ?: Actual time = (\d+)'
	dcfecommpattern = '\[' + entity + ' DCFE comm.?\] ?: With \w+, read = (\d+), written = (\d+)'
	indvTimeIgnorePattern = '\[' + entity + ' (SE|DCFE) Time\]: Total time = \d+,.*'

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
			print line
			if (re.search(indvTimeIgnorePattern, line, re.IGNORECASE)):
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
				assert False
			ct+=1
			print curtuple
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

def parseAndAddToGodData(appArea, singleDO, skipSetup, fileNamePrefixPrefix=None):
	global goddata, goddataForMultiDO
	dirpath = datafolder
	for i in xrange(numdb):
		curFileSize = fileSizes[i]
		curSnpSize = snpSizes[i]
		if fileNamePrefixPrefix is not None:
			fileNamePrefix = fileNamePrefixPrefix+"log_" + curFileSize + "_" + curSnpSize + "_" + appArea +"0_"
		else:
			fileNamePrefix = "log_" + curFileSize + "_" + curSnpSize + "_" + appArea +"0_"
		curFileDataPointData = []
		hammAuthReportedCommCostToAddToServer = 0
		hammServerReportedCommCostToAddToAuth = 0
		chiSqClientReportedCommCostToAddToServer = 0
		chiSqServerReportedCommCostToAddToClient = 0
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
				if (appArea == "MAF" or appArea == "ValueRet" or appArea == "AddHom"):
					dataForFile = parseQueryFileForMAFOrValueRetOrAddHom(fileNameFull, entityRegexName[j], appArea)
				elif (appArea == "ChiSq"):
					(dataForFile, avOtherPartyCommToAdd) = parseQueryFileForChiSq(fileNameFull, entityRegexName[j], appArea)
					if (entityRegexName[j] == "Client"):
						chiSqClientReportedCommCostToAddToServer = avOtherPartyCommToAdd
					elif (entityRegexName[j] == "Server"):
						chiSqServerReportedCommCostToAddToClient = avOtherPartyCommToAdd
				elif (appArea == "Hamming"):
					(dataForFile, avCommCostToAddToOtherParty) = parseQueryFileForHamming(fileNameFull, entityRegexName[j], appArea)
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
			print "(AppArea, curFileSize, authReportedCommCostToAddToServer, serverReportedCommCostToAddToAuth) = (", appArea, ",", curFileSize, ",", hammAuthReportedCommCostToAddToServer, ",", hammServerReportedCommCostToAddToAuth, ")"

		if (appArea == "ChiSq"):
			print curFileDataPointData
			curFileDataPointData[0][3] += chiSqServerReportedCommCostToAddToClient #Server reported comm cost addint to client's dcfe comm cost
			curFileDataPointData[1][3] += chiSqClientReportedCommCostToAddToServer #Client reported comm cost adding to server's dcfe comm cost
			print "(AppArea, curFileSize, chiSqClientReportedCommCostToAddToServer, chiSqServerReportedCommCostToAddToClient) = (", appArea, ",", curFileSize, ",", chiSqClientReportedCommCostToAddToServer, ",", chiSqServerReportedCommCostToAddToClient, ")"
		if (singleDO): 
			goddata[(appArea, curFileSize)] = curFileDataPointData
		else:
			#MultiDO
			goddataForMultiDO[(appArea, curFileSize)] = curFileDataPointData

# Plot helper functions mainly for single DO

def getQueryGraphsListFromGodData(appArea, entityRegexNameIdx, ifSE, ifTime, singleDO):
	global goddata, fileSizes
	ans=[]
	assert entityRegexNameIdx<3
	ifSEInt = 0 if ifSE else 1
	ifTimeInt = 0 if ifTime else 1
	for curFileSize in fileSizes:
		if singleDO:
			ans.append(goddata[(appArea, curFileSize)][entityRegexNameIdx][ifSEInt*2+ifTimeInt])
		else:
			ans.append(goddataForMultiDO[(appArea, curFileSize)][entityRegexNameIdx][ifSEInt*2+ifTimeInt])
	return ans

def getFilteredOrLeastFreqKwDocsSize(curAppArea, forFinalFilteredRecords, singleDO):
	global goddata, fileSizes
	idx = 5 if forFinalFilteredRecords else 4
	if singleDO:
		filteredRecordsLi = list(map(lambda curFileSize: goddata[(curAppArea, curFileSize)][1][idx], fileSizes))
	else:
		filteredRecordsLi = list(map(lambda curFileSize: goddataForMultiDO[(curAppArea, curFileSize)][1][idx], fileSizes))
	return filteredRecordsLi

def getSetupGraphsDataListFromGodData(curAppArea, idx):
	global goddata, fileSizes
	datali = []
	for curFileSize in fileSizes:
		datali.append(goddata[(curAppArea, curFileSize)][3][idx])
	return datali

def legendMappingForIdxSetupPhaseGraphs(idx):
	if idx == 0: return "Unencrypted file size in KB"
	elif idx == 1: return "XSet gen time in sec"
	elif idx == 2: return "XSet file size in KB"
	elif idx == 3: return "TSet gen time in sec"
	elif idx == 4: return "TSet file size in KB"
	elif idx == 5: return "DCFE specfic EDB gen time in sec"
	elif idx == 6: return "DCFE specific EDB gen size in KB"
	else: assert(False)

# Single DO plot functions

def plotAllQueryGraphs():
	global applications
	graphsToPlot = ["Comm", "Timing"]
	figid=0
	for curAppArea in applications:
		for curGraphIdx in xrange(2):
			curGraph = graphsToPlot[curGraphIdx]
			for se in xrange(2):
				#se = 0 means dcfe
				plt.figure(figid)
				figid+=1
				legendsli=[]
				seOrDCFEString = "SE" if se else "DCFE"
				for entityRegexNameIdx in xrange(3):
					datali = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, se, curGraphIdx, True)
					print "Data for", curAppArea, entityRegexName[entityRegexNameIdx], seOrDCFEString, graphsToPlot[curGraphIdx], "is", datali
					plt.plot(fileSizes, datali)
					legendsli.append(curAppArea+"_"+entityRegexName[entityRegexNameIdx]+"_"+seOrDCFEString+"_"+graphsToPlot[curGraphIdx])
				plt.legend(legendsli, loc='upper left')
				plt.xlabel('Number of records')
				plt.ylabel("Communication cost in KB" if curGraphIdx == 0 else "Timing in sec")
				plt.savefig(os.path.join(graphSaveFolder, curAppArea + '_' + seOrDCFEString + '_' + curGraph + '.png'))
				plt.clf()

def plotSetupGraphsForSingleDO():
	global applications
	figid = 0
	#for idx = [0, 4]
	for idx in xrange(5): #idx is used for indexing into setupd
		plt.figure(figid)
		figid += 1
		for curAppArea in applications:
			datali = getSetupGraphsDataListFromGodData(curAppArea, idx)
			plt.plot(fileSizes, datali)
		plt.legend(applications, loc='upper left')
		plt.xlabel('Number of records')
		plt.ylabel(legendMappingForIdxSetupPhaseGraphs(idx))
		plt.savefig(os.path.join(graphSaveFolder, 'setupGraph_' + str(idx)))
		plt.clf()

	#for idx = [5,6]
	for idx in xrange(5,7):
		if idx == 5: appAreasGroupedLi = [["MAF", "ChiSq", "ValueRet", "Hamming"]]
		if idx == 6: appAreasGroupedLi = [["MAF", "ChiSq"], ["ValueRet"], ["Hamming"]]
		for appAreasGrouped in appAreasGroupedLi:
			plt.figure(figid)
			figid += 1
			for curAppArea in appAreasGrouped:
				if (curAppArea in applications):
					datali = getSetupGraphsDataListFromGodData(curAppArea, idx)
					plt.plot(fileSizes, datali)
			plt.legend(appAreasGrouped, loc='upper left')
			plt.xlabel('Number of records')
			plt.ylabel(legendMappingForIdxSetupPhaseGraphs(idx))
			plt.savefig(os.path.join(graphSaveFolder, 'setupGraph_' + str(idx) + '_' + ('_'.join(appAreasGrouped))))
			plt.clf()

#--->Final plot functions for single DO

def plotQEvsSESingleDOSetupComm(figid):
	global applications, goddata, fileSizes
	totalSEComm = []
	totalDCFEComm = []
	for curAppArea in applications:
		totalSEComm.append(list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][2] + goddata[(curAppArea, curFileSize)][3][4], fileSizes)))
		totalDCFEComm.append(list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][6], fileSizes)))

	plt.figure(figid)
	figid += 1

	#plot QE
	plt.subplot(211)
	for ii,curSEComm in enumerate(totalSEComm):
		curDCFEComm = totalDCFEComm[ii]
		curQEComm = list(map(operator.add, curSEComm, curDCFEComm))
		plt.plot(fileSizes, curQEComm)
	plt.legend(applications, loc='upper left')

	plt.subplot(212)
	plt.plot(fileSizes, totalSEComm[applications.index("ChiSq")])
	plt.plot(fileSizes, totalSEComm[applications.index("MAF")])

	plt.legend(["ChiSq", "Others"], loc='upper left')

	plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_comm'))

	return figid

def plotQEvsSESingleDOSetupTimings(figid):
	global applications, goddata, fileSizes
	totalSETime = []
	totalDCFETime = []
	for curAppArea in applications:
		totalSETime.append(list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][1] + goddata[(curAppArea, curFileSize)][3][3], fileSizes)))
		totalDCFETime.append(list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][5], fileSizes)))

	plt.figure(figid)
	figid += 1

	#plot QE
	plt.subplot(211)
	for ii,curSETime in enumerate(totalSETime):
		curDCFETime = totalDCFETime[ii]
		curQETime = list(map(operator.add, curSETime, curDCFETime))
		plt.plot(fileSizes, curQETime)
	plt.legend(applications, loc='upper left')

	plt.subplot(212)
	plt.plot(fileSizes, totalSETime[applications.index("ChiSq")])
	plt.plot(fileSizes, totalSETime[applications.index("MAF")])

	plt.legend(["ChiSq", "Others"], loc='upper left')

	plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))

	return figid

def plotQEvsSESingleDOOrMultiDOQueryComm(figid, singleDO, isProtocolOPRFOne = None):
	global applications, fileSizes, goddata
	
	if singleDO: assert isProtocolOPRFOne == None
	else: isProtocolOPRFOne != None

	#Comm plot
	plt.figure(figid)
	figid += 1
	plt.subplot(211)
	totalSECommDataForApp = {}
	for curAppArea in applications:
		if (curAppArea == "Hamming"): continue ##TODO : Not plotting hamming on this right now
		totalSEComm = [0]*numdb
		totalDCFEComm = [0]*numdb
		for entityRegexNameIdx in xrange(3):
			curEntitySECommData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, True, False, singleDO)
			curEntityDCFECommData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, False, False, singleDO)
			totalSEComm = list(map(operator.add, totalSEComm, curEntitySECommData))
			totalDCFEComm = list(map(operator.add, totalDCFEComm, curEntityDCFECommData))
		totalSEComm = map(lambda x: x/2.0, totalSEComm)
		totalDCFEComm = map(lambda x: x/2.0, totalDCFEComm)
		totalQEComm = list(map(operator.add, totalSEComm, totalDCFEComm))
		# print "apparea = ", curAppArea, "qe = ", totalQEComm
		if curAppArea=="ValueRet": print "###############", getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO), totalQEComm
		plt.plot(getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO), totalQEComm)
		totalSECommDataForApp[curAppArea] = totalSEComm
	plt.legend(map(lambda x : x + ' QE Comm', applications), loc='upper left')
	plt.subplot(212)
	plt.plot(getFilteredOrLeastFreqKwDocsSize("ChiSq", True, singleDO), totalSECommDataForApp["ChiSq"])
	plt.plot(getFilteredOrLeastFreqKwDocsSize("MAF", True, singleDO), totalSECommDataForApp["MAF"])
	plt.legend(["ChiSq SE Comm", "Others SE Comm"], loc = 'upper left')
	# plt.show()
	if singleDO:
		plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_query_comm'))
	else:
		plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_multiDO_' + ('OPRF' if isProtocolOPRFOne else 'EqCheck') + '_query_comm'))
	return figid

def plotQEvsSESingleDOOrMultiDOQueryTimings(figid, singleDO, isProtocolOPRFOne = None):
	global applications, fileSizes

	if singleDO: assert isProtocolOPRFOne == None
	else: isProtocolOPRFOne != None
	
	#Timings plot
	# plt.figure(figid)
	# figid += 1
	f, (a0,a1) = plt.subplots(2,1,gridspec_kw = {'height_ratios':[3, 1]})
	# plt.subplot(411)
	totalSETimingMap = {}
	colors = ['b', 'g', 'r', 'm']
	totalPlotHandles = []
	clientPlotHandles = []
	for curAppAreaIdx, curAppArea in enumerate(applications):
		totalSETiming = [0]*numdb
		totalDCFETiming = [0]*numdb
		totalClientTiming = [0]*numdb
		for entityRegexNameIdx in xrange(3):
			curEntitySETimingData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, True, True, singleDO)
			curEntityDCFETimingData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, False, True, singleDO)
			totalSETiming = list(map(operator.add, totalSETiming, curEntitySETimingData))
			totalDCFETiming = list(map(operator.add, totalDCFETiming, curEntityDCFETimingData))
			if (entityRegexName[entityRegexNameIdx] == "Client"):
				totalClientTiming = list(map(operator.add, curEntitySETimingData, curEntityDCFETimingData))
		totalQETiming = list(map(operator.add, totalSETiming, totalDCFETiming))
		filteredDocsLi = getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO)
		curPlotTotal, = a0.plot(filteredDocsLi, totalQETiming, label=curAppArea+' QE Time', color=colors[curAppAreaIdx],linestyle='-')
		curPlotClient, = a0.plot(filteredDocsLi, totalClientTiming, label=curAppArea+' QE Client Time', color=colors[curAppAreaIdx], marker='o', linestyle=':')
		totalSETimingMap[curAppArea] = totalSETiming
		totalPlotHandles.append(curPlotTotal)
		clientPlotHandles.append(curPlotClient)
	# first_legend = a0.legend(handles=totalPlotHandles, loc='center left')
	# a0.add_artist(first_legend)
	# a0.legend(handles=clientPlotHandles,loc='center right')
	a0.legend(handles=totalPlotHandles + clientPlotHandles,loc='upper left')
	# plt.subplot(413)
	a1.plot(getFilteredOrLeastFreqKwDocsSize('ChiSq', True, singleDO), totalSETimingMap['ChiSq'])
	a1.plot(getFilteredOrLeastFreqKwDocsSize('MAF', True, singleDO), totalSETimingMap['MAF'])
	plt.legend(['ChiSq SE Total', 'Others SE Total'], loc='upper left')

	f.tight_layout()
	if singleDO:
		plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_query_timings'))
	else:
		plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_multiDO_' + ('OPRF' if isProtocolOPRFOne else 'EqCheck') + '_query_timings'))
	return figid

###################################
#			MULTI-DO
###################################

# MultiDO Parsing functions
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

def parseMultiDOSetupPhase(appArea, isProtocolOPRFOne):
	global goddataForMultiDO
	dirpath = datafolder
	for i in xrange(numdb):
		curFileSize = fileSizes[i]
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

		goddataForMultiDO[(appArea, curFileSize)][3] = [DOFinalData, ServFinalData, AuxServFinalData]

def parseAndAddToGodDataForMultiDO(curAppArea, isProtocolOPRFOne):
	parseAndAddToGodData(curAppArea, False, True, "mDOOPRF_" if isProtocolOPRFOne else "mDOEq_") #Use this to parse query files for multiDO in the same way as for single DO
	parseMultiDOSetupPhase(curAppArea, isProtocolOPRFOne)

# MultiDO plotting functions
def helper_plotMultiDO_setupGetData(curAppArea, isThisForComm):
	global goddataForMultiDO
	datali = []
	for curFileSize in fileSizes:
		temp = 0
		for entityIdx in xrange(3):
			if (isThisForComm):
				temp += (goddataForMultiDO[(curAppArea, curFileSize)][3][entityIdx][1] + goddataForMultiDO[(curAppArea, curFileSize)][3][entityIdx][3])
			else:
				temp += (goddataForMultiDO[(curAppArea, curFileSize)][3][entityIdx][0] + goddataForMultiDO[(curAppArea, curFileSize)][3][entityIdx][2])
		datali.append(temp)
	return datali

def plotQEvsSEMultiDOSetupComm(figid, isProtocolOPRFOne):
	global applications, goddataForMultiDO
	plt.figure(figid)
	figid += 1
	for curAppArea in applications:
		datali = helper_plotMultiDO_setupGetData(curAppArea, True)
		print "@@@@@@@@@@@@@@@", curAppArea, datali
		plt.plot(fileSizes, datali)
	plt.legend(applications, loc='upper left')
	plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_multiDO_' + ('OPRF' if isProtocolOPRFOne else 'EqCheck') + '_setup_comm'))
	return figid

def plotQEvsSEMultiDOSetupTimings(figid, isProtocolOPRFOne):
	global applications, goddataForMultiDO
	plt.figure(figid)
	figid += 1
	for curAppArea in applications:
		datali = helper_plotMultiDO_setupGetData(curAppArea, False)
		plt.plot(fileSizes, datali)
	plt.legend(applications, loc = 'upper left')
	plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_multiDO_' + ('OPRF' if isProtocolOPRFOne else 'EqCheck') + '_setup_timings'))
	return figid

### IMP plotting functions
def plotAllGraphsForSingleDO(figid):
	# plotAllQueryGraphs()
	# plotSetupGraphsForSingleDO()
	figid = plotQEvsSESingleDOOrMultiDOQueryComm(figid, True)
	figid = plotQEvsSESingleDOOrMultiDOQueryTimings(figid, True)
	figid = plotQEvsSESingleDOSetupComm(figid)
	figid = plotQEvsSESingleDOSetupTimings(figid)
	return figid

def plotAllGraphsForMultiDO(figid, isProtocolOPRFOne):
	figid = plotQEvsSEMultiDOSetupComm(figid, isProtocolOPRFOne)
	figid = plotQEvsSEMultiDOSetupTimings(figid, isProtocolOPRFOne)
	figid = plotQEvsSESingleDOOrMultiDOQueryComm(figid, False, isProtocolOPRFOne)
	figid = plotQEvsSESingleDOOrMultiDOQueryTimings(figid, False, isProtocolOPRFOne)
	return figid

### IMP parsing functions
def parseDataForSingleDO():
	global applications
	for curAppArea in applications:
		parseAndAddToGodData(curAppArea, True, False)
	print "**************Single DO***************"
	print goddata

def parseDataForMultiDO(isProtocolOPRFOne):
	global applications
	for curAppArea in applications:
		parseAndAddToGodDataForMultiDO(curAppArea, isProtocolOPRFOne)

	print "**************Mutli DO***************"
	print goddataForMultiDO

### New plotting functions

def plotNewPlotFunctionForSingleDO(figid):
	global applications, fileSizes, goddata
	#query phase
	singleDO = True
	curMarkerSize=5
	ifTimeArr = [False, True]
	plt.figure(figid)
	figid += 1
	for ifTime in ifTimeArr:
		if ifTime:
			plt.subplot(212)
		else:
			plt.subplot(211)
		totalSEDataForApp = {}
		plotHandles = []
		for curAppArea in applications:
			if (curAppArea == "Hamming"): continue ##TODO : Not plotting hamming on this right now
			totalSEData = [0]*numdb
			totalDCFEData = [0]*numdb
			for entityRegexNameIdx in xrange(3):
				curEntitySEData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, True, ifTime, singleDO)
				curEntityDCFEData = getQueryGraphsListFromGodData(curAppArea, entityRegexNameIdx, False, ifTime, singleDO)
				totalSEData = list(map(operator.add, totalSEData, curEntitySEData))
				totalDCFEData = list(map(operator.add, totalDCFEData, curEntityDCFEData))
			totalSEData = map(lambda x: x/2.0, totalSEData)
			totalDCFEData = map(lambda x: x/2.0, totalDCFEData)
			totalQEData = list(map(operator.add, totalSEData, totalDCFEData))
			# print "^^^^^^^^^^",getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO)
			handle, = plt.plot(getFilteredOrLeastFreqKwDocsSize(curAppArea, True, singleDO), totalQEData, applicationColors[curAppArea]+"o-", label=curAppArea, alpha=0.7, markersize=curMarkerSize+1, markeredgecolor='k', markerfacecolor='w')
			plotHandles.append(handle)
			totalSEDataForApp[curAppArea] = totalSEData
		
		handle, = plt.plot(getFilteredOrLeastFreqKwDocsSize("MAF", True, singleDO), totalSEDataForApp["MAF"], applicationColors["MAF"]+"^--", label='SE comp (MAF/ValueRet)',markersize=curMarkerSize, markeredgecolor='k', markerfacecolor='k')
		plotHandles.append(handle)
		handle, = plt.plot(getFilteredOrLeastFreqKwDocsSize("ChiSq", True, singleDO), totalSEDataForApp["ChiSq"], applicationColors["ChiSq"]+"^--", label='SE comp (ChiSq)',markersize=curMarkerSize, markeredgecolor='k', markerfacecolor='k')
		plotHandles.append(handle)
		plt.legend(handles=plotHandles, loc='upper left')
	plt.show()
	return figid

def plotNewFuncSetupSingleDO(figid):
	global applications, goddata, fileSizes
	isCommArr = [True, False]
	plt.figure(figid)
	figid += 1
	for isComm in isCommArr:
		if isComm:
			plt.subplot(211)
		else:
			plt.subplot(212)
		totalSEData = []
		totalDCFEData = []
		for curAppArea in applications:
			totalSEData.append(list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][1+isComm] + goddata[(curAppArea, curFileSize)][3][3+isComm], fileSizes)))
			totalDCFEData.append(list(map(lambda curFileSize : goddata[(curAppArea, curFileSize)][3][5+isComm], fileSizes)))

		legendHandles = []
		#plot QE
		for ii,curSEData in enumerate(totalSEData):
			curDCFEData = totalDCFEData[ii]
			curQEData = list(map(operator.add, curSEData, curDCFEData))
			handle,=plt.plot(fileSizes, curQEData, applicationColors[applications[ii]]+'o-', label=applications[ii])
			legendHandles.append(handle)

		handle,=plt.plot(fileSizes, totalSEData[applications.index("ChiSq")], applicationColors["ChiSq"]+'^--', label='SE Comp (ChiSq)')
		legendHandles.append(handle)
		handle,=plt.plot(fileSizes, totalSEData[applications.index("MAF")], applicationColors["MAF"]+'^--', label='SE Comp (Others)')
		legendHandles.append(handle)
		plt.legend(handles=legendHandles, loc='upper left')

	# plt.savefig(os.path.join(graphSaveFolder, 'QEvsSE_singleDO_setup_timings'))
	plt.show()
	return figid


figid = 0
isProtocolOPRFOne = False
parseDataForMultiDO(isProtocolOPRFOne)
figid = plotAllGraphsForMultiDO(figid, isProtocolOPRFOne)

# parseDataForSingleDO()
# # figid = plotAllGraphsForSingleDO(figid)
# # figid = plotNewPlotFunctionForSingleDO(figid)
# figid = plotNewFuncSetupSingleDO(figid)

# x = [i for i in xrange(10)]
# y = [ pow(10,i) for i in x ]
# plt.plot(x,y)
# plt.yscale('log')
# plt.show()