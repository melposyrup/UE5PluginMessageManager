// Copyright Epic Games, Inc. All Rights Reserved.

#include "PEMessageSubsystem.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"

UGameplayMessageSubsystem& UGameplayMessageSubsystem::Get(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);
	check(World);
	UGameplayMessageSubsystem* Router = UGameInstance::GetSubsystem<UGameplayMessageSubsystem>(World->GetGameInstance());
	check(Router);
	return *Router;
}

FGameplayMessageListenerHandle UGameplayMessageSubsystem::RegisterListenerInternal(FGameplayTag Channel,
                                                                                   TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback, const UScriptStruct* StructType)
{
	FChannelListenerList& List = ListenerMap.FindOrAdd(Channel);
	FGameplayMessageListenerData& Entry = List.Listeners.AddDefaulted_GetRef();
	Entry.ReceivedCallback = MoveTemp(Callback);
	Entry.HandleID = ++List.HandleID;
	
	return FGameplayMessageListenerHandle(this,Channel,Entry.HandleID);
}

void UGameplayMessageSubsystem::BroadcastMessage(FGameplayTag Channel, const int32& Message)
{
	checkNoEntry();
}

DEFINE_FUNCTION(UGameplayMessageSubsystem::execBroadcastMessage)
{
	P_GET_STRUCT(FGameplayTag, Channel);

	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);

	P_FINISH

	if (ensure((StructProp != nullptr) && (StructProp->Struct!=nullptr) && (MessagePtr!=nullptr)))
	{
		P_THIS->BroadcastMessageInternal(Channel, StructProp->Struct,MessagePtr);
	}
}

void UGameplayMessageSubsystem::BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType,
	const void* MessageBytes)
{
	bool bOnInitialTag = true;

	for (FGameplayTag Tag = Channel; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FChannelListenerList* pList = ListenerMap.Find(Tag))
		{
			for (const FGameplayMessageListenerData& Listener : pList->Listeners)
			{
				if (bOnInitialTag)
				{
					Listener.ReceivedCallback(Channel,StructType,MessageBytes);
				}
			}
		}
	}
}

IMPLEMENT_MODULE(FDefaultModuleImpl, PEMessageSubsystem)
