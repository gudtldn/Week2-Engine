#include "Actor.h"

#include "Object/PrimitiveComponent/UPrimitiveComponent.h"
#include "Object/World/World.h"
#include "Static/FEditorManager.h"

AActor::AActor() : Depth{ 0 }
{
}

void AActor::BeginPlay()
{
	for (auto& Component : Components)
	{
		Component->BeginPlay();

		if (UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component))
		{
			PrimitiveComponent->RegisterComponentWithWorld(World);
		}
	}
}

void AActor::Tick(float DeltaTime)
{
	for (auto& Component : Components)
	{
		if (Component->CanEverTick())
		{
			Component->Tick(DeltaTime);
		}
	}
}

void AActor::LateTick(float DeltaTime)
{
}

void AActor::Destroyed()
{
	EndPlay(EEndPlayReason::Destroyed);
	
	for (auto& Component : Components)
	{
		Component->Destroyed();
	}
}


void AActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	for (auto& Component : Components)
	{		
		Component->EndPlay(EndPlayReason);
		if (const auto PrimitiveComp = dynamic_cast<UPrimitiveComponent*>(Component))
		{
			if (World->ContainsZIgnoreComponent(PrimitiveComp))
			{
				World->RemoveZIgnoreComponent(PrimitiveComp);
			}
			
			GetWorld()->RemoveRenderComponent(PrimitiveComp);
		}
		if (FEditorManager::Get().GetSelectedActor() == this)
		{
			FEditorManager::Get().SelectActor(nullptr);
		}
		UEngine::Get().GObjects.Remove(Component->GetUUID());
	}
	Components.Empty();
}

const FTransform& AActor::GetActorTransform() const
{
	if (RootComponent)
	{
		return RootComponent->GetComponentTransform();
	}
	else
	{
		return FTransform::Identity;
	}
}

const FTransform& AActor::ActorToWorld() const
{
	return (RootComponent != nullptr) ? RootComponent->GetWorldTransform() : FTransform::Identity;
}

FVector AActor::GetActorForwardVector() const
{
	return (RootComponent != nullptr) ? RootComponent->GetForwardVector() : FVector::ForwardVector;
}

FVector AActor::GetActorRightVector() const
{
	return (RootComponent != nullptr) ? RootComponent->GetRightVector() : FVector::ForwardVector;
}

FVector AActor::GetActorUpVector() const
{
	return (RootComponent != nullptr) ? RootComponent->GetUpVector() : FVector::ForwardVector;
}

FVector AActor::GetActorPosition() const
{
	if (RootComponent)
	{
		return RootComponent->GetComponentTransform().GetPosition();
	}
	return FVector::ZeroVector;
}

FVector AActor::GetActorRotation() const
{
	if (RootComponent)
	{
		return RootComponent->GetComponentTransform().GetRotation().GetEuler();
	}
	return FVector::ZeroVector;
}

FQuat AActor::GetActorRotationQuat() const
{
	if (RootComponent)
	{
		return RootComponent->GetComponentTransform().GetRotation();
	}
	return FQuat::Identity;
}

void AActor::Pick()
{
	if (RootComponent)
	{
		bIsPicked = true;
		RootComponent->Pick(true);
	}
}

void AActor::UnPick()
{
	if (RootComponent)
	{
		bIsPicked = false;
		RootComponent->Pick(false);
	}	
}

void AActor::GetActorBounds(bool bOnlyCollidingComponents, FVector& Origin, FVector& BoxExtent, bool bIncludeFromChildActors) const
{
	const FBox Bounds = GetComponentsBoundingBox(bOnlyCollidingComponents, bIncludeFromChildActors);

	Origin = Bounds.GetCenter();
	BoxExtent = Bounds.GetExtent();
}

FBox AActor::GetComponentsBoundingBox(bool bNonColliding, bool bIncludeFromChildActors) const
{
	FBox Box = FBox();

	for (auto& Component : Components)
	{
		if (UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component))
		{
			Box += PrimitiveComponent->Bounds.GetBox();
		}
	}

	return Box;
}

FBox AActor::CalculateComponentsBoundingBoxInLocalSpace(bool bNonColliding, bool bIncludeFromChildActors) const
{
	FBox Box = FBox();
	// const FTransform& ActorToWorld = GetActorTransform();
	// const FTransform WorldToActor = ActorToWorld.Inverse();

	for (auto& Component : Components)
	{
		if (UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component))
		{
			const FTransform& ComponentToWorld = PrimitiveComponent->GetComponentTransform();
			Box += PrimitiveComponent->CalcBounds(ComponentToWorld).GetBox();
		}
	}

	return Box;
}

FVector AActor::GetActorWorldBoundsMin() const
{
	for (auto& Component : Components)
	{
		if (USceneComponent* SceneComponent = dynamic_cast<USceneComponent*>(Component))
		{
			if (SceneComponent->Min != FVector::ZeroVector)
			{
				return GetActorTransform().TransformPosition(SceneComponent->Min);
			}
		}
	}

	return FVector::ZeroVector;
}

FVector AActor::GetActorWorldBoundsMax() const
{
	for (auto& Component : Components)
	{
		if (USceneComponent* SceneComponent = dynamic_cast<USceneComponent*>(Component))
		{
			if (SceneComponent->Max != FVector::ZeroVector)
			{
				return GetActorTransform().TransformPosition(SceneComponent->Max);
			}
		}
	}

	return FVector::ZeroVector;
}

FVector AActor::GetActorLocalBoundsMin() const
{
	for (auto& Component : Components)
	{
		if (UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component))
		{
			if (PrimitiveComponent->Min != FVector::ZeroVector)
			{
				return PrimitiveComponent->Min;
			}
		}
	}

	return FVector::ZeroVector;
}

