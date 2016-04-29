#!/usr/bin/env python

import sys
import subprocess
import re
import operator

# more information (in particular abot objdump)
# https://github.com/contiki-ochans/contiki/wiki/Reducing-Contiki-OS-firmware-size

def computeSpace(section,f):

	result = subprocess.check_output(["msp430-objdump","-t", "--section=" + section, f])

	map = {}

	for line in result.splitlines():

		#print line
	
		m = re.search(section + "\s+(\w+)\s+(\w+)", line)

#		print(str(m))

		if not m is None:
			#print(m.group(1) + "," + m.group(2) + "\n\n")
			size = int(m.group(1),16)
			symbol_name = m.group(2) 
			map[symbol_name] = size
			

	#print(str(map))
	
	return map


if len(sys.argv) < 3 or (not (sys.argv[1] == "ram" or sys.argv[1] == "rom")):
	print "wrong arguments. first argument: ram or rom, second argument: sky file path"
else:	

	f = sys.argv[2]
	sections = ['.data']

	if sys.argv[1] == "rom":
		sections.append('.text')
	else:
		sections.append('.bss')

	overallSizeMap = {}
	for section in sections:

		sectionMap = computeSpace(section,f)

		for name, size in sectionMap.iteritems():
			if name in overallSizeMap:
				overallSizeMap[name] = overallSizeMap[name] + size
			else:
				overallSizeMap[name] = size
		
	
	#print(str(overallSizeMap))

	sortedItems = sorted(overallSizeMap.items(), key=operator.itemgetter(1),reverse=True)

	print("Aggregated size for sections " + str(sections))
	print("\nfunction/object\tsize")
	for item in sortedItems:
		print(item[0] + "\t" + str(item[1]))










