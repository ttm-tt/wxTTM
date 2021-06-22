/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"
#include "FormViewEx.h"
#include "TT32App.h"
#include "Settings.h"
#include "ComboBoxEx.h"
#include "ListItem.h"
#include "IdStore.h"
#include "Profile.h"
#include "Res.h"
#include "DatabaseLogin.h"
#include "Printer.h"

extern Profile ttProfile;

IMPLEMENT_DYNAMIC_CLASS(CSettings, CFormViewEx)

BEGIN_EVENT_TABLE(CSettings, CFormViewEx)
  EVT_BUTTON(XRCID("BannerFileSelector"), CSettings::OnBannerFile)
  EVT_BUTTON(XRCID("BannerRemove"), CSettings::OnRemoveLogo)
  EVT_BUTTON(XRCID("LogoFileSelector"), CSettings::OnLogoFile)
  EVT_BUTTON(XRCID("LogoRemove"), CSettings::OnRemoveLogo)
  EVT_BUTTON(XRCID("SponsorFileSelector"), CSettings::OnSponsorFile)
  EVT_BUTTON(XRCID("SponsorRemove"), CSettings::OnRemoveSponsor)
  EVT_BUTTON(XRCID("BackupPathSelector"), CSettings::OnBackupPathSelector)
  EVT_BUTTON(XRCID("ChooseFont"), CSettings::OnChooseFont)

  EVT_CHECKBOX(XRCID("WindowsAuthentication"), CSettings::OnWindowsAuthentication)

  EVT_COMBOBOX(XRCID("SelectFont"), CSettings::OnSelectFont)
END_EVENT_TABLE()


class LangItem : public ListItem
{
  public:
    LangItem(int id) : ListItem(id, wxLocale::GetLanguageInfo((wxLanguage) id)->Description)
    {
    }
};


// CSettings dialog

CSettings::CSettings() : CFormViewEx()
{
}

CSettings::~CSettings()
{
}

// CSettings message handlers

