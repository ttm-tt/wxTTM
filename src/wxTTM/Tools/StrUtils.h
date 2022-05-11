/* Copyright (C) 2020 Christoph Theis */

#ifndef  STRUTILS_H
#define  STRUTILS_H

enum
{
  STRIP_BEGIN = 1,
  STRIP_END   = 2
};

wxChar * wxstrstrip(wxChar *string, int flag);
char * strstrip(char *string, int flag);
char * strskip(char *str, char *tok);
char *strtrans(char *str, char *tok, char *rpl);
void  FormatDateTime(char *str, const char *format, const tm &t);
bool  ParseDateTime(const char *str, const char *format, tm &t);
wxString URLEncode(const wxString &);
wxString URLDecode(const wxString &);


#endif