import golly as g 
import copy

inputFile = "C:\\Users\\SimSim314\\Documents\\GitHub\\FastGlue\\results_73500.txt"

snakeLineHor = g.parse("2obob2obo$ob2obob2o!")
snakeLineVer = g.transform(snakeLineHor, -3, 3, 0, 1, 1, 0)

figure8 = [snakeLineVer, g.transform(snakeLineVer, 0, 13),  snakeLineHor, g.transform(snakeLineHor, 0, 13), g.transform(snakeLineHor, 0, 26), g.transform(snakeLineVer, 13, 0), g.transform(snakeLineVer, 13, 13)]

def PlaceDigit(digit, x = 0, y = 0):
   digitIdx = [[0,1,2,4,5,6], [5,6],[1,2,3,4,5],[2,3,4,5,6],[0,3,5,6],[0,2,3,4,6],[0,1,2,3,4,6],[2,5,6],[0,1,2,3,4,5,6],[0,2,3,4,5,6]]
   
   if digit >= 0 and digit <= 9:
      for idx  in digitIdx[digit]:
         g.putcells(figure8[idx], x, y)
		 
def NumDigit(num):
   if num < 10: 
      return 1 
   else: 
      return 1 + NumDigit(int((num - (num % 10))/10))
      
def PlaceNumber(number, x = 0, y = 0):
   curNum = number
   d = 20 * NumDigit(number)
   
   while True:
      PlaceDigit(curNum%10, x + d, y)
      curNum = (curNum - curNum%10) / 10 
      
      if curNum == 0:
         return 
      
      d -= 20

def FindRightMost(cells):
	maxX = -10000
	minY = 10000
	
	for i in xrange(1, len(cells), 2):
		x  = cells[i - 1]
		y = cells[i]
		
		if x == maxX:
			if minY > y:
				minY = y
		
		if  x > maxX:
			maxX = x
			minY = y 
		
		
	return (maxX, minY)
	

def HasEdgeShooter(minx):
	rect = g.getrect()
	
	if len(rect) == 0:
		return -1
		
	y = rect[1] + rect[3]
	
	if y < 100: 
		return -1
	
	if rect[2] > 120:
		return -1
		
	g.run(4)
	
	rect = g.getrect()
	
	if y + 2 == rect[1] + rect[3]:
	
		maxX = -10000
		minL = 10000
		minY = 10000
		for i in xrange(0, 4):
		
			g.run(1)
			rect = g.getrect()
			curCells = g.getcells([rect[0], rect[1] + rect[3] - 10, rect[2], 11])
			curx, cury = FindRightMost(curCells)
			curL = len(curCells)
			
			if curL <= minL:
				minL = curL
				
				if curx > maxX or (curx == maxX and cury < minY):
					maxX = curx
					minY = cury
					
					parity = (int(g.getgen()) % 4) + (cury % 2) * 4
					
					if curL == 22:
						parity += 8
					if curL == 26: 
						parity += 16
						
		if minx - 2 > maxX:
			
			cells = g.getcells([-100, -100, 200, 200])
			
			for i in xrange(1, len(cells), 2):
				if  cells[i - 1] - 2 < maxX:
					return -1
			
			return parity
		
	return -1
	
	
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

edgeShooters = []

for i in xrange(0, 24):
	edgeShooters.append([])
	
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
	
	edgeType = HasEdgeShooter(minX)
	
	if  edgeType >= 0:
		edgeShooters[edgeType].append(copy.copy(r))

dy = 0 
g.new("")
result = ""
idx = 0 

for edgeType in edgeShooters:
	dx = 0
	PlaceNumber(idx, -250, dy + 100)
	idx += 1
	
	for r in edgeType:
		i = 100

		g.putcells(block, dx, dy)

		for res in r:
			g.putcells(gld, dx + i, dy + i + res)
			i += 100
			result += str(res) + ","

		result += "\n"
		dx += 1000
	
	dy += 5000
	result += "---------------\n"
	
g.setclipstr(result)
g.exit("Finish Success")