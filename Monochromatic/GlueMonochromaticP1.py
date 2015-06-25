import golly as g 
from copy import copy

destinationPath = "C:\\Users\\SimSim314\\Glue\\Monochromatic\\"
gld = g.parse("3o$o$bo!")
blck = g.parse("2o$2o!")
existing = []


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
		
		if curF == str(slL):
			f.write("{0},{1}:".format(x1, y1) + str(recipe).strip("]").strip("[") + "\n")
		else:
			f.close() 
			curF = str(slL)
			f = open(destinationPath + str(len(recipes)) + curF + ".txt",'w')
	
def EvolveRecipe(recipe):
	g.new("")
	g.setstep(3)
	
	g.putcells(blck)
	
	for r in recipe:
		g.putcells(gld, 40, 40 + r)
		g.step()
		g.step()

result = []
	
def FindAllHs(cells):
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
		
		rect = g.getrect()
		
		if rect[2] > 25 or rect[3] > 25:
			continue
		
		s = str(g.getcells(g.getrect()))
		g.run(1)
		
		if s == str(g.getcells(g.getrect())):
			
			SLs = []
			
			if rect[2] < 8 and rect[3] < 8:
				SLs = CountSL()
			
			if len(SLs) == 1: 
				SLs[0].sort(key=lambda r: 100000 * r[1] + r[0])
				
				x1 = SLs[0][0][0]
				y1 = SLs[0][0][1]
				
				for o in xrange(0, len(SLs[0])):
					SLs[0][o][0] -= x1
					SLs[0][o][1] -= y1

				answer[0].append([i, x1, y1, SLs[0]])
				
			if s in existing:
				continue 
			else: 
				existing.append(s)
			
			answer[1].append(i)
			
	return answer 
	
recipes = [[]]

def EvolveRecipes(recipes):
	newrecipes = []

	cnt = 0 
	
	for recipe in recipes:
		cnt += 1
		g.show(str(cnt) + "/" + str(len(recipes)))
		
		EvolveRecipe(recipe)
		cells = g.getcells(g.getrect())
		
		answer = FindAllHs(cells)
		
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