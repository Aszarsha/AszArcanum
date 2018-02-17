#ifndef ASZARCANUM_DATTOOLS_UI_SUBFILECONTENTVIEW_HPP
#define ASZARCANUM_DATTOOLS_UI_SUBFILECONTENTVIEW_HPP

#include <iostream>
#include <map>
#include <string>

#include <gtkmm.h>

#include "DAT1/Subfile.hpp"

namespace AszArcanum::dattools::UI {

class SubfileContentView
	: public Gtk::ScrolledWindow {
	public:
		SubfileContentView()
			: dirCache( Gtk::TextBuffer::create() ) {
				set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

				textView.set_editable( false );
				textView.set_size_request( 200, -1 );

				add( textView );
				show_all_children();
		}

		virtual ~SubfileContentView() = default;

		void Show( DAT1::Subfile const & sf ) {
std::cout << "Showing " << sf.GetPathName() << std::endl;
				switch ( sf.GetType() ) {
					case DAT1::Subfile::Type::Dir:
						dirCache->set_text( std::string( sf.GetPathName() ) + " is a directory." );
						textView.set_buffer( dirCache );
						break;

					case DAT1::Subfile::Type::Raw:
					case DAT1::Subfile::Type::Zlib:
						auto & sfFile = sf.AsFile();
						auto it = bufCache.lower_bound( &sfFile );
						if ( it == bufCache.end() || it->first != &sfFile ) {
								it = bufCache.emplace_hint( it, &sfFile, Gtk::TextBuffer::create() );
								auto dataSpan = sfFile.GetData();
								std::string dataStr( reinterpret_cast< char const * >( dataSpan.data() )
								                   , reinterpret_cast< char const * >( dataSpan.data() + dataSpan.size() )
								                   );
								it->second->set_text( Glib::convert( dataStr, "UTF-8", "ISO-8859-1" ) );
std::cout << "CONTENT\n"
          << Glib::convert( dataStr, "UTF-8", "ISO-8859-1" )
          << "END OF CONTENT"
          << std::endl;
						}
						textView.set_buffer( it->second );
						break;
				}
		}

	private:
		using BufPtr = Glib::RefPtr< Gtk::TextBuffer >;

	private:
		Gtk::TextView textView;

		BufPtr dirCache;
		std::map< DAT1::Subfile const *, BufPtr > bufCache;
};

} // namespace

#endif
