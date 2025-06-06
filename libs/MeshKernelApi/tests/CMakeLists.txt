# target name
string(CONCAT TARGET_NAME ${PROJECT_NAME} "UnitTests")

# Make a test executable
add_executable(${TARGET_NAME})

# source directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# include directory
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

# list of target sources
set(SRC_LIST
    ${SRC_DIR}/ApiCacheTest.cpp
    ${SRC_DIR}/ApiTest.cpp
    ${SRC_DIR}/CurvilinearGridTests.cpp
    ${SRC_DIR}/CurvilinearGridUndoTests.cpp
    ${SRC_DIR}/ErrorHandlingTests.cpp
    ${SRC_DIR}/LandBoundaryTests.cpp
    ${SRC_DIR}/Mesh1DTests.cpp
    ${SRC_DIR}/Mesh2DCasulliRefinmentTests.cpp
    ${SRC_DIR}/Mesh2DRefinmentTests.cpp
    ${SRC_DIR}/Mesh2DTests.cpp
    ${SRC_DIR}/MeshPropertyTests.cpp
    ${SRC_DIR}/PolygonTests.cpp
    ${SRC_DIR}/SmoothnessOrthogonalisationTests.cpp
    ${SRC_DIR}/TestMeshGeneration.cpp
    ${SRC_DIR}/UndoTests.cpp
    ${SRC_DIR}/UtilsTests.cpp
)

set(INC_LIST
    ${INC_DIR}/CartesianApiTestFixture.hpp
    ${INC_DIR}/TestMeshGeneration.hpp
)

# add sources to target
target_sources(${TARGET_NAME} PRIVATE ${SRC_LIST} ${INC_LIST})

# Expose the interface of the shared lib
target_include_directories(${TARGET_NAME} PUBLIC ${INC_DIR})

# Should be linked to the main library, as well as the google test library
target_link_libraries(
  ${TARGET_NAME}
  PRIVATE
    MeshKernelApi
    MeshKernelTestUtils
    gmock
    gtest_main
)

# Make sure that coverage information is produced when using gcc
if(ENABLE_CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(
    ${TARGET_NAME}
    PRIVATE
      --coverage # equivalent to -fprofile-arcs -ftest-coverage
      -static-libgcc
      -static-libstdc++
  )
  target_link_libraries(${TARGET_NAME} PRIVATE gcov)
endif()

# If you register a test, then ctest and make test will run it. You can also run
# examples and check the output, as well. Command can be a target.
add_test(
  NAME ${TARGET_NAME}
  COMMAND ${TARGET_NAME}
)

# Copy the MeshKernel shared library to the target directory
add_custom_command(
  TARGET ${TARGET_NAME}
  POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:MeshKernelApi>" "$<TARGET_FILE_DIR:MeshKernelApiUnitTests>"
    COMMENT "Copying MeshKernel shared library"
)

# group the sources in IDE tree
source_group("Source Files" FILES ${SRC_LIST})

# group the headers in IDE tree
source_group("Header Files" FILES ${INC_LIST})