void CSettings::OnInitialUpdate()
{
  CFormViewEx::OnInitialUpdate();

  wxComboBox *cbFonts = XRCCTRL(*this, "SelectFont", wxComboBox);

  wxString key = ttProfile.GetFirstKey("Raster");
  while (!key.IsEmpty())
  {
    cbFonts->Append(key);
    fontMap[key] = ttProfile.GetString(PRF_RASTER, key);
    key = ttProfile.GetNextKey();
  }

  cbFonts->Select(0);
  OnSelectFont(wxCommandEvent());
  
  m_title = CTT32App::instance()->GetReportTitle();
  m_subtitle = CTT32App::instance()->GetReportSubtitle();
  m_language = CTT32App::instance()->GetLanguage();
  m_type = CTT32App::instance()->GetType() - 1;
  m_table = CTT32App::instance()->GetTable() - 1;

  m_backupPath = CTT32App::instance()->GetBackupPath();
  m_backupTime = CTT32App::instance()->GetBackupTime();
  m_appendTimestamp = CTT32App::instance()->GetBackupAppendTimestamp();
  m_backupKeepLast = CTT32App::instance()->GetBackupKeepLast();
  m_backupNofToKeep = CTT32App::instance()->GetBackupKeepNofItems();
  
  m_assocNameWidth = CTT32App::instance()->GetPrintNationNameWidth();
  m_assocDescWidth = CTT32App::instance()->GetPrintNationDescWidth();
  m_assocRegionWidth = CTT32App::instance()->GetPrintNationRegionWidth();
  m_teamNameWidth = CTT32App::instance()->GetPrintTeamNameWidth();
  m_plNrWidth = CTT32App::instance()->GetPrintStartNrWidth();
  m_captionMarginKO = std::floor(100. * CTT32App::instance()->GetPrintCaptionMarginKO());
  m_printAssociation = CTT32App::instance()->GetPrintAssociation();
  m_printAssociationTeam = CTT32App::instance()->GetPrintAssociationTeam();
  m_printBanner = CTT32App::instance()->GetPrintBanner();
  m_printLogo = CTT32App::instance()->GetPrintLogo();
  m_printSponsor = CTT32App::instance()->GetPrintSponsor();
  m_printLanguage = CTT32App::instance()->GetPrintLanguage();
  m_printScaleToPaperSize = CTT32App::instance()->GetPrintScaleToPaperSize();
  
  m_printCallArea = CTT32App::instance()->GetPrintScoreExtras();
  m_printPlayersSignature = CTT32App::instance()->GetPrintPlayersSignature();
  m_printCoaches = CTT32App::instance()->GetPrintScoreCoaches();
  m_printUmpires = CTT32App::instance()->GetPrintScoreUmpires();
  m_printUmpireName = CTT32App::instance()->GetPrintScoreUmpireName();
  m_printStartEnd = CTT32App::instance()->GetPrintScoreStartEnd();
  m_printRemarks = CTT32App::instance()->GetPrintScoreRemarks();
  m_printServiceTimeout = CTT32App::instance()->GetPrintScoreServiceTimeout();

  FindWindow("Title")->SetValidator(wxGenericValidator(&m_title));
  FindWindow("Subtitle")->SetValidator(wxGenericValidator(&m_subtitle));
  FindWindow("Type")->SetValidator(wxGenericValidator(&m_type));
  FindWindow("Table")->SetValidator(wxGenericValidator(&m_table));

  FindWindow("Server")->SetValidator(wxGenericValidator(&m_server));
  FindWindow("Database")->SetValidator(wxGenericValidator(&m_dbName));
  FindWindow("WindowsAuthentication")->SetValidator(wxGenericValidator(&m_useWindowsAuthentication));
  FindWindow("Username")->SetValidator(wxGenericValidator(&m_userName));
  FindWindow("Password")->SetValidator(wxGenericValidator(&m_password));
  FindWindow("BackupPath")->SetValidator(wxGenericValidator(&m_backupPath));
  FindWindow("BackupTime")->SetValidator(wxGenericValidator(&m_backupTime));
  FindWindow("BackupAppendTimestamp")->SetValidator(wxGenericValidator(&m_appendTimestamp));
  FindWindow("BackupKeepLast")->SetValidator(wxGenericValidator(&m_backupKeepLast));
  FindWindow("BackupsToKeep")->SetValidator(wxGenericValidator(&m_backupNofToKeep));

  FindWindow("AssocNameWidth")->SetValidator(wxGenericValidator(&m_assocNameWidth));
  FindWindow("AssocDescWidth")->SetValidator(wxGenericValidator(&m_assocRegionWidth));
  FindWindow("AssocRegionWidth")->SetValidator(wxGenericValidator(&m_assocDescWidth));
  FindWindow("TeamNameWidth")->SetValidator(wxGenericValidator(&m_teamNameWidth));
  FindWindow("PlNrWidth")->SetValidator(wxGenericValidator(&m_plNrWidth));
  FindWindow("CaptionMarginKO")->SetValidator(wxGenericValidator(&m_captionMarginKO));
  FindWindow("PrintAssociation")->SetValidator(wxGenericValidator(&m_printAssociation));
  FindWindow("PrintAssociationTeam")->SetValidator(wxGenericValidator(&m_printAssociationTeam));
  FindWindow("PrintBanner")->SetValidator(wxGenericValidator(&m_printBanner));
  FindWindow("PrintLogo")->SetValidator(wxGenericValidator(&m_printLogo));
  FindWindow("PrintSponsor")->SetValidator(wxGenericValidator(&m_printSponsor));

  FindWindow("BannerImage")->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CSettings::OnBannerImageDoubleClick), NULL, this);
  FindWindow("LogoImage")->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CSettings::OnLogoImageDoubleClick), NULL, this);
  FindWindow("SponsorImage")->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(CSettings::OnSponsorImageDoubleClick), NULL, this);

  CComboBoxEx *langCb = XRCCTRL(*this, "Language", CComboBoxEx);
  CComboBoxEx *printCb = XRCCTRL(*this, "PrintLanguage", CComboBoxEx);

  FindWindow("PrintScaleToPaperSize")->SetValidator(wxGenericValidator(&m_printScaleToPaperSize));

  FindWindow("PrintScoreServiceTimeout")->SetValidator(wxGenericValidator(&m_printServiceTimeout));
  FindWindow("PrintCallArea")->SetValidator(wxGenericValidator(&m_printCallArea));
  FindWindow("PrintPlayersSignature")->SetValidator(wxGenericValidator(&m_printPlayersSignature));
  FindWindow("PrintScoreCoaches")->SetValidator(wxGenericValidator(&m_printCoaches));
  FindWindow("PrintScoreUmpires")->SetValidator(wxGenericValidator(&m_printUmpires));
  FindWindow("PrintScoreUmpireName")->SetValidator(wxGenericValidator(&m_printUmpireName));
  FindWindow("PrintScoreStartEnd")->SetValidator(wxGenericValidator(&m_printStartEnd));
  FindWindow("PrintScoreRemarks")->SetValidator(wxGenericValidator(&m_printRemarks));

  m_server = TTDbse::instance()->GetServer();
  m_dbName = TTDbse::instance()->GetDatabase();
  m_useWindowsAuthentication = TTDbse::instance()->IsWindowsAuthenticaton();
  m_userName = TTDbse::instance()->GetUsername();
  m_password = TTDbse::instance()->GetPassword();

  if (m_server == "(local)")
    m_server = "localhost";

  wxString currentServer = m_server;

  wxComboBox *cbDatabases = XRCCTRL(*this, "Database", wxComboBox);

  std::list<wxString> dbList = TTDbse::instance()->ListDatabases( currentServer, TTDbse::instance()->GetConnectionString() );
  for (std::list<wxString>::iterator it = dbList.begin();
        it != dbList.end(); it++)
  {
    cbDatabases->AppendString( (*it) );
  }

  if (cbDatabases->GetCount() == 0)
  {
    cbDatabases->AppendString(CTT32App::instance()->GetDatabase());
  }

  // cbDatabases->SetSelection(cbDatabases->FindString(CTT32App::instance()->GetDatabase()));

  // Ueber die Verzeichnisse iterieren und alle Sprachen dort drin einfuegen
  wxString xrcPath = CTT32App::instance()->GetResourcesPath();
  wxDir dir(xrcPath);
  wxString tmp;
  for (bool ret = dir.GetFirst(&tmp, wxEmptyString, wxDIR_DIRS); ret; ret = dir.GetNext(&tmp))
  {
    if (!wxFile::Exists(xrcPath + "/" + tmp + "/" TT_MOFILE))
      continue;

    const wxLanguageInfo *info = wxLocale::FindLanguageInfo(tmp);
    if (info)
    {
      langCb->AddListItem(new LangItem(info->Language));
      printCb->AddListItem(new LangItem(info->Language));
    }
  }

  langCb->SetCurrentItem(wxLocale::FindLanguageInfo(m_language)->Language);
  printCb->SetCurrentItem(wxLocale::FindLanguageInfo(m_printLanguage)->Language);

  if (langCb->GetCount() == 0)
    langCb->AddListItem(new LangItem(wxLANGUAGE_ENGLISH));

  if (!langCb->GetCurrentItem())
    langCb->Select(0);

  if (printCb->GetCount() == 0)
    printCb->AddListItem(new LangItem(wxLANGUAGE_ENGLISH));

  if (!printCb->GetCurrentItem())
    printCb->Select(0);

  wxImage banner;
  if (IdStore::GetBannerImage(banner))
  {
    wxImage img = ScaleImage(banner);
    FindWindow("BannerImage")->SetMinSize(img.GetSize());
    FindWindow("BannerImage")->SetSize(img.GetSize());
    XRCCTRL(*this, "BannerImage", wxStaticBitmap)->SetBitmap(wxBitmap(img));
  }
  else
    FindWindow("BannerImage")->Show(false);

  wxImage logo;
  if (IdStore::GetLogoImage(logo))
  {
    wxImage img = ScaleImage(logo);
    FindWindow("LogoImage")->SetMinSize(img.GetSize());
    FindWindow("LogoImage")->SetSize(img.GetSize());
    XRCCTRL(*this, "LogoImage", wxStaticBitmap)->SetBitmap(wxBitmap(img));
  }
  else
    FindWindow("LogoImage")->Show(false);

  wxImage sponsor;
  if (IdStore::GetSponsorImage(sponsor))
  {
    wxImage img = ScaleImage(sponsor);
    FindWindow("SponsorImage")->SetMinSize(img.GetSize());
    FindWindow("SponsorImage")->SetSize(img.GetSize());
    XRCCTRL(*this, "SponsorImage", wxStaticBitmap)->SetBitmap(wxBitmap(img));
  }
  else
    FindWindow("SponsorImage")->Show(false);

  FindWindow("Username")->Enable(!m_useWindowsAuthentication);
  FindWindow("Password")->Enable(!m_useWindowsAuthentication);

  XRCCTRL(*this, "Server", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CSettings::OnKillFocus), NULL, this);


  TransferDataToWindow();
}


