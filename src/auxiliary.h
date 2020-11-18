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

#include <cstdint>
#include <cstddef>

#if defined(_MSC_VER)
#define API(returnType) extern "C" __declspec(dllexport) returnType __cdecl
#else
#define API(returnType) extern "C" returnType
#endif

#define LOG_PREFIX "DracoDecoder | "
