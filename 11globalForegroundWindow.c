/**
Max object: OS-wide foreground window get / set
-- 11oLsen.de
*/
 
#include "ext.h"							
#include "ext_obex.h"						

#include <tlhelp32.h>
#include <dwmapi.h> // add dwmapi.lib at linker additional dependencies



////////////////////////// object struct
typedef struct _globalForegroundWindow 
{
	t_object					ob;			// the object itself (must be first)
	t_symbol	*name;

	void *outa; //outlets
	void *outb;
	void *outc;
	void *outd;
	void *outdump;

	t_systhread		x_systhread;						// thread reference
	t_systhread_mutex	x_mutex;							// mutual exclusion lock for threadsafety
	int				x_systhread_cancel;					// thread cancel flag
	void				*x_qelem;							// for message passing between threads
	void			*clock;
	int				x_outputmode;
	int				x_mask;
	int				x_rot;
	//int				consume;

	// Thread and hook handles.
	DWORD hook_thread_id;
	HWINEVENTHOOK win_event_hhook;

	
	t_atom winLoc[4];

	int screenCount;

} t_globalForegroundWindow;



///////////////////////// function prototypes

void	*globalForegroundWindow_new(t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_free(t_globalForegroundWindow *x);
void	globalForegroundWindow_assist(t_globalForegroundWindow *x, void *b, long m, long a, char *s);

void	globalForegroundWindow_bang(t_globalForegroundWindow *x);
void	globalForegroundWindow_do_bang(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_cons(t_globalForegroundWindow *x, long m);
void	globalForegroundWindow_start(t_globalForegroundWindow *x);
void	globalForegroundWindow_stop(t_globalForegroundWindow *x);
void	*globalForegroundWindow_threadproc(t_globalForegroundWindow *x);
//void	*dispatch_proc(uiohook_event * const event);

t_globalForegroundWindow	*thisobject();
void	globalForegroundWindow_int(t_globalForegroundWindow *x, long m);
void	unregister_running_hooks();
void CALLBACK win_hook_event_proc(HWINEVENTHOOK hook, DWORD event, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

void	globalForegroundWindow_do_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_getwindows(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_getwindows(t_globalForegroundWindow *x);

void	globalForegroundWindow_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_setWithNum(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);

void	globalForegroundWindow_isrunning(t_globalForegroundWindow* x, t_symbol* s, long argc, t_atom* argv);
void	globalForegroundWindow_do_isrunning(t_globalForegroundWindow* x, t_symbol* s, long argc, t_atom* argv);

void	globalForegroundWindow_do_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_minimize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_minimize(t_globalForegroundWindow *x);
void	globalForegroundWindow_do_close(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_close(t_globalForegroundWindow *x);

void	globalForegroundWindow_do_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_do_getscreens(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_getscreens(t_globalForegroundWindow *x);

void	globalForegroundWindow_do_maximize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_maximize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);

void	globalForegroundWindow_test(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void	globalForegroundWindow_testoutput(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);

BOOL CALLBACK	globalForegroundWindow_enumWindowCallback(HWND hWnd, LPARAM lparam);
BOOL CALLBACK	globalForegroundWindow_closeWindowsWithPidCallback(HWND hWnd, LPARAM lparam);
BOOL CALLBACK	globalForegroundWindow_closeWindowsWithProcNameCallback(HWND hWnd, LPARAM lparam);
BOOL CALLBACK	globalForegroundWindow_theMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
BOOL CALLBACK	globalForegroundWindow_setWindowWithProcNameCallback(HWND hWnd, LPARAM lparam);
BOOL CALLBACK	globalForegroundWindow_setWindowWithProcNameAndTitleCallback(HWND hWnd, LPARAM lparam);
BOOL CALLBACK	globalForegroundWindow_findWindowWithProcNameCallback(HWND hWnd, LPARAM lparam);

//Helper
void	globalForegroundWindow_activateWindow(t_globalForegroundWindow *x, HWND hWnd);
int		globalForegroundWindow_getPathAndFilenameForPid(DWORD dwPID, char **path, char **filename);
int		globalForegroundWindow_TerminateProcessWithPid(DWORD ProcessId);

int		globalForegroundWindow_storeBgWindow(t_globalForegroundWindow *x, DWORD dwPID, HWND hWnd);
void	globalForegroundWindow_resetBgWindowWithHwnd(t_globalForegroundWindow *x, HWND hWnd);
void	globalForegroundWindow_resetBgWindowsWithPid(t_globalForegroundWindow *x, DWORD dwPID);
void    globalForegroundWindow_resetBgWindowsWithProcName(t_globalForegroundWindow *x, char* appNameToMatch);
void	globalForegroundWindow_resetAllBgWindows(t_globalForegroundWindow *x);

int		globalForegroundWindow_storeFsWindow(t_globalForegroundWindow *x, HWND hWnd);
void	globalForegroundWindow_resetFsWindowWithHwnd(t_globalForegroundWindow *x, HWND hWnd);
void	globalForegroundWindow_resetAllFsWindows(t_globalForegroundWindow *x);

int		globalForegroundWindow_getRectOfMinimizedWindow(HWND hWnd, RECT* rectPtr);
int		globalForegroundWindow_getFullscreenRectForWindow(HWND hWnd, RECT * rectPtr);
int		globalForegroundWindow_windowTool(t_globalForegroundWindow *x, HWND hWnd, int CaSe);

//////////////////////// global class pointer variable
void *globalForegroundWindow_class;


t_globalForegroundWindow *t_globalForegroundWindow_temp[256]; // object pointer
DWORD globalForegroundWindow_thread_ids[256]; // thread ids

// need  Pid+window+oldParent to store the bg (topmost -1) windows
DWORD bgWndPids[256];
HWND  bgWndHandles[256];
HWND  bgWndParentHandles[256];

// fullscreen windows
HWND  fsWndHandles[256];
DWORD fsWndStyles[256];

// The handle to the DLL module pulled in DllMain on DLL_PROCESS_ATTACH.
HINSTANCE hInst;

static t_symbol	*symNameChange;
static t_symbol	*emptySym;
static t_symbol	*SYMnoProcName;
static t_symbol	*SYMnoTitle;

#pragma region MAIN and New

void ext_main(void *r)
{	
	t_class *c;
	int i;

	c = class_new("11globalForegroundWindow", (method)globalForegroundWindow_new, (method)globalForegroundWindow_free, 
													(long)sizeof(t_globalForegroundWindow), 0L /* leave NULL!! */, A_GIMME, 0);
	
    class_addmethod(c, (method)globalForegroundWindow_assist,	"assist",		A_CANT, 0); /* you CAN'T call this from the patcher */ 
	class_addmethod(c, (method)globalForegroundWindow_bang, "bang", 0);
	class_addmethod(c, (method)globalForegroundWindow_int, "int", A_LONG, 0);	// the method for an int in the right inlet (inlet 1)
	
	class_addmethod(c, (method)globalForegroundWindow_getscreens, "getscreens", 0);
	class_addmethod(c, (method)globalForegroundWindow_getwindows, "getwindows", 0);
	class_addmethod(c, (method)globalForegroundWindow_minimize, "minimize", 0);
	class_addmethod(c, (method)globalForegroundWindow_close, "close", 0);

	class_addmethod(c, (method)globalForegroundWindow_isrunning, "isrunning", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_set, "set", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_size, "size", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_pos, "pos", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_topmost, "topmost", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_fullscreen, "fullscreen", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_killapp, "killapp", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_quitapp, "quitapp", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_agent, "agent", A_GIMME, 0);

	class_addmethod(c, (method)globalForegroundWindow_maximize, "maximize", A_GIMME, 0);
	class_addmethod(c, (method)globalForegroundWindow_test, "test", A_GIMME, 0);
	CLASS_METHOD_ATTR_PARSE(c, "test", "undocumented", gensym("long"), 0, "1");
	CLASS_METHOD_ATTR_PARSE(c, "agent", "undocumented", gensym("long"), 0, "1");

	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	globalForegroundWindow_class = c;


	
	/*for (i = 0; i < 256; i++)
	{
		t_globalForegroundWindow_temp[i] = NULL;
		globalForegroundWindow_thread_ids[i] = NULL;
		
	}*/

	symNameChange = gensym("namechange");
	emptySym = gensym("");
	SYMnoProcName = gensym("<noProcName>");
	SYMnoTitle = gensym("<noTitle>");

	object_post(NULL,"11globalForegroundWindow 2022/03/10 11OLSEN.DE");

	return 0;
}



void *globalForegroundWindow_new(t_symbol *s, long argc, t_atom *argv)									// NEW //////////////////////////
{
	t_globalForegroundWindow *x = NULL;

	if (x = (t_globalForegroundWindow *)object_alloc(globalForegroundWindow_class))
	{
		
		x->x_systhread = NULL;
		//systhread_mutex_new(&x->x_mutex, 0);

		x->outdump = outlet_new(x, NULL);
		x->outd = outlet_new(x, NULL);
		x->outc = outlet_new(x, NULL);
		x->outb = outlet_new(x, NULL);
		x->outa = listout(x);

		// Thread and hook handles.
		x->hook_thread_id = 0;
		x->win_event_hhook = NULL;

		x->name = s;
		x->x_qelem = qelem_new(x, (method)globalForegroundWindow_do_bang);
	}
	return (x);
}
#pragma endregion


#pragma region BANG 

void globalForegroundWindow_bang(t_globalForegroundWindow *x)
{
	defer_low(x, (method)globalForegroundWindow_do_bang, NULL, 0, NULL);
}

void globalForegroundWindow_do_bang(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	
	//HWND
	HWND hWnd = GetForegroundWindow(); // get handle of currently active window;
	int length = 0;
	LPWSTR wnd_title = NULL;
	length = GetWindowTextLengthW(hWnd); // length of title
	length++;// +1 for termination

	

	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	char *path = NULL;
	char *filename = NULL; // without extension

	int success = globalForegroundWindow_getPathAndFilenameForPid(dwPID, &path, &filename);

	if (!success)
	{
		outlet_anything(x->outd, SYMnoProcName, 0, NULL);
		outlet_anything(x->outc, SYMnoProcName, 0, NULL);
	}
	else
	{
		//object_post((t_object*)x, "filename: %s path: %s", filename, path);
		outlet_anything(x->outd, gensym(filename), 0, NULL);
		outlet_anything(x->outc, gensym(path), 0, NULL);
	}
	if (filename)	sysmem_freeptr(filename);
	if (path)		sysmem_freeptr(path);
	
	
	
	//if (wnd_title) free(wnd_title); wnd_title = NULL;

	wnd_title = (wchar_t *)sysmem_resizeptr(wnd_title, length*sizeof(wchar_t));
	

	// get window title
	if (!GetWindowTextW(hWnd, wnd_title, length))
	{
		outlet_anything(x->outb, emptySym, 0, NULL);
	}
	else
	{
		int i;
		char *maxConform = charset_unicodetoutf8(wnd_title, wcslen(wnd_title), NULL);

		// replace all backslashes in title
		for (i = 0; i <= strlen(maxConform); i++)
		{
			if (maxConform[i] == '\\')
			{
				maxConform[i] = '/';
			}
		}

		outlet_anything(x->outb, gensym(maxConform), 0, NULL);
		sysmem_freeptr(maxConform);
	}
	if (wnd_title) sysmem_freeptr(wnd_title);


	//get window rect
	RECT myrect;
	RECT *rect = &myrect;
	GetWindowRect(hWnd, rect);
	atom_setlong(x->winLoc + 0, rect->left);
	atom_setlong(x->winLoc + 1, rect->top);
	atom_setlong(x->winLoc + 2, rect->right - rect->left);
	atom_setlong(x->winLoc + 3, rect->bottom - rect->top);
	outlet_list(x->outa, NULL, 4, x->winLoc);
	//object_post((t_object*)x, "%i %i %i %i ", rect->left, rect->right, rect->top, rect->bottom);
}
#pragma endregion 


#pragma region SIZE

void globalForegroundWindow_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_size, s, argc, argv);
}

void globalForegroundWindow_do_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	int success = false;


	if (argc != 2)
	{
		object_error((t_object*)x, "you need 2 integer arguments");
		return;
	}

	//HWND // get handle of currently active window
	HWND hWnd = GetForegroundWindow();

	success = SetWindowPos(hWnd, NULL, 0, 0, atom_getlong(argv + 0), atom_getlong(argv + 1), SWP_ASYNCWINDOWPOS | SWP_NOMOVE | SWP_NOZORDER);
	if (!success)
		object_error((t_object*)x, "Error setting window size!");

}
#pragma endregion 


#pragma region POSITION

void globalForegroundWindow_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_pos, s, argc, argv);
}

void globalForegroundWindow_do_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	int success = false;
	
	if (argc != 2)
	{
		object_error((t_object*)x, "you need 2 integer arguments");
		return;
	}

	POINT point;
	point.x = atom_getlong(argv + 0);
	point.y = atom_getlong(argv + 1);

	HWND hWnd = GetForegroundWindow();
	HWND parent =  GetAncestor(hWnd, GA_PARENT); // GetParent(hWnd);      //
	HWND desktop = GetDesktopWindow();

	//check if window is child of another window
	if (parent != desktop)
	{
		//object_post((t_object*)x, "positioning a child window");
		MapWindowPoints(desktop, parent, (LPPOINT)&point, 1);

		success = SetWindowPos(hWnd, NULL, point.x, point.y, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER);
		if (!success)
			object_error((t_object*)x, "Error setting window position!");
	}
	else
	{
		success = SetWindowPos(hWnd, NULL, point.x, point.y, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER);
		if (!success)
			object_error((t_object*)x, "Error setting window position!");
	}
}

