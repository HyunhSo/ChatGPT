#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularGameMode.h"
#include "MyModularGameMode.generated.h"

/**
 * ModularGameplay 기반 컴포넌트 주입을 지원하는 기본 GameMode.
 */
UCLASS(Blueprintable)
class MYMODULARGAME_API AMyModularGameMode : public AModularGameModeBase
{
    GENERATED_BODY()

public:
    AMyModularGameMode();
};
