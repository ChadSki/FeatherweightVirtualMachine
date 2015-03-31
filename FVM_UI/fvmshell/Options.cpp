//FileZilla - a Windows ftp client

// Copyright (C) 2002-2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// Options.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "options.h"
//#include "filezilla.h"
#include "misc\MarkupSTL.h"
//#include "SpeedLimit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COptions::t_OptionsCache COptions::m_OptionsCache[OPTIONS_NUM];
BOOL COptions::m_bInitialized=FALSE;
BOOL COptions::m_bUseXML=FALSE;
//CCriticalSection COptions::m_Sync;
CMarkupSTL COptions::m_markup;
//SPEEDLIMITSLIST COptions::m_DownloadSpeedLimits;
//SPEEDLIMITSLIST COptions::m_UploadSpeedLimits;
CString COptions::m_sConfigFile;

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld COptions 

COptions::COptions(CWnd* pParent /*=NULL*/)
	//: CSAPrefsDialog(pParent)
{
	//{{AFX_DATA_INIT(COptions)
		// HINWEIS: Der Klassen-Assistent fügt hier Elementinitialisierung ein
	//}}AFX_DATA_INIT
	Init();
}

void COptions::DoDataExchange(CDataExchange* pDX)
{
	CSAPrefsDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptions)
		// HINWEIS: Der Klassen-Assistent fügt hier DDX- und DDV-Aufrufe ein
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COptions, CSAPrefsDialog)
	//{{AFX_MSG_MAP(COptions)
	ON_BN_CLICKED(IDC_PHELP, OnPhelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten COptions 

BOOL COptions::Show()
{
	AddPage(m_OptionsPage2, IDS_OPTIONSPAGE_CONNECTION);
		AddPage(m_OptionsFirewallPage, IDS_OPTIONSPAGE_FIREWALL, &m_OptionsPage2);
		AddPage(m_OptionsProxyPage, IDS_OPTIONSPAGE_PROXY, &m_OptionsPage2);
		AddPage(m_OptionsFtpProxyPage, IDS_OPTIONSPAGE_FTPPROXY, &m_OptionsPage2);
		AddPage(m_OptionsGssPage, IDS_OPTIONSPAGE_GSS, &m_OptionsPage2);
		AddPage(m_OptionsDirCachePage, IDS_OPTIONSPAGE_DIRCACHE, &m_OptionsPage2);
		AddPage(m_OptionsIdentPage, IDS_OPTIONSPAGE_IDENT, &m_OptionsPage2);
		AddPage(m_OptionsSshPage, IDS_OPTIONSPAGE_SSH, &m_OptionsPage2);
	AddPage(m_OptionsTransferPage, IDS_OPTIONSPAGE_TRANSFER);
		AddPage(m_OptionsTypePage, IDS_OPTIONSPAGE_TYPE, &m_OptionsTransferPage);
		AddPage(m_OptionsSpeedLimitPage, IDS_OPTIONSPAGE_SPEEDLIMIT, &m_OptionsTransferPage);
		AddPage(m_OptionsTransferCompressionPage, IDS_OPTIONSPAGE_TRANSFER_COMPRESSION, &m_OptionsTransferPage);
	AddPage(m_OptionsInterfacePage, IDS_OPTIONSPAGE_INTERFACE);
		AddPage(m_OptionsLocalViewPage, IDS_OPTIONSPAGE_LOCALVIEW, &m_OptionsInterfacePage);
		AddPage(m_OptionsRemoteViewPage, IDS_OPTIONSPAGE_REMOTEVIEW, &m_OptionsInterfacePage);
		AddPage(m_OptionsLanguagePage, IDS_OPTIONSPAGE_LANGUAGE, &m_OptionsInterfacePage);
		AddPage(m_OptionsMiscPage, IDS_OPTIONSPAGE_MISC, &m_OptionsInterfacePage);
		AddPage(m_OptionsViewEditPage, IDS_OPTIONSPAGE_VIEWEDIT, &m_OptionsInterfacePage);
		AddPage(m_OptionsPaneLayoutPage, IDS_OPTIONSPAGE_PANELAYOUT, &m_OptionsInterfacePage);
		AddPage(m_OptionsLoggingPage, IDS_OPTIONSPAGE_LOGGING, &m_OptionsInterfacePage);
	AddPage(m_OptionsDebugPage, IDS_OPTIONSPAGE_DEBUG);
		
	
	SetConstantText("FileZilla");

	m_OptionsFtpProxyPage.m_logontype=GetOptionVal(OPTION_LOGONTYPE);
	m_OptionsFtpProxyPage.m_fwhost=GetOption(OPTION_FWHOST);
	m_OptionsFtpProxyPage.m_fwport=GetOptionVal(OPTION_FWPORT);
	m_OptionsFtpProxyPage.m_fwuser=GetOption(OPTION_FWUSER);
	m_OptionsFtpProxyPage.m_fwpass=CCrypt::decrypt(GetOption(OPTION_FWPASS));
	m_OptionsPage2.m_Timeout=GetOptionVal(OPTION_TIMEOUTLENGTH);
	m_OptionsPage2.m_UseKeepAlive=GetOptionVal(OPTION_KEEPALIVE);
	m_OptionsPage2.m_IntervalLow.Format(_T("%d"), GetOptionVal(OPTION_INTERVALLOW));
	m_OptionsPage2.m_IntervalHigh.Format(_T("%d"), GetOptionVal(OPTION_INTERVALHIGH));
	m_OptionsPage2.m_NumRetries.Format(_T("%d"), GetOptionVal(OPTION_NUMRETRIES));
	m_OptionsPage2.m_Delay.Format(_T("%d"), GetOptionVal(OPTION_RETRYDELAY));
	m_OptionsPage2.m_bEnableIPV6 = GetOptionVal(OPTION_ENABLE_IPV6);
	m_OptionsProxyPage.m_Type=GetOptionVal(OPTION_PROXYTYPE);
	m_OptionsProxyPage.m_Host=GetOption(OPTION_PROXYHOST);
	m_OptionsProxyPage.m_Port.Format(_T("%d"), GetOptionVal(OPTION_PROXYPORT));
	m_OptionsProxyPage.m_User=GetOption(OPTION_PROXYUSER);
	m_OptionsProxyPage.m_Pass=CCrypt::decrypt(GetOption(OPTION_PROXYPASS));
	m_OptionsProxyPage.m_Use=GetOptionVal(OPTION_PROXYUSELOGON);

	//Init type page
	m_OptionsTypePage.m_nTypeMode = GetOptionVal(OPTION_TRANSFERMODE);
	m_OptionsTypePage.m_AsciiFiles = GetOption(OPTION_ASCIIFILES);
	
	//Init misc page
	m_OptionsMiscPage.m_nFolderType = GetOptionVal(OPTION_DEFAULTFOLDERTYPE);
	m_OptionsMiscPage.m_DefaultFolder = GetOption(OPTION_DEFAULTFOLDER);
	m_OptionsMiscPage.m_bOpenSitemanagerOnStart = GetOptionVal(OPTION_SHOWSITEMANAGERONSTARTUP);
	m_OptionsMiscPage.m_bSortSitemanagerFoldersFirst = GetOptionVal(OPTION_SORTSITEMANAGERFOLDERSFIRST);
	m_OptionsMiscPage.m_bExpandSitemanagerFolders = GetOptionVal(OPTION_SITEMANAGERNOEXPANDFOLDERS);

	//Init interface page
	m_OptionsInterfacePage.m_bShowToolBar = !GetOptionVal(OPTION_SHOWNOTOOLBAR);
	m_OptionsInterfacePage.m_bShowQuickconnectBar = !GetOptionVal(OPTION_SHOWNOQUICKCONNECTBAR);
	m_OptionsInterfacePage.m_bShowStatusBar = !GetOptionVal(OPTION_SHOWNOSTATUSBAR);
	m_OptionsInterfacePage.m_bShowMessageLog = !GetOptionVal(OPTION_SHOWNOMESSAGELOG);
	m_OptionsInterfacePage.m_bShowLocalTree = !GetOptionVal(OPTION_SHOWNOTREEVIEW);
	m_OptionsInterfacePage.m_bShowRemoteTree = GetOptionVal(OPTION_SHOWREMOTETREEVIEW);
	m_OptionsInterfacePage.m_bShowQueue = !GetOptionVal(OPTION_SHOWNOQUEUE);
	m_OptionsInterfacePage.m_bShowViewLabels = !GetOptionVal(OPTION_SHOWNOLABEL);
	m_OptionsInterfacePage.m_nViewMode = GetOptionVal(OPTION_REMEMBERVIEWS);
	m_OptionsInterfacePage.m_nMinimize  =  GetOptionVal(OPTION_MINIMIZETOTRAY);
	m_OptionsInterfacePage.m_bRememberWindowPos = GetOptionVal(OPTION_REMEMBERLASTWINDOWPOS);

	//Init local view page
	m_OptionsLocalViewPage.m_nViewMode=GetOptionVal(OPTION_REMEMBERLOCALVIEW);
	m_OptionsLocalViewPage.m_nLocalStyle=GetOptionVal(OPTION_LOCALLISTVIEWSTYLE);
	int nLocalHide=GetOptionVal(OPTION_HIDELOCALCOLUMNS);
	m_OptionsLocalViewPage.m_bSize=!(nLocalHide&0x01);
	m_OptionsLocalViewPage.m_bType=!(nLocalHide&0x02);
	m_OptionsLocalViewPage.m_bTime=!(nLocalHide&0x04);
	m_OptionsLocalViewPage.m_bRememberColumnWidths=GetOptionVal(OPTION_REMEMBERLOCALCOLUMNWIDTHS);
	m_OptionsLocalViewPage.m_nSizeFormat=GetOptionVal(OPTION_LOCALFILESIZEFORMAT);
	m_OptionsLocalViewPage.m_bShowStatusBar = GetOptionVal(OPTION_SHOWLOCALSTATUSBAR);
	m_OptionsLocalViewPage.m_bRememberColumnSort = GetOptionVal(OPTION_LOCALCOLUMNSORT) ? TRUE : FALSE;
	m_OptionsLocalViewPage.m_nDoubleclickAction = GetOptionVal(OPTION_LOCAL_DOUBLECLICK_ACTION);
	
	//Init remote view page
	m_OptionsRemoteViewPage.m_nViewMode = GetOptionVal(OPTION_REMEMBERREMOTEVIEW);
	m_OptionsRemoteViewPage.m_nRemoteStyle = GetOptionVal(OPTION_REMOTELISTVIEWSTYLE);
	int nRemoteHide=GetOptionVal(OPTION_HIDEREMOTECOLUMNS);
	m_OptionsRemoteViewPage.m_bSize = !(nRemoteHide&0x01);
	m_OptionsRemoteViewPage.m_bFiletype = !(nRemoteHide&0x02);
	m_OptionsRemoteViewPage.m_bDate = !(nRemoteHide&0x04);
	m_OptionsRemoteViewPage.m_bTime = !(nRemoteHide&0x08);
	m_OptionsRemoteViewPage.m_bPermissions = !(nRemoteHide&0x10);
	m_OptionsRemoteViewPage.m_bOwnerGroup = !(nRemoteHide&0x20);
	m_OptionsRemoteViewPage.m_bRememberColumnWidths = GetOptionVal(OPTION_REMEMBERREMOTECOLUMNWIDTHS);
	m_OptionsRemoteViewPage.m_nSizeFormat = GetOptionVal(OPTION_REMOTEFILESIZEFORMAT);
	m_OptionsRemoteViewPage.m_bShowHidden = GetOptionVal(OPTION_ALWAYSSHOWHIDDEN);
	m_OptionsRemoteViewPage.m_bShowStatusBar = GetOptionVal(OPTION_SHOWREMOTESTATUSBAR);
	m_OptionsRemoteViewPage.m_bRememberColumnSort = GetOptionVal(OPTION_REMOTECOLUMNSORT) ? TRUE : FALSE;
	m_OptionsRemoteViewPage.m_nDoubleclickAction = GetOptionVal(OPTION_REMOTE_DOUBLECLICK_ACTION);
	
	//Init GSS Page
	m_OptionsGssPage.m_bUseGSS=GetOptionVal(OPTION_USEGSS);
	m_OptionsGssPage.m_GssServers=GetOption(OPTION_GSSSERVERS);	

	//Init Debug Page
	m_OptionsDebugPage.m_bEngineTrace=GetOptionVal(OPTION_DEBUGTRACE);
	m_OptionsDebugPage.m_bShowListing=GetOptionVal(OPTION_DEBUGSHOWLISTING);
	m_OptionsDebugPage.m_bDebugMenu=GetOptionVal(OPTION_ENABLEDEBUGMENU);

	//Init firewall page
	m_OptionsFirewallPage.m_bPasv=GetOptionVal(OPTION_PASV);
	m_OptionsFirewallPage.m_bLimitPortRange=GetOptionVal(OPTION_LIMITPORTRANGE);
	m_OptionsFirewallPage.m_PortRangeLow.Format(_T("%d"), GetOptionVal(OPTION_PORTRANGELOW));
	m_OptionsFirewallPage.m_PortRangeHigh.Format(_T("%d"), GetOptionVal(OPTION_PORTRANGEHIGH));
	m_OptionsFirewallPage.m_bUseTransferIP = (GetOption(OPTION_TRANSFERIP)!="") ? TRUE : FALSE;
	m_OptionsFirewallPage.m_TransferIP = GetOption(OPTION_TRANSFERIP);
	m_OptionsFirewallPage.m_bUseTransferIP6 = (GetOption(OPTION_TRANSFERIP6)!="") ? TRUE : FALSE;
	m_OptionsFirewallPage.m_TransferIP6 = GetOption(OPTION_TRANSFERIP6);
	
	//Init Directory Cache page
	m_OptionsDirCachePage.m_bUseCache=GetOptionVal(OPTION_USECACHE)?1:0;
	m_OptionsDirCachePage.m_nHours=GetOptionVal(OPTION_MAXCACHETIME)/3600;
	m_OptionsDirCachePage.m_nMinutes=(GetOptionVal(OPTION_MAXCACHETIME)/60)%60;
	m_OptionsDirCachePage.m_nSeconds=GetOptionVal(OPTION_MAXCACHETIME)%60;

	//Init Transfer page
	m_OptionsTransferPage.m_bPreserveTime=GetOptionVal(OPTION_PRESERVEDOWNLOADFILETIME);
	m_OptionsTransferPage.m_MaxPrimarySize.Format(_T("%d"), GetOptionVal(OPTION_TRANSFERPRIMARYMAXSIZE));
	m_OptionsTransferPage.m_MaxConnCount.Format(_T("%d"), GetOptionVal(OPTION_TRANSFERAPICOUNT));
	m_OptionsTransferPage.m_bUseMultiple=GetOptionVal(OPTION_TRANSFERUSEMULTIPLE);
	m_OptionsTransferPage.m_nFileExistsAction=GetOptionVal(OPTION_FILEEXISTSACTION);
	m_OptionsTransferPage.m_vmsall = GetOptionVal(OPTION_VMSALLREVISIONS);

	//Init View/Edit page
	m_OptionsViewEditPage.m_Default=COptions::GetOption(OPTION_VIEWEDITDEFAULT);
	m_OptionsViewEditPage.m_Custom2=COptions::GetOption(OPTION_VIEWEDITCUSTOM);

	//Init Ident page
	m_OptionsIdentPage.m_bIdent=GetOptionVal(OPTION_IDENT);
	m_OptionsIdentPage.m_bIdentConnect=GetOptionVal(OPTION_IDENTCONNECT);
	m_OptionsIdentPage.m_bSameIP=GetOptionVal(OPTION_IDENTSAMEIP);
	m_OptionsIdentPage.m_System=GetOption(OPTION_IDENTSYSTEM);
	m_OptionsIdentPage.m_UserID=GetOption(OPTION_IDENTUSER);

	//Init Speed Limit page
	m_OptionsSpeedLimitPage.m_DownloadSpeedLimitType=GetOptionVal(OPTION_SPEEDLIMIT_DOWNLOAD_TYPE);
	m_OptionsSpeedLimitPage.m_DownloadValue=GetOptionVal(OPTION_SPEEDLIMIT_DOWNLOAD_VALUE);
	m_OptionsSpeedLimitPage.m_UploadSpeedLimitType=GetOptionVal(OPTION_SPEEDLIMIT_UPLOAD_TYPE);
	m_OptionsSpeedLimitPage.m_UploadValue=GetOptionVal(OPTION_SPEEDLIMIT_UPLOAD_VALUE);
	m_Sync.Lock();
	m_OptionsSpeedLimitPage.Copy(m_DownloadSpeedLimits, m_OptionsSpeedLimitPage.m_DownloadSpeedLimits);
	m_OptionsSpeedLimitPage.Copy(m_UploadSpeedLimits, m_OptionsSpeedLimitPage.m_UploadSpeedLimits);
	m_Sync.Unlock();

	//Init SSH page
	m_OptionsSshPage.m_nCompression = GetOptionVal(OPTION_SSHUSECOMPRESSION);
	m_OptionsSshPage.m_nProtocol = GetOptionVal(OPTION_SSHPROTOCOL);

	//Init pane layout page
	m_OptionsPaneLayoutPage.m_nLocalTreePos = GetOptionVal(OPTION_LOCALTREEVIEWLOCATION);
	m_OptionsPaneLayoutPage.m_nRemoteTreePos = GetOptionVal(OPTION_REMOTETREEVIEWLOCATION);
	m_OptionsPaneLayoutPage.m_bSwitchLayout = GetOptionVal(OPTION_PANELAYOUT_SWITCHLAYOUT);

	//Init message log page
	m_OptionsLoggingPage.m_bLogToFile = GetOptionVal(OPTION_DEBUGLOGTOFILE);
	m_OptionsLoggingPage.m_LogFile = GetOption(OPTION_DEBUGLOGFILE);
	m_OptionsLoggingPage.m_bUseCustomFont = GetOptionVal(OPTION_MESSAGELOG_USECUSTOMFONT);
	m_OptionsLoggingPage.m_FontName = GetOption(OPTION_MESSAGELOG_FONTNAME);
	m_OptionsLoggingPage.m_nFontSize = GetOptionVal(OPTION_MESSAGELOG_FONTSIZE);
	m_OptionsLoggingPage.m_timestamps = GetOptionVal(OPTION_LOGTIMESTAMPS);

	//Init transfer compressio page
	m_OptionsTransferCompressionPage.m_level.Format(_T("%d"), GetOptionVal(OPTION_MODEZ_LEVEL));
	m_OptionsTransferCompressionPage.m_useCompression = GetOptionVal(OPTION_MODEZ_USE);

	InitLanguagePage();
	//Show the dialog
	BOOL res = DoModal();
	if (res != IDOK)
		return FALSE;
	
	SetOption(OPTION_LOGONTYPE,m_OptionsFtpProxyPage.m_logontype);
	SetOption(OPTION_FWHOST,m_OptionsFtpProxyPage.m_fwhost);
	SetOption(OPTION_FWPORT,m_OptionsFtpProxyPage.m_fwport);
	SetOption(OPTION_FWUSER,m_OptionsFtpProxyPage.m_fwuser);
	SetOption(OPTION_FWPASS,CCrypt::encrypt(m_OptionsFtpProxyPage.m_fwpass));
	SetOption(OPTION_TIMEOUTLENGTH,m_OptionsPage2.m_Timeout);
	SetOption(OPTION_KEEPALIVE,m_OptionsPage2.m_UseKeepAlive);
	
	if (m_OptionsPage2.m_UseKeepAlive)
	{
		SetOption(OPTION_INTERVALLOW,_ttoi(m_OptionsPage2.m_IntervalLow));
		SetOption(OPTION_INTERVALHIGH,_ttoi(m_OptionsPage2.m_IntervalHigh));
	}
	SetOption(OPTION_RETRYDELAY,_ttoi(m_OptionsPage2.m_Delay));
	SetOption(OPTION_NUMRETRIES,_ttoi(m_OptionsPage2.m_NumRetries));
	SetOption(OPTION_ENABLE_IPV6, m_OptionsPage2.m_bEnableIPV6);

	//Store proxy settings
	SetOption(OPTION_PROXYTYPE,m_OptionsProxyPage.m_Type);
	SetOption(OPTION_PROXYHOST,m_OptionsProxyPage.m_Host);
	SetOption(OPTION_PROXYPORT,_ttoi(m_OptionsProxyPage.m_Port));
	SetOption(OPTION_PROXYUSER,m_OptionsProxyPage.m_User);
	SetOption(OPTION_PROXYPASS,CCrypt::encrypt(m_OptionsProxyPage.m_Pass));
	SetOption(OPTION_PROXYUSELOGON,m_OptionsProxyPage.m_Use);

	//Store Misc settings
	SetOption(OPTION_DEFAULTFOLDERTYPE,m_OptionsMiscPage.m_nFolderType);
	SetOption(OPTION_DEFAULTFOLDER,m_OptionsMiscPage.m_DefaultFolder);
	SetOption(OPTION_SHOWSITEMANAGERONSTARTUP,m_OptionsMiscPage.m_bOpenSitemanagerOnStart);
	SetOption(OPTION_SORTSITEMANAGERFOLDERSFIRST,m_OptionsMiscPage.m_bSortSitemanagerFoldersFirst);
	SetOption(OPTION_SITEMANAGERNOEXPANDFOLDERS,m_OptionsMiscPage.m_bExpandSitemanagerFolders);

	//Store Transfermode settings
	SetOption(OPTION_TRANSFERMODE,m_OptionsTypePage.m_nTypeMode);
	SetOption(OPTION_ASCIIFILES,m_OptionsTypePage.m_AsciiFiles);

	//Store interface page
	SetOption(OPTION_SHOWNOTOOLBAR,!m_OptionsInterfacePage.m_bShowToolBar);
	SetOption(OPTION_SHOWNOQUICKCONNECTBAR,!m_OptionsInterfacePage.m_bShowQuickconnectBar);
	SetOption(OPTION_SHOWNOSTATUSBAR,!m_OptionsInterfacePage.m_bShowStatusBar);
	SetOption(OPTION_SHOWNOMESSAGELOG,!m_OptionsInterfacePage.m_bShowMessageLog);
	SetOption(OPTION_SHOWNOTREEVIEW,!m_OptionsInterfacePage.m_bShowLocalTree);
	SetOption(OPTION_SHOWREMOTETREEVIEW, m_OptionsInterfacePage.m_bShowRemoteTree);	
	SetOption(OPTION_SHOWNOQUEUE,!m_OptionsInterfacePage.m_bShowQueue);
	SetOption(OPTION_SHOWNOLABEL,!m_OptionsInterfacePage.m_bShowViewLabels);
	SetOption(OPTION_REMEMBERVIEWS,m_OptionsInterfacePage.m_nViewMode);
	SetOption(OPTION_REMEMBERLASTWINDOWPOS,m_OptionsInterfacePage.m_bRememberWindowPos);
	SetOption(OPTION_MINIMIZETOTRAY, m_OptionsInterfacePage.m_nMinimize);
	
	//Store local view page
	SetOption(OPTION_REMEMBERLOCALVIEW,m_OptionsLocalViewPage.m_nViewMode);
	SetOption(OPTION_LOCALLISTVIEWSTYLE,m_OptionsLocalViewPage.m_nLocalStyle);
	
	nLocalHide=0;
	if (!m_OptionsLocalViewPage.m_bSize)
		nLocalHide|=1;
	if (!m_OptionsLocalViewPage.m_bType)
		nLocalHide|=2;
	if (!m_OptionsLocalViewPage.m_bTime)
		nLocalHide|=4;
	SetOption(OPTION_HIDELOCALCOLUMNS,nLocalHide);
	SetOption(OPTION_REMEMBERLOCALCOLUMNWIDTHS, m_OptionsLocalViewPage.m_bRememberColumnWidths);
	SetOption(OPTION_LOCALFILESIZEFORMAT, m_OptionsLocalViewPage.m_nSizeFormat);
	SetOption(OPTION_SHOWLOCALSTATUSBAR, m_OptionsLocalViewPage.m_bShowStatusBar);
	if (m_OptionsLocalViewPage.m_bRememberColumnSort && !GetOptionVal(OPTION_LOCALCOLUMNSORT))
		SetOption(OPTION_LOCALCOLUMNSORT, 1);
	else if (!m_OptionsLocalViewPage.m_bRememberColumnSort && GetOptionVal(OPTION_LOCALCOLUMNSORT))
		SetOption(OPTION_LOCALCOLUMNSORT, 0);
	SetOption(OPTION_LOCAL_DOUBLECLICK_ACTION, m_OptionsLocalViewPage.m_nDoubleclickAction);
	
	//Store remote view page
	SetOption(OPTION_REMEMBERREMOTEVIEW, m_OptionsRemoteViewPage.m_nViewMode);
	SetOption(OPTION_REMOTELISTVIEWSTYLE, m_OptionsRemoteViewPage.m_nRemoteStyle);
	
	nRemoteHide = 0;
	if (!m_OptionsRemoteViewPage.m_bSize)
		nRemoteHide |= 1;
	if (!m_OptionsRemoteViewPage.m_bFiletype)
		nRemoteHide |= 2;
	if (!m_OptionsRemoteViewPage.m_bDate)
		nRemoteHide |= 4;
	if (!m_OptionsRemoteViewPage.m_bTime)
		nRemoteHide |= 8;
	if (!m_OptionsRemoteViewPage.m_bPermissions)
		nRemoteHide |= 0x10;
	if (!m_OptionsRemoteViewPage.m_bOwnerGroup)
		nRemoteHide |= 0x20;
	SetOption(OPTION_HIDEREMOTECOLUMNS, nRemoteHide);
	SetOption(OPTION_REMEMBERREMOTECOLUMNWIDTHS, m_OptionsRemoteViewPage.m_bRememberColumnWidths);
	SetOption(OPTION_REMOTEFILESIZEFORMAT, m_OptionsRemoteViewPage.m_nSizeFormat);
	SetOption(OPTION_ALWAYSSHOWHIDDEN, m_OptionsRemoteViewPage.m_bShowHidden);
	SetOption(OPTION_SHOWREMOTESTATUSBAR, m_OptionsRemoteViewPage.m_bShowStatusBar);
	if (m_OptionsRemoteViewPage.m_bRememberColumnSort && !GetOptionVal(OPTION_REMOTECOLUMNSORT))
		SetOption(OPTION_REMOTECOLUMNSORT, 1);
	else if (!m_OptionsRemoteViewPage.m_bRememberColumnSort && GetOptionVal(OPTION_REMOTECOLUMNSORT))
		SetOption(OPTION_REMOTECOLUMNSORT, 0);
	SetOption(OPTION_REMOTE_DOUBLECLICK_ACTION, m_OptionsRemoteViewPage.m_nDoubleclickAction);
	
	//Store the GSS settings
	SetOption(OPTION_USEGSS,m_OptionsGssPage.m_bUseGSS);
	SetOption(OPTION_GSSSERVERS,m_OptionsGssPage.m_GssServers);

	//Store debug settings
	SetOption(OPTION_DEBUGTRACE, m_OptionsDebugPage.m_bEngineTrace);
	SetOption(OPTION_DEBUGSHOWLISTING, m_OptionsDebugPage.m_bShowListing);
	SetOption(OPTION_ENABLEDEBUGMENU, m_OptionsDebugPage.m_bDebugMenu);

	//Store firewall settings
	SetOption(OPTION_PASV, m_OptionsFirewallPage.m_bPasv);
	SetOption(OPTION_LIMITPORTRANGE, m_OptionsFirewallPage.m_bLimitPortRange);
	SetOption(OPTION_PORTRANGELOW, _ttoi(m_OptionsFirewallPage.m_PortRangeLow));
	SetOption(OPTION_PORTRANGEHIGH, _ttoi(m_OptionsFirewallPage.m_PortRangeHigh));
	SetOption(OPTION_TRANSFERIP, m_OptionsFirewallPage.m_bUseTransferIP ? m_OptionsFirewallPage.m_TransferIP : "");
	SetOption(OPTION_TRANSFERIP6, m_OptionsFirewallPage.m_bUseTransferIP6 ? m_OptionsFirewallPage.m_TransferIP6 : "");

	//Store Directory Cache settings
	SetOption(OPTION_USECACHE,m_OptionsDirCachePage.m_bUseCache);
	SetOption(OPTION_MAXCACHETIME,m_OptionsDirCachePage.m_nHours*3600+m_OptionsDirCachePage.m_nMinutes*60+m_OptionsDirCachePage.m_nSeconds);
	
	//Store Transfer page
	SetOption(OPTION_PRESERVEDOWNLOADFILETIME, m_OptionsTransferPage.m_bPreserveTime);
	int value=_ttoi(m_OptionsTransferPage.m_MaxPrimarySize);
	if (value<0)
		value=1024;
	SetOption(OPTION_TRANSFERPRIMARYMAXSIZE, value);
	value=_ttoi(m_OptionsTransferPage.m_MaxConnCount);
	if (value<0 || value>10)
		value=2;
	SetOption(OPTION_TRANSFERAPICOUNT, value);
	SetOption(OPTION_TRANSFERUSEMULTIPLE, m_OptionsTransferPage.m_bUseMultiple);
	SetOption(OPTION_FILEEXISTSACTION, m_OptionsTransferPage.m_nFileExistsAction);
	SetOption(OPTION_VMSALLREVISIONS, m_OptionsTransferPage.m_vmsall);

	//Store View/Edit page
	COptions::SetOption(OPTION_VIEWEDITDEFAULT, m_OptionsViewEditPage.m_Default);
	COptions::SetOption(OPTION_VIEWEDITCUSTOM, m_OptionsViewEditPage.m_Custom2);

	//Store ident settings
	SetOption(OPTION_IDENT, m_OptionsIdentPage.m_bIdent);
	SetOption(OPTION_IDENTCONNECT, m_OptionsIdentPage.m_bIdentConnect);
	SetOption(OPTION_IDENTSAMEIP, m_OptionsIdentPage.m_bSameIP);
	SetOption(OPTION_IDENTUSER, m_OptionsIdentPage.m_UserID);
	SetOption(OPTION_IDENTSYSTEM, m_OptionsIdentPage.m_System);

	//Store language page
	ProcessLanguagePage();

	//Store Speed Limit page
	SetOption(OPTION_SPEEDLIMIT_DOWNLOAD_TYPE, m_OptionsSpeedLimitPage.m_DownloadSpeedLimitType);
	SetOption(OPTION_SPEEDLIMIT_DOWNLOAD_VALUE, m_OptionsSpeedLimitPage.m_DownloadValue);

	SetOption(OPTION_SPEEDLIMIT_UPLOAD_TYPE, m_OptionsSpeedLimitPage.m_UploadSpeedLimitType);
	SetOption(OPTION_SPEEDLIMIT_UPLOAD_VALUE, m_OptionsSpeedLimitPage.m_UploadValue);

	m_Sync.Lock();
	m_OptionsSpeedLimitPage.Copy( m_OptionsSpeedLimitPage.m_DownloadSpeedLimits, m_DownloadSpeedLimits);
	m_OptionsSpeedLimitPage.Copy( m_OptionsSpeedLimitPage.m_UploadSpeedLimits, m_UploadSpeedLimits);
	m_Sync.Unlock();

	SetOption(OPTION_SPEEDLIMIT_DOWNLOAD_RULES, GetSpeedLimitsString( m_DownloadSpeedLimits));
	SetOption(OPTION_SPEEDLIMIT_UPLOAD_RULES, GetSpeedLimitsString( m_UploadSpeedLimits));

	//Store SSH Page
	SetOption(OPTION_SSHUSECOMPRESSION, m_OptionsSshPage.m_nCompression);
	SetOption(OPTION_SSHPROTOCOL, m_OptionsSshPage.m_nProtocol);

	//Store pane layout page
	SetOption(OPTION_LOCALTREEVIEWLOCATION, m_OptionsPaneLayoutPage.m_nLocalTreePos);
	SetOption(OPTION_REMOTETREEVIEWLOCATION, m_OptionsPaneLayoutPage.m_nRemoteTreePos);
	SetOption(OPTION_PANELAYOUT_SWITCHLAYOUT, m_OptionsPaneLayoutPage.m_bSwitchLayout);

	//Store message log page
	SetOption(OPTION_DEBUGLOGTOFILE, m_OptionsLoggingPage.m_bLogToFile);
	SetOption(OPTION_DEBUGLOGFILE, m_OptionsLoggingPage.m_LogFile);
	SetOption(OPTION_MESSAGELOG_USECUSTOMFONT, m_OptionsLoggingPage.m_bUseCustomFont);
	SetOption(OPTION_MESSAGELOG_FONTNAME, m_OptionsLoggingPage.m_FontName);
	SetOption(OPTION_MESSAGELOG_FONTSIZE, m_OptionsLoggingPage.m_nFontSize);
	SetOption(OPTION_LOGTIMESTAMPS, m_OptionsLoggingPage.m_timestamps);

	//Store transfer compression page
	SetOption(OPTION_MODEZ_USE, m_OptionsTransferCompressionPage.m_useCompression);
	SetOption(OPTION_MODEZ_LEVEL, _ttoi(m_OptionsTransferCompressionPage.m_level));

	return TRUE;
}

BOOL COptions::OnInitDialog() 
{
	CSAPrefsDialog::OnInitDialog();
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

typedef struct
{
	TCHAR name[30];
	int nType;
} t_Option;

static t_Option options[OPTIONS_NUM]={ _T("Logontype"),					1,	_T("Pasv"),								1,
									_T("FWHost"),						0,	_T("FWPort"),							1,
									_T("FWUser"),						0,	_T("FWPass"),							0,
									_T("Timeout Length"),				1,	_T("Use Keep Alive"),					1,
									_T("Interval Low"),					1,	_T("Interval High"),					1,
									_T("Num Retries"),					1,	_T("Retry Delay"),						1,
									_T("Last Server Host"),				0,	_T("Last Server Port"),					1,
									_T("Last Server User"),				0,	_T("Last Server Pass"),					0,
									_T("Proxy Type"),					1,	_T("Proxy Host"),						0,
									_T("Proxy Port"),					1,	_T("Proxy User"),						0,
									_T("Proxy Pass"),					0,	_T("Proxy Use Logon"),					1,
									_T("Last Server Path"),				0,	_T("Last Server Firewall Bypass"),		1,
									_T("Language"),						0,	_T("Default Folder Type"),				1,
									_T("Default Folder"),				0,	_T("Transfer Mode"),					1,
									_T("ASCII files"),					0,	_T("No view labels"),					1,
									_T("No toolbar"),					1,	_T("No Quickconnbar"),					1,
									_T("No Statusbar"),					1,	_T("No Messagelog"),					1,
									_T("No Treeview"),					1,	_T("No Queue"),							1,
									_T("Remember View settings"),		1,	_T("Local List View Style"),			1,
									_T("Hide Local Columns"),			1,	_T("Remote List View Style"),			1,
									_T("Hide Remote Columns"),			1,	_T("Sitemanager on startup"),			1,
									_T("Use GSS"),						1,	_T("GSS enabled servers"),				0,
									_T("Last Server dont save pass"),	1,	_T("Remember Window Size"),				1,
									_T("Last Window Size"),				0,	_T("Debug Trace"),						1,
									_T("Debug Show Listing"),			1,	_T("Debug Log To File"),				1,
									_T("Debug Log File"),				0,	_T("Limit Port Range"),					1,
									_T("Port Range Low"),				1,	_T("Port Range High"),					1,
									_T("Remember Local View"),			1,	_T("Remember Remote View"),				1,
									_T("Last Splitter Pos"),			0,	_T("Use directory cache"),				1,
									_T("Max Dir Cache Time"),			1,	_T("Last Server Type"),					1,
									_T("Minimize to Tray"),				1,	_T("Show Remote Treeview"),				1,
									_T("Remember Local column widths"),	1,	_T("Remember Remote column widths"),	1,
									_T("Local column widths"),			0,	_T("Remote column widths"),				0,
									_T("Local Filesize format"),		1,	_T("Remote Filesize format"),			1,
									_T("Last server name"),				0,	_T("Preserve DL file time"),			1,
									_T("Run in Secure Mode"),			0,	_T("Default View Edit Prog"),			0,
									_T("Custom View Edit Progs"),		0,	_T("Always Show Hidden Files"),			1,
									_T("Primary TransferMax Size"),		1,	_T("Enable debug menu"),				1,
									_T("Transfer Api Count"),			1,	_T("Use Registry"),						1,
									_T("Use multiple connections"),		1,	_T("Use Ident server"),					1,
									_T("Ident when connecting only"),	1,	_T("Ident reply same IP"),				1,
									_T("Ident OS"),						0,	_T("Ident UserID"),						0,
									_T("File exists Action"),			1,	_T("SpeedLimit Download Type"),			1,
									_T("SpeedLimit Upload Type"),		1,	_T("SpeedLimit Download Value"),		1,
									_T("SpeedLimit Upload Value"),		1,	_T("SpeedLimit Download Rules"),		0,
									_T("SpeedLimit Upload Rules"),		0,	_T("SiteManager Folders First"),		1,
									_T("No Expand SiteManager Folders"),1,	_T("Show Local Statusbar"),				1,
									_T("Show Remote Statusbar"),		1,	_T("Local column sort direction"),		1,
									_T("Remote column sort direction"),	1,	_T("SSH Use Compression"),				1,
									_T("SSH Force Protocol"),			1,	_T("Transfer IP"),						0,
									_T("Local doubleclick action"),		1,	_T("Remote doubleclick action"),		1,
									_T("Local treeview location"),		1,	_T("Remote treeview location"),			1,
									_T("Last Server transfer mode"),	1,  _T("Use custom messaglog font"),		1,
									_T("Messagelog font name"),			0,	_T("Messagelog font size"),				1,
									_T("Switch view locations"),		1,	_T("Use MODE Z "),						1,
									_T("MODE Z level"),					1,	_T("Transfer IP v6"),					0,
									_T("Enable IPv6"),					1,	_T("Log Timestamps"),					1,
									_T("VMS display all revisions"),	1
								};

void COptions::SetOption(int nOptionID,int value)
{
	ASSERT(options[nOptionID-1].nType==1);

	Init();
	CSingleLock lock(&m_Sync, TRUE);

	m_OptionsCache[nOptionID-1].bCached=TRUE;
	m_OptionsCache[nOptionID-1].createtime=CTime::GetCurrentTime();
	m_OptionsCache[nOptionID-1].value=value;
	m_OptionsCache[nOptionID-1].nType=1;

	if (m_bUseXML)
	{
		m_markup.ResetPos();
		if (!m_markup.FindChildElem( _T("Settings") ))
			m_markup.AddChildElem( _T("Settings") );

		CString valuestr;
		valuestr.Format( _T("%d"), value);
		m_markup.IntoElem();
		BOOL res=m_markup.FindChildElem();
		while (res)
		{
			CString name=m_markup.GetChildAttrib( _T("name"));
			if (!_tcscmp(name, options[nOptionID-1].name))
			{
				m_markup.SetChildAttrib(_T("name"), options[nOptionID-1].name);
				m_markup.SetChildAttrib(_T("type"), _T("numeric"));
				m_markup.SetChildData(valuestr);
				break;
			}
			res=m_markup.FindChildElem();
		}
		if (!res)
		{
			m_markup.InsertChildElem( _T("Item"), valuestr );
			m_markup.SetChildAttrib(_T("name"), options[nOptionID-1].name);
			m_markup.SetChildAttrib(_T("type"), _T("numeric"));
		}
		m_markup.Save(GetXmlFileName());
	}
	else
	{
		unsigned char tmp[4];
		memcpy(tmp,&value,4);
		HKEY key;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\FileZilla"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, NULL)==ERROR_SUCCESS) 
		{
			RegSetValueEx(key, options[nOptionID-1].name, 0, REG_DWORD, tmp, 4);
			RegCloseKey(key);
		}
	}
}

void COptions::SetOption(int nOptionID, CString value)
{
	ASSERT(!options[nOptionID-1].nType);

	Init();
	CSingleLock lock(&m_Sync, TRUE);

	m_OptionsCache[nOptionID-1].bCached=TRUE;
	m_OptionsCache[nOptionID-1].createtime=CTime::GetCurrentTime();
	m_OptionsCache[nOptionID-1].str=value;
	m_OptionsCache[nOptionID-1].nType=0;
		
	if (m_bUseXML)
	{
		m_markup.ResetPos();
		if (!m_markup.FindChildElem( _T("Settings") ))
			m_markup.AddChildElem( _T("Settings") );
		
		CString str;
		m_markup.IntoElem();
		str=m_markup.GetTagName();
		BOOL res=m_markup.FindChildElem();
		while (res)
		{
			CString name=m_markup.GetChildAttrib( _T("name"));
			if (!_tcscmp(name, options[nOptionID-1].name))
			{
				m_markup.SetChildAttrib(_T("name"), options[nOptionID-1].name);
				m_markup.SetChildAttrib(_T("type"), _T("string"));
				m_markup.SetChildData(value);
				break;
			}
			res=m_markup.FindChildElem();
		}
		if (!res)
		{
			m_markup.InsertChildElem( _T("Item"), value );
			m_markup.SetChildAttrib(_T("name"), options[nOptionID-1].name);
			m_markup.SetChildAttrib(_T("type"), _T("string"));
		}
		m_markup.Save(GetXmlFileName());
	}
	else
	{
		LPTSTR tmp=new TCHAR[value.GetLength()+1];
		LPCTSTR str=value;
		_tcscpy(tmp,str);
		HKEY key;
		if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\FileZilla"), 0, 0, REG_OPTION_NON_VOLATILE, KEY_WRITE, 0, &key, NULL)==ERROR_SUCCESS) 
		{
			RegSetValueEx(key, options[nOptionID-1].name, 0, REG_SZ, (unsigned char *)tmp, (_tcslen(tmp)+1)*sizeof(TCHAR) );
			RegCloseKey(key);
		}
		delete [] tmp;
	}
}

CString COptions::GetOption(int nOptionID)
{
	ASSERT(!options[nOptionID-1].nType);
	
	Init();
	CSingleLock lock(&m_Sync, TRUE);

	CString res = "";
	if (m_OptionsCache[nOptionID-1].bCached)
	{
		ASSERT(!m_OptionsCache[nOptionID-1].nType);
		CTimeSpan span=CTime::GetCurrentTime()-m_OptionsCache[nOptionID-1].createtime;
		if (span.GetTotalSeconds()<120 || m_bUseXML)	
			res=m_OptionsCache[nOptionID-1].str;
		else
			m_OptionsCache[nOptionID-1].bCached=FALSE;
	}
	if (!m_OptionsCache[nOptionID-1].bCached)
	{
		HKEY key;
		unsigned long tmp=2000;
		unsigned char *buffer=new unsigned char[2000];
		BOOL exists = FALSE;
		
		if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\FileZilla"), &key)==ERROR_SUCCESS)
		{
			memset(buffer, 0, 2000);
			
			if (RegQueryValueEx(key, options[nOptionID-1].name, NULL, NULL, buffer, &tmp)==ERROR_SUCCESS) 
			{
				exists=TRUE;
				res=(LPTSTR)buffer;
			}
			RegCloseKey(key);
		}
		
		if (!exists || (!_ttoi(res) && nOptionID==OPTION_RUNINSECUREMODE))
		{
			memset(buffer, 0, 2000);
			if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\FileZilla") ,&key)==ERROR_SUCCESS)
			{
				if (RegQueryValueEx(key, options[nOptionID-1].name, NULL, NULL, buffer, &tmp)==ERROR_SUCCESS)
					res = buffer;
				RegCloseKey(key);
			}
		}
		delete [] buffer;
	}
	if (res == "")
	{
		if (nOptionID == OPTION_ASCIIFILES)
			res = "ASP;BAT;C;CFM;CGI;CONF;CPP;CSS;DHTML;DIZ;H;HPP;HTM;HTML;INC;JS;MAK;NFO;PAS;PATCH;PHP;PHTML;PINERC;PL;QMAIL;SH;SHTML;SQL;TCL;TPL;TXT;VBS;";
		else if (nOptionID == OPTION_GSSSERVERS)
			res = "mit.edu;";
		else if (nOptionID == OPTION_IDENTSYSTEM)
			res = _T("UNIX");
	}
	if (!m_OptionsCache[nOptionID-1].bCached)
	{
		m_OptionsCache[nOptionID-1].bCached=TRUE;
		m_OptionsCache[nOptionID-1].createtime=CTime::GetCurrentTime();
		m_OptionsCache[nOptionID-1].str=res;
		m_OptionsCache[nOptionID-1].nType=0;
	}
	return res;
}

int COptions::GetOptionVal(int nOptionID)
{
	ASSERT(options[nOptionID-1].nType==1);

	Init();
	CSingleLock lock(&m_Sync, TRUE);

	int val=0;
	if (m_OptionsCache[nOptionID-1].bCached)
	{
		ASSERT(m_OptionsCache[nOptionID-1].nType==1);
		CTimeSpan span=CTime::GetCurrentTime()-m_OptionsCache[nOptionID-1].createtime;
		if (span.GetTotalSeconds()<120 || m_bUseXML)
			return m_OptionsCache[nOptionID-1].value;
		else
			m_OptionsCache[nOptionID-1].bCached=FALSE;
	}
	int error=1;
	if (!m_OptionsCache[nOptionID-1].bCached)
	{
		error=1;
		HKEY key;
		unsigned char *buffer = new unsigned char[200];
		unsigned long tmp=100;
				
		if (RegOpenKey(HKEY_CURRENT_USER, _T("Software\\FileZilla") ,&key)==ERROR_SUCCESS)
		{
			DWORD type;
			if (RegQueryValueEx(key, options[nOptionID-1].name, NULL, &type, buffer, &tmp)==ERROR_SUCCESS) 
			{
				if (type == REG_DWORD)
				{
					if (tmp<=4) 
					{
						memcpy(&val,buffer,tmp);
						error=0;
					}
				}
				else
				{
					int i;
					LPTSTR str = (LPTSTR)buffer;
					for (i = 0; i < 100; i++)
					{
						if (!str[i])
							break;
					}
					if (i == 100)
						str[99] = 0;

					val = _ttoi(str);
					error = 0;
				}
			}
			RegCloseKey(key);
		}
		
		if (error)
		{
			memset(buffer,0,100);
			if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("Software\\FileZilla") ,&key)==ERROR_SUCCESS)
			{
				DWORD type;
				if (RegQueryValueEx(key, options[nOptionID-1].name, NULL, &type, buffer, &tmp)==ERROR_SUCCESS)
				{
					if (type == REG_DWORD)
					{
						if (tmp<=4) 
						{
							memcpy(&val,buffer,tmp);
							error=0;
						}
					}
					else
					{
						int i;
						LPTSTR str = (LPTSTR)buffer;
						for (i = 0; i < 100; i++)
						{
							if (!str[i])
								break;
						}
						if (i == 100)
							str[99] = 0;
						
						val = _ttoi(str);
						error = 0;
					}
				}
				RegCloseKey(key);
			}
		}
		delete [] buffer;
	}	
	switch (nOptionID)
	{
		case OPTION_KEEPALIVE:
			if (error)
				val=1;
			break;
		case OPTION_INTERVALLOW:
			if (val>500)
				val=15;
			else if (val<15)
				val=15;
			break;
		case OPTION_INTERVALHIGH:
			if (val>500)
				val=30;
			else if (val<15)
				val=30;
			break;			
		case OPTION_TIMEOUTLENGTH:
			if (val<30)
				val=30;
			else if (val>999)
				val=30;
			break;
		case OPTION_NUMRETRIES:
			if (val<0 || error)
				val=5;
			else if (val>999)
				val=5;
			break;
		case OPTION_RETRYDELAY:
			if (val<0 || error)
				val=5;
			else if (val>999)
				val=5;
			break;
		case OPTION_DEFAULTFOLDERTYPE:
			if (val!=1)
				val=0;
			break;
		case OPTION_TRANSFERMODE:
			if (val<0 || val>2)
				val=0;
			break;
		case OPTION_PORTRANGELOW:
			if (val<1 || val>65535)
				val=1;
			break;
		case OPTION_PORTRANGEHIGH:
			if (val<1 || val>65535)
				val=65535;
			break;
		case OPTION_MAXCACHETIME:
			if (val<=0)
				val=60*30;
			break;
		case OPTION_LOCALFILESIZEFORMAT:
			if (error || val<0 || val >3)
				val=0;
			break;
		case OPTION_REMOTEFILESIZEFORMAT:
			if (error || val<0 || val >3)
				val=1;
			break;
		case OPTION_USECACHE:
			if (error)
				val=1;
			break;
		case OPTION_TRANSFERPRIMARYMAXSIZE:
			if (error || val < 0 || val > 999999)
				val=1024;
			break;
		case OPTION_TRANSFERAPICOUNT:
			if (error || val < 1 || val > 10)
				val=2;
			break;
		case OPTION_TRANSFERUSEMULTIPLE:
			if (error)
				val=1;
			break;
		case OPTION_FILEEXISTSACTION:
			if (val<0 || val>5)
				val=0;
			break;
		case OPTION_SPEEDLIMIT_DOWNLOAD_VALUE:
		case OPTION_SPEEDLIMIT_UPLOAD_VALUE:
			if (error || val<=0)
				val=10;
			break;
		case OPTION_SHOWLOCALSTATUSBAR:
		case OPTION_SHOWREMOTESTATUSBAR:
			if (error)
				val = 1;
			break;
		case OPTION_HIDEREMOTECOLUMNS:
			if (error)
				val = 0x20;
			break;
		case OPTION_SSHUSECOMPRESSION:
		case OPTION_SSHPROTOCOL:
			if (val < 0 || val > 2)
				val = 0;
			break;
		case OPTION_MINIMIZETOTRAY:
			if (val < 0 || val > 2)
				val = 0;
			break;
		case OPTION_LOCAL_DOUBLECLICK_ACTION:
		case OPTION_REMOTE_DOUBLECLICK_ACTION:
			if (val < 0 || val > 2)
				val = 0;
			break;
		case OPTION_LOCALTREEVIEWLOCATION:
		case OPTION_REMOTETREEVIEWLOCATION:
			if (val < 0 || val > 1)
				val = 0;
			break;
		case OPTION_USEREGISTRY:
			if (error || val < 0 || val > 2)
				val = -1;
			break;
		case OPTION_LASTSERVERTRANSFERMODE:
			if (error || val < 0 || val > 2)
				val = 0;
			break;
		case OPTION_MESSAGELOG_FONTSIZE:
			if (error || val < 1 || val > 72)
				val = 0;
			break;
		case OPTION_MODEZ_USE:
			if (error || val < 0 || val > 2)
				val = 2;
			break;
		case OPTION_MODEZ_LEVEL:
			if (error || val < 1 || val > 9)
				val = 8;
			break;
	}
	if (!m_OptionsCache[nOptionID-1].bCached)
	{
		m_OptionsCache[nOptionID-1].bCached = TRUE;
		m_OptionsCache[nOptionID-1].createtime = CTime::GetCurrentTime();
		m_OptionsCache[nOptionID-1].value = val;
		m_OptionsCache[nOptionID-1].nType = 1;
	}

	return val;
}

