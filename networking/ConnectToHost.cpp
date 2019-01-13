// ConnectToHost
// 
// Simplified version of NetworkingConsoleMenu, used with the IP
// entering screen, with connecting to the host as the only function
//
// Note: This was created to isolate a bug with clicking instead of pressing
// enter to confirm an IP address, but still using this as the IP screen doesn't 
// need everything that NetworkConsoleMenu provides
//
// @Ken

#include "networking\ConnectToHost.h"
#include "kitten\K_Instance.h"
#include "networking\ClientGame.h"

ConnectToHost::ConnectToHost()
	:
	m_bEnteringAddress(false),
	m_bConnected(false),
	m_loadingMessage(nullptr)
{
	
}

ConnectToHost::~ConnectToHost()
{
	kitten::EventManager::getInstance()->removeListener(kitten::Event::EventType::Join_Button_Clicked, this);
	m_inputMan->setPollMode(true);
}

void ConnectToHost::start()
{
	m_textBox = m_attachedObject->getComponent<puppy::TextBox>();
	assert(m_textBox != nullptr);

	m_stringInputDisplay = m_attachedObject->getComponent<StringInputDisplay>();
	assert(m_stringInputDisplay != nullptr);

	m_inputMan = input::InputManager::getInstance();
	assert(m_inputMan != nullptr);

	// Add Listeners for exiting to Main Menu and disconnecting from network

	kitten::EventManager::getInstance()->addListener(
		kitten::Event::EventType::Join_Button_Clicked,
		this,
		std::bind(&ConnectToHost::joinButtonClickedListener, this, std::placeholders::_1, std::placeholders::_2));

	m_loadingMessage = kitten::K_GameObjectManager::getInstance()->createNewGameObject("UI/loading_message.json");
	m_loadingMessage->setEnabled(false);

	// Disable polling so the player can immediately start typing an address
	m_inputMan->setPollMode(false);

	// Create instance of ClientGame and try to connect to localhost to see if someone is hosting
	connectToLocalhost();
}

void ConnectToHost::connect()
{
	std::string address = m_stringInputDisplay->getString();
	printf("Entered address: %s\n", address.c_str());

	networking::ClientGame* client = networking::ClientGame::getInstance();

	// Check if we've already created an instance of ClientGame
	if (client)
	{
		// Check if we're already connected to a host
		if (!networking::ClientGame::isNetworkValid())
		{
			client->setupNetwork(address);
		}
	} else // If not, get address and create ClientGame instance
	{
		networking::ClientGame::createInstance(address);
	}

	if (networking::ClientGame::isNetworkValid())
	{
		m_textBox->setText("Joined host");
		kitten::K_Instance::changeScene("mainscene.json");
	} else
	{
		m_textBox->setText("Network Error");
		m_inputMan->setPollMode(false);
		m_loadingMessage->setEnabled(false);
	}

	m_bConnected = false;
}

void ConnectToHost::connectToLocalhost()
{
	networking::ClientGame* client = networking::ClientGame::getInstance();

	// Check if we've already created an instance of ClientGame
	if (client)
	{
		// Check if we're already connected to a host
		if (!networking::ClientGame::isNetworkValid())
		{
			client->setupNetwork("localhost");
		}
	} else // If not, get address and create ClientGame instance
	{
		networking::ClientGame::createInstance("localhost");
	}

	if (networking::ClientGame::isNetworkValid())
	{
		// Someone is hosting locally, display that to the user
	}
	else
	{
		// No one is hosting locally, display that to the user
	}
}

void ConnectToHost::update()
{
	if (m_bConnected)
	{
		connect();
	}

	if ((m_inputMan->keyDown(GLFW_KEY_ENTER) && !m_inputMan->keyDownLast(GLFW_KEY_ENTER)))
	{
		m_loadingMessage->setEnabled(true);
		//m_bConnected = true;
		networking::ClientGame::getInstance()->sendBasicPacket(JOIN_GAME);
		kitten::K_Instance::changeScene("mainscene.json");
	}	
}

void ConnectToHost::joinButtonClickedListener(kitten::Event::EventType p_type, kitten::Event* p_event)
{
	connect();
}