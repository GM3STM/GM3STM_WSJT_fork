#include "soundout.h"

#include <QAudioDeviceInfo>
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
#include <QAudioSink>
#else
#include <QAudioOutput>
#endif
#include <QSysInfo>
#include <qmath.h>
#include <QDebug>

#include "Logger.hpp"
#include "Audio/AudioDevice.hpp"

#include "moc_soundout.cpp"

bool SoundOutput::checkStream () const
{
  bool result {false};

  Q_ASSERT_X (m_stream, "SoundOutput", "programming error");
  if (m_stream) {
    switch (m_stream->error ())
      {
      case QAudio::OpenError:
        Q_EMIT error (tr ("An error opening the audio output device has occurred."));
        break;

      case QAudio::IOError:
        Q_EMIT error (tr ("An error occurred during write to the audio output device."));
        break;

      case QAudio::UnderrunError:
        Q_EMIT error (tr ("Audio data not being fed to the audio output device fast enough."));
        break;

      case QAudio::FatalError:
        Q_EMIT error (tr ("Non-recoverable error, audio output device not usable at this time."));
        break;

      case QAudio::NoError:
        result = true;
        break;
      }
  }
  return result;
}

void SoundOutput::setFormat (QAudioDeviceInfo const& device, unsigned channels, int frames_buffered)
{
  Q_ASSERT (0 < channels && channels < 3);
  m_device = device;
  m_channels = channels;
  m_framesBuffered = frames_buffered;
}

void SoundOutput::restart (QIODevice * source)
{
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
  m_pump_timer.stop ();
  m_source.clear ();
  m_output_device = nullptr;
#endif
  if (!m_device.isNull ())
    {
      QAudioFormat format (m_device.preferredFormat ());
      //  qDebug () << "Preferred audio output format:" << format;
      format.setChannelCount (m_channels);
      format.setSampleRate (48000);
      format.setSampleFormat (QAudioFormat::Int16);
      if (!format.isValid ())
        {
          Q_EMIT error (tr ("Requested output audio format is not valid."));
        }
      else if (!m_device.isFormatSupported (format))
        {
          Q_EMIT error (tr ("Requested output audio format is not supported on device."));
        }
      else
        {
          // qDebug () << "Selected audio output format:" << format;
          m_stream.reset (new
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
                          QAudioSink {m_device.device (), format}
#else
                          QAudioOutput {m_device, format}
#endif
                          );
          checkStream ();
          m_stream->setVolume (m_volume);
#if QT_VERSION < QT_VERSION_CHECK (6, 0, 0)
          m_stream->setNotifyInterval(1000);
#endif
          error_ = false;

          connect (m_stream.data(),
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
                   &QAudioSink::stateChanged,
#else
                   &QAudioOutput::stateChanged,
#endif
                   this, &SoundOutput::handleStateChanged);
#if QT_VERSION < QT_VERSION_CHECK (6, 0, 0)
          connect (m_stream.data(), &QAudioOutput::notify, [this] () {checkStream ();});
#else
          connect (&m_pump_timer, &QTimer::timeout, this, &SoundOutput::pumpAudio, Qt::UniqueConnection);
#endif

          //      qDebug() << "A" << m_volume << m_stream->notifyInterval();
        }
    }
  if (!m_stream)
    {
      if (!error_)
        {
          error_ = true;        // only signal error once
          Q_EMIT error (tr ("No audio output device configured."));
        }
      return;
    }
  else
    {
      error_ = false;
    }

  // we have to set this before every start on the stream because the
  // Windows implementation seems to forget the buffer size after a
  // stop.
  //qDebug () << "SoundOut default buffer size (bytes):" << m_stream->bufferSize () << "period size:" << m_stream->periodSize ();
  if (m_framesBuffered > 0)
    {
      m_stream->setBufferSize (m_stream->format().bytesForFrames (m_framesBuffered));
    }
#if QT_VERSION < QT_VERSION_CHECK (6, 0, 0)
  m_stream->setCategory ("production");
  m_stream->start (source);
#else
  m_source = source;
  m_output_device = m_stream->start ();
  pumpAudio ();
  m_pump_timer.start (10);
#endif
//  LOG_DEBUG ("Selected buffer size (bytes): " << m_stream->bufferSize () << " period size: " << m_stream->periodSize ());
}

