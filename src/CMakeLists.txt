#-----------------------------------------------------------------------------
#
#  CMake Config
#
#-----------------------------------------------------------------------------

add_executable(check_changeset_timestamps check_changeset_timestamps.cpp)
target_link_libraries(check_changeset_timestamps ${OSMIUM_IO_LIBRARIES})
install(TARGETS check_changeset_timestamps DESTINATION bin)

