/* Copyright (C) 2020 Christoph Theis */

#include "stdafx.h"
#include "Validators.h"

#include "Rec.h"

#include <sstream>


// ------------------------------------------------------------------------
// --  Hilfen, um aus Eingabe ein Ergebnis zu berechnen --

// Alles, was nicht isdigit ist, ueberlesen
// Return: 0, falls Ziffer gefunden (ist aktuelle Position)
static int  SkipNonDigit(std::istringstream &istr)
{
  istr >> std::ws;           // Whitespace abtrennen

  while (istr.good())
  {
    char c = istr.peek();
    if (isdigit((unsigned char) c))
      return 0;

    istr >> c >> std::ws;    // Naechstes Zeichen und folg. ws lesen
  }

  // Keine Ziffer gefunden
  return 1;
}


// Aus einer einzelnen Zahl ein vollstaendiges Ergebnis berechnen
// Parm:   Array aus zwei int
// return: immer 0 (bislang)
// verwendet #define MINUS_NULL
static void  CalcOther(short &resA, short &resX, short win, short ahd)
{
  const short MINUS_NULL = (short) 0x8000;

  if (resA == MINUS_NULL)      // Spieler A hat zu 0 verloren
  {
    resA = 0;
	  resX = win;
  }  
  else if (resA < 0)           // Player A hat verloren
  {
    resA = -resA;
    resX = (resA + ahd < win ? win : resA + ahd);
  }
  else                         // Player A hat gewonnen
  {
	  resX = resA;
    resA = (resX + ahd < win ? win : resX + ahd);
  }
}


// Import Funktion
// Erlaubte Eingabe: Ein Ergebnis
//                   Eine Zahl:    > 0: Spieler A hat gewonnen
//                                 < 0: Spieler A hat verloren
//                                  +0: Spieler A hat zu 0 gewonnen
//                                  -0: Spieler A hat zu 0 verloren
//                   leerer String:     Ergebnis ist 0 : 0
// Im Fehlerfall bleibt das alte Ergebnis bestehen (?)
static bool  ConvertFromText(const char *text, short &resA, short &resX, short win, short ahd)
{
  const short MINUS_NULL = (short) 0x8000;

  short set[] = {0,0};       // Ergebnisse

  if (!text || !*text)               // leerer String
  {
    resA = resX = 0;
    return true;
  }

  std::istringstream  istr(text);    // Stream aufsetzen

  istr >> std::ws;

  if (!istr)
    return false;

  // Zur Unterscheidung von +0 und -0: Naechstes Zeichen lesen
  // Naechstes Zeichen ein '-' ?
  int minus = (istr.peek() == '-');

  // In jedem Fall soll jetzt eine Zahl kommen
  istr >> std::dec >> set[0];

  if (istr.fail())
  {
    Beep(440, 250);
    return false;
  }

  // set[0] auf MINUS_NULL, wenn == 0
  if (set[0] == 0 && minus)
    set[0] = MINUS_NULL;

  // Alles ueberlesen, bis eine Ziffer kommt;
  // d.h. Keine Fehlerprüfung auf dem Rest
  SkipNonDigit(istr);

  if (!istr.good())          // Kein weiteres Ergebnis
    CalcOther(set[0], set[1], win, ahd);   // Zweites Ergebnis berechnen 
  else if ( !minus && isdigit(istr.peek()) )
    istr >> std::dec >> set[1];   // gab weiteres Ergebnis
  else
    return false;

  resA = set[0];
  resX = set[1];

  return true;
}



// Export: wandelt Ergbnis (zwei int) in String um
static void  ConvertToText(char *text, short resA, short resX)
{
  sprintf(text, "%2i : %2i", resA, resX);
}


// =======================================================================
IMPLEMENT_DYNAMIC_CLASS(CLongValidator, wxValidator)

BEGIN_EVENT_TABLE(CLongValidator, wxValidator)
  EVT_CHAR(CLongValidator::OnChar)
END_EVENT_TABLE()

