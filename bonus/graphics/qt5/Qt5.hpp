#pragma once
#include "../common/RasterCanvas.hpp"
#include <QApplication>
#include <QWidget>
#include <memory>

namespace arcade::bonus {
class QtSurface final : public QWidget {
  public:
    QtSurface(RasterCanvas &canvas, std::vector<Input> &inputs);

  protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void closeEvent(QCloseEvent *) override;

  private:
    RasterCanvas &canvas_;
    std::vector<Input> &inputs_;
    void key(QKeyEvent *, bool pressed);
};

class Qt5 final : public IGraphics {
  public:
    ~Qt5() override;
    void open() override;
    void close() noexcept override;
    std::vector<Input> poll() override;
    void draw(const Frame &frame) override;

  private:
    int argc_ = 1;
    char name_[7] = "arcade";
    char *argv_[2]{name_, nullptr};
    std::unique_ptr<QApplication> application_;
    std::unique_ptr<QtSurface> window_;
    RasterCanvas canvas_;
    std::vector<Input> inputs_;
};
} // namespace arcade::bonus
