/* Copyright (C) 2020 Christoph Theis */

// GrEdit.cpp : implementation file
//

#include "stdafx.h"
#include "TT32App.h"
#include "GrEdit.h"

#include "Rec.h"

#include "MdListStore.h"
#include "SyListStore.h"
#include "TmEntryStore.h"
#include "RkListStore.h"
#include "StListStore.h"
#include "GrListStore.h"
#include "MpListStore.h"

#include "GrTemplate.h"

#include "CpItem.h"
#include "MdItem.h"
#include "SyItem.h"
#include "MpItem.h"


IMPLEMENT_DYNAMIC_CLASS(CGrEdit, CFormViewEx)

BEGIN_EVENT_TABLE(CGrEdit, CFormViewEx)
  EVT_COMBOBOX(XRCID("Stage"), CGrEdit::OnSelChangeStage)
  EVT_COMBOBOX(XRCID("GroupSystem"), CGrEdit::OnSelChangeMd)
  EVT_BUTTON(XRCID("Template"), CGrEdit::OnTemplate)
END_EVENT_TABLE()


// -----------------------------------------------------------------------
// CGrEdit
CGrEdit::CGrEdit() : CFormViewEx()
{
}


CGrEdit::~CGrEdit()
{
}


bool CGrEdit::Edit(va_list vaList)
{
  long grID = va_arg(vaList, long);
  long cpID = va_arg(vaList, long);

  gr.SelectById(grID);
  if (!gr.Next())
  {
    // Wenn es nicht die Gruppe war, dann der WB
    gr.cpID = cpID;
    cp.SelectById(cpID);
    cp.Next();

    // gr init.
    gr.grSize = 2;
    gr.grWinner = 1;
    gr.grBestOf = cp.cpBestOf ? cp.cpBestOf : 5;
    gr.syID = cp.syID;
  }

  cp.SelectById(gr.cpID);
  cp.Next();

  m_cpItem->SetListItem(new CpItem(cp));

  MdItem *rrItem = 0, *tmpItem = 0;
  MdItem *skoItem = 0, *ploItem = 0;
  
  if (gr.grID == 0 || gr.grModus != MOD_RR)
  {
    skoItem = new MdItem(MOD_SKO);
    ploItem = new MdItem(MOD_PLO);
    m_cbModus->AddListItem(skoItem);
    m_cbModus->AddListItem(ploItem);
  }

  MdListStore  md;
  md.SelectAll();
  while (md.Next())
  {
    if (gr.grID && gr.grModus == MOD_RR && md.mdSize != gr.grSize)
      continue;
      
    MdItem *itemPtr = new MdItem(md);
    m_cbModus->AddListItem(itemPtr);

    // Gruppengroesse 4 ist was besonderes fuer Oldies
    if (!rrItem || md.mdID == gr.mdID || !gr.mdID && md.mdSize == 4)
      rrItem = itemPtr;
      
    // Und erstes Item merken
    if (!tmpItem)
      tmpItem = itemPtr;
  }
  
  if (!rrItem)
    rrItem = tmpItem;

  // Qualification / Championship / Consolation auswaehlen (und Groesse bestimmen)
  short resQU = GrStore().CountGroups(cp, "Qualification");
  short resCP = GrStore().CountGroups(cp, "Championship");
  short resCO = GrStore().CountGroups(cp, "Consolation");

  if (gr.grID != 0)
    ;
  else if (resQU == 0)
  {
    // Qualification auswaehlen
    wxStrcpy(gr.grStage, "Qualification");
    if (rrItem)
      gr.grModus = MOD_RR;
    else
    {
      gr.grModus = MOD_SKO;
      gr.grSize  = 8;
    }
  }
  else if (resCP == 0)
  {
    // Championship auswaehlen
    wxStrcpy(gr.grStage, "Championship");
    gr.grModus = cp.cpType == CP_TEAM ? MOD_PLO : MOD_SKO;
    short count = 2 * resQU + RkListStore().CountDirectEntries(cp, NaRec());

    for (gr.grSize = 2; gr.grSize < count; gr.grSize *= 2)
      ;
  }
  else if (resCO == 0)
  {
    // Consolation auswaehlen
    wxStrcpy(gr.grStage, "Consolation");
    gr.grModus = cp.cpType == CP_TEAM ? MOD_PLO : MOD_SKO;

    short count = RkListStore().CountQualifiers(cp, NaRec()) - 2 * resQU;
    count -= StListStore().CountNoCons(cp, resQU ? "Qualification" : NULL);

    for (gr.grSize = 2; gr.grSize < count; gr.grSize *= 2)
      ;
  }
  else
  {
    gr.grModus = MOD_SKO;
  }

  if (gr.grModus == MOD_RR)
  {
    m_cbModus->SetCurrentItem(rrItem);
    gr.grSize = rrItem->md.mdSize;
  }
  else if (gr.grModus == MOD_SKO)
    m_cbModus->SetCurrentItem(skoItem);
  else if (gr.grModus == MOD_PLO)
    m_cbModus->SetCurrentItem(ploItem);
  else if (gr.grModus == MOD_DKO)
  {
    MdItem *dkoItem = new MdItem(MOD_DKO);
    m_cbModus->AddListItem(dkoItem);
    
    m_cbModus->SetCurrentItem(dkoItem);
  }
  else if (gr.grModus == MOD_MDK)
  {
    MdItem *mdkItem = new MdItem(MOD_MDK);
    m_cbModus->AddListItem(mdkItem);

    m_cbModus->SetCurrentItem(mdkItem);
  }
  else
    m_cbModus->SetCurrentItem(skoItem);

  // Keine Aenderung, wenn Gruppe bereits spielt oder nicht MOD_RR
  m_cbModus->Enable(gr.grID == 0 || gr.grModus == MOD_RR && !gr.QryStarted());

  // System ist nur sichtbar bei Mannschaftswb.
  if (cp.cpType != CP_TEAM)
  {
    // TODO: Ctrl verstecken, nicht nur ein disable
    // m_cbSystem->ShowWindow(SW_HIDE);
    // GetDlgItem(IDC_GREDIT_SYTEXT)->ShowWindow(SW_HIDE);
    m_cbSystem->Enable(FALSE);
  }
  else
  {
    time_t ct = time(NULL);
    struct tm *tm = localtime(&ct);
    
    SyListStore  sy;
    sy.SelectAll();
    while (sy.Next())
      m_cbSystem->AddListItem(new SyItem(sy));

    if (gr.syID)
      m_cbSystem->SetCurrentItem(gr.syID);
    else if ((tm->tm_year + 1900 - cp.cpYear) <= 15 && m_cbSystem->FindListItem("COR"))
      m_cbSystem->SetCurrentItem("COR");
    else if (m_cbSystem->FindListItem("MCC"))
      m_cbSystem->SetCurrentItem("MCC");
    else if (m_cbSystem->GetListItem((int) 0))
      m_cbSystem->SetCurrentItem(m_cbSystem->GetListItem(0));

    m_cbSystem->Enable(!gr.QryStarted());
  }

  bool grStarted = gr.grID != 0 && gr.QryStarted();

  //Gruppengroesse darf auch nicht geaendert werden
  FindWindow("NofEntries")->Enable( (gr.grModus == MOD_SKO || gr.grModus == MOD_PLO) && !grStarted );

  FindWindow("QualRounds")->Enable(gr.grModus == MOD_SKO);
  FindWindow("NofRounds")->Enable( (gr.grModus == MOD_SKO || gr.grModus == MOD_PLO) && !grStarted );

  FindWindow("NoThirdPlace")->Enable(gr.grModus == MOD_PLO);
  FindWindow("OnlyThirdPlace")->Enable(gr.grModus == MOD_PLO);

  // Ebensoweinig BestOf
  // FindWindow("BestOf")->Enable(!grStarted);

  // Kein Template, wenn die Gruppe existiert
  // GetDlgItem(IDC_GREDIT_TEMPLATE)->EnableWindow(gr.grID == 0);

  if (gr.grNofMatches)
    nofEntries = 2 * gr.grNofMatches;
  else
    nofEntries = gr.grSize;

  TransferDataToWindow();
  
  return true;
}


