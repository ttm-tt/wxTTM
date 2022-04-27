/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"

#include "TT32App.h"
#include "GrOptionsList.h"

#include "GrOptions.h"

#include "ListItem.h"

#include "PoStore.h"

#include "InfoSystem.h"

#include "checked.xpm"
#include "unchecked.xpm"

#include <list>

// -----------------------------------------------------------------------
IMPLEMENT_DYNAMIC_CLASS(CGrOptionsList, CFormViewEx)

BEGIN_EVENT_TABLE(CGrOptionsList, CFormViewEx)
  EVT_BUTTON(XRCID("Publish"), CGrOptionsList::OnPublish)
END_EVENT_TABLE()


class OptionItem : public ListItem
{
  public:
    OptionItem(const wxString &name, long id = 0) : ListItem(id, name) {}

    virtual void DrawColumn(wxDC *pDC, int col, wxRect &rec);

};


void OptionItem::DrawColumn(wxDC *pDC, int col, wxRect &rect)
{
  static wxImage checkedImg(checked);
  static wxImage uncheckedImg(unchecked);

  switch (col)
  {
    case 0 :
      DrawImage(pDC, rect, m_id ? checkedImg : uncheckedImg);
      break;

    case 1 :
      DrawString(pDC, rect, m_label);
      break;
  }
}


// -----------------------------------------------------------------------
CGrOptionsList::CGrOptionsList() : CFormViewEx(), m_listCtrl(0)
{
}

CGrOptionsList::~CGrOptionsList()
{
}


// -----------------------------------------------------------------------
bool CGrOptionsList::Edit(va_list vaList)
{
  std::list<wxString> options = PoStore().List();

  for (wxString option : options)
  {
    if (option.IsEmpty())
      continue;

    m_listCtrl->AddListItem(new OptionItem(option, PoStore().NameToID(option)));
  }

  return true;
}


// -----------------------------------------------------------------------
void CGrOptionsList::OnAdd()
{
  int ret = 0;
  PoStore po;

  // Variable dlg kapseln
  {
    CGrOptions *dlg = (CGrOptions *)wxXmlResource::Get()->LoadDialog(this, "GrOptions");
    dlg->SetPrintRasterOptions(&po);

    ret = dlg->ShowModal();
  }

  if (ret == wxID_CANCEL)
    return;

  while (true)
  {
    wxTextEntryDialog dlg(this, _("Name"), _("Enter Print Option Name"));
    dlg.SetMaxLength(sizeof(po.poName) / sizeof(wxChar) - 1);
    if (dlg.ShowModal() != wxID_OK)
      return;

    wxString name = dlg.GetValue();

    // TODO: Warnung, wenn Name in Gebrauch ist
    if (po.Exists(name))
    {
      if (!infoSystem.Question(_("Print option %s already exists. Overwrite?"), name.c_str()))
        continue;
    }

    wxStrncpy(po.poName, name, sizeof(po.poName) / sizeof(wxChar));

    break;
  }
  
  // Einstellungen ablegen
  po.Write(po.poName);

  if ( !m_listCtrl->FindListItem(po.poName) )
    m_listCtrl->InsertListItem(new OptionItem(po.poName));
}


// -----------------------------------------------------------------------
void  CGrOptionsList::OnEdit()
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  int ret = 0;
  PoStore po;
  po.Read(itemPtr->GetLabel());

  // Variable dlg kapseln
  {
    CGrOptions *dlg = (CGrOptions *)wxXmlResource::Get()->LoadDialog(this, "GrOptions");
    dlg->SetPrintRasterOptions(&po);

    ret = dlg->ShowModal();
  }

  if (ret == wxID_CANCEL)
    return;

  if (ret != wxID_OK)
  {
    while (true)
    {
      wxTextEntryDialog dlg(this, _("Name"), _("Enter Print Option Name"));
      dlg.SetMaxLength(sizeof(po.poName) / sizeof(wxChar) - 1);
      if (dlg.ShowModal() != wxID_OK)
        return;

      wxString name = dlg.GetValue();

      // TODO: Warnung, wenn Name in Gebrauch ist
      if (po.Exists(name))
      {
        if (!infoSystem.Question(_("Print option %s already exists. Overwrite?"), name.c_str()))
          continue;
      }

      wxStrncpy(po.poName, name, sizeof(po.poName) / sizeof(wxChar));

      break;
    }
  }

  // Einstellungen ablegen
  po.Write(po.poName);

  if (!m_listCtrl->FindListItem(po.poName))
    m_listCtrl->InsertListItem(new OptionItem(po.poName));
}


void  CGrOptionsList::OnDelete()
{
  if (m_listCtrl->GetSelectedItemCount() > 0)
  {
    if (infoSystem.Confirmation(_("Are you sure to delete selected print options?")) == false)
      return;

    for (int idx = m_listCtrl->GetItemCount(); idx--; )
    {
      if (m_listCtrl->IsSelected(idx))
      {
        OptionItem *itemPtr = (OptionItem *) m_listCtrl->CutListItem(idx);

        if (!itemPtr)
          continue;

        PoStore po;
        if (!po.Delete(itemPtr->GetLabel()))
          m_listCtrl->InsertListItem(itemPtr);
        else
          delete itemPtr;
      }
    }
  }
}


// -----------------------------------------------------------------------
void CGrOptionsList::OnInitialUpdate()
{
  m_listCtrl = XRCCTRL(*this, "Options", CListCtrlEx);

  m_listCtrl->InsertColumn(0, _("Pbl"), wxALIGN_LEFT, 3 * cW);
  m_listCtrl->InsertColumn(1, _("Name"));

  m_listCtrl->SetResizeColumn(1);
}


// -----------------------------------------------------------------------
void CGrOptionsList::OnPublish(wxCommandEvent &)
{
  ListItem *itemPtr = m_listCtrl->GetCurrentItem();
  if (!itemPtr)
    return;

  if (itemPtr->GetID())
    return;

  wxString name = itemPtr->GetLabel();
  if (!PoStore().Publish(name))
    return;

  delete m_listCtrl->CutListItem(m_listCtrl->GetCurrentIndex());
  m_listCtrl->InsertListItem(new OptionItem(name, PoStore().NameToID(name)));
}