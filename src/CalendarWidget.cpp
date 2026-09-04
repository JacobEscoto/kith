#include "CalendarWidget.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFrame>
#include <QLocale>
#include <QMouseEvent>
#include <QStandardPaths>
#include <QStyle>

namespace {
constexpr const char *kUserStyleSheetName = "style.qss";
}

CalendarWidget::CalendarWidget(QWidget *parent) : QWidget(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::Tool |
                 Qt::WindowStaysOnBottomHint);

  setAttribute(Qt::WA_TranslucentBackground, true);
  setAttribute(Qt::WA_NoSystemBackground, true);

  buildUI();
  applyStyleSheet();
  refreshCalendar();
}

void CalendarWidget::buildUI() {
  auto *outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(0, 0, 0, 0);

  m_container = new QWidget(this);
  m_container->setObjectName("container");

  m_container->setAttribute(Qt::WA_StyledBackground, true);
  outerLayout->addWidget(m_container);

  auto *containerLayout = new QVBoxLayout(m_container);
  containerLayout->setContentsMargins(12, 10, 12, 12);
  containerLayout->setSpacing(true);

  m_monthYearLabel = new QLabel(m_container);
  m_monthYearLabel->setObjectName("monthYearLabel");
  m_monthYearLabel->setAlignment(Qt::AlignCenter);
  containerLayout->addWidget(m_monthYearLabel);

  m_divider = new QFrame(m_container);
  m_divider->setObjectName("divider");
  m_divider->setFrameShape(QFrame::HLine);
  m_divider->setFixedHeight(1);
  containerLayout->addSpacing(8);
  containerLayout->addWidget(m_divider);
  containerLayout->addSpacing(8);

  m_bottomPanel = new QWidget(m_container);
  m_bottomPanel->setObjectName("bottomPanel");
  containerLayout->addWidget(m_bottomPanel);

  m_gridLayout = new QGridLayout(m_bottomPanel);
  m_gridLayout->setContentsMargins(0, 0, 0, 0);
  m_gridLayout->setHorizontalSpacing(6);
  m_gridLayout->setVerticalSpacing(6);

  for (int col = 0; col < kCols; ++col) {
    auto *header = new QLabel(kDayHeaders[col], m_bottomPanel);
    header->setProperty("header", true);
    header->setAlignment(Qt::AlignCenter);
    m_gridLayout->addWidget(header, 0, col);
  }

  for (int row = 0; row < kRows; ++row) {
    for (int col = 0; col < kCols; ++col) {
      auto *cell = new QLabel(m_bottomPanel);
      cell->setObjectName("dayCell");
      cell->setAlignment(Qt::AlignCenter);
      cell->setMinimumSize(28, 24);
      m_gridLayout->addWidget(cell, row + 1, col);
      m_dayCells[row][col] = cell;
    }
  }

  m_midnightTimer = new QTimer(this);
  m_midnightTimer->setSingleShot(true);
  connect(m_midnightTimer, &QTimer::timeout, this,
          &CalendarWidget::refreshCalendar);
}

void CalendarWidget::applyStyleSheet() {
  const QString configDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  const QString userStylePath = configDir + "/" + kUserStyleSheetName;

  QFile userFile(userStylePath);
  if (userFile.exists() &&
      userFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    setStyleSheet(QString::fromUtf8(userFile.readAll()));
    userFile.close();
    qInfo() << "Loading custom styles from" << userStylePath;
    return;
  }

  QFile bundled(":/style.qss");
  if (bundled.open(QIODevice::ReadOnly | QIODevice::Text)) {
    const QByteArray contents = bundled.readAll();
    bundled.close();

    QDir().mkpath(configDir);
    QFile copy(userStylePath);
    if (copy.open(QIODevice::ReadOnly | QIODevice::Text)) {
      copy.write(contents);
      copy.close();
      qInfo() << "Copying QSS by default to" << userStylePath;
    }

    setStyleSheet(QString::fromUtf8(contents));
  } else {
    qWarning() << "Could not load the user QSS; "
                  "The widget will use the default Qt style.";
  }
}

void CalendarWidget::refreshCalendar() {
  const QDate today = QDate::currentDate();
  populateGrid(today);
  scheduleNextMidnightRefresh();
}

void CalendarWidget::populateGrid(const QDate &today) {
  QString monthYear = QLocale(QLocale::English).toString(today, "MMMM yyyy");
  if (!monthYear.isEmpty()) {
    monthYear[0] = monthYear[0].toUpper();
  }

  m_monthYearLabel->setText(monthYear);

  const QDate firstOfMonth(today.year(), today.month(), 1);
  const int daysInMonth = firstOfMonth.daysInMonth();

  const int firstDayColumn = firstOfMonth.dayOfWeek() - 1;

  for (int row = 0; row < kRows; ++row) {
    for (int col = 0; col < kCols; ++col) {
      QLabel *cell = m_dayCells[row][col];
      cell->clear();
      cell->setProperty("today", false);
      cell->style()->unpolish(cell);
      cell->style()->polish(cell);
    }
  }

  int day = 1;

  for (int row = 0; row < kRows && day <= daysInMonth; ++row) {
    for (int col = 0; col < kCols && day <= daysInMonth; ++col) {
      if (row == 0 && col < firstDayColumn) {
        continue;
      }

      QLabel *cell = m_dayCells[row][col];
      cell->setText(QString::number(day));

      const bool isToday = (day == today.day());
      cell->setProperty("today", isToday);
      cell->style()->unpolish(cell);
      cell->style()->polish(cell);

      ++day;
    }
  }
}

void CalendarWidget::scheduleNextMidnightRefresh() {
  const QDateTime now = QDateTime::currentDateTime();

  // 5-second period to prevent race conditions with system clock updates right
  // at midnight.
  QDateTime nextMidnight = QDateTime(now.date().addDays(1), QTime(0, 0, 5));
  qint64 msUntilMidnight = now.msecsTo(nextMidnight);

  constexpr qint64 kOneHourMs = 60 * 60 * 1000;
  if (msUntilMidnight <= 0 || msUntilMidnight > 25 * kOneHourMs) {
    msUntilMidnight = kOneHourMs;
  }

  m_midnightTimer->start(static_cast<int>(msUntilMidnight));
}

namespace {
inline QPoint globalPosOf(QMouseEvent *event) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
  return event->globalPosition().toPoint();
#else
  return event->globalPos();
#endif
}
} // namespace

void CalendarWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragging = true;
    m_dragStartPosition = globalPosOf(event) - frameGeometry().topLeft();
    event->accept();
  }
}

void CalendarWidget::mouseMoveEvent(QMouseEvent *event) {
  if (m_dragging && (event->buttons() & Qt::LeftButton)) {
    move(globalPosOf(event) - m_dragStartPosition);
    event->accept();
  }
}

void CalendarWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_dragging = false;
    event->accept();
  }
}
