#include "Unit.h"
#include "unit/unitComponent/UnitMove.h"
#include "kitten/K_GameObject.h"
#include "unitInteraction/UnitInteractionManager.h"
#include <iostream>

#include "_Project\UniversalPfx.h"

// Networking
#include "networking\ClientGame.h"
//@Rock

namespace unit
{
	Unit::Unit()
	{
		m_commander = nullptr;
		m_turn = nullptr;
		m_statusContainer = new StatusContainer();
		m_statusContainer->m_unit = this;
		m_cdRecorder = new CooldownRecorder();
		m_castTimer = new CastTimer();

		setJoinAD();
	}


	Unit::~Unit()
	{
		delete m_statusContainer;
		delete m_cdRecorder;
		delete m_castTimer;
		for (auto it = m_ADList.begin(); it != m_ADList.end(); it++)
		{
			delete it->second;
		}
		if (isCommander())
		{
			delete m_commander;
		}
	}

	//status
	/*
	void Unit::addStatus(ability::Status *p_newStatus)
	{
		m_statusContainer->addStatus(p_newStatus);
	}

	bool Unit::removeStatus(ability::Status * p_oldStatus)
	{
		return m_statusContainer->removeStatus(p_oldStatus);
	}

	ability::Status* Unit::getStatus(const std::string & p_name)
	{
		return m_statusContainer->getStatus(p_name);
	}*/

	StatusContainer * Unit::getStatusContainer()
	{
		return m_statusContainer;
	}

	void Unit::triggerTP(ability::TimePointEvent::TPEventType p_tp, ability::TimePointEvent* p_event)
	{
		if (p_event == nullptr)
		{
			p_event = new ability::TimePointEvent(p_tp);
		}
		m_statusContainer->triggerTP(p_tp,p_event);

		//special cases
		kitten::K_GameObject* tileGO = getTile();
		switch (p_tp)
		{
		case ability::TimePointEvent::Turn_Start:
		case ability::TimePointEvent::Turn_End:
		case ability::TimePointEvent::New_Tile:
			if(tileGO != nullptr)
				tileGO->getComponent<TileInfo>()->effect(p_tp, this);
			break;
		default:
			break;
		}
	}

	void Unit::setJoinAD()
	{
		m_joinAD.m_stringValue["name"] = ACTION_JOIN;
		m_joinAD.m_intValue["target"] = 1;
		m_joinAD.m_intValue["need_unit"] = 1;
		m_joinAD.m_intValue["min_range"] = 1;
		m_joinAD.m_intValue["max_range"] = 1;
	}

	void Unit::join()
	{
		if (isCommander())//commander can't join to another unit
			return;

		for (std::string it : m_tags)//strucutre can't join to another unit
		{
			if (it == STRUCTURE)
				return;
		}

		UnitInteractionManager::getInstance()->request(this, &m_joinAD);
	}

	void Unit::levelup()
	{
		//reset tile
		m_attachedObject->getComponent<UnitMove>()->reset();

		std::cout << m_name << " is LV "<< m_attributes[UNIT_LV] << std::endl;
		if (m_attributes[UNIT_LV] > 0 && m_attributes[UNIT_LV] < 3)
		{
			m_attributes[UNIT_LV]++;
			ability::TimePointEvent* t = new ability::TimePointEvent(ability::TimePointEvent::Level_Up);
			t->putInt(UNIT_LV, m_attributes[UNIT_LV]);
			triggerTP(ability::TimePointEvent::Level_Up, t);
		}
	}

	void Unit::addCommander(Commander* p_c)
	{
		m_commander = p_c;
		m_commander->init(this);
	}

	bool Unit::isCommander()
	{
		return m_commander != nullptr;
	}

	void Unit::manipulateTile()
	{
		m_commander->manipulateTile();
	}

	void Unit::summonUnit()
	{
		m_commander->spawnUnit();
	}

	//turn
	void Unit::turnStart(UnitTurn * p_t)
	{
		assert(m_turn == nullptr);
		m_turn = p_t;

		m_cdRecorder->reduceCD();//reduce cd at start of turn
		m_castTimer->changeTimer();//reduce ct at start of turn
		if (m_castTimer->isCasting())
		{
			playerSkipTurn();//if it still cast, it skips turn
		}
	}

	bool Unit::canMove()
	{
		if (m_attributes[UNIT_BASE_MV] <= 0)//unit can not move, like structure
			return false;
		else if (m_turn != nullptr)//this is unit's turn and check if it can move
			return m_turn->move;
		return true;
	}

	bool Unit::canAct()
	{
		assert(m_turn != nullptr);
		return m_turn->act;
	}

	void Unit::moveDone()
	{
		if (m_turn != nullptr)//move by action
		{
			bool moveDone = false;
			if (!m_path.empty())//has next tile in path
			{
				auto it = m_path.erase(m_path.begin());//remove first tile
				if (it != m_path.end())
				{
					move(*it);//move to next
				}
				else
					moveDone = true;
			}
			else
				moveDone = true;

			if (moveDone)
			{
				m_turn->move = false;
				m_turn->checkTurn();
			}
		}

		if (lateDestroy)
			destroy();
	}

	void Unit::actDone()
	{
		assert(m_turn != nullptr);
		m_turn->act = false;
		m_turn->checkTurn();
	}

	bool Unit::isTurn()
	{
		//if m_turn is nullptr, it means its not this unit turn
		return m_turn != nullptr;
	}

	void Unit::turnEnd()
	{
		assert(m_turn != nullptr);
		m_turn = nullptr;
	}

	void Unit::playerSkipTurn()
	{
		assert(m_turn != nullptr);
		m_turn->turnEnd();
	}