CLongValidator::CLongValidator() 
              : wxValidator(), m_val(NULL), m_suppressNull(false)
{
}


CLongValidator::CLongValidator(long *val, bool suppressNull) 
              : wxValidator(), m_val(val), m_suppressNull(suppressNull)
{
}


CLongValidator::CLongValidator(const CLongValidator &val)
  : wxValidator(), m_val(val.m_val), m_suppressNull(val.m_suppressNull)
{
}


wxObject * CLongValidator::Clone() const
{
  return new CLongValidator(*this);
}              


bool CLongValidator::TransferFromWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if ( ((wxTextCtrl *) m_validatorWindow)->GetValue() == "")
      *m_val = 0;
    else
      *m_val = wxAtol( ((wxTextCtrl *) m_validatorWindow)->GetValue() );

    return true;
  }
    
  return false;
}


bool CLongValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if (m_suppressNull && *m_val == 0)
      ((wxTextCtrl *) m_validatorWindow)->SetValue("");
    else
      ((wxTextCtrl *) m_validatorWindow)->SetValue(wxString::Format("%ld", *m_val));

    return true;
  }
  
  return false;
}


bool CLongValidator::Validate(wxWindow *parent)
{ 
  return true;
}


void CLongValidator::OnChar(wxKeyEvent &evt)
{
  if (!m_val || !m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    evt.Skip();
    return;
  }
  
  int keyCode = evt.GetKeyCode();
  if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode>= WXK_START)
  {
    evt.Skip();
    return;
  }
  
  if (!wxIsdigit((wxUniChar) keyCode))
  {
    if (!IsSilent())
      wxBell();
      
    return;
  }
  
  evt.Skip();
}

// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CShortValidator, wxValidator)

BEGIN_EVENT_TABLE(CShortValidator, wxValidator)
  EVT_CHAR(CShortValidator::OnChar)
END_EVENT_TABLE()

CShortValidator::CShortValidator() 
               : wxValidator(), m_val(NULL), m_suppressNull(false)
{
}


CShortValidator::CShortValidator(short *val, bool suppressNull) 
               : wxValidator(), m_val(val), m_suppressNull(suppressNull)
{
}


CShortValidator::CShortValidator(const CShortValidator &val)
               : wxValidator(), m_val(val.m_val), m_suppressNull(val.m_suppressNull)
{
}


wxObject * CShortValidator::Clone() const
{
  return new CShortValidator(*this);
}              


bool CShortValidator::TransferFromWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if ( ((wxTextCtrl *) m_validatorWindow)->GetValue() == "")
      *m_val = 0;
    else
      *m_val = (short) wxAtoi( ((wxTextCtrl *) m_validatorWindow)->GetValue() );

    return true;
  }
  else if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl)))
  {
    *m_val = (short) ((wxSpinCtrl *) m_validatorWindow)->GetValue();

    return true;
  }
    
  return false;
}


bool CShortValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if (m_suppressNull && *m_val == 0)
      ((wxTextCtrl *) m_validatorWindow)->SetValue("");
    else
      ((wxTextCtrl *) m_validatorWindow)->SetValue(wxString::Format("%hd", *m_val));

    return true;
  }
  else if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxSpinCtrl)))
  {
    ((wxSpinCtrl *) m_validatorWindow)->SetValue(*m_val);

    return true;
  }
  
  return false;
}


bool CShortValidator::Validate(wxWindow *parent)
{ 
  return true;
}


void CShortValidator::OnChar(wxKeyEvent &evt)
{
  if (!m_val || !m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    evt.Skip();
    return;
  }
  
  int keyCode = evt.GetKeyCode();
  if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode>= WXK_START)
  {
    evt.Skip();
    return;
  }
  
  if (!wxIsdigit((wxUniChar) keyCode))
  {
    if (!IsSilent())
      wxBell();
      
    return;
  }
  
  evt.Skip();
}

// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CDoubleValidator, wxValidator)

