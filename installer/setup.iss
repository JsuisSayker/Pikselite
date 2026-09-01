; PikseliteEngine Installer - Inno Setup Script
; To compile: iscc setup.iss /dSourcePath="..\build\Release"
; Requires Inno Setup 6+ (https://jrsoftware.org/isdl.php)

#define MyAppName "PikseliteEngine"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Pikselite"
#define MyAppURL "https://pikselite.dev"
#define MyAppExeName "PikseliteEngine.exe"
#define MyAppAssocName "Pikselite Scene"
#define MyAppAssocExt ".scene"
#define MyAppAssocKey "PikseliteScene"

#ifndef SourcePath
#define SourcePath "..\build\Release"
#endif

#ifndef ProjectRoot
#define ProjectRoot ".."
#endif

[Setup]
AppId={{B8A3C9D1-4E2F-4A1B-9C5D-7E8F0A1B2C3D}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
InfoBeforeFile={#SourcePath}\..\..\installer\welcome.txt
OutputDir=.\output
OutputBaseFilename=PikseliteEngine-Setup-{#MyAppVersion}
; SetupIconFile={#ProjectRoot}\assets\icon.bmp
Compression=lzma
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
UninstallDisplayIcon={app}\{#MyAppExeName}
ChangesAssociations=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop shortcut"; GroupDescription: "Additional icons:"; Flags: checkedonce

[Files]
; Main executable
Source: "{#SourcePath}\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion

; Runtime DLLs (alongside the executable)
Source: "{#SourcePath}\box2d.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\glew32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\lua.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\nfd.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\SDL2.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\TracyClient.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\zlib1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Assets
Source: "{#ProjectRoot}\assets\icon.bmp"; DestDir: "{app}\assets"; Flags: ignoreversion
Source: "{#ProjectRoot}\assets\.gitkeep"; DestDir: "{app}\assets"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#ProjectRoot}\assets\fonts\*"; DestDir: "{app}\assets\fonts"; Flags: ignoreversion recursesubdirs
Source: "{#ProjectRoot}\assets\scenes\*"; DestDir: "{app}\assets\scenes"; Flags: ignoreversion recursesubdirs
Source: "{#ProjectRoot}\assets\ui\icons\*"; DestDir: "{app}\assets\ui\icons"; Flags: ignoreversion recursesubdirs

; Lua scripts
Source: "{#ProjectRoot}\scripts\*.lua"; DestDir: "{app}\scripts"; Flags: ignoreversion
Source: "{#ProjectRoot}\scripts\demo\*.lua"; DestDir: "{app}\scripts\demo"; Flags: ignoreversion

; Game build system
Source: "{#ProjectRoot}\games\CMakeLists.txt"; DestDir: "{app}\games"; Flags: ignoreversion
Source: "{#ProjectRoot}\games\__template__\src\main.cpp"; DestDir: "{app}\games\__template__\src"; Flags: ignoreversion

[Registry]
; Register .scene file association
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocExt}\OpenWithProgids"; ValueType: string; ValueName: "{#MyAppAssocKey}"; ValueData: ""; Flags: uninsdeletevalue
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}"; ValueType: string; ValueName: ""; ValueData: "{#MyAppAssocName}"; Flags: uninsdeletekey
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\{#MyAppExeName},0"
Root: HKA; Subkey: "Software\Classes\{#MyAppAssocKey}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\{#MyAppExeName}"" ""%1"""
Root: HKA; Subkey: "Software\Classes\Applications\{#MyAppExeName}\SupportedTypes"; ValueType: string; ValueName: ".scene"; ValueData: ""

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"
Name: "{autoprograms}\{#MyAppName}\Uninstall {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Launch {#MyAppName}"; Flags: postinstall nowait skipifsilent shellexec; WorkingDir: "{app}"

[UninstallDelete]
Type: dirifempty; Name: "{app}\assets\fonts"
Type: dirifempty; Name: "{app}\assets\scenes"
Type: dirifempty; Name: "{app}\assets\ui\icons"
Type: dirifempty; Name: "{app}\assets\ui"
Type: dirifempty; Name: "{app}\assets"
Type: dirifempty; Name: "{app}\scripts\demo"
Type: dirifempty; Name: "{app}\scripts"
Type: dirifempty; Name: "{app}\games\__template__\src"
Type: dirifempty; Name: "{app}\games\__template__"
Type: dirifempty; Name: "{app}\games"

[Code]
function InitializeSetup: Boolean;
begin
  Result := True;
end;