#pragma endregion


#pragma region GETWINDOWS
/*
getting real path does not work with the newer modern/universal apps because it returns the name of a helper process
WWAHost.exe on Windows 8 and ApplicationFrameHost.exe on Windows 10 rather than the name of the app
https://android.developreference.com/article/17892596/Name+of+process+for+active+window+in+Windows+8+10
*/

void globalForegroundWindow_getwindows(t_globalForegroundWindow *x)
{
	defer_low(x, (method)globalForegroundWindow_do_getwindows, NULL, NULL, NULL);
}

void globalForegroundWindow_do_getwindows(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	outlet_anything(x->outdump, gensym("getwindows"), 0, NULL);

	EnumWindows(globalForegroundWindow_enumWindowCallback, x);

	outlet_anything(x->outdump, gensym("getwindows_end"), 0, NULL);
}

BOOL CALLBACK globalForegroundWindow_enumWindowCallback(HWND hWnd, LPARAM lparam)
{
	
	t_globalForegroundWindow *x = (t_globalForegroundWindow *)lparam;
	t_atom result[8];
	int i;
	
	
	// STATE
	if (!IsWindowVisible(hWnd))
	{
			return TRUE; // we don't list hidden windows
	}
	else if (IsIconic(hWnd))
			atom_setlong(result + 7, 0); // 0 minimized

	else if (IsZoomed(hWnd))
			atom_setlong(result + 7, 2); // 2 fullscreen
	else
	{
		atom_setlong(result + 7, 1); // 1 visible normal
	}


	// window class name
	/*LPWSTR lpClassName = (wchar_t *)calloc(1024, sizeof(wchar_t));

	if (GetClassNameW(hWnd, lpClassName, 1024) )
	{
		char *maxConform = charset_unicodetoutf8((unsigned short*)lpClassName, wcslen(lpClassName), NULL);
		atom_setsym(result + 3, gensym(maxConform)); 
		sysmem_freeptr(maxConform);
	}
	else
	{
		atom_setsym(result + 3, gensym("<noClassName>") );
	}
	free(lpClassName);*/


	// ID
	//if(id=GetWindowLongPtr(hWnd, GWL_ID))
	long id = (long)hWnd;
	atom_setlong(result + 0, id);
	

	// TITLE
	int length = GetWindowTextLengthW(hWnd);
	if (length)
	{
		length++;// +1 for terminator
		wchar_t *buffer = (wchar_t *)sysmem_newptrclear(length*sizeof(wchar_t)); //newptr

		GetWindowTextW(hWnd, buffer, length);

		char *maxConform = charset_unicodetoutf8(buffer, wcslen(buffer), NULL);
		sysmem_freeptr(buffer);

		// replace all backslashes in title
		for (i = 0; i <= strlen(maxConform); i++)
		{
			if (maxConform[i] == '\\')
			{
				maxConform[i] = '/';
			}
		}
		
		atom_setsym(result + 2, gensym(maxConform));
		sysmem_freeptr(maxConform);
		
	}else
		atom_setsym(result + 2, SYMnoTitle);


	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);


	// PROCESS NAME
	char *path = NULL;
	char *filename = NULL; // without extension

	int success = globalForegroundWindow_getPathAndFilenameForPid(dwPID, &path, &filename); // newptr

	if (success)
	{
		atom_setsym(result + 1, gensym(filename));
	}
	else
		atom_setsym(result + 1, SYMnoProcName);

	if (filename)	sysmem_freeptr(filename);
	if (path)		sysmem_freeptr(path);


	// WINDOW COORDINATES
	RECT thisWndRect = { 0, 0, 0, 0 };
	
	if (IsIconic(hWnd))
	{
		// special case: window is minimized 
		globalForegroundWindow_getRectOfMinimizedWindow(hWnd, &thisWndRect);
	}
	else
	{
		GetWindowRect(hWnd, &thisWndRect);
	}

	atom_setlong(result + 3, thisWndRect.left);
	atom_setlong(result + 4, thisWndRect.top);
	atom_setlong(result + 5, thisWndRect.right - thisWndRect.left);
	atom_setlong(result + 6, thisWndRect.bottom - thisWndRect.top);
		
	//CopyRect(dst,src);
		
	
	// now output all the infos for this window as a list
	outlet_list( x->outdump, NULL, 8, result);
	
	return TRUE;
}


