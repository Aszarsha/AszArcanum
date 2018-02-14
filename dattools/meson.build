project( 'dattools', 'cpp'
       , default_options: ['cpp_std=c++17']
       )

sources = [ 'main.cpp'
          , 'datFile.hpp'
          , 'UI/AppWindow.hpp'
          , 'UI/FileTreeStore.hpp'
          , 'UI/FileTreeView.hpp'
          , 'UI/SubfileContentView.hpp'
          ]

dep_gtk   = dependency( 'gtkmm-3.0' )
dep_zlib  = dependency( 'zlib' )
dep_boost = dependency( 'boost', modules: ['iostreams'] )
deps = [dep_gtk, dep_zlib, dep_boost]

executable( 'dattools', sources, dependencies: deps )
