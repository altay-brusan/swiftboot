# Qt 6.7.3 bundles a glslang snapshot whose headers (e.g. SPIRV/SpvBuilder.h)
# use uint32_t without including <cstdint>. libstdc++ 13+ dropped the
# transitive include that used to make this compile. The native build uses the
# host compiler (g++ 15 here), so force the include in rather than carrying a
# patch against every affected bundled header.
CXXFLAGS:append:class-native = " -include cstdint"
