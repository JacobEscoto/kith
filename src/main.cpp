#include <QApplication>
#include <QDebug>
#include <exception>

#include "CalendarWidget.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QApplication::setQuitOnLastWindowClosed(false);
  QApplication::setApplicationName("kith");

  try {
    CalendarWidget calendar;

    calendar.resize(260, 220);
    calendar.show();

    return QApplication::exec();
  } catch (const std::exception &ex) {
    qCritical() << "unhandled exception, securely closed:" << ex.what();
    return 1;
  } catch (...) {
    qCritical() << "unhandled exception, securely closed.";
    return 1;
  }
}
