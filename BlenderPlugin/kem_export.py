bl_info = {
    "name": "KompotEngine Model",
    "author": "Maxim Stoianov",
    "version": (0, 0, 1),
    "blender": (2, 80, 0),
    "location": "File > Import-Export",
    "description": "Export to KompotEngine model file",
    "warning": "",
    "wiki_url": "",
    "support": 'COMMUNITY',
    "category": "Import-Export"
}

KEM_HEADER   = [0x4b,0x45,0x4d]
KEM_CONSTANT = [0x49,0x4c,0x55,0x18,0x11,0x20,0x15]
KEM_VERSION  = 0x01

KEM_BLOCK_TYPE_VERTEX  = 0x01
KEM_BLOCK_TYPE_NORMALS = 0x02
KEM_BLOCK_TYPE_UV      = 0x03
KEM_BLOCK_TYPE_FACES   = 0x04

UINT32_MAX = 0xffffffff
KEM_BLOCK_MAX_VEC3_SIZE = UINT32_MAX - UINT32_MAX % 12
KEM_BLOCK_MAX_VEC2_SIZE = UINT32_MAX - UINT32_MAX % 8

import bpy
import struct
from bpy.props import (
        BoolProperty,
        FloatProperty,
        StringProperty,
        EnumProperty,
        )
from bpy_extras.io_utils import ExportHelper

def int_to_bytes(x, bytes_count):
    return list(x.to_bytes(bytes_count, 'big'))

#@orientation_helper(axis_forward='-Z', axis_up='Y')
class ExportKEM(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.kem"
    bl_label = 'Export KEM'
    filename_ext = ".kem"
    filter_glob: StringProperty(default="*.kem", options={'HIDDEN'})
    
    file_buffer = list()
    
    def write_bytes_block(self, block_type, flags, bytes_array, max_block_size):
        
            blocks_count = (len(bytes_array) // max_block_size) + 1
            last_block_size = len(bytes_array) % max_block_size
            
            for i in range(blocks_count):
                self.file_buffer += int_to_bytes(block_type, 1) + int_to_bytes(flags, 1) + int_to_bytes(0, 2)
                if i == blocks_count-1:
                    self.file_buffer += int_to_bytes(last_block_size, 4)
                    self.file_buffer += bytes_array[max_block_size*i:]
                else:
                    self.file_buffer += int_to_bytes(max_block_size, 4)
                    self.file_buffer += bytes_array[max_block_size*i:max_block_size*(i+1)]


    def execute(self, context):
        bpy.ops.object.mode_set(mode='OBJECT')
        for arrs in [KEM_HEADER, KEM_CONSTANT, [KEM_VERSION], [0] * 5]:
            for byte in arrs:
                self.file_buffer += int_to_bytes(byte, 1)
        verticies_offset = 0
        meshes = list(bpy.data.meshes)
        for mesh in meshes:
            if not mesh.name in bpy.data.objects:
                continue # not deleted
            mesh_object = bpy.data.objects[mesh.name]
            if not mesh_object.visible_get():
                continue # not hided

            bpy.ops.object.select_all(action='DESELECT')
            mesh_object.select_set(state = True)
            bpy.context.view_layer.objects.active = mesh_object
            bpy.ops.object.transform_apply(location = True, scale = True, rotation = True)
            mesh_object.select_set(state = False)
            bpy.ops.object.select_all(action='DESELECT')
            
            mesh.calc_loop_triangles()
            uv_layer = mesh.uv_layers.active

            vertices = []      
            for tri in mesh.loop_triangles:
                for i in range(3):
                    vertex = mesh.vertices[tri.vertices[i]]
                    vertex = (tuple(vertex.co) + tuple(vertex.normal) + tuple(uv_layer.data[tri.loops[i]].uv))
                    vertices += [vertex]

            indeces = [i for i in range(len(vertices))]

            for i in range(len(vertices)):
                for j in range(i+1, len(vertices)):
                    if vertices[i] == vertices[j]:
                        indeces[j] = i

            new_vertex_list = []
            flag_list = [None] * len(indeces)
            for index in indeces:
                flag_list[index] = 1

            new_i = 0
            for i in range(len(flag_list)):
                if flag_list[i] != None:
                    new_vertex_list += [vertices[i]]
                    for j in range(len(indeces)):
                        if indeces[j] == i:
                            indeces[j] = new_i
                    new_i += 1
            
            vertices_coords = []
            vertices_normals = []
            vertices_uv = []

            for vertex in new_vertex_list:
                (x,y,z, xn,yn,zn, xuv,yuv) = vertex
                vertices_coords += [x,y,z]
                vertices_normals += [xn,yn,zn]
                vertices_uv += [xuv,yuv]
            
            indeces_offseted = [i + verticies_offset for i in indeces]
            verticies_offset += len(new_vertex_list)
            
            verticies_bytes = [b for co in vertices_coords for b in struct.pack('f', co)]
            self.write_bytes_block(KEM_BLOCK_TYPE_VERTEX, 0, verticies_bytes, KEM_BLOCK_MAX_VEC3_SIZE)
                        
            normals_bytes = [b for co in vertices_normals for b in struct.pack('f', co)]
            self.write_bytes_block(KEM_BLOCK_TYPE_NORMALS, 0, normals_bytes, KEM_BLOCK_MAX_VEC3_SIZE)

            uv_bytes = [b for coord in vertices_uv for b in struct.pack('f', coord)]
            self.write_bytes_block(KEM_BLOCK_TYPE_UV, 0, uv_bytes, KEM_BLOCK_MAX_VEC2_SIZE)
            
            indicies_bytes = [b for vertex_index in indeces_offseted for b in int_to_bytes(vertex_index, 4)]
            self.write_bytes_block(KEM_BLOCK_TYPE_FACES, 0, indicies_bytes, KEM_BLOCK_MAX_VEC2_SIZE)
        
        with open(self.filepath, 'wb') as f:
            f.write(bytearray(self.file_buffer))
        
        return {'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(ExportKEM.bl_idname, text="KompotEngine model (.kem)")


def register():
    bpy.utils.register_class(ExportKEM)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    
    bpy.utils.unregister_class(ExportKEM)


if __name__ == "__main__":
    register()
