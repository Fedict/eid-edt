@echo on

svn info | FindStr /L Revision > %~dp0\svn_info.txt

for /f "tokens=2" %%T in (%~dp0svn_info.txt) do set SVNRevision=%%T

cp %~dp0..\Release\EDTWin32.exe %~dp0\..\builds\EDTWin32_%SVNRevision%.exe


@echo off