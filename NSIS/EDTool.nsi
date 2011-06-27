
;--------------------------------
;Include 64bit checks
  !include "x64.nsh"

;--------------------------------
;General

  ;Name and file
  Name "EDTool"
  OutFile "EDTool.exe"

  ;Default installation folder  
  InstallDir "$PROGRAMFILES\Belgium Identity Card Diagnostics"

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin
  ;TargetMinimalOS 5.0

;--------------------------------
;Interface Settings
SilentInstall silent

;Installer Sections
Section "Belgium EDTools"
  SetOutPath "$INSTDIR"
  File "..\Win\Release\EDTWin32.exe"
  ${If} ${RunningX64}
	;MessageBox MB_OK "running on x64"
	File "..\Win\x64\Release\EDTx64.exe"
	ExecWait '"$INSTDIR\EDTx64.exe" a s';a stands for automatic end exe when completed, S stands for 'show startbutton'
	;MessageBox MB_OK "running Win32"
	ExecWait '"$INSTDIR\EDTWin32.exe"'
  ${Else}
	;MessageBox MB_OK "running Win32"
	ExecWait '"$INSTDIR\EDTWin32.exe" s'
  ${EndIf}
  ${If} ${RunningX64}
	Delete "$INSTDIR\EDTx64.exe"
  ${EndIf}
	Delete "$INSTDIR\EDTWin32.exe"
SectionEnd
