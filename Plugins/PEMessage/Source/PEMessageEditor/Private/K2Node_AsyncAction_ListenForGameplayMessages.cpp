#include "K2Node_AsyncAction_ListenForGameplayMessages.h"

#include "AsyncAction_ListenForGameplayMessage.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintFunctionNodeSpawner.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_CallFunction.h"
#include "K2Node_TemporaryVariable.h"
#include "KismetCompiler.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_MODULE(FDefaultModuleImpl, PEMessageEditor);

void UK2Node_AsyncAction_ListenForGameplayMessages::PostReconstructNode()
{
	Super::PostReconstructNode();

	RefreshOutputPayloadType();
}

void UK2Node_AsyncAction_ListenForGameplayMessages::PinDefaultValueChanged(UEdGraphPin* ChangedPin)
{
	if (ChangedPin == GetPayloadTypePin())
	{
		if (ChangedPin->LinkedTo.Num() == 0)
		{
			RefreshOutputPayloadType();
		}
	}
}

void UK2Node_AsyncAction_ListenForGameplayMessages::GetMenuActions(
	FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	Super::GetMenuActions(ActionRegistrar);

	struct GetMenuAction_Utils
	{
		static void SetNodeFunc(UEdGraphNode* NewNode, bool, TWeakObjectPtr<UFunction> FunctionPtr)
		{
			UK2Node_AsyncAction_ListenForGameplayMessages* AsyncTaskNode = CastChecked<
				UK2Node_AsyncAction_ListenForGameplayMessages>(NewNode);
			if (FunctionPtr.IsValid())
			{
				UFunction* Func = FunctionPtr.Get();
				FObjectProperty* ReturnProp = CastFieldChecked<FObjectProperty>(Func->GetReturnProperty());

				AsyncTaskNode->ProxyFactoryFunctionName = Func->GetFName();
				AsyncTaskNode->ProxyFactoryClass = Func->GetOuterUClass();
				AsyncTaskNode->ProxyClass = ReturnProp->PropertyClass;
			}
		}
	};

	UClass* NodeClass = GetClass();
	ActionRegistrar.RegisterClassFactoryActions<UAsyncAction_ListenForGameplayMessage>(
		FBlueprintActionDatabaseRegistrar::FMakeFuncSpawnerDelegate::CreateLambda(
			[NodeClass](const UFunction* FactoryFunc)-> UBlueprintNodeSpawner* {
				UBlueprintNodeSpawner* NodeSpawner = UBlueprintFunctionNodeSpawner::Create(FactoryFunc);
				check(NodeSpawner != nullptr);
				NodeSpawner->NodeClass = NodeClass;

				TWeakObjectPtr<UFunction> FunctionPtr = MakeWeakObjectPtr(const_cast<UFunction*>(FactoryFunc));
				NodeSpawner->CustomizeNodeDelegate = UBlueprintNodeSpawner::FCustomizeNodeDelegate::CreateStatic(
					GetMenuAction_Utils::SetNodeFunc, FunctionPtr);

				return NodeSpawner;
			}));
}

void UK2Node_AsyncAction_ListenForGameplayMessages::AllocateDefaultPins()
{
	Super::AllocateDefaultPins();

	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, FName("Payload"));
}

bool UK2Node_AsyncAction_ListenForGameplayMessages::HandleDelegates(
	const TArray<FBaseAsyncTaskHelper::FOutputPinAndLocalVariable>& VariableOutputs, UEdGraphPin* ProxyObjectPin,
	UEdGraphPin*& InOutLastThenPin, UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;

	for (TFieldIterator<FMulticastDelegateProperty> PropertyIt(ProxyClass); PropertyIt && bIsErrorFree; ++PropertyIt)
	{
		UEdGraphPin* LastActivatedThenPin = nullptr;

		bIsErrorFree &= FBaseAsyncTaskHelper::HandleDelegateImplementation(*PropertyIt,
		                                                                   VariableOutputs, ProxyObjectPin,
		                                                                   InOutLastThenPin, LastActivatedThenPin, this,
		                                                                   SourceGraph, CompilerContext);

		bIsErrorFree &= HandlePayloadImplementation(*PropertyIt,
		                                            VariableOutputs[0], VariableOutputs[2], VariableOutputs[1],
		                                            LastActivatedThenPin, SourceGraph, CompilerContext);
	}

	return bIsErrorFree;
}

