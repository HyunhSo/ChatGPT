#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "MyPawnData.generated.h"

class APawn;

/**
 * Pawn 구성에 필요한 최소 핸들을 담는 데이터 에셋.
 */
UCLASS(BlueprintType, Const)
class MYMODULARGAME_API UMyPawnData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
    TSubclassOf<APawn> PawnClass;
};

