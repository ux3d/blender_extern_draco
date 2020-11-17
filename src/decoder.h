#pragma once

#include "auxiliary.h"

/**
 * The opaque Draco encoder.
 * A single instance is only intended to encode a single primitive.
 */
struct DracoDecoder;

DLL_EXPORT(DracoDecoder *)
decoderCreate ();

DLL_EXPORT(void)
decoderDecoder (DracoDecoder const *decoder);

DLL_EXPORT(uint64_t)
decoderByteLength (DracoDecoder const *decoder);

DLL_EXPORT(void)
encoderCopy (
    DracoDecoder const *decoder,
    uint8_t *o_data
);

DLL_EXPORT(void)
decoderRelease (DracoDecoder const *decoder);
