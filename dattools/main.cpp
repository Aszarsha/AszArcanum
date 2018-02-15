#include <cstdlib>

#include <iostream>

#include "UI/AppWindow.hpp"

using namespace std;
using namespace AszArcanum::dattools;

template< typename Arg >
ostream & var_print( ostream & ostr, Arg && arg ) {
		ostr << arg;
		return ostr;
}

template< typename Arg, typename... Args >
ostream & var_print( ostream & ostr, Arg && arg, Args &&... args ) {
		ostr << arg;
		return var_print( ostr, args... );
}

template< typename... Args >
[[ noreturn ]] void Usage( Args &&... args ) {
		var_print( cerr, args... );
		exit( EXIT_FAILURE );
}

int main( int argc, char * argv[] ) {
		if ( argc != 2 ) {
				Usage( "Usage: ", argv[0], " <dat file>\n" );
		}

		auto app = Gtk::Application::create( "AszArcanum.dattools" );
		UI::AppWindow win( app );
		win.LoadFile( argv[1] );
		return app->run( win );
}
