#ifndef ASZARCANUM_DATTOOLS_UI_SUBFILECONTENTVIEW_HPP
#define ASZARCANUM_DATTOOLS_UI_SUBFILECONTENTVIEW_HPP

#include <map>
#include <string>

#include <gtkmm.h>

namespace AszArcanum::dattools::UI {

class SubfileContentView
	: public Gtk::TextView {
	public:
		virtual ~SubfileContentView() = default;

	private:
		using BufPtr = Glib::RefPtr< Gtk::TextBuffer >;

		std::map< std::string, BufPtr > bufCache;
};

} // namespace

#endif
