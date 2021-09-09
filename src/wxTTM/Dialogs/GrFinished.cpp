/* Copyright (C) 2020 Christoph Theis */

// GrFinished Implementation

#include "stdafx.h"
#include "TT32App.h"
#include "GrFinished.h"

#include "GrItem.h"

#include "InfoSystem.h"
#include "Printer.h"
#include "Profile.h"
#include "Res.h"

#include "SQLException.h"


IMPLEMENT_DYNAMIC_CLASS(CGrFinished, CGrPrintBase)

BEGIN_EVENT_TABLE(CGrFinished, CGrPrintBase)
  EVT_COMBOBOX(XRCID("Type"), CGrFinished::OnTypeChange)
  EVT_BUTTON(IDC_REFRESH, CFormViewEx::OnCommand)
END_EVENT_TABLE()


class GrFinishedItem : public GrItem
{
  public:
    GrFinishedItem(const wxString &cpName_, const GrRec &gr, int rd, const timestamp &ts, MtListStore::Finished what_) : 
      GrItem(gr), cpName(cpName_), mtRound(rd), mtDateTime(ts), what(what_) {}

    virtual int  Compare(const ListItem *itemPtr, int col) const;
    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rect);

  public:
    wxString cpName;
    int mtRound;
    timestamp mtDateTime;
    MtListStore::Finished what;
};


CGrFinished::CGrFinished()
{

}


CGrFinished::~CGrFinished()
{

}


int  GrFinishedItem::Compare(const ListItem *itemPtr, int col) const
{
  const GrFinishedItem *grItem = reinterpret_cast<const GrFinishedItem *>(itemPtr);
  if (!grItem)
    return -1;

  switch (col)
  {
    case 0:
      return wxStrcoll(cpName, grItem->cpName);

    case 1:
      return wxStrcoll(gr.grName, grItem->gr.grName);

    case 2:
      return wxStrcmp(gr.grDesc, grItem->gr.grDesc);

    case 3:
    {
      int ret = wxStrcoll(gr.grStage, grItem->gr.grStage);

      // Wenn gleich (was immer der Fall sein kann), dann auf den Namen zurueckgreifen
      // So hat man eine definierte Sortierung, ohne auf sich auf die Reihenfolge in der DB zu verlassen
      if (ret == 0)
        ret = wxStrcoll(gr.grName, grItem->gr.grName);

      return ret;
    }

    case 4:
      return mtRound - grItem->mtRound;

    case 5 :
      if (mtDateTime < grItem->mtDateTime)
        return +1;
      else if (mtDateTime > grItem->mtDateTime)
        return -1;
      else
        return 0;

    default:
      return ListItem::Compare(itemPtr, col);
  }
}


void  GrFinishedItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  wxRect    rcColumn = rect;

  switch (col)
  {
    case 0:
      if (what >= MtListStore::Finished::Event)
        DrawString(pDC, rcColumn, cpName);
      break;

    case 1:
      if (what >= MtListStore::Finished::Group)
        DrawString(pDC, rcColumn, gr.grName);
      break;

    case 2:
      if (what >= MtListStore::Finished::Group)
        DrawString(pDC, rcColumn, gr.grDesc);
      else
        DrawString(pDC, rcColumn, _("All Groups"));
      break;

    case 3:
      if (what >= MtListStore::Finished::Stage)
        DrawString(pDC, rcColumn, gr.grStage);
      break;

    case 4:
      // Even when only groups are considered show also the (then last played) round
      if (what >= MtListStore::Finished::Group)
        DrawLong(pDC, rcColumn, mtRound);
      break;

    case 5:
      DrawString(pDC, rcColumn, wxString::Format("%02d:%02d", mtDateTime.hour, mtDateTime.minute));
      break;

    case 6:
      if (gr.grPrinted > mtDateTime)
        DrawString(pDC, rcColumn, wxString::Format("%02d:%02d", gr.grPrinted.hour, gr.grPrinted.minute));
      break;

    default:
      break;
  }
}


