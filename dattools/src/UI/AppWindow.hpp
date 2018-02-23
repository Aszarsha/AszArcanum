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

				treeView.signal_TreeSelection().connect( sigc::mem_fun( *this, &AppWindow::FileSelected ) );

				add( gtkHPaned );
				show_all_children();
		}

		virtual ~AppWindow() = default;

		void LoadFile( std::string_view fileName ) {
				file = DAT1::File::LoadFrom( fileName );
				file->ForEachSubfile( [this]( DAT1::Subfile const & sf ) {
						treeStore.AddSubfile( sf );
				});
				set_title( "AszArcanum dattools (" + std::string( fileName ) + ")"  );
		}

		void FileSelected( Gtk::TreeModel::Path const & path ) {
				auto sfPtr = treeStore.GetSubfileFrom( path );
				if ( sfPtr ) {
						subfileContentView.Show( *sfPtr );
				}
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

} // namespace AszArcanum::dattools::UI

#endif
