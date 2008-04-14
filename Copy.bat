@echo off
set install_source=release
mkdir %install_source%
copy /y Z:\SslPyFilter\LspInstall.exe
copy /y Z:\SslPyFilter\%install_source%\*.dll .\%install_source%
copy /y Z:\SslPyFilter\%install_source%\*.exe .\%install_source%
mkdir Filters
copy /y Z:\SslPyFilter\Filters\*.py .\filters
copy /y Z:\SslPyFilter\Install.bat .