#include "AsyncAction_ListenForGameplayMessage.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/GameInstance.h"

UAsyncAction_ListenForGameplayMessage* UAsyncAction_ListenForGameplayMessage::ListenForGameplayMessages(
	UObject* WorldContextObject, FGameplayTag Channel, UScriptStruct* PayloadType)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
	{
		return nullptr;
	}

	UAsyncAction_ListenForGameplayMessage* Action = NewObject<UAsyncAction_ListenForGameplayMessage>();
	Action->WorldPtr = World;
	Action->ChannelToRegister = Channel;
	Action->MessageStructType = PayloadType;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UAsyncAction_ListenForGameplayMessage::Activate()
{
	Super::Activate();

	if (UWorld* World = WorldPtr.Get())
	{
		UGameplayMessageSubsystem& Router = UGameplayMessageSubsystem::Get(World);

		TWeakObjectPtr<UAsyncAction_ListenForGameplayMessage> WeakThis(this);

		ListenerHandle = Router.RegisterListenerInternal(ChannelToRegister,
		                                                 [WeakThis](FGameplayTag Channel,
		                                                            const UScriptStruct* StructType,
		                                                            const void* Payload)
		                                                 {
			                                                 if (UAsyncAction_ListenForGameplayMessage* StrongThis =
				                                                 WeakThis.Get())
			                                                 {
				                                                 StrongThis->HandleMessageReceived(
					                                                 Channel, StructType, Payload);
			                                                 }
		                                                 }, MessageStructType.Get());
	}
}

bool UAsyncAction_ListenForGameplayMessage::GetPayload(int32& OutPayload)
{
	checkNoEntry();
	return false;
}

DEFINE_FUNCTION(UAsyncAction_ListenForGameplayMessage::execGetPayload)
{
	Stack.MostRecentPropertyAddress = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	void* MessagePtr = Stack.MostRecentPropertyAddress;
	FStructProperty* StructProp = CastField<FStructProperty>(Stack.MostRecentProperty);
	P_FINISH;

	bool bSuccess = false;

	if ((StructProp != nullptr)
		&& (StructProp->Struct != nullptr)
		&& (MessagePtr != nullptr)
		&& (StructProp->Struct == P_THIS->MessageStructType.Get())
		&& (P_THIS->ReceivedMessagePayloadPtr != nullptr))
	{
		StructProp->Struct->CopyScriptStruct(MessagePtr, P_THIS->ReceivedMessagePayloadPtr);
		bSuccess = true;
	}

	*(bool*)RESULT_PARAM = bSuccess;
}

void UAsyncAction_ListenForGameplayMessage::HandleMessageReceived(FGameplayTag Channel, const UScriptStruct* StructType,
                                                                  const void* Payload)
{
	if (!MessageStructType.Get() || (MessageStructType.Get() == StructType))
	{
		ReceivedMessagePayloadPtr = Payload;

		OnMessageReceived.Broadcast(this, Channel);
		ReceivedMessagePayloadPtr = nullptr;
	}
}