#pragma endregion



#pragma region GETSCREENS

void globalForegroundWindow_getscreens(t_globalForegroundWindow *x)
{
	defer_low(x, (method)globalForegroundWindow_do_getscreens, NULL, NULL, NULL);
}

BOOL CALLBACK globalForegroundWindow_theMonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
	t_globalForegroundWindow *x = (t_globalForegroundWindow *)dwData;
	t_atom as[6];
	RECT desktopRect;
	MONITORINFO  moninfo;
	moninfo.cbSize = sizeof(MONITORINFO); // !!

	x->screenCount++;

	// this always returns the primary screen rect
	// GetWindowRect(GetDesktopWindow(), &desktopRect);

	GetMonitorInfo(hMonitor, &moninfo);
	
	if (moninfo.dwFlags == MONITORINFOF_PRIMARY)
	{
		atom_setlong(as + 5, 1);
	}
	else
		atom_setlong(as + 5, 0);

	atom_setlong(as + 0, x->screenCount);
	atom_setlong(as + 1, lprcMonitor->left);
	atom_setlong(as + 2, lprcMonitor->top);
	atom_setlong(as + 3, lprcMonitor->right - lprcMonitor->left);
	atom_setlong(as + 4, lprcMonitor->bottom - lprcMonitor->top);


	// output
	outlet_list(x->outdump, NULL, 6, as);
	
	return TRUE;
}


void globalForegroundWindow_do_getscreens(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	x->screenCount = 0;

	//signal start of list
	outlet_anything(x->outdump, gensym("getscreens"), 0, NULL);

	EnumDisplayMonitors(NULL, NULL, globalForegroundWindow_theMonitorEnumProc, x);

	//signal end of list
	outlet_anything(x->outdump, gensym("getscreens_end"), 0, NULL);

	return;

}

#pragma endregion


#pragma region SET


struct appNameAndTitle {
	char * appNameToMatch;
	char * titleToMatch;
};

void globalForegroundWindow_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	// check if we have argument/s
	if (!argc)
	{
		object_error((t_object*)x, "you need at least one arg");
		object_error((t_object*)x, "'set <window number>' or");
		object_error((t_object*)x, "'set <app name>' or");
		object_error((t_object*)x, "'set <app name> <window title>'");
		return;
	}
	if (atom_gettype(argv) == A_LONG)
	{
		defer_low(x, (method)globalForegroundWindow_do_setWithNum, s, argc, argv);
	}
	else
	{
		defer_low(x, (method)globalForegroundWindow_do_set, s, argc, argv);
	}

}

void globalForegroundWindow_do_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	
	int titleInput = false;
	if (argc>1) titleInput = true;
	struct appNameAndTitle matchStruct;

	matchStruct.appNameToMatch = NULL;
	matchStruct.titleToMatch = NULL;

	// get the string from arg 1
	matchStruct.appNameToMatch = calloc(strlen(atom_getsym(argv)->s_name) + 1, sizeof(char));
	strcpy(matchStruct.appNameToMatch, atom_getsym(argv)->s_name);

	if (titleInput)
	{
		// get the string from arg 2
		matchStruct.titleToMatch = calloc(strlen(atom_getsym(argv + 1)->s_name) + 1, sizeof(char));
		strcpy(matchStruct.titleToMatch, atom_getsym(argv + 1)->s_name);

		int result = EnumWindows(globalForegroundWindow_setWindowWithProcNameAndTitleCallback, &matchStruct);
		if (result)
			object_error((t_object*)x, "can't find app with name: %s and title: %s", matchStruct.appNameToMatch, matchStruct.titleToMatch);
	}
	else
	{
		// enum windows and use first window matching proc name    
		// it's the highest in z-order, so probably the last used one of this process
		int result = EnumWindows(globalForegroundWindow_setWindowWithProcNameCallback, matchStruct.appNameToMatch);
		if (result)
			object_error((t_object*)x, "can't find app with name: %s", matchStruct.appNameToMatch);
		// progman windows?
	}

	if (matchStruct.appNameToMatch)
		free(matchStruct.appNameToMatch);

	if (matchStruct.titleToMatch)
		free(matchStruct.titleToMatch);

}

void globalForegroundWindow_activateWindow(t_globalForegroundWindow *x, HWND hWnd)
{
	//SwitchToThisWindow(hWnd, FALSE); // side effect: puts the current foregr.wind to the back

	DWORD fgPID;

	if (IsIconic(hWnd))//if minimized
	{
		ShowWindow(hWnd, SW_RESTORE);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
	}


	GetWindowThreadProcessId(GetForegroundWindow(), &fgPID);
	if (fgPID != GetCurrentProcessId())
	{
		//post("not a window of our process");
		HWND hCurWnd = GetForegroundWindow();
		DWORD dwMyID = GetCurrentThreadId();
		DWORD dwCurID = GetWindowThreadProcessId(hCurWnd, NULL);
		AttachThreadInput(dwCurID, dwMyID, TRUE);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		SetActiveWindow(hWnd);
		AttachThreadInput(dwCurID, dwMyID, FALSE);
	}
	else
	{
		//SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		//SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		SetActiveWindow(hWnd);
	}
}

void globalForegroundWindow_do_setWithNum(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	
	HWND hWnd = (HWND)atom_getlong(argv);

	if (!IsWindow(hWnd))
	{
		object_error((t_object *)x, "Could not find the window for num: %i", atom_getlong(argv));
		return;
	}

	globalForegroundWindow_activateWindow(x, hWnd);
}

BOOL CALLBACK globalForegroundWindow_setWindowWithProcNameAndTitleCallback(HWND hWnd, LPARAM lparam)
{
	bool result = TRUE;

	if (!IsWindowVisible(hWnd))
	{
		return result; // we don't list hidden windows
	}

	struct appNameAndTitle* matchStructPtr = lparam;

	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	char *path = NULL;
	char *filename = NULL; // without extension

	int success = globalForegroundWindow_getPathAndFilenameForPid(dwPID, &path, &filename);

	if (success)
	{
		if (!strcmp(matchStructPtr->appNameToMatch, filename))
		{
			// it's a match 
			//object_post(NULL, "found visible window of: %s", lparam);
			if (GetWindow(hWnd, GW_OWNER) == (HWND)0)
			{
				
				// TITLE
				int length = GetWindowTextLengthW(hWnd);
				if (length)
				{
					int i;
					length++;// +1 for terminator
					wchar_t *buffer = (wchar_t *)sysmem_newptrclear(length*sizeof(wchar_t)); //newptr

					GetWindowTextW(hWnd, buffer, length);

					char *maxConform = charset_unicodetoutf8(buffer, wcslen(buffer), NULL);
					sysmem_freeptr(buffer);

					// replace all backslashes in title
					for (i = 0; i <= strlen(maxConform); i++)
					{
						if (maxConform[i] == '\\')
						{
							maxConform[i] = '/';
						}
					}

					// compare
					if (!strcmp(matchStructPtr->titleToMatch, maxConform))
					{
						// it's a match 
						globalForegroundWindow_activateWindow(NULL, hWnd);
						result = FALSE;
					}
					
					sysmem_freeptr(maxConform);

				}
				 
			}

		}
	}

	if (filename)	sysmem_freeptr(filename);
	if (path)		sysmem_freeptr(path);
	return result;
}

BOOL CALLBACK globalForegroundWindow_setWindowWithProcNameCallback(HWND hWnd, LPARAM lparam)
{
	bool result = TRUE;

	if (!IsWindowVisible(hWnd))
	{
		return result; // we don't list hidden windows
	}
	
	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	char *path = NULL;
	char *filename = NULL; // without extension

	int success = globalForegroundWindow_getPathAndFilenameForPid(dwPID, &path, &filename);

	if (success)
	{
		if (!strcmp(lparam, filename))
		{
			// it's a match 
			//object_post(NULL, "found visible window of: %s", lparam);
			if (GetWindow(hWnd, GW_OWNER) == (HWND)0) 
			{
				globalForegroundWindow_activateWindow(NULL, hWnd);
				result = FALSE;
			}
			
		}
	}

	if (filename)	sysmem_freeptr(filename);
	if (path)		sysmem_freeptr(path);
	return result;
}

#pragma endregion

