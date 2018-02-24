#ifndef ASZARCANUM_DATTOOLS_DAT1_SUBFILE_HPP
#define ASZARCANUM_DATTOOLS_DAT1_SUBFILE_HPP

#include <cstdint>

#include <memory>
#include <string>
#include <vector>

#include <gsl/gsl>

#include "zstring_view.hpp"

namespace AszArcanum::dattools::DAT1 {

class SubfileDir;
class SubfileFile;

class Subfile {
	public:
		enum class Type: uint32_t { Raw  = 0x001
		                          , Zlib = 0x002
		                          , Dir  = 0x400
		                          };

		struct Index {
				std::string pathName;
				struct {
						uint32_t unknown0;
						Type     type;
						uint32_t realSize;
						uint32_t packedSize;
						uint32_t offset;
				} data;
		};

	public:
		Subfile( Index && idx )
			: index( std::move( idx ) ) {
		}

		virtual ~Subfile() = 0;

	public:
		Index const & GetIndex() const;

		zstring_view GetPathName() const;
		Type GetType() const;

		SubfileDir        & AsDir();
		SubfileDir  const & AsDir() const;

		SubfileFile       & AsFile();
		SubfileFile const & AsFile() const;

	private:
		Index index;
};

class SubfileDir final
	: public Subfile {
	public:
		SubfileDir( Index && index )
			: Subfile( std::move( index ) ) {
		}
};

class SubfileFile
	: public Subfile {
	public:
		virtual gsl::span< std::byte const > GetData() const = 0;

	protected:
		SubfileFile( Index && index, std::byte const * ptr )
			: Subfile( std::move( index ) )
			, ptrToData( ptr ) {
		}

	protected:
		std::byte const * ptrToData;
};

class SubfileRaw final
	: public SubfileFile {
	public:
		SubfileRaw( Index && index, std::byte const * ptrToData )
			: SubfileFile( std::move( index ), ptrToData ) {
		}

	public:
		virtual gsl::span< std::byte const > GetData() const final;
};

class SubfileZlib final
	: public SubfileFile {
	public:
		SubfileZlib( Index && index, std::byte const * ptrToData )
			: SubfileFile( std::move( index ), ptrToData ) {
		}

	public:
		virtual gsl::span< std::byte const > GetData() const final;

	private:
		mutable std::vector< std::byte > inflated;   //< initialized (zlib inflate) when required
};

} // namespace AszArcanum::dattools::DAT1

#endif
