#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularPawn.h"
#include "MyModularPawn.generated.h"

class UMyPawnExtensionComponent;
class UMyHeroComponent;

UCLASS(Blueprintable)
class MYMODULARGAME_API AMyModularPawn : public AModularPawn
{
    GENERATED_BODY()

public:
    AMyModularPawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Init", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMyPawnExtensionComponent> PawnExtensionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hero", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UMyHeroComponent> HeroComponent;
};
