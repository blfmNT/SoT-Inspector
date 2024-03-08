#include "common.h"
#include "utils.h"
#include "ue4.h"

#pragma comment(lib,"Comctl32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define WINRPM
#ifndef SINGLETHREADED
#pragma omp
#endif

using namespace UTILS;
using namespace MEMORY;
using namespace UE4;

void InitSoT() {
    HWND hWndGame;
    DWORD dwProcessId;
    while (NULL == (hWndGame = GetSoTWindowProcessId(&dwProcessId)))
    {
        MessageBoxA(0, "SoTGame not found", "Error", MB_ICONERROR | MB_OK);
        ExitProcess(0);
        Sleep(100);
    }
    MessageBoxA(0, "Start SoT", "Warning", MB_ICONWARNING | MB_OK);
}

enum WND_MENU {
    FILTER_STATIC,
    FILTER_LABEL,
    FILTER_BUTTON,
    PTR_STATIC,
    PTR_LABEL,
    PTR_BUTTON,
    SCAN_LISTBOX,
    SCAN_LISTVIEW,
    STATUS_STATIC,
};
bool bFinish = false;
HINSTANCE hInstance;

#define SCREEN_WIDTH  1000
#define SCREEN_HEIGHT 600
HWND hWnd;
HWND hEdit1;
HWND hEdit2;
HWND hStatic;
HWND hListBox;
HWND hListView;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#define SET_STATUS(x) SetWindowText(hStatic,  x)
static HWND showWindow()
{
    INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    const char *wndClass = "wndclass";
    WNDCLASSEX wc;
    ZeroMemory(&wc, sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    //wc.style = CS_DBLCLKS | CS_GLOBALCLASS;// CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = WindowProc;
    wc.lpszClassName = wndClass;
    wc.hInstance = hInstance;//GetModuleHandle(nullptr);
    //wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);//(HBRUSH)GetStockObject(BLACK_BRUSH);//reinterpret_cast<HBRUSH>(COLOR_WINDOW);
    //wc.lpszClassName = wndClass;
    RegisterClassEx(&wc);
    RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    unsigned int dwStyle = ( WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);
    hWnd = CreateWindowEx(NULL, wndClass, "SOT Inspector", dwStyle, 300, 300, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    hStatic = CreateWindowEx(0, WC_STATICA, NULL,
        WS_CHILD | WS_VISIBLE,
        20, 500, 680, 32,
        hWnd, (HMENU)STATUS_STATIC, hInstance, NULL);
    SetWindowText(hStatic, "Status: ");
    SendMessage(hStatic, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

    HWND hStatic3 = CreateWindowEx(0, WC_STATICA, NULL,
        WS_CHILD | WS_VISIBLE,
        20, 380, 280, 32,
        hWnd, (HMENU)FILTER_STATIC, hInstance, NULL);
    SetWindowText(hStatic3, "Filter:");
    SendMessage(hStatic3, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    hEdit1 = CreateWindowEx(0, WC_EDITA, NULL,
        WS_CHILD | WS_VISIBLE,
        60, 380, 100, 24,
        hWnd, (HMENU)FILTER_LABEL, hInstance, NULL);
    SendMessage(hEdit1, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);


    HWND hSearch = CreateWindowEx(0, WC_BUTTONA, NULL,
        WS_CHILD | WS_VISIBLE,
        180, 380, 62, 22,
        hWnd, (HMENU)FILTER_BUTTON, hInstance, NULL);

    SetWindowText(hSearch, "SCAN");
    SendMessage(hSearch, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

    //x

    HWND hStatic2 = CreateWindowEx(0, WC_STATICA, NULL,
        WS_CHILD | WS_VISIBLE,
        20, 420, 280, 32,
        hWnd, (HMENU)PTR_STATIC, hInstance, NULL);
    SetWindowText(hStatic2, "Pointer:");
    SendMessage(hStatic2, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    hEdit2 = CreateWindowEx(0, WC_EDITA, NULL,
        WS_CHILD | WS_VISIBLE,
        60, 420, 100, 24,
        hWnd, (HMENU)PTR_LABEL, hInstance, NULL);
    SendMessage(hEdit2, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);


    HWND hSearch2 = CreateWindowEx(0, WC_BUTTONA, NULL,
        WS_CHILD | WS_VISIBLE,
        180, 420, 62, 22,
        hWnd, (HMENU)PTR_BUTTON, hInstance, NULL);
    SetWindowText(hSearch2, "PTR SCAN");
    SendMessage(hSearch2, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);


    hListBox = CreateWindowEx(0, WC_LISTBOXA, NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY ,
        10, 10, 260, 360,
        hWnd, (HMENU)SCAN_LISTBOX, hInstance, NULL);
    SendMessage(hListBox, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

    hListView = CreateWindowEx(0, WC_LISTVIEWA, NULL,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LVS_REPORT | LVS_ALIGNLEFT | UDS_ALIGNRIGHT | LVS_ALIGNTOP,
        380, 10, 560, 460,
        hWnd, (HMENU)SCAN_LISTVIEW, hInstance, NULL);
    SendMessage(hListView, WM_SETFONT, WPARAM(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    SendMessage(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE,
        0, LVS_EX_FULLROWSELECT); // Set style
    LVCOLUMN LvCol;
    // Here we put the info on the Coulom headers
    // this is not data, only name of each header we like
    memset(&LvCol, 0, sizeof(LvCol));                  // Zero Members

    LvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;    // Type of mask
    LvCol.cx = 0x28;                                   // width between each coloum
    LvCol.pszText = "Offset";                            // First Header Text
    LvCol.cx = 0x32;                                   // width of column
                                                       // Inserting Couloms as much as we want
    SendMessage(hListView, LVM_INSERTCOLUMN, 0, (LPARAM)&LvCol); // Insert/Show the coloum
    LvCol.pszText = "Name";                            // Next coloum
    LvCol.cx = 0x132;                                   // width of column
    SendMessage(hListView, LVM_INSERTCOLUMN, 1, (LPARAM)&LvCol); // ...
    LvCol.pszText = "Value";                            //
    LvCol.cx = 0x132;                                   // width of column
    SendMessage(hListView, LVM_INSERTCOLUMN, 2, (LPARAM)&LvCol); //
    //draw listview and tableview
    
    ShowWindow(hWnd, SW_SHOWNORMAL);
    return hWnd;
}
#include <Shlwapi.h>
#pragma comment(lib,"Shlwapi.lib")
//reference https://puu.sh/nvF9d/04c184dfec.png


#define READ_WORLD
void DoBoxScan() {
    //clear
    /*UINT iItems = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
    for (int i = 0; i < iItems;i++) {
    SendMessage(hListBox, LB_DELETESTRING, 0, 0);
    }*/
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    char szFilter[124];
    GetWindowTextA(hEdit1, szFilter, 124);
    std::vector<std::string> vList;

    Process("SoTGame.exe");

    if (!baseModule) {
        baseModule = Module("SoTGame.exe");
        baseSize = ModuleSize("SoTGame.exe");
    }

    if (!GWorld) {
        ULONG_PTR uworldAddress = FindSignature(baseModule, baseSize,
            (BYTE*)"\x48\x8B\x05\x00\x00\x00\x00\x48\x8B\x88\x00\x00\x00\x00\x48\x85\xC9\x74\x06\x48\x8B\x49\x70",
            (char*)"xxx????xxx????xxxxxxxxx");

        auto uworldoffset = Read<int32_t>((PBYTE)uworldAddress + 3);
        GWorld = uworldAddress + uworldoffset + 7;

    }
    if (!GNames) {
        ULONG_PTR gnamesAddress = FindSignature(baseModule, baseSize,
            (BYTE*)"\x48\x89\x3D\x00\x00\x00\x00\x41\x8B\x75\x0000",
            (char*)"xxx????xxx?");

        auto gnamesoffset = Read<int32_t>((PBYTE)gnamesAddress + 3);
        GNames = Read<ULONG_PTR>((PBYTE)gnamesAddress + gnamesoffset + 7);
    }

    if (!GObjects) {
        ULONG_PTR gobjectsAddress = FindSignature(baseModule, baseSize,
            (BYTE*)"\x48\x8B\x0D\x00\x00\x00\x00\x81\x4C\xD1\x00\x00\x00\x00\x00\x48\x8D\x4D\xD8",
            (char*)"xxx????xxx?????xxxx");
        auto gobjectsoffset = Read<int32_t>((PBYTE)gobjectsAddress + 3);
        GObjects = Read<ULONG_PTR>((PBYTE)gobjectsAddress + gobjectsoffset + 7);
    }


    CWorld w = CWorld(Read<ULONG_PTR>((PBYTE)GWorld));



    char szMsg[1024];
    sprintf_s(szMsg, 1024, "%p - %s", (LPVOID)w._this, w.GetName());
    vList.push_back(szMsg);
#ifndef READ_WORLD
    //read objects instead of world
    int iCount = 0;
    bool bHasFilter = strlen(szFilter);
    for (int i = 0; i < CObjects::GetCount();i++) {
		auto ptr = CObjects::GetObject(i);
		sprintf_s(szMsg, 1024, "Test: %08X", ptr);
		OutputDebugStringA(szMsg);
        UObjectProxy a(CObjects::GetObject(i));
        if (a.ptr == 0)
            continue;
        std::string name = a.GetName();
        sprintf_s(szMsg, 1024, "%i - %p - %s",i, (LPVOID)a.ptr, name.c_str());
        OutputDebugStringA(szMsg);
        //ignore fields
        if (a.IsIgnore() || a.GetClass().As<UClassProxy>().IsIgnore())
            continue;
        if (bHasFilter && !StrStrI(name.c_str(), szFilter))
            continue;
        sprintf_s(szMsg, 1024, "%p - %s", (LPVOID)a.ptr, name.c_str());
        vList.push_back(szMsg);
        iCount++;
    }
    SET_STATUS(std::to_string(iCount).c_str());
#else
	auto actors = w.GetActors();
	//auto actors = w.GetEncActors();
    for each (auto a in actors) {
    const char* name = a.GetName();
    if (strlen(szFilter) && !StrStrI(name,szFilter))
    continue;
    char szMsg[124];
    sprintf_s(szMsg, 124, "%p - %s",(LPVOID)a._this,name);
    vList.push_back(szMsg);
    }
#endif
    for each (auto str in vList) {
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)str.c_str());
    }

}
void AddItem(int offset,std::string name,std::string value,ULONG_PTR lParam = 0) {
    LVITEM LvItem;
    memset(&LvItem, 0, sizeof(LvItem)); // Zero struct's Members

                                        //  Setting properties Of members:

    char msg[1024*4];
    sprintf_s(msg, 124, "%04X", offset);
	LvItem.lParam = (LPARAM)lParam;// "Test";
    LvItem.mask = LVIF_TEXT| LVIF_PARAM;   // Text Style
    LvItem.cchTextMax = 256; // Max size of test
    LvItem.iItem = ListView_GetItemCount(hListView);          // choose item  
    LvItem.iSubItem = 0;       // Put in first coluom
    LvItem.pszText = msg;//"00"; // Text to display (can be from a char variable) (Items)
    SendMessage(hListView, LVM_INSERTITEM, 0, (LPARAM)&LvItem); // Send info to the Listview

	LvItem.mask = LVIF_TEXT;
    strcpy_s(msg, 1024, name.c_str());
    LvItem.iSubItem = 1;
    LvItem.pszText = msg;//(LPSTR)p.GetName().c_str();//"Name";
    SendMessage(hListView, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems

    strcpy_s(msg, 1024*4, value.c_str());
    LvItem.iSubItem = 2;
    LvItem.pszText = msg;
    SendMessage(hListView, LVM_SETITEM, 0, (LPARAM)&LvItem); // Enter text to SubItems
}

bool SortProperty(UPropertyProxy &pPropertyA, UPropertyProxy &pPropertyB) {
    if (pPropertyA.GetOffset() == pPropertyB.GetOffset()
        && pPropertyA.IsBool() && pPropertyB.IsBool()) {
        return pPropertyA.GetBitMask() < pPropertyB.GetBitMask();
    }
    /*if
        (
            pPropertyA->Offset == pPropertyB->Offset
            &&	pPropertyA->IsA(UBoolProperty::StaticClass())
            && pPropertyB->IsA(UBoolProperty::StaticClass())
            )
    {
        return (((UBoolProperty *)pPropertyA)->BitMask < ((UBoolProperty *)pPropertyB)->BitMask);
    }
    else
    {*/
        return (pPropertyA.GetOffset() < pPropertyB.GetOffset());
    //}
}
bool IsBadReadPtr(void* p)
{
    MEMORY_BASIC_INFORMATION mbi = { 0 };
    if (::VirtualQuery(p, &mbi, sizeof(mbi)))
    {
        DWORD mask = (PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
        bool b = !(mbi.Protect & mask);
        // check the page is not a guard page
        if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) b = true;

        return b;
    }
    return true;
}

std::string ws2s(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}
std::string GetObjectValue(ULONG_PTR pObj, UPropertyProxy *pProperty, ULONG_PTR dwOffset,ULONG_PTR &lParam) {
    static char szBuf[1024];
    if (dwOffset == -1) { //get from prop
        //dwOffset = pProperty->Offset;
    }
    dwOffset += pObj;
    //if (IsBadReadPtr((LPVOID)dwOffset)) {
    //    sprintf_s(szBuf,124, "Bad_PTR [%04X] [%p]", dwOffset - (DWORD)pObj, dwOffset);
        //return szBuf;
    //}else
    if (pProperty->IsByte()) { sprintf_s(szBuf, 124, "%i", Read<BYTE>((LPBYTE)dwOffset)); return szBuf; }
    else if (pProperty->IsInt()) { sprintf_s(szBuf, 124, "%i", Read<int>((LPBYTE)dwOffset)); return szBuf; }
    else if (pProperty->IsInt8()) { sprintf_s(szBuf, 124, "%i", Read<char>((LPBYTE)dwOffset)); return szBuf; }
    else if (pProperty->IsUIn32()) { sprintf_s(szBuf, 124, "%i", Read<DWORD>((LPBYTE)dwOffset)); return szBuf; }
    else if (pProperty->IsUInt64()) { sprintf_s(szBuf, 124, "%Ii", Read<DWORD64>((LPBYTE)dwOffset)); return szBuf; }
    else if (pProperty->IsFloat()) { sprintf_s(szBuf, 124, "%f", Read<float>((LPBYTE)dwOffset)); return szBuf; }
    else if (pProperty->IsBool()) { strcpy_s(szBuf,124, Read<DWORD64>((LPBYTE)dwOffset) & pProperty->GetBitMask() ? "true" : "false"); return szBuf; }
    else if (pProperty->IsName()) {
        
        auto fData = CNames::GetName(Read<DWORD>((LPBYTE)dwOffset));
        strcpy_s(szBuf, 124, (std::string("FName ")+std::string(fData)).c_str());
        return szBuf;
    }//return QString(fData->GetName()).prepend("FName "); }
    else if (pProperty->IsObject()) {
        UObjectProxy p(Read<ULONG_PTR>((LPBYTE)dwOffset));
		lParam = p.ptr;
        if (!p.ptr) return "NULL";
        sprintf_s(szBuf, 124, "%s* [%p]",p.GetName().c_str(),(LPBYTE)p.ptr);
        return szBuf;
    }
    else if (pProperty->IsClass()) {
        UClassProxy p(Read<ULONG_PTR>((LPBYTE)dwOffset));
        //read uclass
        sprintf_s(szBuf, 124, "UClass *%s", p.GetName().c_str());
        return szBuf;
    }
    else if (pProperty->IsString()) {
        FString buf = Read<FString>((LPBYTE)dwOffset);
        if (buf.Count == 0) return "\"\"";
        std::wstring sArray;
        sArray += '"';
        for (int i = 0; i < buf.Count-1;i++) {
            wchar_t wchar = Read<wchar_t>((LPBYTE)buf.Data + (i * 2));
            sArray += wchar;
        }
        sArray += '"';
        return ws2s(sArray);
    }
    else if (pProperty->IsMulticastDelegate()) {
        return "ScriptDeletage";
    }
    else if (pProperty->IsArray()) {

        TArray<ULONG_PTR> buf = Read<TArray<ULONG_PTR>>((LPBYTE)dwOffset);
        std::string sPropertyTypeInner = pProperty->GetInner().GetName();
        std::string sArray;
        for (int i = 0; i < buf.Count;i++) {
			ULONG_PTR ptr = Read<ULONG_PTR>((LPBYTE)buf.Data + (i * 8));
			if (i == 0) {
				lParam = ptr;
			}
            char szPtr[32];
            sprintf_s(szPtr, 32, "%p", (LPBYTE)ptr);
            sArray += szPtr + std::string(",");
            //UObjectProxy p(ptr);
            if (i > 30) {
                sArray.append(",...");
                break;
            }
        }
        sprintf_s(szBuf, 1024, "TArray< %s >(%i)", sPropertyTypeInner.c_str(), buf.Count);
        std::string sRet = szBuf;
        sRet.append("{").append(sArray).append("}");
        return sRet;
    }
    else if (pProperty->IsMap()) {
        sprintf_s(szBuf, 124, "TMap< %s , %s >", pProperty->GetKey().GetName().c_str(), pProperty->GetValue().GetName().c_str());
        return szBuf;
    }
    /*
    else if (pProperty->IsA(UStrProperty::StaticClass())) { auto wData = ((FString*)(dwOffset))->Data; return wData ? QString::fromWCharArray(wData).prepend("\"").append("\"") : QString("\"\""); }

    else if (pProperty->IsA(UNameProperty::StaticClass())) { auto fData = ((FName*)(dwOffset)); return QString(fData->GetName()).prepend("FName "); }

    else if (pProperty->IsA(UDelegateProperty::StaticClass())) { return "FScriptDelegate"; }

    else if (pProperty->IsA(UObjectProperty::StaticClass())) {
        UObject* nObj = *(UObject**)dwOffset;
        if (nObj == NULL) {
            return QString("NULL");
        }
        if (IsBadReadPtr((LPVOID)nObj)) {
            return QString("Bad UObject_Ptr");
        }
        //return "UObject* XXXXXXXX";
        //GetValidName(std::string(((UObjectProperty *)pProperty)->PropertyClass->GetNameCPP()))
        DWORD dwAddr = (DWORD)nObj;
        auto names = FName::Names();
        auto d = names->Data(nObj->Name.Index);
        std::string name = d ? d->Name : "UObject";
        QString str = QString().sprintf("%s* [%08X]", name.c_str(), dwAddr);
        //QString str = QString().sprintf("%i [%08X]", nObj->Name.Index, dwAddr);
        //QMessageBox::information(dnpa, "title", str);
        return str;
    }

    else if (pProperty->IsA(UClassProperty::StaticClass())) { return "UClass"; }

    else if (pProperty->IsA(UInterfaceProperty::StaticClass())) { return "UInterface"; }

    else if (pProperty->IsA(UStructProperty::StaticClass())) { return "UStruct"; }       // NOT SAFE !!!

    else if (pProperty->IsA(UArrayProperty::StaticClass())) {
        TArray<UObject*>* tArray = ((TArray<UObject*>*)(dwOffset));
        DWORD dwCount = tArray->Num();

        std::string sPropertyTypeInner;
        if (GetPropertyType(((UArrayProperty *)pProperty)->Inner, sPropertyTypeInner)) {
            QString sList = "";
            for (UINT i = 0; i < dwCount; i++) {
                DWORD dwAddr = (DWORD)tArray->Data[i];
                QString name = QString().sprintf("%08X", dwAddr);
                sList = sList.append(name);
                if (i + 1 != dwCount) {
                    sList = sList.append(",");
                }
            }

            //}
            //list objs
            return QString().sprintf("TArray< %s >(%i) ", sPropertyTypeInner.c_str(), dwCount).append("{").append(sList).append("}");
        }
    }

    else if (pProperty->IsA(UMapProperty::StaticClass())) {
        std::string sPropertyTypeKey;
        std::string sPropertyTypeValue;
        if
            (
                GetPropertyType(((UMapProperty *)pProperty)->Key, sPropertyTypeKey)
                && GetPropertyType(((UMapProperty *)pProperty)->Value, sPropertyTypeValue)
                ) {
            return QString().sprintf("TMap< %s , %s >", sPropertyTypeKey.c_str(), sPropertyTypeValue.c_str());
        }
    }*/

    return std::string("Unknown ").append(pProperty->GetFullName());
}
std::string GetHex(int val) {
    char msg[124];
    sprintf_s(msg, 124, "%x", val);
    return msg;
}

class CApp {
public:
    HMODULE hModule;
    char szDllPath[MAX_PATH];
    void BaseUponModule(HMODULE _hModule) {
        hModule = _hModule;
        if (GetModuleFileNameA(hModule, szDllPath, MAX_PATH)) {
            for (UINT i = strlen(szDllPath); i > 0; i--) {
                if (szDllPath[i] == '\\') {
                    szDllPath[i] = 0;
                    break;
                }
            }
        }
    }
    void AddToLogFileA(char* szFile, LPTSTR szFormat, ...) {
        char szPath[MAX_PATH];
        sprintf_s(szPath, MAX_PATH, "%s\\%s", szDllPath, szFile);
        //MessageBoxA(0, szPath, szPath, 0);
        HGLOBAL hgBuffer;
        va_list	vaarg;

        hgBuffer = GlobalAlloc(GPTR, 1024);

        va_start(vaarg, szFormat);
        wvsprintf((LPTSTR)hgBuffer, szFormat, vaarg);
        char* str = (LPTSTR)hgBuffer;

        FILE* f;
        fopen_s(&f, szPath, "a+");
        fwrite(str, 1, strlen(str), f);
        printf("%s\r\n", str);
        fwrite("\r\n", 1, 2, f);

        fclose(f);

        GlobalFree(hgBuffer);
    }
    std::string GetMyDllPath() {
        return szDllPath;
    }
} gApp;
#include <algorithm>
#include <functional>
void DoPtrScan() {
    char buf[124];
    GetWindowTextA(hEdit2, buf,124);
    ULONG_PTR ptr = _strtoui64(buf, NULL, 16);
    if (!ptr) {
        //Do0Scan();
        return;
    }
    UObjectProxy p = UObjectProxy(ptr);
    UClassProxy c = p.GetClass().As<UClassProxy>();
    //..
    //check class
    std::string status = std::to_string(p.GetId()).append(" ").append(p.GetName()).append(" ").append(c.GetFullClass());
    SET_STATUS(status.c_str());
    std::vector< UPropertyProxy> vProperty;
    SendMessage(hListView, LVM_DELETEALLITEMS, 0, 0);
    //find structure and dump it here..
    int structSize = 0;
    int iLoops = 0;
    while (c.HasSuperClass()) {
        structSize += c.GetSize();
        //print size
        std::string className = c.GetName();
        //AddItem(-1, p.GetName(), className);
        if (!c.HasChildren()) {
            c = c.GetSuperClass();
            continue;
        }
        //list properties
        UPropertyProxy f = c.GetChildren().As<UPropertyProxy>();
        while (1) {
            if (!f.IsFunction()) {
                vProperty.push_back(f);
                //AddItem(f.GetOffset(), f.GetName(), className);
            }
            if (!f.HasNext()) {
                break;
            }
            f = f.GetNext();
            //break;
        }
        c = c.GetSuperClass();
        //break;
    }
    sort(vProperty.begin(), vProperty.end(), SortProperty);
    //sort..

    /*std::function<void(TableModel*, UObject*, UProperty*, std::string)> listStructProperties = [=](TableModel* propertyTable, UObject* pObj, UProperty *pProperty, std::string structName) {
        DWORD baseOffset = pProperty->Offset;
        auto pScriptStruct = ((UStructProperty *)pProperty)->Struct;
        for (UProperty *pProperty = (UProperty *)pScriptStruct->Children; pProperty; pProperty = (UProperty *)pProperty->Next) {

            // get property name
            std::string sPropertyName = structName;
            sPropertyName.append(".").append(GetValidName(std::string(pProperty->GetName())));

            if (pProperty->IsA(UStructProperty::StaticClass())) {
                //listStructProperties(propertyTable, pObj, pProperty, sPropertyName);
                continue;
            }
            DWORD dwOffset = pProperty->Offset + baseOffset;
            QString sValue = GetObjectValue(pObj, pProperty, dwOffset);

            QStringList sData;
            sData << QString::number(dwOffset, 16).toUpper() << QString(sPropertyName.c_str()) << sValue;
            propertyTable->insertRow(new TableItem(sData));
        }
    };*/
    std::function<void(UPropertyProxy fStruct, ULONG_PTR ptr, ULONG_PTR offset)> fnc = [&](UPropertyProxy fStruct,ULONG_PTR ptr, ULONG_PTR offset) {
        std::string structName = fStruct.GetName();
        //iter child
        std::vector< UPropertyProxy> vProperty;

        UClassProxy c = fStruct.GetStruct().As<UClassProxy>();
        //list properties
        //TODO: CHECK SUPER
        UPropertyProxy f = c.GetChildren().As<UPropertyProxy>();

        while (1) {
            if (!f.IsFunction()) {
                vProperty.push_back(f);
            }
            if (!f.HasNext()) {
                break;
            }
            f = f.GetNext();
            //break;
        }
        sort(vProperty.begin(), vProperty.end(), SortProperty);
        //add size to offset
        for each(auto f in vProperty) {
            if (f.IsStruct()) {
                fnc(f, ptr, offset +f.GetOffset());
            }
            else {
                if (f.GetArrayDim() > 1) {
                    AddItem(f.GetOffset(), f.GetFullName(), "ARRAY DIM0");
                    continue;
                }
                //OutputDebugStringA(f.GetName().c_str());
                //auto pScriptStruct = ((UStructProperty *)pProperty)->Struct;
				ULONG_PTR lParam = 0;
                std::string value = GetObjectValue(ptr, &f, offset +f.GetOffset(), lParam);//"value";
                std::string name = structName;
                AddItem(offset +f.GetOffset(), name.append(".").append(f.GetName()), value, lParam);
            }
        }
    };
    auto parseFnc = [fnc](std::vector< UPropertyProxy> vProperty,ULONG_PTR ptr,int structSize) {
        int offset = sizeof(UObject);
        for(int i = 0; i < vProperty.size();i++){
            auto f = vProperty[i];

            //check offset
            DWORD dwOffset = f.GetOffset();
            int size = dwOffset - offset;
            if (dwOffset > offset) {
                AddItem(offset, "MISSED", GetHex(size));
                offset += size;
                //print missed
            }
            size = f.GetSize();
            if (f.IsStruct()) {
                fnc(f,ptr,f.GetOffset());
            }
            else {
                auto arrayDim = f.GetArrayDim();
                if (arrayDim > 1) {
                    DWORD nSize = i + 1 < vProperty.size() ? (vProperty[i + 1].GetOffset() - f.GetOffset()) / arrayDim : arrayDim*size;
                    for (DWORD j = 0; j < arrayDim ; j++) {
                        char name[124];

                        sprintf_s(name, 124, "%s[%i]", f.GetFullName(), j);
                        AddItem(dwOffset, name, "ARRAY DIM");
                        dwOffset += nSize;
                    }
                    continue;
                }
                //auto pScriptStruct = ((UStructProperty *)pProperty)->Struct;
				ULONG_PTR lParam = 0;
                std::string value = GetObjectValue(ptr, &f, f.GetOffset(), lParam);//"value";
                std::string name = /*std::to_string(size) + */f.GetName();
                AddItem(offset, name, value, lParam);
            }
            if (f.IsBool()) {
                //check if next val has diff offset
                if (i+1 < vProperty.size() && dwOffset != vProperty[i + 1].GetOffset()) {
                    offset += 1;
                }
            }
            else {
                offset += size;
            }
        }
        if (offset < structSize) {
            int size = structSize - offset;
            AddItem(offset, "MISSED", GetHex(size));
        }
    };
    parseFnc(vProperty,p.ptr, structSize);
}

int filterException(int code, PEXCEPTION_POINTERS ex) {
    char msg[128];
    memset(msg, 0, sizeof(msg));
    sprintf_s(msg, "Error: %i", code);
    MessageBox(hWnd, msg, "Error", MB_ICONERROR | MB_OK);
    return EXCEPTION_EXECUTE_HANDLER;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_NOTIFY:
		// When right button clicked on mouse
		if ((((LPNMHDR)lParam)->hwndFrom) == hListView)
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case NM_RCLICK:
			{
				HMENU hPopupMenu = CreatePopupMenu();
				POINT p;
				if (GetCursorPos(&p))
				{
                    AppendMenu(hPopupMenu, MF_STRING, 1, ("Follow"));
                    SetForegroundWindow(hWnd);

                    //returns index of menu item, not bool
                    DWORD iRet = TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, p.x, p.y, 0, hWnd, NULL);

                    if (iRet > 0)
                    {
                        int ItemIndex;
                        LVITEM SelectedItem;
                        SelectedItem.mask = LVIF_TEXT | LVIF_PARAM;
                        char buf[128];
                        memset(buf,0, sizeof(buf));
  
                        ItemIndex = SendMessage(hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
                        SelectedItem.iItem = ItemIndex;
                        //that shit crashes
                        ListView_GetItem(hListView, &SelectedItem);
                        sprintf_s(buf, "%p", (LVITEM*)SelectedItem.lParam);

                        SetWindowTextA(hEdit2, buf);
                        DoPtrScan();
                    }
 
				}
				
				break;
			}
			break;
			}
			break;
		}
		break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case FILTER_BUTTON:
            DoBoxScan();
            break;
        case PTR_BUTTON:
            DoPtrScan();
            break;
        case SCAN_LISTBOX:
            switch (HIWORD(wParam))
            {
            case LBN_SELCHANGE: {
                HWND hwndList = hListBox;

                // Get selected index.
                int lbItem = (int)SendMessage(hwndList, LB_GETCURSEL, 0, 0);

                char buf[124];
                // Get item data.
                int i = (int)SendMessage(hwndList, LB_GETTEXT, lbItem, (LPARAM)buf);

                buf[16] = 0;
                SetWindowTextA(hEdit2,buf);
                //MessageBoxA(0, buf, buf, 0);
                return TRUE;
            }
            }
            break;
        }
        break;
    case WM_DESTROY:
    {
        ExitProcess(0);
        PostQuitMessage(0);
        return 0;
    }
    break;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}


// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE _hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow) {
    hInstance = _hInstance;
    showWindow();
    MSG msg;
    while (!bFinish) {
        while (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT || msg.message == WM_DESTROY)
            {
                ExitProcess(0);
                bFinish = true;
                break;
            }
        }
        Sleep(10);
    }
    return 0;
}