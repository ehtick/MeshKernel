# project name
project(
  MeshKernel
  VERSION ${CMAKE_PROJECT_VERSION}
  DESCRIPTION "MeshKernel static library"
  LANGUAGES CXX C
)

# target name
set(TARGET_NAME ${PROJECT_NAME})

# static library
add_library(${TARGET_NAME} STATIC)

# source directories
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(AVERAGING_STRATEGIES_SRC_DIR ${SRC_DIR}/AveragingStrategies)
set(CURVILINEAR_GRID_SRC_DIR ${SRC_DIR}/CurvilinearGrid)
set(UTILITIES_SRC_DIR ${SRC_DIR}/Utilities)
set(UNDO_SRC_DIR ${PROJECT_SOURCE_DIR}/src/UndoActions)
set(CURVILINEAR_UNDO_SRC_DIR ${CURVILINEAR_GRID_SRC_DIR}/UndoActions)

# include directories
set(INC_DIR ${PROJECT_SOURCE_DIR}/include)
set(DOMAIN_INC_DIR ${INC_DIR}/MeshKernel)
set(AVERAGING_STRATEGIES_INC_DIR ${DOMAIN_INC_DIR}/AveragingStrategies)
set(CURVILINEAR_GRID_INC_DIR ${DOMAIN_INC_DIR}/CurvilinearGrid)
set(UTILITIES_INC_DIR ${DOMAIN_INC_DIR}/Utilities)
set(UNDO_INC_DIR ${DOMAIN_INC_DIR}/UndoActions)
set(CURVILINEAR_UNDO_INC_DIR ${CURVILINEAR_GRID_INC_DIR}/UndoActions)

# list of target sources
set(
  SRC_LIST
  ${SRC_DIR}/AveragingInterpolation.cpp
  ${SRC_DIR}/Cartesian3DPoint.cpp
  ${SRC_DIR}/CasulliDeRefinement.cpp
  ${SRC_DIR}/CasulliRefinement.cpp
  ${SRC_DIR}/ConnectMeshes.cpp
  ${SRC_DIR}/Contacts.cpp
  ${SRC_DIR}/Definitions.cpp
  ${SRC_DIR}/Entities.cpp
  ${SRC_DIR}/FlipEdges.cpp
  ${SRC_DIR}/Hessian.cpp
  ${SRC_DIR}/LandBoundaries.cpp
  ${SRC_DIR}/LandBoundary.cpp
  ${SRC_DIR}/Mesh.cpp
  ${SRC_DIR}/MeshEdgeLength.cpp
  ${SRC_DIR}/Mesh1D.cpp
  ${SRC_DIR}/Mesh2D.cpp
  ${SRC_DIR}/Mesh2DGenerateGlobal.cpp
  ${SRC_DIR}/Mesh2DIntersections.cpp
  ${SRC_DIR}/Mesh2DToCurvilinear.cpp
  ${SRC_DIR}/MeshEdgeCenters.cpp
  ${SRC_DIR}/MeshFaceCenters.cpp
  ${SRC_DIR}/MeshOrthogonality.cpp
  ${SRC_DIR}/MeshRefinement.cpp
  ${SRC_DIR}/MeshSmoothness.cpp
  ${SRC_DIR}/MeshTriangulation.cpp
  ${SRC_DIR}/Network1D.cpp
  ${SRC_DIR}/Operations.cpp
  ${SRC_DIR}/OrthogonalizationAndSmoothing.cpp
  ${SRC_DIR}/Orthogonalizer.cpp
  ${SRC_DIR}/Polygon.cpp
  ${SRC_DIR}/PolygonalEnclosure.cpp
  ${SRC_DIR}/Polygons.cpp
  ${SRC_DIR}/RemoveDisconnectedRegions.cpp
  ${SRC_DIR}/SampleAveragingInterpolator.cpp
  ${SRC_DIR}/SampleInterpolator.cpp
  ${SRC_DIR}/SampleTriangulationInterpolator.cpp
  ${SRC_DIR}/SamplesHessianCalculator.cpp
  ${SRC_DIR}/SampleAveragingInterpolator.cpp
  ${SRC_DIR}/SampleInterpolator.cpp
  ${SRC_DIR}/SampleTriangulationInterpolator.cpp
  ${SRC_DIR}/Smoother.cpp
  ${SRC_DIR}/SplineAlgorithms.cpp
  ${SRC_DIR}/Splines.cpp
  ${SRC_DIR}/SplitRowColumnOfMesh.cpp
  ${SRC_DIR}/TriangulationInterpolation.cpp
  ${SRC_DIR}/TriangulationWrapper.cpp
)

