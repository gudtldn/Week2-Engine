#include "UI.h"

#include <algorithm>

#include "Object/Actor/Camera.h"
#include "FDevice.h"
#include "Core/HAL/PlatformMemory.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "Debug/DebugConsole.h"
#include "Debug/EngineShowFlags.h"
#include "Core/Rendering/FViewMode.h"
#include "ImGui/imgui_internal.h"
#include "Object/Actor/Actor.h"
#include "Object/PrimitiveComponent/UPrimitiveComponent.h"
#include "Object/Actor/Sphere.h"
#include "Object/Actor/Cube.h"
#include "Object/Actor/Arrow.h"
#include "Object/Actor/Cone.h"
#include "Object/Actor/Cylinder.h"
#include "Static/FEditorManager.h"
#include "Object/World/World.h"
#include "Object/Gizmo/GizmoHandle.h"
#include <Core/Input/PlayerInput.h>



void UI::Initialize(HWND hWnd, const class FDevice& Device, UINT ScreenWidth, UINT ScreenHeight)
{
    // ImGui 초기화
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // 기본 폰트 크기 설정
    io.FontGlobalScale = 1.0f;
    io.DisplaySize = ScreenSize;
    //io.WantSetMousePos = true;
    // ImGui Backend 초기화
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(Device.GetDevice(), Device.GetDeviceContext());

	ScreenSize = ImVec2(static_cast<float>(ScreenWidth), static_cast<float>(ScreenHeight));
    InitialScreenSize = ScreenSize;
    bIsInitialized = true;
    
    io.DisplaySize = ScreenSize;

    PreRatio = GetRatio();
    CurRatio = GetRatio();
}

void UI::Update()
{
    POINT mousePos;
    if (GetCursorPos(&mousePos)) {
        HWND hwnd = GetActiveWindow();
        ScreenToClient(hwnd, &mousePos);

        ImVec2 CalculatedMousePos = ResizeToScreenByCurrentRatio(ImVec2(mousePos.x, mousePos.y));
        ImGui::GetIO().MousePos = CalculatedMousePos;
        //UE_LOG("MousePos: (%.1f, %.1f), DisplaySize: (%.1f, %.1f)\n",CalculatedMousePos.x, CalculatedMousePos.y, GetRatio().x, GetRatio().y);
    }

    
    // ImGui Frame 생성
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (bWasWindowSizeUpdated)
    {
        PreRatio = CurRatio;
        CurRatio = GetRatio();
        UE_LOG("Current Ratio: %f, %f", CurRatio.x, CurRatio.y);
    }

	RenderSceneManager();
    RenderControlPanel();
    RenderPropertyWindow();
	RenderShowFlagsPanel();
	RenderViewModePanel();

    Debug::ShowConsole(bWasWindowSizeUpdated, PreRatio, CurRatio);

    // ImGui 렌더링
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    bWasWindowSizeUpdated = false;
}


void UI::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void UI::OnUpdateWindowSize(UINT InScreenWidth, UINT InScreenHeight)
{
    // ImGUI 리소스 다시 생성
    ImGui_ImplDX11_InvalidateDeviceObjects();
    ImGui_ImplDX11_CreateDeviceObjects();
   // ImGui 창 크기 업데이트
	//ScreenSize = ImVec2(static_cast<float>(InScreenWidth), static_cast<float>(InScreenHeight));

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(InScreenWidth), static_cast<float>(InScreenHeight));

    bWasWindowSizeUpdated = true;
}

void UI::RenderControlPanel()
{
    ImGui::Begin("Jungle Control Panel");

    if (bWasWindowSizeUpdated)
    {
        auto* Window = ImGui::GetCurrentWindow();

        ImGui::SetWindowPos(ResizeToScreen(Window->Pos));
        ImGui::SetWindowSize(ResizeToScreen(Window->Size));
    }
    
    ImGui::Text("Hello, Jungle World!");
    ImGui::Text("FPS: %.3f (%.2f ms)", ImGui::GetIO().Framerate , 1000.0f / ImGui::GetIO().Framerate);

    RenderMemoryUsage();
    RenderPrimitiveSelection();
    RenderCameraSettings();
    
    ImGui::End();
}

