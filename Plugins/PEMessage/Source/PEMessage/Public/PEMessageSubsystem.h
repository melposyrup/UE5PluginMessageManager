// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

#include "PEMessageSubsystem.generated.h"

class UGameplayMessageSubsystem;

USTRUCT()
struct FGameplayMessageListenerData
{
	GENERATED_BODY()

	TFunction<void(FGameplayTag, const UScriptStruct*, const void*)> ReceivedCallback;

	int32 HandleID;
};

USTRUCT(BlueprintType)
struct PEMESSAGE_API FGameplayMessageListenerHandle
{
	GENERATED_BODY()
	FGameplayMessageListenerHandle()
	{
	}

	bool IsValid() const { return ID != 0; }

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameplayMessageSubsystem> Subsystem;

	UPROPERTY(Transient)
	FGameplayTag Channel;

	UPROPERTY(Transient)
	int32 ID = 0;

	friend UGameplayMessageSubsystem;

	FGameplayMessageListenerHandle(UGameplayMessageSubsystem* InSubsystem, FGameplayTag InChannel,
	                               int32 InID) : Subsystem(InSubsystem), Channel(InChannel), ID(InID)
	{
	}
};


UCLASS(BlueprintType)
class PEMESSAGE_API UGameplayMessageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UGameplayMessageSubsystem& Get(const UObject* WorldContextObject);

	FGameplayMessageListenerHandle RegisterListenerInternal(
		FGameplayTag Channel,
		TFunction<void(FGameplayTag, const UScriptStruct*, const void*)>&& Callback,
		const UScriptStruct* StructType);

protected:
	UFUNCTION(BlueprintCallable, CustomThunk, Category = "PE",
		meta=(CustomStructureParam="Message", AllowAbstract="false", DisplayName="PE_BroadcastMessage"))
	void BroadcastMessage(FGameplayTag Channel, const int32& Message);
	DECLARE_FUNCTION(execBroadcastMessage);

private:
	void BroadcastMessageInternal(FGameplayTag Channel, const UScriptStruct* StructType, const void* MessageBytes);

private:
	struct FChannelListenerList
	{
		TArray<FGameplayMessageListenerData> Listeners;
		int32 HandleID = 0;
	};

private:
	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
};
