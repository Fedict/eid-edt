@echo off

For /F "tokens=1-2 delims=[]" %%T in ('Find /N /V "" %~dp0..\.svn\entries') Do (
If "%%T"=="4" set SVNRevision=%%U)
@echo on
copy %~dp0..\x64\release\EDTx64.exe %~dp0..\builds\EDTx64_%SVNRevision%.exe

@echo off