
#include "controller.h"
#include "unit/InitiativeTracker/InitiativeTracker.h"
#include "unit/Unit.h"
#include "kibble/databank/databank.hpp"
#include "Extract/Action.h"
#include <set>
#include <queue>
#include "ability/AbilityMacro.h"
#include "unit/UnitSpawn.h"
#include "kitten/K_GameObject.h"
#include "unit/unitComponent/UnitMove.h"
#include "_Project/DisableAfterTime.h"
#include "networking/ClientGame.h"
#include "AI/Extract/Behavior.h"


template <typename Iterator>
inline bool next_combination(const Iterator first, Iterator k, const Iterator last)
{
	/* Credits: Thomas Draper */
	// http://stackoverflow.com/a/5097100/8747
	if ((first == last) || (first == k) || (last == k))
		return false;
	Iterator itr1 = first;
	Iterator itr2 = last;
	++itr1;
	if (last == itr1)
		return false;
	itr1 = last;
	--itr1;
	itr1 = k;
	--itr2;
	while (first != itr1)
	{
		if (*--itr1 < *itr2)
		{
			Iterator j = k;
			while (!(*itr1 < *j)) ++j;
			std::iter_swap(itr1, j);
			++itr1;
			++j;
			itr2 = k;
			std::rotate(itr1, j, last);
			while (last != j)
			{
				++j;
				++itr2;
			}
			std::rotate(k, itr2, last);
			return true;
		}
	}
	std::rotate(first, k, last);
	return false;
}

namespace AI {
	std::map<int,controller*> AIcontrollers;
	std::vector<controller*> AIcontrollerlist;
	NearestEnemy defaultBehavior;

	controller::controller() { // TODO make an ID dispensor, i hate this already. 
		setupEventListeners();
		AIcontrollerlist.push_back(this);
	}

	controller::~controller() {
		tearDownEventListeners();
		AIcontrollerlist.erase(std::find(AIcontrollerlist.begin(), AIcontrollerlist.end(), this));
		AIcontrollers.erase(this->m_playerID);

	}

	void controller::runTurn(unit::Unit* p_unit)
	{
		// Disable the previous timer
		this->m_attachedObject->setEnabled(false);

		// Generated Sequences will be stored here
		std::vector<Extract::Sequence> sequences;
		
		// Check if it's a commander poiting out this controller's start of a new round
		if (p_unit->isCommander()) {
			// Reset PowerTracker
			m_model.powerTracker.resetCurrent();

			// Draw Cards
			for (int i = 0; i < 2 && m_model.hand.canAddCard(); ++i) {
				Extract::Deck::InteractionState draw = m_model.deck.drawTop();
				if (draw.status != AI::Extract::Deck::Status::OK) break;
				unit::Unit* unit = new unit::Unit(kibble::getUnitFromId(draw.card)); // IMPORTANT!!!!!!!!! REMEMBER TO DELETE WHEN YOU TAKE IT OUT
				unit->m_clientId = this->m_playerID;
				m_model.hand.addCard(unit);
			}
		}

		// Setup info
		availableInfo info;
		info.sourceUnit = p_unit;
		info.summonableUnits = m_model.getSummonableCards();
		info.curPos = p_unit->getTile()->getComponent<TileInfo>()->getPos();
		info.availableEnergy = m_model.powerTracker.m_iCurrentPower;

		// Run generator of possible sequences
		generateSequences(Extract::Sequence(), sequences, info);

		// Sort Sequences and pick one
		// TODO bother looking for one based on externally set choices instead of the best.
		std::sort(sequences.begin(), sequences.end(), Extract::Sequence::weightComp());
		m_unit = p_unit;
		if (sequences.size() > 0) {
			m_sequence = sequences[0];
		}
		else
		{
			m_sequence = Extract::Sequence();
		}
		kitten::EventManager::getInstance()->queueEvent(kitten::Event::Action_Complete, new kitten::Event(kitten::Event::Action_Complete));
	}

	void controller::start() {
	}

