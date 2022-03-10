/**
 Max object: OS-wide foreground window get / set
 -- 11oLsen.de
 */

#include "ext.h"
#include "ext_obex.h"

#import <AppKit/NSWindow.h>
#import <AppKit/NSScreen.h>
#include <libproc.h>



////////////////////////// object struct
typedef struct _globalForegroundWindow
{
	t_object					ob;			// the object itself (must be first)
	void *outa; //outlets
	void *outb;
    void *outc;
    void *outd;
    void *outdump;
    
    t_systhread		x_systhread;						// thread reference

    //runloop reference
    CFRunLoopRef event_loop;
    t_atom winLoc[4];
    
} t_globalForegroundWindow;

///////////////////////// function prototypes

void *globalForegroundWindow_new(t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_free(t_globalForegroundWindow *x);
void globalForegroundWindow_assist(t_globalForegroundWindow *x, void *b, long m, long a, char *s);

void            globalForegroundWindow_start(t_globalForegroundWindow *x);
void            globalForegroundWindow_stop(t_globalForegroundWindow *x);
void			*globalForegroundWindow_threadproc(t_globalForegroundWindow *x);
 
void globalForegroundWindow_do_bang(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_bang(t_globalForegroundWindow *x);

void globalForegroundWindow_do_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_getwindows(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_getwindows(t_globalForegroundWindow *x);

void globalForegroundWindow_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_setWithNum(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);

void globalForegroundWindow_isrunning(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_isrunning(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);

void globalForegroundWindow_do_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_minimize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_minimize(t_globalForegroundWindow *x);
void globalForegroundWindow_do_close(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_close(t_globalForegroundWindow *x);
void globalForegroundWindow_do_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);

void globalForegroundWindow_do_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_do_getscreens(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_getscreens(t_globalForegroundWindow *x);

void globalForegroundWindow_test(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv);
void globalForegroundWindow_output_noaccess(t_globalForegroundWindow *x);
void globalForegroundWindow_int(t_globalForegroundWindow *x, long m);


// my Helpers
AXUIElementRef          globalForegroundWindow_getFocusedWindow();
int                     globalForegroundWindow_isFullscreen(AXUIElementRef window);
int                     globalForegroundWindow_pressButtonFullscreen(AXUIElementRef window);
int                     globalForegroundWindow_pressButtonClose(AXUIElementRef window);
int                     globalForegroundWindow_pressButtonCancel(AXUIElementRef window);
int                     globalForegroundWindow_getWindowNumberForAXUIElementRef(AXUIElementRef window, pid_t pid);
AXUIElementRef          globalForegroundWindow_getAXUIElementRefForWindowNumber(int windowNumber);
NSRunningApplication*   globalForegroundWindow_getRunningAppPtrForName(char* appNameToMatch);
char *                  globalForegroundWindow_MYCFStringCopyUTF8String(CFStringRef aString);




//////////////////////// global class pointer variable
void *globalForegroundWindow_class;

static t_symbol    *emptySym;

void ext_main(void *r)
{	
	t_class *c;
	
	c = class_new("11globalForegroundWindow", (method)globalForegroundWindow_new, (method)globalForegroundWindow_free, (long)sizeof(t_globalForegroundWindow), 0L /* leave NULL!! */, A_GIMME, 0);
    
	class_addmethod(c, (method)globalForegroundWindow_assist,    "assist",        A_CANT, 0);
    
    class_addmethod(c, (method)globalForegroundWindow_bang, "bang", 0);
    class_addmethod(c, (method)globalForegroundWindow_int, "int", A_LONG, 0);
    
    class_addmethod(c, (method)globalForegroundWindow_getscreens, "getscreens", 0);
    class_addmethod(c, (method)globalForegroundWindow_getwindows, "getwindows", 0);
    class_addmethod(c, (method)globalForegroundWindow_set, "set", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_isrunning, "isrunning", A_GIMME, 0);
    
    class_addmethod(c, (method)globalForegroundWindow_size, "size", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_pos, "pos", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_fullscreen, "fullscreen", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_minimize, "minimize", 0);
    class_addmethod(c, (method)globalForegroundWindow_close, "close", 0);
    class_addmethod(c, (method)globalForegroundWindow_topmost, "topmost", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_killapp, "killapp", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_quitapp, "quitapp", A_GIMME, 0);
    class_addmethod(c, (method)globalForegroundWindow_agent, "agent", A_GIMME, 0);
    CLASS_METHOD_ATTR_PARSE(c, "agent", "undocumented", gensym("long"), 0, "1");
    //class_addmethod(c, (method)globalForegroundWindow_test, "test", A_GIMME, 0);
    
	class_register(CLASS_BOX, c); /* CLASS_NOBOX */
	globalForegroundWindow_class = c;
    emptySym = gensym("");
    object_post(NULL, "11globalForegroundWindow 2022/03/10 11OLSEN.DE");
	return 0;
}


#pragma mark NEW
void *globalForegroundWindow_new(t_symbol *s, long argc, t_atom *argv)
{
	t_globalForegroundWindow *x = NULL;
    
	if ((x = (t_globalForegroundWindow *)object_alloc(globalForegroundWindow_class)))
	{
		
	
    
        x->x_systhread = NULL;
        
        x->outdump = outlet_new(x, NULL);
        x->outd = outlet_new(x, NULL);
        x->outc = outlet_new(x, NULL);
        x->outb = outlet_new(x, NULL);
        x->outa = listout(x);
        
        x->event_loop = nil;
        
        /* Check if 'Enable access for assistive devices' is enabled. */
        if(!AXIsProcessTrusted()) // AXAPIEnabled() depr.
        {
            object_post((t_object *)x,"Accessibility API is not enabled! Only basic functionality." );
            defer_low(x, (method)globalForegroundWindow_output_noaccess, NULL, 0, NULL);
        }
    }
	return (x);
}
void globalForegroundWindow_output_noaccess(t_globalForegroundWindow *x)
{
    outlet_anything(x->outdump, gensym("noaccess"), 0, NULL);
}

void globalForegroundWindow_assist(t_globalForegroundWindow *x, void *b, long msg, long arg, char *dst)
{
    if (msg==1) {     // Inlets
        switch (arg) {
            case 0: strcpy(dst, "input"); break;
            //default: strcpy(dst, "(signal) Audio Input"); break;
        }
    }
    else if (msg==2) { // Outlets
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


#pragma mark SIZE
void globalForegroundWindow_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_size, s, argc, argv);
}
void globalForegroundWindow_do_size(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    if (argc != 2) return;
    
    AXUIElementRef window = globalForegroundWindow_getFocusedWindow();
    
    if (!window)
    {
        object_error((t_object *)x,"error getting focused window");
        return;
    }
    // We have the focused window
    
    CGSize windowSize;
    windowSize = CGSizeMake(atom_getlong(argv + 0), atom_getlong(argv + 1));

    AXValueRef temp = NULL;
    temp = AXValueCreate(kAXValueCGSizeType, &windowSize);
    AXUIElementSetAttributeValue(window, kAXSizeAttribute, temp);
    
    if(temp)
    CFRelease(temp);

    if(window)
    CFRelease(window);
    
}


#pragma mark POS
void globalForegroundWindow_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_pos, s, argc, argv);
}
void globalForegroundWindow_do_pos(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    if (argc != 2) return;
    
    AXUIElementRef window = globalForegroundWindow_getFocusedWindow();
    
    if (!window)
    {
        object_error((t_object *)x,"error getting focused window");
        return;
    }
    // We have the focused window
    
  
    CGPoint windowPosition;
    
    windowPosition = CGPointMake(atom_getlong(argv + 0), atom_getlong(argv + 1));

    AXValueRef temp = NULL;
    temp = AXValueCreate(kAXValueCGPointType, &windowPosition);
    AXUIElementSetAttributeValue(window, kAXPositionAttribute, temp);
    
    if(temp)
    CFRelease(temp);
 
    if(window)
    CFRelease(window);
}


#pragma mark GETWINDOWS
void globalForegroundWindow_getwindows(t_globalForegroundWindow *x)
{
    defer_low(x, (method)globalForegroundWindow_do_getwindows, NULL, NULL, NULL);
}
void globalForegroundWindow_do_getwindows(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    t_atom result[8];
    int wc=0;
    // get all running apps
    NSArray *runningApps=[[NSWorkspace sharedWorkspace]runningApplications];
    
    outlet_anything(x->outdump, gensym("getwindows"), 0, NULL);
    
    // iterate running apps
    for (NSRunningApplication* app  in runningApps)
    {
       
        pid_t pid = [app processIdentifier];
        AXUIElementRef appElem = AXUIElementCreateApplication(pid);
        
        // GET PATH AND NAME OF APP
        int ret;
        char namebuf[PROC_PIDPATHINFO_MAXSIZE];
        
        ret = proc_name(pid, namebuf, sizeof(namebuf));
        if ( ret <= 0 )
        {
            //object_error((t_object *)x, "error getting process name");
            atom_setsym(result+1,emptySym);
            
        }
        else
            atom_setsym(result+1,gensym(namebuf));
        
        
        CFArrayRef windows = NULL;
       
       // get all windows of the app
       if (AXUIElementCopyAttributeValues(appElem, kAXWindowsAttribute, 0, 1024, &windows) == kAXErrorSuccess)
       {
          
           // we have the windows, iterate
           for (NSInteger i = 0; i < CFArrayGetCount(windows); i++)
           {
               wc++;
               //post ("found %s %i", namebuf,i);
               
               AXUIElementRef window = NULL;
               window = CFArrayGetValueAtIndex(windows, i);
               
               //we got the window, get title
               CFStringRef title = NULL;
               AXUIElementCopyAttributeValue (window, kAXTitleAttribute, ((CFTypeRef*)&title) );
               
               if (title)
               {
                   char* conformTitle = NULL;
                   conformTitle = globalForegroundWindow_MYCFStringCopyUTF8String(title);
                   
                   atom_setsym(result+2,gensym(conformTitle));
                   
                   if (conformTitle)
                       free(conformTitle);
                   CFRelease(title);
               }
               else
               {
                   //window has no title or error getting it
                   atom_setsym(result+2,emptySym);
               }
               
               atom_setlong(result+3, 1);
               CFTypeRef TypeRef = nil;
               if (!AXUIElementCopyAttributeValue((AXUIElementRef)window, CFSTR("AXFullScreen"), &TypeRef))
               {
                   if (CFBooleanGetValue(TypeRef) == true)
                       atom_setlong(result+3, 2);
               }
               if (!AXUIElementCopyAttributeValue((AXUIElementRef)window, kAXMinimizedAttribute, &TypeRef))
               {
                   if (CFBooleanGetValue(TypeRef) == true)
                       atom_setlong(result+3, 0);
               }
               
               if (TypeRef)
               CFRelease(TypeRef);
               
               AXValueRef pos = NULL;
               AXValueRef size = NULL;
               CGPoint cpoint = CGPointMake(0, 0);
               CGSize csize = CGSizeMake(0, 0);
               
               
               AXUIElementCopyAttributeValue(window, kAXPositionAttribute, (CFTypeRef *)&pos);
               AXUIElementCopyAttributeValue(window, kAXSizeAttribute, (CFTypeRef *)&size);
               
               if (pos)
               {
                   AXValueGetValue(pos, kAXValueCGPointType, &cpoint);
                   CFRelease(pos);
               }
               if (size)
               {
                   AXValueGetValue(size, kAXValueCGSizeType, &csize);
                   CFRelease(size);
               }
               
               atom_setlong(result + 4, cpoint.x);
               atom_setlong(result + 5, cpoint.y);
               atom_setlong(result + 6, csize.width);
               atom_setlong(result + 7, csize.height);
               
               
               // output all the info for this window
               /*post ("%i : %s : %s : %i",
                     wc,
                     atom_getsym(result+1)->s_name,
                     atom_getsym(result+2)->s_name,
                     atom_getlong(result+3));*/
               
               // get a (temporary !) unique id for window as first atom //
               atom_setlong( result, wc );
               
               outlet_list(x->outdump, NULL, 8, result);
               
           } // stopped iterating windows of this app
           
       } // no windows

       if(windows) CFRelease(windows); // release all windows

       if(appElem)
           CFRelease(appElem);
       
    } // stopped iterating runningApps

    outlet_anything(x->outdump, gensym("getwindows_end"), 0, NULL);
    
}


#pragma mark GETSCREENS
void globalForegroundWindow_getscreens(t_globalForegroundWindow *x)
{
    defer_low(x, (method)globalForegroundWindow_do_getscreens, NULL, NULL, NULL);
}
void globalForegroundWindow_do_getscreens(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    t_atom as[6];
    
    NSScreen *mainScreen = [NSScreen mainScreen];
    NSEnumerator *screenEnum = [[NSScreen screens] objectEnumerator];
    NSScreen *screen;
    int screenCount = 0;
    
    
    outlet_anything(x->outdump, gensym("getscreens"), 0, NULL);
    
    
    //iterate screens
    while (screen = [screenEnum nextObject])
    {
        screenCount ++;
        
        atom_setlong(as,screenCount);
        
        atom_setlong(as+1,[screen frame].origin.x);
        atom_setlong(as+2,[screen frame].origin.y);
        atom_setlong(as+3,[screen frame].size.width);
        atom_setlong(as+4,[screen frame].size.height);
        
        /*post("point %f %f %f %f",
        [screen frame].origin.x,
        [screen frame].origin.y,
        [screen frame].size.width,
        [screen frame].size.height );*/

        if (screen == mainScreen)
            atom_setlong(as+5,1);
        else
            atom_setlong(as+5,0);
        
        // output
        outlet_list(x->outdump, NULL, 6, as);
    }
    
    outlet_anything(x->outdump, gensym("getscreens_end"), 0, NULL);
}



void globalForegroundWindow_test(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
        AXUIElementRef window = NULL;
       AXUIElementRef appElem = NULL;

       NSRunningApplication* app = [[NSWorkspace sharedWorkspace]frontmostApplication];
       pid_t pid = [app processIdentifier];
       appElem = AXUIElementCreateApplication(pid);
       
       if (!appElem)
       {
           error("error creating appElem for process");
           return ;
       }
       
       if (AXUIElementCopyAttributeValue (appElem, kAXFocusedWindowAttribute, (CFTypeRef*)&window) != kAXErrorSuccess)
       {
           error("error getting window");
           if(appElem)
                CFRelease(appElem);
           return ;
       }
       
       if(appElem)
       CFRelease(appElem);
       
      
    post("window number for the frontmost window: %i", globalForegroundWindow_getWindowNumberForAXUIElementRef(window, pid) );
    //works only for the windows of our application
}




#pragma mark SET
void globalForegroundWindow_set(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    // check if we have argument/s
       if (!argc)
       {
           object_error((t_object*)x,"you need at least one arg");
           object_error((t_object*)x,"'set <window number>' or");
           object_error((t_object*)x,"'set <app name>' or");
           object_error((t_object*)x,"'set <app name> <window title>'");
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
    /*
     //AXUIElementRef systemWideElement = AXUIElementCreateSystemWide();
     //SetFrontProcess(&psn); // this works in sync but is deprecated
     // [app activateIgnoringOtherApps:YES];// goto definition (appkit)
     //[window orderFrontRegardless];
     //[app activateWithOptions:NSApplicationActivateIgnoringOtherApps];// seems to be async
     //AXUIElementSetAttributeValue(appElem, kAXMainAttribute, kCFBooleanTrue);
     */
    
    
    int titleInput = false;
    if (argc>1) titleInput = true;
    
    char * appNameToMatch = NULL;
    char * titleToMatch = NULL;
    
    // get the string from arg 1
    appNameToMatch = calloc( strlen(atom_getsym(argv)->s_name)+1 ,sizeof(char) );
    strcpy(appNameToMatch,atom_getsym(argv)->s_name);
    
    if (titleInput)
    {
        // get the string from arg 2
        titleToMatch = calloc( strlen(atom_getsym(argv+1)->s_name)+1 ,sizeof(char) );
        strcpy(titleToMatch,atom_getsym(argv+1)->s_name);
    }
    
    // get all running apps
    NSArray *runningApps=[[NSWorkspace sharedWorkspace]runningApplications];
    int done = false;
    int appFound = false;
    
    // iterate running apps
    for (NSRunningApplication* app  in runningApps)
    {
        if (done) break; // we already found the app, exit loop
        
        pid_t pid = [app processIdentifier];
        AXUIElementRef appElem = AXUIElementCreateApplication(pid);
        //CFStringRef appName = NULL;
        //char* conformAppName = NULL;
        
        // GET PATH AND NAME OF APP
        int ret;
        //char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        char namebuf[PROC_PIDPATHINFO_MAXSIZE];
        
        ret = proc_name(pid, namebuf, sizeof(namebuf));
        if ( ret <= 0 )
        {
            ;//seems to happen for some apps
            //object_error((t_object *)x, "error getting process name");
        }
        
        // get the app name
        //AXUIElementCopyAttributeValue (appElem, kAXTitleAttribute, ((CFTypeRef*)&appName));
        
        if (namebuf)
        {
            //conformAppName = globalForegroundWindow_MYCFStringCopyUTF8String(appName);
            
            // we got the app name now compare with user input
            if ( !strcmp(namebuf, appNameToMatch))
            {
                // this app matches the name we are looking for
                appFound = true;
                
                if (titleInput)
                {
                    // now look for the window with the given title
                    
                    CFArrayRef windows = NULL;
                    
                    // get all windows of the app
                    if (AXUIElementCopyAttributeValues(appElem, kAXWindowsAttribute, 0, 256, &windows) == kAXErrorSuccess)
                    {
                        // we have the windows, iterate
                    
                        for (NSInteger i = 0; i < CFArrayGetCount(windows); i++)
                        {
                            AXUIElementRef window = NULL;
                            window = CFArrayGetValueAtIndex(windows, i);
                            
                            //we got the window, get title
                            CFStringRef title = NULL;
                            AXUIElementCopyAttributeValue (window, kAXTitleAttribute, ((CFTypeRef*)&title) );
                            if (title)
                            {
                                char* conformTitle = NULL;
                                conformTitle = globalForegroundWindow_MYCFStringCopyUTF8String(title);
                                if ( !strcmp(conformTitle, titleToMatch) )
                                {
                                    // it's a match // bring to front
                                    
                                      if (AXUIElementSetAttributeValue(window, (CFStringRef)NSAccessibilityMainAttribute, kCFBooleanTrue) != kAXErrorSuccess)
                                      {
                                        object_error((t_object*)x,"Could not change focus to window with title: %s", titleToMatch);
                                      }
                                      ProcessSerialNumber psn;
                                      GetProcessForPID(pid, &psn);
                                      SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
                                      
                                      done = true;
                                       
                                }
                                
                                if (conformTitle)
                                    free(conformTitle);
                                
                            } //window has no title or error getting it
                            
                            if(title) CFRelease(title);
                            //if(window) CFRelease(window); //don't release single items of array
                            
                            if(done)break;
                            
                        } // stopped iterating windows of this app
                        
                        if (!done) object_error((t_object*)x,"can't find app with name: %s and title: %s", appNameToMatch, titleToMatch);

                    } else object_error((t_object*)x,"can't find app with name: %s and title: %s", appNameToMatch, titleToMatch);
            
                    if(windows) CFRelease(windows); // release all windows
                    
                }else // no title input, just bring app to front
                {
                    ProcessSerialNumber psn;
                    GetProcessForPID(pid, &psn);
                    SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
                    // DEPRECATED but with accessibility APi we cannot "bring to front" in sync
                    // I'm gonna use it until it breaks
                    done = true;
                }
                
            } // name of app doesn't match
                                    
        } // could not get app name
        
        

        if(appElem)
            CFRelease(appElem);
        
    } // stopped iterating runningApps
    
    if (!done && !appFound) object_error((t_object*)x,"can't find app with name: %s", appNameToMatch);
    
    if(appNameToMatch)
    free(appNameToMatch);
    
    if(titleToMatch)
    free(titleToMatch);
    
}

void globalForegroundWindow_do_setWithNum(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    int inputNumber = atom_getlong(argv);
    int done = false;
    int wc=0;
    
    // get all running apps
    NSArray *runningApps=[[NSWorkspace sharedWorkspace]runningApplications];

    // iterate running apps
    for (NSRunningApplication* app  in runningApps)
    {
     
        pid_t pid = [app processIdentifier];
        AXUIElementRef appElem = AXUIElementCreateApplication(pid);

        CFArrayRef windows = NULL;

        // get all windows of the app
        if (AXUIElementCopyAttributeValues(appElem, kAXWindowsAttribute, 0, 1024, &windows) == kAXErrorSuccess)
        {

            // iterate windows
            for (NSInteger i = 0; i < CFArrayGetCount(windows); i++)
            {
                wc++; // window count
                if (wc==inputNumber)
                {
                     // we found the window that we want to activate
                     AXUIElementRef window = NULL;
                     window = CFArrayGetValueAtIndex(windows, i);
                     
                     if (AXUIElementSetAttributeValue(window, (CFStringRef)NSAccessibilityMainAttribute, kCFBooleanTrue) != kAXErrorSuccess)
                     {
                       object_error((t_object*)x,"Could not change focus to window with num: %i", inputNumber);
                     }
                     
                     ProcessSerialNumber psn;
                     GetProcessForPID(pid, &psn);
                     SetFrontProcessWithOptions(&psn, kSetFrontProcessFrontWindowOnly);
                     // DEPRECATED but with accessibility APi we cannot "bring to front" in sync
                     // I'm gonna use it until it breaks
                     done = true;
                     break;
                 
                }

            } // stopped iterating windows of this app
         
        } // no windows

        if(windows) CFRelease(windows); // release all windows
        if(appElem)CFRelease(appElem);

        if (done) break;
        
    } // stopped iterating runningApps

    if (!done)object_error((t_object*)x,"Could not find the window for num: %i", inputNumber);
    //outlet_anything(x->outdump, gensym("getwindows_end"), 0, NULL);
    
}

#pragma mark ISRUNNING
void globalForegroundWindow_isrunning(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_isrunning, s, argc, argv);
}
void globalForegroundWindow_do_isrunning(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    t_atom as[3];
    char * appNameToMatch = NULL;
    if (!argc) return;
    
    // get the string from arg 1
    appNameToMatch = calloc( strlen(atom_getsym(argv)->s_name)+1 ,sizeof(char) );
    strcpy(appNameToMatch,atom_getsym(argv)->s_name);
    
    // get all running apps
    NSArray *runningApps=[[NSWorkspace sharedWorkspace]runningApplications];
    int appFound = false;
    
    // iterate running apps
    for (NSRunningApplication* app  in runningApps)
    {
        pid_t pid = [app processIdentifier];
       
        // GET NAME OF APP
        int ret;
        char namebuf[PROC_PIDPATHINFO_MAXSIZE];
        ret = proc_name(pid, namebuf, sizeof(namebuf));
        
        if (namebuf)
        {
            // we got the app name now compare with user input
            if ( !strcmp(namebuf, appNameToMatch))
            {
                // this app matches the name we are looking for !!
                appFound = true;
                break;
                
            } // name of app doesn't match
        } // could not get app name
    } // stopped iterating runningApps
    
    atom_setsym(as, gensym("isrunning")); 
    atom_setsym(as+1, atom_getsym(argv)); //use input appname sym
    
    if (appFound)
        atom_setlong(as+2, 1);
    else
        atom_setlong(as+2, 0);
    
    outlet_list(x->outdump, 0L, 3, as);
    
    if(appNameToMatch)
    free(appNameToMatch);
}


#pragma mark QUITAPP
void globalForegroundWindow_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_quitapp, s, argc, argv);
}
void globalForegroundWindow_do_quitapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    if (!argc)
    {
        NSRunningApplication* app = [[NSWorkspace sharedWorkspace]frontmostApplication];
        [app terminate];
        return;
    }
    
    // we have an arg
    char * appNameToMatch = NULL;
    // get the string from arg 1
    appNameToMatch = calloc( strlen(atom_getsym(argv)->s_name)+1 ,sizeof(char) );
    strcpy(appNameToMatch,atom_getsym(argv)->s_name);
    
    NSRunningApplication* app = globalForegroundWindow_getRunningAppPtrForName(appNameToMatch);
    
    if (app)
        [app terminate];
    else
        object_error((t_object*)x,"Could not find running app with name: %s", appNameToMatch);
    
    if(appNameToMatch)
    free(appNameToMatch);
}


