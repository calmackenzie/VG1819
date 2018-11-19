#include "StringInputDisplay.h"
#include "kitten\K_GameObject.h"

StringInputDisplay::StringInputDisplay() : m_textBox(nullptr)
{

}

StringInputDisplay::~StringInputDisplay()
{
	input::InputManager::getInstance()->removeStringListener(this);
}

void StringInputDisplay::start()
{
	m_textBox = m_attachedObject->getComponent<puppy::TextBox>();
	
	assert(m_textBox != nullptr);
	m_textBox->setText("");

	input::InputManager::getInstance()->addStringListener(this);
}

void StringInputDisplay::onStringFinished(const std::string& p_string)
{
	m_textBox->setText("");
}

void StringInputDisplay::onStringChanged(const std::string& p_string)
{
	m_textBox->setText(p_string);
}