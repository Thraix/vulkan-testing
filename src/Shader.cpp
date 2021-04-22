#include "Shader.h"

shaderc_compiler_t Shader::compiler = shaderc_compiler_initialize();
shaderc_compile_options_t Shader::compileOptions = shaderc_compile_options_initialize();
