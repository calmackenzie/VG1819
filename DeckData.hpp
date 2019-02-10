#pragma once
#include <vector>
#include <string>

class DeckData {
public: 
	std::string name, filename;
	std::vector<std::pair<int, int>> cards; // the index of card and repetition
	int totalCardCount = 0, commanderID = -1;
};