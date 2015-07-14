#ifndef GTKMMPLPLOTPLOT2DDATA_H
#define GTKMMPLPLOTPLOT2DDATA_H

#include <sigc++/sigc++.h>
#include <vector>
#include <valarray>
#include "gtkmm-plplot/color.h"
#include <plstream.h>
#include <glibmm/ustring.h>

namespace Gtk {
  namespace PLplot {
    class Plot2DData : public sigc::trackable {
    private:
      std::vector<PLFLT> x;
      std::vector<PLFLT> y;
      Color color;
      bool shown;
      sigc::signal<void> _signal_changed;
      Plot2DData();
    protected:
      virtual void on_changed();
    public:
      Plot2DData(const std::vector<PLFLT> &x,
                 const std::vector<PLFLT> &y,
                 Color color = RED);
      Plot2DData(const std::valarray<PLFLT> &x,
                 const std::valarray<PLFLT> &y,
                 Color color = RED);
      Plot2DData(const std::vector<PLFLT> &y,
                 Color color = RED);
      Plot2DData(const std::valarray<PLFLT> &y,
                 Color color = RED);
      Plot2DData(const Plot2DData &data);
      virtual ~Plot2DData() {}
      void set_color(Color color);
      Color get_color();
      void show();
      void hide();
      bool is_showing() const;
      sigc::signal<void> signal_changed() {
        return _signal_changed;
      }
      friend class Plot2D;
    };
  }
}

#endif
