param([string]$Version = '1.3.1')
$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
if (!(Test-Path -LiteralPath '.\Brightness.exe')) { throw 'Run build.ps1 first.' }
$stage = Join-Path $PSScriptRoot ('dist\brightness-v' + $Version + '-windows-x64')
New-Item -ItemType Directory -Path $stage -Force | Out-Null
Copy-Item -LiteralPath .\Brightness.exe,.\brightness.ini,.\README.md,.\LICENSE -Destination $stage
$zip = Join-Path $PSScriptRoot ('dist\brightness-v' + $Version + '-windows-x64.zip')
if (Test-Path -LiteralPath $zip) { throw 'Archive exists; choose a fresh output version.' }
Add-Type -AssemblyName System.IO.Compression.FileSystem
[IO.Compression.ZipFile]::CreateFromDirectory($stage,$zip,[IO.Compression.CompressionLevel]::Optimal,$false)
$exe = Join-Path $PSScriptRoot ('dist\Brightness-v' + $Version + '-x64.exe')
Copy-Item -LiteralPath .\Brightness.exe -Destination $exe
@($zip,$exe) | ForEach-Object { (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant() + '  ' + [IO.Path]::GetFileName($_) } | Set-Content -LiteralPath .\dist\SHA256SUMS.txt -Encoding ascii
Get-Item -LiteralPath $zip,$exe | Select-Object Name,Length