	void controller::setupAIControllers()
	{
		// Alter the client Id to reflect non AI, player controlled opponent. 
		// They'll always set Id to 0 when theres an AI ;
		int startingId = networking::ClientGame::getClientId();

		for (controller* controller: AIcontrollerlist) {
			controller->m_playerID = ++startingId;
			controller->m_model.playerId = controller->m_playerID;
			AIcontrollers[controller->m_playerID] = controller;

			// TODO work on how decks are picked, for now MUH OH SEE
			// Set up deck the AI will use
			controller->m_model.deck.setDeckSource(kibble::getDeckDataFromId(0));
			controller->m_model.deck.setupDeck();

			// Set up hand
			while (controller->m_model.hand.canAddCard()) {
				Extract::Deck::InteractionState draw = controller->m_model.deck.drawTop();
				if (draw.status != AI::Extract::Deck::Status::OK) break;
				unit::Unit* unit = new unit::Unit(kibble::getUnitFromId(draw.card));
				unit->m_clientId = controller->m_playerID;
				controller->m_model.hand.addCard(unit);
			}

			// Spawn Commander
			kitten::K_GameObject* unitGO = unit::UnitSpawn::getInstance()->spawnUnitObject(controller->m_model.deck.m_deckSource->commanderID);
			unitGO->getComponent<unit::UnitMove>()->setTile(BoardManager::getInstance()->getSpawnPoint(controller->m_playerID ));
			unitGO->getComponent<unit::Unit>()->m_clientId = controller->m_playerID;

			// Set up board reference
			controller->m_model.board.setupBoard();

		}
	}

