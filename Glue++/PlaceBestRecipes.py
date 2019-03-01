import golly as g

inputFile = "C:/Users/SimSim314/Documents/GitHub/Glue/Glue++/best.txt"
g.new("")
ins = open(inputFile, "r")
recipes = []
for line in ins:
	
	splitVals = line.split(",")
	res = []
	for i in xrange(0, len(splitVals)):
		res.append(int(splitVals[i]))
		
	recipes.append(res)

gld = g.parse("3o$o$bo!")
block = g.parse("2o$2o!")

d = 0 
for r in recipes:
	g.putcells(block, d)
	for i in xrange(0, len(r)):
		g.putcells(gld, 150 * (i + 1) + d, 150 * (i + 1) + r[i])
		
	d += 1000