//------------------------------------------------------------------------
// CGrEdit message handlers
void CGrEdit::OnInitialUpdate() 
{
  CFormViewEx::OnInitialUpdate();

  m_cpItem = XRCCTRL(*this, "Event", CItemCtrl);
  m_cbStage = XRCCTRL(*this, "Stage", wxComboBox);
  m_cbModus = XRCCTRL(*this, "GroupSystem", CComboBoxEx);
  m_cbSystem = XRCCTRL(*this, "TeamSystem", CComboBoxEx);	  
  
  FindWindow("Stage")->SetValidator(CCharArrayValidator(gr.grStage, sizeof(gr.grStage) / sizeof(wxChar)));  

  std::list<wxString> stages = GrListStore().ListStages(CpRec());
  for (auto it = stages.cbegin(); it != stages.cend(); it++)
  {
    if (*it == "Qualification")
      continue;
    if (*it == "Championship")
      continue;
    if (*it == "Consolation")
      continue;

    m_cbStage->AppendString(*it);
  }

  FindWindow("Name")->SetValidator(CCharArrayValidator(gr.grName, sizeof(gr.grName) / sizeof(wxChar)));
  FindWindow("Description")->SetValidator(CCharArrayValidator(gr.grDesc, sizeof(gr.grDesc) / sizeof(wxChar)));
  FindWindow("NofEntries")->SetValidator(CShortValidator(&nofEntries));
  FindWindow("Winner")->SetValidator(CShortValidator(&gr.grWinner));
  FindWindow("BestOf")->SetValidator(CShortValidator(&gr.grBestOf));
  FindWindow("QualRounds")->SetValidator(CShortValidator(&gr.grQualRounds));
  FindWindow("NofRounds")->SetValidator(CShortValidator(&gr.grNofRounds));
  FindWindow("NoThirdPlace")->SetValidator(CEnumValidator(&gr.grNoThirdPlace, 1));
  FindWindow("OnlyThirdPlace")->SetValidator(CEnumValidator(&gr.grOnlyThirdPlace, 1));
  FindWindow("SortOrder")->SetValidator(CShortValidator(&gr.grSortOrder));

  FindWindow("Stage")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CGrEdit::OnKillFocusStage), NULL, this);
  FindWindow("NofEntries")->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(CGrEdit::OnKillFocusSize), NULL, this);
}


