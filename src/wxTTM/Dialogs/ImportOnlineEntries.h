/* Copyright (C) 2020 Christoph Theis */

#ifndef _IMPORTONLINEENTRIES_H_
#define _IMPORTONLINEENTRIES_H_

#include "TimXmlRpc.h"

#include <map>
#include <set>


class CImportOnlineEntries : public wxWizard
{
  public:
    static bool Import();

  public:
    CImportOnlineEntries();
   ~CImportOnlineEntries();


  private:
    void OnPageChanging(wxWizardEvent &);
    void OnPageShown(wxWizardEvent &);

    void OnTextUrl(wxCommandEvent &);
    void OnSelectDirectory(wxCommandEvent &);
    void OnChangeImportEntries(wxCommandEvent &);

  private:
    void OnPageChangingConnect(wxWizardEvent &);
    void OnPageChangingSelect(wxWizardEvent &);
    void OnPageChangingImport(wxWizardEvent &);

  private:
    static unsigned ConnectThread(void *);
    void ConnectThreadImpl(const wxString &url, const wxString &user, const wxString &pwd);

    static unsigned ImportThread(void *);
    bool ImportThreadRead();
    bool ImportThreadImport();

    void ClearTournament();
    void ImportCP();
    void ImportNA();
    void ImportPL();
    void ImportRP();
    void ImportLT();

  private:
    XmlRpcClient xmlRpcClient;
    wxFileName cpFileName;
    wxFileName naFileName;
    wxFileName plFileName;
    wxFileName phFileName;
    wxFileName rpFileName;
    wxFileName ltsFileName;
    wxFileName ltdFileName;
    wxFileName ltxFileName;
    wxFileName lttFileName;

    bool inThread;

  DECLARE_DYNAMIC_CLASS(CImportOnlineEntries)
  DECLARE_EVENT_TABLE()
};

#endif