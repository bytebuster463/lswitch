#define _WIN32_WINNT 0x500

#include <windows.h>
#include <tchar.h>

TCHAR	g_prog_dir[MAX_PATH*2];
DWORD	g_prog_dir_len;
HHOOK	g_khook;
HANDLE  g_hEvent;
UINT	g_key=VK_APPS;
UINT	g_key_blocker=0; // If g_key pressed while blocker key is in Down state (pressed), ignore g_key. Typical VK_LSHIFT = A0;

// const TCHAR LAYOUT_UNKNOWN[KL_NAMELENGTH] = _T("00000000");
const TCHAR LAYOUT_ENG[KL_NAMELENGTH] = _T("00000409");
const TCHAR LAYOUT_UKR[KL_NAMELENGTH] = _T("00000422");
#define HKL_UNKNOWN 	((HKL)0x00000000)
#define HKL_ENG 			((HKL)0x00000409)
HKL g_layout_eng 	= HKL_ENG;			// what is considered "ENG" layout, unless overridden
HKL g_layout_last = HKL_UNKNOWN;	// last non-ENG layout

// TCHAR g_layout_last[KL_NAMELENGTH] = _T("00000000"); //LAYOUT_UNKNOWN; //= "00000000";

// #define DEBUG

#ifdef DEBUG
TCHAR dmesg[200];
#endif

LRESULT CALLBACK KbdHook(int nCode,WPARAM wParam,LPARAM lParam) {
	if (nCode<0)
		return CallNextHookEx(g_khook,nCode,wParam,lParam);
	if (nCode==HC_ACTION) {
		KBDLLHOOKSTRUCT   *ks=(KBDLLHOOKSTRUCT*)lParam;
		if (ks->vkCode==g_key) {
			// if blocker+g_Key pressed (e.g. Shift+CapsLock), do nothing
			if(g_key_blocker && (GetAsyncKeyState(g_key_blocker) & 0x8000)) {
				return 0;
			}
			if (wParam==WM_KEYDOWN) {
				HWND hWnd = GetForegroundWindow();
				if (hWnd) {
					DWORD threadId = GetWindowThreadProcessId(hWnd, NULL);
					HKL layout_current = GetKeyboardLayout(threadId);
#ifdef DEBUG
					sprintf(&dmesg, "current=%.8x last=%.8x", layout_current, g_layout_last);
					MessageBox(NULL,dmesg,_T("Debug 1"),MB_OK|MB_ICONINFORMATION);
#endif
					if(NULL != layout_current) {
						// ENG->OTH
						if (LOWORD(layout_current) == LOWORD(g_layout_eng)) {
#ifdef DEBUG
							MessageBox(NULL,"ENG->OTH",_T("Debug 2"),MB_OK|MB_ICONINFORMATION);
#endif
							// ENG->UNK ==> simply change to next
							if (g_layout_last == HKL_UNKNOWN) {
#ifdef DEBUG
								sprintf(&dmesg, "ENG->OTH, last %.8x=UNK, ENG->NEXT", g_layout_last);
								MessageBox(NULL,dmesg,_T("Debug 3"),MB_OK|MB_ICONINFORMATION);
#endif
								PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST,0, (LPARAM)HKL_NEXT);
							// ENG->KNOWN ==> load known
							} else {
								// compute string locale name for last layout
								// IMPORTAMT! LOWORD is crucial to trim off the (high) control part
								TCHAR s_layout_last[KL_NAMELENGTH];
								wsprintf(s_layout_last, _T("%.8x"), LOWORD(g_layout_last));
#ifdef DEBUG
								sprintf(&dmesg, "ENG->OTH, last %.8x=%s, ENG->LAST", g_layout_last, s_layout_last);
								MessageBox(NULL,dmesg,_T("Debug 4"),MB_OK|MB_ICONINFORMATION);
#endif
								HKL layout_new = LoadKeyboardLayout(/* LAYOUT_UKR */s_layout_last, (UINT)KLF_ACTIVATE|KLF_SUBSTITUTE_OK);
								if(NULL != layout_new) {
#ifdef DEBUG
									sprintf(&dmesg, "loaded %s, got %.8x", s_layout_last, layout_new);
									MessageBox(NULL,dmesg,_T("Debug 5"),MB_OK|MB_ICONINFORMATION);
#endif
									PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST,0, (LPARAM)layout_new);
									// HKL layout_new2 = ActivateKeyboardLayout(layout_new, (UINT)KLF_REORDER);
									// sprintf(&dmesg, "activated %.8x", layout_new2);
									// MessageBox(NULL,dmesg,_T("Debug 6"),MB_OK|MB_ICONINFORMATION);
								}
							}
						} else {
							// OTH->ENG
#ifdef DEBUG
							MessageBox(NULL,"OTH->ENG",_T("Debug 14"),MB_OK|MB_ICONINFORMATION);
#endif
							// compute string locale name for English
							TCHAR s_layout_eng[KL_NAMELENGTH];
							wsprintf(s_layout_eng, _T("%.8x"), LOWORD(g_layout_eng));
							HKL layout_new = LoadKeyboardLayout(s_layout_eng, (UINT)KLF_ACTIVATE|KLF_SUBSTITUTE_OK);
							if(NULL != layout_new) {
#ifdef DEBUG
								sprintf(&dmesg, "loaded %s, got %.8x", LAYOUT_ENG, layout_new);
								MessageBox(NULL,dmesg,_T("Debug 15"),MB_OK|MB_ICONINFORMATION);
#endif
								PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST,0, (LPARAM)layout_new);
								// HKL layout_new2 = ActivateKeyboardLayout(layout_new, (UINT)0);
								// sprintf(&dmesg, "activated %.8x", layout_new2);
								// MessageBox(NULL,dmesg,_T("Debug 16"),MB_OK|MB_ICONINFORMATION);

								// and save						
								g_layout_last = layout_current;
								// _tcscpy_s(g_layout_last, sizeof(g_layout_last), lay_cur_str);
								// sprintf(&g_layout_last, L"%.8x", layout_last);
							}
						}
						// GetKeyboardLayout(0);
						// ActivateKeyboardLayout( (HKL)HKL_NEXT, (UINT)KLF_REORDER|KLF_SUBSTITUTE_OK/* win8+ unused |KLF_SETFORPROCESS*/);
//					PostMessage(hWnd,WM_INPUTLANGCHANGEREQUEST,0, (LPARAM)HKL_PREV);
					}
				}
			}
			return 1;
		}
	}

	return CallNextHookEx(g_khook,nCode,wParam,lParam);
}

