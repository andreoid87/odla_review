#define MusescorePath "..\..\Binari\WIN\MuseScore ODLA"
#define FileVerStr "3.6.2"
  
#ifndef MusescoreAppName
  #define MusescoreAppName "MuseScore ODLA"
#endif

#ifndef OdlaAppExeName
  #define OdlaAppExeName "MuseScore ODLA.exe"
#endif

; just changed final 2 with 3
#define AppId "96B5CB2A-5AA8-4EAE-8D37-E42000305B53"

[Setup]
;AlwaysRestart=yes  
AppId={#AppId}
AppMutex=MusescoreMutex
AppName={#MusescoreAppName}
AppVersion={#FileVerStr}
AppVerName={#MusescoreAppName} {#FileVerStr}
AppPublisher=Kemonia River s.r.l.
AppPublisherURL=www.odlamusic.com
AppSupportURL=https://odlamusic.com/supporto
AppUpdatesURL=https://odlamusic.com/scarica-software/
Compression=lzma2
CloseApplications=force
DefaultDirName={commonpf64}\MuseScore ODLA
DirExistsWarning=yes
DisableProgramGroupPage=yes
DisableDirPage=no  
DisableWelcomePage=no       
LicenseFile="Licenses\LicenseMusescore.rtf"                          
LZMAUseSeparateProcess=yes
LZMANumBlockThreads=32
MinVersion=0,6.1
OutputBaseFilename=MuseScoreODLA_Installer-{#FileVerStr}
OutputDir=.        
PrivilegesRequired=admin                       
SetupIconFile=imgs\installer.ico
SetupMutex=MusescoreInstallerMutex
SolidCompression=no      
Uninstallable=true
UninstallDisplayIcon={app}\uninstaller.ico
UninstallFilesDir={app}
VersionInfoTextVersion={#FileVerStr}
VersionInfoProductName={#MusescoreAppName}
VersionInfoProductVersion={#FileVerStr}
WizardImageFile=imgs\VerticalBanner.bmp
WizardImageStretch=yes    
WizardResizable=no        
WizardStyle=modern
ChangesAssociations = yes

[Icons]
Name: "{autostartmenu}\{#MusescoreAppName}";         Filename: "{app}\bin\{#MusescoreAppName}.exe";          Tasks:MusescoreStartIcon;       WorkingDir: "{app}";  
Name: "{autodesktop}\{#MusescoreAppName}";           Filename: "{app}\bin\{#MusescoreAppName}.exe";          Tasks:MusescoreDesktopIcon;     WorkingDir: "{app}";  

[Tasks]  
Name: "MusescoreStartIcon";        Description: "{cm:CreateQuickLaunchIcon,ODLA}";       GroupDescription: "{cm:AdditionalIcons}"; 
Name: "MusescoreDesktopIcon";      Description: "{cm:CreateDesktopIcon,ODLA}";           GroupDescription: "{cm:AdditionalIcons}"; 

[Types]
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Languages]
Name: en; MessagesFile: Languages\English.isl;
Name: it; MessagesFile: Languages\Italian.isl;
Name: fr; MessagesFile: Languages\French.isl;

[Files]
; Application files
Source: "{#MusescorePath}\*";      DestDir: "{app}";                  Flags: ignoreversion createallsubdirs recursesubdirs;
Source: "imgs\TopBanner_Musescore.bmp"; Flags: dontcopy noencryption
Source: "imgs\uninstaller.ico"; DestDir: "{app}"

[Run]                                                                                                                
Filename: "{app}\bin\{#MusescoreAppName}.exe"; Description: {cm:MusescoreStartup}; Flags: postinstall runasoriginaluser nowait

[Dirs]
Name: "{app}\workspaces"; Permissions: everyone-full

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
  sUnInstPath := ExpandConstant('Software\Microsoft\Windows\CurrentVersion\Uninstall\{#AppId}_is1'); //Your App GUID/ID
  sUnInstallString := '';
  if not RegQueryStringValue(HKLM, sUnInstPath, 'UninstallString', sUnInstallString) then
    RegQueryStringValue(HKCU, sUnInstPath, 'UninstallString', sUnInstallString);
  Log('Sto restituendo la seugente stringa di disinstallazione: ' + sUnInstallString);
  Result := sUnInstallString;
end;

function IsUpgrade: Boolean;
begin
  Result := (GetUninstallString() <> '');
end;

function InitializeSetup: Boolean;
var 
  V: Integer;
  OldVersionExist: Boolean;
  iResultCode: Integer;   
  sUnInstallString: string;
  Prova: string;
begin
  RegDeleteKeyIncludingSubkeys(HKEY_CURRENT_USER, 'SOFTWARE\Kemonia River s.r.l.');
  Result := True; // in case when no previous version is found
  OldVersionExist := RegValueExists(HKEY_LOCAL_MACHINE,'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#AppId}_is1', 'UninstallString');
  Prova := GetUninstallString();

  if OldVersionExist then begin
    Log('OldVersionExist = True');
    V := MsgBox(ExpandConstant('{cm:OldVersionFoundMusescore}'), mbInformation, MB_YESNO); // Custom Message if App installed
    if V = IDYES then begin
      Log('V = IDYES');
      sUnInstallString := GetUninstallString();
      sUnInstallString :=  RemoveQuotes(sUnInstallString);
      Exec(ExpandConstant(sUnInstallString), '', '', SW_SHOW, ewWaitUntilTerminated, iResultCode);
      // In case uninstalltaion went wrong
      OldVersionExist := RegValueExists(HKEY_LOCAL_MACHINE,'Software\Microsoft\Windows\CurrentVersion\Uninstall\{#AppId}_is1', 'UninstallString');
      if OldVersionExist then  begin 
        Log('OldVersionExist = True again');  
        Result := False;  
      end else begin 
        Log('OldVersionExist = Not exist anymore');  
        Result := True; // if you want to proceed after uninstall
      end;
    end
    else
    begin  
      Log('V = IDNO');
      Result := False; // if you want to proceed after uninstall
      Exit; // if you want to quit after uninstall 
    end;
  end;
 end;
 
procedure InitializeWizard();
begin
  ExtractTemporaryFile('TopBanner_Musescore.bmp');
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
  BitmapImage.Bitmap.LoadFromFile(ExpandConstant('{tmp}\TopBanner_Musescore.bmp'));

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
  //Update the custom TLabel components from the standard hidden components
  PageDescriptionLabel.Caption := WizardForm.PageDescriptionLabel.Caption;
  PageNameLabel.Caption := WizardForm.PageNameLabel.Caption;
end;