#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularGameMode.h"
#include "MyModularGameMode.generated.h"

class UMyGameData;

/**
 * ModularGameplay + InitState 샘플 클래스들을 기본값으로 사용하는 GameMode.
 */
UCLASS(Blueprintable)
class MYMODULARGAME_API AMyModularGameMode : public AModularGameModeBase
{
    GENERATED_BODY()

public:
    AMyModularGameMode();

protected:
    virtual void InitGameState() override;

private:
    UPROPERTY(EditDefaultsOnly, Category = "GameData")
    TObjectPtr<UMyGameData> GameData;
};
