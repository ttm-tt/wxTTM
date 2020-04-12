/* Copyright (C) 2020 Christoph Theis */

#ifndef _WXSTRINGTOKENIZEREX_H_
#define _WXSTRINGTOKENIZEREX_H_

class wxStringTokenizerEx : public wxStringTokenizer
{
public:
  wxStringTokenizerEx(
      const wxString& str, 
      const wxString& delims = wxDEFAULT_DELIMITERS,
      wxStringTokenizerMode mode = wxTOKEN_DEFAULT)
      : wxStringTokenizer(str, delims, mode)
  {
  }


  // Konstruktor mit "char *", String wird als UTF-8 angenommen
  wxStringTokenizerEx(
      const char * str, 
      const wxString& delims = wxDEFAULT_DELIMITERS,
      wxStringTokenizerMode mode = wxTOKEN_DEFAULT)
      : wxStringTokenizer(wxString(str, wxConvAuto()), delims, mode)
  {
  }


  wxString GetNextToken()
  {
    wxString tmp = wxStringTokenizer::GetNextToken();
    if (tmp.Length() > 1 && tmp.GetChar(0) == '"' && tmp.GetChar(tmp.Length() - 1) == '"')
      tmp = tmp.Mid(1, tmp.Length() - 2);

    return tmp;
  }
};

#endif