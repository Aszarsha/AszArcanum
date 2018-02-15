#ifndef ASZARCANUM_DATTOOLS_UI_APPWINDOW_HPP
#define ASZARCANUM_DATTOOLS_UI_APPWINDOW_HPP

#include <string>
#include <string_view>

#include <gtkmm.h>

#include "DAT1/File.hpp"
#include "FileTreeView.hpp"
#include "SubfileContentView.hpp"

namespace AszArcanum::dattools::UI {

class AppWindow
	: public Gtk::ApplicationWindow {
	public:
		AppWindow( Glib::RefPtr< Gtk::Application > const & app )
			: Gtk::ApplicationWindow( app )
			, treeView( treeStore ) {
				set_title( "AszArcanum dattools"  );
				set_default_size( 1000, 800 );

				gtkHPaned.pack1( treeView );
				gtkHPaned.pack2( subfileContentView );

				add( gtkHPaned );
				show_all_children();
		}

		virtual ~AppWindow() = default;

		void LoadFile( std::string_view fileName ) {
				file = DAT1::File::LoadFrom( fileName );
				file->ForEachSubfile( [this]( DAT1::File::SubfileIndex const & index ) {   treeStore.AppendIndex( index );   } );
				set_title( "AszArcanum dattools (" + std::string( fileName ) + ")"  );
		}

	private:
		std::optional< DAT1::File > file;

		FileTreeStore treeStore;

		Gtk::HPaned gtkHPaned;
		FileTreeView treeView;
		SubfileContentView subfileContentView;
};

} // namespace

#endif
