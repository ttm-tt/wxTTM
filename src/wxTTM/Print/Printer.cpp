/* Copyright (C) 2020 Christoph Theis */

// Druckerklasse

#include  "stdafx.h"

#include  "Printer.h"
#include  "Profile.h"
#include  "Preview.h"

#include  "TT32App.h"

#include "wx/pdfdc.h"

// Uncomment wenn man den Generic Page Dialog verwenden will
// #include "wx/generic/prntdlgg.h"

long Printer::leftMargin   = 400;  // 25mm
long Printer::topMargin    = 280;  //  5mm
long Printer::rightMargin  = 200;
long Printer::bottomMargin = 0;

extern Profile ttProfile;

// -----------------------------------------------------------------------
Printer::Printer(bool showDlg) 
{
  wxString lang = CTT32App::instance()->GetPrintLanguage();

  trans = new wxTranslations();
  trans->SetLanguage(lang);

  trans->AddStdCatalog();
  trans->AddCatalog("ttm");

  dcOutput = NULL;

  cH = 0;
  cW = 0;

  height = 0;
  width = 0;

  dcOutput = NULL;

  currentFont = 0;
  
  aborted = false;
  if (showDlg)
    GetPrinterDC();
}


Printer::~Printer()
{
  delete dcOutput;
  delete trans;
}


// -----------------------------------------------------------------------
bool  Printer::StartDoc(const wxString &name)
{
  if (dcOutput == NULL)
  {
    dcOutput = new wxPrinterDC();
    SetCoordSystem(dcOutput);
  }

  dcOutput->StartDoc(name);

  return true;
}


bool  Printer::EndDoc()
{
  if (dcOutput == NULL)
    return false;

  dcOutput->EndDoc();

  return true;
}


bool  Printer::AbortDoc()
{
  if (dcOutput == NULL)
    return false;

  // dcOutput->AbortDoc();
  dcOutput->EndDoc();
  
  aborted = true;

  return true;
}


bool  Printer::StartPage()
{
  if (dcOutput == NULL)
    return false;

  // Feature of Windows: EndPage setzt Koordinatensystem auf MM_TEXT!!!!
  SetCoordSystem(dcOutput);

  // Und StartPage schicken
  dcOutput->StartPage();

  return 0;
}


bool  Printer::EndPage()
{
  if (dcOutput == NULL)
    return false;

  dcOutput->EndPage();

  return true;
}


bool  Printer::PrinterAborted() const
{
  return aborted;
}


// -----------------------------------------------------------------------
bool  Printer::Line(const wxPoint &from, const wxPoint &to, int width, wxPenStyle style)
{
  if (dcOutput == NULL)
    return false;

	# ifdef DEBUG_PRINT
	FILE *file = fopen(DEBUG_PRINT, "a");
	fprintf(file, "Line %d %d %d %d %d\n", 
          from.x, from.y, to.x, to.y, width);
	fclose(file);
	# endif

  wxPen pen(*wxBLACK, (1440L * width) / 254, style);
  wxPen oldPen = dcOutput->GetPen();
  dcOutput->SetPen(pen);

  dcOutput->DrawLine(from.x, from.y, to.x, to.y);

  dcOutput->SetPen(oldPen);

  return true;
}


bool  Printer::Rectangle(const wxRect &rect, int width, bool fill, const wxColor &bgColor)
{
  if (dcOutput == NULL)
		return false;

	# ifdef DEBUG_PRINT
	FILE *file = fopen(DEBUG_PRINT, "a");
	fprintf(file, "Rectangle %d %d %d %d %d %d\n", 
          rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom(), width, fill);
	fclose(file);
	# endif

  if (fill)
  {
    wxBrush brush(bgColor, wxBRUSHSTYLE_SOLID);
    wxBrush oldBrush = dcOutput->GetBrush();
    dcOutput->SetBrush(brush);

    dcOutput->DrawRectangle(rect);

    dcOutput->SetBrush(oldBrush);
  }
  else
  {
    wxPen pen(*wxBLACK, (1440L * width) / 254, wxPENSTYLE_SOLID);
    wxPen oldPen = dcOutput->GetPen();
    dcOutput->SetPen(pen);

    wxBrush oldBrush = dcOutput->GetBrush();
    dcOutput->SetBrush(*wxTRANSPARENT_BRUSH);

    dcOutput->DrawRectangle(rect);

    dcOutput->SetPen(oldPen);    
    dcOutput->SetBrush(oldBrush);
  }

  return true;
}


