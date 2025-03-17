#include "Engine.h"

#include <iostream>
#include "Object/ObjectFactory.h"
#include "Object/World/World.h"
#include "Debug/DebugConsole.h"
#include "Object/Gizmo/Axis.h"
#include "Core/Input/PlayerInput.h"
#include "Core/Input/PlayerController.h"
#include "Object/Actor/Camera.h"
#include "Object/Actor/Sphere.h"
#include "Static/FEditorManager.h"
#include "Static/FLineBatchManager.h"
#include "Core/Rendering/FDevice.h"
#include "Object/Assets/AssetManager.h"


class AArrow;
class APicker;
// ImGui WndProc 정의
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT UEngine::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ImGui의 메시지를 처리
	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
	{
		return true;
	}

	switch (uMsg)
	{
		// 창이 제거될 때 (창 닫기, Alt+F4 등)
	case WM_DESTROY:
		PostQuitMessage(0); // 프로그램 종료
		break;
		break;
	case WM_CAPTURECHANGED://현재 마우스 입력을 독점(capture)하고 있던 창이 마우스 캡처를 잃었을 때
		break;
	case WM_SIZE:
		UEngine::Get().UpdateWindowSize(LOWORD(lParam), HIWORD(lParam));
		break;
	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	return 0;
}

void UEngine::Initialize(
	HINSTANCE hInstance, const WCHAR* InWindowTitle, const WCHAR* InWindowClassName, uint32 InScreenWidth,
	uint32 InScreenHeight,
	EScreenMode InScreenMode
)
{
	WindowInstance = hInstance;
	WindowTitle = InWindowTitle;
	WindowClassName = InWindowClassName;
	ScreenMode = InScreenMode;
	ScreenWidth = InScreenWidth;
	ScreenHeight = InScreenHeight;

    InitWindow(InScreenWidth, InScreenHeight);

	FDevice::Get().Init(WindowHandle);
    InitRenderer();

	InitWorld();

	InitializedScreenWidth = ScreenWidth;
	InitializedScreenHeight = ScreenHeight;
    ui.Initialize(WindowHandle, FDevice::Get(), ScreenWidth, ScreenHeight);

	UAssetManager::Get().RegisterAssetMetaDatas(); // 나중에 멀티쓰레드로?
	UE_LOG("Engine Initialized!");
}

void UEngine::Run()
{
	// FPS 제한
	constexpr int TargetFPS = 750;
	constexpr double TargetDeltaTime = 1000.0f / TargetFPS; // 1 FPS의 목표 시간 (ms)

	// 고성능 타이머 초기화
	LARGE_INTEGER Frequency;
	QueryPerformanceFrequency(&Frequency);

	LARGE_INTEGER StartTime;
	QueryPerformanceCounter(&StartTime);

	UAssetManager::Get().LoadAssets(); // 나중에 멀티쓰레드로?

	IsRunning = true;
	while (IsRunning)
	{
		// DeltaTime 계산 (초 단위)
		const LARGE_INTEGER EndTime = StartTime;
		QueryPerformanceCounter(&StartTime);

		EngineDeltaTime = static_cast<float>(StartTime.QuadPart - EndTime.QuadPart) / static_cast<float>(Frequency.QuadPart);
		// 메시지(이벤트) 처리
		MSG Msg;
		while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
		{
			// 키 입력 메시지를 번역
			TranslateMessage(&Msg);

			// 메시지를 등록한 Proc에 전달
			DispatchMessage(&Msg);

			if (Msg.message == WM_QUIT)
			{
				IsRunning = false;
				break;
			}

		}

		if (!ImGui::GetIO().WantCaptureMouse)
		{		
			FVector winSize = Renderer->GetFrameBufferWindowSize();
			APlayerInput::Get().Update(WindowHandle, winSize.X, winSize.Y);
			APlayerController::Get().ProcessPlayerInput(EngineDeltaTime);
		}


		// Renderer Update -> World Render에서



		// World Update
		if (World)
		{
			World->Tick(EngineDeltaTime);
			World->Render();
		    World->LateTick(EngineDeltaTime);
		}

        //각 Actor에서 TickActor() -> PlayerTick() -> TickPlayerInput() 호출하는데 지금은 Message에서 처리하고 있다

        // TickPlayerInput

        
		// ui Update
        ui.Update();

        FDevice::Get().SwapBuffer();

        // FPS 제한
        double ElapsedTime;
        do
        {
            Sleep(0);

            LARGE_INTEGER CurrentTime;
            QueryPerformanceCounter(&CurrentTime);

            ElapsedTime = static_cast<double>(CurrentTime.QuadPart - StartTime.QuadPart) * 1000.0 / static_cast<double>(Frequency.QuadPart);
        } while (ElapsedTime < TargetDeltaTime);
    }
}


