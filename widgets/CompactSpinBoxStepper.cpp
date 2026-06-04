#include "CompactSpinBoxStepper.hpp"

#include <QAbstractSpinBox>
#include <QComboBox>
#include <QEvent>
#include <QPainter>
#include <QPalette>
#include <QSlider>
#include <QToolButton>
#include <QWidget>
#include <QtGlobal>

namespace
{
  class CompactSpinBoxStepper final : public QObject
  {
  public:
    explicit CompactSpinBoxStepper (QAbstractSpinBox * spin_box)
      : QObject {spin_box}
      , spin_box_ {spin_box}
      , up_button_ {new QToolButton {spin_box}}
      , down_button_ {new QToolButton {spin_box}}
    {
      spin_box_->setButtonSymbols (QAbstractSpinBox::NoButtons);
      spin_box_->setProperty ("compactStepper", "true");
      setup_button (up_button_, "up", QString::fromUtf8 ("\u25B2"));
      setup_button (down_button_, "down", QString::fromUtf8 ("\u25BC"));
      QObject::connect (up_button_, &QToolButton::clicked, spin_box_, [this] {
        spin_box_->stepBy (1);
      });
      QObject::connect (down_button_, &QToolButton::clicked, spin_box_, [this] {
        spin_box_->stepBy (-1);
      });
      spin_box_->installEventFilter (this);
      update_geometry ();
    }

  protected:
    bool eventFilter (QObject * object, QEvent * event) override
    {
      if (object == spin_box_
          && (event->type () == QEvent::Resize
              || event->type () == QEvent::Show
              || event->type () == QEvent::EnabledChange
              || event->type () == QEvent::StyleChange))
        {
          update_geometry ();
        }
      return QObject::eventFilter (object, event);
    }

  private:
    void setup_button (QToolButton * button, char const * step, QString const& text)
    {
      button->setProperty ("role", "spin-stepper");
      button->setProperty ("step", step);
      button->setText (text);
      button->setAutoRepeat (true);
      button->setCursor (Qt::ArrowCursor);
      button->setFocusPolicy (Qt::NoFocus);
      button->show ();
    }

    void update_geometry ()
    {
      auto const rect = spin_box_->rect ();
      int const width = 17;
      int const top = 1;
      int const available_height = qMax (18, rect.height () - 2);
      int const button_height = available_height / 2;
      up_button_->setEnabled (spin_box_->isEnabled ());
      down_button_->setEnabled (spin_box_->isEnabled ());
      up_button_->setGeometry (rect.right () - width, top, width, button_height);
      down_button_->setGeometry (rect.right () - width, top + button_height, width, available_height - button_height);
      up_button_->raise ();
      down_button_->raise ();
    }

    QAbstractSpinBox * spin_box_;
    QToolButton * up_button_;
    QToolButton * down_button_;
  };

  class CompactComboBoxDropDown final : public QObject
  {
  public:
    explicit CompactComboBoxDropDown (QComboBox * combo_box)
      : QObject {combo_box}
      , combo_box_ {combo_box}
      , button_ {new QToolButton {combo_box}}
    {
      combo_box_->setProperty ("compactDropdown", "true");
      button_->setProperty ("role", "combo-dropdown");
      button_->setText (QString::fromUtf8 ("\u25BC"));
      button_->setCursor (Qt::ArrowCursor);
      button_->setFocusPolicy (Qt::NoFocus);
      QObject::connect (button_, &QToolButton::clicked, combo_box_, [this] {
        combo_box_->showPopup ();
      });
      combo_box_->installEventFilter (this);
      button_->show ();
      update_geometry ();
    }

  protected:
    bool eventFilter (QObject * object, QEvent * event) override
    {
      if (object == combo_box_
          && (event->type () == QEvent::Resize
              || event->type () == QEvent::Show
              || event->type () == QEvent::EnabledChange
              || event->type () == QEvent::StyleChange))
        {
          update_geometry ();
        }
      return QObject::eventFilter (object, event);
    }

  private:
    void update_geometry ()
    {
      auto const rect = combo_box_->rect ();
      int const width = 17;
      int const top = 1;
      int const height = qMax (18, rect.height () - 2);
      button_->setEnabled (combo_box_->isEnabled ());
      button_->setGeometry (rect.right () - width, top, width, height);
      button_->raise ();
    }

    QComboBox * combo_box_;
    QToolButton * button_;
  };

