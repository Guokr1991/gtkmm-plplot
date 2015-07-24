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

#include <gtkmm-plplot/plot2d.h>
#include <gtkmm-plplot/exception.h>
#include <iostream>
#include <gdkmm/rgba.h>
#include <gdkmm/general.h>

#define PLPLOT_LIN_LIN 0
#define PLPLOT_LIN_LOG 20
#define PLPLOT_LOG_LIN 10
#define PLPLOT_LOG_LOG 30


using namespace Gtk::PLplot;

Plot2D::Plot2D(
  const Plot2DData &_data,
  const Glib::ustring &_axis_title_x,
  const Glib::ustring &_axis_title_y,
  const Glib::ustring &_plot_title,
  const double _plot_width_norm,
  const double _plot_height_norm,
  const double _plot_offset_horizontal_norm,
  const double _plot_offset_vertical_norm) :
  log10_x(false),
  log10_y(false),
  axis_title_x(_axis_title_x),
  axis_title_y(_axis_title_y),
  plot_title(_plot_title),
  pls(nullptr),
  shown(true),
  background_color("Black"),
  plot_width_norm(_plot_width_norm),
  plot_height_norm(_plot_height_norm),
  plot_offset_horizontal_norm(_plot_offset_horizontal_norm),
  plot_offset_vertical_norm(_plot_offset_vertical_norm),
  region_selectable(true),
  axes_color("Black"),
  titles_color("Black"),
  box_style(BOX_TICKS_TICK_LABELS) {

  background_color.set_alpha(0.0);

  //connect our default signal handlers
  this->signal_select_region().connect(sigc::mem_fun(*this, &Plot2D::on_select_region));
  this->signal_changed().connect(sigc::mem_fun(*this, &Plot2D::on_changed));
  this->signal_data_added().connect(sigc::mem_fun(*this, &Plot2D::on_data_added));
  this->signal_data_removed().connect(sigc::mem_fun(*this, &Plot2D::on_data_removed));
  add_data(_data);
}

Plot2D::Plot2D(const Plot2D &_source) :
  log10_x(_source.log10_x),
  log10_y(_source.log10_y),
  axis_title_x(_source.axis_title_x),
  axis_title_y(_source.axis_title_y),
  plot_title(_source.plot_title),
  pls(nullptr),
  shown(true),
  background_color(_source.background_color),
  plot_width_norm(_source.plot_width_norm),
  plot_height_norm(_source.plot_height_norm),
  plot_offset_horizontal_norm(_source.plot_offset_horizontal_norm),
  plot_offset_vertical_norm(_source.plot_offset_vertical_norm),
  region_selectable(_source.region_selectable),
  axes_color(_source.axes_color),
  titles_color(_source.titles_color),
  box_style(_source.box_style) {

  this->signal_select_region().connect(sigc::mem_fun(*this, &Plot2D::on_select_region));
  this->signal_changed().connect(sigc::mem_fun(*this, &Plot2D::on_changed));
  this->signal_data_added().connect(sigc::mem_fun(*this, &Plot2D::on_data_added));
  this->signal_data_removed().connect(sigc::mem_fun(*this, &Plot2D::on_data_removed));

  for (auto &iter : _source.plot_data) {
    add_data(*iter);
  }
}

Plot2D::~Plot2D() {
  for (auto &iter : plot_data) {
    delete iter;
  }
  if (pls)
    delete pls;
}

void Plot2D::on_select_region(double xmin, double xmax, double ymin, double ymax) {
  set_region(xmin, xmax, ymin, ymax);
}

void Plot2D::on_changed() {
  //this function does nothing
  //it is designed to be overridden by a derived class
}

void Plot2D::show() {
  shown = true;
  _signal_changed.emit();
}

void Plot2D::hide() {
  shown = false;
  _signal_changed.emit();
}

bool Plot2D::is_showing() const {
  return shown;
}

void Plot2D::on_data_added(Plot2DData *added_data) {
  plot_data_modified();
}

void Plot2D::on_data_removed() {
  plot_data_modified();
}

