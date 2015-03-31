// Utility.h : declaration of the Utility Functions
//

#ifndef _UTILITYREG_
#define _UTILITYREG_

#include <string>
#include <vector>
using namespace std;

#define MAX_REG_KEY_NAME		512
#define MAX_REG_KEY_VALUE		32767

struct RegData
{
    LPTSTR	pData;
	DWORD	dwData;
	CString	sData;
};

class RegKeyDetail
{
public:
	CString	sRegKeyName;
	DWORD	dwRegType;
    RegData	m_RegData;
};

void EnumRegistryKey(HKEY hKey, CString sKeyName, vector< CString >& ListEnumKey);
void EnumRegistryKeyValue(HKEY hKey, CString sKeyName, vector< RegKeyDetail >& ListEnumKey);
#endif // _UTILITYREG_
