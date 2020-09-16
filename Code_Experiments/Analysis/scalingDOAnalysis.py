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

# USENIX
CommBandwidthKBps = 620*(1<<10) #KBps #Used to approximate time taken to transfer GC from auth to server in Hamming protocol
						  #	Since GC is currently written to disk by auth and then read from disk by server, to get total time
						  # approximate the time it would take to transfer this.
datafolder = '/mnt/c/Users/t-niskum/Documents/SrchComp/USENIX_submission/Logs_ScalingDO'

applications = ["ValueRet"]
numdb=5

entityNameFileSuffices = ["Client", "Server", "AuxServer", "setup"]
entityRegexName = ["Client", "Server", "Authority", "setup"] #BEWARE : Don't change the order of entities in this list or bad things will happen 

def mapTime(x):
	#return no. of sec
	return x/(1.0*1000000)

def mapSize(x):
	#return no. of KB
	return x/((1<<10))

def showfigure(figureObj, saveFileName):
	if (displayOption == 0):
		figureObj.savefig(os.path.join(graphSaveFolder, saveFileName+'.pdf'), bbox_inches='tight')
	elif (displayOption == 1):
		figureObj.savefig(os.path.join(graphSaveFolder, saveFileName+'.png'))
	else:
		plt.show()

def helper_parseMultiDO_CollectMerge_Files(filename, entity, numEntities):
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
			if (re.search(qeCommPattern, line, re.IGNORECASE)):
				ff = re.search(qeCommPattern, line, re.IGNORECASE)
				assert ff
				data[3] += long(ff.group(1))
				ct+=1
			elif (re.search(qeTimePattern, line, re.IGNORECASE)):
				ff = re.search(qeTimePattern, line, re.IGNORECASE)
				assert ff
				data[2] += long(ff.group(1))
				ct+=1
			elif (re.search(seCommPattern, line, re.IGNORECASE)):
				ff = re.search(seCommPattern, line, re.IGNORECASE)
				assert ff
				data[1] += long(ff.group(1))
				ct+=1
			elif (re.search(seTimePattern, line, re.IGNORECASE)):
				ff = re.search(seTimePattern, line, re.IGNORECASE)
				assert ff
				data[0] += long(ff.group(1))
				ct+=1
			else:
				pass
			if (ct == numEntities*4):
				break

	# subtract qe - se to get data for dcfe
	data[2] -= data[0]
	data[3] -= data[1]

	for i in xrange(4): assert data[i]>=-50

	data[0] = mapTime(data[0])
	data[2] = mapTime(data[2])

	data[1] = mapSize(data[1])
	data[3] = mapSize(data[3])
	# data = [(i+1)*100.0 for i in range(4)]
	return data

def getDataForMultiDOProtocol(isOPRFAnalysis):
	if isOPRFAnalysis:
		fileSizes = ["20000", "40000", "60000", "80000", "100000"]
		snpSizes = ["50", "50", "50", "50", "50"]
		NumDOs=100000
	else:
		fileSizes = ["400", "800", "1200", "1600", "2000"]
		snpSizes = ["50", "50", "50", "50", "50"]
		NumDOs=2000

	typeOfMultiDOProtocol = "OPRF" if isOPRFAnalysis else "EqCheck"
	collectFileNamePrefix = "scaleDOCollect_"+applications[0]+"_"+typeOfMultiDOProtocol+"_"+str(NumDOs)+"_"

	mergeFileNamePrefix = "scaleDOMerge_"+applications[0]+"_"+typeOfMultiDOProtocol+"_"+str(NumDOs)+"_"
	seData = []
	fedData = []
	for curFileSize in fileSizes:
		DOCollectData = helper_parseMultiDO_CollectMerge_Files(os.path.join(datafolder, collectFileNamePrefix+"DO.outp"), "DO", int(curFileSize))
		ServerCollectData = [0.,0.,0.,0.] if not(isOPRFAnalysis) else helper_parseMultiDO_CollectMerge_Files(os.path.join(datafolder, collectFileNamePrefix+"Server.outp"), "Server", int(curFileSize))
		auxServerMergeData = helper_parseMultiDO_CollectMerge_Files(os.path.join(datafolder, 
																	mergeFileNamePrefix+curFileSize+"_AuxServer.outp"), 
																	"AuxServer", 
																	1)
		serverMergeData = helper_parseMultiDO_CollectMerge_Files(os.path.join(datafolder, mergeFileNamePrefix+curFileSize+"_Server.outp"),
																"Server",
																1)
		totalData = list(map(operator.add, DOCollectData, ServerCollectData))
		totalData = list(map(operator.add, auxServerMergeData, totalData))
		totalData = list(map(operator.add, serverMergeData, totalData))
		totalData[1] = totalData[1]/2 #Divide comm by 2 to avoid double counting
		totalData[3] = totalData[3]/2 #Divide comm by 2 to avoid double counting
		seData.append(totalData[0] + (totalData[1]/(1.0*CommBandwidthKBps)))
		fedData.append(totalData[0]+totalData[2]+((totalData[1]+totalData[3])/(1.0*CommBandwidthKBps)))
	return (fileSizes, seData, fedData)

figid=0
fig, axes = plt.subplots(1, 2, num=figid, figsize=(5,2.3))
plt.subplots_adjust(left=0.14, right=0.81, wspace=0.29, bottom=0.27)
figid+=1

for ii,isOPRFAnalysis in enumerate([True, False]):
	(fileSizes, seData, fedData) = getDataForMultiDOProtocol(isOPRFAnalysis)
	line2 = axes[ii].plot(fileSizes, fedData, 'ro-', markerfacecolor=('w'),markeredgecolor='k',markersize=9)
	line1 = axes[ii].plot(fileSizes, seData, 'b^--', markerfacecolor=('w'),markeredgecolor='k', markersize=6)
	axes[ii].tick_params(labelsize=9)
	axes[ii].xaxis.set_major_locator(ticker.MultipleLocator(2))
axes[0].set_title('OPRF', fontdict={'fontsize':9,'weight':'bold','fontname':'sans-serif'})
axes[1].set_title('SFE', fontdict={'fontsize':9,'weight':'bold','fontname':'sans-serif'})
axes[0].set_ylabel("Total time (sec)", fontdict={'fontsize':9})
axes[0].set_xlabel("#DOs", fontdict={'fontsize':9})
axes[1].set_xlabel("#DOs", fontdict={'fontsize':9})
fig.legend((line2[0],line1[0]),('FED\ntime','SED\ntime'), loc='right', ncol=1)
showfigure(fig, 'DOScaling')