// -----------------------------------------------------------------------
void CGrFinished::OnInitialUpdate()
{
  CGrPrintBase::OnInitialUpdate();

  FindWindow("Refresh")->SetId(IDC_REFRESH);

  m_grList = XRCCTRL(*this, "Finished", CListCtrlEx);

  // Event
  m_grList->InsertColumn(0, _("Event"), wxALIGN_LEFT, 6 * cW);

  // Group
  m_grList->InsertColumn(1, _("Group"), wxALIGN_LEFT, 6 * cW);

  // Description
  m_grList->InsertColumn(2, _("Description"), wxALIGN_LEFT);

  // Stufe
  m_grList->InsertColumn(3, _("Stage"), wxALIGN_LEFT, 10 * cW);

  // Stufe
  m_grList->InsertColumn(4, _("Round"), wxALIGN_RIGHT, 6 * cW);

  // Stufe
  m_grList->InsertColumn(5, _("Played"), wxALIGN_RIGHT, 6 * cW);

  // Printed
  m_grList->InsertColumn(6, _("Printed"), wxALIGN_RIGHT, 6 * cW);

  OnTypeChange(wxCommandEvent());
}


void CGrFinished::OnEdit()
{
  OnPrint();
}


void  CGrFinished::OnPrint()
{
  // Aktuelle Option sichern
  ttProfile.AddString(PRF_LOCAL_SETTINGS, PRF_PROPTIONS_LASTUSED, m_po.poName);

  if (!m_grList->GetSelectedCount())
    return;

  int nofItems = 0;
  long *idItems = NULL;

  std::vector<GrFinishedItem *> list;

  for (int idx = 0; idx < m_grList->GetItemCount(); idx++)
  {
    if (!m_grList->IsSelected(idx))
      continue;

    GrFinishedItem *itemPtr = (GrFinishedItem *)m_grList->GetListItem(idx);
    list.push_back(itemPtr);
  }

  std::sort(list.begin(), list.end(),
            [](const GrFinishedItem *it1, const GrFinishedItem *it2)
            {
              if (it1->cpName != it2->cpName)
                return it1->cpName > it2->cpName;

              if (it1->what < MtListStore::Finished::Stage)
                return false;

              if (wxString(it1->gr.grStage) != wxString(it2->gr.grStage))
                return wxString(it1->gr.grStage) > wxString(it2->gr.grStage);

              if (it1->what < MtListStore::Finished::Group)
                return false;

              return it1->gr.grID > it2->gr.grID;
            }

  );


  // Set to find duplicate grIDs
  std::set<long> grIds;
  // List of unique grIDs, sorted by event, group
  std::vector<long> ids;

  for (const GrFinishedItem *itemPtr : list)
  {
    switch (itemPtr->what)
    {
      case MtListStore::Finished::Event:
      {
        CpListStore cp;
        GrListStore gr;

        cp.SelectByName(itemPtr->cpName);
        cp.Next();
        cp.Close();

        gr.SelectAll(cp);
        while (gr.Next())
        {
          if (grIds.find(gr.grID) != grIds.end())
            continue;

          grIds.insert(gr.grID);
          ids.push_back(gr.grID);
        }

        gr.Close();

        break;
      }

      case MtListStore::Finished::Stage:
      {
        CpRec cp;
        GrListStore gr;

        cp.cpID = itemPtr->gr.cpID;

        gr.SelectByStage(cp, itemPtr->gr.grStage);
        while (gr.Next())
        {
          if (grIds.find(gr.grID) != grIds.end())
            continue;

          grIds.insert(gr.grID);
          ids.push_back(gr.grID);
        }

        gr.Close();

        break;
      }

      case MtListStore::Finished::Group:
      case MtListStore::Finished::Round:
      {
        long grID = itemPtr->gr.grID;

        if (grIds.find(grID) != grIds.end())
          continue;

        grIds.insert(grID);
        ids.push_back(grID);

        break;
      }
    }
  }

  idItems = new long[ids.size()];
  for (auto it : ids)
    idItems[nofItems++] = it;

  bool useThread = true;
  Printer *printer = NULL;
  bool showDlg = !wxGetKeyState(WXK_SHIFT);

  if (CTT32App::instance()->GetPrintPreview())
  {
    printer = new PrinterPreview(wxString::Format("%s", _("Finished Groups")), showDlg);
  }
  else if (CTT32App::instance()->GetPrintPdf())
  {
    wxString fname;

    wxDirDialog dirDlg(this, wxDirSelectorPromptStr, CTT32App::instance()->GetPath());
    if (dirDlg.ShowModal() != wxID_OK)
      return;

    fname = dirDlg.GetPath();

    printer = new PrinterPdf(fname, showDlg);
  }
  else
  {
    printer = new Printer(showDlg);
  }

  if (printer->PrinterAborted())
  {
    delete printer;
    return;
  }

  PrintThreadStruct *tmp = new PrintThreadStruct();
  tmp->nofItems = nofItems;
  tmp->idItems = idItems;
  tmp->m_cp = CpRec();
  tmp->m_po = m_po;
  tmp->printer = printer;
  tmp->isPreview = CTT32App::instance()->GetPrintPreview();
  tmp->isPdf = CTT32App::instance()->GetPrintPdf();

  if (useThread)
    tmp->connPtr = TTDbse::instance()->GetNewConnection();
  else
    tmp->connPtr = TTDbse::instance()->GetDefaultConnection();

  tmp->isThread = useThread;
  tmp->isMultiple = true;

  wxString str;
  str = wxString::Format(_("Print %d Result Form(s)"), nofItems);

  if (useThread)
    CTT32App::instance()->ProgressBarThread(CGrPrintBase::PrintThread, tmp, str, nofItems);
  else
    CGrPrintBase::PrintThread(tmp);
}