void COptions::OnOK() 
{
	if (!UpdateData(true))
		return;
	if (!GetCurPage()->UpdateData(TRUE))
		return;
	for (std::list<CSAPrefsSubDlg *>::iterator iter = m_PageList.begin(); iter != m_PageList.end(); iter++)
		if (IsWindow((*iter)->GetSafeHwnd()))
			if (!(*iter)->UpdateData(TRUE))
				return;
	if (m_OptionsPage2.m_UseKeepAlive)
	{
		int low=_ttoi(m_OptionsPage2.m_IntervalLow);
		int high=_ttoi(m_OptionsPage2.m_IntervalHigh);
		if ((low<15)||(low>500))
		{
			ShowPage(&m_OptionsPage2);
			m_OptionsPage2.GetDlgItem(IDC_INTERVALLOW)->SetFocus();
			CString str;
			str.Format(IDS_OPTIONS_KEEPALIVEBORDERLOW,15,500);
			AfxMessageBox(str,MB_ICONEXCLAMATION);
			return;
		}
		if ((high<15)||(high>500))
		{
			ShowPage(&m_OptionsPage2);
			m_OptionsPage2.GetDlgItem(IDC_INTERVALHIGH)->SetFocus();
			CString str;
			str.Format(IDS_OPTIONS_KEEPALIVEBORDERHIGH,15,500);
			AfxMessageBox(str,MB_ICONEXCLAMATION);
			return;
		}
		if (low>high)
		{
			ShowPage(&m_OptionsPage2);
			m_OptionsPage2.GetDlgItem(IDC_INTERVALLOW)->SetFocus();
			CString str;
			str.Format(IDS_OPTIONS_KEEPALIVEBORDERDIFF);
			AfxMessageBox(str,MB_ICONEXCLAMATION);
			return;
		}
	}
	if ((m_OptionsPage2.m_Timeout < 0) || (m_OptionsPage2.m_Timeout > 999))
	{
		ShowPage(&m_OptionsPage2);
		m_OptionsPage2.GetDlgItem(IDC_NUMRETRY)->SetFocus();
		CString str;
		str.Format(IDS_OPTIONS_TIMEOUT, 0, 999);
		AfxMessageBox(str, MB_ICONEXCLAMATION);
		return;			
	}
	
	
	int val=_ttoi(m_OptionsPage2.m_NumRetries);
	if ((val<0)||(val>999))
	{
		ShowPage(&m_OptionsPage2);
		m_OptionsPage2.GetDlgItem(IDC_NUMRETRY)->SetFocus();
		CString str;
		str.Format(IDS_OPTIONS_NUMRETRIES,0,999);
		AfxMessageBox(str,MB_ICONEXCLAMATION);
		return;
			
	}
	
	val=_ttoi(m_OptionsPage2.m_Delay);
	if ((val<0)||(val>999))
	{
		ShowPage(&m_OptionsPage2);
		m_OptionsPage2.GetDlgItem(IDC_DELAY)->SetFocus();
		CString str;
		str.Format(IDS_OPTIONS_RETRYDELAY,0,999);
		AfxMessageBox(str,MB_ICONEXCLAMATION);
		return;
			
	}
	
	if (m_OptionsProxyPage.m_Type)
	{
		if (m_OptionsProxyPage.m_Host=="")
		{
			ShowPage(&m_OptionsProxyPage);
			m_OptionsProxyPage.m_HostCtrl.SetFocus();
			AfxMessageBox(IDS_OPTIONS_PROXYHOST,MB_ICONEXCLAMATION);
			return;
		}
		val=_ttoi(m_OptionsProxyPage.m_Port);
		if (val<=0 || val>=65535)
		{
			ShowPage(&m_OptionsProxyPage);
			m_OptionsProxyPage.m_PortCtrl.SetFocus();
			AfxMessageBox(IDS_OPTIONS_PROXYPORT,MB_ICONEXCLAMATION);
			return;
		}
	}
	
	if (m_OptionsTypePage.m_AsciiFiles=="")
	{
		m_OptionsTypePage.m_AsciiFiles="";
		for (int i=0;i<m_OptionsTypePage.m_cTypeList.GetCount();i++)
		{
			CString tmp;
			m_OptionsTypePage.m_cTypeList.GetText(i,tmp);
			m_OptionsTypePage.m_AsciiFiles+=tmp+";";
		}
		m_OptionsTypePage.m_AsciiFiles.TrimRight( _T(";") );
		m_OptionsTypePage.m_AsciiFiles+=";";
	}
	
	if (m_OptionsGssPage.m_GssServers=="")
	{
		m_OptionsGssPage.m_GssServers="";
		for (int i=0;i<m_OptionsGssPage.m_cServerList.GetCount();i++)
		{
			CString tmp;
			m_OptionsGssPage.m_cServerList.GetText(i,tmp);
			m_OptionsGssPage.m_GssServers+=tmp+";";
		}
		m_OptionsGssPage.m_GssServers.TrimRight( _T(";") );
		m_OptionsGssPage.m_GssServers+=";";
	}

	if (m_OptionsFirewallPage.m_bLimitPortRange)
	{
		int low=_ttoi(m_OptionsFirewallPage.m_PortRangeLow);
		int high=_ttoi(m_OptionsFirewallPage.m_PortRangeHigh);
		if ((low<1)||(low>65534))
		{
			ShowPage(&m_OptionsFirewallPage);
			m_OptionsFirewallPage.m_cPortRangeLow.SetFocus();
			CString str;
			str.Format(IDS_OPTIONS_LIMITPORTRANGELOW,1,65534);
			AfxMessageBox(str,MB_ICONEXCLAMATION);
			return;
		}
		if ((high<2)||(high>65535))
		{
			ShowPage(&m_OptionsFirewallPage);
			m_OptionsFirewallPage.m_cPortRangeLow.SetFocus();
			CString str;
			str.Format(IDS_OPTIONS_LIMITPORTRANGEHIGH,2,65535);
			AfxMessageBox(str,MB_ICONEXCLAMATION);
			return;
		}
		if (low>=high)
		{
			ShowPage(&m_OptionsFirewallPage);
			m_OptionsFirewallPage.m_cPortRangeLow.SetFocus();
			CString str;
			str.Format(IDS_OPTIONS_LIMITPORTRANGEDIFF);
			AfxMessageBox(str,MB_ICONEXCLAMATION);
			return;
		}
	}

	CString str=m_OptionsViewEditPage.m_Custom;
	m_OptionsViewEditPage.m_Custom2="";
	CString line;
	for (int i=0;i<=str.GetLength();i++)
	{
		if (i==str.GetLength() || str[i]=='\r' || str[i]=='\n')
		{
			if (i!=str.GetLength())
			{
				line=str.Left(i);
			
				str=str.Mid(i+1);
				i=-1;
			}
			else
				line=str;

			line.TrimLeft( _T(" ") );
			line.TrimRight( _T(" ") );

			if (line=="")
				continue;

			CString ext;
			int spos=line.Find(_T(" "));
			int qpos=line.Find(_T("\""));
			if (spos==-1)
				continue;
			if (spos<qpos || qpos==-1)
			{
				ext=line.Left(spos);
				ext.Replace( _T(";"), _T("; "));
				line=line.Mid(spos);
				line.TrimLeft( _T(" ") );
				line.Replace( _T(";"), _T("; "));
				if (line=="")
					continue;
				m_OptionsViewEditPage.m_Custom2+=ext + _T(";") + line + _T(";");
				continue;
			}
			else if (qpos!=0)
				continue;
			line=line.Mid(1);
			qpos=line.Find( _T("\"") );
			if (qpos==-1)
				continue;
			
			ext=line.Left(qpos);
			ext.TrimLeft( _T(" ") );
			ext.TrimRight( _T(" ") );
			ext.Replace( _T(";"), _T("; "));
			line=line.Mid(qpos+1);
			line.TrimLeft( _T(" ") );
			line.Replace( _T(";"), _T("; "));
			if (line=="")
				continue;
			m_OptionsViewEditPage.m_Custom2+=ext + _T(";") + line + _T(";");

			continue;
		}
	}
	m_OptionsViewEditPage.m_Custom2.TrimRight( _T(";") );

	if(m_OptionsIdentPage.m_bIdent)
	{
		if(m_OptionsIdentPage.m_UserID.IsEmpty())
		{
			ShowPage(&m_OptionsIdentPage);
			m_OptionsIdentPage.m_cUserID.SetFocus();
			AfxMessageBox(IDS_OPTIONS_IDENTUSERID, MB_ICONEXCLAMATION);
			return;
		}
		
		if(m_OptionsIdentPage.m_System.IsEmpty())
		{
			ShowPage(&m_OptionsIdentPage);
			m_OptionsIdentPage.m_cSystem.SetFocus();
			AfxMessageBox(IDS_OPTIONS_IDENTSYSTEM, MB_ICONEXCLAMATION);
			return;
		}
	}

	CSAPrefsDialog::OnOK();
}

