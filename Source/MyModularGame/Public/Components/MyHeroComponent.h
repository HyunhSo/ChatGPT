#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "MyHeroComponent.generated.h"

/**
 * PawnData 초기화 이후, 로컬 플레이어 조종 상태에서 입력/카메라 셋업 훅을 호출한다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MYMODULARGAME_API UMyHeroComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    UMyHeroComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintImplementableEvent, Category = "Hero")
    void BP_SetupPlayerInput();

private:
    void TrySetupPlayerInput();

private:
    bool bPlayerInputSetup = false;
};
