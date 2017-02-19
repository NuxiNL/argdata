cmake_minimum_required(VERSION 3.5)

project(argdata)

option(BUILD_SHARED_LIBS "Build as shared library" ON)

include(GNUInstallDirs)

add_library(argdata
	src/argdata_create_binary.c
	src/argdata_create_buffer.c
	src/argdata_create_fd.c
	src/argdata_create_float.c
	src/argdata_create_int_s.c
	src/argdata_create_int_u.c
	src/argdata_create_map.c
	src/argdata_create_seq.c
	src/argdata_create_str.c
	src/argdata_create_str_c.c
	src/argdata_create_timestamp.c
	src/argdata_false.c
	src/argdata_free.c
	src/argdata_get_binary.c
	src/argdata_get_bool.c
	src/argdata_get_buffer.c
	src/argdata_get_buffer_length.c
	src/argdata_get_fd.c
	src/argdata_get_float.c
	src/argdata_get_int_s.c
	src/argdata_get_int_u.c
	src/argdata_get_str.c
	src/argdata_get_str_c.c
	src/argdata_get_timestamp.c
	src/argdata_map_iterate.c
	src/argdata_map_next.c
	src/argdata_null.c
	src/argdata_print_yaml.c
	src/argdata_seq_iterate.c
	src/argdata_seq_next.c
	src/argdata_true.c
)

target_link_libraries(argdata m)

set_property(
	TARGET
		argdata
	PROPERTY VERSION
		"1"
)

install(
	TARGETS
		argdata
	DESTINATION
		${CMAKE_INSTALL_LIBDIR}
)

install(
	FILES
		src/argdata.h
		src/argdata.hpp
	DESTINATION
		${CMAKE_INSTALL_INCLUDEDIR}
)
