#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularPawn.h"
#include "MyModularPawn.generated.h"

class UMyPawnInitComponent;

UCLASS(Blueprintable)
class MYMODULARGAME_API AMyModularPawn : public AModularPawn
{
    GENERATED_BODY()

public:
    AMyModularPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Init", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMyPawnInitComponent> PawnInitComponent;
};
