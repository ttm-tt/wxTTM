/* Copyright (C) 2020 Christoph Theis */

#ifndef _STDAFX_H_
#define _STDAFX_H_

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <wx/wx.h>

#include <wx/aboutdlg.h>
#include <wx/colordlg.h>
#include <wx/datectrl.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/fontdlg.h>
#include <wx/grid.h>
#include <wx/hyperlink.h>
#include <wx/listctrl.h>
#include <wx/mdi.h>
// textbuf must come before memtext
#include <wx/textbuf.h>
#include <wx/memtext.h>
#include <wx/metafile.h>
#include <wx/mstream.h>
#include <wx/notebook.h>
#include <wx/numdlg.h>
#include <wx/odcombo.h>
#include <wx/pen.h>
#include <wx/popupwin.h>
#include <wx/printdlg.h>
#include <wx/print.h>
#include <wx/renderer.h>
#include <wx/spinctrl.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>
#include <wx/regex.h>
#include <wx/textdlg.h>
#include <wx/textfile.h>
#include <wx/tokenzr.h>
#include <wx/translation.h>
#include <wx/txtstrm.h>
#include <wx/url.h>
#include <wx/valgen.h>
#include <wx/wfstream.h>
#include <wx/wizard.h>
#include <wx/aui/auibook.h>
#include <wx/msw/dc.h>
#include <wx/xrc/xmlres.h>

// fuer Definition von "timestamp"
#include "SQLInclude.h"


#ifdef UNICODE
# define tistream  wistream
# define tifstream wifstream
# define tostream  wostream
# define tofstream wofstream
# define tostringstream wostringstream
# define tistringstream wistringstream
#else
# define tistream  istream
# define tifstream ifstream
# define tostream  ostream
# define tofstream ofstream
# define tostringstream ostringstream
# define tistringstream istringstream
#endif

inline short _strtos(const wxString &str) {return (short) _ttoi(str.t_str());}
inline int _strtoi(const wxString &str) {return _ttoi(str.t_str());}
inline long _strtol(const wxString &str) {return _ttol(str.t_str());}
inline float _strtof(const wxString &str) {return _ttof(str.t_str());}


BEGIN_DECLARE_EVENT_TYPES()
  DECLARE_EVENT_TYPE(IDC_EDIT, -1)
  DECLARE_EVENT_TYPE(IDC_ADD, -1)
  DECLARE_EVENT_TYPE(IDC_DELETE, -1)
  DECLARE_EVENT_TYPE(IDC_EVENTS, -1)
  DECLARE_EVENT_TYPE(IDC_EXECUTE, -1)
  DECLARE_EVENT_TYPE(IDC_STARTDOC, -1)
  DECLARE_EVENT_TYPE(IDC_ENDDOC, -1)
  DECLARE_EVENT_TYPE(IDC_ABORTDOC, -1)
  DECLARE_EVENT_TYPE(IDC_UPDATEVIEW, -1)
  DECLARE_EVENT_TYPE(IDC_REFRESH, -1)
  DECLARE_EVENT_TYPE(IDC_FIRST, -1)
  DECLARE_EVENT_TYPE(IDC_PREV, -1)
  DECLARE_EVENT_TYPE(IDC_NEXT, -1)
  DECLARE_EVENT_TYPE(IDC_LAST, -1)
  DECLARE_EVENT_TYPE(IDC_PROGRESSBAR_STEP, -1)
  DECLARE_EVENT_TYPE(IDC_PROGRESSBAR_EXIT, -1)
  DECLARE_EVENT_TYPE(IDC_UP, -1)
  DECLARE_EVENT_TYPE(IDC_DOWN, -1)
  DECLARE_EVENT_TYPE(IDC_NM_A, -1)
  DECLARE_EVENT_TYPE(IDC_NM_X, -1)
  DECLARE_EVENT_TYPE(IDC_SET_TITLE, -1)
  DECLARE_EVENT_TYPE(IDC_NB_PAGE_DRAGGED, -1)
END_DECLARE_EVENT_TYPES()  
  
#endif