; Script generated by the Inno Script Studio Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "HISE"
#define MyAppVersion "0.99 beta"
#define MyAppPublisher "Hart Instruments"
#define MyAppURL "http://hartinstruments.net"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{7BF904A4-4CAA-48C9-AC98-7F8223EED0AD}

AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={pf}\{#MyAppName}
DefaultGroupName={#MyAppName}
LicenseFile="..\..\license.txt"
InfoBeforeFile="..\..\changelog.txt"
ArchitecturesInstallIn64BitMode=x64

AllowNoIcons=yes
OutputBaseFilename=RenameInstaller
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Components]
Name: "x86Plugin"; Description: "32bit VST Plugin"; Types: compact custom full
Name: "x64Plugin"; Description: "64bit VST Plugin"; Types: compact custom full
Name: "Standalone64bit"; Description: "64bit Standalone application"; ExtraDiskSpaceRequired: 9000; Types: full compact custom
Name: "Standalone32bit"; Description: "32bit Standalone application"; ExtraDiskSpaceRequired: 9000

[Files]
Source: "..\..\projects\standalone\Builds\VisualStudio2015\x64\Release\HISE.exe"; DestDir: "{app}"; Flags: 64bit; Components: Standalone64bit
Source: "..\..\projects\standalone\Builds\VisualStudio2015\Release\HISE x86.exe"; DestDir: "{app}"; Flags: 32bit; Components: Standalone32bit
Source: "C:\Program Files\VST Plugins\HISE x64.dll"; DestDir: "{code:Getx64bitDir}"; Flags: 64bit; Components: x64Plugin
Source: "..\..\projects\plugin\Builds\VisualStudio2015\Release\HISE x86.dll"; DestDir: "{code:Getx86bitDir}"; Flags: 32bit; Components: x86Plugin

[Icons]
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"; 
Name: "{group}\HISE x86"; Filename: "{app}\HISE x86.exe"; WorkingDir: "{app}"; IconFilename: "{app}\HISE x86.exe"; IconIndex: 0; Components: Standalone32bit
Name: "{group}\HISE"; Filename: "{app}\HISE.exe"; WorkingDir: "{app}"; IconFilename: "{app}\HISE.exe"; IconIndex: 0; Components: Standalone64bit

[Dirs]

[Run]
Filename: "http://hise.audio/manual/Manual.php"; Flags: shellexec runasoriginaluser postinstall; Description: "Open the online documentation.";

[Code]
var x86Page: TInputDirWizardPage;
var x64Page: TInputDirWizardPage;

procedure InitializeWizard;
begin
  // Create the page



  x86Page := CreateInputDirPage(wpSelectComponents,
    'Select VST 32bit Plugin Folder', 'Where should the 32bit version of the VST Plugin be installed?',
    'Select your VST 32bit Folder',
    False, '');
  x86Page.Add('');

  x86Page.Values[0] := GetPreviousData('x86', '');

  x64Page := CreateInputDirPage(wpSelectComponents,
    'Select VST 64bit Plugin Folder', 'Where should the 64bit version of the VST Plugin be installed?',
    'Select your VST 64bit Folder',
    False, '');
  x64Page.Add('');

  x64Page.Values[0] := GetPreviousData('x64', '');

end;

function ShouldSkipPage(PageID: Integer): Boolean;
begin

  if PageFromID(PageID) = x64Page then Result := not isComponentSelected('x64Plugin');
  if PageFromID(PageID) = x86Page then Result := not isComponentSelected('x86Plugin');

end;


function NextButtonClick(CurPageID: Integer): Boolean;
begin



  // Set default folder if empty
  if x86Page.Values[0] = '' then
     x86Page.Values[0] := ExpandConstant('{pf32}\VSTPlugins');
  Result := True;

  if (IsWin64) and (x64Page.Values[0] = '') then
     x64Page.Values[0] := ExpandConstant('{pf64}\VSTPlugins');
  Result := True;
end;

function Getx86bitDir(Param: String): String;
begin
  { Return the selected DataDir }
  Result := x86Page.Values[0];
end;

function Getx64bitDir(Param: String): String;
begin
  { Return the selected DataDir }
  Result := x64Page.Values[0];
end;