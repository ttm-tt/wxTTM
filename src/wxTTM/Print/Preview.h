/* Copyright (C) 2020 Christoph Theis */

#ifndef _PREVIEW_H_
#define _PREVIEW_H_


// -----------------------------------------------------------------------
// CPreview form view

#include  "FormViewEx.h"
#include  "PreviewWnd.h"

class Printer;

class CPreview : public wxFrame
{
  public:
	  CPreview();           
   ~CPreview();

  public:
    virtual bool Show(bool show = true);

  public:
    // "Papiergroesse" setzen
    void  SetSize(long newWidth, long newHeight);

    // Neue Seite einfuegen
    void  AddPage(wxMetafile *metaFile) 
    {
      if (pages == 0)
        pages = (wxMetafile **) malloc(64 * sizeof(wxMetafile *));
      else if ( (lastPage % 64) == 0 )
        pages = (wxMetafile **) realloc(pages, 2 * lastPage * sizeof(wxMetafile *));

      pages[lastPage++] = metaFile;
    }

    const Printer * GetPrinter() const 
    {
      return XRCCTRL(*this, "PreviewWnd", CPreviewWnd)->GetPrinter();
    }
    
    
    void SetPrinter(Printer *ptr)
    {
      if (previewWnd == NULL)
        previewWnd = XRCCTRL(*this, "PreviewWnd", CPreviewWnd);

      previewWnd->SetPrinter(ptr);
    }

  protected:
    void  SaveSize();
    void  RestoreSize();

  private:
    int  origX;
    int  origY;
    int  origW;
    int  origH;
    
  private:
    void OnStartDoc(wxCommandEvent &);
    void OnEndDoc(wxCommandEvent &);
	  void OnPreviewFirst(wxCommandEvent &);
	  void OnPreviewLast(wxCommandEvent &);
	  void OnPreviewNext(wxCommandEvent &);
	  void OnPreviewPrev(wxCommandEvent &);
    void OnZoom(wxCommandEvent &);
    void OnClose(wxCommandEvent &);
    void OnPrint(wxCommandEvent &);
    void OnCharHook(wxKeyEvent &);

    void OnPreview();

  private:
    // Anzeigeobjekt
    CPreviewWnd  *previewWnd;

    // Groesse der Seite
    long  height;
    long  width;

    // Stack fuer die Seiten
    // TODO: deque oder aehnliches
    wxMetafile **pages;
    short  lastPage;
    short  currentPage;

    wxPrintData printData;

  DECLARE_DYNAMIC_CLASS(CPreview)
  DECLARE_EVENT_TABLE()
};

#endif 