#pragma mark KILLAPP
void globalForegroundWindow_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_killapp, s, argc, argv);
}
void globalForegroundWindow_do_killapp(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    if (!argc)
    {
        NSRunningApplication* app = [[NSWorkspace sharedWorkspace]frontmostApplication];
        [app forceTerminate];
        return;
    }
    
    // we have an arg
    char * appNameToMatch = NULL;
    // get the string from arg 1
    appNameToMatch = calloc( strlen(atom_getsym(argv)->s_name)+1 ,sizeof(char) );
    strcpy(appNameToMatch,atom_getsym(argv)->s_name);
    
    NSRunningApplication* app = globalForegroundWindow_getRunningAppPtrForName(appNameToMatch);
    
    if (app)
        [app forceTerminate];
    else
        object_error((t_object*)x,"Could not find running app with name: %s", appNameToMatch);
    
    if(appNameToMatch)
    free(appNameToMatch);
}


#pragma mark AGENT
void globalForegroundWindow_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_agent, s, argc, argv);
}
void globalForegroundWindow_do_agent(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    if (!argc){ object_error((t_object*)x,"you need an integer arg ");return;}
    if (atom_gettype(argv)!=A_LONG){ object_error((t_object*)x,"your arg is not an integer");return;}
    int newState = atom_getlong(argv);
    if (newState)
    {
        //switch on agent mode
        [[NSApplication sharedApplication]setActivationPolicy:NSApplicationActivationPolicyAccessory];
        if(newState!=11)
            [[NSApplication sharedApplication]activateIgnoringOtherApps:YES];
    }
    else
    {
        [[NSApplication sharedApplication]setActivationPolicy:NSApplicationActivationPolicyRegular];
        //[[NSApplication sharedApplication]setPresentationOptions:(NSApplicationPresentationDefault)] ;
        
        //reactivate menu bar // without this, we have a non responding menu bar
        NSArray *docks = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.loginwindow"];
        NSRunningApplication *dock = [docks firstObject];
        [dock activateWithOptions:nil];
        
        // seems to come earlier than dock activation
        //[[NSApplication sharedApplication]activateIgnoringOtherApps:YES];
        
        //this works
        [[[NSWorkspace sharedWorkspace]frontmostApplication]activateWithOptions:NSApplicationActivateIgnoringOtherApps];
        
    }
}