bool UK2Node_AsyncAction_ListenForGameplayMessages::HandlePayloadImplementation(
	FMulticastDelegateProperty* CurrentProperty, const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ProxyObjectVar,
	const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& PayloadVar,
	const FBaseAsyncTaskHelper::FOutputPinAndLocalVariable& ActualChannelVar, UEdGraphPin*& InOutLastActivatedThenPin,
	UEdGraph* SourceGraph, FKismetCompilerContext& CompilerContext)
{
	bool bIsErrorFree = true;
	const UEdGraphPin* PayloadPin = GetPayloadPin();
	const UEdGraphSchema_K2* Schema = CompilerContext.GetSchema();

	check(CurrentProperty && SourceGraph && Schema);

	const FEdGraphPinType& PinType = PayloadPin->PinType;

	UK2Node_TemporaryVariable* TempVarOutput = CompilerContext.SpawnInternalVariable(
		this, PinType.PinCategory, PinType.PinSubCategory, PinType.PinSubCategoryObject.Get(),
		PinType.ContainerType, PinType.PinValueType);

	UK2Node_CallFunction* const CallGetPayloadNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(
		this, SourceGraph);
	CallGetPayloadNode->FunctionReference.SetExternalMember(TEXT("GetPayload"), CurrentProperty->GetOwnerClass());
	CallGetPayloadNode->AllocateDefaultPins();

	UEdGraphPin* GetPayloadCallSelfPin = Schema->FindSelfPin(*CallGetPayloadNode, EGPD_Input);
	if (GetPayloadCallSelfPin)
	{
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadCallSelfPin, ProxyObjectVar.TempVar->GetVariablePin());

		UEdGraphPin* GetPayloadExecPin = CallGetPayloadNode->FindPinChecked(UEdGraphSchema_K2::PN_Execute);
		UEdGraphPin* GetPayloadThenPin = CallGetPayloadNode->FindPinChecked(UEdGraphSchema_K2::PN_Then);

		UEdGraphPin* GetPayloadPin = CallGetPayloadNode->FindPinChecked(TEXT("OutPayload"));
		bIsErrorFree &= Schema->TryCreateConnection(TempVarOutput->GetVariablePin(), GetPayloadPin);

		UK2Node_AssignmentStatement* AssignNode = CompilerContext.SpawnIntermediateNode<UK2Node_AssignmentStatement>(
			this, SourceGraph);
		AssignNode->AllocateDefaultPins();
		bIsErrorFree &= Schema->TryCreateConnection(GetPayloadThenPin, AssignNode->GetExecPin());

		bIsErrorFree &= Schema->TryCreateConnection(PayloadVar.TempVar->GetVariablePin(), AssignNode->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetVariablePin());

		bIsErrorFree &= Schema->TryCreateConnection(AssignNode->GetValuePin(), TempVarOutput->GetVariablePin());
		AssignNode->NotifyPinConnectionListChanged(AssignNode->GetValuePin());

		bIsErrorFree &= CompilerContext.MovePinLinksToIntermediate(*InOutLastActivatedThenPin,
		                                                           *AssignNode->GetThenPin()).CanSafeConnect();
		bIsErrorFree &= Schema->TryCreateConnection(InOutLastActivatedThenPin, GetPayloadExecPin);
	}

	return bIsErrorFree;
}

void UK2Node_AsyncAction_ListenForGameplayMessages::RefreshOutputPayloadType()
{
	UEdGraphPin* PayloadPin = GetPayloadPin();
	UEdGraphPin* PayloadTypePin = GetPayloadTypePin();

	if (PayloadTypePin->DefaultObject != PayloadPin->PinType.PinSubCategoryObject)
	{
		PayloadPin->PinType.PinSubCategoryObject = PayloadTypePin->DefaultObject;
		PayloadPin->PinType.PinCategory = (PayloadTypePin->DefaultObject == nullptr)
			                                  ? UEdGraphSchema_K2::PC_Wildcard
			                                  : UEdGraphSchema_K2::PC_Struct;
	}
}

UEdGraphPin* UK2Node_AsyncAction_ListenForGameplayMessages::GetPayloadPin() const
{
	UEdGraphPin* Pin = FindPinChecked(FName("Payload"));
	check(Pin->Direction == EGPD_Output);
	return Pin;
}

UEdGraphPin* UK2Node_AsyncAction_ListenForGameplayMessages::GetPayloadTypePin() const
{
	UEdGraphPin* Pin = FindPinChecked(FName("PayloadType"));
	check(Pin->Direction == EGPD_Input);
	return Pin;
}