void COptions::InitLanguagePage()
{
	m_OptionsLanguagePage.m_LanguageStringList.insert("English");
	CFileFind find;
	if (find.FindFile(((CFileZillaApp *)AfxGetApp())->m_appPath+"\\*.dll"))
	{
		BOOL bFind=TRUE;
		while(bFind)
		{
			bFind=find.FindNextFile();
			CString fn=find.GetFileName();
			DWORD tmp=0;
			LPTSTR str=new TCHAR[fn.GetLength()+1];
			_tcscpy(str,fn);
			DWORD len=GetFileVersionInfoSize(str,&tmp);
			LPVOID pBlock=new char[len];
			if (GetFileVersionInfo(str,0,len,pBlock))
			{
				LPVOID ptr=0;
				UINT ptrlen;
	
				TCHAR SubBlock[50];
				
				// Structure used to store enumerated languages and code pages.
				struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
				} *lpTranslate;
	
				UINT cbTranslate;
				
				// Read the list of languages and code pages.
				if (VerQueryValue(pBlock, 
							TEXT("\\VarFileInfo\\Translation"),
							(LPVOID*)&lpTranslate,
							&cbTranslate))
				{
					// Read the file description for each language and code page.
				
					_stprintf( SubBlock, 
						_T("\\StringFileInfo\\%04x%04x\\ProductName"),
						lpTranslate[0].wLanguage,
						lpTranslate[0].wCodePage);
					// Retrieve file description for language and code page "0". 
					if (VerQueryValue(pBlock, 
							SubBlock, 
							&ptr, 
								&ptrlen))
					{
						LPTSTR pname=(LPTSTR)ptr;
						CString productname=pname;
						if ( productname==_T("FileZilla Language DLL") )
						{
							_stprintf( SubBlock, 
							_T("\\StringFileInfo\\%04x%04x\\Comments"),
							lpTranslate[0].wLanguage,
							lpTranslate[0].wCodePage);
					
							if (VerQueryValue(pBlock, 
								SubBlock, 
								&ptr, 
								&ptrlen))
							{
								LPTSTR comment=(LPTSTR)ptr;
								if (m_OptionsLanguagePage.m_LanguageStringList.find(comment)==m_OptionsLanguagePage.m_LanguageStringList.end())
									m_OptionsLanguagePage.m_LanguageStringList.insert(comment);			
							}
						}
					}
					
				}
			}
			delete [] str;
			delete [] pBlock;
		}
	}
}

