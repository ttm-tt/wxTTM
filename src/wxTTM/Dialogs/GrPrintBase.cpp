/* Copyright (C) 2020 Christoph Theis */

// CGrPrintBase.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "GrPrintBase.h"

#include "GrListStore.h"

#include "GrItem.h"

#include "Printer.h"
#include "RasterKO.h"
#include "RasterRR.h"

#include "GrOptions.h"

#include "InfoSystem.h"
#include "Printer.h"
#include "Profile.h"
#include "Res.h"

#include "SQLException.h"

#include <time.h>


IMPLEMENT_DYNAMIC_CLASS(CGrPrintBase, CFormViewEx)

BEGIN_EVENT_TABLE(CGrPrintBase, CFormViewEx)
  EVT_COMBOBOX(XRCID("ListOptions"), CGrPrintBase::OnOptionChange)
  EVT_BUTTON(XRCID("Options"), CGrPrintBase::OnOptions)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CGrPrintBase dialog
CGrPrintBase::CGrPrintBase() : CFormViewEx()
{
  m_options = NULL;
  m_grList = NULL;
}


CGrPrintBase::~CGrPrintBase()
{
}


// -----------------------------------------------------------------------
// CGrPrintBase message handlers

void CGrPrintBase::OnInitialUpdate() 
{
  CFormViewEx::OnInitialUpdate();

  m_options = XRCCTRL(*this, "ListOptions", wxComboBox);

  std::list<wxString> options = PoStore().List();
  for each (wxString option in options)
  {
    m_options->AppendString(option);
  }

  wxString poName = ttProfile.GetString(PRF_LOCAL_SETTINGS, PRF_PROPTIONS_LASTUSED);
  if (m_options->FindString(poName) != wxNOT_FOUND)
    m_options->Select(m_options->FindString(poName));
  else
    m_options->Select(0);

  OnOptionChange(wxCommandEvent());
}


