#define FileVer() \
  GetVersionComponents('..\..\Binari\WIN\ODLA\odla.exe',   \
  Local[0], Local[1], Local[2], Local[3]), \
  Str(Local[0]) + "." + Str(Local[1]) + "." + Str(Local[2])

#define OdlaPath "..\..\Binari\WIN\ODLA"

#ifndef OdlaAppName
  #define OdlaAppName "odla"
#endif

#ifndef OdlaAppExeName
  #define OdlaAppExeName "odla.exe"
#endif

#define AppId "96B5CB2A-5AA8-4EAE-8D37-E42000305B52"


[Setup]
;AlwaysRestart=yes  
AppId={#AppId}
AppMutex=OdlaMutex
AppName={#OdlaAppName}
AppVersion={#FileVer}
AppVerName={#OdlaAppName} {#FileVer}
AppPublisher=Kemonia River s.r.l.
AppPublisherURL=www.odlamusic.com
AppSupportURL=https://odlamusic.com/supporto
AppUpdatesURL=https://odlamusic.com/scarica-software/
ArchitecturesInstallIn64BitMode=x64
Compression=lzma2
CloseApplications=force
DefaultDirName={commonpf64}\ODLA
DirExistsWarning=yes
DisableProgramGroupPage=yes
DisableDirPage=no  
DisableWelcomePage=no       
LicenseFile="Licenses\LicenseODLA.rtf"                          
LZMAUseSeparateProcess=yes
LZMANumBlockThreads=32
MinVersion=6.1
OutputBaseFilename=ODLA_Installer-{#FileVer}
OutputDir=.        
PrivilegesRequired=admin                       
SetupIconFile=Imgs\installer.ico
SetupMutex=OdlaInstallerMutex
SolidCompression=no      
Uninstallable=true
UninstallDisplayIcon={app}\uninstaller.ico
UninstallFilesDir={app}
VersionInfoTextVersion={#FileVer}
VersionInfoProductName={#OdlaAppName}
VersionInfoProductVersion={#FileVer}
WizardImageFile=Imgs\VerticalBanner.bmp
WizardImageStretch=yes    
WizardResizable=no        
WizardStyle=modern
ChangesAssociations = yes


[Icons]
Name: "{autostartmenu}\{#OdlaAppName}";         Filename: "{app}\{#OdlaAppName}.exe";          Tasks:odlaStartIcon;       WorkingDir: "{app}";  
Name: "{autodesktop}\{#OdlaAppName}";           Filename: "{app}\{#OdlaAppName}.exe";          Tasks:odlaDesktopIcon;     WorkingDir: "{app}";  
Name: "{userstartup}\{#OdlaAppName}";           Filename: "{app}\{#OdlaAppName}.exe";                                     WorkingDir: "{app}";


[Tasks]  
Name: "odlaStartIcon";        Description: "{cm:CreateQuickLaunchIcon,ODLA}";       GroupDescription: "{cm:AdditionalIcons}"; 
Name: "odlaDesktopIcon";      Description: "{cm:CreateDesktopIcon,ODLA}";           GroupDescription: "{cm:AdditionalIcons}"; 


[Types]
Name: "custom"; Description: "Custom installation"; Flags: iscustom


[Languages]
Name: en; MessagesFile: Languages\English.isl;
Name: it; MessagesFile: Languages\Italian.isl;
Name: fr; MessagesFile: Languages\French.isl;


[Files]
Source: "{#OdlaPath}\*";      DestDir: "{app}"; Flags: ignoreversion createallsubdirs recursesubdirs;
Source: "Imgs\TopBanner.bmp"; DestDir: "{tmp}"; Flags: ignoreversion
Source: "imgs\uninstaller.ico"; DestDir: "{app}"


[Run]
Filename: "{app}\{#OdlaAppName}.exe"; Description: {cm:OdlaStartup}; Flags: postinstall runasoriginaluser nowait


[Code]

var
  BitmapImage: TBitmapImage;
  PageDescriptionLabel: TLabel;
  PageNameLabel: TLabel; 

function CloneStaticTextToLabel(StaticText: TNewStaticText): TLabel;
begin
  Result := TLabel.Create(WizardForm);
  Result.Parent := StaticText.Parent;
  Result.Left := StaticText.Left;
  Result.Top := StaticText.Top;
  Result.Width := StaticText.Width;
  Result.Height := StaticText.Height;
  Result.AutoSize := StaticText.AutoSize;
  Result.ShowAccelChar := StaticText.ShowAccelChar;
  Result.WordWrap := StaticText.WordWrap;
  Result.Font := StaticText.Font;
  StaticText.Visible := False;
end;

// GetLanguage is the function to get selected language string (currently only en, it and fr)
function GetLanguage(Param: String): String;
begin
  Result := ActiveLanguage;
end;   
     
function GetUninstallString: string;
var
  sUnInstPath: string;
  sUnInstallString: String;
begin
  Result := '';
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#AppId}_is1'); // Your App GUID/ID
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
  begin
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  end;
  Result := sUnInstallString;
end;

function IsUpgrade: Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

function ShouldInstallMuseScoreODLA: Boolean;
begin
  // Chiedi all'utente se desidera installare MuseScore ODLA
  Result := MsgBox('Vuoi installare MuseScore ODLA (3.6.2)?', mbConfirmation, MB_YESNO) = IDYES;
end;

function OldVersionInstalled: Boolean;
begin
  Result := RegValueExists(HKEY_LOCAL_MACHINE, 'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#AppId}_is1', 'UninstallString'); 
end;

function InitializeSetup: Boolean;
var 
  UserAgreesUnistallation: Integer;
  iResultCode: Integer;   
  sUnInstallString: string;
begin
  if OldVersionInstalled then 
  begin
    Log('OldVersionExist = True');
    UserAgreesUnistallation := MsgBox(ExpandConstant('{cm:OldVersionFound}'), mbInformation, MB_YESNO); // Custom Message if App installed

    if UserAgreesUnistallation = IDYES then 
    begin
      RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'SOFTWARE\Kemonia River s.r.l.');
      Log('User agrees with unistallation');
      sUnInstallString := RemoveQuotes(GetUninstallString());
      Exec(ExpandConstant(sUnInstallString), '', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
      // In case uninstallation went wrong
      if OldVersionInstalled then  
      begin 
        Log('OldVersionExist = True again');  
        Result := False;  
      end 
      else 
      begin 
        Log('OldVersionExist = Not exist anymore');  
        Result := True; // if you want to proceed after uninstall
      end;
    end
    else
    begin  
      Log('User don''t agrees with unistallation');
      Result := False; // if you want to quit after uninstall 
      Exit;
    end;
  end
  else
  begin
    Log('No old version installed');
    Result := True;  // Continue installation if no old version is found
  end;
end;


procedure InitializeWizard();
begin
  // Estrae il file TopBanner.bmp nella directory temporanea
  ExtractTemporaryFile('TopBanner.bmp');    

  // Crea e configura l'immagine bitmap
  BitmapImage := TBitmapImage.Create(WizardForm);
  BitmapImage.Parent := WizardForm.MainPanel;  
  BitmapImage.Width := WizardForm.MainPanel.Width;
  BitmapImage.Height := WizardForm.MainPanel.Height;
  BitmapImage.Anchors := [akLeft, akTop, akRight, akBottom];
  BitmapImage.Stretch := True;
  // Disabilita l'AutoSize per mantenere le dimensioni
  BitmapImage.AutoSize := False;

  // Carica l'immagine dal file estratto
  BitmapImage.Bitmap.LoadFromFile(ExpandConstant('{tmp}\TopBanner.bmp'));

  // Nasconde l'immagine bitmap predefinita
  WizardForm.WizardSmallBitmapImage.Visible := False;

  // Rende visibili le etichette della pagina
  WizardForm.PageDescriptionLabel.Visible := True;
  WizardForm.PageNameLabel.Visible := True;

  // Imposta i colori di sfondo delle etichette
  WizardForm.PageDescriptionLabel.Color := TColor($E8E8E8);
  WizardForm.PageNameLabel.Color := TColor($E8E8E8);

  // Clona le etichette in componenti TLabel personalizzati
  PageNameLabel := CloneStaticTextToLabel(WizardForm.PageNameLabel);
  PageDescriptionLabel := CloneStaticTextToLabel(WizardForm.PageDescriptionLabel);
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  Log('WizardForm.LicenseMemo.Left: ' + IntToStr(WizardForm.LicenseMemo.Left));
  BitmapImage.Left := WizardForm.LicenseMemo.Left;

  // Update the custom TLabel components from the standard hidden components
  PageDescriptionLabel.Caption := WizardForm.PageDescriptionLabel.Caption;
  PageNameLabel.Caption := WizardForm.PageNameLabel.Caption;
end;