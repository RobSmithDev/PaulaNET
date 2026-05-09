@echo off
set PICOTOOL=%USERPROFILE%\.pico-sdk\picotool\2.2.0-a4\picotool\picotool.exe
set ELF=%~1
set ELF=%ELF:/=\%

# Im using virtual com ports to share the port between a terminal and here
# Sending a "B" makes the device reboot into BOOTSEL mode
mode COM11: BAUD=115200 PARITY=N DATA=8 STOP=1
echo B > \\.\COM11

"%PICOTOOL%" reboot -f -u 2>NUL

:retry
ping -n 3 127.0.0.1 > NUL
"%PICOTOOL%" load "%ELF%" -x
if errorlevel 1 goto retry


