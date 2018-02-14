#include <cstdlib>

#include <optional>
#include <ostream>
#include <iostream>
#include <string>
#include <string_view>
#include <set>

#include <boost/iostreams/device/mapped_file.hpp>

#include <gtkmm.h>

#include "datFile.hpp"

using namespace std;

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


struct DatFileTree {
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

				Gtk::TreeModelColumn< string >   name;
				Gtk::TreeModelColumn< uint32_t > unknown0;
				Gtk::TreeModelColumn< uint32_t > type;
				Gtk::TreeModelColumn< uint32_t > realSize;
				Gtk::TreeModelColumn< uint32_t > packedSize;
				Gtk::TreeModelColumn< uint32_t > offset;
		};

	public:
		explicit DatFileTree( string_view fName )
			: fileName( fName )
			, memMappedFile( fileName )
			, root( "ROOT", nullopt )
			, gtkTreeStore( Gtk::TreeStore::create( gtkRecordModel ) ) {
				cout << "Loading \"" << fileName << '\"' << endl;
				try {
						handle_mmap_file( reinterpret_cast< byte const * >( memMappedFile.data() ), memMappedFile.size()
						                , [this]( DatSubfileIndex const & index ) {
						                  		AppendIndex( index );
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
		}

	public:
		string       & FileName()       {   return fileName;   }
		string const & FileName() const {   return fileName;   }

		DatSubfileRecordModel       & GtkRecordModel()       {   return gtkRecordModel;   }
		DatSubfileRecordModel const & GtkRecordModel() const {   return gtkRecordModel;   }

		Glib::RefPtr< Gtk::TreeStore > const & GtkTreeStore() const {   return gtkTreeStore;   }

		void AppendIndex( DatSubfileIndex const & index ) {
				TouchFile( root, index );
		}

	private:
		struct DatTreeNode {
			public:
				struct NameComparatorFunctor {
						using is_transparent = void;
						bool operator()( DatTreeNode const & a, DatTreeNode const & b ) const {   return a.name < b.name;   }
						bool operator()( DatTreeNode const & a, string_view s ) const {   return a.name < s;   }
						bool operator()( string_view s, DatTreeNode const & a ) const {   return s < a.name;   }
				};

			public:
				string name;

				optional< Gtk::TreeModel::iterator > gtkIt;
				//^ empty if this-node is root, since root doesn't correspond to any children in gtkTree

				mutable optional< DatSubfileIndex > fileIndex;
				//^ empty if dir not specified in dat file but node part of another file path

				mutable set< DatTreeNode, NameComparatorFunctor > children;
				//^ empty if type != directory, or directory in dat file but no child (wtf file, go home)

			public:
				DatTreeNode( string_view n, optional< Gtk::TreeModel::iterator > const & it )
					: name( n )
					, gtkIt( it ) {
				}

				DatTreeNode( DatTreeNode const & ) = delete;
				DatTreeNode & operator=( DatTreeNode const & ) = delete;

			public:
				void SetIndexAttributes( DatSubfileIndex const & index ) const {
						fileIndex = index;
				}

				DatTreeNode const & CreateChild( string_view n, optional< Gtk::TreeModel::iterator > const & it ) const {
						if ( fileIndex && fileIndex->data.type != to_underlying( DatSubfileIndex::Type::Directory ) ) {
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
		string fileName;
		boost::iostreams::mapped_file_source memMappedFile;

		DatTreeNode root;

		DatSubfileRecordModel gtkRecordModel;
		Glib::RefPtr< Gtk::TreeStore > gtkTreeStore;

	private:
		vector< string_view > Tokenize( string_view path ) {
				vector< string_view > tokens;
				auto curPath = path;
				while ( true ) {
						size_t bsPos = curPath.find_first_of( '\\' );
						if ( bsPos != string_view::npos ) {
								tokens.push_back( curPath.substr( 0, bsPos ) );
								curPath = curPath.substr( bsPos+1 );
						} else {
								tokens.push_back( curPath );
								break;
						}
				}
				return tokens;
		}

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

		DatTreeNode const & TouchFile( DatTreeNode const & in, DatSubfileIndex const & index ) {
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

class DattoolsTreeWindow
	: public Gtk::Window {
	public:
		DattoolsTreeWindow( DatFileTree const & tree ) {
				set_title( string( "Dattools: " ) + tree.FileName() );
				set_default_size( 800, 600 );

				treeView.set_model( tree.GtkTreeStore() );
				treeView.append_column( "File name"      , tree.GtkRecordModel().name );
				treeView.append_column( "Unknown0"       , tree.GtkRecordModel().unknown0 );
				treeView.append_column( "Type"           , tree.GtkRecordModel().type );
				treeView.append_column( "Size"           , tree.GtkRecordModel().realSize );
				treeView.append_column( "Compressed Size", tree.GtkRecordModel().packedSize );
				treeView.append_column( "Offset"         , tree.GtkRecordModel().offset );

				scrolledWindow.add( treeView );
				scrolledWindow.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

				add( scrolledWindow );
				show_all_children();
		}

		virtual ~DattoolsTreeWindow() = default;

	private:
		Gtk::TreeView treeView;
		Gtk::ScrolledWindow scrolledWindow;
};

int main( int argc, char * argv[] ) {
		if ( argc != 2 ) {
				Usage( "Usage: ", argv[0], " <dat file>\n" );
		}

		auto app = Gtk::Application::create( "asz.dattools" );

		DatFileTree datFile( argv[1] );
		DattoolsTreeWindow win( datFile );

		return app->run( win );
}
