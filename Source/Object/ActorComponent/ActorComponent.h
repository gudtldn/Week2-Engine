#pragma once
#include "Core/EngineTypes.h"
#include "Core/Math/Vector.h"
#include "Core/UObject/Object.h"
#include "Core/UObject/ObjectMacros.h"

struct FVector;

class UActorComponent : public UObject
{
	DECLARE_CLASS(UActorComponent, UObject)

public:
	UActorComponent() = default;

	virtual void BeginPlay();
	virtual void Tick(float DeltaTime);
	virtual void EndPlay(EEndPlayReason::Type Reason);

	bool CanEverTick() const { return bCanEverTick; }

	virtual class AActor* GetOwner() const;
	virtual void SetOwner(AActor* InOwner) { Owner = InOwner; }

	FVector GetActorPosition() const;

protected:
	bool bCanEverTick = true;
	AActor* Owner = nullptr;
};

