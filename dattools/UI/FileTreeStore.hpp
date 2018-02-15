#ifndef ASZARCANUM_DATTOOLS_UI_FILETREESTORE_HPP
#define ASZARCANUM_DATTOOLS_UI_FILETREESTORE_HPP

#include <cstdint>

#include <optional>
#include <set>
#include <string>
#include <string_view>

#include <gtkmm.h>

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

				Gtk::TreeModelColumn< std::string >   name;
				Gtk::TreeModelColumn< uint32_t > unknown0;
				Gtk::TreeModelColumn< DAT1::File::SubfileIndex::Type > type;
				Gtk::TreeModelColumn< uint32_t > realSize;
				Gtk::TreeModelColumn< uint32_t > packedSize;
				Gtk::TreeModelColumn< uint32_t > offset;
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

		void AppendIndex( DAT1::File::SubfileIndex const & index ) {
				TouchFile( root, index );
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

				mutable std::optional< DAT1::File::SubfileIndex > fileIndex;
				//^ empty if dir not specified in dat file but node part of another file path

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
				void SetIndexAttributes( DAT1::File::SubfileIndex const & index ) const {
						fileIndex = index;
				}

				DatTreeNode const & CreateChild( std::string_view n, std::optional< Gtk::TreeModel::iterator > const & it ) const {
						if ( fileIndex && fileIndex->data.type != DAT1::File::SubfileIndex::Type::Directory ) {
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
		static std::vector< std::string_view > Tokenize( std::string_view path ) {
				std::vector< std::string_view > tokens;
				auto curPath = path;
				while ( true ) {
						size_t bsPos = curPath.find_first_of( '\\' );
						if ( bsPos != std::string_view::npos ) {
								tokens.push_back( curPath.substr( 0, bsPos ) );
								curPath = curPath.substr( bsPos+1 );
						} else {
								tokens.push_back( curPath );
								break;
						}
				}
				return tokens;
		}

	private:
		DatTreeNode root;

		DatSubfileRecordModel gtkRecordModel;
		Glib::RefPtr< Gtk::TreeStore > gtkTreeStore;

	private:
		void SetGtkNameForNode( DatTreeNode const & node ) {
				if ( !node.gtkIt ) {   throw "trying to set gtk name for root... duh!";   }
				auto & row = **node.gtkIt;   //< deref optional AND iter
				row[gtkRecordModel.name] = node.name;
		}

		void SetGtkAttributesForNode( DatTreeNode const & node ) {
				if ( !node.gtkIt ) {   throw "trying to set gtk attributes for root... duh!";   }
				if ( !node.fileIndex ) {   throw "trying to set attributes from uninitialized node... duh!";   }
				auto & row = **node.gtkIt;   //< deref optional AND iter
				row[gtkRecordModel.unknown0]   = node.fileIndex->data.unknown0;
				row[gtkRecordModel.type]       = node.fileIndex->data.type;
				row[gtkRecordModel.realSize]   = node.fileIndex->data.realSize;
				row[gtkRecordModel.packedSize] = node.fileIndex->data.packedSize;
				row[gtkRecordModel.offset]     = node.fileIndex->data.offset;
		}

		DatTreeNode const & TouchFile( DatTreeNode const & in, DAT1::File::SubfileIndex const & index ) {
				DatTreeNode const * pNode = &in;
				auto tokens = Tokenize( index.name );
				for ( size_t i = 0, e = tokens.size(); i != e; ++i ) {
						auto it = pNode->children.find( tokens[i] );
						if ( it != pNode->children.end() ) {
								pNode = &*it;
						} else {
								Gtk::TreeModel::iterator newRowIt;
								if ( pNode->gtkIt ) {
										auto oldRow = **pNode->gtkIt;   //< deref optional AND iter
										newRowIt = gtkTreeStore->append( oldRow.children() );
								} else {
										newRowIt = gtkTreeStore->append();
								}
								auto & newNode = pNode->CreateChild( tokens[i], newRowIt );
								SetGtkNameForNode( newNode );
								pNode = &newNode;
						}
				}
				pNode->SetIndexAttributes( index );
				SetGtkAttributesForNode( *pNode );
				return *pNode;
		}
};

} // namespace

#endif