#pragma mark TOPMOST
void globalForegroundWindow_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_topmost, s, argc, argv);
}
void globalForegroundWindow_do_topmost(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    // only works for windows of our own process on osx
    
    if (!argc){ object_error((t_object*)x,"you need an integer arg ");return;}
    int newState = atom_getlong(argv);
    
    if (![[NSApplication sharedApplication] isActive])
    {
        object_error((t_object*)x,"topmost only works for our own windows");
        return;
    }
    
    // the main window of our application
    NSWindow* window = [[NSApplication sharedApplication] mainWindow];
    
      
    if (newState == 1)
    {
        
        //kCGModalPanelWindowLevel;//kCGFloatingWindowLevel;//NSStatusWindowLevel;
        // when I set level higher than kCGModalPanelWindowLevel, i can't reset the level of the window back to normal
        //| NSWindowCollectionBehaviorFullScreenAuxiliary; // using this you can't reset the poping up at fullscreens behavior
        
        window.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces;
        window.level = kCGModalPanelWindowLevel;
        [window orderFront:nil];
     
    }
    else if (newState==0)
    {
        window.level = kCGNormalWindowLevel;
        window.collectionBehavior = NSWindowCollectionBehaviorDefault;
    }
    else if (newState==-1)
    {
        window.level = kCGNormalWindowLevel-1000;
        window.collectionBehavior = NSWindowCollectionBehaviorCanJoinAllSpaces;
    }
    
      
}



