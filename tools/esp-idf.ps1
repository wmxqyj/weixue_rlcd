param(
    [Parameter(Position = 0)]
    [ValidateSet('build', 'menuconfig', 'flash', 'monitor', 'flash-monitor', 'size')]
    [string]$Command = 'build',

    [string]$Port = 'COM8',

    [string]$ProjectPath = ''
)

$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$idfPath = Join-Path $repoRoot '.espressif\v5.5.2\esp-idf'
$idfToolsPath = Join-Path $repoRoot '.idf-tools'
$idfPython = Join-Path $idfToolsPath 'python_env\idf5.5_py3.14_env\Scripts\python.exe'
$idfScript = Join-Path $idfPath 'tools\idf.py'
$idfExport = Join-Path $idfPath 'export.ps1'

if ([string]::IsNullOrWhiteSpace($ProjectPath)) {
    $ProjectPath = Join-Path $repoRoot 'ESP32-S3-RLCD-4.2-Demo\02_ESP-IDF\10_FactoryProgram'
}

foreach ($requiredPath in @($idfPath, $idfToolsPath, $idfPython, $idfScript, $idfExport, $ProjectPath)) {
    if (-not (Test-Path -LiteralPath $requiredPath)) {
        throw "ESP-IDF required path not found: $requiredPath"
    }
}

$env:IDF_PATH = $idfPath
$env:IDF_TOOLS_PATH = $idfToolsPath
$env:PYTHONUTF8 = '1'
$env:PIP_CACHE_DIR = Join-Path $repoRoot '.cache\pip'
$env:IDF_COMPONENT_CACHE_PATH = Join-Path $repoRoot '.cache\idf-components'
$env:XDG_CACHE_HOME = Join-Path $repoRoot '.cache'

# Add CMake, Ninja, compilers and the ESP-IDF Python environment to this task only.
. $idfExport

Push-Location -LiteralPath $ProjectPath
try {
    [string[]]$idfArgs = @(switch ($Command) {
        'build'         { @('build') }
        'menuconfig'    { @('menuconfig') }
        'flash'         { @('-p', $Port, 'flash') }
        'monitor'       { @('-p', $Port, 'monitor') }
        'flash-monitor' { @('-p', $Port, 'flash', 'monitor') }
        'size'          { @('size') }
    })

    Write-Host "ESP-IDF v5.5.2 | project: $ProjectPath | command: $Command" -ForegroundColor Cyan
    if ($Command -in @('flash', 'monitor', 'flash-monitor')) {
        Write-Host "Serial port: $Port" -ForegroundColor Cyan
    }

    & $idfPython $idfScript @idfArgs
    if ($LASTEXITCODE -ne 0) {
        throw "idf.py failed with exit code $LASTEXITCODE"
    }
}
finally {
    Pop-Location
}