bool  Printer::DrawImage(wxPoint &topLeft, wxPoint &bottomRight, const wxSize &size, const wxString &imgFile)
{
  wxImage  img;
  img.LoadFile(imgFile);
  
  return DrawImage(topLeft, bottomRight, size, img);
}  
 
 
bool  Printer::DrawImage(wxPoint &topLeft, wxPoint &bottomRight, const wxSize &size, const wxImage &img)
{ 
  if (!img.Ok())
    return false;

  double scale = std::min( 
    ((double) size.GetHeight()) / ((double) img.GetHeight()),
    ((double) size.GetWidth())  / ((double) img.GetWidth()) );

  int imgWidth  = (int) (scale * img.GetWidth());
  int imgHeight = (int) (scale * img.GetHeight());

  int l = topLeft.x;
  int r = bottomRight.x;
  int t = topLeft.y;
  int b = bottomRight.y;
  
  if (l == -1)
    l = r - imgWidth; // (imgWidth * rect.Height()) / imgHeight;
  else if (r == -1)
    r = l + imgWidth; // (imgWidth * rect.Height()) / imgHeight;
    
  if (t == -1)
    t = b - imgHeight;
  else if (b == -1)
    b = t + imgHeight;
    
  topLeft = wxPoint(l, t);
  bottomRight = wxPoint(r, b);
    
  // Zum konvertieren wxScreenDC statt dcOutput nehmen:
  // Ohne ist das Ergebnis (zumindest fuer PDF) nicht OK und es gibt einen Fehler.
  dcOutput->DrawBitmap(wxBitmap(img.Scale(imgWidth, imgHeight), wxScreenDC()), topLeft);

  return true;
}


bool  Printer::Text(const wxRect &rect, const wxString &text, int fmt, short font)
{
  if (dcOutput == NULL)
	 return false;

	# ifdef DEBUG_PRINT
	FILE *file = fopen(DEBUG_PRINT, "a");
	_ftprintf(file, wxT("Text %d %d %d %d %s\n"), 
          rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom(), text);
	fclose(file);
  # endif

  // Make sure there is a valid string.
  if (!text || text[0] == '\0')
	  return 0;

  wxFont oldFont;

  if (font > 0 && fontTable[font].IsOk())
  {
    oldFont = dcOutput->GetFont();
    dcOutput->SetFont(fontTable[font]);
  }

  wxString str = text;

  if (TextWidth(str) > rect.GetWidth())
  {
    wxStringTokenizer tokens(str, "\n");
    str = "";
    while (tokens.HasMoreTokens())
    {
      wxString tmp = tokens.GetNextToken();

      if (TextWidth(tmp) > rect.GetWidth())
      {
        do
        {
          tmp = tmp.Left(tmp.Length() - 1);
        } while (tmp.Length() > 0 && TextWidth(tmp + wxT("...")) > rect.GetWidth());

        tmp += "...";
      }

      str += tmp;
      if (tokens.HasMoreTokens())
        str += "\n";
    }
  }

  dcOutput->DrawLabel(str, rect, wxALIGN_CENTER_VERTICAL | fmt);
  // ::DrawText(dcOutput, str, wxStrlen(str), &clip, DT_VCENTER | DT_SINGLELINE | fmt);

  if (oldFont.IsOk())
    dcOutput->SetFont(oldFont);

  return 0;
}


