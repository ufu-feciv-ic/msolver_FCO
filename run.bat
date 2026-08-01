@echo off
setlocal
cd /d "%~dp0"

set "MAIN_EXE=build\NLtool.exe"
set "WORKER_EXE=build\analysis-worker.exe"

if not exist "%MAIN_EXE%" (
    echo [ERRO] Executavel principal nao encontrado: %MAIN_EXE%
    pause
    exit /b 1
)

if not exist "%WORKER_EXE%" (
    echo [AVISO] O worker de analise nao foi encontrado.
)

echo [INFO] Iniciando NLtool...
start "" "%MAIN_EXE%"
endlocal
exit /b 0
