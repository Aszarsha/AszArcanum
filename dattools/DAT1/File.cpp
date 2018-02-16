#include "File.hpp"

using namespace std;

namespace AszArcanum::dattools::DAT1 {

File File::LoadFrom( std::string_view fileName ) {
		std::cout << "Loading \"" << fileName << '\"' << std::endl;
		MemoryMappedFile mmFile;
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

		return File( fileName, move( mmFile ) );
}

File::File( string_view fName, boost::iostreams::mapped_file_source && mmfile )
	: fileName( fName )
	, memMappedFile( move( mmfile ) ) {
		ReadFooter();
		ReadSubfiles();
}

void File::ReadFooter() {
		auto addr = reinterpret_cast< byte const * >( memMappedFile.data() );
		auto size = memMappedFile.size();
		memcpy( &footer, addr + size - sizeof( footer ), sizeof( footer ) );
}

namespace {
	inline void ReadBytesFromDataPtr( byte const* & data, byte * to, size_t sz ) {
			memcpy( to, data, sz );
			data += sz;
	}

	template< typename Type >
	inline Type ReadTypeFromDataPtr( byte const* & data ) {
			Type out;
			ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &out ), sizeof( out ) );
			return out;
	}
}

void File::ReadSubfiles() {
		auto baseAddr = reinterpret_cast< byte const * >( memMappedFile.data() + memMappedFile.size() - footer.treeDescOffset );
		auto data = baseAddr;
		uint32_t numSubfiles = ReadTypeFromDataPtr< uint32_t >( data );
		for ( uint32_t i = 0; i != numSubfiles; ++i ) {
				Subfile::Index index;
				uint32_t filenameSz = ReadTypeFromDataPtr< uint32_t >( data );
				index.pathName.resize( filenameSz-1 );   // do not count the included null-termination
				ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &index.pathName[0] ), filenameSz-1 );
				data += 1;   // skip null-termination char
				ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &index.data ), sizeof( index.data ) );

				subfiles.emplace_back( CreateSubfileFromIndex( move( index ) ) );
		}
}

unique_ptr< Subfile > File::CreateSubfileFromIndex( Subfile::Index && index ) {
		switch ( index.data.type ) {
			case Subfile::Type::Dir:
				return make_unique< SubfileDir  >( move( index ) );
			case Subfile::Type::Raw:
				return make_unique< SubfileRaw  >( move( index )
				                                 , reinterpret_cast< byte const * >( memMappedFile.data() + index.data.offset )
				                                 );
			case Subfile::Type::Zlib:
				return make_unique< SubfileZlib >( move( index )
				                                 , reinterpret_cast< byte const * >( memMappedFile.data() + index.data.offset )
				                                 );
		}
}

} // namespace