#pragma mark FULLSCREEN
void globalForegroundWindow_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    defer_low(x, (method)globalForegroundWindow_do_fullscreen, s, argc, argv);
}
void globalForegroundWindow_do_fullscreen(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    if (!argc){ object_error((t_object*)x,"you need an integer arg ");return;}
    int newState = atom_getlong(argv);
    
    AXUIElementRef window = globalForegroundWindow_getFocusedWindow();
    
    if (!window)
    {
        object_error((t_object *)x,"error getting focused window");
        return;
    }
    // We have the focused window
    
    int r;
    
    if (newState) r = AXUIElementSetAttributeValue(window, CFSTR("AXFullScreen") , kCFBooleanTrue );
    else r = AXUIElementSetAttributeValue(window, CFSTR("AXFullScreen") , kCFBooleanFalse );
    
    if (r) object_error((t_object *)x,"failed to switch fullscreen-state, check if the window is 'topmost 0' and Max is 'agent 0'");
    
    if(window)
    CFRelease(window);
}


#pragma mark MINIMIZE
void globalForegroundWindow_minimize(t_globalForegroundWindow *x)
{
    defer_low(x, (method)globalForegroundWindow_do_minimize, NULL, NULL, NULL);
}
void globalForegroundWindow_do_minimize(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
   
    AXUIElementRef window = globalForegroundWindow_getFocusedWindow();
                
    if (!window)
    {
        object_error((t_object *)x,"error getting focused window");
        return;
    }
    // We have the focused window

    
    int r;
    
    //
    r = AXUIElementSetAttributeValue(window, kAXMinimizedAttribute , kCFBooleanTrue ); //kAXMinimizedAttribute
    //
    if (r) object_error((t_object *)x,"failed to switch minimized-state");
    
    if(window)
    CFRelease(window);
}


