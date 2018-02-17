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
		AppWindow()
			:  treeView( treeStore ) {
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
				file->ForEachSubfile( [this]( DAT1::Subfile const & sf ) {
						static int i = 0;
						if ( i++ == 0 ) {
								subfileContentView.Show( sf );
						}
						treeStore.AddSubfile( sf );
				});
				set_title( "AszArcanum dattools (" + std::string( fileName ) + ")"  );
		}

	private:
		std::optional< DAT1::File > file;

		FileTreeStore treeStore;

		Gtk::HPaned gtkHPaned;
		FileTreeView treeView;
		SubfileContentView subfileContentView;
};

class DattoolsApp
	: public Gtk::Application {
	public:
		DattoolsApp()
			: Gtk::Application( "AszArcanum.dattools", Gio::APPLICATION_HANDLES_OPEN ) {
		}

	public:
		void LoadFile( std::string_view fileName ) {
				appWindow.LoadFile( fileName );
		}

	protected:
		void on_activate() override {
				add_window( appWindow );
				appWindow.present();
		}

	private:
		AppWindow appWindow;
};

} // namespace

#endif
