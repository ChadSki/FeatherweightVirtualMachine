// Utility.cpp : implementation of the Utility Functions
//
/******************************************************************************
Module:  Utility.cpp
Notices: Written 2004 Vikas Mishra
Purpose: All global function to manages (read operation) registry.
******************************************************************************/

#include "stdafx.h"
#include "fvm.h"

#include "utility.h"

#include <windows.h>

void EnumRegistryKey(HKEY hKey, CString sKeyName, vector< CString >& ListEnumKey)
{
	LONG retcode = ERROR_SUCCESS;
	HKEY hOpenKey = NULL;
	TCHAR str[MAX_PATH];
	memset( str, '\0', sizeof(str));
	ListEnumKey.clear();
	
	//MessageBox(NULL, sKeyName, sKeyName, MB_OK);
	if(sKeyName.GetLength()>0)
        retcode = RegOpenKey( hKey, sKeyName, &hOpenKey);
	else
		hOpenKey = hKey;

	if( retcode != (DWORD)ERROR_SUCCESS )
		return;

	for (int i = 0, retCode = ERROR_SUCCESS; 
            retCode == ERROR_SUCCESS; i++) 
    {
        retCode = RegEnumKey(hOpenKey, 
                     i, 
                     str, 
                     MAX_PATH
					); 

        if (retCode == (DWORD)ERROR_SUCCESS) 
        {
			CString sNewKeyName;
			sNewKeyName = str;
			
			if(sNewKeyName.GetLength()>0)
                ListEnumKey.push_back( sNewKeyName );
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );
}

void EnumRegistryKeyValue(HKEY hKey, CString sKeyName, vector< RegKeyDetail >& ListEnumKey)
{
	LONG retcode = ERROR_SUCCESS;
	HKEY hOpenKey = NULL;
	DWORD dwType = REG_SZ;
	TCHAR str[MAX_REG_KEY_NAME];
	memset( str, '\0', sizeof(str));
	
	if(sKeyName.GetLength()>0)
        retcode = RegOpenKeyEx(hKey, sKeyName, 0, KEY_READ, &hOpenKey);
	else
		hOpenKey = hKey;

	if( retcode != (DWORD)ERROR_SUCCESS )
		return;

	BYTE *data;
	data = new BYTE[MAX_REG_KEY_VALUE];
	memset( data, '\0', sizeof(data));

	ListEnumKey.clear();

	DWORD Size;
	DWORD dwNo = 0;

	RegKeyDetail KeyValue;

	for (int i = 0, retCode = ERROR_SUCCESS; 
            retCode == ERROR_SUCCESS; i++) 
    {
		Size = MAX_REG_KEY_NAME;
		DWORD dwNo = MAX_REG_KEY_VALUE;
        
		retCode = RegEnumValue(hOpenKey,
                     i,
                     str,
                     &Size,
					 NULL,
					 &dwType,
					 data,
					 &dwNo
					);

		if (retCode != (DWORD) ERROR_NO_MORE_ITEMS)// && retCode != ERROR_INSUFFICIENT_BUFFER)
		{
			KeyValue.sRegKeyName = str;
			KeyValue.dwRegType = dwType;

			if(dwType==REG_SZ)
			{
				if( dwNo >0 )
				{
					data[(dwNo-1)] = '\0';
                    KeyValue.m_RegData.sData = (WCHAR *)data;
				}
				else
				{
					KeyValue.m_RegData.sData = "";
				}
			}
			else if(dwType==REG_EXPAND_SZ)
			{
				if( dwNo >0 )
				{
					data[(dwNo-1)] = '\0';
                    KeyValue.m_RegData.sData = (WCHAR *)data;
				}
				else
				{
					KeyValue.m_RegData.sData = "";
				}
			}
			else if(dwType==REG_MULTI_SZ)
			{
				if( dwNo >1 )
				{
					for(int i=0;i<dwNo;i++)
					{
						if( data[i]=='\0' )
						{
                            data[i] = '\r';
						}
					}

					data[(dwNo-1)] = '\0';
					data[(dwNo-2)] = '\0';

                    KeyValue.m_RegData.sData = (WCHAR *)data;
				}
				else
				{
					KeyValue.m_RegData.sData = "";
				}
			}
			else if( (dwType==REG_BINARY)|| (dwType==REG_RESOURCE_LIST) || (dwType==REG_FULL_RESOURCE_DESCRIPTOR) || (dwType==REG_RESOURCE_REQUIREMENTS_LIST) )
			{
				if( dwNo >0 )
				{
					TCHAR temp[4];
					memset(temp, '\0', sizeof(temp));

                    KeyValue.m_RegData.pData = new TCHAR[dwNo*3 + 10];
					memset( KeyValue.m_RegData.pData, '\0', (dwNo*3 + 10) );

					for(int j=0; j<dwNo; j++)
					{
						wsprintf(temp, L"%02X ", (long int)data[j]);
                        lstrcat( KeyValue.m_RegData.pData, temp );
					}
				}
				else
				{
					KeyValue.m_RegData.pData = NULL;
				}
			}
			else if(dwType==REG_DWORD)
			{
				memcpy( &KeyValue.m_RegData.dwData, data, sizeof(DWORD));
			}
			else
			{
				int i = 0;
			}
			
			if(KeyValue.sRegKeyName.GetLength()>0)
                ListEnumKey.push_back( KeyValue );

			retCode = ERROR_SUCCESS;
        }
    }

	if(hKey)
		RegCloseKey( hOpenKey );

	delete[] data;
}