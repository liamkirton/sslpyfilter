@echo off
set install_source=release
regsvr32 /s %install_source%\SslPyFilterMarshal.dll
copy /y %install_source%\SslHook.dll C:\Windows\System32
LspInstall.exe /Ifs /Install 1001 SslLsp C:\SslPyFilter\%install_source%\SslLsp.dll
LspInstall.exe /Print