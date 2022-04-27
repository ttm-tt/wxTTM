/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "SQLEditorBook.h"

#include "SQLEditor.h"


IMPLEMENT_DYNAMIC_CLASS(CSQLEditorBook, CFormViewEx)


BEGIN_EVENT_TABLE(CSQLEditorBook, CFormViewEx)
  EVT_CHAR_HOOK(CSQLEditorBook::OnCharHook)
  EVT_AUINOTEBOOK_PAGE_CHANGING(XRCID("Notebook"), CSQLEditorBook::OnPageChanging)
  EVT_AUINOTEBOOK_PAGE_CLOSE(XRCID("Notebook"), CSQLEditorBook::OnPageClose)
  EVT_AUINOTEBOOK_TAB_RIGHT_UP(XRCID("Notebook"), CSQLEditorBook::OnTabRightUp)
END_EVENT_TABLE()


CSQLEditorBook::CSQLEditorBook() : CFormViewEx(), m_inDrag(false), m_notebook(NULL)
{
}


CSQLEditorBook::~CSQLEditorBook()
{
}


// -----------------------------------------------------------------------
bool CSQLEditorBook::Edit(va_list)
{
  return true;
}


// -----------------------------------------------------------------------
void CSQLEditorBook::OnInitialUpdate()
{
  // OnPageChange wird vor OnInitialUpdate (waehrend dem Laden aus XRC) aufgerufen
  if (!m_notebook)
    m_notebook = XRCCTRL(*this, "Notebook", wxAuiNotebook);

  m_notebook->SetArtProvider(new wxAuiGenericTabArt());

  Connect(IDC_SET_TITLE, wxCommandEventHandler(CSQLEditorBook::OnSetTitle));
  Connect(IDC_NB_PAGE_DRAGGED, wxCommandEventHandler(CSQLEditorBook::OnPageDragged));
  m_notebook->Connect(wxEVT_AUINOTEBOOK_END_DRAG, wxAuiNotebookEventHandler(CSQLEditorBook::OnDragEnd));
}


// -----------------------------------------------------------------------
void CSQLEditorBook::OnPageChanging(wxAuiNotebookEvent &evt)
{
  if (m_inDrag)
  {
    evt.Skip();
    return;
  }

  // OnPageChange wird vor OnInitialUpdate (waehrend dem Laden aus XRC) aufgerufen
  if (!m_notebook)
    m_notebook = XRCCTRL(*this, "Notebook", wxAuiNotebook);

  if (evt.GetSelection() == m_notebook->GetPageCount() - 1)
  {
    if (m_notebook->GetPageText(evt.GetSelection()).IsEmpty())
    {
      evt.Veto();
      AddNewPage();
      return;
    }
  }

  evt.Skip();
}


void CSQLEditorBook::OnPageClose(wxAuiNotebookEvent &evt)
{
  if (evt.GetSelection() == m_notebook->GetPageCount())
    evt.Veto();
  else if (m_notebook->GetPageCount() > 2 && evt.GetSelection() == m_notebook->GetPageCount() - 2)
    m_notebook->SetSelection(evt.GetSelection() - 1);
}


void CSQLEditorBook::OnDragEnd(wxAuiNotebookEvent &evt)
{
  // Nachdem wxWidget alles fertig gestellt hat, dafuer sorgen,
  // dass die "+"-Seite (wieder) am Ende ist.
  QueueEvent(new wxCommandEvent(IDC_NB_PAGE_DRAGGED));
}


// Eigenes Event, das verarbeitet wird, nachdem Tab-drag abgeschlossen ist
void CSQLEditorBook::OnPageDragged(wxCommandEvent &evt)
{
  int lastIdx = m_notebook->GetPageCount() - 1;
  if ( !m_notebook->GetPageText(lastIdx).IsEmpty() )
  {
    m_inDrag = true;

    wxString label = m_notebook->GetPageText(lastIdx);
    wxWindow *page = m_notebook->GetPage(lastIdx);
    if (m_notebook->RemovePage(lastIdx))
      m_notebook->InsertPage(lastIdx - 1, page, label, true);

    m_inDrag = false;
  }
}


void CSQLEditorBook::OnSetTitle(wxCommandEvent &evt)
{
  int idx = m_notebook->GetPageIndex( (wxWindow *) evt.GetEventObject() );
  m_notebook->SetPageText(idx, evt.GetString());
}


