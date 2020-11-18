/**
 * @author Jim Eckerlein <eckerlein@ux3d.io>
 * @date   2020-11-18
 */


#include "decoder.h"

#include <memory>
#include <vector>
#include <cinttypes>

#include "draco/mesh/mesh.h"
#include "draco/core/decoder_buffer.h"
#include "draco/compression/decode.h"

enum ComponentType: size_t
{
    Byte = 5120,
    UnsignedByte = 5121,
    Short = 5122,
    UnsignedShort = 5123,
    UnsignedInt = 5125,
    Float = 5126,
};

struct Decoder {
    std::unique_ptr<draco::Mesh> mesh;

    std::vector<uint8_t> indexBuffer;
    
    // One data buffer per attribute.
    std::map<uint32_t, std::vector<uint8_t>> buffers;

    // The buffer the mesh is decoded from.
    draco::DecoderBuffer decoderBuffer;
    
    uint32_t vertexCount;
    uint32_t indexCount;
};

Decoder *decoderCreate()
{
    return new Decoder;
}

void decoderRelease(Decoder const *decoder)
{
    delete decoder;
}

bool decoderDecode(Decoder *decoder, void const *data, size_t byteLength)
{
    draco::Decoder dracoDecoder;
    draco::DecoderBuffer dracoDecoderBuffer;
    dracoDecoderBuffer.Init(reinterpret_cast<char const *>(data), byteLength);
    
    auto decoderStatus = dracoDecoder.DecodeMeshFromBuffer(&dracoDecoderBuffer);
    if (!decoderStatus.ok())
    {
        printf(LOG_PREFIX "Error during Draco decoding: %s\n", decoderStatus.status().error_msg());
        return false;
    }
    
    decoder->mesh = std::move(decoderStatus).value();
    decoder->vertexCount = decoder->mesh->num_points();
    decoder->indexCount = decoder->mesh->num_faces() * 3;
    return true;
}

size_t getNumberOfComponents(char const *dataType)
{
    if (!strcmp(dataType, "SCALAR"))
    {
        return 1;
    }
    if (!strcmp(dataType, "VEC2"))
    {
        return 2;
    }
    if (!strcmp(dataType, "VEC3"))
    {
        return 3;
    }
    if (!strcmp(dataType, "VEC4"))
    {
        return 4;
    }
    if (!strcmp(dataType, "MAT2"))
    {
        return 4;
    }
    if (!strcmp(dataType, "MAT3"))
    {
        return 9;
    }
    if (!strcmp(dataType, "MAT4"))
    {
        return 16;
    }
}

size_t getComponentByteLength(size_t componentType)
{
    switch (componentType)
    {
        case ComponentType::Byte:
        case ComponentType::UnsignedByte:
            return 1;
            
        case ComponentType::Short:
        case ComponentType::UnsignedShort:
            return 2;
            
        case ComponentType::UnsignedInt:
        case ComponentType::Float:
            return 4;
            
        default:
            return 0;
    }
}

size_t getAttributeStride(size_t componentType, char const *dataType)
{
    return getComponentByteLength(componentType) * getNumberOfComponents(dataType);
}

bool decoderAttributeIsNormalized(Decoder const *decoder, uint32_t id)
{
    const draco::PointAttribute* attribute = decoder->mesh->GetAttributeByUniqueId(id);
    return attribute != nullptr && attribute->normalized();
}

bool decoderDecodeAttribute(Decoder *decoder, uint32_t id, size_t componentType, char const *dataType)
{
    const draco::PointAttribute* attribute = decoder->mesh->GetAttributeByUniqueId(id);
    
    if (attribute == nullptr)
    {
        printf(LOG_PREFIX "Attribute with id=%" PRIu32 " does not exist in Draco data\n", id);
        return false;
    }
    
    size_t stride = getAttributeStride(componentType, dataType);
    
    std::vector<uint8_t> decodedData;
    decodedData.resize(stride * decoder->vertexCount);
    
    for (uint32_t i = 0; i < decoder->vertexCount; ++i)
    {
        auto index = attribute->mapped_index(draco::PointIndex(i));
        uint8_t *value = decodedData.data() + i * stride;
        
        bool converted = false;
        
        switch (componentType)
        {
        case ComponentType::Byte:
            converted = attribute->ConvertValue(index, reinterpret_cast<int8_t *>(value));
            break;
        case ComponentType::UnsignedByte:
            converted = attribute->ConvertValue(index, reinterpret_cast<uint8_t *>(value));
            break;
        case ComponentType::Short:
            converted = attribute->ConvertValue(index, reinterpret_cast<int16_t *>(value));
            break;
        case ComponentType::UnsignedShort:
            converted = attribute->ConvertValue(index, reinterpret_cast<uint16_t *>(value));
            break;
        case ComponentType::UnsignedInt:
            converted = attribute->ConvertValue(index, reinterpret_cast<uint32_t *>(value));
            break;
        case ComponentType::Float:
            converted = attribute->ConvertValue(index, reinterpret_cast<float *>(value));
            break;
        default:
            break;
        }

        if (!converted)
        {
            printf(LOG_PREFIX "Failed to convert Draco attribute type to glTF accessor type for attribute with id=%" PRIu32, id);
            return false;
        }
    }
    
    decoder->buffers[id] = decodedData;
    return true;
}

size_t decoderGetBufferSize(Decoder *decoder, size_t id)
{
    auto iter = decoder->buffers.find(id);
    if (iter != decoder->buffers.end())
    {
        return iter->second.size();
    }
    else
    {
        return 0;
    }
}

void *decoderGetBufferData(Decoder *decoder, size_t id)
{
    auto iter = decoder->buffers.find(id);
    if (iter != decoder->buffers.end())
    {
        return iter->second.data();
    }
    else
    {
        return nullptr;
    }
}

template<class T>
void decodeIndices(Decoder *decoder)
{
    std::vector<uint8_t> decodedIndices;
    decodedIndices.resize(decoder->indexCount * sizeof(T));
    T *typedView = reinterpret_cast<T *>(decodedIndices.data());
    
    for (uint32_t faceIndex = 0; faceIndex < decoder->mesh->num_faces(); ++faceIndex)
    {
        draco::Mesh::Face const &face = decoder->mesh->face(draco::FaceIndex(faceIndex));
        typedView[faceIndex * 3 + 0] = face[0].value();
        typedView[faceIndex * 3 + 1] = face[1].value();
        typedView[faceIndex * 3 + 2] = face[2].value();
    }
    
    decoder->indexBuffer = decodedIndices;
}

bool decoderDecodeIndices(Decoder *decoder, size_t indexComponentType)
{
    switch (indexComponentType)
    {
        case ComponentType::Byte:
            decodeIndices<int8_t>(decoder);
            break;
        case ComponentType::UnsignedByte:
            decodeIndices<uint8_t>(decoder);
            break;
        case ComponentType::Short:
            decodeIndices<int16_t>(decoder);
            break;
        case ComponentType::UnsignedShort:
            decodeIndices<uint16_t>(decoder);
            break;
        case ComponentType::UnsignedInt:
            decodeIndices<uint32_t>(decoder);
            break;
        default:
            printf(LOG_PREFIX "Index component type %zu not supported\n", indexComponentType);
            return false;
    }
    
    return true;
}

size_t decoderGetIndexBufferSize(Decoder *decoder)
{
    return decoder->indexBuffer.size();
}

void *decoderGetIndexBufferData(Decoder *decoder)
{
    return decoder->indexBuffer.data();
}
