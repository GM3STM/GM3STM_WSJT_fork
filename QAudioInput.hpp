#pragma once


#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>
#include <QtMultimedia/qaudiosource.h>

#include "QAudioDeviceInfo"

QT_BEGIN_NAMESPACE

class QAudioInput final
  : public QObject
{
  Q_OBJECT

public:
  explicit QAudioInput (QAudioFormat const& format = QAudioFormat {}, QObject * parent = nullptr)
    : QObject {parent}
    , source_ {new QAudioSource {format, this}}
  {
    initialize ();
  }

  explicit QAudioInput (QAudioDeviceInfo const& device, QAudioFormat const& format = QAudioFormat {}, QObject * parent = nullptr)
    : QObject {parent}
    , source_ {new QAudioSource {device.device (), format, this}}
  {
    initialize ();
  }

  QAudioFormat format () const
  {
    return source_->format ();
  }

  void start (QIODevice * device)
  {
    source_->start (device);
    update_notify_timer ();
  }

  QIODevice * start ()
  {
    auto * device = source_->start ();
    update_notify_timer ();
    return device;
  }

  void stop ()
  {
    source_->stop ();
    notify_timer_.stop ();
  }

  void reset ()
  {
    source_->reset ();
  }

  void suspend ()
  {
    source_->suspend ();
  }

  void resume ()
  {
    source_->resume ();
    update_notify_timer ();
  }

  void setBufferSize (qsizetype bytes)
  {
    source_->setBufferSize (bytes);
  }

  qsizetype bufferSize () const
  {
    return source_->bufferSize ();
  }

  qsizetype bytesAvailable () const
  {
    return source_->bytesAvailable ();
  }

  qint64 processedUSecs () const
  {
    return source_->processedUSecs ();
  }

  qint64 elapsedUSecs () const
  {
    return source_->elapsedUSecs ();
  }

  QAudio::Error error () const
  {
    return source_->error ();
  }

  QAudio::State state () const
  {
    return source_->state ();
  }

  void setNotifyInterval (int ms)
  {
    notify_interval_ms_ = ms;
    update_notify_timer ();
  }

Q_SIGNALS:
  void stateChanged (QAudio::State);
  void notify ();

private:
  void initialize ()
  {
    connect (source_, &QAudioSource::stateChanged, this, [this] (QAudio::State state) {
        emit stateChanged (state);
        update_notify_timer ();
      });
    connect (&notify_timer_, &QTimer::timeout, this, &QAudioInput::notify);
  }

  void update_notify_timer ()
  {
    if (notify_interval_ms_ > 0 && source_->state () == QAudio::ActiveState)
      {
        notify_timer_.start (notify_interval_ms_);
      }
    else
      {
        notify_timer_.stop ();
      }
  }

  QAudioSource * source_;
  QTimer notify_timer_;
  int notify_interval_ms_ {0};
};

QT_END_NAMESPACE
