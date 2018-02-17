#ifndef ASZARCANUM_DATTOOLS_UI_SUBFILECONTENTVIEW_HPP
#define ASZARCANUM_DATTOOLS_UI_SUBFILECONTENTVIEW_HPP

#include <iostream>
#include <map>
#include <string>

#include <gtkmm.h>

#include "DAT1/Subfile.hpp"

namespace AszArcanum::dattools::UI {

class SubfileContentView
	: public Gtk::Frame {
	public:
		SubfileContentView()
			: dirCache( Gtk::TextBuffer::create() ) {
				textView.set_editable( false );
				textView.set_size_request( 200, -1 );

				scrolledWindow.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );

				scrolledWindow.add( textView );

				set_border_width( 2 );
				set_label_align( Gtk::ALIGN_CENTER, Gtk::ALIGN_CENTER );
				add( scrolledWindow );
				show_all_children();
		}

		virtual ~SubfileContentView() = default;

		void Show( DAT1::Subfile const & sf ) {
				switch ( sf.GetType() ) {
					case DAT1::Subfile::Type::Dir:
						dirCache->set_text( std::string( sf.GetPathName() ) + " is a directory." );
						textView.set_buffer( dirCache );
						unset_label();
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
						}
						textView.set_buffer( it->second );
						set_label( std::string( sf.GetPathName() ) );
						break;
				}
		}

	private:
		using BufPtr = Glib::RefPtr< Gtk::TextBuffer >;

	private:
		Gtk::ScrolledWindow scrolledWindow;
		Gtk::TextView textView;

		BufPtr dirCache;
		std::map< DAT1::Subfile const *, BufPtr > bufCache;
};

} // namespace

#endif
