from ctypes import *
from io_scene_gltf2 import *
from io_scene_gltf2.blender.exp.gltf2_blender_export import *
from io_scene_gltf2.blender.exp.gltf2_blender_gather import gather_gltf2

bl_info = {
    "name": "Draco Exporter",
    "author": "",
    "version": (1, 0),
    "blender": (2, 80, 0),
    "location": "File > Export > Draco (.drc)",
    "description": "",
    "warning": "",
    "wiki_url": "",
    "tracker_url": "",
    "category": "Import-Export"
}

# Load DLL
dll = cdll.LoadLibrary(r'C:\Users\Jim Eckerlein\Documents\sources\Google\build-win64\Debug\blender-draco-exporter.dll')

dll.startMesh.restype = c_void_p
dll.startMesh.argtypes = []

dll.endMesh.restype = None
dll.endMesh.argtypes = [c_void_p, c_char_p]

dll.setFaces.restype = None
dll.setFaces.argtypes = [c_void_p, c_uint32, c_uint32, c_void_p]

dll.addPositions.restype = None
dll.addPositions.argtypes = [c_void_p, c_uint32, c_char_p]

dll.addNormals.restype = None
dll.addNormals.argtypes = [c_void_p, c_uint32, c_char_p]

dll.addTexcoords.restype = None
dll.addTexcoords.argtypes = [c_void_p, c_uint32, c_char_p]


def register():
    bpy.utils.register_class(DracoExporter)
    bpy.types.TOPBAR_MT_file_export.append(create_menu)


def unregister():
    bpy.utils.unregister_class(DracoExporter)
    bpy.types.TOPBAR_MT_file_export.remove(create_menu)


def create_menu(self, context):
    self.layout.operator(DracoExporter.bl_idname, text="Draco (.drc)")


# The main exporter class.
class DracoExporter(bpy.types.Operator, ExportHelper):
    bl_idname = "export_scene.draco_exporter"
    bl_label = "Export Draco"
    bl_options = {"PRESET"}
    filename_ext = ".drc"
    object_count = 0

    def execute(self, context):
        print("DRACO EXPORTER ...")

        export_settings = {'gltf_format': '', 'gltf_copyright': 'Copyright sample', 'gltf_texcoords': True,
                           'gltf_normals': True, 'gltf_tangents': False, 'gltf_materials': False, 'gltf_colors': False,
                           'gltf_cameras': False, 'gltf_selected': False, 'gltf_layers': True, 'gltf_extras': False,
                           'gltf_yup': True, 'gltf_apply': True, 'gltf_animations': False, 'gltf_current_frame': False,
                           'gltf_frame_range': False, 'gltf_move_keyframes': False, 'gltf_force_sampling': False,
                           'gltf_skins': False, 'gltf_bake_skins': False, 'gltf_all_vertex_influences': False,
                           'gltf_frame_step': False, 'gltf_morph': False, 'gltf_morph_normal': False,
                           'gltf_lights': False, 'gltf_texture_transform': False, 'gltf_displacement': False,
                           'gltf_channelcache': {}}

        scenes, _ = gather_gltf2(export_settings)
        node = scenes[0].nodes[0]
        primitive = node.mesh.primitives[0]
        attributes = primitive.attributes

        # Begin mesh.
        exporter = dll.startMesh()

        # Process position attributes.
        dll.addPositions(exporter, attributes['POSITION'].count, attributes['POSITION'].buffer_view.data)

        # Process normal attributes.
        dll.addNormals(exporter, attributes['NORMAL'].count, attributes['NORMAL'].buffer_view.data)

        # Process texture coordinate attributes.
        for attribute in [attributes[attr] for attr in attributes if attr.startswith('TEXCOORD_')]:
            dll.addTexcoords(exporter, attribute.count, attribute.buffer_view.data)

        # Process faces.
        index_byte_length = {
            'Byte': 1,
            'UnsignedByte': 1,
            'Short': 2,
            'UnsignedShort': 2,
            'UnsignedInt': 4,
        }
        indices = primitive.indices
        dll.setFaces(exporter, indices.count, index_byte_length[indices.component_type.name], indices.buffer_view.data)

        # Write mesh out to file.
        dll.endMesh(exporter, c_char_p(self.filepath.encode("utf-8")))

        return {"FINISHED"}