#pragma region ISRUNNING
void globalForegroundWindow_isrunning(t_globalForegroundWindow* x, t_symbol* s, long argc, t_atom* argv)
{
	defer_low(x, (method)globalForegroundWindow_do_isrunning, s, argc, argv);
}
void globalForegroundWindow_do_isrunning(t_globalForegroundWindow* x, t_symbol* s, long argc, t_atom* argv)
{
	t_atom as[3];
	struct appNameAndTitle matchStruct;
	if (!argc) return;

	matchStruct.appNameToMatch = NULL;
	
	// get the string from arg 1
	matchStruct.appNameToMatch = calloc(strlen(atom_getsym(argv)->s_name) + 1, sizeof(char));
	strcpy(matchStruct.appNameToMatch, atom_getsym(argv)->s_name);

	int result = EnumWindows(globalForegroundWindow_findWindowWithProcNameCallback, matchStruct.appNameToMatch);
	if (result)
		atom_setlong(as + 2, 0);
	else
		atom_setlong(as + 2, 1);

	atom_setsym(as, gensym("isrunning"));
	atom_setsym(as + 1, atom_getsym(argv)); //use input appname sym

	outlet_list(x->outdump, 0L, 3, as);

	if (matchStruct.appNameToMatch)
		free(matchStruct.appNameToMatch);
}

BOOL CALLBACK globalForegroundWindow_findWindowWithProcNameCallback(HWND hWnd, LPARAM lparam)
{
	bool result = TRUE;

	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	char* path = NULL;
	char* filename = NULL; // without extension

	int success = globalForegroundWindow_getPathAndFilenameForPid(dwPID, &path, &filename);

	if (success)
	{
		if (!strcmp(lparam, filename))
		{
			// it's a match 
			
			result = FALSE;
			
		}
	}

	if (filename)	sysmem_freeptr(filename);
	if (path)		sysmem_freeptr(path);
	return result;
}
#pragma endregion


#pragma region QUITAPP*

void globalForegroundWindow_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_quitapp, s, argc, argv);
}
void globalForegroundWindow_do_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	
	HWND hWnd = GetForegroundWindow();
	
	if (!argc)
	{
		DWORD dwPID;
		GetWindowThreadProcessId(hWnd, &dwPID);
		
		// reset bg windows of this proc or they won't be visible in enumWindows list
		globalForegroundWindow_resetBgWindowsWithPid(x, dwPID);
		
		//the softest way may be to send wm_close to all visible  windows of this proc
		EnumWindows(globalForegroundWindow_closeWindowsWithPidCallback, dwPID);
		return;
	}


	// we have an arg
	char * appNameToMatch = NULL;
	// get the string from arg 1
	appNameToMatch = calloc(strlen(atom_getsym(argv)->s_name) + 1, sizeof(char));
	strcpy(appNameToMatch, atom_getsym(argv)->s_name);

	// reset bg windows of this proc or they won't be visible in enumWindows list
	globalForegroundWindow_resetBgWindowsWithProcName(x, appNameToMatch);


	EnumWindows(globalForegroundWindow_closeWindowsWithProcNameCallback, appNameToMatch);

	if (appNameToMatch)
		free(appNameToMatch);

	return;

}


BOOL CALLBACK globalForegroundWindow_closeWindowsWithPidCallback(HWND hWnd, LPARAM lparam)
{
	//HWND desktop = GetDesktopWindow();
	//HWND parent = GetAncestor(hWnd, GA_ROOT);

	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	if (dwPID == lparam)
	{
		if (IsWindowVisible(hWnd))
		{
			//object_post((t_object*)x, "this window is visible");
			int success;
						
			if (GetWindow(hWnd, GW_OWNER) == (HWND)0) //better only close the top windows without owner
			{
				int success = PostMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
				if (!success) object_error(NULL, "failed to send close message to window");
			}
			else
			{
				;// object_post(NULL, "not closing owned window");
			}
			
		}
	}

	return TRUE;
}


BOOL CALLBACK globalForegroundWindow_closeWindowsWithProcNameCallback(HWND hWnd, LPARAM lparam)
{
	if (IsWindowVisible(hWnd))
	{
		DWORD dwPID;
		GetWindowThreadProcessId(hWnd, &dwPID);

		char *path = NULL;
		char *filename = NULL; // without extension

		int success = globalForegroundWindow_getPathAndFilenameForPid(dwPID, &path, &filename);

		if (success)
		{
			if (!strcmp(lparam, filename))
			{
				// it's a match 
				//object_post(NULL, "found visible window of: %s", lparam);
				if (GetWindow(hWnd, GW_OWNER) == (HWND)0) //better only close the top windows without owner
				{
					int success = PostMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
					if (!success) object_error(NULL, "failed to send close message to window");
				}
				else
				{
					object_post(NULL, "not closing owned window");
				}
			}
		}

		if (filename)	sysmem_freeptr(filename);
		if (path)		sysmem_freeptr(path);
	}

	return TRUE;
}


#pragma endregion


#pragma region KILLAPP*
void globalForegroundWindow_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_killapp, s, argc, argv);
}
void globalForegroundWindow_do_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	if (!argc) // no arg ... terminate the process that owns the window
	{
		DWORD dwPID;
		GetWindowThreadProcessId(GetForegroundWindow(), &dwPID);

		if (!globalForegroundWindow_TerminateProcessWithPid(dwPID))
			object_error((t_object*)x, "failed to terminate process");
		
		return;
	}

	// we have an arg, terminate all processes that match the name                                       

	char * appNameToMatch = NULL;
	// get the string from arg 1
	appNameToMatch = calloc(strlen(atom_getsym(argv)->s_name) + 1, sizeof(char));
	strcpy(appNameToMatch, atom_getsym(argv)->s_name);

	
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 procEntry;
	DWORD dwPriorityClass;

	// create snapshot of all processes 
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		object_error((t_object*)x, "CreateToolhelp32Snapshot failed");
		if (appNameToMatch)
			free(appNameToMatch);
		return;
	}

	// Set the size of the structure before using it.
	procEntry.dwSize = sizeof(PROCESSENTRY32);

	// check access
	if (!Process32First(hProcessSnap, &procEntry))
	{
		object_error((t_object*)x, "can't access proc snapshot");
		CloseHandle(hProcessSnap);  // clean the snapshot object
		if (appNameToMatch)
			free(appNameToMatch);
		return;
	}

	// iterate processes 
	do
	{

		char *path = NULL;
		char *filename = NULL; // without extension

		int success = globalForegroundWindow_getPathAndFilenameForPid(procEntry.th32ProcessID, &path, &filename);

		if (!success)
		{
			;// we do nothing, for some processes we can't get the path like this
		}
		else
		{
			if (!strcmp(appNameToMatch, filename)) // it's a match 
			{
				//object_post((t_object*)x, "kill arg found a matching process");

				if (!globalForegroundWindow_TerminateProcessWithPid(procEntry.th32ProcessID))
					object_error((t_object*)x, "failed to terminate: %s", appNameToMatch);
				else
					object_post((t_object*)x, "success terminating: %s", appNameToMatch);
			}

		}

		if (filename)	sysmem_freeptr(filename);
		if (path)		sysmem_freeptr(path);


	} while (Process32Next(hProcessSnap, &procEntry));

	CloseHandle(hProcessSnap);
	if (appNameToMatch)
		free(appNameToMatch);

}
#pragma endregion


#pragma region AGENT

void globalForegroundWindow_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_agent, s, argc, argv);
}
void globalForegroundWindow_do_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	;
	// leave this Mac only
	
}

#pragma endregion


#pragma region TOPMOST*
void globalForegroundWindow_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_topmost, s, argc, argv);
}

void globalForegroundWindow_do_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	if (argc < 1 || atom_gettype(argv) != A_LONG)
	{
		object_error((t_object*)x, "you need an integer argument");
		return;
	}

	int state = atom_getlong(argv);

	HWND hWnd = GetForegroundWindow();
	// HWND desktop = GetDesktopWindow();
	HWND parent = GetAncestor(hWnd, GA_PARENT);
	HWND ProgmanHwnd = FindWindow("Progman", "Program Manager");
	
	DWORD dwPID;
	GetWindowThreadProcessId(hWnd, &dwPID);

	//RECT fgWhndRect;
	//GetWindowRect(hWnd, &fgWhndRect);
	
	int fc = 0;
	int tm = 0;

	//check if we are fullscreen/topmost
	if (GetWindowLongPtr(hWnd, GWL_STYLE) & WS_MAXIMIZE)
		fc = 1;
	if (GetWindowLongPtr(hWnd, GWL_EXSTYLE) & WS_EX_TOPMOST)
		tm = 1;


	if (state==1) // MAKE TOPMOST
	{
		//reset the parent if needed
		globalForegroundWindow_resetBgWindowWithHwnd(x, hWnd);
		// set topmost level
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}


	else if (state == 0) // RESET TO NORMAL LEVEL
	{
		//reset the parent if needed
		globalForegroundWindow_resetBgWindowWithHwnd(x, hWnd);
		
		if (tm) // reset topmost
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}


	else if (state == -1) // FORCE TO THE BACK
	{

		int success = globalForegroundWindow_storeBgWindow(x, dwPID, hWnd);
		// may fail if we are already -1
		
	}

	// reset if it was fullscreen before
	/*if (fc) 
		if (!ShowWindow(hWnd, SW_MAXIMIZE))
			object_error((t_object *)x, "failed to re-maximize window");*/


}
#pragma endregion


