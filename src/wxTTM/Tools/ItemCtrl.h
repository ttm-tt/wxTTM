/* Copyright (C) 2020 Christoph Theis */

#ifndef _ITEMCTRL_H_
#define _ITEMCTRL_H_

class ListItem;


class CItemCtrl : public wxControl
{
  public:
    CItemCtrl();
   ~CItemCtrl();
   
  public:
    void SetListItem(ListItem *itemPtr) {m_itemPtr = itemPtr;}
    ListItem * GetListItem() {return m_itemPtr;}
    
    void SetItemHeight(int height);
    
  private:
    void OnPaint(wxPaintEvent &);
    void OnSize(wxSizeEvent &);
    
  private:
    ListItem *m_itemPtr;
    
  DECLARE_DYNAMIC_CLASS(CItemCtrl)
  DECLARE_EVENT_TABLE()     
};


// =======================================================================
class CItemCtrlXmlResourceHandler : public wxXmlResourceHandler
{
  public:
    virtual wxObject *DoCreateResource();
    virtual bool CanHandle(wxXmlNode *node);

  DECLARE_DYNAMIC_CLASS(CItemCtrlXmlResourceHandler)
};

#endif