void CGrEdit::OnSelChangeStage(wxCommandEvent &)
{
  TransferDataFromWindow();

  if (wxStrcmp(gr.grStage, wxT("Qualification")) == 0)
  {
    if (gr.grModus != MOD_RR)
    {
      if (gr.mdID != 0)
        m_cbModus->SetCurrentItem(gr.mdID);
      else if (m_cbModus->GetCount() > 3)
        m_cbModus->SetCurrentItem(m_cbModus->GetListItem(2));

      OnSelChangeMd(wxCommandEvent_);
    }
  }

  // Siehe OnSelChangeMd
  // TransferDataToWindow();
}


void CGrEdit::OnSelChangeMd(wxCommandEvent &evt) 
{
  TransferDataFromWindow();

  ListItem *itemPtr = m_cbModus->GetCurrentItem();
  wxASSERT(itemPtr);

  if ( ((MdItem *) itemPtr)->GetModus() != MOD_RR )
  {
    FindWindow("NofEntries")->Enable(gr.grID == 0);    
    
    int size = 0;
    
    if (wxStrcmp(gr.grStage, "Qualification") == 0)
    {
      size = TmEntryStore().CountTeams(cp);
    }
    else
    {
      size = 2 * GrStore().CountGroups(cp, "Qualification");
    }
    
    for (nofEntries = 2; nofEntries < size; nofEntries *= 2)
      ;
  }
  else
  {
    nofEntries = ((MdItem *) itemPtr)->md.mdSize;
    FindWindow("NofEntries")->Enable(false);
  }

  FindWindow("QualRounds")->Enable(((MdItem *) itemPtr)->GetModus() == MOD_SKO);
  FindWindow("NofRounds")->Enable(((MdItem *) itemPtr)->GetModus() == MOD_SKO || ((MdItem *) itemPtr)->GetModus() == MOD_PLO);

  FindWindow("NoThirdPlace")->Enable(((MdItem *) itemPtr)->GetModus() == MOD_PLO);
  FindWindow("OnlyThirdPlace")->Enable(((MdItem *) itemPtr)->GetModus() == MOD_PLO);

  // Ich kann TransferDataToWindow nicht aufrufen, weil die Funktion von OnSelChangeStage aufgerufen wird.
  // Und wenn ich dort TransferDataToWindow aufrufe wird die ComboBox "Stage" auf Leerstring gesetzt
  XRCCTRL(*this, "NofEntries", wxTextCtrl)->GetValidator()->TransferToWindow();
  // TransferDataToWindow();
}


