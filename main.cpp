#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "dbghelp.lib")

#include "Core/HAL/PlatformType.h"
#include <dbghelp.h>

#include "Core/Engine.h"
#include "Core/Config/ConfigManager.h"


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// 그 다음 질문에 있는 코드 블록 추가
#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif // _DEBUG

namespace
{
// 예외 처리기 및 메모리 덤프 생성
LONG WINAPI ExceptionCallBack(EXCEPTION_POINTERS* exceptionInfo)
{
	MINIDUMP_EXCEPTION_INFORMATION info {
		.ThreadId = GetCurrentThreadId(),
		.ExceptionPointers = exceptionInfo,
		.ClientPointers = false
	};

	const std::wstring stamp(L"crash.dmp");
	const HANDLE hFile = CreateFileW(
		stamp.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_WRITE,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

#ifdef _DEBUG
	constexpr DWORD flags = MiniDumpWithFullMemory
						| MiniDumpWithFullMemoryInfo
						| MiniDumpWithHandleData
						| MiniDumpWithUnloadedModules
						| MiniDumpWithThreadInfo;
#else
	constexpr DWORD flags = MiniDumpNormal;
#endif

	// 위에서 받은 내용들을 토대로 덤프 파일을 만든다.
	MiniDumpWriteDump(
		GetCurrentProcess(),
		GetCurrentProcessId(),
		hFile,
		static_cast<MINIDUMP_TYPE>(flags),
		&info,
		nullptr,
		nullptr
	);
	CloseHandle(hFile);

	MessageBoxW(nullptr, L"An error occurred. The program will be terminated.", L"Error", MB_OK | MB_ICONERROR);
	return 0L;
}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(274); // 예: _CrtSetBreakAlloc(74);
	// 사용 안하는 파라미터들
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nShowCmd);

	// 예외 처리기 등록
	SetUnhandledExceptionFilter(ExceptionCallBack);


	UConfigManager::Get().LoadConfig("editor.ini");

	UEngine& Engine = UEngine::Get();
	Engine.Initialize(hInstance, L"Jungle Engine", L"JungleWindow", 1600, 900);

	Engine.Run();

	Engine.Shutdown();

	UConfigManager::Get().SaveConfig("editor.ini");

	_CrtDumpMemoryLeaks();
	
    return 0;
}