void COptions::ProcessLanguagePage()
{
	unsigned __int64 version=0;
	CString lang=GetOption(OPTION_LANGUAGE);
	if (lang=="")
		lang="English";
	if (lang==m_OptionsLanguagePage.m_selLang)
		return; // No language selected, don't change current one
	if (m_OptionsLanguagePage.m_selLang==_T(""))
		return;
	if (m_OptionsLanguagePage.m_selLang==_T("English"))
	{
		//Set the new language
		SetOption(OPTION_LANGUAGE,"English");
		if (((CFileZillaApp *)AfxGetApp())->m_bLangSet)
			FreeLibrary(AfxGetResourceHandle());
		((CFileZillaApp *)AfxGetApp())->m_bLangSet = FALSE;
		AfxSetResourceHandle(AfxGetInstanceHandle());
		return;
	}

	CFileFind find;
	if (find.FindFile(((CFileZillaApp *)AfxGetApp())->m_appPath+"\\*.dll"))
	{
		BOOL bFind=TRUE;
		while(bFind)
		{
			bFind=find.FindNextFile();
			CString fn=find.GetFileName();
			DWORD tmp=0;
			LPTSTR str=new TCHAR[fn.GetLength()+1];
			_tcscpy(str,fn);
			DWORD len=GetFileVersionInfoSize(str,&tmp);
			LPVOID pBlock=new char[len];
			if (GetFileVersionInfo(str,0,len,pBlock))
			{
				LPVOID ptr=0;
				UINT ptrlen;

				TCHAR SubBlock[50];

				// Structure used to store enumerated languages and code pages.
				struct LANGANDCODEPAGE {
				WORD wLanguage;
				WORD wCodePage;
				} *lpTranslate;

				UINT cbTranslate;

				// Read the list of languages and code pages.
				if (VerQueryValue(pBlock, 
							TEXT("\\VarFileInfo\\Translation"),
							(LPVOID*)&lpTranslate,
							&cbTranslate))
				{
					// Read the file description for each language and code page.

					_stprintf( SubBlock, 
				           _T("\\StringFileInfo\\%04x%04x\\ProductName"),
				           lpTranslate[0].wLanguage,
					       lpTranslate[0].wCodePage);
					// Retrieve file description for language and code page "0". 
					if (VerQueryValue(pBlock, 
						SubBlock, 
						&ptr, 
							&ptrlen))
					{
						LPTSTR pname=(LPTSTR)ptr;
						CString productname=pname;
						if ( productname==("FileZilla Language DLL") )
						{
							_stprintf( SubBlock, 
							_T("\\StringFileInfo\\%04x%04x\\Comments"),
							lpTranslate[0].wLanguage,
							lpTranslate[0].wCodePage);
	
							if (VerQueryValue(pBlock, 
								SubBlock, 
								&ptr, 
								&ptrlen))
							{
								LPTSTR comment=(LPTSTR)ptr;
								CString Comment=comment;
								if (Comment.Find(m_OptionsLanguagePage.m_selLang) != -1)
								{
									if (VerQueryValue(pBlock, _T("\\"), &ptr, &ptrlen))
									{
										VS_FIXEDFILEINFO *fi=(VS_FIXEDFILEINFO*)ptr;
										unsigned __int64 curver=(((__int64)fi->dwFileVersionMS)<<32)+fi->dwFileVersionLS;
										if (curver>version)
										{
											version=curver;
											if (version>=MINVALIDDLLVERSION)
											{
												//Set the new language
												SetOption(OPTION_LANGUAGE,m_OptionsLanguagePage.m_selLang);
												m_OptionsLanguagePage.m_LanguageStringList.insert(comment);
												if (((CFileZillaApp *)AfxGetApp())->m_bLangSet)
													FreeLibrary(AfxGetResourceHandle());
												((CFileZillaApp *)AfxGetApp())->m_bLangSet=TRUE;
		
												HINSTANCE dll=LoadLibrary(fn);
												if (dll)
													AfxSetResourceHandle(dll);
											}
										}
									}
								}
							}
						}
					}
				}
			}
			delete [] str;
			delete [] pBlock;
		}
	}
	if (!version)
		return;
	if (version<MINVALIDDLLVERSION)
		AfxMessageBox(IDS_ERRORMSG_LANGUAGEDLLVERSIONINVALID);
	else if (version<MINDLLVERSION)
		AfxMessageBox(IDS_STATUSMSG_LANGUAGEVERSIONDIFFERENT);
}