void  CGrEdit::OnKillFocusSize(wxFocusEvent &evt)
{
  evt.Skip();

  wxTextCtrl *wndPtr = reinterpret_cast<wxTextCtrl *>(FindWindow(evt.GetId()));
  if (!wndPtr || !wndPtr->IsModified())
    return;

  int size = _ttoi(wndPtr->GetValue().t_str());

  int grModus = ((MdItem *) m_cbModus->GetCurrentItem())->GetModus();

  switch (grModus)
  {
    case MOD_DKO :
      {
        int i = 2;
        while (i < size)
          i *= 2;

        if (i != size)
        {
          wndPtr->SetValue(wxString::Format("%d", i));
          wxBell();
        }

        break;
      }

    case MOD_PLO :
    case MOD_SKO :
      {
        int i = (size + 1) / 2;
        i *= 2;

        if (gr.WasOK() && i > gr.grSize)
          i = gr.grSize;

        if (i != size)
        {
          wndPtr->SetValue(wxString::Format("%d", i));
          wxBell();
        }

        break;
      }
  }
}


void CGrEdit::OnKillFocusStage(wxFocusEvent &evt)
{
  evt.Skip();

  TransferDataFromWindow();

  CalculateNextSortOrder();

  TransferDataToWindow();
}


void CGrEdit::CalculateNextSortOrder()
{
  // Sort Order immer berechnen:
  // Macht man sonst einen Fehler in grStage wuerde sie nicht automatisch berechnet werden
  // Auf der anderen Seite kommt man bei Sort Order ohnehin spaeter vorbei und kann korrigieren
  gr.grSortOrder = gr.CalculateSortOrder();
}

// -----------------------------------------------------------------------


