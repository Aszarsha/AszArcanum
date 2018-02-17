#ifndef ASZARCANUM_DATTOOLS_UI_FILETREEVIEW_HPP
#define ASZARCANUM_DATTOOLS_UI_FILETREEVIEW_HPP

#include <gtkmm.h>

#include "FileTreeStore.hpp"

namespace AszArcanum::dattools::UI {

class FileTreeView
	: public Gtk::ScrolledWindow {
	public:
		FileTreeView( FileTreeStore const & treeStore ) {
				set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

				treeView.set_model( treeStore.GtkTreeStore() );
				treeView.append_column( "File name"      , treeStore.GtkRecordModel().name );
				treeView.append_column( "Unknown0"       , treeStore.GtkRecordModel().unknown0 );
				treeView.append_column( "Type"           , treeStore.GtkRecordModel().type );
				treeView.append_column( "Size"           , treeStore.GtkRecordModel().realSize );
				treeView.append_column( "Compressed Size", treeStore.GtkRecordModel().packedSize );
				treeView.append_column( "Offset"         , treeStore.GtkRecordModel().offset );

				auto columns = treeView.get_columns();
				std::for_each( columns.begin(), columns.end(), []( auto & col ) {
						col->set_resizable( true );
						col->set_expand( false );
						col->set_min_width( 50 );
				});

				treeView.signal_row_activated().connect( sigc::mem_fun( *this, &FileTreeView::TreeSelectionSlot ) );

				add( treeView );
				show_all_children();
		}

		virtual ~FileTreeView() = default;

		using TreeSelectionSignalType = sigc::signal< void, Gtk::TreeModel::Path const & >;
		TreeSelectionSignalType signal_TreeSelection() {   return treeSelectionSignal;   }

	private:

	private:
		Gtk::TreeView treeView;

		TreeSelectionSignalType treeSelectionSignal;

	private:
		void TreeSelectionSlot( Gtk::TreeModel::Path const & path, Gtk::TreeViewColumn * ) {
				treeSelectionSignal.emit( path );
		}
};

} // namespace

#endif