unsigned CGrPrintBase::PrintThread(void *arg)
{
  int  offstX = 0, offstY = 0, page = 0;

  long lastCpID = 0;

  bool isMultiple = ((PrintThreadStruct *)arg)->isMultiple;
  bool isThread = ((PrintThreadStruct *) arg)->isThread;
  long nofItems = ((PrintThreadStruct *) arg)->nofItems;
  long *idItems = ((PrintThreadStruct *) arg)->idItems;
  CpRec m_cp    = ((PrintThreadStruct *) arg)->m_cp;
  PrintRasterOptions m_po = ((PrintThreadStruct *) arg)->m_po;
  Connection *connPtr = ((PrintThreadStruct *) arg)->connPtr;
  Printer *printer = ((PrintThreadStruct *) arg)->printer;  
  bool isPreview = ((PrintThreadStruct *)arg)->isPreview;
  bool isPdf = ((PrintThreadStruct *)arg)->isPdf;

  timestamp ct;

  // I don't want those variable floating around
  {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    ct.year = tm->tm_year + 1900;
    ct.month = tm->tm_mon + 1;
    ct.day = tm->tm_mday;
    ct.hour = tm->tm_hour;
    ct.minute = tm->tm_min;
    ct.second = tm->tm_sec;
    ct.fraction = 0;
  }

  wxString path;

  if (isMultiple)
    path = printer->GetFilename();

  // Reset some flags
  if (!m_po.rrSlctRound)
    m_po.rrFromRound = m_po.rrToRound = 0;
  if (!m_po.koSlctMatch)
    m_po.koFromMatch = m_po.koToMatch = m_po.koLastMatches = 0;
  if (!m_po.koSlctRound)
    m_po.koFromRound = m_po.koToRound = m_po.koLastRounds = 0;

  wxString docString = wxString::Format("%s %s", _("Result Forms"), m_cp.cpDesc);

  // Wenn ich PDF in mehrere File drucke darf ich hier noch kein StartDoc machen:
  // Wenn der Filename bekannt wird muss ich ein EndDoc machen und das schlaegt fehlt, wenn ich keinen Filenamen habe
  if (isMultiple || printer->StartDoc(docString))
  {
    // printer->StartPage();

    wxString lastStage;

    for (int idx = 0; idx < nofItems; isThread ? CTT32App::ProgressBarStep(), idx++ : idx++)
    {
      GrListStore  gr(connPtr);
      gr.SelectById(idItems[idx]);
      if (!gr.Next())
        continue;

      if (isMultiple && lastCpID != gr.cpID)
      {
        if (lastCpID && !printer->IsPreview())
          printer->EndDoc();

        CpListStore cp(connPtr);
        cp.SelectById(gr.cpID);
        cp.Next();

        m_cp = cp;

        printer->SetFilename(wxFileName(path, cp.cpName, "pdf").GetFullPath());

        if (!printer->IsPreview())
          printer->StartDoc(docString);

        lastCpID = gr.cpID;

        // Start new page
        offstX = 0;
        offstY = 0;
        page = 0;
      }

      PrintRasterOptions options = m_po;

      // Immer Seitenumbruch, wenn sich die Stufe aendert
      if (gr.grStage != lastStage)
      {
        if (gr.grModus == MOD_RR)
          options.rrNewPage = 1;
        else
          options.koNewPage = 1;
      }

      lastStage = gr.grStage;

      if (gr.grModus != MOD_RR)
      {
        bool fromLastRounds = options.koLastRounds;

        if (options.koLastRounds)
        {
          options.koFromRound = gr.NofRounds() - options.koFromRound + 1;
          options.koToRound = gr.NofRounds() - options.koToRound + 1;

          options.koLastRounds = 0;
        }
        else if (!options.koSlctRound)
        {
          options.koFromRound = 1;
          options.koToRound = gr.NofRounds();
        }

        if (options.koFromRound < 0)
          options.koFromRound = gr.NofRounds() + options.koFromRound;
        if (options.koToRound < 0)
          options.koToRound = gr.NofRounds() + options.koToRound;

        if (options.koFromRound <= 0)
          options.koFromRound = 1;
        if (options.koToRound <= 0)
          options.koToRound = gr.NofRounds();

        if (gr.grNofRounds)
          options.koToRound = std::min(options.koToRound, gr.grNofRounds);

        if (options.koSlctRound && options.koNoQuRounds && gr.grQualRounds)
        {
          if (fromLastRounds)
          {
            if (options.koFromRound <= gr.grQualRounds)
              options.koFromRound = gr.grQualRounds + 1;
          }
          else
          {
            options.koFromRound += gr.grQualRounds;
          }
        }

        if (options.koLastMatches)
        {
          options.koFromMatch = gr.NofMatches(options.koFromRound) - options.koFromMatch + 1;
          options.koToMatch = gr.NofMatches(options.koFromRound) - options.koToMatch + 1;

          options.koLastMatches = 0;
        }
        else if (!options.koSlctMatch)
        {
          options.koFromMatch = 1;
          options.koToMatch = gr.NofMatches(options.koFromRound);
        }

        if (options.koFromMatch <= 0)
          options.koFromMatch = 1;
        if (options.koFromMatch > gr.NofMatches(options.koFromRound))
          options.koFromMatch = gr.NofMatches(options.koFromRound);

        if (options.koToMatch <= 0 || options.koToMatch > gr.NofMatches(options.koFromRound))
          options.koToMatch = gr.NofMatches(options.koFromRound);

        if (gr.grNofMatches)
          options.koToMatch = std::min((int)options.koToMatch, gr.grNofMatches >> (options.koFromRound - 1));

        if (options.koPrintOnlyScheduled)
          options.koToRound = std::min(options.koToRound, gr.GetLastScheduledRound(gr.grID));
      }

      switch (gr.grModus)
      {
        case MOD_SKO:
        {
          // Neue Seite fuer Gruppe?
          if (page && options.koNewPage)
          {
            printer->EndPage();
            page = 0;
          }

          RasterSKO  rasterSKO(printer, connPtr);
          rasterSKO.Print(m_cp, gr, options, &offstX, &offstY, &page);

          if (options.koPrintNotes)
            rasterSKO.PrintNotes(gr, options, &offstX, &offstY, &page);

          if (m_cp.cpType == CP_TEAM && options.koTeamDetails)
            rasterSKO.PrintMatches(m_cp, gr, options, &offstX, &offstY, &page);

          break;
        }

        case MOD_DKO:
        case MOD_MDK:
        {
          // Neue Seite fuer Gruppe?
          if (page && options.koNewPage)
          {
            printer->EndPage();
            page = 0;
          }

          RasterDKO  rasterDKO(printer, connPtr);
          rasterDKO.Print(m_cp, gr, options, &offstX, &offstY, &page);

          if (options.koPrintNotes)
            rasterDKO.PrintNotes(gr, options, &offstX, &offstY, &page);

          if (options.koTeamDetails)
            rasterDKO.PrintMatches(m_cp, gr, options, &offstX, &offstY, &page);

          break;
        }

        case MOD_PLO:
        {
          // Neue Seite fuer Gruppe?
          if (page && options.koNewPage)
          {
            printer->EndPage();
            page = 0;
          }

          RasterPLO  rasterPLO(printer, connPtr);
          rasterPLO.Print(m_cp, gr, options, &offstX, &offstY, &page);

          if (options.koPrintNotes)
            rasterPLO.PrintNotes(gr, options, &offstX, &offstY, &page);

          if (options.koTeamDetails)
            rasterPLO.PrintMatches(m_cp, gr, options, &offstX, &offstY, &page);

          break;
        }

        case MOD_RR:
        {
          // Neue Seite je Gruppe
          if (page && options.rrNewPage)
          {
            printer->EndPage();
            page = 0;
          }

          RasterRR  rasterRR(printer, connPtr);
          rasterRR.Print(m_cp, gr, options, &offstX, &offstY, &page);

          if (options.rrPrintNotes)
            rasterRR.PrintNotes(gr, options, &offstX, &offstY, &page);

          if (options.rrResults) //  || m_po.rrCombined)
          {
            rasterRR.PrintMatches(m_cp, gr, options, &offstX, &offstY, &page);

            if (options.rrCombined && !CTT32App::instance()->GetPrintPreview())
              MtStore(connPtr).UpdateScorePrintedForGroup(gr.grID, true);
          }

          break;
        }

        default:
          break;
      }

      if (!isPreview && !isPdf)
        GrStore(gr, gr.GetConnectionPtr()).SetPrinted(ct);
    }

    printer->EndPage();
    printer->EndDoc();
  }

  delete printer;
  delete[] ((PrintThreadStruct *) arg)->idItems;
  delete (PrintThreadStruct *) arg;

  try
  {
    if (isThread)
      delete connPtr;
  }
  catch (SQLException &e)
  {
    infoSystem.Exception("Closing connection to data source", e);
  }

  return 0;
}


