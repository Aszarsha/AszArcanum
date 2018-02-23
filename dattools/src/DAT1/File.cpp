#include "File.hpp"

#include <gsl/gsl>

using std::move;

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

namespace {
	auto ReadFooter( std::byte const * fileData, size_t fileSize ) noexcept {
			File::Footer footer{};

			std::advance( fileData, fileSize - sizeof( footer ) );
			memcpy( &footer, fileData, sizeof( footer ) );

			return footer;
	}

	inline void ReadBytesFromDataPtr( std::byte const* & data, std::byte * to, size_t sz ) noexcept {
			memcpy( to, data, sz );
			std::advance( data, sz );
	}

	template< typename Type >
	inline Type ReadTypeFromDataPtr( std::byte const* & data ) noexcept {
			Type out;
			// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
			ReadBytesFromDataPtr( data, reinterpret_cast< std::byte * >( &out ), sizeof( out ) );
			return out;
	}

	std::unique_ptr< Subfile > CreateSubfileFromIndex( std::byte const * fileData, Subfile::Index && index ) {
			switch ( index.data.type ) {
				case Subfile::Type::Dir:
					return std::make_unique< SubfileDir  >( move( index ) );
				case Subfile::Type::Raw:
					std::advance( fileData, index.data.offset );
					return std::make_unique< SubfileRaw  >( move( index ), fileData );
				case Subfile::Type::Zlib:
					std::advance( fileData, index.data.offset );
					return std::make_unique< SubfileZlib >( move( index ), fileData );
				default:
					Ensures( !"Should never get here" );
			}
	}

	auto ReadSubfiles( std::byte const * fileData, size_t subfilesOffset ) {
			std::vector< std::unique_ptr< Subfile > > subfiles;

			auto curDataPtr = fileData;
			std::advance( curDataPtr, subfilesOffset );
			auto numSubfiles = ReadTypeFromDataPtr< uint32_t >( curDataPtr );
			for ( uint32_t i = 0; i != numSubfiles; ++i ) {
					Subfile::Index index;

					auto filenameSz = ReadTypeFromDataPtr< uint32_t >( curDataPtr );
					index.pathName.resize( filenameSz-1 );   // do not count the included null-termination
					// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
					ReadBytesFromDataPtr( curDataPtr, reinterpret_cast< std::byte * >( &index.pathName[0] ), filenameSz-1 );
					curDataPtr += 1;   // skip null-termination char
					// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
					ReadBytesFromDataPtr( curDataPtr, reinterpret_cast< std::byte * >( &index.data ), sizeof( index.data ) );

					subfiles.emplace_back( CreateSubfileFromIndex( fileData, move( index ) ) );
			}

			return subfiles;
	}
} // namespace

File::File( std::string_view fName, boost::iostreams::mapped_file_source && mmFile )
	: fileName{ fName }
	, memMappedFile{ move( mmFile ) }
	, footer{ ReadFooter( reinterpret_cast< std::byte const * >( memMappedFile.data() ), memMappedFile.size() ) }
	, subfiles{ ReadSubfiles( reinterpret_cast< std::byte const * >( memMappedFile.data() ), memMappedFile.size() - footer.treeDescOffset ) } {
}

} // namespace AszArcanum::dattools::DAT1
