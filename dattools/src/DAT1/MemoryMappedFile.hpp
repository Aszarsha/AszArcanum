#ifndef ASZARCANUM_DATTOOLS_DAT1_MEMORYMAPPEDFILE_HPP
#define ASZARCANUM_DATTOOLS_DAT1_MEMORYMAPPEDFILE_HPP

#include <cstddef> // std::byte

#include <gsl/gsl>

#include <boost/iostreams/device/mapped_file.hpp>

#include "zstring_view.hpp"

namespace AszArcanum::dattools::DAT1 {

class ReadOnlyMemoryMappedFile {
	public:
		using DataSpanT = gsl::span< std::byte const >;

	public:
		explicit ReadOnlyMemoryMappedFile( zstring_view fileName )
			: mappedFile( std::string( fileName.data(), fileName.size() ) ) {
		}

	public:
		DataSpanT GetData() const noexcept {
				// NOLINTNEXTLINE( cppcoreguidelines-pro-type-reinterpret-cast )
				return { reinterpret_cast< std::byte const * >( mappedFile.data() )
				       , gsl::narrow< DataSpanT::index_type >( mappedFile.size() )
				       };
		}

	private:
		using UnderlyingType = boost::iostreams::mapped_file_source;

	private:
		UnderlyingType mappedFile;
};

} // namespace AszArcanum::dattools::DAT1

#endif