set(
  UNDO_SRC_LIST
  ${UNDO_SRC_DIR}/AddEdgeAction.cpp
  ${UNDO_SRC_DIR}/AddNodeAction.cpp
  ${UNDO_SRC_DIR}/CompoundUndoAction.cpp
  ${UNDO_SRC_DIR}/DeleteEdgeAction.cpp
  ${UNDO_SRC_DIR}/DeleteNodeAction.cpp
  ${UNDO_SRC_DIR}/FullUnstructuredGridUndo.cpp
  ${UNDO_SRC_DIR}/MeshConversionAction.cpp
  ${UNDO_SRC_DIR}/NoActionUndo.cpp
  ${UNDO_SRC_DIR}/NodeTranslationAction.cpp
  ${UNDO_SRC_DIR}/ResetEdgeAction.cpp
  ${UNDO_SRC_DIR}/ResetNodeAction.cpp
  ${UNDO_SRC_DIR}/SphericalCoordinatesOffsetAction.cpp
  ${UNDO_SRC_DIR}/UndoAction.cpp
  ${UNDO_SRC_DIR}/UndoActionStack.cpp
)

set(
  UTILITIES_SRC_LIST
  ${UTILITIES_SRC_DIR}/Utilities.cpp
  ${UTILITIES_SRC_DIR}/RTreeFactory.cpp
  ${UTILITIES_SRC_DIR}/RTreeSphericalToCartesian.cpp
)

set(
  AVERAGING_STRATEGIES_SRC_LIST
  ${AVERAGING_STRATEGIES_SRC_DIR}/AveragingStrategyFactory.cpp
  ${AVERAGING_STRATEGIES_SRC_DIR}/ClosestAveragingStrategy.cpp
  ${AVERAGING_STRATEGIES_SRC_DIR}/InverseWeightedAveragingStrategy.cpp
  ${AVERAGING_STRATEGIES_SRC_DIR}/MaxAveragingStrategy.cpp
  ${AVERAGING_STRATEGIES_SRC_DIR}/MinAbsAveragingStrategy.cpp
  ${AVERAGING_STRATEGIES_SRC_DIR}/MinAveragingStrategy.cpp
  ${AVERAGING_STRATEGIES_SRC_DIR}/SimpleAveragingStrategy.cpp
)

set(
  CURVILINEAR_GRID_SRC_LIST
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGrid.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridAlgorithm.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridBlock.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridCurvature.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridDeRefinement.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridDeleteExterior.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridDeleteInterior.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridFromPolygon.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridFromSplines.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridFromSplinesTransfinite.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridGenerateCircularGrid.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridLine.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridLineAttractionRepulsion.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridLineMirror.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridLineShift.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridMeshExpansionCalculator.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridOrthogonalization.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridRectangular.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridRefinement.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridFullRefinement.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridSmoothing.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridSmoothness.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridSnapGridToLandBoundary.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridSnapGridToSpline.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridSnapping.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridSplineToGrid.cpp
  ${CURVILINEAR_GRID_SRC_DIR}/CurvilinearGridUtilities.cpp
)


set(
  CURVILINEAR_GRID_UNDO_ACTION_SRC_LIST
  ${CURVILINEAR_UNDO_SRC_DIR}/AddGridLineUndoAction.cpp
  ${CURVILINEAR_UNDO_SRC_DIR}/CurvilinearGridBlockUndoAction.cpp
  ${CURVILINEAR_UNDO_SRC_DIR}/CurvilinearGridRefinementUndoAction.cpp
  ${CURVILINEAR_UNDO_SRC_DIR}/ResetCurvilinearNodeAction.cpp
)

