#ifndef ASZARCANUM_DATTOOLS_DATFILE_HPP
#define ASZARCANUM_DATTOOLS_DATFILE_HPP

#include <cstddef>
#include <cstdint>

#include <ostream>
#include <iostream>
#include <vector>
#include <string>

#include <zlib.h>

using namespace std;

template< typename Enum >
constexpr auto to_underlying( Enum e ) noexcept {
    return static_cast< std::underlying_type_t< Enum > >( e );
}

struct DatFileFooter {
		byte     unknown0[16];
		uint32_t dat1Tag;
		uint32_t unknown1;
		uint32_t treeDescOffset;
};

DatFileFooter read_footer( byte const * data, size_t size ) {
		DatFileFooter footer;
		//cout << "Reading footer" << endl;
		memcpy( &footer, data + size - sizeof( footer ), sizeof( footer ) );
		cout << "Footer content, unknown0: '''" << string( reinterpret_cast< char const * >( footer.unknown0 ), 16 ) << "'''" << endl
		     << "                 dat1Tag: " << string( reinterpret_cast< char const * >( &footer.dat1Tag ), 4 ) << endl
		     << "                unknown1: " << footer.unknown1 << endl
		     << "          treeDescOffset: " << footer.treeDescOffset << endl;
		return footer;
}

namespace {
	inline void read_bytes_from_data_ptr( byte const* & data, byte * to, size_t sz ) {
			memcpy( to, data, sz );
			data += sz;
	}

	template< typename Type >
	inline Type read_type_from_data_ptr( byte const* & data ) {
			Type out;
			read_bytes_from_data_ptr( data, reinterpret_cast< byte * >( &out ), sizeof( out ) );
			return out;
	}
}

struct DatSubfileIndex {
		enum class Type: uint32_t { Raw            = 0x001
		                          , ZlibCompressed = 0x002
		                          , Directory      = 0x400
		                          };

		string name;
		struct {
				uint32_t unknown0;
				uint32_t type;
				uint32_t realSize;
				uint32_t packedSize;
				uint32_t offset;
		} data;
};
using DatSubfilesIndices = vector< DatSubfileIndex >;

DatSubfilesIndices read_file_indices( byte const * indicesData ) {
		cout << "Reading file indices" << endl;
		uint32_t numIndices = read_type_from_data_ptr< uint32_t >( indicesData );
		//cout << "num indices: " << numIndices << endl;
		DatSubfilesIndices indices( numIndices );
		for ( uint32_t i = 0; i != numIndices; ++i ) {
				uint32_t filenameSz = read_type_from_data_ptr< uint32_t >( indicesData );
				indices[i].name.resize( filenameSz-1 );   // do not count the included null-termination
				read_bytes_from_data_ptr( indicesData, reinterpret_cast< byte * >( indices[i].name.data() ), filenameSz-1 );
				indicesData += 1;   // skip null-termination char
				read_bytes_from_data_ptr( indicesData, reinterpret_cast< byte * >( &indices[i].data ), sizeof( indices[i].data ) );
		}
		//cout << "ADDR0: " << indicesData << endl;
		///^ ADDR0 == ADDR1 (below) means everything is read and footer is the right size (even though there are unknown values)
		cout << "Indices read" << endl;
		return indices;
}

vector< byte > zlibDecompress( byte const * data, size_t realSize, size_t packedSize ) {
		vector< byte > outData( realSize );
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

template< typename Func >
void handle_mmap_file( byte const * data, size_t size, Func && f ) {
		//cout << "ADDR1: " << data + size - sizeof( DatFileFooter ) << endl;
		auto footer = read_footer( data, size );
		auto indices = read_file_indices( data + size - footer.treeDescOffset );
		for ( size_t i = 0, e = indices.size(); i != e; ++i ) {
				f( indices[i] );
		}
/*
		auto f0dat = zlibDecompress( data + indices[0].data.offset, indices[0].data.realSize, indices[0].data.packedSize );
		cout << "File 0 Uncompressed data" << endl;
		for ( size_t i = 0; i != f0dat.size(); ++i ) {
				cout << (char)f0dat[i];
		}
		cout << endl;
*/
}

#endif
