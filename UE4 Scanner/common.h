#pragma once
#include <Windows.h>
#include <CommCtrl.h>
#include "memory.h"
#include <locale>
#include <codecvt>
#include <TlHelp32.h>
#include <vector>
#include <string>

ULONG_PTR GNames = NULL;
ULONG_PTR GWorld = NULL;
ULONG_PTR GObjects = NULL;
ULONG_PTR baseModule = NULL;
ULONG_PTR baseSize = NULL;
HANDLE hProcess;
uintptr_t PID = 0;