void UEngine::Shutdown()
{
	Renderer->Release();
	FDevice::Get().Release();
    ShutdownWindow();
}


void UEngine::InitWindow(int InScreenWidth, int InScreenHeight)
{
    // 윈도우 클래스 등록
    WNDCLASSW wnd_class{};
    wnd_class.lpfnWndProc = WndProc;
    wnd_class.hInstance = WindowInstance;
    wnd_class.lpszClassName = WindowClassName;
    RegisterClassW(&wnd_class);

    // Window Handle 생성
    WindowHandle = CreateWindowExW(
        0, WindowClassName, WindowTitle,
        WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        InScreenWidth, InScreenHeight,
        nullptr, nullptr, WindowInstance, nullptr
    );

    // TODO: 전체화면 구현
    if (ScreenMode != EScreenMode::Windowed)
    {
        std::cout << "not implement Fullscreen and Borderless mode." << '\n';
    }

    // 윈도우 포커싱
    ShowWindow(WindowHandle, SW_SHOW);
    SetForegroundWindow(WindowHandle);
    SetFocus(WindowHandle);

    //AllocConsole(); // 콘솔 창 생성

    //// 표준 출력 및 입력을 콘솔과 연결
    //freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    //freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

    //std::cout << "Debug Console Opened!" << '\n';
}

void UEngine::InitRenderer()
{
	// 렌더러 초기화
	Renderer = std::make_unique<URenderer>();
	Renderer->Create(WindowHandle);
	Renderer->CreateShader();
	Renderer->CreateConstantBuffer();
}

void UEngine::InitWorld()
{
    World = FObjectFactory::ConstructObject<UWorld>();

	World->SetCamera(World->SpawnActor<ACamera>());

    FEditorManager::Get().SetCamera(World->GetCamera());

	////Test
	//FLineBatchManager::Get().AddLine(FVector{ 3.0f,3.0f,0.0f }, { -3.f,-3.f,0.0f });
	//FLineBatchManager::Get().AddLine(FVector{ 6.0f,6.0f,6.0f }, { -6.f,-6.f,-6.0f });
	//FLineBatchManager::Get().AddLine(FVector{ 6.0f,6.0f,7.0f }, { -6.f,-6.f,-7.0f });
	//FLineBatchManager::Get().AddLine(FVector{ 6.0f,6.0f,8.0f }, { -6.f,-6.f,-8.0f });

	FLineBatchManager::Get().DrawWorldGrid(100.f,1.f);

    //// Test
    //AArrow* Arrow = World->SpawnActor<AArrow>();
    //World->SpawnActor<ASphere>();

	World->BeginPlay();
}

void UEngine::ShutdownWindow()
{
    DestroyWindow(WindowHandle);
    WindowHandle = nullptr;

    UnregisterClassW(WindowClassName, WindowInstance);
    WindowInstance = nullptr;

	ui.Shutdown();
}

void UEngine::UpdateWindowSize(uint32 InScreenWidth, uint32 InScreenHeight)
{
	ScreenWidth = InScreenWidth;
	ScreenHeight = InScreenHeight;

	//디바이스 초기화전에 진입막음
	if (FDevice::Get().IsInit() == false)
	{
		return;
	}
	
	FDevice::Get().OnUpdateWindowSize(ScreenWidth, ScreenHeight);

    if(Renderer)
    {
        Renderer->OnUpdateWindowSize(ScreenWidth, ScreenHeight);
    }

	if (ui.bIsInitialized)
	{
		ui.OnUpdateWindowSize(ScreenWidth, ScreenHeight);
	}
	
	FDevice::Get().OnResizeComplete();
	if (Renderer)
	{
		Renderer->OnResizeComplete();
	}
}

UObject* UEngine::GetObjectByUUID(uint32 InUUID) const
{
    if (const auto Obj = GObjects.Find(InUUID))
    {
        return Obj->get();
    }
    return nullptr;
}