#pragma mark CLOSE
void globalForegroundWindow_close(t_globalForegroundWindow *x)
{
    defer_low(x, (method)globalForegroundWindow_do_close, NULL, NULL, NULL);
}
void globalForegroundWindow_do_close(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    AXUIElementRef window = globalForegroundWindow_getFocusedWindow();
                
    if (!window)
    {
        object_error((t_object *)x,"error getting focused window");
        return;
    }
    // We have the focused window
    
    
    if (!globalForegroundWindow_pressButtonClose(window))
        object_error((t_object *)x,"failed to close window");
    
    // fallback
    globalForegroundWindow_pressButtonCancel(window);
    
    
    if(window)
    CFRelease(window);
}



void globalForegroundWindow_int(t_globalForegroundWindow *x, long m)
{
    if (m) defer_low(x, (method)globalForegroundWindow_start, NULL, 0, NULL);
    else defer_low(x, (method)globalForegroundWindow_stop, NULL, 0, NULL);
}


#pragma mark BANG
void globalForegroundWindow_bang(t_globalForegroundWindow *x)
{
    defer_low(x, (method)globalForegroundWindow_do_bang, NULL, 0, NULL);
}
void globalForegroundWindow_do_bang(t_globalForegroundWindow *x, t_symbol *s, long argc, t_atom *argv)
{
    NSRunningApplication* app = [[NSWorkspace sharedWorkspace]frontmostApplication];
    
    pid_t pid = [app processIdentifier];
    
    AXUIElementRef appElem = AXUIElementCreateApplication(pid);
    
    if (!appElem)
    {
        object_error((t_object *)x, "error creating appElem");
        return;
    }
    
    // GET PATH AND NAME OF APP
    int ret;
    char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
    char namebuf[PROC_PIDPATHINFO_MAXSIZE];
    
    ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
    if ( ret <= 0 )
    {
        object_error((t_object *)x, "error getting process path");
    }
    ret = proc_name(pid, namebuf, sizeof(namebuf));
    if ( ret <= 0 )
    {
        object_error((t_object *)x, "error getting process name");
    }
    
        
    AXUIElementRef window = NULL;
    CFStringRef title = NULL;
    char* conformTitle = NULL;
    
    AXValueRef pos = NULL;
    AXValueRef size = NULL;
    CGPoint cpoint = CGPointMake(0, 0);
    CGSize csize = CGSizeMake(0, 0);
    
    
    // getting the focused window of the app
    if (AXUIElementCopyAttributeValue (appElem, kAXFocusedWindowAttribute, (CFTypeRef*)&window) != kAXErrorSuccess)
    {
        object_error((t_object *)x,"error getting window");
        goto free; // if we not even have the window we can't go on
    }
    
    
    if (window)
    {
        AXUIElementCopyAttributeValue(window, kAXPositionAttribute, (CFTypeRef *)&pos);
        AXUIElementCopyAttributeValue(window, kAXSizeAttribute, (CFTypeRef *)&size);
    }
    if (pos)
    {
        AXValueGetValue(pos, kAXValueCGPointType, &cpoint);
        CFRelease(pos);
    }
    if (size)
    {
        AXValueGetValue(size, kAXValueCGSizeType, &csize);
        CFRelease(size);
    }
    
    //post("point %f %f %f %f", cpoint.x, cpoint.y, csize.width, csize.height);
    // kAXPositionAttribute, kAXSizeAttribute
    
    
    if(AXUIElementCopyAttributeValue(window, kAXTitleAttribute, (CFTypeRef*)&title)!= kAXErrorSuccess)
    {
        // that happens e.g. clicking dektop window
        
        //object_error((t_object *)x,"Problem Copying title");
        
        goto free;
    }
    conformTitle = globalForegroundWindow_MYCFStringCopyUTF8String(title);
    //object_post((t_object*)x,conformTitle);
    
    
free:
    
    outlet_anything(x->outd, gensym(namebuf), 0, NULL);
    outlet_anything(x->outc, gensym(pathbuf), 0, NULL);
    
    if(conformTitle)
    {
        outlet_anything(x->outb, gensym(conformTitle), 0, NULL);
        free(conformTitle);
    }else
        outlet_anything(x->outb, emptySym, 0, NULL);
     
    atom_setlong(x->winLoc + 0, cpoint.x);
    atom_setlong(x->winLoc + 1, cpoint.y);
    atom_setlong(x->winLoc + 2, csize.width);
    atom_setlong(x->winLoc + 3, csize.height);
    outlet_list(x->outa, NULL, 4, x->winLoc);
    
    
    
    if(title)
    CFRelease(title);
    
    if(window)
    CFRelease(window);
    
    
    if(appElem)
    CFRelease(appElem);
    
    return;
}
            
 
        
       


