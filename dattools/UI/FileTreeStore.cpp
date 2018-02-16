#include "FileTreeStore.hpp"

using namespace std;

namespace AszArcanum::dattools::UI {

namespace {
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
}

void FileTreeStore::SetGtkNameForNode( DatTreeNode const & node ) {
		if ( !node.gtkIt ) {   throw "trying to set gtk name for root... duh!";   }
		auto & row = **node.gtkIt;   //< deref optional AND iter
		row[gtkRecordModel.name] = node.name;
}

void FileTreeStore::SetGtkAttributesForNode( DatTreeNode const & node ) {
		if ( !node.gtkIt ) {   throw "trying to set gtk attributes for root... duh!";   }
		if ( !node.subfile ) {   throw "trying to set attributes from uninitialized node... duh!";   }
		auto & row = **node.gtkIt;   //< deref optional AND iter
		auto & indexData = node.subfile->GetIndex().data;

		row[gtkRecordModel.unknown0] = to_string( indexData.unknown0 );
		switch ( node.subfile->GetType() ) {
			case DAT1::Subfile::Type::Dir: {
					row[gtkRecordModel.type] = "DIR";
				} break;
			case DAT1::Subfile::Type::Raw: {
					row[gtkRecordModel.type]     = "RAW";
					row[gtkRecordModel.realSize] = to_string( indexData.realSize );
					row[gtkRecordModel.offset]   = to_string( indexData.offset );
				} break;
			case DAT1::Subfile::Type::Zlib: {
					row[gtkRecordModel.type]       = "ZLIB";
					row[gtkRecordModel.realSize]   = to_string( indexData.realSize );
					row[gtkRecordModel.packedSize] = to_string( indexData.packedSize );
					row[gtkRecordModel.offset]     = to_string( indexData.offset );
				} break;
		}
}

FileTreeStore::DatTreeNode const & FileTreeStore::TouchFile( DatTreeNode const & in, DAT1::Subfile const & subfile ) {
		DatTreeNode const * pNode = &in;
		auto tokens = Tokenize( subfile.GetPathName() );
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
		pNode->SetSubfile( subfile );
		SetGtkAttributesForNode( *pNode );
		return *pNode;
}

} // namespace
