; Packages the staged editor folder (dist\MyRevoke\) into
; dist\MyRevoke-Setup-<version>.exe. Compile it with:
;   iscc installer\MyRevoke.iss
; See installer\README.md.

#define StageDir AddBackslash(SourcePath) + "..\dist\MyRevoke"

#if !DirExists(StageDir)
  #error dist\MyRevoke not found - stage the editor folder first
#endif

; The version lives only in the repo root VERSION file, so the installer and
; the build can never disagree about it.
#define VersionFile FileOpen(AddBackslash(SourcePath) + "..\VERSION")
#define AppVersion Trim(FileRead(VersionFile))
#expr FileClose(VersionFile)

#if Len(AppVersion) == 0
  #error VERSION is empty
#endif

[Setup]
AppId={{31C354C6-35E6-4BFA-AD4F-ED1F2A3E4B76}
AppName=MyRevoke
AppVersion={#AppVersion}
AppPublisher=Revokeeee
DefaultDirName={autopf}\MyRevoke
DefaultGroupName=MyRevoke
DisableProgramGroupPage=yes
UninstallDisplayName=MyRevoke {#AppVersion}
UninstallDisplayIcon={app}\RevokeCraft.exe
; Program Files and the VC++ redistributable are both per-machine.
PrivilegesRequired=admin
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
OutputDir={#AddBackslash(SourcePath)}..\dist
OutputBaseFilename=MyRevoke-Setup-{#AppVersion}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
VersionInfoVersion={#AppVersion}

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\MyRevoke"; Filename: "{app}\RevokeCraft.exe"
Name: "{autodesktop}\MyRevoke"; Filename: "{app}\RevokeCraft.exe"; Tasks: desktopicon

[Run]
Filename: "{tmp}\vc_redist.x64.exe"; Parameters: "/install /passive /norestart"; StatusMsg: "Installing Microsoft Visual C++ 2015-2022 Redistributable (x64)..."; Check: VCRedistNeeded
Filename: "{app}\RevokeCraft.exe"; Description: "{cm:LaunchProgram,MyRevoke}"; Flags: nowait postinstall skipifsilent

[UninstallDelete]
; The editor writes imgui.ini next to its exe, which would keep the install
; folder alive after an uninstall. User projects are never in here.
Type: files; Name: "{app}\imgui.ini"

[Code]
function RuntimeRegistered(RootKey: Integer): Boolean;
var
  Installed: Cardinal;
begin
  Result := RegQueryDWordValue(RootKey, 'SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64', 'Installed', Installed) and (Installed = 1);
end;

// The redistributable registers itself in the 32-bit registry view on some
// Windows versions and the 64-bit one on others, so both are checked.
function VCRedistNeeded: Boolean;
begin
  Result := not (RuntimeRegistered(HKLM32) or RuntimeRegistered(HKLM64));
end;

function OnDownloadProgress(const Url, FileName: String; const Progress, ProgressMax: Int64): Boolean;
begin
  Result := True;
end;

// Downloading here rather than from a wizard page keeps silent installs working.
function PrepareToInstall(var NeedsRestart: Boolean): String;
begin
  Result := '';
  if VCRedistNeeded then
  begin
    try
      DownloadTemporaryFile('https://aka.ms/vs/17/release/vc_redist.x64.exe', 'vc_redist.x64.exe', '', @OnDownloadProgress);
    except
      Result := 'Could not download the Microsoft Visual C++ 2015-2022 Redistributable (x64): ' + GetExceptionMessage;
    end;
  end;
end;
