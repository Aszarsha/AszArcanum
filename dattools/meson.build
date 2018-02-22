project( 'dattools', 'cpp'
       , default_options: ['cpp_std=c++17']
       )

sources = [ 'src/main.cpp'
          , 'src/DAT1/File.hpp'
          , 'src/DAT1/File.cpp'
          , 'src/DAT1/Subfile.hpp'
          , 'src/DAT1/Subfile.cpp'
          , 'src/UI/AppWindow.hpp'
          , 'src/UI/FileTreeStore.hpp'
          , 'src/UI/FileTreeStore.cpp'
          , 'src/UI/FileTreeView.hpp'
          , 'src/UI/SubfileContentView.hpp'
          ]

dep_gtk   = dependency( 'gtkmm-3.0' )
dep_zlib  = dependency( 'zlib' )
dep_boost = dependency( 'boost', modules: ['iostreams'] )
deps = [dep_gtk, dep_zlib, dep_boost]

executable( 'dattools', sources, dependencies: deps, include_directories: include_directories( 'src' ) )
