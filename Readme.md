# Draco Mesh Compression for Blender

[Draco](https://github.com/google/draco) encoding and decoding for [glTF-Blender-IO](https://github.com/KhronosGroup/glTF-Blender-IO).

## Release build

```
git clone git@github.com:ux3d/blender_extern_draco.git --recursive
cd blender_extern_draco
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j 4
```

## Integration

The Draco Mesh Compression library for the Blender glTF exported/importer ([glTF-Blender-IO](https://github.com/KhronosGroup/glTF-Blender-IO)) can be overridden at runtime by setting the environment variable `BLENDER_EXTERN_DRACO_LIBRARY_PATH` to the absolute path of the built dynamic library. The library itself is assumed to be called `extern_draco.dll`, `libextern_draco.so` or `libextern_draco.dylib`, depending on the host platform.
