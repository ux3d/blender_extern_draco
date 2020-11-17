#pragma once

#include "auxiliary.h"

/**
 * The opaque Draco encoder.
 * A single instance is only intended to encode a single primitive.
 */
struct DracoEncoder;

DLL_EXPORT(DracoEncoder *)
encoderCreate ();

DLL_EXPORT(void)
encoderSetCompressionLevel (
    DracoEncoder *encoder,
    uint32_t compressionLevel
);

DLL_EXPORT(void)
encoderSetQuantizationBits (
        DracoEncoder *encoder,
        uint32_t position,
        uint32_t normal,
        uint32_t texCoord,
        uint32_t generic
);

/// Encodes a mesh.
/// Use `encode_morphed` when compressing primitives which have morph targets.
DLL_EXPORT(bool)
encoderEncode (
    DracoEncoder *encoder
);

/// Encodes a mesh which is used together with morph targets.
/// Use this instead of `encode`, because this procedure takes into account that mesh triangles must not be reordered.
DLL_EXPORT(bool)
encoderEncodeMorphed (
    DracoEncoder *encoder
);

/**
 * Returns the size of the encoded data in bytes.
 */
DLL_EXPORT(uint64_t)
encoderGetByteLength (
    DracoEncoder const *encoder
);

/**
 * Copies the compressed mesh into the given byte buffer.
 *
 * @param[o_data] A Python `bytes` object.
 */
DLL_EXPORT(void)
encoderCopy (
    DracoEncoder const *encoder,
    uint8_t *o_data
);

/**
 * Releases all memory allocated by the encoder.
 */
DLL_EXPORT(void)
encoderRelease (
    DracoEncoder *encoder
);

DLL_EXPORT(void)
encoderSetFaces (
    DracoEncoder *encoder,
    uint32_t index_count,
    uint32_t index_byte_length,
    uint8_t const *indices
);

/// Adds a single-precision float position attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
encoderAddPositions (
    DracoEncoder *encoder,
    uint32_t count,
    uint8_t const *data
);

/// Adds a single-precision float normal attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
encoderAddNormals (
    DracoEncoder *encoder,
    uint32_t count,
    uint8_t const *data
);

/// Adds a single-precision float texture coordinate attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
encoderAddUVs (
    DracoEncoder *encoder,
    uint32_t count,
    uint8_t const *data
);

/// Adds an unsigned 16-bit integer joint attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
encoderAddJoints (
	DracoEncoder *encoder,
	uint32_t count,
	uint8_t const *data
);

/// Adds a single-precision float weight attribute to the current mesh.
/// Returns the id Draco has assigned to this attribute.
DLL_EXPORT(uint32_t)
encoderAddWeights (
	DracoEncoder *encoder,
	uint32_t count,
    uint8_t const *data
);
