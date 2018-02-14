#ifndef ASZARCANUM_DATTOOLS_UI_FILETREEVIEW_HPP
#define ASZARCANUM_DATTOOLS_UI_FILETREEVIEW_HPP

#include <gtkmm.h>

#include "FileTreeStore.hpp"

namespace AszArcanum::dattools::ui {

class FileTreeView
	: public Gtk::Frame {
	public:
		FileTreeView( FileTreeStore const & treeStore ) {
				treeView.set_model( treeStore.GtkTreeStore() );
				treeView.append_column( "File name"      , treeStore.GtkRecordModel().name );
				treeView.append_column( "Unknown0"       , treeStore.GtkRecordModel().unknown0 );
				treeView.append_column( "Type"           , treeStore.GtkRecordModel().type );
				treeView.append_column( "Size"           , treeStore.GtkRecordModel().realSize );
				treeView.append_column( "Compressed Size", treeStore.GtkRecordModel().packedSize );
				treeView.append_column( "Offset"         , treeStore.GtkRecordModel().offset );

				scrolledWindow.add( treeView );
				scrolledWindow.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

				add( scrolledWindow );
				show_all_children();
		}

	private:
		Gtk::TreeView treeView;
		Gtk::ScrolledWindow scrolledWindow;
};

} // namespace

#endif
