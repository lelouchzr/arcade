#include "Qt5.hpp"
#include "../common/NativeKeys.hpp"
#include <QCloseEvent>
#include <QKeyEvent>
#include <QPainter>
#include <stdexcept>
#include <utility>

namespace arcade::bonus {
QtSurface::QtSurface(RasterCanvas &canvas, std::vector<Input> &inputs)
    : canvas_(canvas), inputs_(inputs) {
    setWindowTitle("ARCADE / 400 - Qt5");
    resize(Frame::Width, Frame::Height);
    setFocusPolicy(Qt::StrongFocus);
}

void QtSurface::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.fillRect(rect(), QColor(10, 15, 23));
    const QImage image(reinterpret_cast<const uchar *>(canvas_.pixels().data()), Frame::Width,
                       Frame::Height, QImage::Format_RGB32);
    auto size = image.size();
    size.scale(this->size(), Qt::KeepAspectRatio);
    painter.drawImage(QRect((width() - size.width()) / 2, (height() - size.height()) / 2,
                            size.width(), size.height()),
                      image);
}

void QtSurface::key(QKeyEvent *event, bool pressed) {
    unsigned symbol = 0;
    switch (event->key()) {
    case Qt::Key_Left:
        symbol = 0xff51;
        break;
    case Qt::Key_Up:
        symbol = 0xff52;
        break;
    case Qt::Key_Right:
        symbol = 0xff53;
        break;
    case Qt::Key_Down:
        symbol = 0xff54;
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        symbol = 0xff0d;
        break;
    case Qt::Key_Escape:
        symbol = 0xff1b;
        break;
    case Qt::Key_Tab:
        symbol = 0xff09;
        break;
    case Qt::Key_Backspace:
        symbol = 0xff08;
        break;
    default:
        if (event->key() >= Qt::Key_F2 && event->key() <= Qt::Key_F7)
            symbol = 0xffbfU + static_cast<unsigned>(event->key() - Qt::Key_F2);
        else if (!event->text().isEmpty())
            symbol = event->text().at(0).unicode();
    }
    const auto input = nativeKey(symbol, pressed);
    if (input.action != Action::None)
        inputs_.push_back(input);
}

void QtSurface::keyPressEvent(QKeyEvent *e) {
    key(e, true);
}

void QtSurface::keyReleaseEvent(QKeyEvent *e) {
    if (!e->isAutoRepeat())
        key(e, false);
}

void QtSurface::closeEvent(QCloseEvent *e) {
    inputs_.push_back({Action::Quit});
    e->ignore();
}

Qt5::~Qt5() {
    close();
}

void Qt5::open() {
    if (window_)
        return;
    if (!QApplication::instance())
        application_ = std::make_unique<QApplication>(argc_, argv_);
    QApplication::setQuitOnLastWindowClosed(false);
    window_ = std::make_unique<QtSurface>(canvas_, inputs_);
    window_->show();
    QApplication::processEvents();
}

void Qt5::close() noexcept {
    window_.reset();
    inputs_.clear();
    application_.reset();
}

std::vector<Input> Qt5::poll() {
    QApplication::processEvents();
    return std::exchange(inputs_, {});
}

void Qt5::draw(const Frame &frame) {
    if (!window_)
        throw std::runtime_error("Qt5 display is closed");
    canvas_.render(frame);
    window_->repaint();
    QApplication::processEvents();
}
} // namespace arcade::bonus
ARCADE_PLUGIN(arcade::bonus::Qt5, arcade::PluginKind::Graphics, "qt5", "QT5 / NATIVE WIDGET")
