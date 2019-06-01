import KompotEngine as engine

def onTick(*args, **kwargs):
    world = kwargs['worldId']
    engine.addObject(world, 'Cube')
