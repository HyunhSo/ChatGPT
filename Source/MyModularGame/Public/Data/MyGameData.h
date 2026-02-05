#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MyGameData.generated.h"

class UMyPawnData;

/**
 * 레벨/모드 단위 기본 PawnData를 제공하는 최소 GameData.
 */
UCLASS(BlueprintType, Const)
class MYMODULARGAME_API UMyGameData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game")
    TObjectPtr<const UMyPawnData> DefaultPawnData = nullptr;
};
