IF(FALSE)

ADD_SUBDIRECTORY(dependencies)

set(ENV{VULKAN_SDK} "C:/VulkanSDK/1.3.243.0")
FIND_PACKAGE(Vulkan)

ADD_EXECUTABLE(
    tide

    main.cpp
    imgui_unity.cpp
    example.cpp
    node.cpp
    bezier.cpp
    block.cpp

    ast_render_visitor.cpp
    ast_render.cpp
    ast_input.cpp

    convert/to_graph.cpp
    convert/to_ast.cpp

    engine/vk_engine.cpp
    engine/vk_initializers.cpp
    engine/vk_mesh.cpp
    engine/vk_textures.cpp

    ${BACKWARD_ENABLE}
)
add_backward(tide)

TARGET_INCLUDE_DIRECTORIES(tide PUBLIC .)
TARGET_INCLUDE_DIRECTORIES(tide PUBLIC engine)
TARGET_INCLUDE_DIRECTORIES(tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/imgui)
# TARGET_INCLUDE_DIRECTORIES(tide PUBLIC /home/newton/work/lython/dependencies/SDL/include)


IF(WIN32) 
    SET(SYS_LIBRARY Pathcch)
ENDIF()

IF (WIN32)
    TARGET_INCLUDE_DIRECTORIES (tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/SDL/include)
    target_link_directories(tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/SDL/VisualC/x64/Debug/)

    TARGET_INCLUDE_DIRECTORIES (tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/SDL_ttf)
    target_link_directories(tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/SDL_ttf/VisualC/x64/Debug/)
    SET(SDL_LIBRARY SDL2 SDL2main SDL2_ttf)

ELSE()
    TARGET_INCLUDE_DIRECTORIES (tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/SDL_ttf)
    SET(SDL_LIBRARY SDL2::SDL2-static SDL2::SDL2main SDL2_ttf::SDL2_ttf-static)
ENDIF()
# FIND_PACKAGE(SDL REQUIRED)

# ARGET_INCLUDE_DIRECTORIES(tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/imgui-node-editor/external/imgui)
TARGET_INCLUDE_DIRECTORIES(tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/imgui/backends)
TARGET_INCLUDE_DIRECTORIES(tide PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/imgui-node-editor)
TARGET_LINK_LIBRARIES(tide 
    liblython
    vkbootstrap vma glm tinyobjloader stb_image liblogging ${SYS_LIBRARY} 
    Vulkan::Vulkan ${SDL_LIBRARY} liblogging argparse::argparse
    flecs::flecs_static
)

# TARGET_LINK_LIBRARIES(tide Vulkan::Vulkan 
#     /home/newton/work/lython/build/lib/libSDL2d.a 
#     /home/newton/work/lython/build/lib/libSDL2maind.a
# )


# get_target_property(OUT SDL2::SDL2-static  LINK_LIBRARIES)
# message(STATUS ${OUT})

FILE(GLOB_RECURSE SHADERS_SRC *.frag *.vert)

# SET(SHADER_COMPILER /opt/vulkan/1.2.148.0/x86_64/bin/glslc)
FUNCTION(add_shader SHADER)
    # Get filename only
    get_filename_component(SHADER_FILE_NAME ${SHADER} NAME)

    # Get relative path to shader
    FILE(RELATIVE_PATH SHADER_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER})

    # Get relative directories
    get_filename_component(SHADER_PATH ${SHADER_FILE_PATH} DIRECTORY)

    # make the shader folder structure mirror the build folder
    # so the path is consistent between source and build
    # later when we will zip all shaders together for fast loading
    SET(BTYPE Debug)
    SET(OUT_FOLDER "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${BTYPE}/${SHADER_PATH}")
    FILE(MAKE_DIRECTORY "${OUT_FOLDER}")
    SET(SHADER_OUT "${OUT_FOLDER}/${SHADER_FILE_NAME}.spv")

    ADD_CUSTOM_TARGET(
        shader_${SHADER_FILE_NAME}
        COMMAND Vulkan::glslc ${SHADER} -g -o ${SHADER_OUT}
        DEPENDS ${SHADER}
        BYPRODUCTS ${SHADER_OUT}
        COMMENT "Building ${SHADER}.spv"
    )
    ADD_DEPENDENCIES(tide shader_${SHADER_FILE_NAME})
ENDFUNCTION()

ADD_CUSTOM_TARGET(shaders ALL SOURCES ${SHADERS_SRC})

FOREACH(SHADER ${SHADERS_SRC})
    add_shader(${SHADER})
ENDFOREACH()

ENDIF()