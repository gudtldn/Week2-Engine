#pragma once
#include "Core/UObject/Object.h"
#include "Core/UObject/ObjectMacros.h"


class UInterface : public UObject
{
	DECLARE_UINTERFACE(UInterface, UObject)
};


class IInterface
{
	DECLARE_IINTERFACE(IInterface, UInterface)
};
