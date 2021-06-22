/* Copyright (C) 2020 Christoph Theis */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <map>

// CSettings dialog

class CSettings : public CFormViewEx
{
  public:
	  CSettings();
	 ~CSettings();

  protected:
    void OnInitialUpdate();
    void OnOK();

  private:
    wxImage & ScaleImage(wxImage &);
    
  private:
    void OnBannerFile(wxCommandEvent &);
    void OnRemoveBanner(wxCommandEvent &);
    void OnLogoFile(wxCommandEvent &);
    void OnRemoveLogo(wxCommandEvent &);
    void OnSponsorFile(wxCommandEvent &);
    void OnRemoveSponsor(wxCommandEvent &);
    void OnChooseFont(wxCommandEvent &);

    void OnBackupPathSelector(wxCommandEvent &);

    void OnBannerImageDoubleClick(wxMouseEvent &);
    void OnLogoImageDoubleClick(wxMouseEvent &);
    void OnSponsorImageDoubleClick(wxMouseEvent &);

    void OnWindowsAuthentication(wxCommandEvent &);
    
	  void OnKillFocus(wxFocusEvent &);

    void OnSelectFont(wxCommandEvent &);

  private:
    wxString m_title;
    wxString m_subtitle;
    int      m_type;
    int      m_table;
    wxString m_language;
    bool     m_printCallArea;
    bool     m_printPlayersSignature;
    bool     m_printCoaches;
    bool     m_printUmpires;
    bool     m_printUmpireName;
    bool     m_printStartEnd;
    bool     m_printRemarks;
    bool     m_printServiceTimeout;

    wxString m_server;
    wxString m_dbName;
    bool     m_useWindowsAuthentication;
    wxString m_userName;
    wxString m_password;
    wxString m_backupPath;
    int      m_backupTime;
    bool     m_appendTimestamp;
    bool     m_backupKeepLast;
    int      m_backupNofToKeep;

    bool     m_printBanner;
    bool     m_printLogo;
    bool     m_printSponsor;
    int      m_assocNameWidth;
    int      m_assocDescWidth;
    int      m_assocRegionWidth;
    int      m_teamNameWidth;
    int      m_plNrWidth;
    int      m_captionMarginKO;
    int      m_printAssociation;
    int      m_printAssociationTeam;
    wxString m_printLanguage;
    wxString m_bannerFilename;
    wxString m_logoFilename;
    wxString m_sponsorFilename;
    bool     m_printScaleToPaperSize;

    std::map<wxString, wxString> fontMap;
    
    
  DECLARE_DYNAMIC_CLASS(CSettings)
  DECLARE_EVENT_TABLE()
};

#endif