	void controller::generateSequences(Extract::Sequence p_currentSeq,std::vector<Extract::Sequence>& p_sequences, availableInfo p_info)
	{
		if (p_info.canMove) {

			// give the movement check a max value of  1
			// TODO take into account the various fields he's currently on.
			Model::TargetRange targetRange;
			targetRange.currentPlacement = p_info.curPos;
			targetRange.unit = p_info.sourceUnit;
			targetRange.blockedPos = p_info.blockedPos;
			for (auto move : m_model.getAvailableMoves(targetRange)) {
				Extract::Sequence moveSeq(p_currentSeq);
				targetRange.targetPlacement = std::make_pair(move.targetX,move.targetY);
				if (targetRange.unit->m_AbilityBehavior.find(UNIT_MV) != targetRange.unit->m_AbilityBehavior.end()) {
					moveSeq.weight += targetRange.unit->m_AbilityBehavior[UNIT_MV]->calculateWeight(targetRange, m_model);
				}
				else {
					moveSeq.weight += defaultBehavior.calculateWeight(targetRange, m_model);
				}
				moveSeq.actions.push_back(new Extract::Move(move));
				p_sequences.push_back(moveSeq);
				availableInfo info(p_info);
				info.curPos = std::make_pair(move.targetX,move.targetY);
				info.canMove = false;
				generateSequences(moveSeq, p_sequences, info);
			}
		}

		if (p_info.canAct) {
			// For unit AD
			for (auto ability : p_info.sourceUnit->m_ADList) {
				// check if our unit can even use it
				if (ability->m_intValue[UNIT_LV] > p_info.sourceUnit->m_attributes[UNIT_LV]
					|| ability->m_intValue[UNIT_NEED_UNIT] < 1 // TODO REMOVE THIS AFTER TAKING the need for tiles INTO ACCOUNT
					// TODO CHECK IF THE ABILITY IS ON COOLDOWN
					|| (ability->m_intValue.find(COUNTER_POWER) != ability->m_intValue.end() && ability->m_intValue[ability->m_stringValue[COUNTER_NAME]] <ability->m_intValue[COUNTER_POWER]) // make sure we have enough power
					) continue;
				std::string abilityName = ability->m_stringValue[ABILITY_NAME];

				// give target max value of 1
				// give the damage a value boost of difference it does to enemy, which can be a lot. 

				// Give this weight based on closeness to lowest health targets. 

				// this is just for me to understand how to check for each different type of ability. 
				// this will be stripped down to essentials based on what ability information we get. 
				// TODO add general unit preferences, and ability preferences that override the general. these should affect the target and move

				Model::TargetRange targetRange;
				targetRange.min_range = ability->m_intValue[MIN_RANGE];
				targetRange.max_range = ability->m_intValue[MAX_RANGE];
				targetRange.currentPlacement = p_info.curPos;
				targetRange.unit = p_info.sourceUnit;
				targetRange.blockedPos = p_info.blockedPos;
				targetRange.select_repeat = (ability->m_intValue.find(UNIT_SELECT_REPEAT) != ability->m_intValue.end()) ? ability->m_intValue[UNIT_SELECT_REPEAT]: false; // default is false
				std::vector<std::pair<int,int>> possibleTargets;

				// TODO this should only be done when need_unit is set, otherwise, it should be looking for specific tiles
				auto targets = m_model.getTargetsInRange(targetRange);
				// The following for normal attacks that have a set number of targets
				int targetsToHit = ability->m_intValue[UNIT_TARGETS];

				targetRange.hasPower = ability->m_intValue.find(UNIT_POWER) != ability->m_intValue.end();
				targetRange.addsCounter = ability->m_intValue.find(COUNTER_CHANGE) != ability->m_intValue.end();
				targetRange.needsCounter = ability->m_intValue.find(COUNTER_POWER) != ability->m_intValue.end(); // If there's no special case, remove this

				if (targetRange.hasPower) targetRange.abilityPower = ability->m_intValue[UNIT_POWER];
				if (targetRange.addsCounter) {
					targetRange.counterChange = ability->m_intValue[COUNTER_CHANGE];
					targetRange.counterMax = ability->m_intValue[COUNTER_MAX];
					targetRange.counterName = ability->m_stringValue[COUNTER_NAME];
				}

				// Now go through the list of target units available and pickout the ones we can target.
				for (auto target : targets) {
					if (target->m_attributes[UNIT_HP] <= 0 // if no HP left
							|| (target->m_clientId == targetRange.unit->m_clientId && !targetRange.allyHit) // if it can't hit allies
						)
						continue;

					int numberOfRepetitionsPossibleOnATarget = 1;
					if (targetRange.select_repeat) // if the targets can be repeatedly selected
						if (targetRange.hasPower) // If it's based on power
							numberOfRepetitionsPossibleOnATarget = MAX(MIN(targetsToHit, std::ceil((double)target->m_attributes[UNIT_HP] / targetRange.abilityPower)), 1);

					possibleTargets.resize(possibleTargets.size() + numberOfRepetitionsPossibleOnATarget, target->getTile()->getComponent<TileInfo>()->getPos());
				}

				if (possibleTargets.size() == 0
					|| (!targetRange.select_repeat && possibleTargets.size() < targetsToHit)
					) continue;

				// If the number of possible targets end up being less than the target needed, repeat the last one over again. 
				// Due to the earlier repeat check, this will always get triggered only if the target can be repeatedly targetted.
				if (possibleTargets.size() < targetsToHit)
				{
					possibleTargets.resize(targetsToHit, possibleTargets.back());
				}

				std::sort(possibleTargets.begin(), possibleTargets.end());
				do {
					Extract::Sequence abilitySeq(p_currentSeq);
					abilitySeq.actions.push_back(new Extract::MultiTargetAbility(std::vector<std::pair<int, int>>(possibleTargets.begin(), possibleTargets.begin()+ targetsToHit), abilityName));
					if (targetRange.unit->m_AbilityBehavior.find(abilityName) != targetRange.unit->m_AbilityBehavior.end()) {
						abilitySeq.weight += targetRange.unit->m_AbilityBehavior[abilityName]->calculateWeight(targetRange, m_model);
					}
					else {
						abilitySeq.weight += defaultBehavior.calculateWeight(targetRange, m_model);
					}
					p_sequences.push_back(abilitySeq);
					availableInfo info(p_info);
					info.canAct = false;
					generateSequences(abilitySeq, p_sequences, info);

				} while (next_combination(possibleTargets.begin(), possibleTargets.begin() + targetsToHit, possibleTargets.end()));


			}

			if (p_info.sourceUnit->isCommander()) {
				// For Tile Manip
				{
					Model::TargetRange targetRange;
					targetRange.min_range = 1;
					targetRange.max_range = 1;
					targetRange.currentPlacement = p_info.curPos;
					targetRange.unit = p_info.sourceUnit;
					targetRange.checkTargetFailIfOwnedByAny = true;

					for (auto tile : m_model.getTargetTilesInRange(targetRange)) {
						if (tile->getOwnerId() != m_playerID) {
							Extract::Sequence manip(p_currentSeq);
							targetRange.targetPlacement = tile->getPos();
							if (targetRange.unit->m_AbilityBehavior.find(ABILITY_MANIPULATE_TILE) != targetRange.unit->m_AbilityBehavior.end()) {
								manip.weight += targetRange.unit->m_AbilityBehavior[ABILITY_MANIPULATE_TILE]->calculateWeight(targetRange, m_model);
							}
							else {
								manip.weight += defaultBehavior.calculateWeight(targetRange, m_model);
							}
							manip.actions.push_back(new Extract::ManipulateTile(tile->getPosX(), tile->getPosY()));
							p_sequences.push_back(manip);

							availableInfo info(p_info);
							info.canAct = false;
							generateSequences(manip, p_sequences, info);
						}
					}
				}

				// For summoning, Currently limited for 2, so that it doesn't go too deep. 
				if (p_info.summoned < 2) {
					for (int i = 0; i < p_info.summonableUnits.size();++i) {
						if (p_info.availableEnergy < m_model.hand.m_cards[p_info.summonableUnits[i]]->m_attributes[UNIT_COST])
							continue;

						Model::TargetRange targetRange;
						targetRange.min_range = 1;
						targetRange.max_range = 1;
						targetRange.currentPlacement = p_info.curPos;
						targetRange.unit = p_info.sourceUnit;
						targetRange.blockedPos = p_info.blockedPos;
						targetRange.checkTargetFailIfNotOwnedBySelf = true;
						targetRange.checkTargetFailIfBlocked = true;

						for (auto target : m_model.getTargetTilesInRange(targetRange)) {
							Extract::Sequence summoning(p_currentSeq);
							targetRange.targetPlacement = target->getPos();
							targetRange.targetUnit = m_model.hand.m_cards[p_info.summonableUnits[i]];
							if (targetRange.unit->m_AbilityBehavior.find(ABILITY_SUMMON_UNIT) != targetRange.unit->m_AbilityBehavior.end()) {
								summoning.weight += targetRange.unit->m_AbilityBehavior[ABILITY_SUMMON_UNIT]->calculateWeight(targetRange, m_model);
							}
							else {
								summoning.weight += defaultBehavior.calculateWeight(targetRange, m_model);
							}
							int handOffset = 0;
							for (auto handCardPicked : p_info.handCardsPicked)
								if (handCardPicked < p_info.summonableUnits[i])
									++handOffset;
							summoning.actions.push_back(new Extract::Summon(target->getPosX(), target->getPosY(), p_info.summonableUnits[i],handOffset));
							p_sequences.push_back(summoning);
							availableInfo info(p_info);
							info.blockedPos.push_back(target->getPos());
							info.summonableUnits.erase(info.summonableUnits.begin() + i);
							info.availableEnergy -= m_model.hand.m_cards[p_info.summonableUnits[i]]->m_attributes[UNIT_COST];
							++info.summoned;
							info.handCardsPicked.push_back(p_info.summonableUnits[i]);
							generateSequences(summoning, p_sequences, info);
						}
					}
				}
			}
		}
	}

