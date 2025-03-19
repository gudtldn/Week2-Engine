// ReSharper disable CppClangTidyBugproneMacroParentheses
#pragma once
#include <memory>
#include "Class.h"
#include "Core/HAL/PlatformMemory.h"


#define DECLARE_CLASS(TClass, TSuperClass) \
private: \
	TClass(const TClass&) = delete; \
	TClass& operator=(const TClass&) = delete; \
	TClass(TClass&&) = delete; \
	TClass& operator=(TClass&&) = delete; \
public: \
	using Super = TSuperClass; \
	using ThisClass = TClass; \
	inline static UClass* StaticClass() \
	{ \
		static std::unique_ptr<UClass, UClassDeleter> StaticClassInfo = nullptr; \
		if (!StaticClassInfo) \
		{ \
			constexpr size_t ClassSize = sizeof(UClass); \
			void* RawMemory = FPlatformMemory::Malloc<EAT_Object>(ClassSize); \
			UClass* ClassPtr = new (RawMemory) UClass{ #TClass, static_cast<uint32>(sizeof(TClass)), static_cast<uint32>(alignof(TClass)), TSuperClass::StaticClass() }; \
			StaticClassInfo = std::unique_ptr<UClass, UClassDeleter>(ClassPtr, UClassDeleter{}); \
		} \
		return StaticClassInfo.get(); \
	} \


#define DECLARE_UINTERFACE(TClass, TSuperClass) \
	DECLARE_CLASS(TClass, TSuperClass) \
public: \
	virtual ~TClass() = default;

#define DECLARE_IINTERFACE(IInterface, UInterface) \
protected: \
	virtual ~IInterface() = default; \
public: \
	using UClassType = UInterface; \
	using ThisClass = IInterface;
