#ifndef ASZARCANUM_DATTOOLS_UI_FILETREESTORE_HPP
#define ASZARCANUM_DATTOOLS_UI_FILETREESTORE_HPP

#include <cstdint>

#include <optional>
#include <set>
#include <string>
#include <string_view>

#include <gtkmm.h>

#include "DAT1/Subfile.hpp"

namespace AszArcanum::dattools::UI {

struct FileTreeStore {
	public:
		class DatSubfileRecordModel
			: public Gtk::TreeModel::ColumnRecord {
			public:
				DatSubfileRecordModel() {
						add( name );
						add( unknown0 );
						add( type );
						add( realSize );
						add( packedSize );
						add( offset );
				}

				Gtk::TreeModelColumn< std::string > name;
				Gtk::TreeModelColumn< std::string > unknown0;
				Gtk::TreeModelColumn< std::string > type;
				Gtk::TreeModelColumn< std::string > realSize;
				Gtk::TreeModelColumn< std::string > packedSize;
				Gtk::TreeModelColumn< std::string > offset;
		};

	public:
		FileTreeStore()
			: root( "ROOT", std::nullopt )
			, gtkTreeStore( Gtk::TreeStore::create( gtkRecordModel ) ) {
		}

		virtual ~FileTreeStore() = default;

	public:
		DatSubfileRecordModel       & GtkRecordModel()       {   return gtkRecordModel;   }
		DatSubfileRecordModel const & GtkRecordModel() const {   return gtkRecordModel;   }

		Glib::RefPtr< Gtk::TreeStore > const & GtkTreeStore() const {   return gtkTreeStore;   }

		void AddSubfile( DAT1::Subfile const & subfile ) {
				TouchFile( root, subfile );
		}

	private:
		struct DatTreeNode {
			public:
				struct NameComparatorFunctor {
						using is_transparent = void;
						bool operator()( DatTreeNode const & a, DatTreeNode const & b ) const {   return a.name < b.name;   }
						bool operator()( DatTreeNode const & a, std::string_view s ) const {   return a.name < s;   }
						bool operator()( std::string_view s, DatTreeNode const & a ) const {   return s < a.name;   }
				};

			public:
				std::string name;

				std::optional< Gtk::TreeModel::iterator > gtkIt;
				//^ empty if this-node is root, since root doesn't correspond to any children in gtkTree

				mutable DAT1::Subfile const * subfile = nullptr;
				//^ null if dir not specified in dat file but node part of another file path

				mutable std::set< DatTreeNode, NameComparatorFunctor > children;
				//^ empty if type != directory, or directory in dat file but no child (wtf file, go home)

			public:
				DatTreeNode( std::string_view n, std::optional< Gtk::TreeModel::iterator > const & it )
					: name( n )
					, gtkIt( it ) {
				}

				DatTreeNode( DatTreeNode const & ) = delete;
				DatTreeNode & operator=( DatTreeNode const & ) = delete;

			public:
				void SetSubfile( DAT1::Subfile const & sf ) const {
						subfile = &sf;
				}

				DatTreeNode const & CreateChild( std::string_view n, std::optional< Gtk::TreeModel::iterator > const & it ) const {
						if ( subfile && subfile->GetType() != DAT1::Subfile::Type::Dir ) {
								throw "cannot create a child node inside a non-directory node";
						}
						auto p = children.emplace( n, it );
						if ( !p.second ) {
								throw "node already exists";
						} else {
								return *p.first;
						}
				}
		};

	private:
		DatTreeNode root;

		DatSubfileRecordModel gtkRecordModel;
		Glib::RefPtr< Gtk::TreeStore > gtkTreeStore;

	private:
		void SetGtkNameForNode( DatTreeNode const & node );
		void SetGtkAttributesForNode( DatTreeNode const & node );

		DatTreeNode const & TouchFile( DatTreeNode const & in, DAT1::Subfile const & subfile );
};

} // namespace

#endif
