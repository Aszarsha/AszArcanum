#include "File.hpp"

#include <gsl/gsl>

using std::move;
using std::string_view;

namespace AszArcanum::dattools::DAT1 {

File File::LoadFrom( std::string_view fileName ) {
		std::cout << "Loading \"" << fileName << '\"' << std::endl;
		MemoryMappedFile mmFile{};
		try {
				mmFile.open( std::string( fileName ) );
		} catch ( char const * s ) {
				std::cerr << "error: " << s << std::endl;
				throw;
		} catch ( ... ) {
				std::cerr << "error: could not open file" << std::endl;
				throw;
		}
		std::cout << "File loaded" << std::endl;

		return File{ fileName, move( mmFile ) };
}

File::File( string_view fName, boost::iostreams::mapped_file_source && mmFile )
	: fileName{ fName }
	, memMappedFile{ move( mmFile ) }
	, footer{} {
		ReadFooter();
		ReadSubfiles();
}

void File::ReadFooter() {
		// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
		auto addr = reinterpret_cast< std::byte const * >( memMappedFile.data() );
		auto size = memMappedFile.size();
		memcpy( &footer, addr + size - sizeof( footer ), sizeof( footer ) );
}

namespace {
	inline void ReadBytesFromDataPtr( std::byte const* & data, std::byte * to, size_t sz ) {
			memcpy( to, data, sz );
			// NOLINTNEXTLINE( cppcoreguidelines-pro-bounds-pointer-arithmetic )
			data += sz;
	}

	template< typename Type >
	inline Type ReadTypeFromDataPtr( std::byte const* & data ) {
			Type out;
			// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
			ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &out ), sizeof( out ) );
			return out;
	}
}

void File::ReadSubfiles() {
		// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
		auto baseAddr = reinterpret_cast< std::byte const * >( memMappedFile.data() + memMappedFile.size() - footer.treeDescOffset );
		auto data = baseAddr;
		auto numSubfiles = ReadTypeFromDataPtr< uint32_t >( data );
		for ( uint32_t i = 0; i != numSubfiles; ++i ) {
				Subfile::Index index;
				auto filenameSz = ReadTypeFromDataPtr< uint32_t >( data );
				index.pathName.resize( filenameSz-1 );   // do not count the included null-termination
				// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
				ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &index.pathName[0] ), filenameSz-1 );
				data += 1;   // skip null-termination char
				// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
				ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &index.data ), sizeof( index.data ) );

				subfiles.emplace_back( CreateSubfileFromIndex( move( index ) ) );
		}
}

std::unique_ptr< Subfile > File::CreateSubfileFromIndex( Subfile::Index && index ) {
		switch ( index.data.type ) {
			case Subfile::Type::Dir:
				return std::make_unique< SubfileDir  >( move( index ) );
			case Subfile::Type::Raw:
				return std::make_unique< SubfileRaw  >( move( index )
				// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
				                                      , reinterpret_cast< std::byte const * >( memMappedFile.data() + index.data.offset )
				                                      );
			case Subfile::Type::Zlib:
				return std::make_unique< SubfileZlib >( move( index )
				// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
				                                      , reinterpret_cast< std::byte const * >( memMappedFile.data() + index.data.offset )
				                                      );
			default: Ensures( !"Should never get here" );
		}
}

} // namespace
