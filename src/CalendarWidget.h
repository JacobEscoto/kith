#pragma once

#include <QDate>
#include <QGridLayout>
#include <QLabel>
#include <QPoint>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <array>

class QFrame;
class QMouseEvent;

class CalendarWidget : public QWidget {
  Q_OBJECT

public:
  explicit CalendarWidget(QWidget *parent = nullptr);
  ~CalendarWidget() override = default;

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private slots:

  /*
   * @brief Slot that updates the current date and starts the refresh cycle.
   */
  void refreshCalendar();

private:
  void buildUI();

  /*
   * @brief Loads and applies a QSS style template of the widget.
   *
   * Searches for the 'style.qss' file in the user configuration folder.
   * If not exists, it automatically generates a copy by default of the
   * integrated resource.
   */
  void applyStyleSheet();

  /*
   * @brief Calculates the time remaining until the next midnight and schedule a
   * timer.
   *
   * Sets m_midnightTimer to trigger at 00:00:05 on the following day, allowing
   * the calendar to automatically update the highlighted date without user
   * intervention.
   */
  void scheduleNextMidnightRefresh();
  void populateGrid(const QDate &today);

  QPoint m_dragStartPosition;
  bool m_dragging = false;

  QWidget *m_container = nullptr;
  QLabel *m_monthYearLabel = nullptr;
  QFrame *m_divider = nullptr;
  QWidget *m_bottomPanel = nullptr;
  QGridLayout *m_gridLayout = nullptr;

  static constexpr int kRows = 6;
  static constexpr int kCols = 7;
  std::array<std::array<QLabel *, kCols>, kRows> m_dayCells{};

  static constexpr std::array<const char *, kCols> kDayHeaders = {
      "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};

  QTimer *m_midnightTimer = nullptr;
};
