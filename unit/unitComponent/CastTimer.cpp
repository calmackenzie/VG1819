#include "CastTimer.h"
#include "kitten/K_GameObjectManager.h"
#include "TimerSymbol.h"
unit::CastTimer::CastTimer()
	:m_timerSymbol(nullptr)
{
	m_cast = false;
	m_pack = nullptr;
}

unit::CastTimer::~CastTimer()
{
	if (m_pack != nullptr)
	{
		delete m_pack;
	}
}

bool unit::CastTimer::isCasting()
{
	return m_cast;
}

std::vector<kitten::K_GameObject*> unit::CastTimer::getTarget()
{
	return m_pack->m_targetTilesGO;
}

void unit::CastTimer::set(std::string p_abilityName, ability::AbilityInfoPackage * p_pack, int p_timer)
{
	if (p_timer > 0 && p_timer <= 10)
	{
		m_abilityName = p_abilityName;
		m_pack = p_pack;
		m_timer = p_timer;
		m_cast = true;

		if (m_timerSymbol == nullptr)
			m_timerSymbol = kitten::K_GameObjectManager::getInstance()->createNewGameObject("cast_timer.json");

		m_timerSymbol->getComponent<TimerSymbol>()->changeTexture(m_timer);
	}
}

int unit::CastTimer::changeTimer(int p_n)
{
	if (!m_cast)
		return -1;

	m_timer += p_n;
	if (m_timer <= 0)
	{
		m_cast = false;
		cast();
		return 0;
	}
	else
		m_timerSymbol->getComponent<TimerSymbol>()->changeTexture(m_timer);

	return 1;
}

void unit::CastTimer::cancelCast()
{
	if (m_cast)
	{
		delete m_pack;
		m_pack = nullptr;
		m_cast = false;

		kitten::K_GameObjectManager::getInstance()->destroyGameObject(m_timerSymbol);
		m_timerSymbol = nullptr;
	}
}

void unit::CastTimer::cast()
{
	ability::AbilityManager::getInstance()->useAbility(m_abilityName, m_pack);
	m_pack = nullptr;

	kitten::K_GameObjectManager::getInstance()->destroyGameObject(m_timerSymbol);
	m_timerSymbol = nullptr;
}
