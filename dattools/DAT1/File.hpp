#ifndef ASZARCANUM_DATTOOLS_DAT1_FILE_HPP
#define ASZARCANUM_DATTOOLS_DAT1_FILE_HPP

#include <cstddef>
#include <cstdint>

#include <ostream>
#include <iostream>
#include <vector>
#include <string>

#include <zlib.h>

#include <boost/iostreams/device/mapped_file.hpp>

template< typename Enum >
constexpr auto to_underlying( Enum e ) noexcept {
    return static_cast< std::underlying_type_t< Enum > >( e );
}

namespace AszArcanum::dattools::DAT1 {

class File {
	public:
		struct Footer {
				std::byte unknown0[16];
				uint32_t  dat1Tag;
				uint32_t  unknown1;
				uint32_t  treeDescOffset;
		};

		struct SubfileIndex {
				enum class Type: uint32_t { Raw            = 0x001
				                          , ZlibCompressed = 0x002
				                          , Directory      = 0x400
				                          };

				std::string name;
				struct {
						uint32_t unknown0;
						Type     type;
						uint32_t realSize;
						uint32_t packedSize;
						uint32_t offset;
				} data;
		};

	public:
		static File LoadFrom( std::string_view fileName );

	public:
		template< typename Func >
		void ForEachSubfile( Func && f ) const {
				auto indicesData = reinterpret_cast< std::byte const * >( memMappedFile.data() + memMappedFile.size() - footer.treeDescOffset );
				uint32_t numIndices = ReadTypeFromDataPtr< uint32_t >( indicesData );
				std::cout << "num indices: " << numIndices << std::endl;
				for ( uint32_t i = 0; i != numIndices; ++i ) {
						SubfileIndex index;
						uint32_t filenameSz = ReadTypeFromDataPtr< uint32_t >( indicesData );
						index.name.resize( filenameSz-1 );   // do not count the included null-termination
						ReadBytesFromDataPtr( indicesData, reinterpret_cast< std::byte * >( &index.name[0] ), filenameSz-1 );
						indicesData += 1;   // skip null-termination char
						ReadBytesFromDataPtr( indicesData, reinterpret_cast< std::byte * >( &index.data ), sizeof( index.data ) );
						f( index );
				}
		/*
				auto f0dat = zlibDecompress( data + indices[0].data.offset, indices[0].data.realSize, indices[0].data.packedSize );
				cout << "File 0 Uncompressed data" << std::endl;
				for ( size_t i = 0; i != f0dat.size(); ++i ) {
						cout << (char)f0dat[i];
				}
				cout << std::endl;
		*/
		}

	public:
		Footer const & GetFooter() const {   return footer;   }

	private:
		using MemoryMappedFile = boost::iostreams::mapped_file_source;

	private:
		static Footer ReadFooter( std::byte const * data, size_t size );

		static inline void ReadBytesFromDataPtr( std::byte const* & data, std::byte * to, size_t sz ) {
				memcpy( to, data, sz );
				data += sz;
		}

		template< typename Type >
		static inline Type ReadTypeFromDataPtr( std::byte const* & data ) {
				Type out;
				ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &out ), sizeof( out ) );
				return out;
		}

	private:
		std::string fileName;
		MemoryMappedFile memMappedFile;

		Footer footer;

	private:
		File( std::string_view fileName, MemoryMappedFile && memMappedFile );
};

/*
vector< std::byte > zlibDecompress( std::byte const * data, size_t realSize, size_t packedSize ) {
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
*/


} // namespace

#endif
