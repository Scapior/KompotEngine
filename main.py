import KompotEngine as engine
import time
from copy import deepcopy
from random import randint

figures = [ [[1],[1],[1],[1]],   #I
            [[0,1],[0,1],[1,1]], #J
            [[1,0],[1,0],[1,1]], #L
            [[1,1],[1,1]],       #O
            [[0,1,1],[1,1,0]],   #S
            [[1,1,0],[0,1,1]],   #Z
            [[1,1,1],[0,1,0]],   #T
            [[1,1,1,1]],         #I
            [[1,1,1],[0,0,1]],   #J
            [[1,1,1],[1,0,0]],   #L
            [[1,0],[1,1],[0,1]], #S
            [[0,1],[1,1],[1,0]], #Z
            [[0,1,0],[1,1,1]]   #T
        ]

gameIsEnded = False
level = [[None for i in range(10)] for i in range(20)]
currentObject = None
currentObjectX = None
currentObjectY = None

isWorldInitialized = False

KEY_RIGHT = 262
KEY_LEFT = 263
KEY_ENTER = 257
KEY_SPACE = 32

lastTime = time.time()

def spawnRandomObject(world):
    global currentObject
    global currentObjectX
    global currentObjectY
    
    currentObjectX = 3
    currentObjectY = 19
    
    currentObject = deepcopy(figures[randint(0, len(figures))])
    for i, line in enumerate(currentObject):        
        for j in range(len(line)):
            if currentObject[i][j] == 0:
                currentObject[i][j] = None
            else:
                currentObject[i][j] = engine.addObject(world, 'Cube')
                engine.scaleObject(world, currentObject[i][j], 0.5, 0.5, 0.5)
                engine.moveObjectTo(world, currentObject[i][j], float(3+j), float(0), float(19-i))
                
def checkIntersection():
    for i, line in enumerate(currentObject):        
        for j in range(len(line)):
            try:
                if level[currentObjectY-i][currentObjectX+j] != None and currentObject[i][j] != None:
                    return True
            except:
                return True
    return False
                
def tryMoveCurrentObject(world, xOffset, yOffset):
    global currentObjectX
    global currentObjectY
    currentObjectX += int(xOffset)
    currentObjectY += int(yOffset)
    if checkIntersection() or \
       len(currentObject)-1 > currentObjectY or \
       currentObjectX < 0 or \
       currentObjectX + len(currentObject[0]) > 10:
        currentObjectX -= int(xOffset)
        currentObjectY -= int(yOffset)
        return False
    for i, line in enumerate(currentObject):        
        for j in range(len(line)):
            if currentObject[i][j] == None:
                continue
            x,y,z = engine.getObjectPosition(world, currentObject[i][j])
            engine.moveObjectTo(world, currentObject[i][j], x+xOffset, y, z+yOffset)
    return True

def freezeObject(world):
    global currentObject
    global level
    for i, line in enumerate(currentObject):        
        for j in range(len(line)):
            level[currentObjectY-i][currentObjectX+j] = currentObject[i][j]
    currentObject = None        
    n = 0 # check complete line
    while n < 19:
        if None in level[n]:
            n += 1
        else:
            for cube in level[n]:
                engine.deleteObject(world, cube)
            for i in range(n, 19):
                for j in range(10):
                    objectId = level[i+1][j]
                    level[i][j] = objectId
                    if objectId is not None:                        
                        x,y,z = engine.getObjectPosition(world, objectId)
                        engine.moveObjectTo(world, objectId, x, y, z-1.0)
            level[19] = [None for i in range(10)]
                    

def onTick(*args, **kwargs):
    global lastTime
    global currentObject
    global gameIsEnded
    global isWorldInitialized
    
    world = kwargs['worldId']
    
    if not isWorldInitialized:
        isWorldInitialized = True
        for i in range(20):
            for j in [-1, 10]:
                borderCube = engine.addObject(world, 'Cube')
                engine.scaleObject(world, borderCube, 0.5, 0.5, 0.5)
                engine.moveObjectTo(world, borderCube, float(j), 0.0, float(i))
        for i in range(-1, 11):
            for j in [-1, 20]:
                borderCube = engine.addObject(world, 'Cube')
                engine.scaleObject(world, borderCube, 0.5, 0.5, 0.5)
                engine.moveObjectTo(world, borderCube, float(i), 0.0, float(j))
    
    if gameIsEnded:
        return
    
    keys = engine.getInput(world)
    for key in keys:
        if key == KEY_RIGHT:
            tryMoveCurrentObject(world, -1.0, 0.0)
        if key == KEY_LEFT:
            tryMoveCurrentObject(world, 1.0, 0.0)
        if key == KEY_ENTER:
            while tryMoveCurrentObject(world, 0.0, -1.0):
                pass
            freezeObject(world)        
    
    if lastTime == None:
        lastTime = time.time()
    currentTime = time.time()
    if currentTime - lastTime < 0.5:
        return    
    lastTime = currentTime

    if currentObject == None:
        spawnRandomObject(world)
        if checkIntersection():
            gameIsEnded = True
            engine.log("game over")
    else:
        if not tryMoveCurrentObject(world, 0.0, -1.0):
            freezeObject(world)

