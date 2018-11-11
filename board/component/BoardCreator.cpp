// BoardCreator
//
// Originally only created the tile objects that make up the board @Callum
// Multipurposed to handle highlighting and unhighlighting of tiles @Ken
// *Should rename component to something that better represents this.
// TODO: Either verify tiles to be highlighted are valid here or ensure it is
//	done before the tile list is sent here. E.g. A commander cannot highlight
//	tiles owned by the other commander

#include "BoardCreator.h"
#include "kitten\K_GameObjectManager.h"
#include "kitten\K_ComponentManager.h"

#include "kitten\QuadRenderable.h"
#include "_Project/UseAbilityWhenClicked.h"
//clickable
#include "board/clickable/ManipulateTileOnClick.h"
#include "board/clickable/PrintWhenClicked.h"
#include "board/clickable/SendSelfOnClick.h"
//tile
#include "board/tile/TileInfo.h"

#include "board/BoardManager.h"


BoardCreator::BoardCreator() :m_x(15), m_z(15)
{

}

BoardCreator::~BoardCreator()
{

}

void BoardCreator::start()
{
	//create tile
	std::vector<kitten::K_GameObject*> list;

	for (int x = 0; x < m_x; x++)
	{
		for (int z = 0; z < m_z; z++)
		{
			kitten::K_GameObject* tileGO = createTile(x, z);
			list.push_back(tileGO);

			kitten::Transform& transform = tileGO->getTransform();
			transform.setParent(&m_attachedObject->getTransform());
			transform.setIgnoreParent(true);
		}
	}
	//pass tile list and dimension to board manager
	BoardManager::getInstance()->setTileList(&list);
	BoardManager::getInstance()->setDimension(m_x,m_z);

	// PowerTracker component attached to Board GO
	kitten::K_Component* powerTracker = kitten::K_ComponentManager::getInstance()->createComponent("PowerTracker");
	m_attachedObject->addComponent(powerTracker);
	BoardManager::getInstance()->setPowerTracker(static_cast<PowerTracker*>(powerTracker));

	//delete this
	kitten::K_ComponentManager::getInstance()->destroyComponent(this);
}

void BoardCreator::setDimension(int x, int z)
{
	m_x = x;
	m_z = z;
}


kitten::K_GameObject * BoardCreator::createTile(int x, int z)
{
	kitten::K_GameObjectManager* gameObjMan = kitten::K_GameObjectManager::getInstance();
	kitten::K_ComponentManager* compMan = kitten::K_ComponentManager::getInstance();

	kitten::K_GameObject* tileGO = gameObjMan->createNewGameObject("tileobj.txt");

	PrintWhenClicked* printWhenClick = static_cast<PrintWhenClicked*>(compMan->createComponent("PrintWhenClicked"));
	printWhenClick->setMessage("grassy tile: " + std::to_string(x) + ", " + std::to_string(z));
	tileGO->addComponent(printWhenClick);

	ManipulateTileOnClick* manipTileOnClick = static_cast<ManipulateTileOnClick*>(compMan->createComponent("ManipulateTileOnClick"));
	tileGO->addComponent(manipTileOnClick);

	SendSelfOnClick* sendSelfOnClick = static_cast<SendSelfOnClick*>(compMan->createComponent("SendSelfOnClick"));
	tileGO->addComponent(sendSelfOnClick);

	TileInfo* tileInfo = static_cast<TileInfo*>(compMan->createComponent("TileInfo"));
	tileInfo->setPos(x, z);//set position
	tileInfo->setType(LandInformation::Grassland);
	//for test
	{
		if (x == 5 && z == 4)
		{
			tileInfo->setType(LandInformation::Swampland);
		}
	}
	tileGO->addComponent(tileInfo);

	kitten::K_Component* clickBox = compMan->createComponent("ClickableBox");
	tileGO->addComponent(clickBox);

	kitten::Transform& transform = tileGO->getTransform();
	transform.move(x, -1, z);
	return tileGO;
}