void UI::RenderMemoryUsage()
{
    const uint64 ContainerAllocByte = FPlatformMemory::GetAllocationBytes<EAT_Container>();
    const uint64 ContainerAllocCount = FPlatformMemory::GetAllocationCount<EAT_Container>();
    const uint64 ObjectAllocByte = FPlatformMemory::GetAllocationBytes<EAT_Object>();
    const uint64 ObjectAllocCount = FPlatformMemory::GetAllocationCount<EAT_Object>();
    ImGui::Text(
        "Container Memory Uses: %llubyte, Count: %llu",
        ContainerAllocByte,
        ContainerAllocCount
    );
    ImGui::Text(
        "Object Memory Uses: %llubyte, Count: %llu Objects",
        ObjectAllocByte,
        ObjectAllocCount
    );
    ImGui::Text(
        "Total Memory Uses: %llubyte, Count: %llu",
        ContainerAllocByte + ObjectAllocByte,
        ContainerAllocCount + ObjectAllocCount
    );

    ImGui::Separator();
}

void UI::RenderPrimitiveSelection()
{
    const char* items[] = { "Sphere", "Cube", "Cylinder", "Cone" };

    ImGui::Combo("Primitive", &currentItem, items, IM_ARRAYSIZE(items));

    if (ImGui::Button("Spawn"))
    {
        UWorld* World = UEngine::Get().GetWorld();
        for (int i = 0 ;  i < NumOfSpawn; i++)
        {
            if (strcmp(items[currentItem], "Sphere") == 0)
            {
                World->SpawnActor<ASphere>();
            }
            else if (strcmp(items[currentItem], "Cube") == 0)
            {
                World->SpawnActor<ACube>();
            }
            else if (strcmp(items[currentItem], "Cylinder") == 0)
            {
                World->SpawnActor<ACylinder>();
            }
            else if (strcmp(items[currentItem], "Cone") == 0)
            {
                World->SpawnActor<ACone>();
            }
            //else if (strcmp(items[currentItem], "Triangle") == 0)
            //{
            //    Actor->AddComponent<UTriangleComp>();   
            //}
        }
    }
    ImGui::SameLine();
    ImGui::InputInt("Number of spawn", &NumOfSpawn, 0);

    ImGui::Separator();

    UWorld* World = UEngine::Get().GetWorld();
    uint32 bufferSize = 100;
    char* SceneNameInput = new char[bufferSize];
    strcpy_s(SceneNameInput, bufferSize, *World->SceneName);
    
	if (ImGui::InputText("Scene Name", SceneNameInput, bufferSize))
	{
		World->SceneName = SceneNameInput;
	}
    
    if (ImGui::Button("New Scene"))
    {
        World->ClearWorld();
    }
    if (ImGui::Button("Save Scene"))
    {
        World->SaveWorld();   
    }
    if (ImGui::Button("Load Scene"))
    {
        World->LoadWorld(SceneNameInput);
    }

	delete[] SceneNameInput;
    ImGui::Separator();
}

