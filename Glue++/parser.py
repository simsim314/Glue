import golly as g 


def FindRightMost(cells):
	maxX = -10000
	
	for i in xrange(1, len(cells), 2):
		if cells[i - 1] > maxX:
			maxX = cells[i - 1]
			
	return maxX
	
def HasEdgeShooter(minx):
	rect = g.getrect()
	
	if len(rect) == 0:
		return False
		
	y = rect[1] + rect[3]
	
	if y < 100: 
		return False
	
	if rect[2] > 120:
		return False
		
	g.run(4)
	
	rect = g.getrect()
	
	if y + 2 == rect[1] + rect[3]:
	
		maxX = -10000
		maxL = -10000
		
		for i in xrange(0, 4):
		
			g.run(1)
			rect = g.getrect()
			curCells = g.getcells([rect[0], rect[1] + rect[3] - 10, rect[2], 11])
			curx = FindRightMost(curCells)
			curL = len(curCells)
			
			if curx > maxX:
				maxX = curx
				
			if curL > maxL:
				maxL = curL
		
		if minx - 2 > maxX:
			
			cells = g.getcells([-100, -100, 200, 200])
			
			for i in xrange(1, len(cells), 2):
				if  cells[i - 1] - 2 < maxX:
					return False
			
			return True
		
	return False
	
ins = open("C:\\Users\\SimSim314\\Documents\\GitHub\\FastGlue\\results_10.txt", "r")
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

edgeShooters = []

for r in recipes:
	d += 1
	
	if d% 500 == 0:
		g.show(str([d, len(recipes), len(edgeShooters)]))
	g.new("")
	g.setstep(3)
	g.putcells(block)
	
	minX = 10000
	for res in xrange(0, len(r) - 1):
		g.putcells(gld, 40, 40 + r[res])
		g.step()
		g.step()
		
		rect = g.getrect()
			
		if rect[0] < minX:
			minX = rect[0]
			
	g.putcells(gld, 40, 40 + r[len(r) - 1])
	g.setstep(4)
	g.step()
	g.step()
	
	if(HasEdgeShooter(minX)):
		edgeShooters.append(r)

d = 0 
g.new("")
result = ""

for r in edgeShooters:

	i = 100
	
	g.putcells(block, d, 0)
	
	for res in r:
		g.putcells(gld, d + i, i + res)
		i += 100
		result += str(res) + ","
		
	result += "\n"
	d += 1000
	
g.setclipstr(result)
g.exit("Finish Success")