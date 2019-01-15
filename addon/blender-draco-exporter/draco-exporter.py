from ctypes import *
import bpy
from bpy_extras.io_utils import ExportHelper

bl_info = {
    'name': 'Draco Exporter',
    'author': '',
    'version': (1, 0),
    'blender': (2, 65, 0),
    'location': 'File > Export > Draco (.drc)',
    'description': '',
    'warning': '',
    'wiki_url': '',
    'tracker_url': '',
    'category': 'Import-Export'
}

# Load DLL
dll = cdll.LoadLibrary('C:/Users/Jim Eckerlein/Documents/sources/Google/build-win64/Debug/blender-draco-exporter.dll')

dll.createExporter.restype = c_void_p
dll.createExporter.argtypes = [c_int]

dll.disposeExporter.restype = None
dll.disposeExporter.argtypes = [c_void_p]

dll.addPosition.restype = None
dll.addPosition.argtypes = [c_void_p, c_float, c_float, c_float]

dll.addNormal.restype = None
dll.addNormal.argtypes = [c_void_p, c_float, c_float, c_float]

dll.addFace.restype = None
dll.addFace.argtypes = [c_void_p, c_int, c_int, c_int]

dll.exportMesh.restype = None
dll.exportMesh.argtypes = [c_void_p, c_char_p]


def register():
    """
    Handles the registration of the Blender Addon.
    """
    bpy.utils.register_module(__name__)
    bpy.types.INFO_MT_file_export.append(create_menu)


def unregister():
    """
    Handles the unregistering of this Blender Addon.
    """
    bpy.utils.unregister_module(__name__)
    bpy.types.INFO_MT_file_export.remove(create_menu)


if __name__ == '__main__':
    """
    Handle running the script from Blender's text editor.
    """
    register()
    bpy.ops.export_scene.my_exporter()


def create_menu(self, context):
    """
    Create the menu option for exporting.
    """
    self.layout.operator(MyExporter.bl_idname, text='Draco (.drc)')


# The main exporter class.
class MyExporter(bpy.types.Operator, ExportHelper):
    bl_idname = 'export_scene.draco_exporter'
    bl_label = 'Export Draco'
    bl_options = {'PRESET'}
    filename_ext = '.drc'
    object_count = 0

    def execute(self, context):
        print(dir(self.filepath))

        if self.filepath == '':
            print('No suitable filename was provided to save to.')
            return {'FINISHED'}

        # Collect data from objects representing meshes.
        # - Mesh with applied modifiers.
        # - Model to world matrix.
        # - Model to world normal matrix.
        object_data = [(
            o.to_mesh(bpy.context.scene, True, 'RENDER', True),
            o.matrix_world.transposed(),
            o.matrix_world.transposed().inverted()
        ) for o in bpy.data.objects if o.type == 'MESH']

        # Count number of vertices which are going to be processed.
        count_vertices = sum(map(lambda o: len(o[0].vertices), object_data))

        print('Exporting %d meshes, %d vertices' % (len(object_data), count_vertices))
        exporter = dll.createExporter(count_vertices)

        vertex_written_count = 0
        for (mesh, matrix, n_matrix) in object_data:
            vertex_written_count = self.export_mesh(exporter, vertex_written_count, mesh, matrix, n_matrix)

        dll.exportMesh(exporter, c_char_p(self.filepath.encode('utf-8')))
        dll.disposeExporter(exporter)

        # Parse all the objects in the scene.
        return {'FINISHED'}

    # Because all meshes are merged into a single Draco file,
    # There local indices must be converted to global ones:
    # [ 0 1 2 3 ] [ 0 1 2 3 ]
    # [ 0 1 2 3     4 5 6 7 ]
    # To do this, this function takes the count of the vertices which has been
    # written up to this point. Vertex indices in faces are than added to this
    # value. Finally, the new vertex count is returned, and can be passed
    # to the next invocation of this function.
    def export_mesh(self, exporter, vertex_written_count, mesh, matrix, n_matrix):
        """
        Converts a single mesh to draco.
        """

        for vertex in mesh.vertices:
            v = vertex.co * matrix
            n = vertex.normal * n_matrix
            n.normalize()
            dll.addPosition(exporter, v.x, v.y, v.z)
            dll.addNormal(exporter, n.x, n.y, n.z)

        for face in mesh.polygons:

            # Triangular face.
            if len(face.vertices) == 3:
                dll.addFace(
                    exporter,
                    vertex_written_count + face.vertices[0],
                    vertex_written_count + face.vertices[1],
                    vertex_written_count + face.vertices[2],
                )

            # Quadrilateral face.
            # Split into two triangles.
            elif len(face.vertices) == 4:
                dll.addFace(
                    exporter,
                    vertex_written_count + face.vertices[0],
                    vertex_written_count + face.vertices[1],
                    vertex_written_count + face.vertices[2],
                )
                dll.addFace(
                    exporter,
                    vertex_written_count + face.vertices[0],
                    vertex_written_count + face.vertices[2],
                    vertex_written_count + face.vertices[3],
                )

            # N-gon with n > 4.
            # User must triangulate mesh first.
            else:
                raise Exception('Found faces with more than 4 vertices. Triangulate mesh prior to exporting.')

        return len(mesh.vertices) + vertex_written_count
