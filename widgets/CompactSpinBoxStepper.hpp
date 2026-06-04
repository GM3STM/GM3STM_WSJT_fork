#ifndef COMPACT_SPIN_BOX_STEPPER_HPP__
#define COMPACT_SPIN_BOX_STEPPER_HPP__

class QAbstractSpinBox;
class QComboBox;
class QSlider;
class QWidget;

void install_compact_dropdown (QComboBox *);
void install_slider_tick_markers (QSlider *);
void install_compact_stepper (QAbstractSpinBox *);
void install_compact_steppers (QWidget *);

#endif