#pragma mark My Helpers

AXUIElementRef globalForegroundWindow_getFocusedWindow()
{
    AXUIElementRef window = NULL;
    AXUIElementRef appElem = NULL;

    NSRunningApplication* app = [[NSWorkspace sharedWorkspace]frontmostApplication];
    pid_t pid = [app processIdentifier];
    appElem = AXUIElementCreateApplication(pid);
    
    if (!appElem)
    {
        //error("error creating appElem for process");
        return NULL;
    }
    
    if (AXUIElementCopyAttributeValue (appElem, kAXFocusedWindowAttribute, (CFTypeRef*)&window) != kAXErrorSuccess)
    {
        //error("error getting window");
        if(appElem)
             CFRelease(appElem);
        return NULL;
    }
    
    if(appElem)
    CFRelease(appElem);
    
    // We have the frontmost window
    return window;
}


NSRunningApplication* globalForegroundWindow_getRunningAppPtrForName(char* appNameToMatch)
{
    // get all running apps
   NSArray *runningApps=[[NSWorkspace sharedWorkspace]runningApplications];
   
   // iterate running apps
   for (NSRunningApplication* app  in runningApps)
   {
       pid_t pid = [app processIdentifier];
       
       char namebuf[PROC_PIDPATHINFO_MAXSIZE];
       proc_name(pid, namebuf, sizeof(namebuf));
       
       // we got the app name now compare with user input
       if ( !strcmp(namebuf, appNameToMatch))
       {
           // this app matches the name we are looking for
           return app;
       }
   }
   return NULL;
}


// return screen number on which the window is fullscreen on or 0
int globalForegroundWindow_isFullscreen(AXUIElementRef window)
{

    
    // get rect of window
    AXValueRef pos = NULL;
    AXValueRef size = NULL;
    CGPoint cpoint = CGPointMake(0, 0);
    CGSize csize = CGSizeMake(0, 0);
    
    AXUIElementCopyAttributeValue(window, kAXPositionAttribute, (CFTypeRef *)&pos);
    AXUIElementCopyAttributeValue(window, kAXSizeAttribute, (CFTypeRef *)&size);
    
    if (pos && size)
    {
        AXValueGetValue(pos, kAXValueCGPointType, &cpoint);
        AXValueGetValue(size, kAXValueCGSizeType, &csize);
        
        CFRelease(pos);
        CFRelease(size);
    }
    else return -1;
    
    //CGRect aRect = CGRectMake(aPoint.x, aPoint.y, aSize.width, aSize.height);
    CGRect windowBounds = { cpoint, csize };
    
    //iterate screens
    NSEnumerator *screenEnum = [[NSScreen screens] objectEnumerator];
    NSScreen *screen;
    int screenCount = 0;
    while (screen = [screenEnum nextObject])
    {
        screenCount ++;
        
        /*post("point %f %f %f %f",
        [screen frame].origin.x,
        [screen frame].origin.y,
        [screen frame].size.width,
        [screen frame].size.height );*/
        
        // check if window already has fullsize of one of the screens
        if (CGRectEqualToRect([screen frame], windowBounds))
        {
            
            
            return screenCount;
        }
        
    }
    
    // window is not fullscreen on a screen
    return 0;
}


