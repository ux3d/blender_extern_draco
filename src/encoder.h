/**
 * C++ library for the Draco compression feature inside the glTF-Blender-IO project.
 *
 * The python side uses the CTypes library to open the DLL, load function
 * pointers add pass the data to the encoder as raw bytes.
 *
 * The encoder intercepts the regular glTF exporter after data has been
 * gathered and right before the data is converted to a JSON representation,
 * which is going to be written out.
 *
 * The original uncompressed data is removed and replaces an extension,
 * pointing to the newly created buffer containing the compressed data.
 *
 * @author Jim Eckerlein <eckerlein@ux3d.io>
 * @date   2020-11-17
 */

#include <cstdint>

#if defined(_MSC_VER)
#define DLL_EXPORT(retType) extern "C" __declspec(dllexport) retType __cdecl
#else
#define DLL_EXPORT(retType) extern "C" retType
#endif

/**
 * The opaque Draco encoder.
 * A single instance is only intended to encode a single primitive.
 */
struct DracoEncoder;

DLL_EXPORT(DracoEncoder *)
create_encoder ();

DLL_EXPORT(void)
set_compression_level (
        DracoEncoder *encoder,
        uint32_t compressionLevel
);

DLL_EXPORT(void)
set_position_quantization (
        DracoEncoder *encoder,
        uint32_t quantizationBitsPosition
);

DLL_EXPORT(void)
set_normal_quantization (
        DracoEncoder *encoder,
        uint32_t quantizationBitsNormal
);

DLL_EXPORT(void)
set_uv_quantization (
	DracoEncoder *encoder,
	uint32_t quantizationBitsTexCoord
);

DLL_EXPORT(void)
set_generic_quantization (
	DracoEncoder *encoder,
	uint32_t bits
);

/// Encodes a mesh.
/// Use `encode_morphed` when compressing primitives which have morph targets.
DLL_EXPORT(bool)
encode (
    DracoEncoder *encoder
);

/// Encodes a mesh which is used together with morph targets.
/// Use this instead of `encode`, because this procedure takes into account that mesh triangles must not be reordered.
DLL_EXPORT(bool)
encode_morphed (
    DracoEncoder *encoder
);

/**
 * Returns the size of the encoded data in bytes.
 */
DLL_EXPORT(uint64_t)
get_encoded_size (
        DracoEncoder const *encoder
);

/**
 * Copies the compressed mesh into the given byte buffer.
 *
 * @param[o_data] A Python `bytes` object.
 */
DLL_EXPORT(void)
copy_to_bytes (
        DracoEncoder const *encoder,
        uint8_t *o_data
);

/**
 * Releases all memory allocated by the encoder.
 */
DLL_EXPORT(void)
destroy_encoder (
        DracoEncoder *encoder
);

DLL_EXPORT(void)
set_faces (
        DracoEncoder *encoder,
        uint32_t index_count,
        uint32_t index_byte_length,
        uint8_t const *indices
);

/// Adds a `float` position attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
add_positions_f32 (
    DracoEncoder *encoder,
    uint32_t count,
    uint8_t const *data
);

/// Adds a `float` normal attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
add_normals_f32 (
    DracoEncoder *encoder,
    uint32_t count,
    uint8_t const *data
);

/// Adds a `float` texture coordinate attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
add_uvs_f32 (
    DracoEncoder *encoder,
    uint32_t count,
    uint8_t const *data
);

/// Adds a `unsigned short` joint attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
add_joints_u16 (
	DracoEncoder *encoder,
	uint32_t count,
	uint8_t const *data
);

/// Adds a `float` weight attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
add_weights_f32 (
	DracoEncoder *encoder,
	uint32_t count,
    uint8_t const *data
);