void CSettings::OnKillFocus(wxFocusEvent &evt)
{
  evt.Skip();

  if (m_server == "(local)")
    m_server = "localhost";
  
  wxString tmp = m_server;
  
  TransferDataFromWindow();
  
  if (m_server == "(local)")
    m_server = "localhost";

  if (tmp != m_server)
  {
    wxString connStr = TTDbse::IsLocalAddress(m_server) ? TTDbse::instance()->GetConnectionString() : "";
    XRCCTRL(*this, "Database", wxComboBox)->Clear();

    if (m_server != "localhost")
    {
      CDatabaseLogin *dlg = (CDatabaseLogin *)wxXmlResource::Get()->LoadDialog(CTT32App::instance()->GetTopWindow(), "DatabaseLogin");
      dlg->SetServer(m_server);
      dlg->SetDatabase("");

      if (dlg->ShowModal() != wxID_OK)
      {
        delete dlg;
        return;
      }

      wxString connStr = dlg->GetConnectionString();

      m_useWindowsAuthentication = dlg->GetWindowsAuthentication();
      if (m_useWindowsAuthentication)
      {
        m_userName = "";
        m_password = "";
      }
      else
      {
        m_userName = dlg->GetUser();
        m_password = dlg->GetPassword();
      }

      FindWindow("Username")->Enable(!m_useWindowsAuthentication);
      FindWindow("Password")->Enable(!m_useWindowsAuthentication);

      delete dlg;
    }

    SetCursor(*wxHOURGLASS_CURSOR);

    wxComboBox *cbDatabases = XRCCTRL(*this, "Database", wxComboBox);

    wxString currentServer = m_server;
    std::list<wxString> dbList = TTDbse::instance()->ListDatabases( currentServer, connStr );
    for (std::list<wxString>::iterator it = dbList.begin();
          it != dbList.end(); it++)
    {
      cbDatabases->AppendString( (*it) );
    }

    SetCursor(wxNullCursor);    

    FindWindow("BackupPathLabel")->Show(TTDbse::IsLocalAddress(tmp));
    FindWindow("BackupPath")->Show(TTDbse::IsLocalAddress(tmp));
  } 

  TransferDataToWindow();
}


