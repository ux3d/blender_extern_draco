/**
 * @author Jim Eckerlein <eckerlein@ux3d.io>
 * @date   2019-11-29
 */

#include "encoder.h"

#include <memory>
#include <vector>

#include "draco/mesh/mesh.h"
#include "draco/core/encoder_buffer.h"
#include "draco/compression/encode.h"

/**
 * Prefix used for logging messages.
 */
static char const *logPrefix = "DracoEncoder";

struct DracoEncoder {
    draco::Mesh mesh;

    // One data buffer per attribute.
    std::vector<std::unique_ptr<draco::DataBuffer>> buffers;

    // The buffer the mesh is encoded into.
    draco::EncoderBuffer encoderBuffer;

    // Level of compression [0-10].
    // Higher values mean slower encoding.
    uint32_t compressionLevel = 7;

    struct {
        uint32_t positions = 14;
        uint32_t normals = 10;
        uint32_t uvs = 12;
        uint32_t generic = 12;
    } quantization;
};

DracoEncoder *create_encoder() {
    return new DracoEncoder;
}

void set_compression_level(
        DracoEncoder *const encoder,
        uint32_t const compressionLevel
) {
    encoder->compressionLevel = compressionLevel;
}

void set_position_quantization(
        DracoEncoder *const encoder,
        uint32_t const quantizationBitsPosition
) {
    encoder->quantization.positions = quantizationBitsPosition;
}

void set_normal_quantization(
        DracoEncoder *const encoder,
        uint32_t const quantizationBitsNormal
) {
    encoder->quantization.normals = quantizationBitsNormal;
}

void set_uv_quantization(
	DracoEncoder *const encoder,
	uint32_t const quantizationBitsTexCoord
) {
	encoder->quantization.uvs = quantizationBitsTexCoord;
}

void set_generic_quantization(
	DracoEncoder *const encoder,
	uint32_t const bits
) {
	encoder->quantization.generic = bits;
}

bool encode(
    DracoEncoder *const encoder
) {
    draco::Encoder dracoEncoder;

    int speed = 10 - static_cast<int>(encoder->compressionLevel);
    dracoEncoder.SetSpeedOptions(speed, speed);

    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, encoder->quantization.positions);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, encoder->quantization.normals);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, encoder->quantization.uvs);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, encoder->quantization.generic);

    return dracoEncoder.EncodeMeshToBuffer(encoder->mesh, &encoder->encoderBuffer).ok();
}

bool encode_morphed(
    DracoEncoder *const encoder
) {
    draco::Encoder dracoEncoder;

    int speed = 10 - static_cast<int>(encoder->compressionLevel);
    dracoEncoder.SetSpeedOptions(speed, speed);

    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, encoder->quantization.positions);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, encoder->quantization.normals);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, encoder->quantization.uvs);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, encoder->quantization.generic);

    // Enforce triangle order preservation.
    dracoEncoder.SetEncodingMethod(draco::MESH_SEQUENTIAL_ENCODING);

    return dracoEncoder.EncodeMeshToBuffer(encoder->mesh, &encoder->encoderBuffer).ok();
}

uint64_t get_encoded_size(
        DracoEncoder const *const encoder
) {
    return encoder->encoderBuffer.size();
}

void copy_to_bytes(
        DracoEncoder const *const encoder,
        uint8_t *const o_data
) {
    memcpy(o_data, encoder->encoderBuffer.data(), encoder->encoderBuffer.size());
}

void destroy_encoder(
        DracoEncoder *const encoder
) {
    delete encoder;
}

template<class T>
void set_faces_impl(
        draco::Mesh &mesh,
        int const index_count,
        T const *const indices
) {
    int face_count = index_count / 3;
    mesh.SetNumFaces(static_cast<size_t>(face_count));

    for (int i = 0; i < face_count; ++i)
    {
        draco::Mesh::Face face = {
            draco::PointIndex(indices[3 * i + 0]),
            draco::PointIndex(indices[3 * i + 1]),
            draco::PointIndex(indices[3 * i + 2])
        };
        mesh.SetFace(draco::FaceIndex(static_cast<uint32_t>(i)), face);
    }
}

void set_faces(
        DracoEncoder *const encoder,
        uint32_t const index_count,
        uint32_t const index_byte_length,
        uint8_t const *const indices
) {
	switch (index_byte_length)
    {
        case 1:
        {
            set_faces_impl(encoder->mesh, index_count, reinterpret_cast<uint8_t const *>(indices));
            break;
        }
        case 2:
        {
            set_faces_impl(encoder->mesh, index_count, reinterpret_cast<uint16_t const *>(indices));
            break;
        }
        case 4:
        {
            set_faces_impl(encoder->mesh, index_count, reinterpret_cast<uint32_t const *>(indices));
            break;
        }
        default:
        {
            printf("%s: Unsupported index size %d\n", logPrefix, index_byte_length);
            break;
        }
    }
}

uint32_t add_attribute_to_mesh(
        DracoEncoder *const encoder,
        draco::GeometryAttribute::Type const semantics,
        draco::DataType const data_type,
        uint32_t const count,
        uint8_t const component_count,
        uint8_t const component_size,
        uint8_t const *const data
) {
    auto buffer = std::make_unique<draco::DataBuffer>();

	draco::GeometryAttribute attribute;

	attribute.Init(
		semantics,
		&*buffer,
		component_count,
        data_type,
		false,
        component_size * component_count,
		0
	);

    auto const id = static_cast<uint32_t>(encoder->mesh.AddAttribute(attribute, true, count));

    for (uint32_t i = 0; i < count; i++)
    {
        encoder->mesh.attribute(id)->SetAttributeValue(
            draco::AttributeValueIndex(i),
            data + i * component_count * component_size
        );
    }

    encoder->buffers.emplace_back(std::move(buffer));

    return id;
}

uint32_t add_positions_f32(
    DracoEncoder *const encoder,
    uint32_t const count,
    uint8_t const *const data
) {
    encoder->mesh.set_num_points(count);

    return add_attribute_to_mesh(encoder, draco::GeometryAttribute::POSITION,
        draco::DT_FLOAT32, count, 3, sizeof(float), data);
}

uint32_t add_normals_f32(
    DracoEncoder *const encoder,
    uint32_t const count,
    uint8_t const *const data
) {
    return add_attribute_to_mesh(encoder, draco::GeometryAttribute::NORMAL,
        draco::DT_FLOAT32, count, 3, sizeof(float), data);
}

uint32_t add_uvs_f32(
    DracoEncoder *const encoder,
    uint32_t const count,
    uint8_t const *const data
) {
    return add_attribute_to_mesh(encoder, draco::GeometryAttribute::TEX_COORD,
        draco::DT_FLOAT32, count, 2, sizeof(float), data);
}

uint32_t add_joints_u16(
	DracoEncoder *encoder,
	uint32_t const count,
	uint8_t const *const data
) {
	return add_attribute_to_mesh(encoder, draco::GeometryAttribute::GENERIC,
	    draco::DT_UINT16, count, 4, sizeof(uint16_t), data);
}

uint32_t add_weights_f32(
	DracoEncoder *encoder,
	uint32_t const count,
    uint8_t const *const data
) {
    return add_attribute_to_mesh(encoder, draco::GeometryAttribute::GENERIC,
        draco::DT_FLOAT32, count, 4, sizeof(float), data);
}
