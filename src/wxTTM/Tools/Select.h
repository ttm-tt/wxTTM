/* Copyright (C) 2020 Christoph Theis */

#ifndef _SELECT_H_
#define _SELECT_H_


#include  "ListCtrlEx.h"

class ListItem;


// -----------------------------------------------------------------------
// CSelect dialog
class CSelect : public wxDialog
{
  // Construction
  public:
    CSelect(const wxString &title = "");
   ~CSelect();
   
  protected:
	  void OnCommand(wxMouseEvent &);
	  void OnChar(wxKeyEvent &);

  public:
    ListItem *  Select(long id = 0);
    void  AddListItem(ListItem *, int idx = -1);
    void  InsertListItem(ListItem *);
    void  RemoveListItem(long id);
    ListItem *  CutListItem(long id);
    
    void SetSortColumn(int col) { listBox->SortItems(col); }

  protected:
    void  SaveSize();
    void  RestoreSize();

  private:
    CListCtrlEx *  listBox;
    ListItem *  itemPtr;
    
    int  origX;
    int  origY;
    int  origW;
    int  origH;
    
  DECLARE_DYNAMIC_CLASS(CSelect)
};


#endif 