void CSettings::OnWindowsAuthentication(wxCommandEvent &evt)
{
  evt.Skip();

  bool val = XRCCTRL(*this, "WindowsAuthentication", wxCheckBox)->GetValue();

  FindWindow("Username")->Enable(!val);
  FindWindow("Password")->Enable(!val);
}


void CSettings::OnOK()
{
  TransferDataFromWindow();

  if ( m_server != TTDbse::instance()->GetServer() ||
       m_dbName != TTDbse::instance()->GetDatabase() ||
       m_useWindowsAuthentication != TTDbse::instance()->IsWindowsAuthenticaton() ||
       !m_useWindowsAuthentication && m_userName != TTDbse::instance()->GetUsername() ||
       !m_useWindowsAuthentication && m_password != TTDbse::instance()->GetPassword() )
  {
    short type = m_type + 1, table = m_table + 1;
    CTT32App::instance()->CreateTournament(CTT32App::instance()->GetTournament(), m_dbName, m_server, m_useWindowsAuthentication, m_userName, m_password, type, table);
  }
  
  m_language = wxLocale::GetLanguageInfo(
    XRCCTRL(*this, "Language", CComboBoxEx)->GetCurrentItem()->GetID())->CanonicalName;
  m_printLanguage = wxLocale::GetLanguageInfo(
    XRCCTRL(*this, "PrintLanguage", CComboBoxEx)->GetCurrentItem()->GetID())->CanonicalName;

  CTT32App::instance()->SetReportTitle(m_title);
  CTT32App::instance()->SetReportSubtitle(m_subtitle);
  CTT32App::instance()->SetLanguage(m_language);
  CTT32App::instance()->SetType(m_type + 1);
  CTT32App::instance()->SetTable(m_table + 1);
  CTT32App::instance()->SetPrintScoreServiceTimeout(m_printServiceTimeout);
  CTT32App::instance()->SetPrintPlayersSignature(m_printPlayersSignature);
  CTT32App::instance()->SetPrintScoreCoaches(m_printCoaches);
  CTT32App::instance()->SetPrintScoreUmpires(m_printUmpires);
  CTT32App::instance()->SetPrintScoreUmpireName(m_printUmpireName);
  CTT32App::instance()->SetPrintScoreExtras(m_printCallArea);
  CTT32App::instance()->SetPrintScoreStartEnd(m_printStartEnd);
  CTT32App::instance()->SetPrintScoreRemarks(m_printRemarks);

  CTT32App::instance()->SetBackupTime(m_backupTime);
  CTT32App::instance()->SetBackupPath(m_backupPath);
  CTT32App::instance()->SetBackupAppendTimestamp(m_appendTimestamp);
  CTT32App::instance()->SetBackupKeepLast(m_backupKeepLast);
  CTT32App::instance()->SetBackupKeepNofItems(m_backupNofToKeep);
  
  CTT32App::instance()->SetPrintNationNameWidth(m_assocNameWidth);
  CTT32App::instance()->SetPrintNationDescWidth(m_assocDescWidth);
  CTT32App::instance()->SetPrintNationDescWidth(m_assocRegionWidth);
  CTT32App::instance()->SetPrintTeamNameWidth(m_teamNameWidth);
  CTT32App::instance()->SetPrintStartNrWidth(m_plNrWidth);
  CTT32App::instance()->SetPrintCaptionMarginKO( ((double) m_captionMarginKO) / 100. );
  CTT32App::instance()->SetPrintAssociation(m_printAssociation);
  CTT32App::instance()->SetPrintAssociationTeam(m_printAssociationTeam);
  CTT32App::instance()->SetPrintBanner(m_printBanner);
  CTT32App::instance()->SetPrintLogo(m_printLogo);
  CTT32App::instance()->SetPrintSponsor(m_printSponsor);
  CTT32App::instance()->SetPrintLanguage(m_printLanguage);
  CTT32App::instance()->SetPrintScaleToPaperSize(m_printScaleToPaperSize);

  if (m_bannerFilename == "<DELETE>")
    IdStore::SetBannerImage(wxString());
  else if (!m_bannerFilename.IsEmpty())
    IdStore::SetBannerImage(m_bannerFilename);

  if (m_logoFilename == "<DELETE>")
    IdStore::SetLogoImage(wxString());
  else if (!m_logoFilename.IsEmpty())
    IdStore::SetLogoImage(m_logoFilename);

  if (m_sponsorFilename == "<DELETE>")
    IdStore::SetSponsorImage(wxString());
  else if (!m_sponsorFilename.IsEmpty())
    IdStore::SetSponsorImage(m_sponsorFilename);

  for (auto it = fontMap.begin(); it != fontMap.end(); it++)
    ttProfile.AddString(PRF_RASTER, it->first, it->second);
    
  CFormViewEx::OnOK();
}