void  failedx(const TCHAR *msg) {
	MessageBox(NULL,msg,_T("Error"),MB_OK|MB_ICONERROR);
	ExitProcess(1);
}

void  failed(const TCHAR *msg) {
	DWORD		fm;
	TCHAR		*msg1,*msg2;
	const TCHAR	*args[2];

	fm=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,GetLastError(),0,(LPTSTR)&msg1,0,NULL);
	if (fm==0)
		ExitProcess(1);
	args[0]=msg;
	args[1]=msg1;
	fm=FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING|
		FORMAT_MESSAGE_ARGUMENT_ARRAY,
		_T("%1: %2"),0,0,(LPTSTR)&msg2,0,(va_list*)&args[0]);
	if (fm==0)
		ExitProcess(1);
	MessageBox(NULL,msg2,_T("Error"),MB_OK|MB_ICONERROR);
	ExitProcess(1);
}

void CALLBACK TimerCb(HWND hWnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime) {
	if (WaitForSingleObject(g_hEvent,0)==WAIT_OBJECT_0)
		PostQuitMessage(0);
}

int xMain() {
	MSG	  msg;
	DWORD	  sz;
	BOOL	  fQuit=FALSE;
	TCHAR	  *cmd;

	cmd=GetCommandLine();
	sz=lstrlen(cmd);
	if (sz>2 && lstrcmp(cmd+sz-2,_T(" q"))==0)
		fQuit=TRUE;
	else {
		UINT    arg_key = 0;
		ULONG		arg_locale = 0;
		ULONG		arg_blocker = 0;
    // Create the pointer of which
    // the next token is required
    TCHAR 	*ptr;		
		// Skip the program name (first token)
		TCHAR		*token = _tcstok_s(cmd, " ", &ptr);
		// parse hotkey (int)
		if ((token = _tcstok_s(NULL, " ", &ptr)) != NULL) {
			arg_key = _tstoi(token);
#ifdef DEBUG
			sprintf(&dmesg, "token = %s = %d", token, arg_key);
			MessageBox(NULL,dmesg,_T("Debug 21"),MB_OK|MB_ICONINFORMATION);
#endif
			if(arg_key) g_key = arg_key;
		}
		// parse ENG code (long/hex)
		if ((token = _tcstok_s(NULL, " ", &ptr)) != NULL) {
			arg_locale = _tcstol(token, NULL, 16);
#ifdef DEBUG
			sprintf(&dmesg, "token = %s = %d", token, arg_locale);
			MessageBox(NULL,dmesg,_T("Debug 22"),MB_OK|MB_ICONINFORMATION);
#endif
			if(arg_locale) g_layout_eng = (HKL)arg_locale;
		}

		// parse blocker code (long/hex)
		if ((token = _tcstok_s(NULL, " ", &ptr)) != NULL) {
			arg_blocker = _tcstol(token, NULL, 16);
#ifdef DEBUG
			sprintf(&dmesg, "token = %s = %d", token, arg_blocker);
			MessageBox(NULL,dmesg,_T("Debug 22"),MB_OK|MB_ICONINFORMATION);
#endif
			if(arg_blocker) g_key_blocker = arg_blocker;
		}

		// UINT    nk=0;
		// TCHAR   *qq=cmd+sz;

		// while (qq>cmd && qq[-1]>=_T('0') && qq[-1]<=_T('9'))
		// 	--qq;
		// while (*qq)
		// 	nk=nk*10+*qq++-_T('0');
		// if (nk)
		// 	g_key=nk;
	}

	g_hEvent=CreateEvent(NULL,TRUE,FALSE,_T("HaaliLSwitch"));
	if (g_hEvent==NULL)
		failed(_T("CreateEvent()"));
	if (GetLastError()==ERROR_ALREADY_EXISTS) {
		if (fQuit) {
			SetEvent(g_hEvent);
			goto quit;
		}
		failedx(_T("LSwitch is already running!"));
	}

	if (fQuit)
		failedx(_T("LSwitch is not running!"));

	sz=GetModuleFileName(NULL,g_prog_dir,MAX_PATH);
	if (sz==0)
		failed(_T("GetModuleFileName()"));
	if (sz==MAX_PATH)
		failedx(_T("Module file name is too long."));
	while (sz>0 && g_prog_dir[sz-1]!=_T('\\'))
		--sz;
	g_prog_dir_len=sz;

	if (SetTimer(NULL,0,500,TimerCb)==0)
		failed(_T("SetTimer()"));

	g_khook=SetWindowsHookEx(WH_KEYBOARD_LL,KbdHook,GetModuleHandle(0),0);
	if (g_khook==0)
		failed(_T("SetWindowsHookEx()"));

	while (GetMessage(&msg,0,0,0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnhookWindowsHookEx(g_khook);
quit:
	CloseHandle(g_hEvent);

	ExitProcess(0);
}
