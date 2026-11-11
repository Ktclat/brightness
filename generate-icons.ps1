$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$assetDir = Join-Path $PSScriptRoot 'assets'
New-Item -ItemType Directory -Path $assetDir -Force | Out-Null
$sizes = @(16,20,24,32,40,48,64,128,256)
foreach ($variant in @('Brightness','TrayLight','TrayDark')) {
    $images = @()
    foreach ($size in $sizes) {
        $canvas = New-Object Drawing.Bitmap(($size*4),($size*4))
        $g = [Drawing.Graphics]::FromImage($canvas)
        $g.SmoothingMode = 'AntiAlias'; $g.ScaleTransform(($size*4/32.0),($size*4/32.0))
        $color = switch ($variant) { 'TrayLight' {[Drawing.Color]::White}; 'TrayDark' {[Drawing.Color]::FromArgb(32,32,32)}; default {[Drawing.Color]::FromArgb(136,144,156)} }
        $pen = New-Object Drawing.Pen($color,3)
        $pen.StartCap = 'Round'; $pen.EndCap = 'Round'
        for ($bar=0;$bar -lt 4;$bar++) { $x=5+$bar*7; $top=21-$bar*5.5; $g.DrawLine($pen,[single]$x,[single]27,[single]$x,[single]$top) }
        $pen.Dispose(); $g.Dispose()
        $bmp = New-Object Drawing.Bitmap($size,$size)
        $g = [Drawing.Graphics]::FromImage($bmp); $g.InterpolationMode = 'HighQualityBicubic'; $g.CompositingQuality='HighQuality'; $g.DrawImage($canvas,0,0,$size,$size); $g.Dispose(); $canvas.Dispose()
        if ($size -eq 256) { $bmp.Save((Join-Path $assetDir ($variant+'.png')),[Drawing.Imaging.ImageFormat]::Png) }
        $stream = New-Object IO.MemoryStream; $bmp.Save($stream,[Drawing.Imaging.ImageFormat]::Png)
        $images += ,@($size,$stream.ToArray()); $stream.Dispose(); $bmp.Dispose()
    }
    $file=[IO.File]::Create((Join-Path $assetDir ($variant+'.ico')));$writer=New-Object IO.BinaryWriter($file)
    try {
        $writer.Write([uint16]0);$writer.Write([uint16]1);$writer.Write([uint16]$images.Count)
        $offset=6+16*$images.Count
        foreach($entry in $images) {
            $dim=if($entry[0]-eq 256){0}else{$entry[0]}
            $writer.Write([byte]$dim);$writer.Write([byte]$dim);$writer.Write([uint16]0);$writer.Write([uint16]1);$writer.Write([uint16]32)
            $writer.Write([uint32]$entry[1].Length);$writer.Write([uint32]$offset);$offset+=$entry[1].Length
        }
        foreach($entry in $images){$writer.Write([byte[]]$entry[1])}
    } finally {$writer.Dispose();$file.Dispose()}
}
Write-Output 'Created unified application and light/dark tray icons at 9 sizes (16-256 px).'

