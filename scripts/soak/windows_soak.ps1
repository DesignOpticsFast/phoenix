#!/usr/bin/env pwsh
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$soakBin = if ($env:SOAK_BIN) { $env:SOAK_BIN } else { 'build\dev-01-relwithdebinfo\phoenix_app.exe' }
$soakAppLog = if ($env:SOAK_APP_LOG) { $env:SOAK_APP_LOG } else { Join-Path $env:TEMP 'phoenix_soak_app.log' }
$soakCsv = if ($env:SOAK_CSV) { $env:SOAK_CSV } else { Join-Path $env:TEMP 'phoenix_soak_memory.csv' }
$durationMin = if ($env:DURATION_MIN) { [int]$env:DURATION_MIN } else { 60 }
$defaultArgs = '--test-i18n --lang=en'
$soakArgs = if ($env:SOAK_ARGS) { $env:SOAK_ARGS } else { $defaultArgs }

if (-not (Test-Path $soakBin)) {
    Write-Error "SOAK_BIN '$soakBin' not found. Run 'make soak-build' or set SOAK_BIN."
}

Remove-Item -ErrorAction SilentlyContinue $soakAppLog, $soakCsv
New-Item -ItemType Directory -Force -Path (Split-Path $soakAppLog) | Out-Null
New-Item -ItemType Directory -Force -Path (Split-Path $soakCsv) | Out-Null
"timestamp,elapsed_sec,rss_kb,rss_mb,threads,fds,handles" | Set-Content -Path $soakCsv

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = (Resolve-Path $soakBin).Path
$psi.WorkingDirectory = Split-Path $psi.FileName
$psi.Arguments = $soakArgs
$psi.UseShellExecute = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardError = $true
$psi.StandardOutputEncoding = [System.Text.Encoding]::UTF8
$psi.StandardErrorEncoding = [System.Text.Encoding]::UTF8
$psi.EnvironmentVariables['QT_QPA_PLATFORM'] = if ($env:QT_QPA_PLATFORM) { $env:QT_QPA_PLATFORM } else { 'offscreen' }

$process = New-Object System.Diagnostics.Process
$process.StartInfo = $psi
$logWriter = [System.IO.StreamWriter]::new($soakAppLog, $false, [System.Text.Encoding]::UTF8)
$handler = [System.Diagnostics.DataReceivedEventHandler]{
    param($sender, $args)
    if ($args.Data) {
        $logWriter.WriteLine($args.Data)
        $logWriter.Flush()
    }
}
$process.add_OutputDataReceived($handler)
$process.add_ErrorDataReceived($handler)
$null = $process.Start()
$process.BeginOutputReadLine()
$process.BeginErrorReadLine()

$startTime = [DateTime]::UtcNow
$deadline = $startTime.AddMinutes($durationMin)

function Write-Sample {
    param($proc)
    if (-not $proc) { return }
    $now = [DateTime]::UtcNow
    $elapsed = [int][Math]::Floor(($now - $startTime).TotalSeconds)
    $timestamp = $now.ToString('yyyy-MM-ddTHH:mm:ssZ')
    $rssKB = [int][Math]::Round($proc.WorkingSet64 / 1KB)
    $rssMB = [Math]::Round($proc.WorkingSet64 / 1MB, 3)
    $threads = $proc.Threads.Count
    $handles = $proc.HandleCount
    [System.IO.File]::AppendAllText($soakCsv, "$timestamp,$elapsed,$rssKB,$rssMB,$threads,,${handles}`n")
}

Start-Sleep -Seconds 0
Write-Sample -proc (Get-Process -Id $process.Id -ErrorAction SilentlyContinue)

while (-not $process.HasExited) {
    if ([DateTime]::UtcNow -ge $deadline) {
        $logWriter.WriteLine("Reached configured duration ($durationMin min); terminating process.")
        $logWriter.Flush()
        $process.CloseMainWindow() | Out-Null
        Start-Sleep -Seconds 5
        if (-not $process.HasExited) {
            $process.Kill()
        }
        break
    }
    Start-Sleep -Seconds 5
    $proc = Get-Process -Id $process.Id -ErrorAction SilentlyContinue
    Write-Sample -proc $proc
}

$process.WaitForExit()
Write-Sample -proc (Get-Process -Id $process.Id -ErrorAction SilentlyContinue)

$process.remove_OutputDataReceived($handler)
$process.remove_ErrorDataReceived($handler)
$logWriter.Close()