#pragma region FULLSCREEN*
//TODO MAKE bg window fullscreen

void globalForegroundWindow_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_fullscreen, s, argc, argv);
}
void globalForegroundWindow_do_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	if (argc < 1 || atom_gettype(argv) != A_LONG)
	{
		object_error((t_object*)x, "you need an integer argument");
		return;
	}
	int state = atom_getlong(argv);

	// get handle of currently active window
	HWND hWnd = GetForegroundWindow();


	if (!state)
	{
		globalForegroundWindow_resetFsWindowWithHwnd(x, hWnd);

	}
	else
	{
		globalForegroundWindow_storeFsWindow(x, hWnd); 
	}

}
#pragma endregion


#pragma region MAXIMIZE
void globalForegroundWindow_maximize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	defer_low(x, (method)globalForegroundWindow_do_maximize, s, argc, argv);
}
void globalForegroundWindow_do_maximize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	if (argc < 1 || atom_gettype(argv) != A_LONG)
	{
		object_error((t_object*)x, "you need an integer argument");
		return;
	}
	int state = atom_getlong(argv);

	if (!state)
	{
		// get handle of currently active window
		HWND hWnd = GetForegroundWindow();
		if (!ShowWindow(hWnd, SW_NORMAL))
			object_error((t_object *)x, "failed to reset window");

	}
	else
	{
		// get handle of currently active window
		HWND hWnd = GetForegroundWindow();
		if (!ShowWindow(hWnd, SW_MAXIMIZE))
			object_error((t_object *)x, "failed to maximize window");
	}
}
#pragma endregion 


#pragma region MINIMIZE*
void globalForegroundWindow_minimize(t_globalForegroundWindow *x)
{
	defer_low(x, (method)globalForegroundWindow_do_minimize, NULL, NULL, NULL);
}
void globalForegroundWindow_do_minimize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	// get handle of currently active window
	HWND hWnd = GetForegroundWindow();
	if(!ShowWindow(hWnd, SW_MINIMIZE))
		object_error((t_object *)x, "failed to minimize window");
}
#pragma endregion


#pragma region CLOSE*

void globalForegroundWindow_close(t_globalForegroundWindow *x)
{
	defer_low(x, (method)globalForegroundWindow_do_close, NULL, NULL, NULL);
}
void globalForegroundWindow_do_close(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	int success;
	
	HWND hWnd = GetForegroundWindow();

	success = PostMessage(hWnd, WM_CLOSE, 0, 0);
	if (!success) object_error((t_object *)x, "failed to send close message to window");
}
#pragma endregion


#pragma region INT
void globalForegroundWindow_int(t_globalForegroundWindow *x, long m)
{
	if (m) defer_low(x, (method)globalForegroundWindow_start, NULL, 0, NULL);
	else defer_low(x, (method)globalForegroundWindow_stop, NULL, 0, NULL);
}
#pragma endregion


#pragma region START
void globalForegroundWindow_start(t_globalForegroundWindow *x)
{
	int i;
	
	if (x->x_systhread == NULL)
	{
		//object_post((t_object *)x, "starting a new thread");
		systhread_create((method)globalForegroundWindow_threadproc, x, 0, 1, 0, &x->x_systhread);
	}
	else
	{
		object_warn((t_object *)x, "already running");
	}
		
}
#pragma endregion


#pragma region THREAD
void *globalForegroundWindow_threadproc(t_globalForegroundWindow *x)
{
	int i;
	DWORD hook_thread_id = GetCurrentThreadId();

	
	//register in a free slot in global arrays
	for (i = 0; i < 256; i++)
	{
		if (t_globalForegroundWindow_temp[i] == NULL)
		{
			t_globalForegroundWindow_temp[i] = x;
			globalForegroundWindow_thread_ids[i] = hook_thread_id;
			//object_post((t_object *)x, "instance %d started. thread id: %d", i + 1, hook_thread_id);
			break;
		}
		if (i == 255)
		{
			object_error((t_object *)x, "you can not start more than 256 Instances of this object");
			return;
		}
	}

	
	// Set the thread id we want to signal later.
	x->hook_thread_id = hook_thread_id;


	// Spot check the hInst incase the library was statically linked and DllMain
	// did not receive a pointer on load.
	if (hInst == NULL)
	{
		
		hInst = GetModuleHandle(NULL);
		if (hInst != NULL) {
			// Initialize native input helper functions.
			//load_input_helper();
		}
		else {
			object_error((t_object *)x, "Could not determine hInst for SetWindowsHookEx()!");

			//status = UIOHOOK_ERROR_GET_MODULE_HANDLE;
		}
	}

	
	//trigger now to have a startpoint
	defer_low(x, (method)globalForegroundWindow_do_bang, NULL, 0, NULL);


	// Create a window event hook  
	x->win_event_hhook = SetWinEventHook(
		EVENT_SYSTEM_FOREGROUND, EVENT_OBJECT_NAMECHANGE,  // EVENT_SYSTEM_FOREGROUND EVENT_OBJECT_NAMECHANGE
		NULL,
		win_hook_event_proc,
		0, 0,
		WINEVENT_OUTOFCONTEXT ); // | WINEVENT_SKIPOWNPROCESS

	if (x->win_event_hhook != NULL) 
	{
		

		// here's the loop
		// Block until the thread receives an WM_QUIT request.
		MSG message;
		while (GetMessage(&message, (HWND)NULL, 0, 0) > 0) 
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}


	}
	else 
	{
		object_error((t_object *)x, "SetWindowsHookEx() failed! ");
		
	}
	
	// Unregister any hooks that may still be installed.
	if (x->win_event_hhook != NULL)
	{
		UnhookWinEvent(x->win_event_hhook);
		x->win_event_hhook = NULL;
	}

	// remove this object/thread from global arrays
	for (i = 0; i < 256; i++)
	{
		if (t_globalForegroundWindow_temp[i] == x)
		{
			t_globalForegroundWindow_temp[i] = NULL;
			globalForegroundWindow_thread_ids[i] = 0;
			break;
		}
		if (i == 255)
		{
			// the object wasn't in the list
			object_error((t_object *)x, "Thread failed to remove itself from global array!");
		}
	}

	
	systhread_exit(0);
	return NULL;
}
#pragma endregion


