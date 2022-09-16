# Open-V2

## 1.0) Installation Guide

- 1.0) Update your visual studio to latest in visual studio installer.
- 2.0) Copy mod/OpenV2 to your Vic2 folder.
- 3.0) Build container_generator, gui_window_generator & parser_generator.
- 4.0) Open file_explorer.cpp, go to line 222,
change 'E:\\programs\\Victoria2' to the path to your Vic2 folder,
make a new folder anywhere and set 'E:\\programs\\V2_scenario' to the path.
- 5.0) Set file_explorer to startup project and run. If it doesn't work try running again.

## 2.0) Credits

- schombert - Creating the original Open V2 project.
- coderguy57 - Improving schombert's code.

## 3.0) Licensing

Open V2 is licensed under the GNU General Public License version 3.0.
For the complete license text, see the file '[LICENSE](./LICENSE)'
This license applies to all files in this distribution, except as noted below.

The Eigen implementation in `Eigen` is licensed under the Mozilla Public License version 2.0.
See `Eigen/COPYING.MPL2` for the complete license text.

The Boost implementation in `lib/boost` is licensed under the Boost Software License version 1.0.
See `lib/boost/LICENSE_1_0.txt` for the complete license text.

The Freetype implementation in `lib/freetype` is licensed under the Freetype Project License.
See `lib/freetype/FTL.txt` for the complete license text.

The Zlib implementation in `lib/zlib` is licensed under the zlib License.
See `lib/zlib/README` for the complete license text.

The GLEW implementation in `lib`, `soil` and `graphics` is licensed under the Mesa-3D License.
See `lib/GLEW_LICENSE.txt` for the complete license text.

The Stb Image implementation in `soil` is licensed under the Public Domain.
See `soil/stb_image.h` for the complete license text.

The Image Helper implementation in `soil` is licensed under the MIT License.
See `soil/image_helper.c` for the complete license text.