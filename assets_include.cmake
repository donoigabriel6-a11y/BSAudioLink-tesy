# Embed binary assets into the native shared library.
set(ASSETS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets")
set(ASSET_BINARIES_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/binaryAssets")
set(PREPENDED_ASSETS_DIR "${CMAKE_CURRENT_BINARY_DIR}/prependedAssets")
set(ASSET_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/assets.hpp")
set(BASE64_DECODER "${CMAKE_CURRENT_SOURCE_DIR}/scripts/decode_base64.py")

set(ASSET_HEADER_DATA "#pragma once
#include <string_view>
#include <span>
#include \"beatsaber-hook/shared/utils/typedefs.h\"

struct IncludedAsset {
    IncludedAsset(uint8_t* start, uint8_t* end) : array(reinterpret_cast<Array<uint8_t>*>(start)) {
        array->klass = nullptr;
        array->monitor = nullptr;
        array->bounds = nullptr;
        array->max_length = end - start - 33;
        *(end - 1) = '\\0';
    }
    operator ArrayW<uint8_t>() const { init(); return array; }
    operator std::string_view() const { return { reinterpret_cast<char*>(array->_values), array->get_Length() }; }
    operator std::span<uint8_t>() const { return { array->_values, array->get_Length() }; }
    void init() const { if (!array->klass) array->klass = classof(Array<uint8_t>*); }
private:
    Array<uint8_t>* array;
};
#define DECLARE_FILE(name) \\
    extern \"C\" uint8_t _binary_##name##_start[]; \\
    extern \"C\" uint8_t _binary_##name##_end[]; \\
    const IncludedAsset name { _binary_##name##_start, _binary_##name##_end};
namespace IncludedAssets {
")

file(MAKE_DIRECTORY "${ASSET_BINARIES_DIRECTORY}" "${PREPENDED_ASSETS_DIR}")
set(BINARY_ASSET_FILES "")
if(EXISTS "${ASSETS_DIRECTORY}")
    file(GLOB ASSETS LIST_DIRECTORIES false "${ASSETS_DIRECTORY}/*")
    set(HAS_BASE64_ASSET FALSE)
    foreach(FILE IN LISTS ASSETS)
        get_filename_component(ASSET_NAME "${FILE}" NAME)
        if(ASSET_NAME MATCHES "\\.base64$")
            set(HAS_BASE64_ASSET TRUE)
        endif()
    endforeach()
    if(HAS_BASE64_ASSET)
        find_package(Python3 COMPONENTS Interpreter REQUIRED)
        if(NOT EXISTS "${BASE64_DECODER}")
            message(FATAL_ERROR "Base64 asset decoder missing: ${BASE64_DECODER}")
        endif()
    endif()

    foreach(FILE IN LISTS ASSETS)
        get_filename_component(ASSET "${FILE}" NAME)
        set(SOURCE_FILE "${FILE}")
        if(ASSET MATCHES "\\.base64$")
            string(REGEX REPLACE "\\.base64$" "" ASSET "${ASSET}")
            set(SOURCE_FILE "${ASSET_BINARIES_DIRECTORY}/${ASSET}")
            add_custom_command(
                OUTPUT "${SOURCE_FILE}"
                COMMAND "${Python3_EXECUTABLE}" "${BASE64_DECODER}" "${FILE}" "${SOURCE_FILE}"
                DEPENDS "${FILE}" "${BASE64_DECODER}"
                COMMENT "Decoding embedded asset ${ASSET}"
                VERBATIM
            )
        endif()

        message(STATUS "Including asset: ${ASSET}")
        set(PREPENDED "${PREPENDED_ASSETS_DIR}/${ASSET}")
        add_custom_command(
            OUTPUT "${PREPENDED}"
            COMMAND ${CMAKE_COMMAND} -E echo_append "                                " > "${PREPENDED}"
            COMMAND ${CMAKE_COMMAND} -E cat "${SOURCE_FILE}" >> "${PREPENDED}"
            COMMAND ${CMAKE_COMMAND} -E echo_append " " >> "${PREPENDED}"
            DEPENDS "${SOURCE_FILE}"
            VERBATIM
        )

        set(OUTPUT_FILE "${ASSET_BINARIES_DIRECTORY}/${ASSET}.o")
        add_custom_command(
            OUTPUT "${OUTPUT_FILE}"
            COMMAND ${CMAKE_OBJCOPY} "${ASSET}" "${OUTPUT_FILE}" --input-target binary --output-target elf64-aarch64 --set-section-flags binary=strings
            DEPENDS "${PREPENDED}"
            WORKING_DIRECTORY "${PREPENDED_ASSETS_DIR}"
            VERBATIM
        )
        list(APPEND BINARY_ASSET_FILES "${OUTPUT_FILE}")
        string(REGEX REPLACE "[^a-zA-Z0-9]" "_" FIXED_ASSET "${ASSET}")
        set(ASSET_HEADER_DATA "${ASSET_HEADER_DATA}    DECLARE_FILE(${FIXED_ASSET})\n")
    endforeach()
endif()

set(ASSET_HEADER_DATA "${ASSET_HEADER_DATA}\n}\n")
file(GENERATE OUTPUT "${ASSET_HEADER_PATH}" CONTENT "${ASSET_HEADER_DATA}")
if(BINARY_ASSET_FILES)
    target_sources(${COMPILE_ID} PRIVATE ${BINARY_ASSET_FILES})
else()
    message(FATAL_ERROR "No runtime assets found in ${ASSETS_DIRECTORY}; AudioLink cannot initialize shader assets.")
endif()