// Callback function that handles events.
void CALLBACK win_hook_event_proc(HWINEVENTHOOK hook, DWORD event, HWND hWnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{

	t_globalForegroundWindow   *x = thisobject(); //get x

	switch (event)
	{

	case EVENT_OBJECT_NAMECHANGE:
		if (hWnd == GetForegroundWindow())
		{
			//object_post((t_object*)x, "fg window namechange change");
			qelem_set(x->x_qelem);
		}
		break;

	case EVENT_SYSTEM_FOREGROUND:
		qelem_set(x->x_qelem);
		break;

	case EVENT_OBJECT_LOCATIONCHANGE:
		if (hWnd == GetForegroundWindow() && idObject == 0)
		{
			//object_post((t_object*)x, "fg window location change");
			qelem_set(x->x_qelem);
		}
		break;

		//case EVENT_SYSTEM_MOVESIZESTART:
		//case EVENT_SYSTEM_MOVESIZEEND:  
		//case EVENT_SYSTEM_MINIMIZEEND: 
		//case EVENT_OBJECT_STATECHANGE:

	default:
		//object_post((t_object *)x, "Unhandled Windows window event: %x", event);
		break;
	}
}


void globalForegroundWindow_stop(t_globalForegroundWindow *x)                                                   //STOP//---------------------
{
	unsigned int ret; 

	// tell the thread to stop
	PostThreadMessage(x->hook_thread_id, WM_QUIT, (WPARAM)NULL, (LPARAM)NULL);
	
	if (x->x_systhread) 
	{							
		systhread_join(x->x_systhread, &ret); // wait for the thread to stop
		x->x_systhread = NULL;
	}
}


void unregister_running_hooks()
{
	t_globalForegroundWindow   *x = thisobject();
	if (x->win_event_hhook != NULL)
	{
		UnhookWinEvent(x->win_event_hhook);
		x->win_event_hhook = NULL;
	}
}


t_globalForegroundWindow   *thisobject()
{
	int i;
	int myObjectIndex;
	// find my object
	DWORD hook_thread_id = GetCurrentThreadId();
	for (i = 0; i < 256; i++)
	{

		if (globalForegroundWindow_thread_ids[i] == hook_thread_id)
		{
			myObjectIndex = i;
			break;
		}
		if (i == 255)
		{
			// if we did it well, this is never posted
			error( "11globalForegroundWindow: thisobject() could not return an object pointer!");  
			return NULL;
		}
	}
	return t_globalForegroundWindow_temp[myObjectIndex];
}


void globalForegroundWindow_free(t_globalForegroundWindow *x)												//FREE///////////////////
{
	globalForegroundWindow_stop(x);
	globalForegroundWindow_resetAllFsWindows(x);
	globalForegroundWindow_resetAllBgWindows(x);
	qelem_free(x->x_qelem);
}


void globalForegroundWindow_assist(t_globalForegroundWindow *x, void *b, long msg, long arg, char *dst)
{
	if (msg == 1) {     // Inlets
		switch (arg) {
		case 0: strcpy(dst, "input"); break;
			//default: strcpy(dst, "(signal) Audio Input"); break;
		}
	}
	else if (msg == 2) { // Outlets
		if (arg == 0)
			strcpy(dst, "window loc / size");
		else if (arg == 1)
			strcpy(dst, "window title");
		else if (arg == 2)
			strcpy(dst, "process path");
		else if (arg == 3)
			strcpy(dst, "process name");
		else if (arg == 4)
			strcpy(dst, "info output");
		else
			strcpy(dst, "?");
	}
}





/*  HELPER */


int globalForegroundWindow_getPathAndFilenameForPid(DWORD dwPID, char **path, char **shortFilename)
{
	HANDLE Handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
		FALSE,
		dwPID
		);
	if (Handle)
	{
		LPWSTR pathWide[MAX_PATH_CHARS];
		DWORD dwSize = MAX_PATH_CHARS;

		if (!QueryFullProcessImageNameW(Handle, 0, pathWide, &dwSize))
		{
			//object_error(NULL, "Error getting path of the process!");
			CloseHandle(Handle);
			return 0;
		}
		CloseHandle(Handle);

		char *pathUTF = charset_unicodetoutf8(pathWide, wcslen(pathWide), NULL);

		*path = sysmem_newptrclear((MAX_PATH_CHARS + 1)* sizeof(char));

		path_nameconform(pathUTF, *path, PATH_STYLE_MAX, PATH_TYPE_ABSOLUTE);

		sysmem_freeptr(pathUTF);

		wchar_t widefilename[MAX_PATH_CHARS];


		//get filename without extension
		_wsplitpath(pathWide, NULL, NULL, widefilename, NULL);
		*shortFilename = charset_unicodetoutf8(widefilename, wcslen(widefilename), NULL);

		return 1;
	}
	else
	{
		//object_error(NULL, "Error getting handle of the process!");
		return 0;
	}
}


int globalForegroundWindow_TerminateProcessWithPid(DWORD ProcessId)
{
	
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, ProcessId);
	if (hProcess == NULL)
		return FALSE;

	BOOL result = TerminateProcess(hProcess, 0);

	CloseHandle(hProcess);

	return result;
}


int globalForegroundWindow_storeBgWindow(t_globalForegroundWindow *x, DWORD dwPID, HWND hWnd)
{
	int i;

	// check if the window is already bg
	for (i = 0; i < 256; i++)
	{
		if (bgWndHandles[i] == hWnd)
		{
			// do nothing the window is already bg
			object_post((t_object *)x, "window is already 'topmost -1'");
			return 0;
		}
	}

	HWND desktop = GetDesktopWindow();
	HWND parent = GetAncestor(hWnd, GA_PARENT);
	HWND ProgmanHwnd = FindWindow("Progman", "Program Manager");
	RECT fgWhndRect;
	GetWindowRect(hWnd, &fgWhndRect);

	if (parent != desktop)
	{
		object_warn((t_object *)x, "we're applying a child of another window to 'topmost -1'");
	}

	// Set the window's parent to progman, so it stays always on desktop
	if (ProgmanHwnd)
	{
		SetParent(hWnd, ProgmanHwnd);
	}
	else
	{
		object_error((t_object *)x, "failed to 'topmost -1', can't get Program Manager window");
		return 0;
	}
	
	// bookholding
	for (i = 0; i < 256; i++)
	{
		if (bgWndHandles[i] == NULL) // find a free slot
		{
			bgWndHandles[i] = hWnd;
			bgWndParentHandles[i] = parent;
			bgWndPids[i] = dwPID;
			
			break;
		}
		else
		{
			// we use the chance to remove dead windows
			if (!IsWindow(bgWndHandles[i]))
			{
				bgWndHandles[i] = NULL;
				bgWndParentHandles[i] = NULL;
				bgWndPids[i] = NULL;
				//object_post((t_object *)x, "removed a dead window reference from bg...");
				//and use this cleared slot
				bgWndHandles[i] = hWnd;
				bgWndParentHandles[i] = parent;
				bgWndPids[i] = dwPID;
				break;
			}
		}
		if (i == 255)
		{
			object_error((t_object *)x, "you can not put more than 256 windows to 'topmost -1' ");
			// rollback
			SetParent(hWnd, parent);
			return 0;
		}
	}

	MapWindowPoints(parent, ProgmanHwnd, (LPPOINT)&fgWhndRect, 2);
	SetWindowPos(hWnd, NULL, fgWhndRect.left, fgWhndRect.top, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER);
	// object_post((t_object*)x, "%i %i %i %i", fgWhndRect.left, fgWhndRect.top, fgWhndRect.right, fgWhndRect.bottom);

	return 1;
}


void globalForegroundWindow_resetBgWindowWithHwnd(t_globalForegroundWindow *x, HWND hWnd)
{
	int i; 

		
	for (i = 0; i < 256; i++)
	{
		if (bgWndHandles[i] == hWnd)
		{
			HWND ProgmanHwnd = FindWindow("Progman", "Program Manager");
			HWND desktop = GetDesktopWindow();
			RECT fgWhndRect;
			GetWindowRect(hWnd, &fgWhndRect);
			SetParent(hWnd, bgWndParentHandles[i]);
			bgWndParentHandles[i] = NULL;
			bgWndHandles[i] = NULL;
			bgWndPids[i] = NULL;

			// remap position

			MapWindowPoints(desktop, bgWndParentHandles[i], (LPPOINT)&fgWhndRect, 2);
			SetWindowPos(hWnd, NULL, fgWhndRect.left, fgWhndRect.top, 0, 0, SWP_ASYNCWINDOWPOS | SWP_NOSIZE | SWP_NOZORDER);
			break;// 
		}
		if (i == 255)
		{
			// the object wasn't in the list
			//object_post((t_object *)x, "window is not 'topmost -1'");
		}
	}
}


void globalForegroundWindow_resetBgWindowsWithPid(t_globalForegroundWindow *x, DWORD dwPID)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		if (bgWndPids[i] == dwPID)
		{
			if (IsWindow(bgWndHandles[i])) // if the window lives
			{
				SetParent(bgWndHandles[i], bgWndParentHandles[i]);
			}
						
			bgWndParentHandles[i] = NULL;
			bgWndHandles[i] = NULL;
			bgWndPids[i] = NULL;
		}
		
	}
}


void globalForegroundWindow_resetBgWindowsWithProcName(t_globalForegroundWindow *x, char* appNameToMatch)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		if (bgWndPids[i] != NULL)
		{
			char *path = NULL;
			char *filename = NULL; // without extension

			int success = globalForegroundWindow_getPathAndFilenameForPid(bgWndPids[i], &path, &filename);

			if (!success)
			{
				;
			}
			else
			{
				if (!strcmp(appNameToMatch, filename))
				{
					// it's a match , must reset to normal
					if (IsWindow(bgWndHandles[i])) // if the window lives
					{
						SetParent(bgWndHandles[i], bgWndParentHandles[i]);
					}

					bgWndParentHandles[i] = NULL;
					bgWndHandles[i] = NULL;
					bgWndPids[i] = NULL;
				}

			}
			if (filename)	sysmem_freeptr(filename);
			if (path)		sysmem_freeptr(path);
		}
	}
}


void globalForegroundWindow_resetAllBgWindows(t_globalForegroundWindow *x)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		if (bgWndHandles[i] != NULL)
		{
			if (IsWindow(bgWndHandles[i])) // if the window lives
			{
				SetParent(bgWndHandles[i], bgWndParentHandles[i]);
			}
			
			bgWndParentHandles[i] = NULL;
			bgWndHandles[i] = NULL;
			bgWndPids[i] = NULL;
		}
		
	}
}

