#include "windows.h"
#include "plugin.h"
#include "resource.h"
#include "stdlib.h"
#include "str.h"


//----Typedef's--------------------------

//-----------------Global variables------
HWND mainWnd;
HMODULE hinstt;
void* Addr_DbgBreakPoint;
//---------exports-----------------------

int __stdcall DialogProc(HWND h,int msg,int wparam,int lparam)
{
	if(msg==WM_INITDIALOG)
	{
		HWND hedit=GetDlgItem(h,IDC_EDIT);
		SetFocus(hedit);
	}
	else if( (msg==WM_COMMAND) && ((wparam&0xFFFF)==IDOK) )
	{
			char buffer[11]={0};
			HWND hedit=GetDlgItem(h,IDC_EDIT);
			GetWindowText(hedit,buffer,11);
			unsigned long pid=atol(buffer);

			HANDLE hProcess=OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_WRITE,FALSE,pid);
			if(!hProcess)
			{
				MessageBox(0,"Can't open target process","SilentAttach",0);
				goto END;
			}
			else
			{
				unsigned char V=0x90; //Nop
				unsigned long Written;
				if(!WriteProcessMemory(hProcess,Addr_DbgBreakPoint,&V,1,&Written))
				{
					MessageBox(0,"Can't write to target process","SilentAttach",0);
					goto END;
				}
			}

			if(Attachtoactiveprocess(pid)==-1) MessageBox(0,"Unable to attach","SilentAttach",0);
END:
			if(hProcess) CloseHandle(hProcess);
			EndDialog(h,0);
			return true;
	}
	else if(msg==WM_CLOSE)
	{
		EndDialog(h,0);
		return true;
	}
	return false;
}



extern "C"
{
_declspec(dllexport)  int __cdecl ODBG_Plugindata(char shortname[32]) 
{
    strcpy(shortname,"SilentAttach");       // Name of plugin
    return 110;   //version 1.10
}

_declspec(dllexport) int __cdecl ODBG_Plugininit(int ollydbgversion,HWND hw,ulong *features) 
{
  mainWnd=hw;
  Addtolist(0,0,"SilentAttach");
  Addtolist(0,-1,"started successfully");
  return 0;
}

_declspec(dllexport) int __cdecl ODBG_Pluginmenu(int origin,char data[4096],void *item)
{
   if(origin==PM_MAIN)
   {
		strcpy(data,"0 &SilentAttach\tAlt+F12");// plug.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#define _CHAR_UNSIGNED
		return 1;
   }
   return 0;
}

_declspec(dllexport) int __cdecl ODBG_Pluginshortcut(int origin,int ctrl,int alt,int shift,int key,void *item) 
{
  if (ctrl==0 && alt==1 && shift==0 && key==VK_F12) 
  {
        DialogBox(hinstt,MAKEINTRESOURCE(IDD_DLG),mainWnd,(DLGPROC)&DialogProc);
        return 1; 
  }                       // Shortcut recognized
  return 0;                            // Shortcut not recognized
}

_declspec(dllexport) void __cdecl ODBG_Pluginaction(int origin,int action,void *item) 
{
  //unsigned long value=0;
  if(origin==PM_MAIN) 
  {
	  if(action==0) DialogBox(hinstt,MAKEINTRESOURCE(IDD_DLG),mainWnd,(DLGPROC)&DialogProc);
  }
}

}
//------------------------------
BOOL APIENTRY Dllmain(HMODULE hModule,int reason,LPVOID lpReserved)
{
	if(reason==DLL_PROCESS_ATTACH)
	{
		hinstt=hModule;
		Addr_DbgBreakPoint=(void*)GetProcAddress(GetModuleHandle("ntdll.dll"),"DbgBreakPoint");
	}
    return TRUE;
}