void Plot2D::plot_data_modified() {
  //update ranges
  std::vector<PLFLT> min_x, min_y;
  std::vector<PLFLT> max_x, max_y;

  for (auto &iter : plot_data) {
    min_x.push_back(*std::min_element(iter->x.begin(), iter->x.end()));
    max_x.push_back(*std::max_element(iter->x.begin(), iter->x.end()));
    min_y.push_back(*std::min_element(iter->y.begin(), iter->y.end()));
    max_y.push_back(*std::max_element(iter->y.begin(), iter->y.end()));
  }
  plot_data_range_x[0] = *std::min_element(min_x.begin(), min_x.end());
  plot_data_range_x[1] = *std::max_element(max_x.begin(), max_x.end());
  plot_data_range_y[0] = *std::min_element(min_y.begin(), min_y.end());
  plot_data_range_y[1] = *std::max_element(max_y.begin(), max_y.end());

  if (log10_x) {
    plot_data_range_x[0] = log10(plot_data_range_x[0]);
    plot_data_range_x[1] = log10(plot_data_range_x[1]);
  }
  if (log10_y) {
    plot_data_range_y[0] = log10(plot_data_range_y[0]);
    plot_data_range_y[1] = log10(plot_data_range_y[1]);
  }

  plotted_range_x[0] = plot_data_range_x[0];
  plotted_range_x[1] = plot_data_range_x[1];
  plotted_range_y[0] = plot_data_range_y[0];
  plotted_range_y[1] = plot_data_range_y[1];
  _signal_changed.emit();
}

void Plot2D::add_data(const Plot2DData &data) {
  Plot2DData *data_copy = new Plot2DData(data);
  plot_data.push_back(data_copy);
  data_copy->signal_changed().connect([this](){_signal_changed.emit();});
  data_copy->signal_data_modified().connect([this](){plot_data_modified();});

  _signal_data_added.emit(data_copy);
}

void Plot2D::set_axis_logarithmic_x(bool _log10) {
  //need to check that all values are positive!
  if (_log10) {
    for (auto &iter : plot_data) {
      if (std::count_if(iter->x.begin(), iter->x.end(), std::bind2nd(std::less_equal<PLFLT>(), double(0.0))) > 0) {
        throw Exception("Gtkmm::Plplot::Plot2D::set_axis_logarithmic_x -> plot X-values must be strictly positive");
      }
    }
  }
  log10_x = _log10;
  plot_data_modified();
}

void Plot2D::set_axis_logarithmic_y(bool _log10) {
  //need to check that all values are positive!
  if (_log10) {
    for (auto &iter : plot_data) {
      if (std::count_if(iter->y.begin(), iter->y.end(), std::bind2nd(std::less_equal<PLFLT>(), double(0.0))) > 0) {
        throw Exception("Gtkmm::Plplot::Plot2D::set_axis_logarithmic_y -> plot Y-values must be strictly positive");
      }
    }
  }
  log10_y = _log10;
  plot_data_modified();
}

bool Plot2D::get_axis_logarithmic_x() {
  return log10_x;
}

bool Plot2D::get_axis_logarithmic_y() {
  return log10_y;
}

void Plot2D::set_axis_title_x(Glib::ustring &title) {
  axis_title_x = title;
  _signal_changed.emit();
}

void Plot2D::set_axis_title_y(Glib::ustring &title) {
  axis_title_y = title;
  _signal_changed.emit();
}

Glib::ustring Plot2D::get_axis_title_x() {
  return axis_title_x;
}

Glib::ustring Plot2D::get_axis_title_y() {
  return axis_title_y;
}

void Plot2D::set_plot_title(Glib::ustring &title) {
  plot_title = title;
  _signal_changed.emit();
}

Glib::ustring Plot2D::get_plot_title() {
  return plot_title;
}

void Plot2D::set_box_style(BoxStyle _box_style) {
  box_style = _box_style;
  _signal_changed.emit();
}

BoxStyle Plot2D::get_box_style() {
  return box_style;
}

