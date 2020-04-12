/* Copyright (C) 2020 Christoph Theis */

// Anzeige eines Spielstatus in der Spieluebersicht

#ifndef  OVITEM_H
#define  OVITEM_H

#include  "ListItem.h"
#include  "MtListStore.h"
#include  "TmEntryStore.h"
#include  "MtEntryStore.h"

#include  <string>

class  Printer;

class  OvItem : public ListItem
{
  public:
    enum  OvType
    {
      OVERROR = 0,
      OVUNKNOWN,
      OVNOTPRINTED,
      OVPRINTED,
      OVFINISHED,
      OVCHECKED,
      OVLAST
    };
    
  public:
  
    static void SetFgColor(OvType type, const wxColor &color) 
    {
      if (type < OVLAST)
        colors[type][0] = color;
    }

    static wxColor GetFgColor(OvType type)
    {
      if (type >= OVLAST)        
        return wxColor(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
      else if (colors[type][1] == wxNullColour)
        return defaultColors[type][1];
      else
        return colors[type][1];
    }


    static wxColor GetDefaultFgColor(OvType type) 
    {
      if (type < OVLAST)
        return defaultColors[type][1];

      return wxNullColour;
    }

    static void SetBkColor(OvType type, const wxColor &color)
    {
      if (type < OVLAST)
        colors[type][0] = color;
    }
    
    static wxColor GetBkColor(OvType type)
    {
      if (type >= OVLAST)
        return wxColor(wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE));
      else if (colors[type][0] == wxNullColour)
        return defaultColors[type][0];
      else
        return colors[type][0];
    }

    static wxColor GetDefaultBkColor(OvType type)
    {
      if (type < OVLAST)
        return defaultColors[type][0];

      return wxNullColour;
    }

    static wxString OvTypeToName(OvType type)
    {
      switch (type)
      {
        case OVERROR : 
          return "Error";

        case OVUNKNOWN :
          return "Incomplete";

        case OVNOTPRINTED :
          return "NotPrinted";

        case OVPRINTED :
          return "Printed";

        case OVFINISHED :
          return "Finished";

        case OVCHECKED :
          return "Checked";

        case OVLAST :
          return "";
      }

      return "";
    }

    static OvType NameToOvType(const wxString name)
    {
      for (int i = 0; i < OVLAST; i++)
        if (OvTypeToName( OvType(i) ) == name)
          return (OvType) i;

      return OVLAST;
    }

  public:
    OvItem(const wxString & label, OvType  type);   // Fuer die Beispiele
    OvItem(const MtListRec &mt);               // Ein Spiel
    OvItem(const MtEntry &mt);
    OvItem(const MtListRec &mt, OvType type);  // Farbe explzit setzen
    OvItem(const MtEntry &mt, OvType type);

    void  SetValue(const MtListRec &mt);
    void  SetValue(const MtEntry &mt);
    void  SetValue(const wxString & val) {label = val;}
    void  SetShowUmpire(bool * b);
    bool  GetShowUmpire() const {return *showUmpires;}

    void  SetToolTipText(const wxString &text) {ttText = text;}
    const wxString & GetToolTipText() const {return ttText;}

  public:
    // Text in einer ComboBox ausgeben
    virtual void DrawItem(wxDC *pDC, wxRect &rect);
    virtual void PrintItem(Printer *printer, wxRect &rect);

    // Vordergrund- / Hintergrundfarbe ermitteln
    wxColor  GetFgColor() const;
    wxColor  GetBkColor() const;
    
    OvType    GetOvType() const {return ovType;}
    void      SetOvType(OvType type) {ovType = type;}

    // Tooltips
    virtual wxSize GetToolTipSize(wxDC *pDC);
    virtual void DrawToolTip(wxDC *pDC, wxRect &rect);

  private:
    static  wxColor  defaultColors[][2];
    static  wxColor  colors[][2];

  private:
    int  GetTextWidth(wxDC *pDC, const TmEntry &tm);

  private:
    OvType  ovType;
    wxString  label;  // Fuer fixe Texte
    wxString ttText;
    bool * showUmpires;  // Referenz, um schneller umzuschalten

  // Unschoen, aber ich brauch den Zugriff drauf
  public:
    MtListRec mt;
    TmEntry   tmA;
    TmEntry   tmX;
};


#endif