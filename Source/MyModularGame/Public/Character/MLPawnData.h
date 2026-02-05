#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MLPawnData.generated.h"

class APawn;

/**
 * Minimal pawn bootstrap data used by the experience.
 */
UCLASS(BlueprintType, Const)
class MYMODULARGAME_API UMLPawnData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** Default pawn class for this data set. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pawn")
    TSubclassOf<APawn> PawnClass;
};
