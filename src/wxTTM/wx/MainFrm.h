/* Copyright (C) 2020 Christoph Theis */


class CMainFrame : public wxMDIParentFrame
{
  public:
    CMainFrame();
   ~CMainFrame() 
    {
      // Damit kann CTT32App den loeschen. Vorher aber das window menu loeschen.
      if (GetWindowMenu() && GetWindowMenu()->IsAttached())
        SetWindowMenu(NULL);
      SetMenuBar(NULL);
    } 
    
  public:
    void  SetFindString(const wxString &str);
    void  SetDefaultCP(const wxString & str) { m_statusBar->SetStatusText(str, 1); }
    void  SetDefaultGR(const wxString & str) { m_statusBar->SetStatusText(str, 2); }
    void  SetDefaultNA(const wxString & str) { m_statusBar->SetStatusText(str, 3); } 

public:
    void  SetMenuBar(wxMenuBar *menuBar);
    
  private:
    void  OnInitDialog(wxInitDialogEvent &event);
    void  OnMenuCommand(wxCommandEvent &);
    void  OnUpdateUI(wxUpdateUIEvent &);

  private:
    void Import(const wxString &title, const wxString &defaultName, bool (*func)(wxTextBuffer &));
    void Export(const wxString &ttile, const wxString &defaultName, bool (*func)(wxTextBuffer &));
    

  private:
    wxStatusBar * m_statusBar;
  
  
  DECLARE_DYNAMIC_CLASS(CMainFrame)
  DECLARE_EVENT_TABLE()
};