BEGIN_EVENT_TABLE(CDoubleValidator, wxValidator)
  EVT_CHAR(CDoubleValidator::OnChar)
END_EVENT_TABLE()

CDoubleValidator::CDoubleValidator() 
              : wxValidator(), m_val(NULL)
{
}


CDoubleValidator::CDoubleValidator(double *val, const wxString &fmt) 
              : wxValidator(), m_val(val), m_fmt(fmt)
{
}


CDoubleValidator::CDoubleValidator(const CDoubleValidator &val)
              : wxValidator(), m_val(val.m_val), m_fmt(val.m_fmt)
{
}


wxObject * CDoubleValidator::Clone() const
{
  return new CDoubleValidator(*this);
}              


bool CDoubleValidator::TransferFromWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    *m_val = wxAtof( ((wxTextCtrl *) m_validatorWindow)->GetValue() );
    return true;
  }
    
  return false;
}


bool CDoubleValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    ((wxTextCtrl *) m_validatorWindow)->SetValue(wxString::Format(m_fmt, *m_val));
    return true;
  }
  
  return false;
}


bool CDoubleValidator::Validate(wxWindow *parent)
{ 
  return true;
}


void CDoubleValidator::OnChar(wxKeyEvent &evt)
{
  if (!m_val || !m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    evt.Skip();
    return;
  }
  
  int keyCode = evt.GetKeyCode();
  if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode>= WXK_START)
  {
    evt.Skip();
    return;
  }
  
  wxString s = ((wxTextCtrl *) m_validatorWindow)->GetValue();
  s += wxUniChar(keyCode);
  
  wxChar *endPtr= NULL;
  wxStrtod(s, &endPtr);
  
  if (endPtr && *endPtr)
  {
    if (!IsSilent())
      wxBell();
      
    return;
  }
  
  evt.Skip();
}

// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CCharArrayValidator, wxValidator)

BEGIN_EVENT_TABLE(CCharArrayValidator, wxValidator)
  EVT_CHAR(CCharArrayValidator::OnChar)
END_EVENT_TABLE()


CCharArrayValidator::CCharArrayValidator()
                   : wxValidator(), m_val(NULL), m_len(0)
{
}
                   

CCharArrayValidator::CCharArrayValidator(wxChar *val, size_t len) 
              : wxValidator(), m_val(val), m_len(len)
{
}


CCharArrayValidator::CCharArrayValidator(const CCharArrayValidator &val)
              : wxValidator(), m_val(val.m_val), m_len(val.m_len)
{
}


wxObject * CCharArrayValidator::Clone() const
{
  return new CCharArrayValidator(*this);
}              


bool CCharArrayValidator::TransferFromWindow()
{
  if (!m_val)
    return false;
  else if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    wxString s = ((wxTextCtrl *) m_validatorWindow)->GetValue();
    wxStrncpy(m_val, s, m_len - 1);
    m_val[m_len - 1] = 0;
    
    return true;    
  }
  else if (m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox)))
  {
    wxString s = ((wxComboBox *) m_validatorWindow)->GetValue();
    wxStrncpy(m_val, s, m_len - 1);
    m_val[m_len - 1] = 0;
    
    return true;    
  }
  
  return false;
}


bool CCharArrayValidator::TransferToWindow()
{
  if (!m_val)
    return false;
  else if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    ((wxTextCtrl *) m_validatorWindow)->SetValue(wxString(m_val));
    return true;    
  }
  else if (m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox)))
  {
    ((wxComboBox *) m_validatorWindow)->SetValue(wxString(m_val));
    return true;    
  }
  
  return false;
}


bool CCharArrayValidator::Validate(wxWindow *parent)
{ 
  return true;
}

