/**
 * Library for the Draco encoding/decoding feature inside the glTF-Blender-IO project.
 *
 * The python side uses the CTypes library to open the DLL, load function
 * pointers add pass the data to the encoder as raw bytes.
 *
 * The encoder intercepts the regular glTF exporter after data has been
 * gathered and right before the data is converted to a JSON representation,
 * which is going to be written out.
 *
 * The original uncompressed data is removed and replaces an extension,
 * pointing to the newly created buffer containing the compressed data.
 *
 * @author Jim Eckerlein <eckerlein@ux3d.io>
 * @date   2020-11-17
 */

#pragma once

#include <cstdint>

#if defined(_MSC_VER)
#define DLL_EXPORT(retType) extern "C" __declspec(dllexport) retType __cdecl
#else
#define DLL_EXPORT(retType) extern "C" retType
#endif
