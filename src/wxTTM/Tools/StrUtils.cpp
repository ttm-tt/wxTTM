/* Copyright (C) 2020 Christoph Theis */

// strutils.cpp
// Utilities zur Stringmanipulation
// 26.03.94 (ChT)

#include  "stdafx.h"

#include  "StrUtils.h"

#include  <string.h>
#include  <stdlib.h>

const  char BLANK = ' ';

char * strstrip(char *string, int flag)
{
  if (!string || !string[0])
    return string;

  char *str = strdup(string);
  int  len  = strlen(str);

  char *start = str;      // Erstes Zeichen
  char *end = str + len;  // Abschliessende '\0'

  if (flag & STRIP_BEGIN)
    while (*start == BLANK)
      start++;

  if (flag & STRIP_END)
    while (end > start && *(end-1) == BLANK)
      end--;

  *end = '\0';
  strcpy(string, start);
  free(str);

  return string;   // war 'str'
}


char * strskip(char *str, char *tok)
{
  char *p = str;
  while (*p && strchr(tok, *p))
    p++;

  return p;
}



// Translate tok to rpl
char *strtrans(char *str, char *tok, char *rpl)
{
  char *string = strdup(str);

  char *start = string;
  char *token;

  *str = '\0';

  while (*start && (token = strstr(start, tok)) != NULL)
  {
    memset(token, '\0', strlen(tok));
    strcat(str, start);
    strcat(str, rpl);

    start = token + strlen(tok);
  }

  strcat(str, start);

  free(string);

  return str;
}


void  FormatDateTime(char *str, const char *format, const tm &t)
{
  short year   = (t.tm_year < 0 ? 0 : t.tm_year + 1900);
  short month  = t.tm_mon < 0 ? 0 : t.tm_mon + 1;
  short day    = t.tm_mday <= 0 ? 0 : t.tm_mday;
  short hour   = t.tm_hour < 0 ? 0 : t.tm_hour;
  short min    = t.tm_min < 0 ? 0 : t.tm_min;
  short sec    = t.tm_sec < 0 ? 0 : t.tm_sec;

  // Ergebnis mit '\0' initialisieren
  memset(str, 0, strlen(format)+1);

  for (int i = strlen(format); i--; )
  {
    switch (format[i])
    {
      case 'D' :
        str[i] = (day % 10) + '0';
        day /= 10;
        break;

      case 'M' :
        str[i] = (month % 10) + '0';
        month /= 10;
        break;

      case 'Y' :
        str[i] = (year % 10) + '0';
        year /= 10;
        break;

      case 'H' :
      case 'h' :
        str[i] = (hour % 10) + '0';
        hour /= 10;
        break;

      case 'm' :
        str[i] = (min % 10) + '0';
        min /= 10;
        break;

      case 's' :
        str[i] = (sec % 10) + '0';
        sec /= 10;
        break;

      default :
        str[i] = format[i];
        break;
    }
  }
}


bool  ParseDateTime(const char *str, const char *format, tm &t)
{
  short  year = -1, month = -1, day = -1, hour = -1, min = -1, sec = -1;

  unsigned posFormat, posStr, n = strlen(str);
  for (posFormat = 0, posStr = 0; posStr < n; posStr++, posFormat++) 
  {
    // Abbruch, wenn das Ende der Maske erreicht wurde
    if (posFormat >= strlen(format))
      break;

    // Fehler, wenn eine Ziffer in str steht, aber keine in der Maske 
    // erwartet wurde
    if ( isdigit(str[posStr]) && !strchr("YMDHhms", format[posFormat]) )
      return false;

    // Wenn keine Ziffer vorliegt, auf die naechste Formatierung vorruecken
    if (!isdigit(str[posStr]))
    {
      while ( strchr("YMDHhms", format[posFormat]) )
      {
        if ( (++posFormat) >= strlen(format))
          return false;
      }

      // Weiter mit naechstem Zeichen in der Eingabe
      continue;
    }

    switch (format[posFormat]) 
    {
      case 'D' :
        if (day < 0)
          day = 0;
        day = 10 * day + (str[posStr] - '0');  // TODO: Besserer Hack!
        break;

      case 'M' :
        if (month < 0)
          month = 0;
        month = 10 * month + (str[posStr] - '0');  // TODO: Besserer Hack!
        break;

      case 'Y' :
        if (year < 0)
          year = 0;
        year = 10 * year + (str[posStr] - '0');  // TODO: Besserer Hack!
        break;

      case 'H' :
      case 'h' :
        if (hour < 0)
          hour = 0;
        hour = 10 * hour + (str[posStr] - '0');  // TODO: Besserer Hack!
        break;

      case 'm' :
        if (min < 0)
          min = 0;
        min = 10 * min + (str[posStr] - '0');  // TODO: Besserer Hack!
        break;

      case 's' :
        if (sec < 0) 
          sec = 0;
        sec = 10 * sec + (str[posStr] - '0');  // TODO: Besserer Hack!
        break;

      default :
        break;
    }
  }

  if (day && month && year >= 0 && year < 100)
    year += (year < 70 ? 2000 : 1900);

  if (month > 12)
      return false;

  if (day >= 0)
  {
    if (day > 31)
      return false;

    if ( (month == 2) || (month == 4) || (month == 6) ||
         (month == 9) || (month == 11) )
    {
      if (day > 30)
        return false;
    }
    
    if (month == 2)
    {
      if (day > 29)
        return false;

      // TODO: Vereinfachte Schaltjahrberechnung
      if ( (year > 0) && (year % 4) && day > 28)
        return false;
    }
  }

  if (hour > 24)
    return false;

  if ( (hour == 24) && ((min > 0) || (sec > 0)) )
    return false;

  if (min >= 60)
    return false;

  if (sec >= 60)
    return false;

  if (year > 0)
    t.tm_year = year - 1900;
  if (month > 0)
    t.tm_mon = month - 1;
  if (day > 0)
    t.tm_mday = day;
  if (hour >= 0)
    t.tm_hour = hour;
  if (min >= 0)
    t.tm_min = min;
  if (sec >= 0)
    t.tm_sec = sec;
      
  return true;
}


// -----------------------------------------------------------------------
wxString URLEncode(const wxString &str)
{
  if (str.Length() == 0)
    return wxEmptyString;

  wxString res;

  const char *utf8 = str.ToUTF8();

  for (const char *ptr = utf8; *ptr; ptr++)
  {
    if (isalnum((unsigned char) *ptr))
      res += *ptr;
    else 
      res += wxString::Format("%%%02X", (unsigned) *ptr);
  }

  return res;
}

wxString URLDecode(const wxString &str)
{
  if (str.Length() == 0)
    return wxEmptyString;

  const char *utf8 = str.ToUTF8();

  char *out = new char[strlen(utf8) + 1];
  memset(out, 0, strlen(utf8) + 1);
  int outidx = 0;

  for (const char *ptr = utf8; *ptr; ptr++)
  {
    if (*ptr != '%')
      out[outidx++] = *ptr;
    else if (strlen(ptr) < 3)
      break;
    else
    {
      unsigned int c; 
      if (sscanf(ptr, "%%%02X", &c) != 1)
        break;

      out[outidx++] = (const char) c;
      ptr += 2;      
    }
  }

  wxString res = wxString::FromUTF8(out);
  delete[] out;

  return res;
}