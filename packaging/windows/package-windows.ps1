[CmdletBinding()]
param(
  [string]$Version = "0.1.0 beta",
  [string]$PackageVersion = "0.1.0-beta",
  [string]$BuildDir,
  [string]$DistRoot,
  [string]$WinDeployQt,
  [string]$InnoCompiler,
  [switch]$SkipInstaller
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Resolve-OptionalPath {
  param([string[]]$Candidates)
  foreach ($candidate in $Candidates) {
    if ($candidate -and (Test-Path -LiteralPath $candidate)) {
      return (Resolve-Path -LiteralPath $candidate).Path
    }
  }
  return $null
}

function Assert-PathUnder {
  param(
    [string]$Path,
    [string]$Parent
  )
  $resolvedParent = (Resolve-Path -LiteralPath $Parent).Path.TrimEnd('\')
  $fullPath = [System.IO.Path]::GetFullPath($Path).TrimEnd('\')
  if (-not $fullPath.StartsWith($resolvedParent, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "Refusing to use path outside '$resolvedParent': $fullPath"
  }
}

function Copy-FileIfExists {
  param(
    [string]$Path,
    [string]$Destination
  )
  if (Test-Path -LiteralPath $Path -PathType Leaf) {
    Copy-Item -LiteralPath $Path -Destination $Destination -Force
  }
}

function Copy-DirectoryIfExists {
  param(
    [string]$Path,
    [string]$Destination
  )
  if (Test-Path -LiteralPath $Path -PathType Container) {
    $name = Split-Path -Path $Path -Leaf
    $target = Join-Path $Destination $name
    if (Test-Path -LiteralPath $target) {
      Remove-Item -LiteralPath $target -Recurse -Force
    }
    Copy-Item -LiteralPath $Path -Destination $Destination -Recurse -Force
  }
}

function Convert-ToMsysPath {
  param(
    [string]$Bash,
    [string]$Path
  )
  $escaped = $Path.Replace("'", "'\''")
  return (& $Bash -lc "cygpath -u '$escaped'").Trim()
}

function Copy-MsysRuntimeDependencies {
  param(
    [string]$StageDir,
    [string]$DeployBin,
    [string]$Bash
  )

  if (-not (Test-Path -LiteralPath $Bash -PathType Leaf)) {
    Write-Warning "MSYS bash was not found; skipping recursive runtime dependency scan."
    return
  }

  $queue = [System.Collections.Generic.Queue[string]]::new()
  $scanned = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
  foreach ($file in Get-ChildItem -LiteralPath $StageDir -Recurse -File | Where-Object { $_.Extension -in @(".exe", ".dll") }) {
    $queue.Enqueue($file.FullName)
  }

  while ($queue.Count -gt 0) {
    $file = $queue.Dequeue()
    if (-not $scanned.Add($file)) {
      continue
    }

    $msysFile = Convert-ToMsysPath -Bash $Bash -Path $file
    $lddOutput = & $Bash -lc "export PATH=/ucrt64/bin:/usr/bin:`$PATH; ldd '$msysFile'" 2>$null
    foreach ($line in $lddOutput) {
      $matches = [regex]::Matches($line, "/ucrt64/bin/[^ ()]+\.dll")
      foreach ($match in $matches) {
        $dllName = Split-Path -Path $match.Value -Leaf
        $source = Join-Path $DeployBin $dllName
        $destination = Join-Path $StageDir $dllName
        if ((Test-Path -LiteralPath $source -PathType Leaf) -and -not (Test-Path -LiteralPath $destination -PathType Leaf)) {
          Copy-Item -LiteralPath $source -Destination $destination -Force
          $queue.Enqueue($destination)
          Write-Host "Added runtime dependency $dllName"
        }
      }
    }
  }
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDir = (Resolve-Path -LiteralPath (Join-Path $ScriptDir "..\..")).Path
$WorkspaceDir = (Resolve-Path -LiteralPath (Join-Path $SourceDir "..")).Path

if (-not $BuildDir) {
  $BuildDir = Join-Path $WorkspaceDir "build-qt6-msys"
}
$BuildDir = (Resolve-Path -LiteralPath $BuildDir).Path

if (-not $DistRoot) {
  $DistRoot = Join-Path $WorkspaceDir "dist"
}
New-Item -ItemType Directory -Force -Path $DistRoot | Out-Null
$DistRoot = (Resolve-Path -LiteralPath $DistRoot).Path

if (-not $WinDeployQt) {
  $WinDeployQt = Resolve-OptionalPath @(
    "D:\msys64\ucrt64\bin\windeployqt6.exe",
    "D:\msys64\ucrt64\bin\windeployqt.exe",
    "C:\msys64\ucrt64\bin\windeployqt6.exe",
    "C:\msys64\ucrt64\bin\windeployqt.exe"
  )
}
if (-not $WinDeployQt) {
  $command = Get-Command windeployqt6, windeployqt -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($command) {
    $WinDeployQt = $command.Source
  }
}
if (-not $WinDeployQt) {
  throw "Unable to find windeployqt. Pass -WinDeployQt or install Qt tools."
}

if (-not $InnoCompiler) {
  $InnoCompiler = Resolve-OptionalPath @(
    "D:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "D:\Program Files\Inno Setup 6\ISCC.exe",
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe",
    "C:\Program Files\Inno Setup 6\ISCC.exe"
  )
}
if (-not $InnoCompiler) {
  $command = Get-Command ISCC.exe -ErrorAction SilentlyContinue | Select-Object -First 1
  if ($command) {
    $InnoCompiler = $command.Source
  }
}

$StageDir = Join-Path $DistRoot "GM3STM-WSJT-Fork-$PackageVersion-win64"
$InstallerOutDir = Join-Path $DistRoot "installer"
$InnoScript = Join-Path $ScriptDir "gm3stm-wsjtfork.iss"

Assert-PathUnder -Path $StageDir -Parent $DistRoot
Assert-PathUnder -Path $InstallerOutDir -Parent $DistRoot

if (-not (Test-Path -LiteralPath (Join-Path $BuildDir "wsjtx.exe"))) {
  throw "wsjtx.exe was not found in '$BuildDir'. Build the app before packaging."
}

if (Test-Path -LiteralPath $StageDir) {
  Remove-Item -LiteralPath $StageDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $StageDir, $InstallerOutDir | Out-Null

Write-Host "Staging release files in $StageDir"

foreach ($exe in @("wsjtx.exe", "jt9.exe", "jt9code.exe", "qmap.exe")) {
  Copy-FileIfExists -Path (Join-Path $BuildDir $exe) -Destination $StageDir
}

$deployBin = Split-Path -Parent $WinDeployQt
$bash = Resolve-OptionalPath @(
  "D:\msys64\usr\bin\bash.exe",
  "C:\msys64\usr\bin\bash.exe"
)
$env:PATH = "$deployBin;$env:PATH"
Write-Host "Running windeployqt from $WinDeployQt"
& $WinDeployQt --release --compiler-runtime --dir $StageDir (Join-Path $StageDir "wsjtx.exe")
if ($LASTEXITCODE -ne 0) {
  throw "windeployqt failed with exit code $LASTEXITCODE"
}

Get-ChildItem -LiteralPath $BuildDir -Filter "*.dll" -File |
  Where-Object { $_.Name -ne "libhamlib-4_old.dll" } |
  Copy-Item -Destination $StageDir -Force

foreach ($pluginDir in @("platforms", "imageformats", "multimedia", "networkinformation", "sqldrivers", "styles", "tls", "translations", "generic")) {
  Copy-DirectoryIfExists -Path (Join-Path $BuildDir $pluginDir) -Destination $StageDir
}

Copy-MsysRuntimeDependencies -StageDir $StageDir -DeployBin $deployBin -Bash $bash

foreach ($asset in @(
  "ALLCALL7.TXT",
  "CALL3.TXT",
  "COPYING",
  "README",
  "README.md",
  "Release_Notes.txt",
  "THANKS",
  "cty.dat",
  "cty.dat_copyright.txt",
  "grid.dat",
  "jt9.txt",
  "package_description.txt",
  "sat.dat",
  "wsjtx_changelog.txt"
)) {
  Copy-FileIfExists -Path (Join-Path $SourceDir $asset) -Destination $StageDir
}

Copy-DirectoryIfExists -Path (Join-Path $SourceDir "Palettes") -Destination $StageDir
Copy-DirectoryIfExists -Path (Join-Path $SourceDir "sounds") -Destination $StageDir
Copy-FileIfExists -Path (Join-Path $SourceDir "qmap\wsjt.ico") -Destination (Join-Path $StageDir "wsjt.ico")

if (-not (Test-Path -LiteralPath (Join-Path $StageDir "qt.conf"))) {
  @"
[Paths]
Plugins=.
"@ | Set-Content -LiteralPath (Join-Path $StageDir "qt.conf") -Encoding ASCII
}

if ($SkipInstaller) {
  Write-Host "Skipping Inno Setup compilation."
  Write-Host "StageDir=$StageDir"
  exit 0
}

if (-not $InnoCompiler) {
  throw "Unable to find ISCC.exe. Install Inno Setup 6 or pass -InnoCompiler."
}

Write-Host "Compiling installer with $InnoCompiler"
& $InnoCompiler `
  "/DStageDir=$StageDir" `
  "/DOutputDir=$InstallerOutDir" `
  "/DAppVersion=$Version" `
  "/DPackageVersion=$PackageVersion" `
  $InnoScript
if ($LASTEXITCODE -ne 0) {
  throw "Inno Setup failed with exit code $LASTEXITCODE"
}

$installer = Join-Path $InstallerOutDir "GM3STM-WSJT-Fork-$PackageVersion-win64-setup.exe"
if (-not (Test-Path -LiteralPath $installer)) {
  throw "Installer was not created at expected path: $installer"
}

Write-Host "Installer ready: $installer"
