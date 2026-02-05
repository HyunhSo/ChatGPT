#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MLExperienceDefinition.generated.h"

class UMLPawnData;

/**
 * Minimal experience definition that carries startup pawn data.
 */
UCLASS(BlueprintType, Const)
class MYMODULARGAME_API UMLExperienceDefinition : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Pawn data used once the experience is ready. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Experience")
    TObjectPtr<const UMLPawnData> DefaultPawnData;
};
