project( 'dattools', 'cpp'
       , default_options: ['cpp_std=c++17']
       )

dep_gtk   = dependency( 'gtkmm-3.0' )
dep_zlib  = dependency( 'zlib' )
dep_boost = dependency( 'boost', modules: ['iostreams'] )
deps = [dep_gtk, dep_zlib, dep_boost]

executable( 'dattools', ['main.cpp', 'datFile.hpp'], dependencies: deps )
