A Beta Qt6 port of WSJT-X, laying the groundwork for a more modern UI. Unofficial and not part of the official WSJT-X release stream.  USE AT OWN RISK

# Weak Signal Communication Software

## Programs
*WSJT-X*, *MAP65*, and *QMAP* are open-source, multi-platform programs designed for weak-signal digital communication by amateur radio. *WSJT-X* works with a standard SSB transceiver, while *MAP65* and *QMAP* use wideband SDR-style hardware. The programs are open source, free of charge, and licensed under the GNU General Public License.

*WSJT-X* implements communication protocols or "modes" called **FST4**, **FST4W**, **FT4**, **FT8**, **JT4**, **JT9**, **JT65**, **MSK144**, **Q65**, **WSPR**, and **Echo**. The first nine modes were designed for making reliable, confirmed QSOs in a wide variety of weak-signal propagation circumstances. These modes use timed Transmit/Receive sequences of specific lengths, synchronized with UTC. **WSPR** mode is for probing potential propagation paths with low-power transmissions, and **Echo** mode is for detecting and measuring reflections of your own signals from the Moon.

*MAP65* implements a wideband receiver for **JT65** and **Q65** signals, optimized for EME on the VHF/UHF bands. It can be used together with *Linrad* (by SM5BSZ) or with direct input from a soundcard, FUNcube Dongle, or similar hardware. The program decodes all **JT65** or **Q65** signals in a passband up to 90 kHz wide, producing a sorted band map of decoded callsigns. In a dual-polarization system, *MAP65* optimally matches the linear polarization angle of each signal, thereby eliminating problems with Faraday rotation and spatial polarization offsets. *MAP65* also handles T/R switching and generates suitable messages and audio waveforms for the selected mode.

*QMAP* is similar to *MAP65* in providing wideband reception of signals over a full EME sub-band. It works cooperatively with *WSJT-X*, supporting the **Q65** mode in both 30-second and 60-second submodes. The *QMAP* + *WSJT-X* combination provides full rig control and Doppler compensation for the EME (Earth-Moon-Earth) path.


## Mode Details
**JT4**, **JT9**, and **JT65** use nearly identical message structure, efficient compression of messages for minimal QSOs, and 60-second T/R sequences. **JT4** and **JT65** were designed for EME on the VHF, UHF, and microwave bands, while **JT9** is optimized for the MF and HF bands. **JT9** is about 2 dB more sensitive than **JT65** while using less than 10% of the bandwidth.

**FT4** and **FT8** use T/R cycles of only 7.5 and 15 s, respectively. They have become extremely popular for world-wide DXing on the HF bands. **MSK144** is designed for Meteor Scatter on the VHF bands. **Q65** offers submodes with T/R sequence lengths from 15 seconds to 5 minutes, and a wide range of tone spacings. Particular Q65 submodes are highly recommended for EME, ionospheric scatter, and other weak signal work on VHF, UHF, and microwave bands. These modes include message formats explicitly supporting nonstandard callsigns and some popular radio contests.

**FST4** and **FST4W** are designed particularly for the LF and MF bands. On these bands their fundamental sensitivities are better than other *WSJT-X* modes with the same sequence lengths, approaching the theoretical limits for their rates of information throughput. **FST4** is optimized for two-way QSOs, while **FST4W** is for quasi-beacon transmissions of **WSPR**-style messages. **FST4** and **FST4W** do not require the strict, independent time synchronization and phase locking of modes like EbNaut.
# WSJT-X Qt6 Fork

This project is a beta fork of WSJT-X that has been ported from Qt5 to Qt6 as the foundation for future modernization work. The Qt6 migration is the first major step toward a more modern look and feel in later releases, while keeping the application stable and usable in its current form.

In addition to the framework migration, this fork includes rig-control integration and stability work to support reliable day-to-day testing and ongoing development. The current state of the project should be considered beta software: functional and usable, but still under active refinement as more testing and UI improvements are completed.

## Current Focus

- Porting the application from Qt5 to Qt6
- Preserving core WSJT-X functionality during the transition
- Improving rig-control integration and runtime stability
- Preparing the codebase for future UI modernization
- Publishing development progress openly on GitHub

## Status

This repository is an early beta release. It is intended for testing, development, and feedback rather than as a final polished release. Some features, behaviors, or interface details may still change as the Qt6 transition continues.

## Goals

The goals of this fork are to:

- modernize the WSJT-X codebase by moving to Qt6
- provide a stable base for future interface improvements
- continue refining rig-control compatibility
- support further testing and iterative public development

## Notes

This project is based on WSJT-X and remains a work in progress. If you use this beta build, expect ongoing changes as development continues.
