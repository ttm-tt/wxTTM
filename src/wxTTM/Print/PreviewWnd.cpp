/* Copyright (C) 2020 Christoph Theis */

// CPreviewWnd.cpp : implementation file

#include "stdafx.h"
#include "TT32App.h"
#include "PreviewWnd.h"
#include "ListItem.h"
#include "Printer.h"


IMPLEMENT_DYNAMIC_CLASS(CPreviewWnd, wxScrolledWindow)

BEGIN_EVENT_TABLE(CPreviewWnd, wxScrolledWindow)
  EVT_PAINT(CPreviewWnd::OnPaint)
  EVT_SIZE(CPreviewWnd::OnSize)
END_EVENT_TABLE()



// -----------------------------------------------------------------------
// CCPreviewWnd

CPreviewWnd::CPreviewWnd()
{
  width  = 1;
  height = 1;
  horzOffset = 0;
  vertOffset = 0;
  zoom   = 1.;

  metaFile = NULL;
  printer = NULL;

  ShowScrollbars(wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS);
}

CPreviewWnd::~CPreviewWnd()
{
  if (printer)
    printer->AbortDoc();
}


// -----------------------------------------------------------------------
void  CPreviewWnd::SetMetafile(wxMetafile *file)
{
  metaFile = file;
  Refresh();
}


void  CPreviewWnd::SetSize(long newWidth, long newHeight)
{
  width = newWidth;
  height = newHeight;
}


void  CPreviewWnd::ZoomIn()
{
  SetZoom(zoom * 2);
}


void  CPreviewWnd::ZoomOut()
{
  SetZoom(zoom / 2);
}


void  CPreviewWnd::SetZoom(double newZoom)
{
  zoom = newZoom;
  
  SetScrollRate(1, 1);

  wxRect rect = GetClientRect();

  double scale = ((double) rect.GetHeight()) / height;
  double rectWidth = ((double) width) * scale * zoom;
  double rectHeight = ((double) rect.GetHeight()) * zoom;

  SetScrollPageSize(rect.GetWidth(), rect.GetHeight());
  SetVirtualSize(rectWidth, rectHeight);

  Refresh();
}


void  CPreviewWnd::SetHorzOffset(long value)
{
  horzOffset = value;
  Refresh();
}


void  CPreviewWnd::SetVertOffset(long value)
{
  vertOffset = value;
  Refresh();
}


// -----------------------------------------------------------------------
// CCPreviewWnd message handlers

void CPreviewWnd::OnPaint(wxPaintEvent &evt) 
{
  // Get Device context
	wxPaintDC dc(this); // device context for painting

  // Noetig fuer Scrolled Windows
  DoPrepareDC(dc);

  wxRect rect = GetClientRect();

  dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE)));
  dc.DrawRectangle(rect);

  // Skalierung ist das Verhaeltnis zwischen Fensterbreite und Papierbreite (width)
  // Die dargestellte Breite wird entsprechend gewaehlt
  double scale = ((double) rect.GetHeight()) / height;
  double rectWidth = ((double) width) * scale * zoom;
  double rectHeight = ((double) rect.GetHeight()) * zoom;

  // Jetzt noch um Offset korrigieren, offset ebenfalls "zoomen"
  double rectLeft = ((double) horzOffset) * scale * zoom;
  double rectTop  = ((double) vertOffset) * scale * zoom;

  dc.SetBrush(wxBrush(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW)));
  dc.SetTextBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
  dc.SetTextForeground(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));

  dc.DrawRectangle(rectLeft, rectTop, rectWidth, rectHeight);

  // Der Inhalt ist um xxxOffset verkuerzt
  double horzMargins = Printer::leftMargin + Printer::rightMargin;
  double vertMargins = Printer::topMargin + Printer::bottomMargin;

  rectWidth -= horzMargins * scale * zoom;
  rectHeight -= vertMargins * scale * zoom;

  if (metaFile)
  {
    metaFile->Play(&dc, &wxRect(-rectLeft, -rectTop, rectWidth, rectHeight));
    // PlayEnhMetaFile(dc, hmf, &CRect(CPoint(-rectLeft, -rectTop), CSize(rectWidth, rectHeight)));
  }
}


void CPreviewWnd::OnSize(wxSizeEvent &)
{
  wxRect rect = GetClientRect();

  double scale = ((double) rect.GetHeight()) / height;
  double rectWidth = ((double) width) * scale * zoom;
  double rectHeight = ((double) rect.GetHeight()) * zoom;

  SetScrollRate(1, 1);

  SetScrollPageSize(rect.GetWidth(), rect.GetHeight());
  SetVirtualSize(rectWidth, rectHeight);

  Refresh();
}