# list of target headers
set(
  INC_LIST
  ${DOMAIN_INC_DIR}/AveragingInterpolation.hpp
  ${DOMAIN_INC_DIR}/BilinearInterpolationOnGriddedSamples.hpp
  ${DOMAIN_INC_DIR}/BoundingBox.hpp
  ${DOMAIN_INC_DIR}/Cartesian3DPoint.hpp
  ${DOMAIN_INC_DIR}/CasulliDeRefinement.hpp
  ${DOMAIN_INC_DIR}/CasulliRefinement.hpp
  ${DOMAIN_INC_DIR}/ConnectMeshes.hpp
  ${DOMAIN_INC_DIR}/Constants.hpp
  ${DOMAIN_INC_DIR}/Contacts.hpp
  ${DOMAIN_INC_DIR}/Definitions.hpp
  ${DOMAIN_INC_DIR}/Entities.hpp
  ${DOMAIN_INC_DIR}/Exceptions.hpp
  ${DOMAIN_INC_DIR}/FlipEdges.hpp
  ${DOMAIN_INC_DIR}/Formatting.hpp
  ${DOMAIN_INC_DIR}/Hessian.hpp
  ${DOMAIN_INC_DIR}/LandBoundaries.hpp
  ${DOMAIN_INC_DIR}/LandBoundary.hpp
  ${DOMAIN_INC_DIR}/Mesh.hpp
  ${DOMAIN_INC_DIR}/MeshEdgeLength.hpp
  ${DOMAIN_INC_DIR}/Mesh1D.hpp
  ${DOMAIN_INC_DIR}/Mesh2D.hpp
  ${DOMAIN_INC_DIR}/Mesh2DGenerateGlobal.hpp
  ${DOMAIN_INC_DIR}/Mesh2DIntersections.hpp
  ${DOMAIN_INC_DIR}/Mesh2DToCurvilinear.hpp
  ${DOMAIN_INC_DIR}/MeshFaceCenters.hpp
  ${DOMAIN_INC_DIR}/MeshConversion.hpp
  ${DOMAIN_INC_DIR}/MeshEdgeCenters.hpp
  ${DOMAIN_INC_DIR}/MeshInterpolation.hpp
  ${DOMAIN_INC_DIR}/MeshOrthogonality.hpp
  ${DOMAIN_INC_DIR}/MeshRefinement.hpp
  ${DOMAIN_INC_DIR}/MeshSmoothness.hpp
  ${DOMAIN_INC_DIR}/MeshTransformation.hpp
  ${DOMAIN_INC_DIR}/MeshTriangulation.hpp
  ${DOMAIN_INC_DIR}/Network1D.hpp
  ${DOMAIN_INC_DIR}/Operations.hpp
  ${DOMAIN_INC_DIR}/OrthogonalizationAndSmoothing.hpp
  ${DOMAIN_INC_DIR}/Orthogonalizer.hpp
  ${DOMAIN_INC_DIR}/Parameters.hpp
  ${DOMAIN_INC_DIR}/Point.hpp
  ${DOMAIN_INC_DIR}/Polygon.hpp
  ${DOMAIN_INC_DIR}/PolygonalEnclosure.hpp
  ${DOMAIN_INC_DIR}/Polygons.hpp
  ${DOMAIN_INC_DIR}/ProjectionConversions.hpp
  ${DOMAIN_INC_DIR}/RangeCheck.hpp
  ${DOMAIN_INC_DIR}/RemoveDisconnectedRegions.hpp
  ${DOMAIN_INC_DIR}/SampleAveragingInterpolator.hpp
  ${DOMAIN_INC_DIR}/SampleInterpolator.hpp
  ${DOMAIN_INC_DIR}/SampleTriangulationInterpolator.hpp
  ${DOMAIN_INC_DIR}/SamplesHessianCalculator.hpp
  ${DOMAIN_INC_DIR}/SampleAveragingInterpolator.hpp
  ${DOMAIN_INC_DIR}/SampleInterpolator.hpp
  ${DOMAIN_INC_DIR}/SampleTriangulationInterpolator.hpp
  ${DOMAIN_INC_DIR}/Smoother.hpp
  ${DOMAIN_INC_DIR}/SplineAlgorithms.hpp
  ${DOMAIN_INC_DIR}/Splines.hpp
  ${DOMAIN_INC_DIR}/SplitRowColumnOfMesh.hpp
  ${DOMAIN_INC_DIR}/TriangulationInterpolation.hpp
  ${DOMAIN_INC_DIR}/TriangulationWrapper.hpp
  ${DOMAIN_INC_DIR}/Vector.hpp
)