void Plot2D::coordinate_transform(PLFLT x_old, PLFLT y_old, PLFLT *x_new, PLFLT *y_new, PLPointer object) {
  Plot2D *plot2d = static_cast<Plot2D*>(object);

  //let's start with giving the new ones the old values, in case no transformation is necessary
  *x_new = x_old;
  *y_new = y_old;

  if (plot2d->log10_x)
    *x_new = log10(x_old);

  if (plot2d->log10_y)
    *y_new = log10(y_old);
}


//it's possible I can get the width and height also from the Cairo context through its surface
//problem is that this only works for image surfaces, not for PDF etc :-(
void Plot2D::draw_plot(const Cairo::RefPtr<Cairo::Context> &cr, const int width, const int height) {
  if (!shown)
    return;

  canvas_width = width;
  canvas_height = height;
  plot_width = width * plot_width_norm;
  plot_height= height * plot_height_norm;
  plot_offset_x = width * plot_offset_horizontal_norm;
  plot_offset_y = height * plot_offset_vertical_norm;

  if (pls)
    delete pls;
  pls = new plstream;

  pls->sdev("extcairo");
  pls->spage(0.0, 0.0, plot_width , plot_height, plot_offset_x, plot_offset_y);
  Gdk::Cairo::set_source_rgba(cr, background_color);
  cr->rectangle(plot_offset_x, plot_offset_y, plot_width, plot_height);
  cr->fill();
  cr->save();
  cr->translate(plot_offset_x, plot_offset_y);
  pls->init();

  pls->cmd(PLESC_DEVINIT, cr->cobj());

  int plplot_axis_style;

  plplot_axis_style = box_style;

  if (box_style >= 0 ) {
    if (log10_x && log10_y)
      plplot_axis_style += PLPLOT_LOG_LOG;
    else if (log10_x)
      plplot_axis_style += PLPLOT_LOG_LIN;
    else if (log10_y)
      plplot_axis_style += PLPLOT_LIN_LOG;
    else
      plplot_axis_style += PLPLOT_LIN_LIN;
  }

  //set the axes color
  PLINT red_u_old, green_u_old, blue_u_old;
  PLFLT alpha_old;

  pls->gcol0a(5, red_u_old, green_u_old, blue_u_old, alpha_old);
  pls->scol0a(5, axes_color.get_red_u()/256, axes_color.get_green_u()/256, axes_color.get_blue_u()/256, axes_color.get_alpha());
  pls->col0(5);

  //plot the box with its axes
  pls->env(plotted_range_x[0], plotted_range_x[1],
           plotted_range_y[0], plotted_range_y[1],
           0, plplot_axis_style);
  //restore color
  pls->scol0a(5, red_u_old, green_u_old, blue_u_old, alpha_old);

  //set the label color
  pls->gcol0a(5, red_u_old, green_u_old, blue_u_old, alpha_old);
  pls->scol0a(5, titles_color.get_red_u()/256, titles_color.get_green_u()/256, titles_color.get_blue_u()/256, titles_color.get_alpha());
  pls->col0(5);

  pls->lab(axis_title_x.c_str(), axis_title_y.c_str(), plot_title.c_str());

  //restore color
  pls->scol0a(5, red_u_old, green_u_old, blue_u_old, alpha_old);

  //hook up the coordinate transform
  pls->stransform(&Plot2D::coordinate_transform, this);

  for (auto &iter : plot_data) {
    iter->draw_plot_data(cr, pls);
  }
  cr->restore();

  convert_plplot_to_cairo_coordinates(plotted_range_x[0], plotted_range_y[0],
                                      cairo_range_x[0], cairo_range_y[0]);
  convert_plplot_to_cairo_coordinates(plotted_range_x[1], plotted_range_y[1],
                                      cairo_range_x[1], cairo_range_y[1]);
}


