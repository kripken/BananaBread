Name "Sauerbraten"

OutFile "sauerbraten_20YY_MM_DD_foo_edition_win32_setup.exe"

InstallDir $PROGRAMFILES\Sauerbraten

InstallDirRegKey HKLM "Software\Sauerbraten" "Install_Dir"

SetCompressor /SOLID lzma
XPStyle on

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Section "Sauerbraten (required)"

  SectionIn RO
  
  SetOutPath $INSTDIR
  
  File /r "..\..\*.*"
  
  WriteRegStr HKLM SOFTWARE\Sauerbraten "Install_Dir" "$INSTDIR"
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sauerbraten" "DisplayName" "Sauerbraten"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sauerbraten" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sauerbraten" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sauerbraten" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

  IfFileExists "$DOCUMENTS\My Games\Sauerbraten\config.cfg" ConfigFound NoConfig  
  ConfigFound:
     Delete "$DOCUMENTS\My Games\Sauerbraten\old-config.cfg"
     Rename "$DOCUMENTS\My Games\Sauerbraten\config.cfg" "$DOCUMENTS\My Games\Sauerbraten\old-config.cfg"
  NoConfig:

SectionEnd

Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Sauerbraten"
  
  SetOutPath "$INSTDIR"
  
  CreateShortCut "$INSTDIR\Sauerbraten.lnk"                "$INSTDIR\sauerbraten.bat" "" "$INSTDIR\bin\sauerbraten.exe" 0 SW_SHOWMINIMIZED
  CreateShortCut "$SMPROGRAMS\Sauerbraten\Sauerbraten.lnk" "$INSTDIR\sauerbraten.bat" "" "$INSTDIR\bin\sauerbraten.exe" 0 SW_SHOWMINIMIZED
  CreateShortCut "$SMPROGRAMS\Sauerbraten\Uninstall.lnk"   "$INSTDIR\uninstall.exe"   "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Sauerbraten\README.lnk"      "$INSTDIR\README.html"     "" "$INSTDIR\README.html" 0
  
SectionEnd

Section "Uninstall"
  
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Sauerbraten"
  DeleteRegKey HKLM SOFTWARE\Sauerbraten

  RMDir /r "$SMPROGRAMS\Sauerbraten"
  RMDir /r "$INSTDIR"

SectionEnd