set(
  UNDO_INC_LIST
  ${UNDO_INC_DIR}/AddEdgeAction.hpp
  ${UNDO_INC_DIR}/AddNodeAction.hpp
  ${UNDO_INC_DIR}/BaseMeshUndoAction.hpp
  ${UNDO_INC_DIR}/DeleteEdgeAction.hpp
  ${UNDO_INC_DIR}/DeleteNodeAction.hpp
  ${UNDO_INC_DIR}/FullUnstructuredGridUndo.hpp
  ${UNDO_INC_DIR}/MeshConversionAction.hpp
  ${UNDO_INC_DIR}/NoActionUndo.hpp
  ${UNDO_INC_DIR}/NodeTranslationAction.hpp
  ${UNDO_INC_DIR}/ResetEdgeAction.hpp
  ${UNDO_INC_DIR}/ResetNodeAction.hpp
  ${UNDO_INC_DIR}/SphericalCoordinatesOffsetAction.hpp
  ${UNDO_INC_DIR}/UndoActionStack.hpp
  ${UNDO_INC_DIR}/CompoundUndoAction.hpp
  ${UNDO_INC_DIR}/UndoAction.hpp
)


set(
  AVERAGING_STRATEGIES_INC_LIST
  ${AVERAGING_STRATEGIES_INC_DIR}/AveragingStrategy.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/AveragingStrategyFactory.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/ClosestAveragingStrategy.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/InverseWeightedAveragingStrategy.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/MaxAveragingStrategy.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/MinAbsAveragingStrategy.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/MinAveragingStrategy.hpp
  ${AVERAGING_STRATEGIES_INC_DIR}/SimpleAveragingStrategy.hpp
)

set(
  CURVILINEAR_GRID_INC_LIST
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGrid.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridAlgorithm.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridBlock.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridCurvature.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridDeRefinement.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridDeleteExterior.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridDeleteInterior.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridFromPolygon.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridFromSplines.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridFromSplinesTransfinite.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridGenerateCircularGrid.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridLine.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridLineAttractionRepulsion.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridLineMirror.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridLineShift.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridMeshExpansionCalculator.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridNodeIndices.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridOrthogonalization.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridRectangular.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridRefinement.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridFullRefinement.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridSmoothing.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridSmoothness.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridSnapGridToLandBoundary.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridSnapGridToSpline.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridSnapping.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridSplineToGrid.hpp
  ${CURVILINEAR_GRID_INC_DIR}/CurvilinearGridUtilities.hpp
)


set(
  CURVILINEAR_UNDO_ACTION_INC_LIST
  ${CURVILINEAR_UNDO_INC_DIR}/AddGridLineUndoAction.hpp
  ${CURVILINEAR_UNDO_INC_DIR}/CurvilinearGridBlockUndoAction.hpp
  ${CURVILINEAR_UNDO_INC_DIR}/CurvilinearGridRefinementUndoAction.hpp
  ${CURVILINEAR_UNDO_INC_DIR}/ResetCurvilinearNodeAction.hpp
)