void Plot2D::convert_plplot_to_cairo_coordinates(
  double x_pl, double y_pl,
  double &x_cr, double &y_cr) {
    //inspired by http://www.mail-archive.com/plplot-devel@lists.sourceforge.net/msg02383.html
    //but the last equation was incorrect and is fixed here
    double nxmin, nxmax, nymin, nymax;
    double wxmin, wxmax, wymin, wymax;

    pls->gvpd(nxmin, nxmax, nymin, nymax);
    pls->gvpw(wxmin, wxmax, wymin, wymax);

    double xmin = plot_width * nxmin + plot_offset_x;
    double xmax = plot_width * nxmax + plot_offset_x;
    double ymin = plot_height * (nymin - 1.0) + canvas_height - plot_offset_y;
    double ymax = plot_height * (nymax - 1.0) + canvas_height - plot_offset_y;

    x_cr = xmin + ((xmax - xmin) * ((x_pl - wxmin) / (wxmax - wxmin)));
    y_cr = ymin + ((ymax - ymin) * ((y_pl - wymin) / (wymax - wymin)));
}

void Plot2D::convert_cairo_to_plplot_coordinates(
  double x_cr, double y_cr,
  double &x_pl, double &y_pl) {

    double nxmin, nxmax, nymin, nymax;
    double wxmin, wxmax, wymin, wymax;

    pls->gvpd(nxmin, nxmax, nymin, nymax);
    pls->gvpw(wxmin, wxmax, wymin, wymax);

    double xmin = plot_width * nxmin + plot_offset_x;
    double xmax = plot_width * nxmax + plot_offset_x;
    double ymin = plot_height * (nymin - 1.0) + canvas_height - plot_offset_y;
    double ymax = plot_height * (nymax - 1.0) + canvas_height - plot_offset_y;

    x_pl = wxmin + ((x_cr - xmin) * (wxmax - wxmin) / (xmax - xmin));
    y_pl = wymin + ((y_cr - ymin) * (wymax - wymin) / (ymax - ymin));
}

void Plot2D::set_region(double xmin, double xmax, double ymin, double ymax) {
  if (xmin == xmax && ymin == ymax) {
    //due to signal propagation, this function will actually be called twice on a double-click event,
    //the second time after the plot has already been resized to its normal geometry
    //this condition avoids the warning message...
    return;
  }

  if (log10_x) {
    xmin = log10(xmin);
    xmax = log10(xmax);
  }
  if (log10_y) {
    ymin = log10(ymin);
    ymax = log10(ymax);
  }

  if (xmin >= xmax || ymin >= ymax /*||
      xmin < plot_data_range_x[0] ||
      xmax > plot_data_range_x[1] ||
      ymin < plot_data_range_y[0] ||
      ymax > plot_data_range_y[1]*/) {
    throw Exception("Gtk::PLplot::Plot2D::set_region -> Invalid arguments");
  }
  plotted_range_x[0] = xmin;
  plotted_range_x[1] = xmax;
  plotted_range_y[0] = ymin;
  plotted_range_y[1] = ymax;

  _signal_changed.emit();
}

Plot2DData *Plot2D::get_data(unsigned int index) {
  if (index < plot_data.size()) {
    return plot_data[index];
  }
  throw Exception("Gtk::PLplot::Plot2D::get_data -> Invalid index");
}

Gdk::RGBA Plot2D::get_background_color() {
  return background_color;
}

void Plot2D::set_background_color(Gdk::RGBA _background_color) {
  background_color = _background_color;
  _signal_changed.emit();
}

Gdk::RGBA Plot2D::get_axes_color() {
  return axes_color;
}

void Plot2D::set_axes_color(Gdk::RGBA _axes_color) {
  axes_color = _axes_color;
  _signal_changed.emit();
}

Gdk::RGBA Plot2D::get_titles_color() {
  return titles_color;
}

void Plot2D::set_titles_color(Gdk::RGBA _titles_color) {
  titles_color = _titles_color;
  _signal_changed.emit();
}

bool Plot2D::get_region_selectable() {
  return region_selectable;
}

void Plot2D::set_region_selectable(bool _region_selectable) {
  region_selectable = _region_selectable;
}