int globalForegroundWindow_storeFsWindow(t_globalForegroundWindow *x, HWND hWnd)
{
	int i;

	// check if the window is already fs
	for (i = 0; i < 256; i++)
	{
		if (fsWndHandles[i] == hWnd)
		{
			// do nothing the window is already fs
			object_post((t_object *)x, "window is already 'fullscreen'");
			return 0;
		}
	}

	

	/* On Windows, fullscreen is something handled by the app itself,
	// so this can only be a hack that hopefully works for the most of possible target windows
	// without to much side effects
	//
	// - some windows seem to maximize over taskbar only if  WS_BORDER is removed
	// 
	*/

	DWORD currentStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
	
	globalForegroundWindow_windowTool(x, hWnd, 4); //topmost
	globalForegroundWindow_windowTool(x, hWnd, -1); // maximize window, resize client area to fullscreen

	
	if (globalForegroundWindow_windowTool(x, hWnd, 11))//if we are blocked by taskbar or something
	{
		globalForegroundWindow_windowTool(x, hWnd, 10); // remove border
		globalForegroundWindow_windowTool(x, hWnd, -1); // maximize window, resize client area to fullscreen
	}
	
	//globalForegroundWindow_windowTool(x, hWnd, 2); //negative margin
	//globalForegroundWindow_windowTool(x, hWnd, 5); //reduce window region to client area
	//globalForegroundWindow_windowTool(x, hWnd, 6); //disable nc rendering
	//globalForegroundWindow_windowTool(x, hWnd, 1); //margin = 0

	
	// bookholding
	for (i = 0; i < 256; i++)
	{
		if (fsWndHandles[i] == NULL) // find a free slot
		{
			fsWndHandles[i] = hWnd; 
			fsWndStyles[i] = currentStyle;
			break;
		}
		else
		{
			// we use the chance to remove dead windows
			if (!IsWindow(fsWndHandles[i]))
			{
				fsWndHandles[i] = NULL;
				fsWndStyles[i] = NULL;
				
				//object_post((t_object *)x, "removed a dead window reference from fs...");
				//and use this cleared slot
				fsWndHandles[i] = hWnd;
				fsWndStyles[i] = currentStyle;
				break;
			}
		}
		if (i == 255)
		{
			// restore window 0 and use this slot
			globalForegroundWindow_resetFsWindowWithHwnd(x, fsWndHandles[0]);
			fsWndHandles[0] = hWnd;
			fsWndStyles[0] = currentStyle;
		}
	}
}

void globalForegroundWindow_resetFsWindowWithHwnd(t_globalForegroundWindow *x, HWND hWnd)
{
	int i;

	for (i = 0; i < 256; i++)
	{
		if (fsWndHandles[i] == hWnd)
		{

			globalForegroundWindow_windowTool(x, hWnd, 50);// reset region
			globalForegroundWindow_windowTool(x, hWnd, 60); //enable nc rendering
			globalForegroundWindow_windowTool(x, hWnd, -11); //reset maximization
			globalForegroundWindow_windowTool(x, hWnd, 40); //reset topmost
			SetWindowLongPtr(hWnd, GWL_STYLE, fsWndStyles[i]); // reset style

			//? do we need the update 
			SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE); //
			
			fsWndHandles[i] = NULL;
			fsWndStyles[i] = NULL;

			break;// 
		}
		if (i == 255)
		{
			// the object wasn't in the list
			//object_post((t_object *)x, "window is not 'topmost -1'");
		}
	}
}

void globalForegroundWindow_resetAllFsWindows(t_globalForegroundWindow *x)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		if (fsWndHandles[i] != NULL)
		{
			if (IsWindow(fsWndHandles[i])) // if the window lives
			{
				globalForegroundWindow_resetFsWindowWithHwnd(x, fsWndHandles[i]);
			}

			fsWndHandles[i] = NULL;
		}

	}
}

int globalForegroundWindow_getFullscreenRectForWindow(HWND hWnd, RECT * rectPtr)
{
	// restored window location in workspace
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);

	MONITORINFO  moninfo;
	moninfo.cbSize = sizeof(MONITORINFO); // !!

	// find monitor that displays top-left corner of window
	POINT pt = { wp.rcNormalPosition.left, wp.rcNormalPosition.top };
	HMONITOR hmon = NULL;
	// hmon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
	hmon = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	if (hmon)
	{

		if (GetMonitorInfo(hmon, &moninfo))
		{

			rectPtr->left = moninfo.rcMonitor.left;
			rectPtr->top = moninfo.rcMonitor.top ;
			rectPtr->right = moninfo.rcMonitor.right;
			rectPtr->bottom = moninfo.rcMonitor.bottom;

			return 1;
		}
		else
		{
			object_error(NULL, "error getting monitor info");
			return  0;
		}
	}
	else
	{
		object_error(NULL, "error getting monitor from point of minimized window");
		return 0;
	}

	return 0;
}


int globalForegroundWindow_getRectOfMinimizedWindow(HWND hWnd, RECT* rectPtr)
{	
	// SystemParametersInfo gets the workspace only for the primary screen 
	// if (SystemParametersInfo(SPI_GETWORKAREA,UINT  uiParam,PVOID pvParam,UINT  fWinIni))
	
	
	// restored window location in workspace
	WINDOWPLACEMENT wp = { sizeof(WINDOWPLACEMENT) };
	GetWindowPlacement(hWnd, &wp);

	MONITORINFO  moninfo;
	moninfo.cbSize = sizeof(MONITORINFO); // !!

	// find monitor that displays top-left corner of window
	POINT pt = { wp.rcNormalPosition.left, wp.rcNormalPosition.top };
	HMONITOR hmon = NULL;
	hmon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);

	if (hmon)
	{
		
		if (GetMonitorInfo(hmon, &moninfo))
		{
			int  tOffset;
			int lOffset;
			
			 //if (!&moninfo) object_post((t_object*)x, "no moninfo"); 
			tOffset = abs((int)moninfo.rcWork.top - (int)moninfo.rcMonitor.top);
			lOffset = abs((int)moninfo.rcWork.left - (int)moninfo.rcMonitor.left);

			rectPtr->left = wp.rcNormalPosition.left + lOffset;
			rectPtr->top = wp.rcNormalPosition.top + tOffset;
			rectPtr->right = wp.rcNormalPosition.right + lOffset;
			rectPtr->bottom = wp.rcNormalPosition.bottom + tOffset; 

			/*object_post((t_object*)x, "window location in workspace: %i %i %i %i",
				wp.rcNormalPosition.left,
				wp.rcNormalPosition.top,
				wp.rcNormalPosition.right - wp.rcNormalPosition.left,
				wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);

			object_post((t_object*)x, "workspace rect: %i %i %i %i",
				moninfo.rcWork.left,
				moninfo.rcWork.top,
				moninfo.rcWork.right - moninfo.rcWork.left,
				moninfo.rcWork.bottom - moninfo.rcWork.top);

			object_post((t_object*)x, "monitor rect: %i %i %i %i",
				moninfo.rcMonitor.left,
				moninfo.rcMonitor.top,
				moninfo.rcMonitor.right - moninfo.rcMonitor.left,
				moninfo.rcMonitor.bottom - moninfo.rcMonitor.top);*/

			//object_post((t_object*)x, "topOffset %i", tOffset);
			//object_post((t_object*)x, "leftOffset %i", lOffset);

			/*object_post((t_object*)x, "window location on vitual screen: %i %i %i %i",
				wp.rcNormalPosition.left + lOffset,
				wp.rcNormalPosition.top + tOffset,
				wp.rcNormalPosition.right - wp.rcNormalPosition.left,
				wp.rcNormalPosition.bottom - wp.rcNormalPosition.top);*/

			return 1;

			
		}
		else
		{
			object_error(NULL, "error getting monitor info");
			return 0;
		}
	}
	else
	{
		object_error(NULL, "error getting monitor from point of minimized window");
		return 0;
	}
	return 0;
}