// -----------------------------------------------------------------------
void CGrFinished::OnTypeChange(wxCommandEvent &)
{
  wxString type = XRCCTRL(*this, "Type", wxComboBox)->GetValue();

  m_grList->Freeze();

  m_grList->RemoveAllListItems();

  MtListStore::Finished what = MtListStore::Finished::Event;

  if (type == "Event")
    what = MtListStore::Finished::Event;
  else if (type == "Stage")
    what = MtListStore::Finished::Stage;
  else if (type == "Group")
    what = MtListStore::Finished::Group;
  else if (type == "Round")
    what = MtListStore::Finished::Round;

  std::list <std::tuple<long, short, timestamp>> list = MtListStore().GetFinishedRounds(what);

  std::set<long> grIDs;
  std::set<long> cpIDs;

  std::map<long, GrRec> grList;
  std::map<long, CpRec> cpList;

  for (const std::tuple<long, short, timestamp> &tuple : list)
    grIDs.insert(std::get<0>(tuple));

  GrListStore gr;
  gr.SelectById(grIDs);
  while (gr.Next())
  {
    cpIDs.insert(gr.cpID);
    grList.insert({ gr.grID, gr });
  }
  gr.Close();

  CpListStore cp;
  cp.SelectById(cpIDs);
  while (cp.Next())
  {
    cpList.insert({ cp.cpID, cp });
  }
  cp.Close();

  std::vector<GrFinishedItem *> itemList;

  for (std::tuple<long, short, timestamp> &tuple : list)
  {
    long grID = std::get<0>(tuple);
    GrRec gr = grList[grID];
    CpRec cp = cpList[gr.cpID];

    itemList.push_back(new GrFinishedItem(cp.cpName, gr, std::get<1>(tuple), std::get<2>(tuple), what));
  }

  std::sort(itemList.begin(), itemList.end(),
            [](const GrFinishedItem *it1, const GrFinishedItem *it2)
            {
              if (it1->mtDateTime != it2->mtDateTime)
                return it1->mtDateTime > it2->mtDateTime;

              if (it1->mtRound != it2->mtRound)
                return it1->mtRound < it2->mtRound;

              if (wxString(it1->gr.grName) != wxString(it2->gr.grName))
                return wxString(it1->gr.grName) < wxString(it2->gr.grName);

              if (it1->gr.grSortOrder != it2->gr.grSortOrder)
                return it1->gr.grSortOrder < it2->gr.grSortOrder;

              return it1->cpName < it2->cpName;
            }
  );

  for (GrFinishedItem *itemPtr : itemList)
    m_grList->AddListItem(itemPtr);

  itemList.clear();

  m_grList->Thaw();

  // Recolor entries if only part of group is printed
  OnOptionChange(wxCommandEvent());
}


// -----------------------------------------------------------------------
void CGrFinished::OnRefresh()
{
  OnTypeChange(wxCommandEvent());
}