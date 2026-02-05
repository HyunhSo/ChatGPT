#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ModularPlayerController.h"
#include "MLPlayerController.generated.h"

UCLASS()
class MYMODULARGAME_API AMLPlayerController : public AModularPlayerController
{
    GENERATED_BODY()

public:
    virtual void OnPossess(APawn* InPawn) override;

protected:
    virtual void OnRep_PlayerState() override;

private:
    void NotifyPawnExtension(APawn* InPawn) const;
};