int globalForegroundWindow_windowTool(t_globalForegroundWindow *x, HWND hWnd, int CaSe)
{

	if (CaSe == -1)  //maximize and resize so that client area fills screen
	{
		// maximize
		if (!ShowWindow(hWnd, SW_MAXIMIZE))
			object_error((t_object *)x, "failed to maximize window");


		// fg window rect
		RECT fgWhndRect;
		GetWindowRect(hWnd, &fgWhndRect);

		//object_post((t_object*)x, "fgWhndRect : %i %i %i %i", fgWhndRect.left, fgWhndRect.top, fgWhndRect.right, fgWhndRect.bottom);


		// get fullsreen coords of the used monitor
		RECT fullScreenRect;
		if (!globalForegroundWindow_getFullscreenRectForWindow(hWnd, &fullScreenRect))
			object_error((t_object*)x, "error getting fullscreen size for this window's monitor");

		//object_post((t_object*)x, "Monitor Rect : %i %i %i %i", fullScreenRect.left, fullScreenRect.top, fullScreenRect.right, fullScreenRect.bottom);


		// new window rect if client area grows to fullscreen size
		RECT fullScreenWindowRect;
		CopyRect(&fullScreenWindowRect, &fullScreenRect);
		AdjustWindowRect(&fullScreenWindowRect, GetWindowLongPtr(hWnd, GWL_STYLE), GetMenu(hWnd));

		//object_post((t_object*)x, "fullScreenWindowRect : %i %i %i %i", fullScreenWindowRect.left, fullScreenWindowRect.top, fullScreenWindowRect.right, fullScreenWindowRect.bottom);

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		// client rect position on screen
		POINT pt_topleft = { 0, 0 };
		if (!ClientToScreen(hWnd, &pt_topleft))
			object_error((t_object*)x, "error converting client to screen coords");

		int amountToMoveX = fullScreenRect.left - pt_topleft.x;
		int amountToMoveY = fullScreenRect.top - pt_topleft.y;

		//object_post((t_object*)x, "amountToMoveX,amountToMoveY : %i %i", amountToMoveX, amountToMoveY);

		// client rect position on screen
		POINT pt_botright = { clientRect.right, clientRect.bottom };
		if (!ClientToScreen(hWnd, &pt_botright))
			object_error((t_object*)x, "error converting client to screen coords");

		int amountToGrowX = fullScreenRect.right - pt_botright.x;
		int amountToGrowY = fullScreenRect.bottom - pt_botright.y;

		//object_post((t_object*)x, "amountToGrowX,amountToGrowY : %i %i", amountToGrowX, amountToGrowY);


		RECT resizeTargetRect = { fgWhndRect.left + amountToMoveX, fgWhndRect.top + amountToMoveY, fullScreenWindowRect.right, fullScreenWindowRect.bottom };
		//object_post((t_object*)x, "setting window pos to (pos/size): %i %i %i %i", fgWhndRect.left + amountToMoveX, fgWhndRect.top + amountToMoveY, fullScreenWindowRect.right - fullScreenWindowRect.left, fullScreenWindowRect.bottom - fullScreenWindowRect.top);

		if (!SetWindowPos(hWnd, 0, fgWhndRect.left + amountToMoveX,
			fgWhndRect.top + amountToMoveY,
			fullScreenWindowRect.right - fullScreenWindowRect.left,
			fullScreenWindowRect.bottom - fullScreenWindowRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)) //| SWP_NOSIZE SWP_NOZORDER  
		{
			object_error((t_object*)x, "error setting size");
		}


	}

	else if (CaSe == 0) // redraw
	{
		RECT fgWhndRect;
		GetWindowRect(hWnd, &fgWhndRect);

		SetWindowPos(hWnd, 0, fgWhndRect.left,
			fgWhndRect.top,
			fgWhndRect.right - fgWhndRect.left,
			fgWhndRect.bottom - fgWhndRect.top,
			SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}

	else if (CaSe == 1) // margins 0
	{
		MARGINS margins;

		margins.cxLeftWidth = 0;
		margins.cxRightWidth = 0;
		margins.cyBottomHeight = 0;
		margins.cyTopHeight = 0;

		HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
		if (!SUCCEEDED(hr))
		{
			object_error((t_object*)x, "error extending frame");
		}
	}

	else if (CaSe == 2) // margin -1
	{

		MARGINS margins;

		margins.cxLeftWidth = -1;
		margins.cxRightWidth = -1;
		margins.cyBottomHeight = -1;
		margins.cyTopHeight = -1;

		HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
		if (!SUCCEEDED(hr))
		{
			object_error((t_object*)x, "error extending frame");
		}
	}

	else if (CaSe == 3) // margin 1
	{
		MARGINS margins;

		margins.cxLeftWidth = 1;
		margins.cxRightWidth = 1;
		margins.cyBottomHeight = 1;
		margins.cyTopHeight = 1;

		HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, &margins);
		if (!SUCCEEDED(hr))
		{
			object_error((t_object*)x, "error extending frame");
		}
	}

	else if (CaSe == 4) // topmost to cover taskbar
	{
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE); //| SWP_NOSIZE SWP_NOZORDER 
	}

	else if (CaSe == 5) // set window REGION
	{

		// fg window rect
		RECT fgWhndRect;
		GetWindowRect(hWnd, &fgWhndRect);
		object_post((t_object*)x, "fgWhndRect : %i %i %i %i", fgWhndRect.left, fgWhndRect.top, fgWhndRect.right, fgWhndRect.bottom);

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		MapWindowPoints(hWnd, NULL, (LPPOINT)(&clientRect), 2);
		object_post((t_object*)x, "ClientRect (screen coord): %i %i %i %i", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);


		RECT visibleRect;
		visibleRect.left = clientRect.left - fgWhndRect.left;
		visibleRect.top = clientRect.top - fgWhndRect.top;
		visibleRect.right = visibleRect.left + clientRect.right - clientRect.left;
		visibleRect.bottom = visibleRect.top + clientRect.bottom - clientRect.top;


		HRGN WinRgn;
		//WinRgn = CreateRoundRectRgn(0, 0, 500, 500, 500, 500); // Specify ROUND shape
		WinRgn = CreateRectRgnIndirect(&visibleRect);


		// reduce the window region to the client area
		SetWindowRgn(hWnd, WinRgn, true); //CreateRectRgnIndirect(&visibleRect)

		int success = PostMessage(hWnd, WM_THEMECHANGED, NULL, NULL);
		if (!success) object_error((t_object*)x, "failed to send WM_THEMECHANGED message to window");

		DeleteObject(WinRgn);
	}

	else if (CaSe == 6)// disable NC rendering
	{

		enum DWMNCRENDERINGPOLICY ncrp = DWMNCRP_DISABLED;

		// Disable non-client area rendering on the window.
		if (DwmSetWindowAttribute(hWnd,
			DWMWA_NCRENDERING_POLICY,
			&ncrp,
			sizeof(ncrp)) != S_OK)
		{
			object_error((t_object *)x, "failed to disable non-client area");
		}

		// SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOMOVE); //

	}

	else if (CaSe == 7)// disable NC_PAINT
	{

		BOOL fal = FALSE;
		if (DwmSetWindowAttribute(hWnd, DWMWA_ALLOW_NCPAINT, &fal, sizeof(BOOL)) != S_OK)
		{
			object_error((t_object *)x, "failed to disable NCPAINT");
		}
	}

	else if (CaSe == 8)// redraw
	{
		RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
	}

	else if (CaSe == 9)// erase bg
	{
		HDC			hdc;
		hdc = GetDCEx(hWnd, NULL, DCX_WINDOW | DCX_PARENTCLIP);//DCX_EXCLUDERGN
		SendMessage(hWnd, WM_ERASEBKGND, (WPARAM)hdc, NULL);
		ReleaseDC(hWnd, hdc);
	}

	else if (CaSe == 10)// remove border
	{
		DWORD dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
		DWORD dwRemove = WS_BORDER;
		SetWindowLongPtr(hWnd, GWL_STYLE, dwStyle & ~dwRemove);

	}

	else if (CaSe == 11)// return 1 if client area has not the size of monitor
	{
		
		// get fullsreen coords of the used monitor
		RECT fullScreenRect;
		if (!globalForegroundWindow_getFullscreenRectForWindow(hWnd, &fullScreenRect))
			object_error((t_object*)x, "error getting fullscreen size for this window's monitor");

		//object_post((t_object*)x, "Monitor Rect : %i %i %i %i", fullScreenRect.left, fullScreenRect.top, fullScreenRect.right, fullScreenRect.bottom);
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		MapWindowPoints(hWnd, NULL, (LPPOINT)(&clientRect), 2);

		//object_post((t_object*)x, "ClientRect (screen coord): %i %i %i %i", clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
		if (!EqualRect(&fullScreenRect, &clientRect))
		{
			//object_post((t_object*)x, "resize was blocked");
			return 1;
		}
		else
			return 0;
	}

	else if (CaSe == 60)//// enable NC rendering
	{
		// enable non-client area rendering on the window.
		enum DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
		if (DwmSetWindowAttribute(hWnd,
			DWMWA_NCRENDERING_POLICY,
			&ncrp,
			sizeof(ncrp)) != S_OK)
		{
			object_error((t_object *)x, "failed to enable non-client area");
		}
	}
	else if (CaSe == 50)//// reset region
	{
		SetWindowRgn(hWnd, NULL, true);
		int success = PostMessage(hWnd, WM_THEMECHANGED, NULL, NULL);
		if (!success) object_error((t_object*)x, "failed to send WM_THEMECHANGED message to window");
	}
	else if (CaSe == 40)//// reset topmost
	{
		if (!SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE))
			object_error((t_object*)x, "failed reset topmost state of window");
	}
	else if (CaSe == -11)//// reset maximization
	{
		if (!ShowWindow(hWnd, SW_NORMAL))
			object_error((t_object *)x, "failed to restore window");
	}
	
}


void globalForegroundWindow_test(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	if (!argc) return;
	int caSE = atom_getlong(argv);
	globalForegroundWindow_windowTool(x, GetForegroundWindow(), caSE);
}
void globalForegroundWindow_testoutput(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
	object_post((t_object*)x, "test");
}


/*
region shortcuts:

CTRL + M + O will collapse all.

CTRL + M + L will expand all.

CTRL + M + P will expand all and disable outlining.

CTRL + M + M will collapse/expand the current section.

*/