#pragma once
#include "../common/RasterCanvas.hpp"
#include <gtk/gtk.h>

namespace arcade::bonus {
class Gtk final : public IGraphics {
  public:
    ~Gtk() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    GtkWidget *window_ = nullptr;
    GtkWidget *area_ = nullptr;
    RasterCanvas canvas_;
    std::vector<Input> inputs_;
    static gboolean paint(GtkWidget *, cairo_t *, gpointer);
    static gboolean key(GtkWidget *, GdkEventKey *, gpointer);
    static gboolean closing(GtkWidget *, GdkEvent *, gpointer);
};
} // namespace arcade::bonus
