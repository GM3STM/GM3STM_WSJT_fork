#define AppName "GM3STM WSJT Fork"
#define AppPublisher "GM3STM"
#define AppExeName "wsjtx.exe"

#ifndef StageDir
  #error StageDir must be supplied by package-windows.ps1
#endif

#ifndef OutputDir
  #error OutputDir must be supplied by package-windows.ps1
#endif

#ifndef AppVersion
  #define AppVersion "0.1.1 beta"
#endif

#ifndef PackageVersion
  #define PackageVersion "0.1.1-beta"
#endif

#define StagePath AddBackslash(StageDir)

[Setup]
AppId={{B6431E1F-1784-4CE0-8E95-7AE6B2769D1B}
AppName={#AppName}
AppVersion={#AppVersion}
AppVerName={#AppName} {#AppVersion}
AppPublisher={#AppPublisher}
AppCopyright=Copyright (C) 2001-2026 by the WSJT-X contributors
DefaultDirName={autopf}\GM3STM WSJT Fork
DefaultGroupName={#AppName}
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename=GM3STM-WSJT-Fork-{#PackageVersion}-win64-setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
CloseApplications=yes
RestartApplications=no
SetupIconFile={#StagePath}wsjt.ico
UninstallDisplayIcon={app}\wsjtx.exe
VersionInfoVersion=0.1.1.0
VersionInfoCompany={#AppPublisher}
VersionInfoDescription={#AppName} installer
VersionInfoProductName={#AppName}
VersionInfoProductVersion=0.1.1.0

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#StagePath}*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#AppName}"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\wsjt.ico"
Name: "{autodesktop}\{#AppName}"; Filename: "{app}\{#AppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\wsjt.ico"; Tasks: desktopicon

[Run]
Filename: "{app}\{#AppExeName}"; Description: "Launch {#AppName}"; Flags: nowait postinstall skipifsilent
