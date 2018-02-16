#include "Subfile.hpp"

#include <zlib.h>

using namespace std;

namespace AszArcanum::dattools::DAT1 {

// =========== Subfile =========== //
Subfile::~Subfile() {   }

Subfile::Index const & Subfile::GetIndex() const {
		return index;
}

std::string_view Subfile::GetPathName() const {
		return GetIndex().pathName;
}

Subfile::Type Subfile::GetType() const {
		return GetIndex().data.type;
}

SubfileDir & Subfile::AsDir() {
		return dynamic_cast< SubfileDir & >( *this );
};

SubfileDir const & Subfile::AsDir() const {
		return const_cast< Subfile * >( this )->AsDir();
};

SubfileFile & Subfile::AsFile() {
		return dynamic_cast< SubfileFile & >( *this );
};

SubfileFile const & Subfile::AsFile() const {
		return const_cast< Subfile * >( this )->AsFile();
};

// =========== SubfileRaw =========== //
gsl::span< std::byte const > SubfileRaw::GetData() const {
		return gsl::span{ ptrToData, GetIndex().data.realSize };
}

// =========== SubfileZlib =========== //
namespace {
	vector< std::byte > zlibInflate( std::byte const * data, size_t realSize, size_t packedSize ) {
			Expects( data != nullptr );
			Expects( realSize != 0 );

			vector< std::byte > outData( realSize );
			z_stream zstr;
			zstr.zalloc = nullptr;
			zstr.zfree  = nullptr;
			zstr.opaque = nullptr;
			zstr.next_in  = const_cast< Bytef * >( reinterpret_cast< Bytef const * >( data ) );
			zstr.avail_in = packedSize;
			zstr.next_out  = reinterpret_cast< Bytef * >( outData.data() );
			zstr.avail_out = realSize;
			if ( inflateInit( &zstr ) != Z_OK ) {
					throw "inflateInit error";
			}
			if ( inflate( &zstr, Z_FINISH ) != Z_STREAM_END ) {
					throw "whoupsies";
			}
			if ( inflateEnd( &zstr ) != Z_OK ) {
					throw "Ugh";
			}
			return outData;
	}
}

gsl::span< std::byte const > SubfileZlib::GetData() const {
		if ( inflated.empty() ) {
				inflated = zlibInflate( ptrToData, GetIndex().data.realSize, GetIndex().data.packedSize );
		}
		return gsl::span< std::byte >{ inflated };
}

} // namespace
