// -*- Mode: C++ -*-
#ifndef SOUNDOUT_H__
#define SOUNDOUT_H__

#include <QObject>
#include <QPointer>
#include <QString>
#include <QTimer>
#include <QAudioDeviceInfo>
#include <QOperatingSystemVersion>
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
#include <QAudioSink>
#else
#include <QAudioOutput>
#endif

class QIODevice;
class QAudioDeviceInfo;

// An instance of this sends audio data to a specified soundcard.

class SoundOutput
  : public QObject
{
  Q_OBJECT;
  
public:
  SoundOutput ()
    : m_framesBuffered {0}
    , m_volume {1.0}
    , error_ {false}
  {
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
    m_pump_timer.setParent (this);
    m_pump_timer.setTimerType (Qt::PreciseTimer);
#endif
  }

  qreal attenuation () const;

public Q_SLOTS:
  void setFormat (QAudioDeviceInfo const& device, unsigned channels, int frames_buffered = 0);
  void restart (QIODevice *);
  void suspend ();
  void resume ();
  void reset ();
  void stop ();
  void setAttenuation (qreal);	/* unsigned */
  void resetAttenuation ();	/* to zero */
  
Q_SIGNALS:
  void error (QString message) const;
  void status (QString message) const;

private:
  bool checkStream () const;

private Q_SLOTS:
  void handleStateChanged (QAudio::State);
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
  void pumpAudio ();
#endif

private:
  QAudioDeviceInfo m_device;
  unsigned m_channels;
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
  QScopedPointer<QAudioSink> m_stream;
  QPointer<QIODevice> m_source;
  QIODevice * m_output_device {nullptr};
  QTimer m_pump_timer;
  QByteArray m_pump_buffer;
#else
  QScopedPointer<QAudioOutput> m_stream;
#endif
  int m_framesBuffered;
  qreal m_volume;
  bool error_;
};

#endif