bool  Printer::Text(long left, long baseline, const wxString &text, int fmt, short font)
{
  if (dcOutput == NULL)
	 return false;

	# ifdef DEBUG_PRINT
	FILE *file = fopen(DEBUG_PRINT, "a");
	_ftprintf(file, wxT("Text %d %d %s\n"), 
          left, baseline, text, "");
	fclose(file);
  # endif

  // Make sure there is a valid string.
  if (!text || text[0] == '\0')
	return 0;

  wxFont oldFont;

  if (font > 0 && fontTable[font].IsOk())
  {
    oldFont = dcOutput->GetFont();
    dcOutput->SetFont(fontTable[font]);
  }

  wxString str = text;

  wxRect clip(left, baseline, 0, 0);

  // Text ohne Clipping zeichnen
  dcOutput->DrawLabel(str, clip, wxALIGN_BOTTOM | fmt);  
  // ::DrawText(dcOutput, str, -1, &clip, DT_NOCLIP | DT_VCENTER | DT_SINGLELINE | fmt);

  if (oldFont.IsOk())
    dcOutput->SetFont(oldFont);

  return 0;
}


// -----------------------------------------------------------------------
long  Printer::TextHeight(const wxString &text, short font)
{
  if (dcOutput == NULL)
    return 0;

  // Make sure there is a valid string.
  if (!text || !text[0])
	  return (cH);

  wxFont oldFont ;

  if (font && fontTable[font].IsOk() && currentFont != font)
  {
    oldFont = dcOutput->GetFont();
    dcOutput->SetFont(fontTable[font]);
  }

  wxSize size;
  size = dcOutput->GetTextExtent(text);

  long height = size.GetHeight();

  if (oldFont.IsOk())
    dcOutput->SetFont(oldFont);

  // Return the text height.
  return (height);
}


long  Printer::TextWidth(const wxString &text, short font)
{
  if (dcOutput == NULL)
    return 0;

  // Make sure there is a valid string.
  if (!text || !text[0])
	  return (0);

  font &= 0x0FFF;

  wxFont oldFont;

  if (font && fontTable[font].IsOk() && currentFont != font)
  {
    oldFont = dcOutput->GetFont();
    dcOutput->SetFont(fontTable[font]);
  }

  int width = 0;

  if (wxStrchr(text, '\n'))
  {
    wxStringTokenizer tokens(text, "\n");

    while (tokens.HasMoreTokens())
      width = std::max(width, dcOutput->GetTextExtent(tokens.GetNextToken()).GetWidth());
  }
  else
    width = dcOutput->GetTextExtent(text).GetWidth();

  if (oldFont.IsOk())
    dcOutput->SetFont(oldFont);

  // Return the text width.
  return width;
}


// -----------------------------------------------------------------------
// Parse INI setting
bool  Printer::ParseFontString(const wxString &defString, wxString &face, int &height, int &weight, int &attr)
{
  // Build String with:
  // Font name, height, width,
  // escapement, orientation, weight,
  // italic, underline, strikeout
  wxString family = wxT("Courier New");
  height = 10;
  weight = 400;
  attr = 0;

  wxStringTokenizer tokens(defString, ",");
  family = tokens.GetNextToken();
  height = _strtos(tokens.GetNextToken());
  int width = _strtos(tokens.GetNextToken());
  int escapement = _strtos(tokens.GetNextToken());
  int orientation = _strtos(tokens.GetNextToken());
  weight = _strtos(tokens.GetNextToken());
  int italic = _strtos(tokens.GetNextToken());
  int underline = _strtos(tokens.GetNextToken());
  int strikeout = _strtos(tokens.GetNextToken());
  if (italic)    attr |= 0x1;
  if (underline) attr |= 0x2;
  if (strikeout) attr |= 0x4;

  return true;
}


wxString Printer::GetFontDescription(const wxString &defString)
{
  wxString faceName;
  int height, weight, attr;

  ParseFontString(defString, faceName, height, weight, attr);

  wxFontFamily fontFamily = wxFONTFAMILY_DEFAULT;
  if (faceName.Find(wxT("Courier")) >= 0)
    fontFamily = wxFONTFAMILY_TELETYPE;
  else if (faceName.Find(wxT("Arial")) >= 0)
    fontFamily = wxFONTFAMILY_SWISS;
  else if (faceName.Find(wxT("Times")) >= 0)
    fontFamily = wxFONTFAMILY_ROMAN;

  return wxFont(
    height, fontFamily,
    attr & 0x01 ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
    weight >= 500 ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
    (attr & 0x02) == 0x02, faceName
  ).GetNativeFontInfoUserDesc();
}


