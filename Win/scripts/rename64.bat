@echo on

svn info | FindStr /L Revision > %~dp0/svn_info.txt

for /f "tokens=2" %%T in (%~dp0/svn_info.txt) do set SVNRevision=%%T

cp %~dp0../x64/release/EDTx64.exe %~dp0../builds/EDTWin64_%SVNRevision%.exe


@echo off