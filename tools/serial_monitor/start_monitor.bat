@echo off
chcp 65001 > nul
title GT2001 双串口回读与波控监视软件
echo ========================================================
echo   GT2001 相控阵天线双串口监视与回读控制台
echo   默认极化反推算法: +2.0*Pol (当前固件修正版本)
echo   (如需旧版算法可运行 start_monitor_minus_pol.bat 或在界面右上角点击一键切换)
echo   UART0: 发送控制命令 ($PTS, $xxATS, $xxREG)
echo   UART3: 接收回读调试信息 (BEAM_REG, BEAM_UPDATE)
echo ========================================================
echo.

cd /d "%~dp0"

where python >nul 2>nul
if %errorlevel% neq 0 (
    echo [错误] 系统未检测到 Python，请先安装 Python 3.8+ 并添加至 PATH!
    pause
    exit /b 1
)

echo [1/2] 正在检查并安装 pyserial 依赖...
python -m pip install pyserial -i https://pypi.tuna.tsinghua.edu.cn/simple --quiet >nul 2>nul

echo [2/2] 正在启动本地监视控制台服务 (端口: 8088)...
echo 浏览器即将自动打开 http://127.0.0.1:8088
echo 按 Ctrl+C 可停止运行。
echo.

python app.py --pol-mode=+2*pol

pause