CSAPrefsSubDlg* COptions::GetCurPage()
{
	int iPage=m_iCurPage;
	// show the new one
	if ((iPage >= 0) && (iPage < static_cast<int>(m_pages.size())))
	{
		pageStruct *pPS = (pageStruct *)m_pages[iPage];
		ASSERT(pPS);

		if (pPS)
		{
			ASSERT(pPS->pDlg);
			if (pPS->pDlg)
			{
				return pPS->pDlg;
			}
		}
	}
	return NULL;
}

bool COptions::AddPage(CSAPrefsSubDlg &page, UINT nCaptionID, CSAPrefsSubDlg *pDlgParent /*= NULL*/)
{
	CString str;
	str.LoadString(nCaptionID);
	m_PageList.push_back(&page);
	return CSAPrefsDialog::AddPage(page, str, pDlgParent);
}

void COptions::Export()
{
	Init();
	CSingleLock lock(&m_Sync, TRUE);

	CFileDialog dlg(FALSE, _T(".xml"), _T("FileZilla settings"), OFN_OVERWRITEPROMPT, _T("XML files (*.xml)|*.xml||") );
	if (dlg.DoModal()==IDOK)
	{
		CMarkupSTL markup;
		markup.AddElem( _T("FileZilla") );
		markup.AddChildElem( _T("Settings") );
		markup.IntoElem();
		for (int i=0;i<OPTIONS_NUM; i++)
		{
			if (!options[i].nType)
			{
				CString value=GetOption(i+1);
				markup.AddChildElem(_T("Item"), value);
				markup.AddChildAttrib(_T("name"), options[i].name);
				markup.AddChildAttrib(_T("type"), _T("string") );
			}
			else if (options[i].nType==1)
			{
				CString value;
				value.Format(_T("%d"), GetOptionVal(i+1));
				markup.AddChildElem(_T("Item"), value);
				markup.AddChildAttrib(_T("name"), options[i].name);
				markup.AddChildAttrib(_T("type"), _T("numeric") );
			}
		}
		if (!markup.Save(dlg.GetPathName()))
		{
			AfxMessageBox(IDS_ERRORMSG_CANTCREATEFILE, MB_ICONEXCLAMATION);
			return;
		}
		else
			AfxMessageBox(IDS_OPTIONS_EXPORTOK, MB_ICONINFORMATION);
	}
}