  class SliderTickMarkers final : public QWidget
  {
  public:
    explicit SliderTickMarkers (QSlider * slider)
      : QWidget {slider}
      , slider_ {slider}
    {
      slider_->setProperty ("sliderTickMarkers", "true");
      setAttribute (Qt::WA_TransparentForMouseEvents);
      setAttribute (Qt::WA_NoSystemBackground);
      slider_->installEventFilter (this);
      show ();
      update_geometry ();
    }

  protected:
    bool eventFilter (QObject * object, QEvent * event) override
    {
      if (object == slider_
          && (event->type () == QEvent::Resize
              || event->type () == QEvent::Show
              || event->type () == QEvent::EnabledChange
              || event->type () == QEvent::StyleChange
              || event->type () == QEvent::PaletteChange))
        {
          update_geometry ();
          update ();
        }
      return QWidget::eventFilter (object, event);
    }

    void paintEvent (QPaintEvent *) override
    {
      int const interval = marker_interval ();
      int const min = slider_->minimum ();
      int const max = slider_->maximum ();
      if (interval <= 0 || max <= min)
        {
          return;
        }

      QPainter painter {this};
      painter.setRenderHint (QPainter::Antialiasing, false);
      auto color = slider_->palette ().color (QPalette::Mid);
      color.setAlpha (170);
      painter.setPen (QPen {color, 1});

      if (slider_->orientation () == Qt::Horizontal)
        {
          int const left = 8;
          int const right = qMax (left + 1, width () - 8);
          int const y = height () / 2;
          for (int value = first_marker (min, interval); value <= max; value += interval)
            {
              qreal const ratio = (value - min) / double (max - min);
              int const x = left + qRound (ratio * (right - left));
              int const len = (value == min || value == max || value == 0) ? 7 : 4;
              painter.drawLine (x, y - len, x, y - 2);
              painter.drawLine (x, y + 2, x, y + len);
            }
        }
      else
        {
          int const top = 8;
          int const bottom = qMax (top + 1, height () - 8);
          int const x = width () / 2;
          for (int value = first_marker (min, interval); value <= max; value += interval)
            {
              qreal const ratio = (value - min) / double (max - min);
              int const y = bottom - qRound (ratio * (bottom - top));
              int const len = (value == min || value == max || value == 0) ? 7 : 4;
              painter.drawLine (x - len, y, x - 2, y);
              painter.drawLine (x + 2, y, x + len, y);
            }
        }
    }

  private:
    int marker_interval () const
    {
      if (slider_->tickInterval () > 0)
        {
          return slider_->tickInterval ();
        }

      int const range = slider_->maximum () - slider_->minimum ();
      if (range <= 0)
        {
          return 0;
        }
      return qMax (1, range / 10);
    }

    static int first_marker (int minimum, int interval)
    {
      int const remainder = minimum % interval;
      if (remainder == 0)
        {
          return minimum;
        }
      return minimum < 0 ? minimum - remainder : minimum + interval - remainder;
    }

    void update_geometry ()
    {
      setGeometry (slider_->rect ());
      raise ();
    }

    QSlider * slider_;
  };
}

void install_compact_dropdown (QComboBox * combo_box)
{
  if (!combo_box || combo_box->property ("compactDropdown").toBool ())
    {
      return;
    }
  new CompactComboBoxDropDown {combo_box};
}

void install_compact_stepper (QAbstractSpinBox * spin_box)
{
  if (!spin_box || spin_box->property ("compactStepper").toBool ())
    {
      return;
    }
  new CompactSpinBoxStepper {spin_box};
}

void install_slider_tick_markers (QSlider * slider)
{
  if (!slider || slider->property ("sliderTickMarkers").toBool ())
    {
      return;
    }

  if (slider->tickInterval () <= 0)
    {
      int const range = slider->maximum () - slider->minimum ();
      slider->setTickInterval (qMax (1, range / 10));
    }
  if (slider->tickPosition () == QSlider::NoTicks)
    {
      slider->setTickPosition (slider->orientation () == Qt::Horizontal
                               ? QSlider::TicksAbove
                               : QSlider::TicksLeft);
    }
  new SliderTickMarkers {slider};
}

void install_compact_steppers (QWidget * root)
{
  if (!root)
    {
      return;
    }

  for (auto * spin_box : root->findChildren<QAbstractSpinBox *> ())
    {
      install_compact_stepper (spin_box);
    }

  for (auto * combo_box : root->findChildren<QComboBox *> ())
    {
      install_compact_dropdown (combo_box);
    }

  for (auto * slider : root->findChildren<QSlider *> ())
    {
      install_slider_tick_markers (slider);
    }
}