void SoundOutput::suspend ()
{
  if (m_stream && QAudio::ActiveState == m_stream->state ())
    {
      m_stream->suspend ();
      checkStream ();
    }
}

void SoundOutput::resume ()
{
  if (m_stream && QAudio::SuspendedState == m_stream->state ())
    {
      m_stream->resume ();
      checkStream ();
    }
}

void SoundOutput::reset ()
{
  if (m_stream)
    {
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
      m_pump_timer.stop ();
      m_output_device = nullptr;
      m_source.clear ();
#endif
      m_stream->reset ();
      checkStream ();
    }
}

void SoundOutput::stop ()
{
  if (m_stream)
    {
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
      m_pump_timer.stop ();
      m_output_device = nullptr;
      m_source.clear ();
#endif
      m_stream->reset ();
      m_stream->stop ();
    }
#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
  m_stream.reset ();
#elif defined (__APPLE__)
  // this code is here to help certain rigs not drop audio, however on Sequoia this causes audio to drop, so we don't do it!
  if( QOperatingSystemVersion::current() < QOperatingSystemVersion(QOperatingSystemVersion::MacOS, 15) )
    m_stream.reset ();
#endif
}

qreal SoundOutput::attenuation () const
{
  return -(20. * qLn (m_volume) / qLn (10.));
}

void SoundOutput::setAttenuation (qreal a)
{
  Q_ASSERT (0. <= a && a <= 999.);
  m_volume = qPow(10.0, -a/20.0);
  //  qDebug () << "SoundOut: attn = " << a << ", vol = " << m_volume;
  if (m_stream)
    {
      m_stream->setVolume (m_volume);
    }
}

void SoundOutput::resetAttenuation ()
{
  m_volume = 1.;
  if (m_stream)
    {
      m_stream->setVolume (m_volume);
    }
}

void SoundOutput::handleStateChanged (QAudio::State newState)
{
  switch (newState)
    {
    case QAudio::IdleState:
      Q_EMIT status (tr ("Idle"));
      break;

    case QAudio::ActiveState:
      Q_EMIT status (tr ("Sending"));
      break;

    case QAudio::SuspendedState:
      Q_EMIT status (tr ("Suspended"));
      break;

#if QT_VERSION >= QT_VERSION_CHECK (5, 10, 0) && QT_VERSION < QT_VERSION_CHECK (6, 0, 0)
    case QAudio::InterruptedState:
      Q_EMIT status (tr ("Interrupted"));
      break;
#endif

    case QAudio::StoppedState:
      if (!checkStream ())
        {
          Q_EMIT status (tr ("Error"));
        }
      else
        {
          Q_EMIT status (tr ("Stopped"));
        }
      break;
    }
}

#if QT_VERSION >= QT_VERSION_CHECK (6, 0, 0)
void SoundOutput::pumpAudio ()
{
  if (!m_stream || !m_output_device || !m_source)
    {
      return;
    }

  auto bytes_to_write = m_stream->bytesFree ();
  auto const frame_size = qMax (1, m_stream->format ().bytesPerFrame ());
  bytes_to_write -= bytes_to_write % frame_size;
  if (bytes_to_write <= 0)
    {
      return;
    }

  m_pump_buffer.resize (static_cast<int> (bytes_to_write));
  auto const bytes_read = m_source->read (m_pump_buffer.data (), bytes_to_write);
  if (bytes_read > 0)
    {
      auto const bytes_written = m_output_device->write (m_pump_buffer.constData (), bytes_read);
      if (m_stream
          && bytes_written > 0
          && (QAudio::IdleState == m_stream->state ()
              || QAudio::SuspendedState == m_stream->state ()))
        {
          m_stream->resume ();
        }
    }
}
#endif
