#pragma once

#include "NativeGameplayTags.h"

namespace MyInitStateTags
{
    MYMODULARGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_Spawned);
    MYMODULARGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_DataAvailable);
    MYMODULARGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_DataInitialized);
    MYMODULARGAME_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_InitState_GameplayReady);
}
