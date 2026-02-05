#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularGameState.h"
#include "MyModularGameState.generated.h"

class UMyGameData;

UCLASS(Blueprintable)
class MYMODULARGAME_API AMyModularGameState : public AModularGameStateBase
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "GameData")
    const UMyGameData* GetGameData() const;

    void SetGameData(const UMyGameData* InGameData);

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    UPROPERTY(Replicated)
    TObjectPtr<const UMyGameData> GameData = nullptr;
};