set(
  UTILITIES_INC_LIST
  ${UTILITIES_INC_DIR}/LinearAlgebra.hpp
  ${UTILITIES_INC_DIR}/NumericFunctions.hpp
  ${UTILITIES_INC_DIR}/RTree.hpp
  ${UTILITIES_INC_DIR}/RTreeBase.hpp
  ${UTILITIES_INC_DIR}/RTreeFactory.hpp
  ${UTILITIES_INC_DIR}/RTreeSphericalToCartesian.hpp
  ${UTILITIES_INC_DIR}/Utilities.hpp
)

# add sources to target
target_sources(
  ${TARGET_NAME}
  PRIVATE
    ${SRC_LIST}
    ${UNDO_SRC_LIST}
    ${UTILITIES_SRC_LIST}
    ${AVERAGING_STRATEGIES_SRC_LIST}
    ${CURVILINEAR_GRID_SRC_LIST}
    ${CURVILINEAR_GRID_UNDO_ACTION_SRC_LIST}
    ${UTILITIES_SRC_LIST}
    PUBLIC
    FILE_SET HEADERS
      BASE_DIRS
        ${INC_DIR}
      FILES
        ${INC_LIST}
        ${UNDO_INC_LIST}
        ${AVERAGING_STRATEGIES_INC_LIST}
        ${CURVILINEAR_GRID_INC_LIST}
        ${UTILITIES_INC_LIST}
)

# Expose the interface of the static lib
target_include_directories(${TARGET_NAME} PUBLIC ${INC_DIR})

# Add target link dependencies

# Boost
find_package(Boost REQUIRED)
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC ${Boost_INCLUDE_DIR})

# OpenMP
find_package(OpenMP REQUIRED)

# link required libraries
target_link_libraries(
  ${TARGET_NAME}
  LINK_PUBLIC
    Triangle
    OpenMP::OpenMP_CXX
    Eigen3::Eigen
)

if(${USE_LIBFMT})
  target_link_libraries(${TARGET_NAME} PUBLIC fmt::fmt)
endif()

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

# Add precompiled headers to speed up compilation
target_precompile_headers(
  ${TARGET_NAME}
  PRIVATE
    <algorithm>
    <array>
    <cassert>
    <cmath>
    <concepts>
    <initializer_list>
    <limits>
    <map>
    <memory>
    <numeric>
    <source_location>
    <stdexcept>
    <sstream>
    <string>
    <string_view>
    <tuple>
    <type_traits>
    <unordered_map>
    <utility>
    <vector>
)

install(TARGETS ${TARGET_NAME} FILE_SET HEADERS DESTINATION "include")

# group the sources in IDE tree
source_group("Source Files" FILES ${SRC_LIST})
source_group("Source Files/AveragingStrategies" FILES ${AVERAGING_STRATEGIES_SRC_LIST})
source_group("Source Files/CurvilinearGrid" FILES ${CURVILINEAR_GRID_SRC_LIST})
source_group("Source Files/CurvilinearGrid/UndoActions" FILES ${CURVILINEAR_GRID_UNDO_ACTION_SRC_LIST})
source_group("Source Files/UndoActions" FILES ${UNDO_SRC_LIST})
source_group("Source Files/Utilities" FILES ${UTILITIES_SRC_LIST})

# group the headers in IDE tree
source_group("Header Files" FILES ${INC_LIST})
source_group("Header Files/AveragingStrategies" FILES ${AVERAGING_STRATEGIES_INC_LIST})
source_group("Header Files/CurvilinearGrid" FILES ${CURVILINEAR_GRID_INC_LIST})
source_group("Header Files/CurvilinearGrid/UndoActions" FILES ${CURVILINEAR_UNDO_ACTION_INC_LIST})
source_group("Header Files/UndoActions" FILES ${UNDO_INC_LIST})
source_group("Header Files/Utilities" FILES ${UTILITIES_INC_LIST})

# Add unit tests
if(ENABLE_UNIT_TESTING)
  add_subdirectory(tests)
endif()

# Add benchmarks
if(ENABLE_BENCHMARKING)
  add_subdirectory(benchmark)
endif()
