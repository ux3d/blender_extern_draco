/**
 * Library for the Draco encoding/decoding feature inside the glTF-Blender-IO project.
 *
 * The python script within glTF-Blender-IO uses the CTypes library to open the DLL,
 * load function pointers add pass the raw data to the encoder.
 *
 * @author Jim Eckerlein <eckerlein@ux3d.io>
 * @date   2020-11-18
 */

#pragma once

#include "auxiliary.h"

struct Encoder;

API(Encoder *) encoderCreate ();

API(void) encoderRelease(Encoder *encoder);

API(void) encoderSetCompressionLevel(Encoder *encoder, uint32_t compressionLevel);

API(void) encoderSetQuantizationBits(Encoder *encoder, uint32_t position, uint32_t normal, uint32_t texCoord, uint32_t generic);

API(bool) encoderEncode(Encoder *encoder);

API(bool) encoderEncodeMorphed(Encoder *encoder);

API(uint64_t) encoderGetByteLength(Encoder *encoder);

API(void) encoderCopy(Encoder *encoder, uint8_t *o_data);

API(void) encoderSetFaces(Encoder *encoder, uint32_t index_count, uint32_t index_byte_length, uint8_t *indices);

API(uint32_t) encoderAddPositions(Encoder *encoder, uint32_t count, uint8_t *data); // float

API(uint32_t) encoderAddNormals (Encoder *encoder, uint32_t count, uint8_t *data); // float

API(uint32_t) encoderAddUVs(Encoder *encoder, uint32_t count, uint8_t *data); // float

API(uint32_t) encoderAddJoints(Encoder *encoder, uint32_t count, uint8_t *data); // uint16

API(uint32_t) encoderAddWeights(Encoder *encoder, uint32_t count, uint8_t *data); // float
