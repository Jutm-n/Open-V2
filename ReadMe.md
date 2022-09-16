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

The Eigen implementation in `3rdparty/Eigen` is licensed under the Mozilla Public License version 2.0.
See `3rdparty/Eigen/COPYING.MPL2` for the complete license text.
Note: Incompatible with secondary licenses, will have to go.

The Boost implementation in `3rdparty/boost` is licensed under the Boost Software License version 1.0.
See `3rdparty/boost/LICENSE_1_0.txt` for the complete license text.

The Freetype implementation in `3rdparty/freetype` is licensed under the Freetype Project License.
See `3rdparty/freetype/FTL.txt` for the complete license text.

The Zlib implementation in `3rdparty/zlib` is licensed under the zlib License.
See `3rdparty/zlib/README` for the complete license text.

The GLEW implementation in `3rdparty/glew` is licensed under the Mesa-3D License.
See `3rdparty/glew/LICENSE.txt` for the complete license text.

The Stb Image implementation in `3rdparty/STB_Image` is licensed under the Public Domain.
See `3rdparty/STB_Image/stb_image.h` for the complete license text.

The SOIL implementation in `3rdparty/SOIL` is licensed under the Public Domain.
See `3rdparty/SOIL/SOIL.h` for the complete license text.

The Image Helper and Image DXT implementations in `3rdparty/IMG_Helper` are licensed under the MIT License and Public Domain respectively.
See `3rdparty/IMG_Helper/image_helper.c` and `3rdparty/IMG_Helper/image_DXT.c` for the complete license texts.

The SQLite implementation in `3rdparty/SQLite` is licensed under the Public Domain.
See `3rdparty/SQLite/sqlite3.c` for the complete license text.

The Google Test implementation in `3rdparty/gtest` is licensed under the BSD 3-clause License.
See `3rdparty/gtest/gtest.h` for the complete license text.