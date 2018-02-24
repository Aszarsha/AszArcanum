#ifndef ASZARCANUM_DATTOOLS_DAT1_FILE_HPP
#define ASZARCANUM_DATTOOLS_DAT1_FILE_HPP

#include <cstddef>
#include <cstdint>

#include <ostream>
#include <iostream>
#include <vector>
#include <string>

#include <zlib.h>

#include "zstring_view.hpp"
#include "MemoryMappedFile.hpp"
#include "Subfile.hpp"

namespace AszArcanum::dattools::DAT1 {

class File {
	public:
		struct Footer {
				std::byte unknown0[16];
				uint32_t  dat1Tag;
				uint32_t  unknown1;
				uint32_t  subfilesDescriptionsOffset;
		};

	public:
		static File LoadFrom( zstring_view fileName );

	public:
		Footer const & GetFooter() const noexcept {   return footer;   }

		template< typename Func >
		void ForEachSubfile( Func && f ) const {
				std::for_each( subfiles.cbegin(), subfiles.cend(), [f = std::forward< Func >( f )]( auto & sf ) {
						f( *sf );
				});
		}

	private:
		std::string fileName;
		ReadOnlyMemoryMappedFile mappedFile;

		Footer footer;
		std::vector< std::unique_ptr< Subfile > > subfiles;

	private:
		File( zstring_view fName, ReadOnlyMemoryMappedFile && memMappedFile );
};

} // namespace AszArcanum::dattools::DAT1

#endif
