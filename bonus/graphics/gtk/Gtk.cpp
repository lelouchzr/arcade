#include "Gtk.hpp"
#include "../common/NativeKeys.hpp"
#include <algorithm>
#include <dlfcn.h>
#include <stdexcept>
#include <utility>

namespace arcade::bonus {
Gtk::~Gtk() {
    close();
}

void Gtk::open() {
    if (window_)
        return;
#ifdef RTLD_NODELETE
    // GTK registers process-wide GTypes that other toolkits can reuse after this plugin closes.
    // Pin the dependency, while allowing the Arcade plugin and its widgets to unload normally.
    Dl_info library{};
    if (!dladdr(reinterpret_cast<void *>(gtk_init_check), &library) || !library.dli_fname)
        throw std::runtime_error("GTK+: cannot locate runtime library");
    void *resident = dlopen(library.dli_fname, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
    if (!resident)
        throw std::runtime_error("GTK+: cannot retain runtime type registry");
    dlclose(resident);
#endif
    if (!gtk_init_check(nullptr, nullptr))
        throw std::runtime_error("GTK+: cannot initialize display");
    window_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    area_ = gtk_drawing_area_new();
    gtk_window_set_title(GTK_WINDOW(window_), "ARCADE / 400 - GTK+");
    gtk_window_set_default_size(GTK_WINDOW(window_), Frame::Width, Frame::Height);
    gtk_container_add(GTK_CONTAINER(window_), area_);
    g_signal_connect(area_, "draw", G_CALLBACK(paint), this);
    g_signal_connect(window_, "key-press-event", G_CALLBACK(key), this);
    g_signal_connect(window_, "key-release-event", G_CALLBACK(key), this);
    g_signal_connect(window_, "delete-event", G_CALLBACK(closing), this);
    gtk_widget_show_all(window_);
}

void Gtk::close() noexcept {
    if (window_)
        gtk_widget_destroy(window_);
    window_ = nullptr;
    area_ = nullptr;
    inputs_.clear();
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
}

gboolean Gtk::closing(GtkWidget *, GdkEvent *, gpointer value) {
    static_cast<Gtk *>(value)->inputs_.push_back({Action::Quit});
    return TRUE;
}

gboolean Gtk::key(GtkWidget *, GdkEventKey *event, gpointer value) {
    auto input = nativeKey(event->keyval, event->type == GDK_KEY_PRESS);
    if (input.action != Action::None)
        static_cast<Gtk *>(value)->inputs_.push_back(input);
    return TRUE;
}

gboolean Gtk::paint(GtkWidget *widget, cairo_t *cr, gpointer value) {
    auto &self = *static_cast<Gtk *>(value);
    const int width = gtk_widget_get_allocated_width(widget),
              height = gtk_widget_get_allocated_height(widget);
    const double scale = std::min(static_cast<double>(width) / Frame::Width,
                                  static_cast<double>(height) / Frame::Height);
    cairo_set_source_rgb(cr, 10. / 255, 15. / 255, 23. / 255);
    cairo_paint(cr);
    auto *surface = cairo_image_surface_create_for_data(
        reinterpret_cast<unsigned char *>(
            const_cast<std::uint32_t *>(self.canvas_.pixels().data())),
        CAIRO_FORMAT_RGB24, Frame::Width, Frame::Height, Frame::Width * 4);
    cairo_save(cr);
    cairo_translate(cr, (width - Frame::Width * scale) / 2, (height - Frame::Height * scale) / 2);
    cairo_scale(cr, scale, scale);
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
    cairo_paint(cr);
    cairo_restore(cr);
    cairo_surface_destroy(surface);
    return FALSE;
}

std::vector<Input> Gtk::poll() {
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
    return std::exchange(inputs_, {});
}

void Gtk::draw(const Frame &frame) {
    if (!window_)
        throw std::runtime_error("GTK+ display is closed");
    canvas_.render(frame);
    gtk_widget_queue_draw(area_);
    while (gtk_events_pending())
        gtk_main_iteration_do(FALSE);
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Gtk, arcade::PluginKind::Graphics, "gtk", "GTK+ / CAIRO CANVAS")
