$ErrorActionPreference = 'Stop'
Set-Location -LiteralPath $PSScriptRoot
& windres brightness.rc -O coff -o brightness-res.o
if ($LASTEXITCODE -ne 0) { throw 'Resource build failed.' }
& gcc brightness.c brightness-res.o -o Brightness.exe -Os -s -nostdlib -fno-stack-protector -fno-builtin -mwindows '-Wl,-e,entry' '-Wl,--no-insert-timestamp' -lkernel32 -luser32 -ldxva2 -ladvapi32 -lshell32
if ($LASTEXITCODE -ne 0) { throw 'Native build failed.' }
Get-Item -LiteralPath .\Brightness.exe | Select-Object Name,Length
