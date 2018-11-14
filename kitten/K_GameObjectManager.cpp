#include "K_GameObjectManager.h"
#include "kibble/kibble.hpp"

namespace kitten
{
	K_GameObjectManager* K_GameObjectManager::sm_instance = nullptr;

	K_GameObjectManager::K_GameObjectManager()
	{

	}

	K_GameObjectManager::~K_GameObjectManager()
	{

	}

	K_GameObject* K_GameObjectManager::createNewGameObject()
	{
		K_GameObject* toReturn = new K_GameObject();

		toReturn->m_objectIndex = m_createdObjects;
		m_gameObjects.insert(std::make_pair(m_createdObjects,toReturn));

		m_createdObjects++;

		return toReturn;
	}

	K_GameObject* K_GameObjectManager::createNewGameObject(const std::string& filename)
	{
		kibble::GameObjectDataParser* parser = kibble::getGameObjectDataParserInstance();
		return parser->getGameObject(filename);
	}

	void K_GameObjectManager::destroyGameObject(K_GameObject* p_toDestroy)
	{
		assert(p_toDestroy != nullptr);

		auto found = m_gameObjects.find(p_toDestroy->m_objectIndex);
		assert(found != m_gameObjects.end());
		m_gameObjects.erase(found);

		m_toDelete.push_back(p_toDestroy);
	}

	void K_GameObjectManager::deleteQueuedObjects()
	{
		for (auto it = m_toDelete.begin(); it != m_toDelete.end(); ++it)
		{
			delete *it;
		}

		m_toDelete.clear();
	}

	void K_GameObjectManager::destroyAllGameObjects()
	{
		deleteQueuedObjects();

		auto end = m_gameObjects.end();
		for (auto it = m_gameObjects.begin(); it != end; ++it)
		{
			delete  (*it).second;
		}
		m_gameObjects.clear();
	}
}