void CSettings::OnBannerFile(wxCommandEvent &)
{
  wxFileDialog dlg(this, _("Select Banner Image"), CTT32App::instance()->GetPath(), "",
    _("Image Files (*.bmp, *.jpg, *.png, *.gif)|*.bmp;*.jpg;*.png;*.gif|All Files (*.*)|*.*"),
    wxFD_OPEN | wxFD_FILE_MUST_EXIST);

  if (dlg.ShowModal() == wxID_OK)
  {
    wxImage banner(m_bannerFilename = dlg.GetPath());
    XRCCTRL(*this, "BannerImage", wxStaticBitmap)->SetBitmap(wxBitmap(ScaleImage(banner)));
    FindWindow("BannerImage")->Show(true);
  }
}


void CSettings::OnRemoveBanner(wxCommandEvent &)
{
  m_bannerFilename = "<DELETE>";
  FindWindow("BannerImage")->Show(false);
}


void CSettings::OnLogoFile(wxCommandEvent &)
{
  wxFileDialog dlg(this, _("Select Logo Image"), CTT32App::instance()->GetPath(), "", 
                   _("Image Files (*.bmp, *.jpg, *.png, *.gif)|*.bmp;*.jpg;*.png;*.gif|All Files (*.*)|*.*"), 
                   wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                   
  if (dlg.ShowModal() == wxID_OK)
  {
    wxImage logo(m_logoFilename = dlg.GetPath());
    XRCCTRL(*this, "LogoImage", wxStaticBitmap)->SetBitmap(wxBitmap(ScaleImage(logo)));
    FindWindow("LogoImage")->Show(true);
  }                   
}


void CSettings::OnRemoveLogo(wxCommandEvent &)
{
  m_logoFilename = "<DELETE>";
  FindWindow("LogoImage")->Show(false);
}


void CSettings::OnSponsorFile(wxCommandEvent &)
{
  wxFileDialog dlg(this, _("Select Sponsor Image"), CTT32App::instance()->GetPath(), "", 
                   _("Image Files (*.bmp, *.jpg, *.png, *.gif)|*.bmp;*.jpg;*.png;*.gif|All Files (*.*)|*.*"), 
                   wxFD_OPEN | wxFD_FILE_MUST_EXIST);
                   
  if (dlg.ShowModal() == wxID_OK)
  {
    wxImage sponsor(m_sponsorFilename = dlg.GetPath());
    XRCCTRL(*this, "SponsorImage", wxStaticBitmap)->SetBitmap(wxBitmap(ScaleImage(sponsor)));
    FindWindow("SponsorImage")->Show(true);
  }                   
}


void CSettings::OnRemoveSponsor(wxCommandEvent &)
{
  m_sponsorFilename = "<DELETE>";
  FindWindow("SponsorImage")->Show(false);
}


void CSettings::OnSelectFont(wxCommandEvent &)
{
  wxString key = XRCCTRL(*this, "SelectFont", wxComboBox)->GetValue();
  wxString defString = fontMap[key];
  wxString fontDesc = Printer::GetFontDescription(defString);
  wxFont font;
  font.SetNativeFontInfoUserDesc(fontDesc);
  XRCCTRL(*this, "FontText", wxTextCtrl)->SetValue(fontDesc);
  XRCCTRL(*this, "FontText", wxTextCtrl)->SetFont(font);
}


void CSettings::OnChooseFont(wxCommandEvent &)
{
  wxString key = XRCCTRL(*this, "SelectFont", wxComboBox)->GetValue();
  wxString defString = fontMap[key];

  if (Printer::ChooseFont(defString))
    fontMap[key] = defString;

  XRCCTRL(*this, "FontText", wxTextCtrl)->SetValue(Printer::GetFontDescription(defString));
}


void CSettings::OnBackupPathSelector(wxCommandEvent &)
{
  TransferDataFromWindow();

  wxString path = wxDirSelector(_("Select Backup Path"), m_backupPath);
  if (!path.IsEmpty())
    m_backupPath = path;

  TransferDataToWindow();
}


void CSettings::OnBannerImageDoubleClick(wxMouseEvent &)
{
  wxImage img;
  if (!IdStore::GetBannerImage(img))
    return;

  wxDialog dlg(wxTheApp->GetTopWindow(), wxID_ANY, wxT("Banner"), wxDefaultPosition, img.GetSize());
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  dlg.SetSizer(sizer);
  wxStaticBitmap *bmp = new wxStaticBitmap(&dlg, wxID_ANY, wxNullBitmap);
  bmp->SetSize(img.GetSize());
  sizer->Add(bmp);
  bmp->SetBitmap(wxBitmap(img));

  dlg.ShowModal();
}


void CSettings::OnLogoImageDoubleClick(wxMouseEvent &)
{
  wxImage img;
  if (!IdStore::GetLogoImage(img))
    return;

  wxDialog dlg(wxTheApp->GetTopWindow(), wxID_ANY, wxT("Logo"), wxDefaultPosition, img.GetSize());
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  dlg.SetSizer(sizer);
  wxStaticBitmap *bmp = new wxStaticBitmap(&dlg, wxID_ANY, wxNullBitmap);
  bmp->SetSize(img.GetSize());
  sizer->Add(bmp);
  bmp->SetBitmap(wxBitmap(img));

  dlg.ShowModal();
}


void CSettings::OnSponsorImageDoubleClick(wxMouseEvent &)
{
  wxImage img;
  if (!IdStore::GetSponsorImage(img))
    return;

  wxDialog dlg(wxTheApp->GetTopWindow(), wxID_ANY, wxT("Sponsor"), wxDefaultPosition, img.GetSize());
  wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
  dlg.SetSizer(sizer);
  wxStaticBitmap *bmp = new wxStaticBitmap(&dlg, wxID_ANY, wxNullBitmap);
  bmp->SetSize(img.GetSize());
  sizer->Add(bmp);
  bmp->SetBitmap(wxBitmap(img));

  dlg.ShowModal();
}


wxImage & CSettings::ScaleImage(wxImage &img)
{
  static int maxWidth = 150;
  static int maxHeight = 64;

  img.Rescale(img.GetWidth() * maxHeight / img.GetHeight(), maxHeight);
  if (img.GetWidth() > maxWidth)
  {
    img.Rescale(150, img.GetHeight() * maxWidth / img.GetWidth());
    img.Resize(wxSize(maxWidth, maxHeight), wxPoint(0, (maxHeight - img.GetHeight()) / 2));
  }

  return img;
}