void UI::RenderCameraSettings()
{
    ImGui::Text("Camera");

    ACamera* Camera = FEditorManager::Get().GetCamera();

    bool IsOrthogonal;
    if (Camera->ProjectionMode == ECameraProjectionMode::Orthographic)
    {
        IsOrthogonal = true;
    }
    else if (Camera->ProjectionMode == ECameraProjectionMode::Perspective)
    {
        IsOrthogonal = false;
    }

    if (ImGui::Checkbox("Orthogonal", &IsOrthogonal))
    {
        if (IsOrthogonal)
        {
            Camera->ProjectionMode = ECameraProjectionMode::Orthographic;
        }
        else
        {
            Camera->ProjectionMode = ECameraProjectionMode::Perspective;
        }
    }

    float FOV = Camera->GetFieldOfView();
    if (ImGui::DragFloat("FOV", &FOV, 0.1f))
    {
        FOV = std::clamp(FOV, 0.01f, 179.99f);
        Camera->SetFieldOfVew(FOV);
    }

    float NearFar[2] = { Camera->GetNear(), Camera->GetFar() };
    if (ImGui::DragFloat2("Near, Far", NearFar, 0.1f))
    {
        NearFar[0] = FMath::Max(0.01f, NearFar[0]);
        NearFar[1] = FMath::Max(0.01f, NearFar[1]);

        if (NearFar[0] < NearFar[1])
        {
            Camera->SetNear(NearFar[0]);
            Camera->SetFar(NearFar[1]);
        }
        else
        {
            if (abs(NearFar[0] - Camera->GetNear()) < 0.00001f)
            {
                Camera->SetFar(NearFar[0] + 0.01f);
            }
            else if (abs(NearFar[1] - Camera->GetFar()) < 0.00001f)
            {
                Camera->SetNear(NearFar[1] - 0.01f);
            }
        }
    }
    
    FVector CameraPosition = Camera->GetActorTransform().GetPosition();
    if (ImGui::DragFloat3("Camera Location", reinterpret_cast<float*>(&CameraPosition), 0.1f))
    {
        FTransform Trans = Camera->GetActorTransform();
        Trans.SetPosition(CameraPosition);
        Camera->SetActorTransform(Trans);
    }

    FVector PrevEulerAngle = Camera->GetActorTransform().GetRotation().GetEuler();
    FVector UIEulerAngle = { PrevEulerAngle.X, PrevEulerAngle.Y, PrevEulerAngle.Z };
    if (ImGui::DragFloat3("Camera Rotation", reinterpret_cast<float*>(&UIEulerAngle), 0.1f))
    {
        FTransform Transform = Camera->GetActorTransform();

        //FVector DeltaEulerAngle = UIEulerAngle - PrevEulerAngle;
        //Transform.Rotate(DeltaEulerAngle);
        
        UIEulerAngle.Y = FMath::Clamp(UIEulerAngle.Y, -Camera->MaxYDegree, Camera->MaxYDegree);
        Transform.SetRotation(UIEulerAngle);
        Camera->SetActorTransform(Transform);
    }
    ImGui::DragFloat("Camera Speed", &Camera->CameraSpeed, 0.1f);
	ImGui::DragFloat("Camera Sensitivity", &Camera->Sensitivity, 0.1f);

    FVector Forward = Camera->GetActorTransform().GetForward();
    FVector Up = Camera->GetActorTransform().GetUp();
    FVector Right = Camera->GetActorTransform().GetRight();

    ImGui::Text("Camera GetForward(): (%.2f %.2f %.2f)", Forward.X, Forward.Y, Forward.Z);
    ImGui::Text("Camera GetUp(): (%.2f %.2f %.2f)", Up.X, Up.Y, Up.Z);
    ImGui::Text("Camera GetRight(): (%.2f %.2f %.2f)", Right.X, Right.Y, Right.Z);
	ImGui::Text("MouseLeftDown: %s", APlayerInput::Get().GetKeyDown(EKeyCode::LButton) ? "True" : "False");
	ImGui::Text("MousePress : %s", APlayerInput::Get().GetKeyPress(EKeyCode::LButton) ? "True" : "False");
	ImGui::Text("MosueLeftUp: %s", APlayerInput::Get().GetKeyUp(EKeyCode::LButton) ? "True" : "False");
	ImGui::Separator();
}

void UI::RenderPropertyWindow()
{

    ImGui::Begin("Properties");

    if (bWasWindowSizeUpdated)
    {
        auto* Window = ImGui::GetCurrentWindow();

        ImGui::SetWindowPos(ResizeToScreen(Window->Pos));
        ImGui::SetWindowSize(ResizeToScreen(Window->Size));
    }
    
    AActor* selectedActor = FEditorManager::Get().GetSelectedActor();
    if (selectedActor != nullptr)
    {
        FTransform selectedTransform = selectedActor->GetActorTransform();
        float position[] = { selectedTransform.GetPosition().X, selectedTransform.GetPosition().Y, selectedTransform.GetPosition().Z };
        float scale[] = { selectedTransform.GetScale().X, selectedTransform.GetScale().Y, selectedTransform.GetScale().Z };

        if (ImGui::DragFloat3("Translation", position, 0.1f))
        {
            selectedTransform.SetPosition(position[0], position[1], position[2]);
            selectedActor->SetActorTransform(selectedTransform);
        }

        FVector PrevEulerAngle = selectedTransform.GetRotation().GetEuler();
        FVector UIEulerAngle = PrevEulerAngle;
        if (ImGui::DragFloat3("Rotation", reinterpret_cast<float*>(&UIEulerAngle), 0.1f))
        {
            FVector DeltaEulerAngle = UIEulerAngle - PrevEulerAngle;

            selectedTransform.Rotate(DeltaEulerAngle);
			UE_LOG("Rotation: %.2f, %.2f, %.2f", DeltaEulerAngle.X, DeltaEulerAngle.Y, DeltaEulerAngle.Z);
            selectedActor->SetActorTransform(selectedTransform);
        }
        if (ImGui::DragFloat3("Scale", scale, 0.1f))
        {
            selectedTransform.SetScale(scale[0], scale[1], scale[2]);
            selectedActor->SetActorTransform(selectedTransform);
        }
		/*if (FEditorManager::Get().GetGizmoHandle() != nullptr)
		{
			AGizmoHandle* Gizmo = FEditorManager::Get().GetGizmoHandle();
            if(Gizmo->GetGizmoType() == EGizmoType::Translate)
			{
				ImGui::Text("GizmoType: Translate");
			}
			else if (Gizmo->GetGizmoType() == EGizmoType::Rotate)
			{
				ImGui::Text("GizmoType: Rotate");
			}
			else if (Gizmo->GetGizmoType() == EGizmoType::Scale)
			{
				ImGui::Text("GizmoType: Scale");
			}
		}*/
    }
    ImGui::End();
}

