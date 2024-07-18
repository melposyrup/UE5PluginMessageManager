#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "PEMessageSubsystem.h"
#include "Engine/CancellableAsyncAction.h"

#include "AsyncAction_ListenForGameplayMessage.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAsyncGameplayMessageDelegate, UAsyncAction_ListenForGameplayMessage*,
                                             ProxyObject, FGameplayTag, ActualChannel);

UCLASS(BlueprintType, meta=(HasDedicatedAsyncNode))
class PEMESSAGE_API UAsyncAction_ListenForGameplayMessage : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PEMessage", meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true"))
	static UAsyncAction_ListenForGameplayMessage* ListenForGameplayMessages(
		UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType);

	virtual void Activate() override;

	UFUNCTION(BlueprintCallable, CustomThunk,Category = "PEMessage", meta = (CustomStructureParam = "OutPayload"))
	bool GetPayload(UPARAM(ref) int32& OutPayload);
	DECLARE_FUNCTION(execGetPayload);

public:
	UPROPERTY(BlueprintAssignable)
	FAsyncGameplayMessageDelegate OnMessageReceived;

private:
	void HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType, const void* Payload);

private:
	const void* ReceivedMessagePayloadPtr = nullptr;

	TWeakObjectPtr<UWorld> WorldPtr;
	FGameplayTag ChannelToRegister;
	TWeakObjectPtr<UScriptStruct> MessageStructType = nullptr;

	FGameplayMessageListenerHandle ListenerHandle;
};
