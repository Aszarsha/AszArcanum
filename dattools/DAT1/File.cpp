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
		, memMappedFile( move( mmfile ) )
		, footer( ReadFooter( reinterpret_cast< std::byte const * >( memMappedFile.data() ), memMappedFile.size() ) ) {
	}


	File::Footer File::ReadFooter( byte const * data, size_t size ) {
			Footer footer;
			memcpy( &footer, data + size - sizeof( footer ), sizeof( footer ) );
			return footer;
	}

} // namespace