void COptions::OnPhelp() 
{
	CString help=AfxGetApp()->m_pszHelpFilePath;
	help+="::/configuration.htm";
		HINSTANCE hDLL=LoadLibrary( _T("hhctrl.ocx") );
	if (hDLL)
	{
		#ifdef UNICODE
			HWND (WINAPI* pHtmlHelp)(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData)=(HWND (WINAPI*)(HWND hwndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData))GetProcAddress(hDLL,"HtmlHelpW");
		#else
			HWND (WINAPI* pHtmlHelp)(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData)=(HWND (WINAPI*)(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData))GetProcAddress(hDLL,"HtmlHelpA");
		#endif

		if (pHtmlHelp)
			pHtmlHelp(m_hWnd, help,HH_DISPLAY_TOC,0);	
	}
}

void COptions::Init()
{
	CSingleLock lock(&m_Sync, TRUE);
	if (m_bInitialized)
		return;
	m_bInitialized=TRUE;

	for (int i=0; i<OPTIONS_NUM; i++)
		m_OptionsCache[i].bCached = FALSE;

	if (GetOptionVal(OPTION_USEREGISTRY) == 2)
		return;

	CFileStatus status;
	if (!CFile::GetStatus(GetXmlFileName(), status))
		return;
	else if (status.m_attribute&FILE_ATTRIBUTE_DIRECTORY)
		return;
	
	if (!status.m_size)
	{
		m_bUseXML = TRUE;
		m_markup.AddElem( _T("FileZilla") );
		return;
	}

	if (m_markup.Load(GetXmlFileName()))
	{
		if (m_markup.FindElem( _T("FileZilla") ))
		{
			if (!m_markup.FindChildElem( _T("Settings") ))
				m_markup.AddChildElem( _T("Settings") );
			
			CString str;
			m_markup.IntoElem();
			str=m_markup.GetTagName();
			while (m_markup.FindChildElem())
			{
				CString value=m_markup.GetChildData();
				CString name=m_markup.GetChildAttrib( _T("name") );
				CString type=m_markup.GetChildAttrib( _T("type") );
				for (int i=0;i<OPTIONS_NUM;i++)
				{
					if (!_tcscmp(name, options[i].name))
					{
						if (m_OptionsCache[i].bCached)
							::AfxTrace( _T("Item '%s' is already in cache, ignoring item\n"), name);
						else
						{
							m_OptionsCache[i].bCached=TRUE;
							if (type=="numeric")
							{
								m_OptionsCache[i].nType=1;
								m_OptionsCache[i].value=_ttoi(value);
							}
							else
							{
								if (type!="string")
									::AfxTrace( _T("Unknown option type '%s' for item '%s', assuming string\n"), type, name);
								m_OptionsCache[i].nType=0;
								m_OptionsCache[i].str=value;
							}
						}
						break;
					}
				}
			}
			m_bUseXML = TRUE;
			return;
		}
	}
	AfxMessageBox(IDS_ERRORMSG_SETTINGS_XMLFILE_INVALID);
}

