find_package(Python3 3.10 REQUIRED COMPONENTS Interpreter)
add_library(ps1_flags INTERFACE)
target_compile_options(ps1_flags INTERFACE
  -O2 -g -Wall -Wextra -ffreestanding -fno-builtin -fno-pic -nostdlib
  -fdata-sections -ffunction-sections -fsigned-char -fno-strict-overflow
  -march=r3000 -mabi=32 -mfp32 -mno-mt -mno-llsc -mno-abicalls
  -mgpopt -mno-extern-sdata -G8)
target_link_options(ps1_flags INTERFACE -static -nostdlib -Wl,-gc-sections -G8
  "-T${CMAKE_CURRENT_LIST_DIR}/../third_party/ps1-bare-metal/cmake/executable.ld")

function(ps1_embed_file target symbol input)
  set(output "${CMAKE_CURRENT_BINARY_DIR}/embedded/${symbol}.s")
  if(IS_ABSOLUTE "${input}")
    set(full_input "${input}")
  else()
    get_filename_component(full_input "${CMAKE_CURRENT_SOURCE_DIR}/${input}" ABSOLUTE)
  endif()
  file(GENERATE OUTPUT "${output}" CONTENT
".section .rodata.${symbol}, \"a\"\n.balign 8\n.global ${symbol}\n.global ${symbol}_end\n${symbol}:\n.incbin \"${full_input}\"\n${symbol}_end:\n")
  target_sources(${target} PRIVATE "${output}")
  set_source_files_properties("${output}" PROPERTIES OBJECT_DEPENDS "${full_input}")
endfunction()
