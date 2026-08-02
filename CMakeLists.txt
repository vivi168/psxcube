cmake_minimum_required(VERSION 3.25)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake")
project(psxcube LANGUAGES C ASM)

set(PS1_BARE_METAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ps1-bare-metal" CACHE PATH "ps1-bare-metal submodule directory")
if(NOT EXISTS "${PS1_BARE_METAL_DIR}/src/ps1/registers.h")
  message(FATAL_ERROR "ps1-bare-metal submodule is missing; run: git submodule update --init --recursive")
endif()

include(cmake/ps1.cmake)
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/embedded")

add_library(ps1_runtime OBJECT
  ${PS1_BARE_METAL_DIR}/src/libc/clz.s
  ${PS1_BARE_METAL_DIR}/src/libc/crt0.c
  ${PS1_BARE_METAL_DIR}/src/libc/malloc.c
  ${PS1_BARE_METAL_DIR}/src/libc/misc.c
  ${PS1_BARE_METAL_DIR}/src/libc/setjmp.s
  ${PS1_BARE_METAL_DIR}/src/libc/string.c
  ${PS1_BARE_METAL_DIR}/src/libc/string.s
  ${PS1_BARE_METAL_DIR}/src/ps1/cache.s
  ${PS1_BARE_METAL_DIR}/src/vendor/printf.c
)
target_include_directories(ps1_runtime PUBLIC
  ${PS1_BARE_METAL_DIR}/src
  ${PS1_BARE_METAL_DIR}/src/libc
  ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(ps1_runtime PUBLIC ps1_flags)

add_executable(MAIN.ELF
  camera.c
  embedded_assets.c
  hashmap.c
  input.c
  io.c
  linalg.c
  main.c
  mesh.c
  noise.c
  renderer.c
  stdafx.c
  terrain.c
  system.c
  third_party/squareroot.s
)
target_link_libraries(MAIN.ELF PRIVATE ps1_runtime ps1_flags gcc)
target_include_directories(MAIN.ELF PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# TODO: CD routines to dynamically load assets instead of embedding them in the exe
ps1_embed_file(MAIN.ELF asset_cube_m3d       assets/CUBE.M3D)
ps1_embed_file(MAIN.ELF asset_house_m3d      assets/HOUSE.M3D)
ps1_embed_file(MAIN.ELF asset_cubeguy_md5m   assets/CUBEGUY.MD5M)
ps1_embed_file(MAIN.ELF asset_running_md5a   assets/RUNNING.MD5A)
ps1_embed_file(MAIN.ELF asset_cube_tim       assets/CUBE.TIM)
ps1_embed_file(MAIN.ELF asset_house_tim      assets/HOUSE.TIM)
ps1_embed_file(MAIN.ELF asset_terrain_tim    assets/TERRAIN.TIM)
ps1_embed_file(MAIN.ELF asset_branch5_tim    assets/BRANCH5.TIM)
ps1_embed_file(MAIN.ELF asset_trunk3_tim     assets/TRUNK3.TIM)
ps1_embed_file(MAIN.ELF asset_sword1_tim     assets/SWORD1.TIM)
ps1_embed_file(MAIN.ELF asset_tree1_m3d      assets/TREE1.M3D)
ps1_embed_file(MAIN.ELF asset_sword1_m3d     assets/SWORD1.M3D)

# ps1-bare-metal 4bpp debug font data
ps1_embed_file(MAIN.ELF asset_debug_font_texture assets/DEBUG_FONT_4BPP.DAT)
ps1_embed_file(MAIN.ELF asset_debug_font_palette assets/DEBUG_FONT_CLUT.DAT)

add_custom_command(TARGET MAIN.ELF POST_BUILD
  BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/MAIN.psexe"
  COMMAND "${Python3_EXECUTABLE}" "${PS1_BARE_METAL_DIR}/tools/convertExecutable.py"
          "$<TARGET_FILE:MAIN.ELF>" "${CMAKE_CURRENT_BINARY_DIR}/MAIN.psexe"
  VERBATIM)
