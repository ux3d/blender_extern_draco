#pragma once

#include "auxiliary.h"

/**
 * The opaque Draco encoder.
 * A single instance is only intended to encode a single primitive.
 */
struct Decoder;

DLL_EXPORT(Decoder *)
decoderCreate();

DLL_EXPORT(void)
decoderRelease(Decoder const *decoder);

DLL_EXPORT(bool)
decoderDecode(Decoder *decoder, void const *data, size_t byteLength);

DLL_EXPORT(bool)
decoderAttributeIsNormalized(Decoder const *decoder, uint32_t id);

DLL_EXPORT(bool)
decoderDecodeAttribute(Decoder *decoder, uint32_t id, size_t componentType, char const *dataType);

DLL_EXPORT(size_t)
decoderGetBufferSize(Decoder *decoder, size_t id);

DLL_EXPORT(void *)
decoderGetBufferData(Decoder *decoder, size_t id);

DLL_EXPORT(bool)
decoderDecodeIndices(Decoder *decoder, size_t indexComponentType);

DLL_EXPORT(size_t)
decoderGetIndexBufferSize(Decoder *decoder);

DLL_EXPORT(void *)
decoderGetIndexBufferData(Decoder *decoder);