void CCharArrayValidator::OnChar(wxKeyEvent &evt)
{
  if (!m_val)
  {
    evt.Skip();
    return;
  }
  
  int keyCode = evt.GetKeyCode();
  if (keyCode < WXK_SPACE || keyCode == WXK_DELETE || keyCode>= WXK_START)
  {
    evt.Skip();
    return;
  }

  wxString value;
  
  if (m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
    value = ((wxTextCtrl *) m_validatorWindow)->GetValue();
  else if (m_validatorWindow->IsKindOf(CLASSINFO(wxComboBox)))
    value = ((wxComboBox *) m_validatorWindow)->GetValue();
  else
  {
    evt.Skip();    
    return;
  }  
  
  if ( value.Length() >= m_len - 1 )
  {
    if (!IsSilent())
      wxBell();
      
    return;
  }  
  
  evt.Skip();
}


// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CEnumValidator, wxValidator)

CEnumValidator::CEnumValidator()
              : wxValidator(), m_val(NULL), m_set(0)
{
}
                   

CEnumValidator::CEnumValidator(short *val, short set) 
              : wxValidator(), m_val(val), m_set(set)
{
}


CEnumValidator::CEnumValidator(const CEnumValidator &val)
              : wxValidator(), m_val(val.m_val), m_set(val.m_set)
{
}


wxObject * CEnumValidator::Clone() const
{
  return new CEnumValidator(*this);
}              


bool CEnumValidator::TransferFromWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxRadioButton)))
  {
    if ( ((wxRadioButton *) m_validatorWindow)->GetValue() )
      *m_val = m_set;

    return true;    
  }
  else if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxCheckBox)))
  {
    if ( ((wxCheckBox *) m_validatorWindow)->GetValue() )
      *m_val |= m_set;
    else
      *m_val &= ~m_set;

    return true;    
  }
  
  
  return false;
}


bool CEnumValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxRadioButton)))
  {
    ((wxRadioButton *) m_validatorWindow)->SetValue(*m_val == m_set);
    return true;    
  }
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxCheckBox)))
  {
    ((wxCheckBox *) m_validatorWindow)->SetValue( (*m_val & m_set) != 0 );
    return true;    
  }
  
  return false;
}


bool CEnumValidator::Validate(wxWindow *parent)
{ 
  return true;
}

// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CDateValidator, wxValidator)

CDateValidator::CDateValidator()
              : wxValidator(), m_val(NULL)
{
}
                   

CDateValidator::CDateValidator(timestamp *val) 
              : wxValidator(), m_val(val)
{
}


CDateValidator::CDateValidator(const CDateValidator &val)
              : wxValidator(), m_val(val.m_val)
{
}


wxObject * CDateValidator::Clone() const
{
  return new CDateValidator(*this);
}              


bool CDateValidator::TransferFromWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    wxString str = ((wxTextCtrl *) m_validatorWindow)->GetValue();
    
    str.Strip(wxString::both);
    
    wxDateTime dateTime;
    
    if (str.Length() > 0)
    {
      str.Replace("  ", " ", true);
      str.Replace(" ", ".", true);
      
      dateTime.ParseFormat(str, wxT("%d.%m.%Y"));
      if (!dateTime.IsValid() && str.Length() <= 5)
        dateTime.ParseFormat(str, wxT("%d.%m"));
      if (!dateTime.IsValid() && str.Length() <= 2)
        dateTime.ParseFormat(str, wxT("%d"));  
      
      if (dateTime.IsValid())
      {    
        m_val->year = dateTime.GetYear();
        m_val->month = dateTime.GetMonth() + 1;
        m_val->day = dateTime.GetDay();

        // Wenn das Jahr < 1900 ist (frueher macht kaum Sinn, selbst Dorothy ist juenger), 
        // 2000 drauf addieren. Vermutlich meinte man das aktuelle Jahrhundert.
        if (m_val->year < 1900)
          m_val->year += 2000;
      }
      else    
      {
        if (!IsSilent())
          wxBell();
          
        return false;
      }
    }
    else
    {
      m_val->year = 0;
      m_val->month = 0;
      m_val->day = 0;
    }
    
    return true;
  }
  
  return false;
}