	void controller::nextActionInSequenceHandler(kitten::Event::EventType p_type, kitten::Event * p_data)
	{
		unit::Unit* currentUnit = unit::InitiativeTracker::getInstance()->getCurrentUnit()->getComponent<unit::Unit>();
		if (currentUnit->m_clientId != m_playerID || !m_unit->isTurn()) return;
		this->m_attachedObject->getComponent<DisableAfterTime>()->setTime(1);
		if (!m_attachedObject->isEnabled())
			this->m_attachedObject->setEnabled(true);
		else
			this->m_attachedObject->setEnabled(false);
	}

	void controller::nextTurnHandler(kitten::Event::EventType p_type, kitten::Event* p_data) {
		unit::Unit* currentUnit = unit::InitiativeTracker::getInstance()->getCurrentUnit()->getComponent<unit::Unit>();
		if (currentUnit->m_clientId == m_playerID) {
			runTurn(currentUnit);
		}
	}

	controller * controller::getAIController(int p_playerId)
	{
		return AIcontrollers[p_playerId];
	}

	int controller::getAIControllerSize()
	{
		return AIcontrollerlist.size();
	}

	Model * controller::getAIModel(int p_playerId)
	{
		return &AIcontrollers[p_playerId]->m_model;
	}

	bool controller::AIPresent()
	{
		return AIcontrollerlist.size()> 0;
	}

	void controller::onDisabled()
	{
		if (m_unit != nullptr && m_unit == unit::InitiativeTracker::getInstance()->getCurrentUnit()->getComponent<unit::Unit>()) {
			m_sequence.step(m_unit);
		}
	}

	void controller::setupEventListeners() {
		kitten::EventManager::getInstance()->addListener(
			kitten::Event::EventType::Next_Units_Turn_Start,
			this,
			std::bind(&controller::nextTurnHandler, this, std::placeholders::_1, std::placeholders::_2));
		kitten::EventManager::getInstance()->addListener(
			kitten::Event::EventType::Action_Complete,
			this,
			std::bind(&controller::nextActionInSequenceHandler, this, std::placeholders::_1, std::placeholders::_2));
	}

	void controller::tearDownEventListeners() {

		kitten::EventManager::getInstance()->removeListener(kitten::Event::EventType::Next_Units_Turn_Start, this);
		kitten::EventManager::getInstance()->removeListener(kitten::Event::EventType::Action_Complete, this);
	}
}