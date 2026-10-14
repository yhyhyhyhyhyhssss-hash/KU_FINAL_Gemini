; Script generated for GT2001 Phased Array Monitor & Control Console
; Inno Setup 6 Script

#define MyAppName "GT2001 相控阵天线综合监视与控制台"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "Phased Array Lab"
#define MyAppExeName "GT2001_Monitor.exe"
#define MyAppIconName "app_icon.ico"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
AppId={{D9835A9F-634F-4DE1-9F12-GT2001ARRAY01}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\GT2001_Monitor
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
OutputDir=dist_installer
OutputBaseFilename=GT2001_Monitor_v1.0_Setup
SetupIconFile=app_icon.ico
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
DisableProgramGroupPage=auto
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; Use default language (English) with custom UTF-8 Chinese titles


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Files]
Source: "dist\GT2001_Monitor\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "app_icon.ico"; DestDir: "{app}"; Flags: ignoreversion
Source: "redist\MicrosoftEdgeWebview2Setup.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; Check: NeedsWebView2

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppIconName}"
Name: "{group}\{#MyAppName}（浏览器兼容模式）"; Filename: "{app}\{#MyAppExeName}"; Parameters: "--browser"; IconFilename: "{app}\{#MyAppIconName}"
Name: "{group}\卸载 {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; IconFilename: "{app}\{#MyAppIconName}"; Tasks: desktopicon

[Run]
Filename: "{tmp}\MicrosoftEdgeWebview2Setup.exe"; Parameters: "/silent /install"; StatusMsg: "正在安装 Microsoft Edge WebView2 Runtime..."; Flags: waituntilterminated runhidden; Check: NeedsWebView2
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Code]
function IsWebView2Installed(): Boolean;
var
  Version: String;
  ClientKey: String;
begin
  ClientKey := 'SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}';
  Result := RegQueryStringValue(HKCU, ClientKey, 'pv', Version) and
            (Version <> '') and (Version <> '0.0.0.0');

  if not Result then
    Result := RegQueryStringValue(HKLM32, ClientKey, 'pv', Version) and
              (Version <> '') and (Version <> '0.0.0.0');

  if IsWin64 and (not Result) then
    Result := RegQueryStringValue(HKLM64, ClientKey, 'pv', Version) and
              (Version <> '') and (Version <> '0.0.0.0');
end;

function NeedsWebView2(): Boolean;
begin
  Result := not IsWebView2Installed();
end;
