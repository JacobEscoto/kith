#include <QApplication>
#include <QDebug>
#include <exception>

#include "CalendarWidget.h"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  QApplication::setQuitOnLastWindowClosed(false);
  QApplication::setApplicationName("kith");
  QApplication::setOrganizationName("kith");

  try {
    CalendarWidget calendar;

    calendar.resize(260, 220);
    calendar.show();

    return QApplication::exec();
  } catch (const std::exception &ex) {
    qCritical();
    return 1;
  } catch (...) {
    qCritical();
    return 1;
  }
}
