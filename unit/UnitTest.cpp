#include "UnitTest.h"
#include "ability/node/AbilityNodeManager.h"
#include "unit/InitiativeTracker/InitiativeTracker.h"
#include <iostream>
#include "kibble/kibble.hpp"
#include "kibble/databank/databank.hpp"

// Includes for tile highlighting and manipulating tile
#include "_Project\UseAbilityWhenClicked.h"
//board clickable
#include "board/clickable/PrintWhenClicked.h"
#include "kitten\K_ComponentManager.h"

//Includes for creating spawn tile
#include "unit/unitComponent/UnitMove.h"
#include "kitten/K_GameObjectManager.h"

#include "board/BoardManager.h"

//Rock
//test the unit data

namespace unit
{
	UnitTest* UnitTest::m_instance = nullptr;

	UnitTest::UnitTest()
	{
	}

	UnitTest * UnitTest::getInstance()
	{
		return m_instance;
	}

	UnitTest * UnitTest::getInstanceSafe()
	{
		if (m_instance == nullptr)
			m_instance = new UnitTest();
		return m_instance;
	}

	void UnitTest::test()
	{
		kibble::UnitDataParser* parser = kibble::getUnitDataParserInstance();

		//comment for testing IT
		
		// Testing highlighting tiles and manipulating tiles using testDummy.txt
		{
			kitten::K_GameObject* testDummyGO = UnitSpawn::getInstance()->spawnUnitObject(kibble::getUnitFromId(0));
			unit::Unit* testDummy = testDummyGO->getComponent<unit::Unit>();
			UnitMonitor::getInstanceSafe()->printUnit(testDummy);

			//initialize position
			testDummyGO->getComponent<unit::UnitMove>()->setTile(4,4);

			kitten::K_ComponentManager* compMan = kitten::K_ComponentManager::getInstance();
			testDummyGO->addComponent(compMan->createComponent("UseAbilityWhenClicked"));
			testDummyGO->addComponent(compMan->createComponent("SelectAbility"));

			//PrintWhenClicked* printWhenClick = static_cast<PrintWhenClicked*>(compMan->createComponent("PrintWhenClicked"));
			//printWhenClick->setMessage("Test Dummy");
			//testDummyGO->addComponent(printWhenClick);
		}
		// End testing selecting spawned unit
		

		/*
		//create test tile for unit spawn
		kitten::K_GameObject* testTile = kitten::K_GameObjectManager::getInstance()->createNewGameObject("tileobj.txt");
		PrintWhenClicked* printWhenClick = static_cast<PrintWhenClicked*>(kitten::K_ComponentManager::getInstance()->createComponent("PrintWhenClicked"));
		printWhenClick->setMessage("spawn tile is clicked");
		testTile->addComponent(printWhenClick);
		kitten::K_Component* clickBox = kitten::K_ComponentManager::getInstance()->createComponent("ClickableBox");
		testTile->addComponent(clickBox);

		testTile->getTransform().place(0.0f, -1.0f, 16.0f);
		//end of test tile
		*/


		//kitten::K_GameObject* u1 = UnitSpawn::getInstance()->spawnUnitObject(parser->getUnit("Priest.txt"));
		kitten::K_GameObject* u1 = UnitSpawn::getInstance()->spawnUnitObject(kibble::getUnitFromId(1));
		//kitten::K_GameObject* u2 = UnitSpawn::getInstance()->spawnUnitObject(parser->getUnit("Engineer.txt"));
		kitten::K_GameObject* u2 = UnitSpawn::getInstance()->spawnUnitObject(kibble::getUnitFromId(4));
		//u1->getTransform().move(10.0f, 0.0f, 0.0f);
		//set initial position
		u1->getComponent<unit::UnitMove>()->setTile(1,1);
		u2->getComponent<unit::UnitMove>()->setTile(0,1);

		//test unit 
		unit::Unit* u = u1->getComponent<unit::Unit>();
		//UnitMonitor::getInstanceSafe()->printUnit(u);

		//Test Initiative Tracker
		//kitten::K_GameObject* u2 = UnitSpawn::getInstance()->spawnUnitObject(parser->getUnit("Engineer.txt"));
		//kitten::K_GameObject* u3 = UnitSpawn::getInstance()->spawnUnitObject(parser->getUnit("Duelist.txt"));

		UnitMonitor::getInstanceSafe()->printIT();
		InitiativeTracker::getInstance()->gameTurnStart();
		//UnitMonitor::getInstanceSafe()->printIT();
		//InitiativeTracker::getInstance()->removeUnit(u2);
		//UnitMonitor::getInstanceSafe()->printIT();
	}
}