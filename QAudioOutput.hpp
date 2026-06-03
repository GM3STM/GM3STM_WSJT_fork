#pragma once


#include <QtCore/qobject.h>
#include <QtCore/qtimer.h>
#include <QtMultimedia/qaudiosink.h>

#include "QAudioDeviceInfo"

QT_BEGIN_NAMESPACE

class QAudioOutput final
  : public QObject
{
  Q_OBJECT

public:
  explicit QAudioOutput (QAudioFormat const& format = QAudioFormat {}, QObject * parent = nullptr)
    : QObject {parent}
    , sink_ {new QAudioSink {format, this}}
  {
    initialize ();
  }

  explicit QAudioOutput (QAudioDeviceInfo const& device, QAudioFormat const& format = QAudioFormat {}, QObject * parent = nullptr)
    : QObject {parent}
    , sink_ {new QAudioSink {device.device (), format, this}}
  {
    initialize ();
  }

  QAudioFormat format () const
  {
    return sink_->format ();
  }

  void start (QIODevice * device)
  {
    sink_->start (device);
    update_notify_timer ();
  }

  QIODevice * start ()
  {
    auto * device = sink_->start ();
    update_notify_timer ();
    return device;
  }

  void stop ()
  {
    sink_->stop ();
    notify_timer_.stop ();
  }

  void reset ()
  {
    sink_->reset ();
  }

  void suspend ()
  {
    sink_->suspend ();
  }

  void resume ()
  {
    sink_->resume ();
    update_notify_timer ();
  }

  void setBufferSize (qsizetype bytes)
  {
    sink_->setBufferSize (bytes);
  }

  qsizetype bufferSize () const
  {
    return sink_->bufferSize ();
  }

  qsizetype bytesFree () const
  {
    return sink_->bytesFree ();
  }

  qint64 processedUSecs () const
  {
    return sink_->processedUSecs ();
  }

  qint64 elapsedUSecs () const
  {
    return sink_->elapsedUSecs ();
  }

  QAudio::Error error () const
  {
    return sink_->error ();
  }

  QAudio::State state () const
  {
    return sink_->state ();
  }

  void setVolume (qreal volume)
  {
    sink_->setVolume (volume);
  }

  qreal volume () const
  {
    return sink_->volume ();
  }

  void setNotifyInterval (int ms)
  {
    notify_interval_ms_ = ms;
    update_notify_timer ();
  }

  void setCategory (QString const&)
  {
  }

Q_SIGNALS:
  void stateChanged (QAudio::State);
  void notify ();

private:
  void initialize ()
  {
    connect (sink_, &QAudioSink::stateChanged, this, [this] (QAudio::State state) {
        emit stateChanged (state);
        update_notify_timer ();
      });
    connect (&notify_timer_, &QTimer::timeout, this, &QAudioOutput::notify);
  }

  void update_notify_timer ()
  {
    if (notify_interval_ms_ > 0 && sink_->state () == QAudio::ActiveState)
      {
        notify_timer_.start (notify_interval_ms_);
      }
    else
      {
        notify_timer_.stop ();
      }
  }

  QAudioSink * sink_;
  QTimer notify_timer_;
  int notify_interval_ms_ {0};
};

QT_END_NAMESPACE
