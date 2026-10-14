@echo off
chcp 65001 > nul
title GT2001 安装包自动化构建脚本

echo ========================================================
echo   GT2001 双串口相控阵天线监视软件 - 一键安装包构建工具
echo   流程:
echo     1. 检查 Python 及 PyInstaller / Inno Setup 环境
echo     2. 准备 Microsoft Edge WebView2 安装引导程序
echo     3. 打包 Python 核心程序与 static 静态前端页面
echo     4. 编译生成 Windows Setup 独立安装包 (.exe)
echo ========================================================
echo.

cd /d "%~dp0"

:: 1. 检查 Python
where python >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 系统未检测到 Python，请先安装 Python 3.8+ 并添加至 PATH!
    pause
    exit /b 1
)

:: 2. 检查并安装打包所需依赖
echo [1/4] 检查构建依赖 (PyInstaller, pyserial, pywebview)...
python -m pip install PyInstaller pyserial pywebview -i https://pypi.tuna.tsinghua.edu.cn/simple --quiet

:: 3. 下载微软官方 WebView2 Evergreen Bootstrapper（安装包中作为前置依赖）
echo.
echo [2/4] 准备 Microsoft Edge WebView2 Runtime 安装引导程序...
if not exist "redist" mkdir "redist"
if not exist "redist\MicrosoftEdgeWebview2Setup.exe" (
    powershell -NoProfile -ExecutionPolicy Bypass -Command "Invoke-WebRequest -UseBasicParsing 'https://go.microsoft.com/fwlink/p/?LinkId=2124703' -OutFile 'redist\MicrosoftEdgeWebview2Setup.exe'"
    if errorlevel 1 (
        echo [错误] WebView2 安装引导程序下载失败！
        echo 请联网重试，或从微软官网下载后保存为:
        echo redist\MicrosoftEdgeWebview2Setup.exe
        pause
        exit /b 1
    )
)
echo [OK] WebView2 安装引导程序已就绪。

:: 4. 运行 PyInstaller 重新编译后端及前端静态资源
echo.
echo [3/4] 正在使用 PyInstaller 编译核心程序与 static 静态资源...
python -m PyInstaller --noconfirm GT2001_Monitor.spec
if %errorlevel% neq 0 (
    echo [错误] PyInstaller 编译失败，请检查上方日志输出！
    pause
    exit /b 1
)
echo [OK] PyInstaller 编译成功，输出目录: dist\GT2001_Monitor\

:: 5. 自动定位 Inno Setup 编译器 ISCC.exe
echo.
echo [4/4] 正在查找 Inno Setup 编译器 (ISCC.exe)...
set "ISCC_PATH="

where iscc >nul 2>nul
if %errorlevel% equ 0 (
    for /f "delims=" %%i in ('where iscc') do set "ISCC_PATH=%%i"
)

if "%ISCC_PATH%"=="" (
    if exist "%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe" (
        set "ISCC_PATH=%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe"
    ) else if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" (
        set "ISCC_PATH=%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
    ) else if exist "%ProgramFiles%\Inno Setup 6\ISCC.exe" (
        set "ISCC_PATH=%ProgramFiles%\Inno Setup 6\ISCC.exe"
    )
)

if "%ISCC_PATH%"=="" (
    echo [警告] 未检测到 Inno Setup 6 命令行编译器 (ISCC.exe)！
    echo 绿色免安装目录已生成在: dist\GT2001_Monitor\
    echo 如需生成 Setup 安装包，请安装 Inno Setup 6 后重新运行本脚本，
    echo 或在 Inno Setup IDE 中手动打开 installer.iss 并点击 Compile。
    pause
    exit /b 0
)

echo 找到 Inno Setup: "%ISCC_PATH%"
echo 正在编译生成 Windows 安装包...
"%ISCC_PATH%" installer.iss
if %errorlevel% neq 0 (
    echo [错误] Inno Setup 编译失败！
    pause
    exit /b 1
)

echo.
echo ========================================================
echo   [构建完成] 安装包已生成！
echo   输出路径: dist_installer\GT2001_Monitor_v1.0_Setup.exe
echo ========================================================
echo.
choice /t 5 /d N /m "是否立即打开输出目录(5秒后默认否)"
if %errorlevel% equ 1 (
    explorer dist_installer
)

pause
