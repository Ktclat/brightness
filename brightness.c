#define UNICODE
#define _UNICODE
#include <windows.h>
#include <physicalmonitorenumerationapi.h>
#include <highlevelmonitorconfigurationapi.h>
#include <lowlevelmonitorconfigurationapi.h>
#include <dbt.h>
#include <shellapi.h>

/* Native tray utility; no CRT or third-party runtime. Settings live beside EXE. */
void *memset(void *p,int c,size_t n){volatile unsigned char *b=p;while(n--)*b++=(unsigned char)c;return p;}
void *memcpy(void *p,const void *q,size_t n){unsigned char *b=p;const unsigned char *s=q;while(n--)*b++=*s++;return p;}
static const WCHAR CLASS_NAME[]=L"Brightness.Native";
static const WCHAR LEGACY_CLASS_NAME[]=L"Z1SilentBrightness.Native";
static WCHAR exe[MAX_PATH],ini[MAX_PATH];
static HANDLE stop_event,wake_event;
static int target=100,check_only=0,found=0,success=0,reading=201;
static NOTIFYICONDATAW tray;
static HICON tray_icon;
static UINT taskbar_created;
static int auto_enabled,menu_open;
#define TRAY_MESSAGE (WM_APP+1)

static int configured_brightness(void){int n=(int)GetPrivateProfileIntW(L"Settings",L"Brightness",100,ini);return n>=0&&n<=100?n:100;}
static BOOL remove_startup_entries(int include_current){
    const WCHAR *paths[]={L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run",L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run32"};
    const WCHAR *names[]={L"Brightness",L"Z1SilentBrightness",L"PortableBrightnessZ1"};
    HKEY key;LSTATUS status;int p,n;BOOL ok=TRUE;
    for(p=0;p<3;p++){
        status=RegOpenKeyExW(HKEY_CURRENT_USER,paths[p],0,KEY_SET_VALUE,&key);
        if(status==ERROR_FILE_NOT_FOUND)continue;
        if(status!=ERROR_SUCCESS){ok=FALSE;continue;}
        for(n=include_current?0:1;n<3;n++){
            status=RegDeleteValueW(key,names[n]);
            if(status!=ERROR_SUCCESS&&status!=ERROR_FILE_NOT_FOUND)ok=FALSE;
        }
        RegCloseKey(key);
    }
    return ok;
}
static BOOL startup_registry(int enabled){
    HKEY key;WCHAR command[MAX_PATH+16];LSTATUS result;
    if(!enabled)return remove_startup_entries(1);
    if(!remove_startup_entries(0))return FALSE;
    result=RegCreateKeyExW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",0,NULL,0,KEY_SET_VALUE,NULL,&key,NULL);
    if(result!=ERROR_SUCCESS)return FALSE;
    wsprintfW(command,L"\"%s\"",exe);result=RegSetValueExW(key,L"Brightness",0,REG_SZ,(const BYTE*)command,(lstrlenW(command)+1)*sizeof(WCHAR));
    RegCloseKey(key);return result==ERROR_SUCCESS;
}
static void tray_update(void){
    wsprintfW(tray.szTip,L"Brightness - 默认亮度 %d%%",configured_brightness());
    tray.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP|NIF_SHOWTIP;
    Shell_NotifyIconW(NIM_MODIFY,&tray);
}
static void tray_add(HWND hwnd){
    HKEY key;DWORD light=0,size=sizeof(light);HICON previous=tray_icon;
    if(RegOpenKeyExW(HKEY_CURRENT_USER,L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",0,KEY_QUERY_VALUE,&key)==ERROR_SUCCESS){RegQueryValueExW(key,L"SystemUsesLightTheme",NULL,NULL,(BYTE*)&light,&size);RegCloseKey(key);}
    tray_icon=(HICON)LoadImageW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(light?3:2),IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
    memset(&tray,0,sizeof(tray));tray.cbSize=sizeof(tray);tray.hWnd=hwnd;tray.uID=1;
    tray.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP|NIF_SHOWTIP;tray.hIcon=tray_icon;tray.uCallbackMessage=TRAY_MESSAGE;
    wsprintfW(tray.szTip,L"Brightness - 默认亮度 %d%%",configured_brightness());
    if(!Shell_NotifyIconW(NIM_ADD,&tray))Shell_NotifyIconW(NIM_MODIFY,&tray);
    tray.uVersion=NOTIFYICON_VERSION_4;Shell_NotifyIconW(NIM_SETVERSION,&tray);
    if(previous)DestroyIcon(previous);
}
static void menu_error(void){
    tray.uFlags=NIF_INFO;lstrcpyW(tray.szInfoTitle,L"Brightness");
    lstrcpyW(tray.szInfo,L"无法保存设置或更新启动项，请确认软件文件夹可写入后重试。");
    tray.dwInfoFlags=NIIF_WARNING;Shell_NotifyIconW(NIM_MODIFY,&tray);tray_update();
}
static void show_menu(HWND hwnd){
    HMENU menu,presets;UINT command;POINT point;WCHAR text[64],value[16],folder[MAX_PATH];int n,current,i;
    const int levels[]={0,10,25,50,75,90,100};
    if(menu_open)return;menu=CreatePopupMenu();presets=CreatePopupMenu();
    if(!menu||!presets){if(menu)DestroyMenu(menu);if(presets)DestroyMenu(presets);return;}
    menu_open=1;current=configured_brightness();
    AppendMenuW(menu,MF_STRING|(auto_enabled?MF_CHECKED:0),1,L"开机自启");
    for(i=0;i<7;i++){wsprintfW(text,L"%d%%",levels[i]);AppendMenuW(presets,MF_STRING|(current==levels[i]?MF_CHECKED:0),100+levels[i],text);}
    wsprintfW(text,L"默认亮度：%d%%",current);AppendMenuW(menu,MF_POPUP,(UINT_PTR)presets,text);
    AppendMenuW(menu,MF_SEPARATOR,0,NULL);AppendMenuW(menu,MF_STRING,2,L"打开文件路径");
    AppendMenuW(menu,MF_STRING,4,L"删除启动注册信息");AppendMenuW(menu,MF_STRING,3,L"退出");
    GetCursorPos(&point);SetForegroundWindow(hwnd);
    command=TrackPopupMenuEx(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON,point.x,point.y,hwnd,NULL);
    PostMessageW(hwnd,WM_NULL,0,0);DestroyMenu(menu);menu_open=0;
    if(command==1){
        n=!auto_enabled;
        if(!startup_registry(n)){menu_error();return;}
        if(!WritePrivateProfileStringW(L"Settings",L"AutoStart",n?L"1":L"0",ini)){startup_registry(auto_enabled);menu_error();return;}
        auto_enabled=n;
    }else if(command==2){
        lstrcpyW(folder,exe);for(i=lstrlenW(folder);i>0;i--)if(folder[i-1]==L'\\'){folder[i]=0;break;}
        ShellExecuteW(hwnd,L"open",folder,NULL,NULL,SW_SHOWNORMAL);
    }else if(command==4){
        if(!WritePrivateProfileStringW(L"Settings",L"AutoStart",L"0",ini)){menu_error();return;}
        auto_enabled=0;
        if(!remove_startup_entries(1))menu_error();
    }else if(command==3)PostMessageW(hwnd,WM_CLOSE,0,0);
    else if(command>=100&&command<=200){
        wsprintfW(value,L"%u",command-100);
        if(!WritePrivateProfileStringW(L"Settings",L"Brightness",value,ini))menu_error();
        else SetEvent(wake_event);
    }
    tray_update();
}

static int contains(const WCHAR *text,const WCHAR *needle){
    const WCHAR *a,*b;while(*text){a=text++;b=needle;while(*b&&*a==*b){a++;b++;}if(!*b)return 1;}return 0;
}
static int has_arg(const WCHAR *arg){return contains(GetCommandLineW(),arg);}
static void load_target(void){
    target=(int)GetPrivateProfileIntW(L"Settings",L"Brightness",100,ini);
    if(target<0||target>100)target=100;
}
static BOOL CALLBACK visit(HMONITOR monitor,HDC dc,LPRECT rect,LPARAM unused){
    MONITORINFOEXW info;DISPLAY_DEVICEW device;DWORD count=0,i,type,current,maximum,minimum=0,value;
    PHYSICAL_MONITOR *physical;BOOL low,ok;
    (void)dc;(void)rect;(void)unused;
    memset(&info,0,sizeof(info));info.cbSize=sizeof(info);
    memset(&device,0,sizeof(device));device.cb=sizeof(device);
    if(!GetMonitorInfoW(monitor,(LPMONITORINFO)&info)||!EnumDisplayDevicesW(info.szDevice,0,&device,EDD_GET_DEVICE_INTERFACE_NAME))return TRUE;
    if(!contains(device.DeviceID,L"DISPLAY#DZX1560#"))return TRUE;
    found=1;
    if(!GetNumberOfPhysicalMonitorsFromHMONITOR(monitor,&count)||!count||count>32)return TRUE;
    physical=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,count*sizeof(*physical));
    if(!physical)return TRUE;
    if(GetPhysicalMonitorsFromHMONITOR(monitor,count,physical)){
        for(i=0;i<count;i++){
            minimum=0;
            low=GetVCPFeatureAndVCPFeatureReply(physical[i].hPhysicalMonitor,0x10,(LPMC_VCP_CODE_TYPE)&type,&current,&maximum);
            if(!low&&!GetMonitorBrightness(physical[i].hPhysicalMonitor,&minimum,&current,&maximum))continue;
            if(maximum<=minimum)continue;
            reading=(int)(((ULONGLONG)(current-minimum)*100+(maximum-minimum)/2)/(maximum-minimum));
            if(check_only){success=1;break;}
            value=minimum+(DWORD)(((ULONGLONG)(maximum-minimum)*target+50)/100);
            ok=low?SetVCPFeature(physical[i].hPhysicalMonitor,0x10,value):SetMonitorBrightness(physical[i].hPhysicalMonitor,value);
            if(!ok)continue;
            Sleep(150);
            ok=low?GetVCPFeatureAndVCPFeatureReply(physical[i].hPhysicalMonitor,0x10,(LPMC_VCP_CODE_TYPE)&type,&current,&maximum):GetMonitorBrightness(physical[i].hPhysicalMonitor,&minimum,&current,&maximum);
            if(ok&&maximum>minimum){
                reading=(int)(((ULONGLONG)(current-minimum)*100+(maximum-minimum)/2)/(maximum-minimum));
                if(reading>=target-2&&reading<=target+2){success=1;break;}
            }
        }
        DestroyPhysicalMonitors(count,physical);
    }
    HeapFree(GetProcessHeap(),0,physical);return !success;
}
static int apply(void){
    load_target();found=0;success=0;reading=201;
    EnumDisplayMonitors(NULL,NULL,visit,0);
    return success?(check_only?reading:0):(found?201:200);
}
static DWORD WINAPI worker(void *unused){
    HANDLE events[2];DWORD delays[4]={0,1000,2500,5000},i;
    (void)unused;events[0]=stop_event;events[1]=wake_event;
    while(WaitForMultipleObjects(2,events,FALSE,INFINITE)==WAIT_OBJECT_0+1){
        for(i=0;i<4;i++){
            if(WaitForSingleObject(stop_event,delays[i])==WAIT_OBJECT_0)return 0;
            if(apply()==0)break;
        }
    }
    return 0;
}
static LRESULT CALLBACK window_proc(HWND hwnd,UINT message,WPARAM w,LPARAM l){
    if(taskbar_created&&message==taskbar_created){tray_add(hwnd);return 0;}
    if(message==WM_SETTINGCHANGE){tray_add(hwnd);return 0;}
    if(message==TRAY_MESSAGE){UINT action=LOWORD(l);if(action==WM_CONTEXTMENU||action==NIN_SELECT||action==NIN_KEYSELECT)show_menu(hwnd);return 0;}
    if(message==WM_APP+2){auto_enabled=0;WritePrivateProfileStringW(L"Settings",L"AutoStart",L"0",ini);return 0;}
    if(message==WM_DISPLAYCHANGE||(message==WM_POWERBROADCAST&&(w==PBT_APMRESUMEAUTOMATIC||w==PBT_APMRESUMESUSPEND))){SetEvent(wake_event);return TRUE;}
    if(message==WM_DEVICECHANGE&&(w==DBT_DEVICEARRIVAL||w==DBT_DEVICEREMOVECOMPLETE)){SetEvent(wake_event);return TRUE;}
    if(message==WM_CLOSE){DestroyWindow(hwnd);return 0;}
    if(message==WM_DESTROY){Shell_NotifyIconW(NIM_DELETE,&tray);if(tray_icon){DestroyIcon(tray_icon);tray_icon=NULL;}SetEvent(stop_event);PostQuitMessage(0);return 0;}
    return DefWindowProcW(hwnd,message,w,l);
}
void entry(void){
    DWORD length,i,exit_code;int resident,auto_start;HANDLE mutex,thread;HWND hwnd;WNDCLASSW cls;MSG msg;
    DEV_BROADCAST_DEVICEINTERFACE_W filter;HDEVNOTIFY notification;
    length=GetModuleFileNameW(NULL,exe,MAX_PATH);
    if(!length||length>=MAX_PATH)ExitProcess(204);
    lstrcpyW(ini,exe);for(i=length;i>0;i--)if(ini[i-1]==L'\\'){ini[i]=0;break;}
    if(i+15>=MAX_PATH)ExitProcess(204);lstrcatW(ini,L"brightness.ini");
    if(has_arg(L"--quit")){hwnd=FindWindowW(CLASS_NAME,NULL);if(hwnd)PostMessageW(hwnd,WM_CLOSE,0,0);hwnd=FindWindowW(LEGACY_CLASS_NAME,NULL);if(hwnd)PostMessageW(hwnd,WM_CLOSE,0,0);ExitProcess(0);}
    if(has_arg(L"--remove-startup")){
        BOOL saved=WritePrivateProfileStringW(L"Settings",L"AutoStart",L"0",ini);
        BOOL removed=remove_startup_entries(1);
        hwnd=FindWindowW(CLASS_NAME,NULL);if(hwnd)SendMessageW(hwnd,WM_APP+2,0,0);
        ExitProcess(saved&&removed?0:206);
    }
    check_only=has_arg(L"--check");
    resident=(int)GetPrivateProfileIntW(L"Settings",L"Resident",1,ini);
    if(check_only||has_arg(L"--once")||!resident){exit_code=(DWORD)apply();ExitProcess(exit_code);}
    mutex=CreateMutexW(NULL,FALSE,L"Local\\Z1SilentBrightness.Native");
    if(!mutex)ExitProcess(205);
    if(GetLastError()==ERROR_ALREADY_EXISTS){CloseHandle(mutex);ExitProcess(0);}
    auto_start=(int)GetPrivateProfileIntW(L"Settings",L"AutoStart",0,ini);
    auto_enabled=auto_start&&startup_registry(1);if(!auto_start)startup_registry(0);
    stop_event=CreateEventW(NULL,TRUE,FALSE,NULL);wake_event=CreateEventW(NULL,FALSE,TRUE,NULL);
    if(!stop_event||!wake_event)ExitProcess(205);
    memset(&cls,0,sizeof(cls));cls.lpfnWndProc=window_proc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=CLASS_NAME;cls.hIcon=LoadIconW(cls.hInstance,MAKEINTRESOURCEW(1));
    if(!RegisterClassW(&cls))ExitProcess(205);
    hwnd=CreateWindowExW(WS_EX_TOOLWINDOW,CLASS_NAME,L"",WS_POPUP,0,0,0,0,NULL,NULL,cls.hInstance,NULL);
    if(!hwnd)ExitProcess(205);
    taskbar_created=RegisterWindowMessageW(L"TaskbarCreated");tray_add(hwnd);
    memset(&filter,0,sizeof(filter));filter.dbcc_size=sizeof(filter);filter.dbcc_devicetype=DBT_DEVTYP_DEVICEINTERFACE;
    {const GUID monitor_guid={0xe6f07b5f,0xee97,0x4a90,{0xb0,0x76,0x33,0xf5,0x7b,0xf4,0xea,0xa7}};memcpy(&filter.dbcc_classguid,&monitor_guid,sizeof(GUID));}
    notification=RegisterDeviceNotificationW(hwnd,&filter,DEVICE_NOTIFY_WINDOW_HANDLE);
    thread=CreateThread(NULL,0,worker,NULL,0,NULL);if(!thread)ExitProcess(205);
    while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
    SetEvent(stop_event);WaitForSingleObject(thread,INFINITE);
    if(notification)UnregisterDeviceNotification(notification);
    CloseHandle(thread);CloseHandle(stop_event);CloseHandle(wake_event);CloseHandle(mutex);ExitProcess(0);
}


