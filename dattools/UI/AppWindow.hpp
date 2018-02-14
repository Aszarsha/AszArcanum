#ifndef ASZARCANUM_DATTOOLS_UI_APPWINDOW_HPP
#define ASZARCANUM_DATTOOLS_UI_APPWINDOW_HPP

#include <string>

#include <boost/iostreams/device/mapped_file.hpp>

#include <gtkmm.h>

#include "../datFile.hpp"
#include "FileTreeView.hpp"
#include "SubfileContentView.hpp"

namespace AszArcanum::dattools::ui {

class AppWindow
	: public Gtk::ApplicationWindow {
	public:
		AppWindow( Glib::RefPtr< Gtk::Application > const & app )
			: Gtk::ApplicationWindow( app )
			, treeView( treeStore ) {
				set_title( "AszArcanum dattools"  );
				set_default_size( 600, 800 );

				gtkHPaned.pack1( treeView );
				gtkHPaned.pack2( subfileContentView );

				add( gtkHPaned );
				show_all_children();
		}

		void LoadFile( string_view fName ) {
				fileName = fName;
				cout << "Loading \"" << fileName << '\"' << endl;
				try {
						memMappedFile.open( fileName );
						handle_mmap_file( reinterpret_cast< byte const * >( memMappedFile.data() ), memMappedFile.size()
						                , [this]( DatSubfileIndex const & index ) {
						                  		treeStore.AppendIndex( index );
						                  }
						                );
				} catch ( char const * s ) {
						cerr << "error: " << s << endl;
						throw;
				} catch ( ... ) {
						cerr << "error: could not open file" << endl;
						throw;
				}
				cout << "File loaded" << endl;
				set_title( "AszArcanum dattools (" + string( fileName ) + ")"  );

		}

	private:
		std::string fileName;
		boost::iostreams::mapped_file_source memMappedFile;

		FileTreeStore treeStore;

		Gtk::HPaned gtkHPaned;
		FileTreeView treeView;
		SubfileContentView subfileContentView;
};

}

#endif
