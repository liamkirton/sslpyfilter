@echo off
set install_source=release
mkdir %install_source%
copy /y Z:\LspInstall.exe
copy /y Z:\%install_source%\*.dll .\%install_source%
copy /y Z:\%install_source%\*.exe .\%install_source%
mkdir Filters
copy /y Z:\Filters\*.py .\filters
copy /y Z:\Install.bat .