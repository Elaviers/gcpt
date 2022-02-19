cd %~dp0
git clone --branch VER-2-11-1 https://gitlab.freedesktop.org/freetype/freetype.git freetype-2.11.1
git clone --branch v1.87 https://github.com/ocornut/imgui.git imgui-1.87

cmake -S freetype-2.11.1 -B build/freetype -G "Visual Studio 17 2022"
cd build/freetype
cmake --build . --config Release
cmake --install . --prefix ../../installed

pause