void  CGrEdit::OnOK()
{
  GrStore::CreateGroupStruct  *cgs = 0;
  
  TransferDataFromWindow();

  // System kann immer korrigiert werden, wenn es moeglich ist.
  // Wenn die Gruppe bereits spielt, ist die ComboBox gesperrt.
  if (cp.cpType == CP_TEAM)
  {
    ListItem *itemPtr = m_cbSystem->GetCurrentItem();
    if (itemPtr)
      gr.syID = itemPtr->GetID();
  }

  // Modus auswaehlen. Wenn die Gruppe bereits existiert, ist es der 
  // gleiche Modus (KO, PLO) oder ein RR-Modus gleicher Groesse
  ListItem *itemPtr;
  itemPtr = m_cbModus->GetCurrentItem();
  wxASSERT(itemPtr);
  gr.grModus = ((MdItem *) itemPtr)->GetModus();
  gr.mdID = ((MdItem *) itemPtr)->GetID();

  if (gr.grModus == MOD_RR)
  {
    gr.grSize = nofEntries;
    gr.grNofMatches = 0;
  }
  else 
  {
    // NofEntries kann nicht groesser werden als die bereits existierende Gruppe
    if (gr.WasOK() && nofEntries > gr.grSize)
      nofEntries = gr.grSize;

    // Fuer neue Gruppen die Groesse berechnen
    if (!gr.WasOK())
    {
      gr.grSize = 2;
      while (gr.grSize < nofEntries)
        gr.grSize *= 2;
    }
  
    // Gruppen == MOD_PLO haben nur dann eine Anzahl von Spielen, wenn die Anzahl von Runden == 1 ist
    // Gruppen != MOD_SKO ausser oben haben keine Anzahl von Spielen
    // Wenn die Anzahl von Spielen == Gruppengroesse ist, braucht man sie nicht zu speichern
    // Ansonsten ist es die Anzahl von Spielen, die man dafuer braucht
    if (gr.grModus == MOD_PLO && gr.grNofRounds == 1)
      /* Nothing */ ;
    else if (gr.grModus != MOD_SKO)
      gr.grNofMatches = 0;

    if (gr.grSize == nofEntries)
      gr.grNofMatches = 0;
    else
      gr.grNofMatches = (nofEntries + 1) / 2;
  }

  // Anzahl Runden korrigieren
  if (gr.grNofRounds >= gr.NofRounds())
    gr.grNofRounds = 0;

  if (gr.WasOK())
  {
    TTDbse::instance()->GetDefaultConnection()->StartTransaction();

    if (gr.Update())
      TTDbse::instance()->GetDefaultConnection()->Commit();
    else
      TTDbse::instance()->GetDefaultConnection()->Rollback();
 
    CFormViewEx::OnOK();
    return;
  }

  // Only Third Place sicher, weil davon abhaengt, wie viele Spiele es je Runde gibt
  int only3rdPlace = gr.grOnlyThirdPlace;
  gr.grOnlyThirdPlace = 0;

  int count = 1;  // GrStore
  
  count += gr.grSize;  // StStore
  
  for (int rd = gr.NofRounds(false); rd; rd--)
    count += gr.NofMatches(rd, false);
    
  for (int rd = gr.NofRounds(true); rd; rd--)
    count += gr.NofMatches(rd, true);

  gr.grOnlyThirdPlace = only3rdPlace;

  cgs = new GrStore::CreateGroupStruct;

  cgs->connPtr = TTDbse::instance()->GetNewConnection();
  cgs->cp = cp;
  cgs->gr = gr;
  cgs->count = 1;
  cgs->start = 1;
  
  if (wxStrstr(gr.grName, wxT("%c")))
    cgs->numeric = 2;
  else
    cgs->numeric = 1;

  cgs->nameTempl = cgs->gr.grName;
  cgs->descTempl = cgs->gr.grDesc;

  CTT32App::instance()->ProgressBarThread(GrStore::CreateGroup, cgs, "Create Group(s)", count);

  CFormViewEx::OnOK();
}