FVector AActor::GetActorLocalBoundsMax() const
{
	for (auto& Component : Components)
	{
		if (UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component))
		{
			if (PrimitiveComponent->Max != FVector::ZeroVector)
			{
				return PrimitiveComponent->Max;
			}
		}
	}

	return FVector::ZeroVector;
}

bool AActor::SetActorPosition(const FVector& InPosition)
{
	if (RootComponent)
	{
		const FVector Delta = InPosition - GetActorPosition();
		return RootComponent->MoveComponent(Delta, GetActorRotation());
	}

	return false;
}

bool AActor::SetActorRotation(const FVector& InRotation)
{
	return SetActorRotation(FQuat(InRotation));
}

bool AActor::SetActorRotation(const FQuat& InQuat)
{
	if (RootComponent)
	{
		return RootComponent->MoveComponent(FVector::ZeroVector, InQuat);
	}
	return false;
}

bool AActor::SetActorPositionAndRotation(const FVector& InPosition, const FVector& InRotation)
{
	if (RootComponent)
	{
		const FVector Delta = InPosition - GetActorPosition();
		return RootComponent->MoveComponent(Delta, FQuat(InRotation));
	}
	return false;
}

bool AActor::SetActorPositionAndRotation(const FVector& InPosition, const FQuat& InQuat)
{
	if (RootComponent)
	{
		const FVector Delta = InPosition - GetActorPosition();
		return RootComponent->MoveComponent(Delta, InQuat);
	}

	return false;
}

bool AActor::SetActorScale(const FVector& InScale)
{
	if (RootComponent)
	{
		RootComponent->SetWorldScale(InScale);
		return true;
	}

	return false;
}

FVector AActor::GetActorScale() const
{
	if (RootComponent)
	{
		return RootComponent->GetComponentScale();
	}
	return FVector::OneVector;
}

void AActor::AddWorldOffset(const FVector& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddWorldOffset(Delta);
	}
}

void AActor::AddActorWorldRotation(const FVector& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddWorldRotation(Delta);
	}
}

void AActor::AddActorWorldRotation(const FQuat& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddWorldRotation(Delta);
	}
}

void AActor::AddActorWorldTransform(const FTransform& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddWorldTransform(Delta);
	}
}

void AActor::AddActorWorldTransformKeepScale(const FTransform& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddWorldTransformKeepScale(Delta);
	}
}

void AActor::AddActorLocalOffset(const FVector& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddLocalOffset(Delta);
	}
}

void AActor::AddActorLocalRotation(const FVector& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddLocalRotation(Delta);
	}
}

void AActor::AddActorLocalRotation(const FQuat& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddLocalRotation(Delta);
	}
}

void AActor::AddActorLocalTransform(const FTransform& Delta)
{
	if (RootComponent)
	{
		RootComponent->AddLocalTransform(Delta);
	}
}

void AActor::SetActorTransform(const FTransform& InTransform)
{
	// InTransform은 월드 기준임
	if (RootComponent)
	{
		RootComponent->SetWorldTransform(InTransform);
	}
	else
	{
		UE_LOG("RootComponent is nullptr");
	}
}

void AActor::SetActorRelativePosition(const FVector& InPosition)
{
	if (RootComponent)
	{
		RootComponent->SetRelativePosition(InPosition);
	}
}

void AActor::SetActorRelativeRotation(const FVector& InRotation)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeRotation(InRotation);
	}
}

void AActor::SetActorRelativeRotation(const FQuat& InRotation)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeRotation(InRotation);
	}
}


void AActor::SetActorRelativeTransform(const FTransform& InTransform)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeTransform(InTransform);
	}	
}

void AActor::SetActorRelativeScale(const FVector& InScale)
{
	if (RootComponent)
	{
		RootComponent->SetRelativeScale(InScale);
	}
}

FVector AActor::GetActorRelativeScale() const
{
	if (RootComponent)
	{
		return RootComponent->GetRelativeScale();
	}
	return FVector::OneVector;
}

const char* AActor::GetTypeName()
{
	return "Actor";
}

bool AActor::Destroy()
{
	if (UWorld* World = GetWorld())
	{
		World->DestroyActor(this);
	}

	return true;
}

void AActor::SetRootComponent(USceneComponent* InRootComponent)
{
	if (InRootComponent == nullptr || InRootComponent->GetOwner() == this)
	{
		if (RootComponent != InRootComponent)
		{
			USceneComponent* OldRootComponent = RootComponent;
			RootComponent = InRootComponent;

			OldRootComponent->SetupAttachment(RootComponent);
		}
	}
}

void AActor::SetColor(FVector4 InColor)
{
	if (RootComponent == nullptr)
	{
		return;
	}

	UPrimitiveComponent* RootPrimitive = dynamic_cast<UPrimitiveComponent*>(RootComponent);
	if (RootPrimitive)
	{
		RootPrimitive->SetCustomColor(InColor);
	}

	for (auto& Component : Components)
	{
		UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component);
		if (PrimitiveComponent)
		{
			PrimitiveComponent->SetCustomColor(InColor);
		}
	}
}

void AActor::SetUseVertexColor(bool bUseVertexColor)
{
	if (RootComponent == nullptr)
	{
		return;
	}

	UPrimitiveComponent* RootPrimitive = dynamic_cast<UPrimitiveComponent*>(RootComponent);
	if (RootPrimitive)
	{
		RootPrimitive->SetUseVertexColor(bUseVertexColor);
	}

	for (auto& Component : Components)
	{
		UPrimitiveComponent* PrimitiveComponent = dynamic_cast<UPrimitiveComponent*>(Component);
		if (PrimitiveComponent)
		{
			PrimitiveComponent->SetUseVertexColor(bUseVertexColor);
		}
	}
}