	kitten::K_GameObject * Unit::getTile()
	{
		return m_attachedObject->getComponent<unit::UnitMove>()->getTile();
	}

	void Unit::move()//move by instruction
	{
		if (!canMove())
			return;

		unit::UnitMove* moveComponet = m_attachedObject->getComponent<unit::UnitMove>();
		moveComponet->attempToMove();
	}

	void Unit::move(int p_min, int p_max)//move by ability with range
	{
		if (!canMove())
			return;

		unit::UnitMove* moveComponet = m_attachedObject->getComponent<unit::UnitMove>();
		moveComponet->attempToMove(p_min, p_max);
	}

	void Unit::move(kitten::K_GameObject * p_tile)//move by ability with fixed target
	{
		if (!canMove())
			return;

		if (networking::ClientGame::getInstance())
		{
			int unitIndex = networking::ClientGame::getInstance()->getUnitGameObjectIndex(m_attachedObject);
			int posX = p_tile->getComponent<TileInfo>()->getPosX();
			int posY = p_tile->getComponent<TileInfo>()->getPosY();

			//networking::ClientGame::getInstance()->moveUnit(unitIndex, posX, posY);
			networking::ClientGame::getInstance()->sendMovementPacket(unitIndex, posX, posY);
		}

		unit::UnitMove* moveComponet = m_attachedObject->getComponent<unit::UnitMove>();
		moveComponet->move(p_tile);
	}

	void Unit::move(std::vector<kitten::K_GameObject*> p_path)
	{
		m_path = p_path;
		move(m_path[0]);
	}

	int Unit::useAbility(const std::string& p_abilityName)
	{
		if (!canAct())
			return -1;

		AbilityDescription* ad;
		bool find = m_ADList.find(p_abilityName) != m_ADList.end();
		if (m_ADList.find(p_abilityName) != m_ADList.end())
		{
			std::cout << "use ability: " << p_abilityName << std::endl;
			ad = m_ADList[p_abilityName];
		}
		else
		{
			std::cout << "Ability: " << p_abilityName << " isn't found" << std::endl;
			return -2;//doesn't have ability
		}

		//non commander unit will check unit's lv
		if (m_attributes[UNIT_LV] < ad->m_intValue[UNIT_LV] && !isCommander())
		{
			std::cout << p_abilityName << "require lv (" << ad->m_intValue["lv"] << ") " << std::endl;
			std::cout << m_name << " is lv (" << m_attributes["lv"] << ")" << std::endl;
			return -3;//means unit can not use this ability
		}

		//check cd
		if (m_cdRecorder->checkCD(ad) != 0)
		{//it's in cool down
			std::cout << p_abilityName << " is in Cooldown. CD remain:"<<m_cdRecorder->checkCD(ad) << std::endl;
			return -4;
		}

		m_cdRecorder->addCD(ad);
		UnitInteractionManager::getInstance()->request(this, ad);

		return 0;
	}

	void Unit::cancelAbility(AbilityDescription* p_ad)
	{
		m_cdRecorder->cancelCD(p_ad);
	}

	void Unit::setCast(AbilityDescription * p_ad, ability::AbilityInfoPackage * p_pack)
	{
		std::string name = p_ad->m_stringValue["name"];
		int time = p_ad->m_intValue[UNIT_CT];
		m_castTimer->set(name,p_pack,time);
		playerSkipTurn();
	}

	void Unit::cancelCast()
	{
		m_castTimer->cancelCast();
	}

	int Unit::destroyedByDamage()
	{
		//TO DO: send destroyed event
		destroy();
		return 0;
	}

	int Unit::destroyedByJoin()
	{
		//TO DO: send destroyed event
		lateDestroy = true;
		return 0;
	}

	void Unit::destroy()
	{	
		const glm::vec3& pos = getTransform().getTranslation();
		auto pfxInstance = UniversalPfx::getInstance();
		assert(pfxInstance != nullptr);
		pfxInstance->playEffect(UNIT_DEATH_EFFECT_NAME, pos);
		

		std::cout << m_name << " is destroyed! " << std::endl;
		//remove from tile
		getTile()->getComponent<TileInfo>()->removeUnit();
		//remove from intiative tracker
		InitiativeTracker::getInstance()->removeUnit(m_attachedObject);

		// Commander Death Victory/Defeat Condition
		if (isCommander())
		{
			if (networking::ClientGame::isNetworkValid())
			{
				// Pass in false as the parameter in order to signal to the other client of disconnect
				networking::ClientGame* client = networking::ClientGame::getInstance();
				client->removeUnitGameObject(client->getUnitGameObjectIndex(m_attachedObject));

				kitten::Event* eventData = new kitten::Event(kitten::Event::Tile_Clicked);
				if (m_clientId == client->getClientId())
				{
					printf("Defeat; your Commander has been slain.");
					eventData->putInt(PLAYER_COMMANDER_DEATH, FALSE);
				}
				else
				{
					printf("Victory, the enemy Commander has been slain!");
					eventData->putInt(PLAYER_COMMANDER_DEATH, TRUE);
				}
				// Rather than straight disconnect, we should display a screen that says Victory or Defeat
				// and a button that takes you back to the main menu
				//client->disconnectFromNetwork(true);
				
				kitten::EventManager::getInstance()->triggerEvent(kitten::Event::End_Game_Screen, eventData);
			}
			// Or maybe straight disconnect but disable unit control and display Victory/Defeat screen here
			// This would allow for Victory/Defeat screen offline as well, but would need a way to determine whose
			// Commander was defeated when offline
			
		}
	}
}