void  CGrEdit::OnTemplate(wxCommandEvent &)
{
  GrStore::CreateGroupStruct  *cgs = 0;

  TransferDataFromWindow();

  // Modus und System auswaehlen
  ListItem *itemPtr;

  if (cp.cpType == CP_TEAM)
  {
    itemPtr = m_cbSystem->GetCurrentItem();
    if (itemPtr)
      gr.syID = itemPtr->GetID();
  }

  itemPtr = m_cbModus->GetCurrentItem();
  wxASSERT(itemPtr);
  gr.grModus = ((MdItem *) itemPtr)->GetModus();
  gr.mdID = ((MdItem *) itemPtr)->GetID();

  if (gr.grModus == MOD_RR)
  {
    gr.grSize = nofEntries;
    gr.grNofMatches = 0;
  }
  else if (!gr.WasOK() || gr.grModus == MOD_SKO)
  {
    // NofEntries kann nicht groesser werden als die bereits existierende Gruppe
    if (gr.WasOK() && nofEntries > gr.grSize)
      nofEntries = gr.grSize;

    // Fuer neue Gruppen die Groesse berechnen
    if (!gr.WasOK())
    {
      gr.grSize = 2;
      while (gr.grSize < nofEntries)
        gr.grSize *= 2;
    }
  
    // Gruppen != MOD_SKO haben keine Anzahl von Spielen
    // Wenn die Anzahl von Spielen == Gruppengroesse ist, braucht man sie nicht zu speichern
    // Ansonsten ist es die Anzahl von Spielen, die man dafuer braucht
    if (gr.grModus != MOD_SKO)
      gr.grNofMatches = 0;
    else if (gr.grSize == nofEntries)
      gr.grNofMatches = 0;
    else
      gr.grNofMatches = (nofEntries + 1) / 2;
  }

  // Anzahl Runden korrigieren
  if (gr.grNofRounds >= gr.NofRounds())
    gr.grNofRounds = 0;

  cgs = new GrStore::CreateGroupStruct;

  cgs->connPtr = TTDbse::instance()->GetNewConnection();
  cgs->cp = cp;
  cgs->gr = gr;
  cgs->start = gr.CountGroups(cp, gr.grStage) + 1;
  cgs->delConnPtr = true;

  cgs->numeric = (wxStrstr(gr.grDesc, wxT("%c")) == NULL ? 1 : 2);  

  if (wxStrcmp(gr.grStage, wxT("Qualification")) == 0)
  {
    cgs->count = (RkListStore().CountQualifiers(cp, NaRec()) + gr.grSize - 1) / gr.grSize - cgs->start + 1;
  }
  else
  {
    cgs->count = 1;
  }

  CGrTemplate  grTemplate(cgs, this);
  if (grTemplate.ShowModal() == wxID_OK)
  {
    cgs->nameTempl = cgs->gr.grName;
    cgs->descTempl = cgs->gr.grDesc;

    wxString  tmp = "";

    if ( cgs->numeric == 2 )
      tmp = "%c";
    else if ( (cgs->start + cgs->count - 1) < 10 )
    {
      tmp = "%01i";
    }
    else if ( (cgs->start + cgs->count - 1) < 100 )
    {
      tmp = "%02i";
    }
    else
    {
      tmp = "%03i";
    }
    
    if (cgs->nameTempl.length() == 0)
    {    
      if (cgs->numeric == 2)
        cgs->nameTempl += "%c";
      else
        cgs->nameTempl += "%i";
    }
    else if (!strchr(cgs->nameTempl.data(), '%'))
    {
      cgs->nameTempl += tmp;
    }
    
    if (!strchr(cgs->descTempl.data(), '%'))
    {
      if (cgs->numeric == 2)
        cgs->descTempl += " %c";
      else
        cgs->descTempl += " %i";
    }

    // Fuer den Progressbar: Anzahl der Schritte
    int count = gr.grSize + 1;                // GrStore + StStore
    for (int rd = gr.NofRounds(true); rd; rd--)
      count += gr.NofMatches(rd);             // MtStore
    for (int rd = gr.NofRounds(false); rd; rd--)
      count += gr.NofMatches(rd);             // MtStore (Trostrunde)

    count *= cgs->count;

    if (gr.WasOK())
      CTT32App::instance()->ProgressBarThread(
          GrStore::UpdateGroup, cgs, "Update Group(s)", cgs->count);
    else      
      CTT32App::instance()->ProgressBarThread(
          GrStore::CreateGroup, cgs, "Create Group(s)", count);
  }
  else
  {
    delete cgs;
    return;
  }

  CFormViewEx::OnOK();
}
