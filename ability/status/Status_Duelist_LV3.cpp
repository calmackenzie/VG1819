#pragma once
#include "ability/status/Status.h"
#include "unit/Unit.h"
//Rock

namespace ability
{
	Status_Duelist_LV3::Status_Duelist_LV3() : Status_LV::Status_LV()
	{

	}

	int Status_Duelist_LV3::effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent * p_event)
	{
		if (Status_LV::effect(p_type, p_event) == 0)
		{
			AbilityNode* node = AbilityNodeManager::getInstance()->findNode("ChangeAbilityDescriptionNode");

			node->effect(m_unit, "Slay", "power", 2);//increase slay dmg by 2
			node->effect(m_unit, "Dodge", "CD", -1);//decrease dodge cd by 1

			return 0;
		}
		return 1;
	}
}