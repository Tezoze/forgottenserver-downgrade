#include "otpch.h"
#include "talkaction.h"

bool TalkActions::reload()
{
    clear(false);
    return loadFromXml();
} 