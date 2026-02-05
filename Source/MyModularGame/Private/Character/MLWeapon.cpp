#include "Character/MLWeapon.h"

AMLWeapon::AMLWeapon(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryActorTick.bCanEverTick = false;
}