// Erzeugen und Loeschen von Fonts
bool  Printer::ChooseFont(wxString &defString)
{
  wxString faceName;
  int height, weight, attr;

  ParseFontString(defString, faceName, height, weight, attr);

  if (!ChooseFont(faceName, height, weight, attr))
    return false;

  defString = wxString::Format(
      "%s,%d,%d,%d,%d,%d,%d,%d,%d", 
    faceName.wx_str(), height, 0, 0, 0,
      weight, attr & 0x01 ? 1 : 0, attr & 0x02 ? 1 : 0, attr & 0x04 ? 1 : 0
  );

  return true;
}

bool  Printer::ChooseFont(wxString &faceName, int &height, int &weight, int &attr)
{
  wxFontFamily fontFamily = wxFONTFAMILY_DEFAULT;
  if (faceName.Find(wxT("Courier")) >= 0)
    fontFamily = wxFONTFAMILY_TELETYPE;
  else if (faceName.Find(wxT("Arial")) >= 0)
    fontFamily = wxFONTFAMILY_SWISS;
  else if (faceName.Find(wxT("Times")) >= 0)
    fontFamily = wxFONTFAMILY_ROMAN;

  wxFontData fontData;
  fontData.SetInitialFont(wxFont(
    height, fontFamily,
    attr & 0x01 ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
    weight >= 500 ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
    (attr & 0x02) == 0x02, faceName
  ));

  wxFontDialog fontDlg(wxTheApp->GetTopWindow(), fontData);
  if (fontDlg.ShowModal() != wxID_OK)
    return false;

  wxFont font = fontDlg.GetFontData().GetChosenFont();

  faceName = font.GetFaceName();
  height = font.GetPointSize();
  weight = font.GetWeight() == wxFONTWEIGHT_BOLD ? 700 : 400;

  attr = 0;
  if (font.GetStyle() == wxFONTSTYLE_ITALIC)
    attr |= 0x01;
  if (font.GetUnderlined())
    attr |= 0x02;

  return true;
}


short  Printer::CreateFont(const wxString &faceName, int height, int weight, int attr)
{
  // Ich kenne hier nicht immer den Drucker, aber haette ich einen, waere er TWIPS
  long  logY = 1440L; 

  // wxWidgets rechnet pt in pixel anhand vom ScreenDC um, 
  // ich brauche aber Pixel im Printer. Daher von pt nach pixel im Printer 
  // und dann nach pt in Screen umrechnen
  int h = wxRound( ((1. * height * logY) / wxScreenDC().GetPPI().GetHeight()) + 0.5 );

  for (short i = 1; i < MAX_LOGICAL_FONTS; i++)
  {
    if (!fontTable[i].IsOk())
    {
      wxFontFamily fontFamily = wxFONTFAMILY_DEFAULT;
      if (faceName.Find(wxT("Courier")) >= 0)
        fontFamily = wxFONTFAMILY_TELETYPE;
      else if (faceName.Find(wxT("Arial")) >= 0)
        fontFamily = wxFONTFAMILY_SWISS;
      else if (faceName.Find(wxT("Times")) >= 0)
        fontFamily = wxFONTFAMILY_ROMAN;

      wxFont *newFont = wxTheFontList->FindOrCreateFont(
          h, fontFamily, 
          attr & 0x01 ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL, 
          weight >= 500 ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL, 
          (attr & 0x02) == 0x02, faceName);

      if (newFont) 
      {
        fontTable[i] = *newFont;
        return i;
      }
      else
        return 0;
    }
  }

  return 0;
}


