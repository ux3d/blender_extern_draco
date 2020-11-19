/**
 * @author Jim Eckerlein <eckerlein@ux3d.io>
 * @date   2019-11-18
 */

#include "encoder.h"

#include <memory>
#include <vector>

#include "draco/mesh/mesh.h"
#include "draco/core/encoder_buffer.h"
#include "draco/compression/encode.h"

#define LOG_PREFIX "DracoEncoder | "

struct Encoder
{
    draco::Mesh mesh;
    std::vector<std::unique_ptr<draco::DataBuffer>> buffers;
    draco::EncoderBuffer encoderBuffer;
    uint32_t compressionLevel = 7;
    struct
    {
        uint32_t positions = 14;
        uint32_t normals = 10;
        uint32_t uvs = 12;
        uint32_t generic = 12;
    } quantization;
};

Encoder *encoderCreate(uint32_t vertexCount)
{
    Encoder *encoder = new Encoder;
    encoder->mesh.set_num_points(vertexCount);
    return encoder;
}

void encoderRelease(Encoder *encoder)
{
    delete encoder;
}

void encoderSetCompressionLevel(Encoder *encoder, uint32_t compressionLevel) {
    encoder->compressionLevel = compressionLevel;
}

void encoderSetQuantizationBits(Encoder *encoder, uint32_t position, uint32_t normal, uint32_t texCoord, uint32_t generic)
{
    encoder->quantization.positions = position;
    encoder->quantization.normals = normal;
    encoder->quantization.uvs = texCoord;
    encoder->quantization.generic = generic;
}

bool encoderEncode(Encoder *encoder, uint8_t preserveTriangleOrder)
{
    printf(LOG_PREFIX "Preserve triangle order: %s\n", preserveTriangleOrder ? "yes" : "no");
    
    draco::Encoder dracoEncoder;

    int speed = 10 - static_cast<int>(encoder->compressionLevel);
    dracoEncoder.SetSpeedOptions(speed, speed);

    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::POSITION, encoder->quantization.positions);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::NORMAL, encoder->quantization.normals);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::TEX_COORD, encoder->quantization.uvs);
    dracoEncoder.SetAttributeQuantization(draco::GeometryAttribute::GENERIC, encoder->quantization.generic);
    
    if (preserveTriangleOrder)
    {
        dracoEncoder.SetEncodingMethod(draco::MESH_SEQUENTIAL_ENCODING);
    }
    
    auto encoderStatus = dracoEncoder.EncodeMeshToBuffer(encoder->mesh, &encoder->encoderBuffer);
    if (encoderStatus.ok())
    {
        printf(LOG_PREFIX "Encoded %" PRIu32 " vertices, %" PRIu32 " indices\n", encoder->mesh.num_points(), encoder->mesh.num_faces() * 3);
        return true;
    }
    else
    {
        printf(LOG_PREFIX "Error during Draco encoding: %s\n", encoderStatus.error_msg());
        return false;
    }
}

uint64_t encoderGetByteLength(Encoder *encoder)
{
    return encoder->encoderBuffer.size();
}

void encoderCopy(Encoder *encoder, uint8_t *data)
{
    memcpy(data, encoder->encoderBuffer.data(), encoder->encoderBuffer.size());
}

template<class T>
void setFaces(draco::Mesh &mesh, int indexCount, T *indices)
{
    int face_count = indexCount / 3;
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

void encoderSetFaces(Encoder *encoder, uint32_t indexCount, uint32_t indexStride, uint8_t *indices)
{
	switch (indexStride)
    {
        case 1:
        {
            setFaces(encoder->mesh, indexCount, reinterpret_cast<uint8_t *>(indices));
            break;
        }
        case 2:
        {
            setFaces(encoder->mesh, indexCount, reinterpret_cast<uint16_t *>(indices));
            break;
        }
        case 4:
        {
            setFaces(encoder->mesh, indexCount, reinterpret_cast<uint32_t *>(indices));
            break;
        }
        default:
        {
            printf(LOG_PREFIX "Unsupported index stride %d\n", indexStride);
            break;
        }
    }
}

uint32_t addAttributeToMesh(Encoder *encoder, draco::GeometryAttribute::Type semantics, draco::DataType dataType, uint32_t count, uint8_t componentCount, uint8_t componentSize, uint8_t *data)
{
    auto buffer = std::make_unique<draco::DataBuffer>();

	draco::GeometryAttribute attribute;
	attribute.Init(semantics, &*buffer, componentCount, dataType, false, componentSize * componentCount, 0);

    auto id = static_cast<uint32_t>(encoder->mesh.AddAttribute(attribute, true, count));

    for (uint32_t i = 0; i < count; i++)
    {
        encoder->mesh.attribute(id)->SetAttributeValue(draco::AttributeValueIndex(i), data + i * componentCount * componentSize);
    }

    encoder->buffers.emplace_back(std::move(buffer));

    return id;
}

draco::GeometryAttribute::Type getAttributeSemantics(char *attribute)
{
    if (!strcmp(attribute, "POSITION"))
    {
        return draco::GeometryAttribute::POSITION;
    }
    if (!strcmp(attribute, "NORMAL"))
    {
        return draco::GeometryAttribute::NORMAL;
    }
    if (!strncmp(attribute, "TEXCOORD", strlen("TEXCOORD")))
    {
        return draco::GeometryAttribute::TEX_COORD;
    }
    if (!strncmp(attribute, "COLOR", strlen("COLOR")))
    {
        return draco::GeometryAttribute::COLOR;
    }
    
    return draco::GeometryAttribute::GENERIC;
}

draco::DataType getDataType(size_t componentType)
{
    switch (componentType)
    {
        case ComponentType::Byte:
            return draco::DataType::DT_INT8;
            
        case ComponentType::UnsignedByte:
            return draco::DataType::DT_UINT8;
            
        case ComponentType::Short:
            return draco::DataType::DT_INT16;
            
        case ComponentType::UnsignedShort:
            return draco::DataType::DT_UINT16;
            
        case ComponentType::UnsignedInt:
            return draco::DataType::DT_UINT32;
            
        case ComponentType::Float:
            return draco::DataType::DT_FLOAT32;
            
        default:
            return draco::DataType::DT_INVALID;
    }
}

API(uint32_t) encoderSetAttribute(Encoder *encoder, char *attributeName, size_t componentType, char *dataType, void *data)
{
    auto buffer = std::make_unique<draco::DataBuffer>();
    uint32_t count = encoder->mesh.num_points();
    size_t componentCount = getNumberOfComponents(dataType);
    size_t stride = getAttributeStride(componentType, dataType);
    draco::DataType dracoDataType = getDataType(componentType);
    
    if (dracoDataType == draco::DataType::DT_INVALID)
    {
        printf(LOG_PREFIX "Unsupported component type %zu.\n", componentType);
        return -1;
    }
    
    draco::GeometryAttribute::Type semantics = getAttributeSemantics(attributeName);
    draco::GeometryAttribute attribute;
    attribute.Init(semantics, &*buffer, componentCount, getDataType(componentType), false, stride, 0);

    auto id = static_cast<uint32_t>(encoder->mesh.AddAttribute(attribute, true, count));
    auto dataBytes = reinterpret_cast<uint8_t *>(data);

    for (uint32_t i = 0; i < count; i++)
    {
        encoder->mesh.attribute(id)->SetAttributeValue(draco::AttributeValueIndex(i), dataBytes + i * stride);
    }

    encoder->buffers.emplace_back(std::move(buffer));
    return id;
}
