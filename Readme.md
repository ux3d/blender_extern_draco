# Draco Mesh Compression for Blender

[Draco](https://github.com/google/draco) encoding and decoding for [glTF-Blender-IO](https://github.com/KhronosGroup/glTF-Blender-IO).

The Blender repository has copies of the files in `src` and the *required* Draco source files located in `<blender-root>/extern/draco`.

## Changes

Blender does not need to be built locally to test changes in Draco or the bridging code.

- Build the target `extern_draco`
- Set the environment variable `BLENDER_EXTERN_DRACO_LIBRARY_PATH` to the built dynamic library
- Launch Blender and export with compression enabled or inspect the console output if compression options are missing

## Release

- Checkout the Blender source code repository
- Copy all changed files to `<blender-root>/extern/draco`
- Open a new Blender Differential
