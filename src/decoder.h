#pragma once

#include "auxiliary.h"

/**
 * The opaque Draco encoder.
 * A single instance is only intended to encode a single primitive.
 */
struct Decoder;

API(Decoder *) decoderCreate();

API(void) decoderRelease(Decoder *decoder);

API(bool) decoderDecode(Decoder *decoder, void *data, size_t byteLength);

API(bool) decoderAttributeIsNormalized(Decoder *decoder, uint32_t id);

API(bool) decoderDecodeAttribute(Decoder *decoder, uint32_t id, size_t componentType, char *dataType);

API(size_t) decoderGetBufferSize(Decoder *decoder, size_t id);

API(void *) decoderGetBufferData(Decoder *decoder, size_t id);

API(bool) decoderDecodeIndices(Decoder *decoder, size_t indexComponentType);

API(size_t) decoderGetIndexBufferSize(Decoder *decoder);

API(void *) decoderGetIndexBufferData(Decoder *decoder);