short Printer::DeriveFont(short font, int weight, int attr)
{
  if (!fontTable[font].IsOk())
    return 0;

  for (int i = 1; i < MAX_LOGICAL_FONTS; i++)
  {
    if (!fontTable[i].IsOk())
    {
      fontTable[i] = fontTable[font];
      fontTable[i].SetWeight(weight > 500 ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
      fontTable[i].SetStyle((attr & 0x01) == 0x01 ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL);
      fontTable[i].SetUnderlined((attr & 0x02) == 0x02);

      return i;
    }
  }

  return 0;
}


short  Printer::LoadFont(const wxString &prof, const wxString &app, const wxString &key)
{
  // Beschreibun steht in ini
  wxString string = ttProfile.GetString(app, key);

  if (!string)
    return CreateFont(wxT("Courier New"), 10, 400, 0);

  wxString family;
  int height, weight, attr;

  ParseFontString(string, family, height, weight, attr);

  short font = CreateFont(family, height, weight, attr);

  return font;
}


void  Printer::DeleteFont(short font)
{
  if (font == 0 || font == currentFont)
    return;

  fontTable[font] = wxFont();
}


short  Printer::SelectFont(short font)
{
  if (dcOutput && !fontTable[0].IsOk())
    fontTable[0] = dcOutput->GetFont();

  if (!dcOutput || !fontTable[font].IsOk())
    return 0;

  if (font == currentFont)
    return currentFont;

  short  oldFont = currentFont;
  currentFont = font;

  dcOutput->SetFont(fontTable[font]);

  cW = dcOutput->GetCharWidth();
  cH = dcOutput->GetCharHeight();

  return oldFont;
}


// -----------------------------------------------------------------------
// Holt Device Kontext
void  Printer::GetPrinterDC()
{
  wxPrintDialog dlg(wxTheApp->GetTopWindow(), (wxPrintData *) NULL);
  if (dlg.ShowModal() != wxID_OK)
  {
    dcOutput = NULL;
    aborted = true;

    return;
  }

  dcOutput = dlg.GetPrintDC();

  SetCoordSystem(dcOutput);
}


// Koordinatensystem setzen
void  Printer::SetCoordSystem(wxDC *dc)
{
  if (CTT32App::instance()->GetPrintScaleToPaperSize())
  {
    double scaleX = 1., scaleY = 1.;

    wxRect paperSize = dc->GetSizeMM();
    if (paperSize.width < paperSize.height && paperSize.width != 210 && paperSize.height != 297)
    {
      scaleX = (double) paperSize.width / 210.;
      scaleY = (double) paperSize.height / 297.;
    }
  
    if (paperSize.width > paperSize.height && paperSize.width != 297 && paperSize.height != 210)
    {
      scaleX = (double) paperSize.width / 297.;
      scaleY = (double) paperSize.height / 210.;
    }

    dc->SetUserScale(std::min(scaleX, scaleY), std::min(scaleX, scaleY));
  }

  // dc->SetDeviceOrigin((leftMargin * dc->GetPPI().GetWidth()) / 1440, (topMargin * dc->GetPPI().GetHeight()) / 1440);

  dc->SetMapMode(wxMM_TWIPS);

  dc->SetDeviceOrigin(dc->LogicalToDeviceXRel(leftMargin), dc->LogicalToDeviceYRel(topMargin));

  width = abs(dc->DeviceToLogicalXRel(dc->GetSize().GetWidth()));
  height = abs(dc->DeviceToLogicalYRel(dc->GetSize().GetHeight()));

  height -= (topMargin + bottomMargin);
  width  -= (leftMargin + rightMargin);
}



// -----------------------------------------------------------------------
// Preview-Klasse
PrinterPreview::PrinterPreview(const wxString &docName, bool showPageDlg)
              : Printer(false)
{
  previewWindow = NULL;
  dcPrinter = NULL;

  if (showPageDlg)
  {
  #ifdef __PRINTDLGH_G_
    wxGenericPageSetupDialog dlg(wxTheApp->GetTopWindow());
  #else
    wxPageSetupDialog dlg(wxTheApp->GetTopWindow());
  #endif

    if (dlg.ShowModal() != wxID_OK)
    {
      aborted = true;
      return;
    }

    printData = dlg.GetPageSetupDialogData().GetPrintData();
  }

  // Drucker als Referenzdevice merken
  dcPrinter = new wxPrinterDC(printData);
  SetCoordSystem(dcPrinter);

  // Immer mit neuem Preview-Fenster anfangen
  previewWindow = (CPreview *) wxXmlResource::Get()->LoadFrame(wxTheApp->GetTopWindow(), "Preview");
  wxASSERT(previewWindow);

  previewWindow->SetPrinter(this);

  previewWindow->SetSize(width + leftMargin + rightMargin, height + topMargin + bottomMargin);
}


PrinterPreview::~PrinterPreview()
{
  delete dcPrinter;
  // Preview-Fenster nicht zerstoeren
}  


bool  PrinterPreview::StartDoc(const wxString &docName)
{
  previewWindow->SetTitle(docName);

  return dcPrinter != NULL;
}


bool  PrinterPreview::EndDoc()
{
  if (previewWindow)
  {
    wxCommandEvent evt(IDC_ENDDOC);

    previewWindow->Show(true);
    previewWindow->ProcessWindowEvent(evt);
  }

  return true;
}


bool  PrinterPreview::AbortDoc()
{
  if (previewWindow)
  {
    wxCommandEvent evt(IDC_ENDDOC);

    previewWindow->Show(true);
    previewWindow->ProcessWindowEvent(evt);
  }

  previewWindow = NULL;

  aborted = true;
  
  return true;
}


bool  PrinterPreview::StartPage()
{
  // Abmessungen in Pixel umrechnen, wxMetaFileDC getht davon aus.
  int w = dcPrinter->GetSize().GetWidth();
  int h = dcPrinter->GetSize().GetHeight();

  // int w = (width * dcPrinter->GetPPI().GetWidth()) / 1440;
  // int h = (height * dcPrinter->GetPPI().GetHeight()) / 1440;

  dcOutput = new wxMetafileDC(*dcPrinter, wxEmptyString, w, h);
  
  SetCoordSystem(dcOutput);

  return true;
}


bool  PrinterPreview::EndPage()
{
  if (dcOutput == NULL)
    return false;

  wxMetafile *metaFile = ((wxMetafileDC *) dcOutput)->Close();
  if (previewWindow)
    previewWindow->AddPage(metaFile);

  delete dcOutput;
  dcOutput = 0;

  return true;
}


// -----------------------------------------------------------------------
PrinterPdf::PrinterPdf(const wxString &fileName, bool showPageDlg) 
          : Printer(false)
{
  delete dcOutput;
  dcOutput = NULL;

  if (showPageDlg)
  {
#ifdef __PRINTDLGH_G_
    wxGenericPageSetupDialog dlg(wxTheApp->GetTopWindow());
#else
    wxPageSetupDialog dlg(wxTheApp->GetTopWindow());
#endif

    if (dlg.ShowModal() != wxID_OK)
    {
      aborted = true;
      return;
    }

    printData = dlg.GetPageSetupDialogData().GetPrintData();
  }

  SetFilename(fileName);
}


void PrinterPdf::SetFilename(const wxString &fileName)
{
  if (dcOutput)
    delete dcOutput;

  printData.SetFilename(fileName);
  dcOutput = new wxPdfDC(printData);
  SetCoordSystem(dcOutput);
}


wxString PrinterPdf::GetFilename() const
{
  return printData.GetFilename();
}


PrinterPdf::~PrinterPdf()
{
}


bool PrinterPdf::StartDoc(const wxString &docName)
{
  if (!Printer::StartDoc(docName))
    return false;

  ((wxPdfDC *) dcOutput)->GetPdfDocument()->SetTitle(docName);
  ((wxPdfDC *) dcOutput)->GetPdfDocument()->SetAuthor("TTM");

  return true;
}


bool PrinterPdf::Bookmark(const wxString &txt, int level, double y)
{
  if  (dcOutput == NULL || ((wxPdfDC *) dcOutput)->GetPdfDocument() == NULL)
    return false;

  ((wxPdfDC *) dcOutput)->GetPdfDocument()->Bookmark(txt, level, y);

  return true;
}