int globalForegroundWindow_pressButtonFullscreen(AXUIElementRef window)
{
    // perform button action
    AXUIElementRef buttonRef = nil;
    
    if (AXUIElementCopyAttributeValue(window, kAXFullScreenButtonAttribute, (CFTypeRef*)&buttonRef) != kAXErrorSuccess)
    {
        return 0;
    }
    if (AXUIElementPerformAction(buttonRef, kAXPressAction) != kAXErrorSuccess)
    {
        if (buttonRef)
        CFRelease(buttonRef);
        return 0;
    }
    
    if (buttonRef)
    CFRelease(buttonRef);
    
    return 1;
}

int globalForegroundWindow_pressButtonClose(AXUIElementRef window)
{
    // perform button action
    AXUIElementRef buttonRef = nil;
    
    if (AXUIElementCopyAttributeValue(window, kAXCloseButtonAttribute, (CFTypeRef*)&buttonRef) != kAXErrorSuccess)
    {
        return 0;
    }
    if (AXUIElementPerformAction(buttonRef, kAXPressAction) != kAXErrorSuccess)
    {
        if (buttonRef)
        CFRelease(buttonRef);
        return 0;
    }
    
    if (buttonRef)
    CFRelease(buttonRef);
    
    return 1;
}

int globalForegroundWindow_pressButtonCancel(AXUIElementRef window)
{
    // perform button action
    AXUIElementRef buttonRef = nil;
    
    if (AXUIElementCopyAttributeValue(window, kAXCancelButtonAttribute, (CFTypeRef*)&buttonRef) != kAXErrorSuccess)
    {
        return 0;
    }
    if (AXUIElementPerformAction(buttonRef, kAXPressAction) != kAXErrorSuccess)
    {
        if (buttonRef)
        CFRelease(buttonRef);
        return 0;
    }
    
    if (buttonRef)
    CFRelease(buttonRef);
    
    return 1;
}


int globalForegroundWindow_getWindowNumberForAXUIElementRef(AXUIElementRef inputWindow, pid_t pid)
{
    /* Unused function.
     The goal here is to get a unique window number that core graphics provides in it's window list.
     We only have a relative reference, an AXUIElementRef from the Accessibility Api. We can only
     make a connection by comparing procPid, title, and rect of both items.
     But since 10.15.1 this door is closed - You can't get the titles of other apps windows with CG
     if the user hasn't granted "Screen Recording" permissions.
     So this would be the second permission one has to give for this external.
     */
    
    NSMutableArray *windows = (NSMutableArray *)CGWindowListCopyWindowInfo(kCGWindowListExcludeDesktopElements , kCGNullWindowID); //kCGWindowListOptionOnScreenOnly
   
    CFStringRef title = NULL;
    AXUIElementCopyAttributeValue((AXUIElementRef)inputWindow, kAXTitleAttribute, (CFTypeRef*)&title);
    char* conformTitle = NULL;
    conformTitle = globalForegroundWindow_MYCFStringCopyUTF8String(title);
    if(title) CFRelease(title);
    
    AXValueRef pos = NULL;
    AXValueRef size = NULL;
    CGPoint cpoint = CGPointMake(0, 0);
    CGSize csize = CGSizeMake(0, 0);
    AXUIElementCopyAttributeValue(inputWindow, kAXPositionAttribute, (CFTypeRef *)&pos);
    AXUIElementCopyAttributeValue(inputWindow, kAXSizeAttribute, (CFTypeRef *)&size);
    if (pos)
    {
        AXValueGetValue(pos, kAXValueCGPointType, &cpoint);
        CFRelease(pos);
    }
    if (size)
    {
        AXValueGetValue(size, kAXValueCGSizeType, &csize);
        CFRelease(size);
    }
    CGRect inputRect = { cpoint, csize };
    
    post("searched title: %s", conformTitle);
    
    for (NSDictionary *window in windows) //iterate all windows
    {
        /*
        kCGWindowIsOnscreen
        kCGWindowLayer
        kCGWindowMemoryUsage
        kCGWindowName
        kCGWindowNumber
        kCGWindowOwnerName
        kCGWindowOwnerPID
        kCGWindowSharingState
        kCGWindowStoreType */
        
        NSNumber* thisPid = (NSNumber*)[window objectForKey:@"kCGWindowOwnerPID"];
        
        if ([thisPid intValue] == pid)
        {
            
            NSString *name = [window objectForKey:@"kCGWindowName" ];
            //post("pid is matching, found title: %s",[name UTF8String]);
            
            if ( (!name && conformTitle) ||
                (name && !conformTitle)  )
            {
                //post("one string is NULL");
                continue;
            }
            
            
            if( (!name && !conformTitle) || !strcmp([name UTF8String], conformTitle) )
            {
                //post("title is matching");
                // match rects
                CGRect windowRect;
                CGRectMakeWithDictionaryRepresentation((__bridge CFDictionaryRef)[window objectForKey:@"kCGWindowBounds" ], &windowRect);
                
//                post("point %f %f %f %f",
//                windowRect.origin.x,
//                windowRect.origin.y,
//                windowRect.size.width,
//                windowRect.size.height );
                
                if (CGRectEqualToRect(windowRect, inputRect))
                {
                    // we have a full match
                    //post("found a matching window"   );
                    
                    if (conformTitle) free(conformTitle);
                    NSNumber *wn = [window objectForKey:@"kCGWindowNumber" ];
                    return [wn intValue];
                }
                
            }
            
        }
        
    }// end for loop
    
    if (conformTitle) free(conformTitle);
    return NULL;
}


AXUIElementRef globalForegroundWindow_getAXUIElementRefForWindowNumber(int windowNumber)
{
    return NULL;
}




