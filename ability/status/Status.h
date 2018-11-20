

//Rock

//Status include Buff, Debuff, Passive Ability, and level up bonus
//It has ability nodes as Ability, but the effects are triggered by some event or time point

//Event can be a enum, such as Level up, Turn Start, Tile Change....
//Status should register some of the event to something high level
//when any of the event occurs, high level class should notify this
//and this should do something based on which event occur.

//EXAMPLE: Demonic Restriction will handle Tile Change event
//if it's not Demonic Presence Tile, its effect will activate
//otherwise deactivate

//EXAMPLE: a "Until next turn" Status will handle Turn Start event
//it will decrease duration by 1 and see if it's zero
//then decide to remove this effect

#pragma once
#include "ability/node/AbilityNodeManager.h"
#include "ability/status/statusEvent/TimePointEvent.h"

#include <string>
#include <vector>

#define INFO_PACKAGE_KEY "info"

#define STATUS_ENCOURAGE "Status_Encourage"
#define STATUS_LV "Status_LV"
#define STATUS_PRIEST_LV3 "Status_Priest_LV3"
#define STATUS_ARCHER_LV3 "Status_Archer_LV3"
#define STATUS_DUELIST_LV3 "Status_Duelist_LV3"
#define STATUS_DODGE "Status_Dodge"
#define STATUS_TEMP_CHANGE "Status_Temp_Change"
#define STATUS_AD_CHANGE "Status_AD_Change"
#define STATUS_LOAD "Status_Load"
#define STATUS_SHIELD "Status_Shield"

namespace ability
{
	class Status
	{
	public:
		std::string m_name;

		Status();
		virtual ~Status();

		void changeLV(int p_lv);
		void changeDescription(const std::string & p_msg);
		void setEffectedAD(const std::string & p_msg);
		void addCounter(const std::string & p_key, int p_value);
		void addAttributeChange(const std::string & p_key, int p_value);
		void addTimePoint(ability::TimePointEvent::TPEventType p_value);
		std::vector<ability::TimePointEvent::TPEventType> getTPlist();

		void attach(unit::Unit* p_u);

		virtual Status* clone() const = 0;

		virtual int effect();//activate when attached to unit
		virtual int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);

		void registerTPEvent();

		//for test
		void print();

	protected:
		unit::Unit * m_unit;//the unit this status attached to

		std::unordered_map<std::string, int> m_counter;
		//Most commonly counter is duration. But it can be more, such as how many times it can be used

		std::string m_description;//the text that will be showed to player
		int m_LV;
		std::unordered_map<std::string, int> m_attributeChange;
		std::string m_effectedAD;

		std::vector<ability::TimePointEvent::TPEventType> m_TPList;//the list of event that will be registered

		void removeThis();
		int changeCounter(const std::string& p_cName = "duration", int p_value = -1);
		void checkDuration();
		virtual void effectEnd();
	};

	class Status_LV : public Status
	{
		//this class handle the attribute change for all lv up status
	public:
		Status_LV();
		virtual Status* clone() const { return new Status_LV(*this); };
		virtual int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	};

	class Status_Encourage : public Status
	{
		//this is buff apply by ability [Encourage]
		//it triggers when unit will deal damage
		//then it increase the ability's power by this status power
	public:
		Status_Encourage();
		Status* clone() const { return new Status_Encourage(*this); };
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	};

	class Status_Dodge : public Status
	{
		//this is buff apply by ability [Dodge]
		//it triggers when unit will receive damage
	public:
		Status_Dodge();
		Status* clone() const { return new Status_Dodge(*this); };
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	};

	class Status_Priest_LV3 : public Status_LV
	{
		//this is trigger when preiest is lv3
	private:
		bool m_activate = false;
	public:
		Status_Priest_LV3();
		Status* clone() const { return new Status_Priest_LV3(*this); };
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	}; 

	class Status_Archer_LV3 : public Status_LV
	{
		//this is trigger when Archer is lv3
	public:
		Status_Archer_LV3();
		Status* clone() const { return new Status_Archer_LV3(*this); };

		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	};

	class Status_Duelist_LV3 : public Status_LV
	{
		//this is trigger when Duelist is lv3
	public:
		Status_Duelist_LV3();
		Status* clone() const { return new Status_Duelist_LV3(*this); };

		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	};

	class Status_Temp_Change : public Status
	{
	public:
		Status_Temp_Change();
		Status* clone() const { return new Status_Temp_Change(*this); };
		int effect();
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
		void effectEnd();
	};

	class Status_AD_Change : public Status
	{
	public:
		Status_AD_Change();
		Status* clone() const { return new Status_AD_Change(*this); };
		int effect();
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
		void effectEnd();
	};

	class Status_Load : public Status
	{
	public:
		Status_Load();
		Status* clone() const { return new Status_Load(*this); };
		int effect();
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
		void effectEnd();
	};

	class Status_Shield : public Status
	{
	public:
		Status_Shield();
		Status* clone() const { return new Status_Shield(*this); };
		int effect(ability::TimePointEvent::TPEventType p_type, ability::TimePointEvent* p_event);
	};
}



