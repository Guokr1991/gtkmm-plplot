/*
Copyright (C) 2015 Tom Schoonjans

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtkmm-plplot/gtkmm-plplot.h>
#include <gtkmm/application.h>
#include <glibmm/miscutils.h>
#include <gtkmm/window.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <glib.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/switch.h>
#include <gtkmm/comboboxtext.h>
#include <vector>

namespace Test2 {
  class Window : public Gtk::Window {
  private:
    Gtk::PLplot::Canvas canvas;
    Gtk::Grid grid;
    Gtk::Label x_log_label;
    Gtk::Label x_box_label;
    Gtk::Label y_log_label;
    Gtk::Label y_box_label;
    Gtk::Label box_label;
    Gtk::Switch x_log_switch;
    Gtk::Switch y_log_switch;
    Gtk::ComboBoxText box_combo;
  public:
    Window(std::vector<PLFLT> &x,
           std::vector<PLFLT> &y1,
           std::vector<PLFLT> &y2,
           std::vector<PLFLT> &y3,
           std::vector<PLFLT> &y4,
           std::string x_title,
           std::string y_title,
           std::string plot_title) :
           canvas(Gtk::PLplot::Plot2D(Gtk::PLplot::PlotData2D(x, y1, Gdk::RGBA("red")), x_title, y_title, plot_title)),
           x_log_label("X-axis logarithmic"),
           y_log_label("Y-axis logarithmic"),
           box_label("Box options") {


        Gtk::PLplot::Plot2D *plot = dynamic_cast<Gtk::PLplot::Plot2D *>(canvas.get_plot(0));
        plot->add_data(Gtk::PLplot::PlotData2D(x, y2, Gdk::RGBA("blue")));
        plot->add_data(Gtk::PLplot::PlotData2D(x, y3, Gdk::RGBA("Blue Violet")));
        plot->add_data(Gtk::PLplot::PlotData2D(x, y4, Gdk::RGBA("Green")));
        plot->set_axis_logarithmic_y();

        set_default_size(720, 580);
        Gdk::Geometry geometry;
        geometry.min_aspect = geometry.max_aspect = double(720)/double(580);
        set_geometry_hints(*this, geometry, Gdk::HINT_ASPECT);
        set_title("Gtkmm-PLplot test2");
        canvas.set_hexpand(true);
        canvas.set_vexpand(true);

        x_log_switch.set_active(plot->get_axis_logarithmic_x());
        y_log_switch.set_active(plot->get_axis_logarithmic_y());
        x_log_switch.property_active().signal_changed().connect([this, plot](){
          if (x_log_switch.get_active()) {
            plot->set_axis_logarithmic_x(true);
          }
          else {
            plot->set_axis_logarithmic_x(false);
          }
        });

        box_combo.append("No box, no ticks, no labels, no axes");
        box_combo.append("Box only");
        box_combo.append("Box, ticks and tick labels");
        box_combo.append("Box, ticks, tick labels, and main axes");
        box_combo.append("Box, ticks, tick labels, main axes and major tick grid");
        box_combo.append("Box, ticks, tick labels, main axes and major and minor tick grid");
        box_combo.signal_changed().connect(
          [this, plot](){
            plot->set_box_style(static_cast<Gtk::PLplot::BoxStyle>(
              box_combo.get_active_row_number() - 2
            ));
          }
        );
        box_combo.set_active(plot->get_box_style() + 2);

        y_log_switch.property_active().signal_changed().connect([this, plot](){
          if (y_log_switch.get_active()) {
            plot->set_axis_logarithmic_y(true);
          }
          else {
            plot->set_axis_logarithmic_y(false);
          }
        });
        x_log_label.set_hexpand(false);
        y_log_label.set_hexpand(false);
        box_label.set_hexpand(false);
        x_log_switch.set_hexpand(false);
        y_log_switch.set_hexpand(false);
        box_combo.set_hexpand(false);
        x_log_label.set_halign(Gtk::ALIGN_END);
        y_log_label.set_halign(Gtk::ALIGN_END);
        box_label.set_halign(Gtk::ALIGN_END);
        x_log_switch.set_halign(Gtk::ALIGN_START);
        y_log_switch.set_halign(Gtk::ALIGN_START);
        box_combo.set_halign(Gtk::ALIGN_START);

        grid.attach(x_log_label, 0, 0, 1, 1);
        grid.attach(y_log_label, 0, 1, 1, 1);
        grid.attach(x_log_switch, 1, 0, 1, 1);
        grid.attach(y_log_switch, 1, 1, 1, 1);
        grid.attach(box_label, 0, 2, 1, 1);
        grid.attach(box_combo, 1, 2, 1, 1);
        grid.attach(canvas, 0, 3, 2, 1);
        grid.set_column_spacing(5);
        grid.set_column_homogeneous(false);

        add(grid);
        set_border_width(10);
        grid.show_all();
    }
    virtual ~Window() {}
  };
}


int main(int argc, char *argv[]) {
  Glib::set_application_name("gtkmm-plplot-test2");
  Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "eu.tomschoonjans.gtkmm-plplot-test2");

  //open our test file
  std::ifstream fs;
  fs.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
  try {
	   fs.open(TEST_CSV);
  }
  catch (std::exception &e) {
    std::cerr << "Error opening file " << TEST_CSV << " -> " << e.what() << std::endl;
    return 1;
  }

  std::vector<PLFLT> x;
  std::vector<PLFLT> y1, y2, y3, y4;

  while (1) {
    try {
      fs.clear();
      std::string line;
      std::getline(fs, line);
      gchar **splitted = g_strsplit(line.c_str(), ",", 0);

      x.push_back(g_ascii_strtod(splitted[1], NULL));
      y1.push_back(g_ascii_strtod(splitted[2], NULL));
      y2.push_back(g_ascii_strtod(splitted[3], NULL));
      y3.push_back(g_ascii_strtod(splitted[4], NULL));
      y4.push_back(g_ascii_strtod(splitted[5], NULL));
      g_strfreev(splitted);
    }
    catch (std::exception &e) {
      if (fs.eof()) {
        break;
      }
      std::cerr << "Error parsing " << TEST_CSV << " -> " << e.what() << std::endl;
      return 1;
    }
    catch (...) {
      std::cerr << "Unknown exception occurred" << std::endl;
      return 1;
    }
  }

  //ensure our y-values are greater than 1!
  //the x-values are already...
  std::for_each(std::begin(y1), std::end(y1), [](PLFLT &a) { if (a < 1.0 ) a = 1.0;});
  std::for_each(std::begin(y2), std::end(y2), [](PLFLT &a) { if (a < 1.0 ) a = 1.0;});
  std::for_each(std::begin(y3), std::end(y3), [](PLFLT &a) { if (a < 1.0 ) a = 1.0;});
  std::for_each(std::begin(y4), std::end(y4), [](PLFLT &a) { if (a < 1.0 ) a = 1.0;});

  std::string x_title("Energy (keV)"), y_title("Intensity (counts)"), plot_title("NIST SRM 1155 Stainless steel");
  Test2::Window window(x, y1, y2, y3, y4, x_title, y_title, plot_title);

	return app->run(window);
}