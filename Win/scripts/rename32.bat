@echo off

For /F "tokens=1-2 delims=[]" %%T in ('Find /N /V "" %~dp0..\.svn\entries') Do (
If "%%T"=="4" set SVNRevision=%%T)
@echo on
copy %~dp0..\release\EDTWin32.exe %~dp0..\builds\EDTWin32_%SVNRevision%.exe

@echo off