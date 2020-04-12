// Crypt.cpp : Defines the entry point for the console application.
//

#if __has_include("crypt.h")
#  include "crypt.h"
#else
# include "crypt_template.h"
#endif

#include <windows.h>
#include <iostream>


int main(int argc, char* argv[])
{
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <license file>" << std::endl;
    ::exit(1);
  }

  DWORD ret;
  char res[256];
  unsigned long hash;

  ret = GetPrivateProfileStringA("License", "licensee", "", res, sizeof(res), argv[1]);

  if (ret == 0)
  {
    std::cerr << "Cannot get key \"licensee\" from file" << std::endl;
    exit(1);
  }

  ret = GetPrivateProfileStringA("License", "expires", "", res + ret, sizeof(res) - ret, argv[1]);

  if (ret == 0)
  {
    std::cerr << "Cannot get key \"expires\" from file" << std::endl;
    exit(1);
  }

  hash = crypt(res);

  _ltoa(hash, res, 10);

  WritePrivateProfileStringA("License", "code", res, argv[1]);

  return 0;
}

