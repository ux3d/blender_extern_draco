# Google

Google Draco for Blender glTF I/O

## Building

[Google Draco](https://github.com/google/draco) is required as a git submodule, so don't forget to pull all submodules prior to building.

- Windows x64:  `cmake -G 'Visual Studio 15 2017 Win64' [path to source]`

Building directly with CMake: `cmake --build [path to build]`

Running the CMake build generates the `blender-draco-exporter.dll`. The DLL is used by the python addon to pass mesh data from Blender down to Draco.
