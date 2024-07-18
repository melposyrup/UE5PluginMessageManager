#pragma once

#include "CoreMinimal.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_AsyncAction_ListenForGameplayMessages.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(PEMessageEditor, All, All)

UCLASS(BlueprintType)
class PEMESSAGEEDITOR_API UK2Node_AsyncAction_ListenForGameplayMessages : public UK2Node_AsyncAction
{
	GENERATED_BODY()

	virtual void PostReconstructNode() override;
	virtual void PinDefaultValueChanged(UEdGraphPin* ChangedPin) override;

	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual void AllocateDefaultPins() override;

protected:
	virtual bool HandleDelegates(const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs,
	                             UEdGraphPin* ProxyObjectPin, UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph,
	                             FKismetCompilerContext& CompilerContext) override;

private:
	bool HandlePayloadImplementation(
		FMulticastDelegateProperty* CurrentProperty,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
		const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar,
		UEdGraphPin*& InOutLastActivatedThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext
	);

private:
	void RefreshOutputPayloadType();

	UEdGraphPin* GetPayloadPin() const;
	UEdGraphPin* GetPayloadTypePin() const;
};
