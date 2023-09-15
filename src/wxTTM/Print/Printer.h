/* Copyright (C) 2020 Christoph Theis */

// Eine Drucker Klasse
#ifndef  PRINTER_H
#define  PRINTER_H

#define  MAX_LOGICAL_FONTS  20

// Windows definiert ChooseFont entsprefhend von UNICODE
#ifdef ChooseFont
# undef ChooseFont
#endif

class  CPreview;

class  Printer
{
  public:
    static bool ChooseFont(wxString &defString);
    static bool ChooseFont(wxString &face, int &height, int &weight, int &attr);
    static wxString GetFontDescription(const wxString &defString);

  private:
    static bool ParseFontString(const wxString &defString, wxString &face, int &height, int &weight, int &attr);

  public:
    static long  leftMargin;   // Abstand des Textes vom linken Rand
    static long  topMargin;    // Abstand des Textes vom oberen Rand
    static long  rightMargin;  // Abstand zum rechten Rand
    static long  bottomMargin; // Abstand zum unteren Rand

  public:
    Printer(bool showPrintDlg = true);
    virtual  ~Printer();

  public:
    virtual bool   IsPDF() const {return false;}
    virtual bool   IsPreview() const {return false;}

  // Dokumentenverwaltung
  public:
    virtual  bool  StartDoc(const wxString &);
    virtual  bool  EndDoc();
    virtual  bool  AbortDoc();

    virtual  bool  StartPage();
    virtual  bool  EndPage();

    virtual  bool  PrinterAborted() const;

    virtual  bool  Bookmark(const wxString &txt, int level = 0, double y = 0) {return true;}
    virtual  void SetFilename(const wxString &) {};
    virtual  wxString GetFilename() const {return wxEmptyString;}

  // I18N
  public:
    wxString GetString(const wxString &str) { const wxString *tmp = trans->GetTranslatedString(str); return tmp ? *tmp : str; }

  private:
    wxTranslations *trans;

  // Zeichenroutinen
  public:
    bool  Line(const wxPoint &from, const wxPoint &to, int width = 1, wxPenStyle style = wxPENSTYLE_SOLID);
    bool  Line(int left, int top, int right, int bottom, int width = 1, wxPenStyle style = wxPENSTYLE_SOLID)
    { return Line(wxPoint(left, top), wxPoint(right, bottom), width, style); }

    bool  Rectangle(const wxRect &rect, int width = 1, bool  fill = false, const wxColor &bgColor = *wxBLACK);
    bool  Rectangle(int left, int top, int right, int bottom, int width = 1, bool fill = false, const wxColor &bgColor = *wxBLACK)
    { return Rectangle(wxRect(left, top, right - left, bottom - top), width, fill, bgColor); }

    bool  Text(const wxRect &rect, const wxString &text, int fmt = DT_LEFT, short font = 0);
    bool  Text(long left, long baseline, const wxString &text, int fmt = DT_LEFT, short font = 0);

    long  TextHeight(const wxString &, short font = 0); // Hoehe des Textes
    long  TextWidth(const wxString &, short font = 0);  // Breite des Textes
    
    // bool  DrawImage(const wxRect &rect, const wxString &bmpFile);
    bool  DrawImage(wxPoint &topleft, wxPoint &bottomRight, const wxSize &size, const wxString &bmpFile);    
    bool  DrawImage(wxPoint &topleft, wxPoint &bottomRight, const wxSize &size, const wxImage &image);

  // Verwaltung von Fonts
  public:
    short  CreateFont(const wxString &faceName, int height, int weight, int attr);
    short  DeriveFont(short font, int weight, int attr, double size = 1.0);
    short  LoadFont(const wxString &prof, const wxString &app, const wxString &key);
    void   DeleteFont(short font);
    short  SelectFont(short font);
    short  GetFont() const {return currentFont;}

  public:
    long  cH;           // Durchschnittliche Hoehe eines Zeichens im akt. Font
    long  cW;           // Durchschnittliche Breite eines Zeichens im akt. Font

    long  width;        // Breite des druckbaren Bereiches in twips
    long  height;       // Hoehe des druckbaren Bereiches in twips

  public:
    void  SetPreviewDC(wxDC *dc) {dcOutput = dc;}

    const wxPrintData & GetPrintData() const {return printData;}

    virtual wxDC * GetDC() const {return dcOutput;}

  protected:
    void  GetPrinterDC();
    void  SetCoordSystem(wxDC *dc);

  protected:
    short  currentFont;
    wxFont fontTable[MAX_LOGICAL_FONTS];
    wxDC * dcOutput;
    wxPrintData printData;
    
    bool   aborted;

};

class  PrinterPreview : public Printer
{
  public:
    PrinterPreview(const wxString &title, bool showPageDlg = true);
    virtual  ~PrinterPreview();

  public:
    virtual  bool  IsPreview() const {return true;}

    virtual  bool  StartDoc(const wxString &);
    virtual  bool  EndDoc();
    virtual  bool  AbortDoc();

    virtual  bool  StartPage();
    virtual  bool  EndPage();

  protected:
    // Fenster fuer den Preview
    CPreview *previewWindow;
    // Referenz-Devicecontext
    wxDC *    dcPrinter;
};


class PrinterPdf : public Printer
{
  public:
    PrinterPdf(const wxString &fileName, bool showPageDlg = true);
   ~PrinterPdf();

  public:
    virtual bool IsPDF() const {return true;}

    virtual bool StartDoc(const wxString &);

    virtual bool Bookmark(const wxString &txt, int level = 0, double y = 0);
    virtual void SetFilename(const wxString &);
    virtual wxString GetFilename() const;
};

#endif