void UI::RenderSceneManager()
{
	ImGui::Begin("SceneManager");
	TArray<AActor*>& Actors = UEngine::Get().GetWorld()->GetActors();

	if (Actors.Num() == 0)
		return;

	if (PrevSize != Actors.Num())
	{
		if (CurActor != nullptr)
		{
			CurActor = nullptr;
		}

		// 사용 전에 항상 비우기
		UUIDNames.Empty();
		cUUIDNames.Empty();
		UUIDs.Empty();
		UUIDNames.Reserve(Actors.Num());
		cUUIDNames.Reserve(Actors.Num());
		UUIDs.Reserve(Actors.Num());


		int Cnt = 0;
		for (int i = 0; i < Actors.Num(); i++) {

			FString UUIDName = Actors[i]->GetTypeName();
			UUIDName += std::to_string(Actors[i]->GetUUID());
			UUIDNames.Add(UUIDName);

			UUIDs.Add(Actors[i]->GetUUID());
		}

		// 모든 문자열이 추가된 후에 포인터 설정
		for (const auto& str : UUIDNames) {
			cUUIDNames.Add(*str);
		}
	}

		PrevSize = Actors.Num();

		static int SelectUUIDIndex = 0;

		if (ImGui::ListBox("ActorList", &SelectUUIDIndex, &cUUIDNames[0], static_cast<int>(cUUIDNames.Num())))
		{
			uint32 UUID = UUIDs[SelectUUIDIndex];

			for (int i = 0; i < Actors.Num(); i++)
			{
				AActor* Actor = Actors[i];
				if (Actor->GetUUID() == UUID)
				{
					//if (CurActor != nullptr)
						//CurActor->IsHighlightValue = false;
					CurActor = Actor;
					FEditorManager::Get().SelectActor(CurActor);
				}
			}
		}

	

	

	// if (CurActor != nullptr)
	// {
	// 	// 선택된 오브젝트의 정보를 출력
	// 	FVector Location = CurActor->RelativeLocation();
	// 	FVector Rotation = CurActor->RelativeRotation();
	// 	FVector Scale = CurActor->RelativeScale();
	// 	float LocationArray[3] = { Location.X, Location.Y, Location.Z };
	// 	float RotationArray[3] = { Rotation.X, Rotation.Y, Rotation.Z };
	// 	float ScaleArray[3] = { Scale.X, Scale.Y, Scale.Z };
	// 	if (ImGui::DragFloat3("Location", LocationArray))
	// 	{
	// 		CurObject->SetRelativeLocation(FVector(LocationArray[0], LocationArray[1], LocationArray[2]));
	// 	}
	// 	if (ImGui::DragFloat3("Rotation", RotationArray, 0.05f))
	// 	{
	// 		CurObject->SetRelativeRotation(FVector(RotationArray[0], RotationArray[1], RotationArray[2]));
	// 	}
	// 	if (ImGui::DragFloat3("Scale", ScaleArray,0.02))
	// 	{
	// 		CurObject->SetRelativeScale(FVector(ScaleArray[0], ScaleArray[1], ScaleArray[2]));
	// 	}
	// 	CurObject->IsHighlightValue = true;
	// }
	
	ImGui::End();
}

void UI::RenderShowFlagsPanel()
{
	if (ImGui::Begin("Show Flags"))
	{
		bool bPrimitives = FEngineShowFlags::Get().GetSingleFlag(EEngineShowFlags::SF_Primitives);
		if (ImGui::Checkbox("Primitives", &bPrimitives))
		{
			FEngineShowFlags::Get().SetSingleFlag(EEngineShowFlags::SF_Primitives, bPrimitives);
		}

		bool bBillboardText = FEngineShowFlags::Get().GetSingleFlag(EEngineShowFlags::SF_BillboardText);
		if (ImGui::Checkbox("Billboard Text", &bBillboardText))
		{
			FEngineShowFlags::Get().SetSingleFlag(EEngineShowFlags::SF_BillboardText, bBillboardText);
		}
	}
	ImGui::End();
}

void UI::RenderViewModePanel()
{
	if (ImGui::Begin("View Mode"))
	{
		static const char* viewModeNames[] = { "Solid", "Wireframe" };
		int currentViewMode = static_cast<int>(FViewMode::Get().GetViewMode());

		if (ImGui::Combo("View Mode", &currentViewMode, viewModeNames, IM_ARRAYSIZE(viewModeNames)))
		{
			FViewMode::Get().SetViewMode((static_cast<EViewModeIndex>(currentViewMode)));
		}
	}
	ImGui::End();
}