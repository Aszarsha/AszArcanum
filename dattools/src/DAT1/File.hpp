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

#include "Subfile.hpp"

namespace AszArcanum::dattools::DAT1 {

class File {
	public:
		struct Footer {
				std::byte unknown0[16];
				uint32_t  dat1Tag;
				uint32_t  unknown1;
				uint32_t  treeDescOffset;
		};

	public:
		static File LoadFrom( std::string_view fileName );

	public:
		Footer const & GetFooter() const {   return footer;   }

		template< typename Func >
		void ForEachSubfile( Func && f ) const {
				std::for_each( subfiles.cbegin(), subfiles.cend(), [f = std::forward< Func >( f )]( auto & sf ) {
						f( *sf );
				});
		}

	private:
		using MemoryMappedFile = boost::iostreams::mapped_file_source;

	private:
		std::string fileName;
		MemoryMappedFile memMappedFile;

		Footer footer;
		std::vector< std::unique_ptr< Subfile > > subfiles;

	private:
		File( std::string_view fName, MemoryMappedFile && mmFile );

	private:
		void ReadFooter();
		void ReadSubfiles();

	private:
		std::unique_ptr< Subfile > CreateSubfileFromIndex( Subfile::Index && index );
};

} // namespace

#endif