BOOL COptions::LockXML(CMarkupSTL **pXml)
{
	Init();
	ASSERT(pXml);

	m_Sync.Lock();
	if (!m_bUseXML)
	{
		m_Sync.Unlock();
		return FALSE;
	}

	*pXml=&m_markup;
	return TRUE;
}

void COptions::UnlockXML()
{
	ASSERT (m_bUseXML);
	m_markup.Save(GetXmlFileName());
	m_Sync.Unlock();
}

void COptions::Import()
{
	Init();
	CSingleLock lock(&m_Sync, TRUE);

	CFileDialog dlg(TRUE, _T(".xml"), _T("FileZilla settings"), OFN_FILEMUSTEXIST, _T("XML files (*.xml)|*.xml||") );
	if (dlg.DoModal()==IDOK)
	{
		if (m_markup.Load( dlg.GetPathName() ))
		{
			if (m_markup.FindElem( _T("FileZilla") ))
			{
				if (!m_markup.FindChildElem( _T("Settings") ))
					m_markup.AddChildElem( _T("Settings") );
				
				CString str;
				m_markup.IntoElem();
				str=m_markup.GetTagName();
				while (m_markup.FindChildElem())
				{
					CString value=m_markup.GetChildData();
					CString name=m_markup.GetChildAttrib( _T("name") );
					CString type=m_markup.GetChildAttrib( _T("type") );
					for (int i=0;i<OPTIONS_NUM;i++)
					{
						if (!_tcscmp(name, options[i].name))
						{
							if (type=="numeric")
								SetOption(i+1, _ttoi(value));
							else
							{
								if (type!="string")
									::AfxTrace( _T("Unknown option type '%s' for item '%s', assuming string\n"), type, name);
								SetOption(i+1, value);
							}
							break;
						}
					}
				}
				AfxMessageBox(IDS_OPTIONS_IMPORTOK, MB_ICONINFORMATION);
				return;
			}
		}
		CString str;
		str.Format(IDS_OPTIONS_IMPORTFAILURE, dlg.GetPathName());
		AfxMessageBox(str, MB_ICONEXCLAMATION);
	}
}