void  CGrPrintBase::OnOptions(wxCommandEvent &)
{
  int ret = 0;

  // Variable dlg kapseln
  {
    CGrOptions *dlg = (CGrOptions *) wxXmlResource::Get()->LoadDialog(this, "GrOptions");
    dlg->SetPrintRasterOptions(&m_po);
  
    ret = dlg->ShowModal();
  }

  switch (ret)
  {
    case wxID_CANCEL :
      return;

    case wxID_OK :
      break;

    default :
    {
      // Kann eigentlich nur "Save As" sein, aber ich kenne die ID nicht
      while (true)
      {
#if 0
        wxTextEntryDialog dlg(this, _("Name"), _("Enter Print Option Name"));
        dlg.SetMaxLength(sizeof(m_po.poName) / sizeof(wxChar) - 1);
        if (dlg.ShowModal() != wxID_OK)
          return;

        wxString name = dlg.GetValue();
#else
        std::list<wxString> options = PoStore().List();
        wxDialog *dlg = wxXmlResource::Get()->LoadDialog(CTT32App::instance()->GetTopWindow(), "EditableSelection");
        dlg->SetTitle(_("Enter Print Option Name"));

        for each (wxString opt in options)
        {
          XRCCTRL(*dlg, "Options", wxComboBox)->AppendString(opt);
        }

        wxString name = XRCCTRL(*this, "ListOptions", wxComboBox)->GetValue();
        XRCCTRL(*dlg, "Options", wxComboBox)->SetValue(name);

        if (dlg->ShowModal() != wxID_OK)
        {
          delete dlg;
          return;
        }

        name = XRCCTRL(*dlg, "Options", wxComboBox)->GetValue();
        delete dlg;
#endif

        // TODO: Warnung, wenn Name in Gebrauch ist
        if (m_po.Exists(name))
        {
          if (!infoSystem.Question(_("Print option %s already exists. Overwrite?"), name.t_str()))
            continue;
        }

        wxStrncpy(m_po.poName, name, sizeof(m_po.poName) / sizeof(wxChar));

        break;
      }
    }
  }

	// Einstellungen ablegen
  m_po.Write(m_po.poName);

  if (!m_options->SetStringSelection(m_po.poName))
  {
    m_options->AppendString(m_po.poName);
    m_options->SetStringSelection(m_po.poName);
  }

  OnOptionChange(wxCommandEvent());
}


void CGrPrintBase::OnOptionChange(wxCommandEvent &)
{
  m_po.Read(m_options->GetStringSelection());

  if (!m_grList)
    return;

  for (int idx = 0; idx < m_grList->GetItemCount(); idx++)
  {
    GrItem *ptr = (GrItem *)m_grList->GetListItem(idx);
    if (ptr == NULL)
      continue;

    if (ptr->gr.grModus == MOD_RR && m_po.rrSlctRound)
      ptr->SetForeground(*wxRED);
    else if (ptr->gr.grModus != MOD_RR && (m_po.koSlctRound || m_po.koSlctMatch))
      ptr->SetForeground(*wxRED);
    else
      ptr->SetForeground(wxNullColour);
  }

  m_grList->Refresh();
}

