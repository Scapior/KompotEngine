import KompotEngine as engine

def onTick(*args, **kwargs):
    world = kwargs['worldId']
    
    newObjectId = engine.addObject(world, 'Cube')    
    pos = newObjectId * -0.1
    engine.scaleObject(world, newObjectId, 0.1, 0.1, 0.1)
    engine.moveObjectTo(world, newObjectId, pos, pos, pos)
    