bool COptions::InitUDRules()
{
	FillSpeedLimitsListFromString(m_DownloadSpeedLimits, GetOption( OPTION_SPEEDLIMIT_DOWNLOAD_RULES));
	FillSpeedLimitsListFromString(m_UploadSpeedLimits, GetOption( OPTION_SPEEDLIMIT_UPLOAD_RULES));

	return true;
}

void COptions::FillSpeedLimitsListFromString(SPEEDLIMITSLIST &list, CString str)
{
	list.clear();

	while ( str.GetLength() > 0)
	{
		int i = str.Find( '|');
		CString parse;

		if ( i < 0)
		{
			parse = str;
			str.Empty();
		}
		else
		{
			parse = str.Left( i);
			str = str.Mid( i + 1);
		}

		CSpeedLimit *sl = CSpeedLimit::ParseSpeedLimit( parse);

		if ( sl)
			list.push_back(sl);
	}
}

CString COptions::GetSpeedLimitsString(SPEEDLIMITSLIST &list)
{
	CString str;

	CSingleLock lock(&m_Sync, TRUE);

	for (unsigned int i = 0; i < list.size(); i++)
		str += (( i > 0) ? _T("|") : _T("")) + list[i]->GetSpeedLimitString();

	return str;
}

void COptions::ClearStaticOptions()
{
	unsigned int i;
	for (i = 0; i < m_DownloadSpeedLimits.size(); i++)
		delete m_DownloadSpeedLimits[i];

	m_DownloadSpeedLimits.clear();

	for (i = 0; i < m_UploadSpeedLimits.size(); i++)
		delete m_UploadSpeedLimits[i];

	m_UploadSpeedLimits.clear();
}

BOOL COptions::CheckUseXML()
{
	CSingleLock lock(&m_Sync, TRUE);

	Init();

	if (m_bUseXML)
		return TRUE;

	if (GetOptionVal(OPTION_USEREGISTRY) <= 0)
	{
		CFileStatus status;
		BOOL res = CFile::GetStatus(GetXmlFileName(), status);
		if (!res || !status.m_size)
		{
			if (!GetOptionVal(OPTION_USEREGISTRY) ||
				AfxMessageBox(IDS_QUESTION_OPTIONS_FIRSTSTART, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2)==IDNO)
			{
				m_markup.AddElem( _T("FileZilla") );
				m_markup.Save(GetXmlFileName());
				m_bUseXML = TRUE;
				return TRUE;
			}
			else
			{
				SetOption(OPTION_USEREGISTRY, 1);
				return FALSE;
			}
		}
		else
			return TRUE;
	}
	else 
		return FALSE;
}

void COptions::GetKey(HKEY key, CString keyname, CString &value)
{
	unsigned char *buffer=new unsigned char[1000];
	
	unsigned long tmp=1000;
	if (RegQueryValueEx(key, keyname, NULL, NULL, buffer, &tmp)!=ERROR_SUCCESS) 
		value="";
	else 
		value=(LPTSTR)buffer;
	delete [] buffer;
	return;
}

void COptions::GetKey(HKEY key, CString keyname, int &value)
{
	__int64 val2;
	GetKey(key, keyname, val2);
	value=static_cast<int>(val2);
}

void COptions::GetKey(HKEY key, CString keyname, __int64 &value)
{
	unsigned char *buffer=new unsigned char[1000];
	
	unsigned long tmp=1000;
	if (RegQueryValueEx(key,keyname,NULL,NULL,buffer,&tmp)!=ERROR_SUCCESS)
		value=0;
	else 
	{
		if (_tcslen((LPTSTR)buffer)>20)
			value=0;
		else
			value=_ttoi64((LPTSTR)buffer);
	}
	delete [] buffer;
	return;
}

void COptions::SetKey(HKEY key, CString keyname, LPCTSTR value)
{
	RegSetValueEx(key,keyname,0,REG_SZ, (unsigned char *)value, (_tcslen(value)+1)*sizeof(TCHAR) );
}

void COptions::SetKey(HKEY key, CString keyname, __int64 value)
{
	TCHAR tmp[100];
	_stprintf(tmp, _T("%I64d"), value);
	RegSetValueEx(key, keyname, 0, REG_SZ, (unsigned char *)tmp, (_tcslen(tmp)+1)*sizeof(TCHAR) );
}

void COptions::SaveServer(CMarkupSTL *pXML, const t_server &server)
{
	pXML->AddChildElem( _T("Server") );
	pXML->AddChildAttrib( _T("Host"), server.host);
	pXML->AddChildAttrib( _T("Port"), server.port);
	pXML->AddChildAttrib( _T("User"), server.user);
	pXML->AddChildAttrib( _T("Pass"), CCrypt::encrypt(server.pass));
	pXML->AddChildAttrib( _T("FirewallBypass"), server.fwbypass);
	pXML->AddChildAttrib( _T("DontRememberPass"), server.bDontRememberPass);
	pXML->AddChildAttrib( _T("ServerType"), server.nServerType);
	pXML->AddChildAttrib( _T("Path"), server.path);
	pXML->AddChildAttrib( _T("PasvMode"), server.nPasv);
	pXML->AddChildAttrib( _T("TimeZoneOffset"), server.nTimeZoneOffset);
}

void COptions::SaveServer(HKEY key, const t_server &server)
{
	SetKey(key, "Server.Host", server.host);
	SetKey(key, "Server.Port", server.port);
	SetKey(key, "Server.User", server.user);
	SetKey(key, "Server.Pass", CCrypt::encrypt(server.pass));
	SetKey(key, "Firewall bypass", server.fwbypass);
	SetKey(key, "Dont Remember Pass", server.bDontRememberPass);
	SetKey(key, "Server Type", server.nServerType);
	SetKey(key, "Path", server.path);
	SetKey(key, "Pasv Mode", server.nPasv);
	SetKey(key, "Time Zone Offset", server.nTimeZoneOffset);
}

BOOL COptions::LoadServer(CMarkupSTL *pXML, t_server &server)
{
	if (!pXML->FindChildElem( _T("Server") ))
		return FALSE;
	
	server.host = pXML->GetChildAttrib( _T("Host") );
	server.port = _ttoi(pXML->GetChildAttrib( _T("Port") ));
	server.user = pXML->GetChildAttrib( _T("User") );
	server.pass = CCrypt::decrypt(pXML->GetChildAttrib( _T("Pass") ));
	server.fwbypass = _ttoi(pXML->GetChildAttrib( _T("FirewallBypass") ));
	server.bDontRememberPass = _ttoi(pXML->GetChildAttrib( _T("DontRememberPass") ));
	server.nServerType = _ttoi(pXML->GetChildAttrib( _T("ServerType") ));
	server.path = pXML->GetChildAttrib( _T("Path") );
	server.nPasv = _ttoi(pXML->GetChildAttrib( _T("PasvMode") ));
	server.nTimeZoneOffset = _ttoi(pXML->GetChildAttrib( _T("TimeZoneOffset") ));

	if (server.nPasv < 0 || server.nPasv > 2)
		server.nPasv = 0;

	if (server.nTimeZoneOffset < -24 || server.nTimeZoneOffset > 24)
		server.nTimeZoneOffset = 0;

	if (server.host=="" || server.port<1 || server.port>65535)
		return FALSE;
	return TRUE;
}

BOOL COptions::LoadServer(HKEY key, t_server &server)
{
	GetKey(key, "Server.Host", server.host);
	GetKey(key, "Server.Port", server.port);
	GetKey(key, "Server.User", server.user);
	CString tmp;
	GetKey(key, "Server.Pass", tmp);
	server.pass = CCrypt::decrypt(tmp);
	GetKey(key, "Firewall bypass", server.fwbypass);
	GetKey(key, "Dont Remember Pass", server.bDontRememberPass);
	GetKey(key, "Server Type", server.nServerType);
	GetKey(key, "Path", server.path);
	GetKey(key, "Pasv Mode", server.nPasv);
	GetKey(key, "Time Zone Offset", server.nTimeZoneOffset);

	if (server.nPasv < 0 || server.nPasv > 2)
		server.nPasv = 0;

	if (server.nTimeZoneOffset < -24 || server.nTimeZoneOffset > 24)
		server.nTimeZoneOffset = 0;

	if (server.host=="" || server.port<1 || server.port>65535)
		return FALSE;
	return TRUE;

}

void COptions::SetConfig(LPCTSTR pConfigFile)
{
	m_sConfigFile = pConfigFile;
}

CString COptions::GetXmlFileName()
{
	if (m_sConfigFile != "")
		return m_sConfigFile;
	else
		return ((CFileZillaApp *)AfxGetApp())->m_appPath + _T("FileZilla.xml");
}