/* Copyright (C) 2020 Christoph Theis */

// Preview-Objekt
#ifndef  PREVIEWWND_H
#define  PREVIEWWND_H

#include "Printer.h"

class CPreviewWnd : public wxScrolledWindow
{
  // Construction
  public:
	  CPreviewWnd();
   ~CPreviewWnd();

  // Attributes
  public:

  // Operations
  public:
    // Setzt das aktuelle Metafile fuer Display
    void  SetMetafile(wxMetafile *file);

    // Setzt die "Papiergroesse"
    void  SetSize(long newWidth, long newHeight);

    // ZoomIn / ZoomOut
    void  ZoomIn();   // zoom *= 2.
    void  ZoomOut();  // zoom /= 2.
    void  SetZoom(double newZoom);

    // Horizontales / Vertikales Scrollen
    void  SetHorzOffset(long value);
    void  SetVertOffset(long value);
    
    const Printer * GetPrinter() const {return printer;}
    // Capture (!) pointer, it is deleted outside
    void  SetPrinter(Printer *ptr) {printer = ptr;}

  private:
    void OnPaint(wxPaintEvent &);
    void OnSize(wxSizeEvent &);

  private:
    wxMetafile *metaFile;
    long    width;
    long    height;
    long    vertOffset;
    long    horzOffset;
    double  zoom;
    
    Printer *printer;

  DECLARE_DYNAMIC_CLASS(CPreviewWnd)
  DECLARE_EVENT_TABLE()
};



#endif