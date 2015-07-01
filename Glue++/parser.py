import golly as g 

ins = open("results_9.txt", "r")
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

	i = 100
	
	g.putcells(block, d, 0)
	
	for res in r:
		g.putcells(gld, d + i, i + res)
		i += 100
	
	d += 1000