# Follows the Effective CMake recommandation
# https://www.youtube.com/watch?v=bsXLMQ6WgIk
cmake_minimum_required( VERSION 3.8 )
#project( dattools )

### GLOBAL ###
if ( MSVC )
	add_compile_options( /W3 )
else()
	add_compile_options( -W -Wall -Wextra )
endif()

set( CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*,-objc*,-google*,-llvm*,-fuschsia*,-clang-diagnostic-unused-command-line-argument" )

### dattools target ###
add_executable( dattools
	src/main.cpp
	src/DAT1/File.hpp
	src/DAT1/File.cpp
	src/DAT1/Subfile.hpp
	src/DAT1/Subfile.cpp
	src/UI/AppWindow.hpp
	src/UI/FileTreeStore.hpp
	src/UI/FileTreeStore.cpp
	src/UI/FileTreeView.hpp
	src/UI/SubfileContentView.hpp
)
target_include_directories( dattools
	PRIVATE
		src/
)

find_package( Boost REQUIRED COMPONENTS iostreams )
find_package( ZLIB REQUIRED )

target_link_libraries( dattools
	PRIVATE
		Boost::iostreams
		ZLIB::ZLIB
)
target_compile_features( dattools
	PRIVATE
		cxx_std_17
)

# Ideally: find_package( gtkmm-3.0 REQUIRED )
#[--
find_package( PkgConfig REQUIRED )
pkg_check_modules( GTKMM3 REQUIRED gtkmm-3.0 )
link_directories( ${GTKMM3_LIBRARY_DIRS} )
add_definitions( ${GTKMM3_CFLAGS_OTHER} )
target_link_libraries( dattools PRIVATE ${GTKMM3_LIBRARIES} )
target_include_directories( dattools PRIVATE ${GTKMM3_INCLUDE_DIRS} )
#--]

set_target_properties( dattools PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)
