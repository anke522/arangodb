# set cmdline directories
set(CMDLINE_INCLUDE_DIR
  ${EXTERNAL_INCLUDE_DIRS}/cmdline
)

if (MSVC)
  # disable "checked iterators" feature
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
endif()

################################################################################
### iresearch-util
################################################################################

set (IResearchUtil_TARGET_NAME
  "iresearch-index-util"
  CACHE INTERNAL
  ""
)

add_executable(${IResearchUtil_TARGET_NAME}
  ./common.cpp
  ./index-dump.cpp
  ./index-convert.cpp
  ./index-util.cpp
  ./main.cpp
)

target_include_directories(${IResearchUtil_TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}/core
)

set_target_properties(${IResearchUtil_TARGET_NAME}
  PROPERTIES
  OUTPUT_NAME iresearch-index-util
  COMPILE_DEFINITIONS "$<$<CONFIG:Debug>:IRESEARCH_DEBUG>;IRESEARCH_DLL;BOOST_ALL_DYN_LINK"
)

set_target_properties(${IResearchUtil_TARGET_NAME}
  PROPERTIES
  OUTPUT_NAME iresearch-index-util
  COMPILE_DEFINITIONS "$<$<CONFIG:Debug>:IRESEARCH_DEBUG>"
)

target_link_libraries(${IResearchUtil_TARGET_NAME}
  ${IResearch_TARGET_NAME}-static
  ${PTHREAD_LIBRARY}
)

target_compile_features(${IResearchUtil_TARGET_NAME}
  PRIVATE
  cxx_final
  cxx_variadic_templates
)

include_directories(${IResearchUtil_TARGET_NAME}
  ${IReSearch_generated_INCLUDE_DIR}
  ${IResearch_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${CMDLINE_INCLUDE_DIR}
)

################################################################################
### iresearch-benchmarks
################################################################################

set (IResearchBencmarks_TARGET_NAME
  "iresearch-benchmarks"
  CACHE INTERNAL
  ""
)

add_executable(${IResearchBencmarks_TARGET_NAME}
  ./common.cpp
  ./index-put.cpp
  ./index-search.cpp
  ./index-benchmarks.cpp
  ./main.cpp
)

add_dependencies(${IResearchBencmarks_TARGET_NAME}
  ${IResearch_TARGET_NAME}-analyzer-text-static
  ${IResearch_TARGET_NAME}-format-1_0-static
  ${IResearch_TARGET_NAME}-scorer-tfidf-static
  ${IResearch_TARGET_NAME}-scorer-bm25-static
)

target_include_directories(${IResearchBencmarks_TARGET_NAME}
  PRIVATE ${PROJECT_BINARY_DIR}/core
)

set_target_properties(${IResearchBencmarks_TARGET_NAME}
  PROPERTIES
  OUTPUT_NAME iresearch-benchmarks
  COMPILE_DEFINITIONS "$<$<CONFIG:Debug>:IRESEARCH_DEBUG>"
)

if (NOT MSVC AND GCC_VERSION VERSION_LESS 4.9)
  # GCC before 4.9 does not support std::regex
  set(REGEX_LIBRARY ${Boost_STATIC_sharedRT_REGEX_LIBRARY_RELEASE})
endif()

target_link_libraries(${IResearchBencmarks_TARGET_NAME}
  ${IResearch_TARGET_NAME}-static
  ${PTHREAD_LIBRARY}
  ${REGEX_LIBRARY}
)

target_compile_features(${IResearchBencmarks_TARGET_NAME}
  PRIVATE
  cxx_final
  cxx_variadic_templates
)

include_directories(${IResearchBencmarks_TARGET_NAME}
  ${IResearch_INCLUDE_DIR}
  ${ICU_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${CMDLINE_INCLUDE_DIR}
)
