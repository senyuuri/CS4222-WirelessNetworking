"""
Convert stdout readings to csv files

For internal use. NOT to be submitted.
"""
flist = ['1min30sec163steps', '1min30sec164hold', '1min30sec167steps', '1min30sec177hold']

for f in flist:
	with open('out/'+f, 'r') as fin:
		with open('csv/'+f+'.csv', 'w') as fout:
			lines = fin.readlines()
			for line in lines:
				if 'G' in line:
					fout.write(line.replace('G','').replace(' ',''))

