/* Copyright (C) 2020 Christoph Theis */

// Basisklasse fuer ListCtrl(Ex)

#ifndef  LISTITEM_H
#define  LISTITEM_H

#include "Rec.h"


struct  TmEntry;
struct  TmPlayer;
struct  TmTeam;
struct  TmGroup;

struct  PlRec;
struct  TmRec;



class  ListItem
{
  public:
    static int CompareFunction(const ListItem *item1, const ListItem *item2, int col);
    
  public:
    ListItem();
    ListItem(long id, const wxString & label = wxEmptyString, long type = IDC_EDIT);
    virtual ~ListItem();

    // Typ des Items fuer WM_CMD-msgs
    long  GetType() const {return m_type;}
    void  SetType(long type) {m_type = type;}

    // Die ID ist immer fix hier
    long  GetID() const   {return m_id;}

    // Der Label nicht
    const wxString & GetLabel() const {return m_label;}    
    
    void SetLabel(const wxString &str) {m_label = str;}

    // Textfarbe aendern    
    void  SetForeground(wxColor c) {m_foreColor = c;}

    // If item is deleted, depends on overloaded class
    virtual bool  IsDeleted() const { return false; }

    wxColor GetForeground() {return m_foreColor;}
    
    // Hoehe des Items bestimmen
    virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMIS);

    // Vergleich zweiter Items nach Spalte. Per Default nach ID
    // Return -1, wenn this kleiner ist, +1 wenn groesser, 0 wenn gleich
    virtual int  Compare(const ListItem *itemPtr, int col) const;

    // Text einer einzelnen Spalte. nr == 0 ist das Item Label
    virtual void DrawColumn(wxDC *pDC, int nr, wxRect &rect) {}

    // Text in einer ComboBox ausgeben
    virtual void DrawItem(wxDC *pDC, wxRect &rect) { DrawString(pDC, rect, m_label.c_str()); }

    // Soll eine Trennlinie in Listen gezeichnet werden?
    virtual bool DrawSeparatingLine();

    // Enthaelt Item den String?
    virtual bool HasString(const wxString &str) const;
    
    // Liefert den Wert der Spalte
    virtual wxString  BeginEditColumn(int col) const;
    
    // Setz den Wert der Spalte
    virtual void EndEditColumn(int col, const wxString &value);
    
  public:
    // Strings etc. zeichnen
    static void  DrawString(wxDC *pDC, const wxRect &rect, const wxString &str, unsigned fmt = wxALIGN_LEFT);
    static void  DrawLong(wxDC *pDC, const wxRect &rect, long val, unsigned fmt = wxALIGN_RIGHT);
    static void  DrawDouble(wxDC *pDC, const wxRect &rect, double val, unsigned fmt = wxALIGN_RIGHT);

    static void  DrawStringCentered(wxDC *pDC, const wxRect &rect, const wxString &str);
    static void  DrawBye(wxDC *pDC, const wxRect &rect);

    static void  DrawImage(wxDC *pDC, const wxRect &rect, const wxBitmap &img);

    // Komplexere Ausgaben
    static void  DrawPlayer(wxDC *pDC, const wxRect &rect, const PlRec &pl, bool showNaName = true);

    static void  DrawTeam(wxDC *pDC, const wxRect &rect, const TmTeam &tm, bool showNaName = true);
    static void  DrawGroup(wxDC *pDC, const wxRect &rect, const TmGroup &gr);
    static void  DrawEntry(wxDC *pDC, const wxRect &rect, const TmEntry &tm, bool showNaName = true);

    // Ergebnisausgabe
    static void  DrawResult(wxDC *pDC, const wxRect &rect, short resA, short resX);

    // Zeichenhilfen: 
    // String verkuerzen, bis er in das Rechteck passt
    static wxString MakeShortString(wxDC* pDC, const wxString &str, const wxRect &rect);
  protected:
    // Variablen fuers Zeichnen
    static  unsigned  offset;       // Abstand vom Rand des Rechtecks
    static  unsigned  naNameWidth;  // Breite fuer das Verbandskuerzel
    static  unsigned  plNrWidth;    // Breite fuer die Startnummer
    static  unsigned  tmNameWidth;  // Breite fuer den Mannschaftsnamen

    long  m_type;   // Fuer WM_COMMAND msg
    long  m_id;
    wxString m_label;
    wxColor  m_foreColor;
};


inline  ListItem::ListItem()
{
  m_type  = IDC_EDIT;
  m_id    = 0;
  m_foreColor = wxNullColour;
}


inline  ListItem::ListItem(long id, const wxString &label, long type)
{
  m_type  = type;
  m_id    = id; 
  m_label = label;
  m_foreColor = wxNullColour;
}



inline  void  ListItem::DrawLong(wxDC *pDC, const wxRect &rect, long val, unsigned fmt)
{
  wxChar  buf[32];
  _ltot(val, buf, 10);

  DrawString(pDC, rect, buf, fmt);
}
 


inline  void  ListItem::DrawDouble(wxDC *pDC, const wxRect &rect, double val, unsigned fmt)
{
  DrawString(pDC, rect, wxString::FromCDouble(val), fmt);
}
 


inline  void  ListItem::DrawStringCentered(wxDC *pDC, const wxRect &rect, const wxString &str)
{
  DrawString(pDC, rect, str, wxALIGN_CENTER);
}


inline  void  ListItem::DrawBye(wxDC *pDC, const wxRect &rect)
{
  DrawStringCentered(pDC, rect, _("BYE"));
}



#endif