// we have to free the result
char * globalForegroundWindow_MYCFStringCopyUTF8String(CFStringRef aString) {
  if (aString == NULL) {
    return NULL;
  }

  CFIndex length = CFStringGetLength(aString);
  CFIndex maxSize =
  CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1;
  char *buffer = (char *)malloc(maxSize);
  if (CFStringGetCString(aString, buffer, maxSize,
                         kCFStringEncodingUTF8)) {
    return buffer;
  }
  free(buffer); // If we failed
  return NULL;
}




#pragma mark MyOBSERVER CLASS
@interface myObserver : NSObject {

    NSMutableDictionary     *_observers;
    pid_t                    _currentPid;
    AXObserverRef _observer; // the current app observer
    CFRunLoopRef _event_loop;
    t_globalForegroundWindow *_x;
    
}
    



@end

@implementation myObserver




 #pragma mark app_callback_in_C
 static void applicationSwitched(AXObserverRef observer, AXUIElementRef element, CFStringRef notification, void *self)
 {
     [(id)self applicationSwitched]; //call the objC method
 }
 - (void)applicationSwitched
 {
     //post ("app callback thread: %i", [NSThread currentThread]);
     defer_low(_x, (method)globalForegroundWindow_do_bang, NULL, 0, NULL);
 }


 

#pragma mark start

- (void)start:(t_globalForegroundWindow *)y;
{
    _x = y;
    _observer = nil;
    _event_loop = CFRunLoopGetCurrent();

    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    
    /* Register for application launch notifications */
    [[workspace notificationCenter] addObserver:self
                                       selector:@selector(didActivateApplication:)
                                           name:NSWorkspaceDidActivateApplicationNotification
                                         object:workspace];
    
   
    [self didActivateApplication:nil];
 
}



#pragma mark workspace_callback
- (void) didActivateApplication:(NSNotification *)notification
{
    defer_low(_x, (method)globalForegroundWindow_do_bang, NULL, 0, NULL);
    
    //we are probably not in our thread!!
    //post("didActivateApplication-callback. Thread: %i ", [NSThread currentThread]);
    
    /*
     Hmm, sounds like I don't need to care about the object being released
     while the async code block is processed
     
     from:
     https://stackoverflow.com/questions/11915030/modifying-data-from-parent-scope-inside-a-dispatch-queue
     "That's why blocks have another feature - all objects that are referenced inside a block will be retained for lifetime of a block to ensure that object will not be deallocated unexpectedly."
        
     */
    
    
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_HIGH, 0),
    ^{
        //post("async block. Thread: %i ", [NSThread currentThread]);
           
        
        // releasing the AXObserverRef automatically removes
        // the run loop source from the run loop
        if (_observer != nil)
        {
            CFRelease(_observer);
            _observer = nil;
        }
        
        NSRunningApplication* app = [[NSWorkspace sharedWorkspace]frontmostApplication];
        pid_t pid = [app processIdentifier];
        
        if(AXObserverCreate(pid, applicationSwitched, &_observer) != kAXErrorSuccess)
        {
            int ret;
            char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
            ret = proc_pidpath (pid, pathbuf, sizeof(pathbuf));
            
            error("We could not create an observer to watch this application: %s",
                  pathbuf);
           
            return;
        }
        
        AXUIElementRef element = AXUIElementCreateApplication(pid);
        
        
        AXObserverAddNotification( _observer, element, kAXMovedNotification, self );
        AXObserverAddNotification( _observer, element, kAXResizedNotification, self );
        AXObserverAddNotification( _observer, element, kAXTitleChangedNotification, self );
        AXObserverAddNotification( _observer, element, kAXFocusedWindowChangedNotification, self );
        
        CFRunLoopAddSource(_event_loop,AXObserverGetRunLoopSource(_observer),kCFRunLoopDefaultMode);

        CFRelease(element);

    });// end of block
}


#pragma mark deallocate resources
- (void)dealloc
{
    //[[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self.observer];
    
    /* Stop listening workspace notifications */
    [[[NSWorkspace sharedWorkspace] notificationCenter] removeObserver:self];
    
    // releasing the AXObserverRef automatically removes
    // the run loop source from the run loop
    if (_observer != nil)
    {
        CFRelease(_observer);
        _observer = nil;
    }
    
    // CFRunLoopRemoveSource(CFRunLoopGetCurrent(), AXObserverGetRunLoopSource(self.observer),kCFRunLoopDefaultMode);

    [super dealloc];
}


@end





#pragma mark START
void globalForegroundWindow_start(t_globalForegroundWindow *x)
{
    if (x->x_systhread == NULL)
    {
        //object_post((t_object *)x, "starting a new thread");
        systhread_create((method)globalForegroundWindow_threadproc, x, 0, 1, 0, &x->x_systhread);
    }
    else
    {
        object_error((t_object *)x, "thread is still running");
    }
    
}



#pragma mark THREADPROC
void *globalForegroundWindow_threadproc(t_globalForegroundWindow *x)
{
    // post("Thread %i start...", [NSThread currentThread]); //(int)pthread_self() );
    
    // get the ref to be able to stop it
    x->event_loop = CFRunLoopGetCurrent();
    
    myObserver *ob1 = [[myObserver alloc]init]; // start workspace observer
    
    [ob1 start:x];
    
    
    // a channel to our runloop
    NSRunLoop *q = [NSRunLoop currentRunLoop];
    [q addPort:[NSMachPort port] forMode:NSDefaultRunLoopMode];
    
    
    CFRunLoopRun();

    
    [ob1 release];
 
    //post( "Thread end!");
    
    systhread_exit(0);
    return NULL;
}



#pragma mark STOP
void globalForegroundWindow_stop(t_globalForegroundWindow *x)
{
    unsigned int ret;
    
    if (x->event_loop)
    {

    
        // Stop the run loop.
        if (x->event_loop)
            CFRunLoopStop(x->event_loop);
    
        //CFRelease(x->event_loop); // don't release !!
        x->event_loop = NULL;
    }
    
    if (x->x_systhread)
    {
        systhread_join(x->x_systhread, &ret);
        x->x_systhread = NULL;
    }
    
}


#pragma mark FREE
void globalForegroundWindow_free(t_globalForegroundWindow *x)
{
    globalForegroundWindow_stop(x);
}

