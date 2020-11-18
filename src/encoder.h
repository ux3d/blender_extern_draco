#pragma once

#include "auxiliary.h"

/**
 * The opaque Draco encoder.
 * A single instance is only intended to encode a single primitive.
 */
struct DracoEncoder;

API(DracoEncoder *) encoderCreate ();

API(void) encoderRelease(DracoEncoder *encoder);

API(void) encoderSetCompressionLevel(DracoEncoder *encoder, uint32_t compressionLevel);

API(void) encoderSetQuantizationBits(DracoEncoder *encoder, uint32_t position, uint32_t normal, uint32_t texCoord, uint32_t generic);

API(bool) encoderEncode(DracoEncoder *encoder);

API(bool) encoderEncodeMorphed(DracoEncoder *encoder);

API(uint64_t) encoderGetByteLength(DracoEncoder const *encoder);

API(void) encoderCopy(DracoEncoder const *encoder, uint8_t *o_data);

API(void) encoderSetFaces(DracoEncoder *encoder, uint32_t index_count, uint32_t index_byte_length, uint8_t const *indices);

API(uint32_t) encoderAddPositions(DracoEncoder *encoder, uint32_t count, uint8_t const *data); // float

API(uint32_t) encoderAddNormals (DracoEncoder *encoder, uint32_t count, uint8_t const *data); // float

API(uint32_t) encoderAddUVs(DracoEncoder *encoder, uint32_t count, uint8_t const *data); // float

API(uint32_t) encoderAddJoints(DracoEncoder *encoder, uint32_t count, uint8_t const *data); // uint16

API(uint32_t) encoderAddWeights(DracoEncoder *encoder, uint32_t count, uint8_t const *data); // float
