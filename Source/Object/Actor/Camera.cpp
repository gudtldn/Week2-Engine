#include "Camera.h"

#include "Core/Rendering/URenderer.h"
#include "Object/PrimitiveComponent/UPrimitiveComponent.h"
#include "Core/Input/PlayerInput.h"
#include "functional"
#include "Core/Config/ConfigManager.h"

ACamera::ACamera()
{
    bIsGizmo = true;
    
    Near = 1.f;
    Far = 100.f;
    FieldOfView = 45.f;
    ProjectionMode = ECameraProjectionMode::Perspective;
	CameraSpeed = 1.0f;
	Sensitivity = std::stof(UConfigManager::Get().GetValue("Camera", "Sensitivity"));

    RootComponent = AddComponent<USceneComponent>();
    
    FTransform StartPos = GetActorTransform();
    StartPos.SetPosition(FVector(-5, 0, 0));
    SetActorTransform(StartPos);
}

void ACamera::BeginPlay()
{
	Super::BeginPlay();
	APlayerInput::Get().RegisterKeyPressCallback(EKeyCode::W, [this] { MoveForward(); }, GetUUID());
	APlayerInput::Get().RegisterKeyPressCallback(EKeyCode::S, [this] { MoveBackward(); }, GetUUID());
	APlayerInput::Get().RegisterKeyPressCallback(EKeyCode::A, [this] { MoveLeft(); }, GetUUID());
	APlayerInput::Get().RegisterKeyPressCallback(EKeyCode::D, [this] { MoveRight(); }, GetUUID());
	APlayerInput::Get().RegisterKeyPressCallback(EKeyCode::Q, [this] { MoveDown(); }, GetUUID());
	APlayerInput::Get().RegisterKeyPressCallback(EKeyCode::E, [this] { MoveUp(); }, GetUUID());

	///APlayerInput::Get().RegisterMousePressCallback(EKeyCode::RButton, std::bind(&ACamera::Rotate, this, std::placeholders::_1), GetUUID());

	UConfigManager::Get().SetValue("Camera", "Sensitivity", std::to_string(Sensitivity));
}

void ACamera::SetFieldOfVew(float Fov)
{
    FieldOfView = Fov;
}  

void ACamera::SetFar(float Far)
{
    this->Far = Far;
}

void ACamera::SetNear(float Near)
{
    this->Near = Near;
}

float ACamera::GetFieldOfView() const
{
    return  FieldOfView;
}

float ACamera::GetNear() const
{
    return Near;
}

float ACamera::GetFar() const
{
    return Far;
}

void ACamera::InitMatrix()
{
	ViewMatrix = FMatrix::Identity();
	ProjectionMatrix = FMatrix::Identity();
	ViewProjectionMatrix = FMatrix::Identity();
}

void ACamera::UpdateCameraMatrix()
{
	//뷰 매트릭스 업데이트
	ViewMatrix = GetActorTransform().GetViewMatrix();
	
	// 프로젝션 매트릭스 업데이트
	float AspectRatio = UEngine::Get().GetScreenRatio();

	float FOV = FMath::DegreesToRadians(GetFieldOfView());
	float Near =GetNear();
	float Far = GetFar();

	if (ProjectionMode == ECameraProjectionMode::Perspective)
	{
		ProjectionMatrix = FMatrix::PerspectiveFovLH(FOV, AspectRatio, Near, Far);
	}
	else if (ProjectionMode == ECameraProjectionMode::Perspective)
	{
		ProjectionMatrix = FMatrix::PerspectiveFovLH(FOV, AspectRatio, Near, Far);

		// TODO: 추가 필요.
		// ProjectionMatrix = FMatrix::OrthoForLH(FOV, AspectRatio, Near, Far);
	}

	ViewProjectionMatrix = ViewMatrix * ProjectionMatrix ;
}

FMatrix ACamera::GetProjectionMatrix(float FrameBufferWidth, float FrameBufferHeight) const
{
	return FMatrix::PerspectiveFovLH(FieldOfView, FrameBufferWidth/FrameBufferHeight, Near, Far);
}

void ACamera::MoveForward()
{
	FTransform tr = GetActorTransform();
	tr.SetPosition(tr.GetPosition() + (GetForward() * CameraSpeed * UEngine::GetDeltaTime()));
	SetActorTransform(tr);
}

void ACamera::MoveBackward()
{
	FTransform tr = GetActorTransform();
	tr.SetPosition(tr.GetPosition() - (GetForward() * CameraSpeed * UEngine::GetDeltaTime()));
	SetActorTransform(tr);
}

void ACamera::MoveLeft()
{
	FTransform tr = GetActorTransform();
	tr.SetPosition(tr.GetPosition() - (GetRight() * CameraSpeed * UEngine::GetDeltaTime()));
	SetActorTransform(tr);
}

void ACamera::MoveRight()
{
	FTransform tr = GetActorTransform();
	tr.SetPosition(tr.GetPosition() + (GetRight() * CameraSpeed * UEngine::GetDeltaTime()));
	SetActorTransform(tr);
}

void ACamera::MoveUp()
{
	FTransform tr = GetActorTransform();
	tr.SetPosition(tr.GetPosition() + (FVector::UpVector * CameraSpeed * UEngine::GetDeltaTime()));
	SetActorTransform(tr);
}

void ACamera::MoveDown()
{
	FTransform tr = GetActorTransform();
	tr.SetPosition(tr.GetPosition() - (FVector::UpVector * CameraSpeed * UEngine::GetDeltaTime()));
	SetActorTransform(tr);
}

void ACamera::Rotate(const FVector& mouseDelta)
{
	FTransform tr = GetActorTransform();
	FVector TargetRotation = tr.GetRotation().GetEuler();
	TargetRotation.Y += CameraSpeed * mouseDelta.Y;
	TargetRotation.Z += CameraSpeed * mouseDelta.X;
	TargetRotation.Y = FMath::Clamp(TargetRotation.Y, -MaxYDegree, MaxYDegree);
	tr.SetRotation(TargetRotation);
	tr.Rotate(FVector(-mouseDelta.X, - mouseDelta.Y, 0));

	SetActorTransform(tr);
}
