#include "File.hpp"

#include <gsl/gsl>

using std::move;

namespace AszArcanum::dattools::DAT1 {

File File::LoadFrom( zstring_view fileName ) {
		std::cout << "Loading \"" << fileName << '\"' << std::endl;
		try {
				return File{ fileName, ReadOnlyMemoryMappedFile{ fileName } };
		} catch ( ... ) {
				std::cerr << "error: could not open file" << std::endl;
				throw;
		}
		std::cout << "File loaded" << std::endl;

}

namespace {
	auto ReadFooter( gsl::span< std::byte const > fileDataSpan ) noexcept {
			File::Footer footer{};

			auto footerDataSpan = fileDataSpan.last( sizeof( footer ) );
			memcpy( &footer, footerDataSpan.data(), sizeof( footer ) );

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

	auto ReadSubfiles( gsl::span< std::byte const > fileDataSpan, size_t subfilesDescOffset ) {
			std::vector< std::unique_ptr< Subfile > > subfiles;

			auto const fileData = fileDataSpan.data();
			auto const subfilesDataSpan = fileDataSpan.subspan( fileDataSpan.size() - subfilesDescOffset
			                                                  , subfilesDescOffset - sizeof( File::Footer )
			                                                  );
			auto curDataPtr = subfilesDataSpan.data();   // TODO get rid of raw pointer eventually
			auto const numSubfiles = ReadTypeFromDataPtr< uint32_t >( curDataPtr );
			for ( uint32_t i = 0; i != numSubfiles; ++i ) {
					Subfile::Index index;

					auto const filenameSz = ReadTypeFromDataPtr< uint32_t >( curDataPtr );
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

File::File( zstring_view fName, ReadOnlyMemoryMappedFile && memMappedFile )
	: fileName{ fName }
	, mappedFile{ move( memMappedFile ) }
	, footer{ ReadFooter( mappedFile.GetData() ) }
	, subfiles{ ReadSubfiles( mappedFile.GetData(), footer.subfilesDescriptionsOffset ) } {
}

} // namespace AszArcanum::dattools::DAT1