void CSQLEditorBook::OnCharHook(wxKeyEvent &evt)
{
  switch (evt.GetKeyCode())
  {
    case 'T' :
      if (evt.GetModifiers() == wxMOD_CONTROL)
        AddNewPage();
      else
        evt.Skip();

      break;

    case 'W' :
      if (evt.GetModifiers() == wxMOD_CONTROL)
      {
        // Das vorige Item selektieren, sonst wandert die Auswahl auf "+"
        size_t idx = m_notebook->GetSelection();
        if (idx > 0)
          m_notebook->SetSelection(idx - 1);
        m_notebook->DeletePage(idx);
      }
      else
        evt.Skip();

      break;

    case WXK_RIGHT :
    case WXK_PAGEDOWN :
      if (evt.GetModifiers() == wxMOD_ALT)
      {
        size_t idx = m_notebook->GetSelection();
        if (m_notebook->GetPageCount() < 3)
          ;
        else if (idx >= m_notebook->GetPageCount() - 2)
          idx = 0;
        else
          ++idx;

        m_notebook->SetSelection(idx);
      }
      else
        evt.Skip();

      break;

    case WXK_LEFT :
    case WXK_PAGEUP :
      if (evt.GetModifiers() == wxMOD_ALT)
      {
        size_t idx = m_notebook->GetSelection();
        if (m_notebook->GetPageCount() < 3)
          ;
        else if (idx == 0)
          idx = m_notebook->GetPageCount() - 2;
        else
          --idx;

        m_notebook->SetSelection(idx);
      }
      else
        evt.Skip();

      break;

    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :
      if (evt.GetModifiers() == wxMOD_ALT)
      {
        size_t idx = evt.GetKeyCode() - '0' - 1;
        
        if (idx < m_notebook->GetPageCount() - 1)
          m_notebook->SetSelection(idx);
      }
      else
        evt.Skip();

      break;

    default :
      evt.Skip();
  }
}


// -----------------------------------------------------------------------


void CSQLEditorBook::OnRefresh()
{
  // FIXME: Send Event anstatt Funktion aufrufen.
  // Allerdings verarbeitet SQLEditor dann nicht das Event
  // m_notebook->GetPage(m_notebook->GetSelection())->GetEventHandler()->ProcessEvent(wxCommandEvent(IDC_REFRESH));

  wxCommandEvent cmdRefresh(IDC_REFRESH);

  if ( m_notebook->GetPage(m_notebook->GetSelection()) )
    m_notebook->GetPage(m_notebook->GetSelection())->GetEventHandler()->ProcessEvent(cmdRefresh);
    // ((COvList *) m_notebook->GetPage(m_notebook->GetSelection()))->OnRefresh();  
}


void CSQLEditorBook::OnPrint()
{
  // FIXME: Send Event anstatt Funktion aufrufen.
  // Allerdings verarbeitet SQLEditor dann nicht das Event
  // m_notebook->GetPage(m_notebook->GetSelection())->GetEventHandler()->ProcessEvent(wxCommandEvent(wxID_PRINT));

  wxCommandEvent cmdPrint(wxID_PRINT);

  if ( m_notebook->GetPage(m_notebook->GetSelection()) )
    m_notebook->GetPage(m_notebook->GetSelection())->GetEventHandler()->ProcessEvent(cmdPrint);
    // ((COvList *) m_notebook->GetPage(m_notebook->GetSelection()))->OnPrint();  
}

// -----------------------------------------------------------------------
void CSQLEditorBook::AddNewPage()
{
  wxPanel *panel = wxXmlResource::Get()->LoadPanel(this, "SQLEditor");
  m_notebook->InsertPage(m_notebook->GetPageCount() - 1, panel, _("SQL Editor"), false);

  wxInitDialogEvent wxInitDialogEvent_;
  panel->GetEventHandler()->ProcessEvent(wxInitDialogEvent_);
  ((CSQLEditor *) panel)->Edit(0);

  m_notebook->SetSelection(m_notebook->GetPageIndex(panel));
  panel->SetFocus();
}


// -----------------------------------------------------------------------
void CSQLEditorBook::OnTabRightUp(wxAuiNotebookEvent &evt)
{
  static wxString tabMenu[] = 
  {
    wxTRANSLATE("Close"),
    wxTRANSLATE("Close Others"),
    wxTRANSLATE("Rename")
  };

  wxMenu popupMenu("");  
  
  for (int i = 0; i < WXSIZEOF(tabMenu); i++)
    popupMenu.Append(2000 + i, wxGetTranslation(tabMenu[i]));

  wxPoint popupPos = ScreenToClient(wxGetMousePosition());
  
  int rc = GetPopupMenuSelectionFromUser(popupMenu, popupPos);
  if (rc == wxID_NONE)
    return;

  int page = evt.GetSelection();

  // Niemals auf "+" reagieren
  if (page == m_notebook->GetPageCount() - 1)
    return;

  switch (rc)
  {
    case 2000 : // Close
      m_notebook->DeletePage(page);
      break;

    case 2001 : // Close others
    {
      m_notebook->SetSelection(page);

      for (int idx = m_notebook->GetPageCount() - 1; idx--; )
      {
        if (idx != page)
          m_notebook->DeletePage(idx);
      }
    }

    break;

    case 2002 : // Rename
    {
      wxTextEntryDialog dlg(this, _("New title"), _("New Title"), m_notebook->GetPageText(page));
      if (dlg.ShowModal() != wxID_OK)
        return;

      if (dlg.GetValue().IsEmpty())
      {
        // Default Titel
        m_notebook->SetPageText(page, _("SQL Editor"));
        ((CSQLEditor *) m_notebook->GetPage(page))->FreezeTitle(false);
      }
      else
      {
        m_notebook->SetPageText(page, dlg.GetValue());
        ((CSQLEditor *) m_notebook->GetPage(page))->FreezeTitle(true);
      }

      break;
    }
  }
}