bool CDateValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if (m_val->day)
    {
      wxDateTime dateTime(m_val->day, (wxDateTime::Month) (m_val->month - 1), m_val->year);
      ((wxTextCtrl *) m_validatorWindow)->SetValue(dateTime.Format(wxT("%d.%m.%Y")));
    }
    else
    {
      ((wxTextCtrl *) m_validatorWindow)->SetValue("");
    }
    
    return true;
  }
  
  return false;
}


bool CDateValidator::Validate(wxWindow *parent)
{ 
  return true;
}

// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CTimeValidator, wxValidator)

CTimeValidator::CTimeValidator(bool empty)
              : wxValidator(), m_val(NULL), emptyIfNull(empty)
{
}
                   

CTimeValidator::CTimeValidator(timestamp *val, bool empty) 
              : wxValidator(), m_val(val), emptyIfNull(empty)
{
}


CTimeValidator::CTimeValidator(const CTimeValidator &val)
              : wxValidator(), m_val(val.m_val), emptyIfNull(val.emptyIfNull)
{
}


wxObject * CTimeValidator::Clone() const
{
  return new CTimeValidator(*this);
}              


bool CTimeValidator::TransferFromWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    wxString str = ((wxTextCtrl *) m_validatorWindow)->GetValue();
    
    str.Strip(wxString::both);
    
    wxDateTime dateTime;
    
    if (str.Length() > 0)
    {
      str.Replace("  ", " ", true);
      str.Replace(" ", ":", true);

      dateTime.ParseFormat(str, wxT("%H:%M"));
      if (!dateTime.IsValid() && str.Length() <= 2)
        dateTime.ParseFormat(str, wxT("%H"));
      
      if (dateTime.IsValid())
      {
        m_val->hour = dateTime.GetHour();
        m_val->minute = dateTime.GetMinute();
      }
      else    
      {
        if (!IsSilent())
          wxBell();
          
        return false;
      }
    }
    else
    {
      m_val->hour = 0;
      m_val->minute = 0;
    }
    
    return true;
  }
  
  return false;
}


bool CTimeValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if (m_val->hour || m_val->minute)
    {
      wxDateTime dateTime(m_val->hour, m_val->minute, m_val->second, 0);
      ((wxTextCtrl *) m_validatorWindow)->SetValue(dateTime.Format("%H:%M"));
    }
    else
    {
      ((wxTextCtrl *) m_validatorWindow)->SetValue(emptyIfNull ? "" : "00:00");
    }
    
    return true;
  }
  
  return false;
}


bool CTimeValidator::Validate(wxWindow *parent)
{ 
  return true;
}


// -----------------------------------------------------------------------  
IMPLEMENT_DYNAMIC_CLASS(CDateTimeValidator, wxValidator)

CDateTimeValidator::CDateTimeValidator()
  : wxValidator(), m_val(NULL)
{
}


CDateTimeValidator::CDateTimeValidator(timestamp* val)
  : wxValidator(), m_val(val)
{
}


CDateTimeValidator::CDateTimeValidator(const CDateTimeValidator& val)
  : wxValidator(), m_val(val.m_val)
{
}


wxObject* CDateTimeValidator::Clone() const
{
  return new CDateTimeValidator(*this);
}


bool CDateTimeValidator::TransferFromWindow()
{
  // Fields should be r/o 
  return false;
}


bool CDateTimeValidator::TransferToWindow()
{
  if (m_val && m_validatorWindow->IsKindOf(CLASSINFO(wxTextCtrl)))
  {
    if (m_val->day)
    {
      wxDateTime dateTime(
        m_val->day, (wxDateTime::Month)(m_val->month - 1), m_val->year,
        m_val->hour, m_val->minute, m_val->second
      );
      ((wxTextCtrl*)m_validatorWindow)->SetValue(dateTime.Format("%d.%m.%Y %H:%M"));
    }
    else
    {
      ((wxTextCtrl*)m_validatorWindow)->SetValue("");
    }

    return true;
  }

  return false;
}


bool CDateTimeValidator::Validate(wxWindow* parent)
{
  return true;
}

