; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "Springbeats Virtual MIDI Cable"
#define MyAppVersion "1.0.1"
#define MyAppPublisher "Springbeats"
#define MyAppURL "http://www.springbeats.com/"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{A8CB35CC-1005-456C-8930-54BE87A593B6}}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=postinstall
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
LicenseFile=preinstall\eula.rtf

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "japanese"; MessagesFile: "compiler:Languages\Japanese.isl"
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Files]
Source: "preinstall\devcon32.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "preinstall\devcon64.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "preinstall\dinstall.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "preinstall\dremove.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "preinstall\x32\sbvmidi.cat"; DestDir: "{app}\x32"; Flags: ignoreversion
Source: "preinstall\x32\sbvmidi.inf"; DestDir: "{app}\x32"; Flags: ignoreversion
Source: "preinstall\x32\sbvmidi.sys"; DestDir: "{app}\x32"; Flags: ignoreversion
Source: "preinstall\x64\sbvmidi.cat"; DestDir: "{app}\x64"; Flags: ignoreversion
Source: "preinstall\x64\sbvmidi.inf"; DestDir: "{app}\x64"; Flags: ignoreversion
Source: "preinstall\x64\sbvmidi64.sys"; DestDir: "{app}\x64"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\dinstall.bat"

[UninstallRun]
Filename: "{app}\dremove.bat"