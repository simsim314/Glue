import golly as g 
from copy import copy

destinationPath = "C:\\Users\\SimSim314\\Glue\\AllWSS\\"
gld = g.parse("3o$o$bo!")
blck = g.parse("2o$2o!")
existingDic = {}
existingKeys = []

def FindRightMost(cells):
	maxX = -10000
	
	for i in xrange(1, len(cells), 2):
		if cells[i - 1] > maxX:
			maxX = cells[i - 1]
			
	return maxX
	
def HasEdgeShooter(minx):
	rect = g.getrect()
	
	y = rect[1] + rect[3]
	
	if y < 70: 
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
		
		if minx - 2 > maxL:
			return 100 * maxL
			
		return maxL
	
	return False
def FindActive():

   rect = g.getrect()
   
   cells = g.getcells([rect[0], rect[1], rect[2], 1])
   
   return [cells[0], cells[1]]
         
def FindConnected(listXY):
   result = copy(listXY)
   
   for xy in listXY:
      x = xy[0]
      y = xy[1]
      
      for i in xrange(-1, 2): 
         for j in xrange(-1, 2): 
            if g.getcell(x + i, y + j) > 0 and len([i for t in listXY if (t[0] == x + i and t[1] == y + j)]) == 0:
				if len([i for t in result if (t[0] == x + i and t[1] == y + j)]) == 0:
					result.append([x + i, y + j])
         
   return result
   
def RemoveList(listXY):

   for xy in listXY:
      x = xy[0]
      y = xy[1]
      g.setcell(x, y, 0)

def CountSL():
	
	result = []
	while int(g.getpop()) > 0: 
	   xy = [FindActive()]
	   
	   while True: 
		  newXY = FindConnected(xy)
		  
		  #g.getstring(str(xy) + " : " + str(newXY) )
		  
		  if len(newXY) == len(xy):
			 break
			 
		  xy = newXY
	   
	   result.append(xy)
	   RemoveList(xy)
	  
	return result
  
def PlaceRecipeList(recipes):
	
	if len(recipes) == 0:
		return 
	
	g.new("")
	dx = 0
	
	recipes.sort(key=lambda f: str(f[3]))
	curF = str(recipes[0][3])
	f = open(destinationPath + str(len(recipes)) + curF + ".txt",'w')
	
	for res in recipes:
		recipe = res[0]
		x1 = res[1]
		y1 = res[2]
		slL = res[3]
				
		g.putcells(blck, dx, 0)
		d = 0 
		for r in recipe:
			g.putcells(gld, 40 + d * 256 + dx, 40 + r + d * 256)
			d += 1
			
		dx += 256
		
		if curF != str(slL):
			f.close() 
			curF = str(slL)
			f = open(destinationPath + str(len(recipes)) + curF + ".txt",'w')
			
		f.write("{0},{1}:".format(x1, y1) + str(recipe).strip("]").strip("[") + "\n")

def EvolveRecipe(recipe):
	g.new("")
	g.setstep(3)
	
	g.putcells(blck)
	
	minx = g.getrect()[0]
	
	for r in recipe:
		g.putcells(gld, 40, 40 + r)
		g.step()
		g.step()
		
		if minx > g.getrect()[0]:
			minx > g.getrect()[0]
	
	return minx
	
result = []
	
def FindAllHs(cells, minx):
	g.new("")
	g.putcells(cells)
	rect = g.getrect()
	
	min = 10000
	max = -10000
	
	for i in xrange(1, len(cells), 2):
		cx = cells[i - 1]
		cy = cells[i]
		
		dx = 40 - cx
		cy += dx
		
		if cy < min:
			min = cy
		
		if cy > max:
			max = cy
		
	answer = [[],[]] 
	
	if (min - 9) % 2 != 0:
		min += 1
		
	for i in xrange(min - 9 - 40, max + 6 - 40, 2):
		g.new("")
		g.putcells(cells)
		g.putcells(gld, 40, 40 + i)
		g.setstep(3)
		g.step()
		g.step()
		
		if int(g.getpop()) > 80 or int(g.getpop()) == 0:
			continue 
		
		if g.getrect()[0] < -120:
			continue 
		
		edgeType = HasEdgeShooter(minx)
		
		if edgeType != False:
			answer[0].append([i, 0, 0, [edgeType]])
			
		rect = g.getrect()
		
		if rect[2] > 25 or rect[3] > 25:
			continue
		
		s = str(g.getcells(g.getrect()))
		g.run(1)
		
		if s == str(g.getcells(g.getrect())):
			
			key = str(rect) + ":" + str(g.getpop())
			
			if not (key in existingKeys):
				existingDic[key] = []
				existingKeys.append(key)
				
			if s in existingDic[key]:
				continue 
			else:
				existingDic[key].append(s)
		
			answer[1].append(i)
			
	return answer 
	
recipes = [[]]

def EvolveRecipes(recipes):
	newrecipes = []

	cnt = 0 
	
	for recipe in recipes:
		cnt += 1
		
		if cnt % 100 == 0:
			g.show(str(cnt) + "/" + str(len(recipes)))
		
		minx = EvolveRecipe(recipe)
		cells = g.getcells(g.getrect())
		
		answer = FindAllHs(cells, minx)
		
		for h in answer[0]:
			i = h[0]
			x1 = h[1]
			y1 = h[2]
			xySL = h[3]

			res = copy(recipe)
			res.append(i)
			result.append([res, x1, y1, xySL])
			
		for h in answer[1]:
			res = copy(recipe)
			res.append(h)
			newrecipes.append(res)
	
	return newrecipes
	
for i in xrange(0, 15):
	recipes = EvolveRecipes(recipes)
	
	g.show(str(i))
	g.update()

	if i >= 0:
		PlaceRecipeList(result)