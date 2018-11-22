#pragma once
#include <string.h>

// For ResizablePacket
#include <vector>
#include <unordered_map>
#include "kitten\K_GameObject.h"
#include "unit\Unit.h"

#define MAX_PACKET_SIZE 1000000
#define MAX_CHAR_BUFSIZE 512

#define BASIC_PACKET_SIZE sizeof(Packet)
#define SUMMON_UNIT_PACKET_SIZE sizeof(SummonUnitPacket)
#define TEST_PACKET_SIZE sizeof(TestPacket)

enum PacketTypes {

	INIT_CONNECTION = 0,
	SERVER_SHUTDOWN,
	CLIENT_DISCONNECT,
	SEND_CLIENT_ID,
	ABILITY_PACKET,
	SUMMON_UNIT,
	SKIP_TURN,
};

class Buffer
{
	friend class AbilityPacket;
	friend struct Packet;
public:
	char* m_data;		// pointer to buffer data
	int m_size;			// size of buffer data in bytes
	int getIndex() { return m_index; }
private:
	int m_index = 0;	// index of next byte to be read/written
};

// Use to get packet type first, then deserialize into appropriate packet
struct Packet {

	unsigned int m_packetType;
	int m_clientId;

	void writeInt(Buffer& buffer, int value)
	{
		assert(buffer.m_index + sizeof(int) <= buffer.m_size);

		*((int*)(buffer.m_data + buffer.m_index)) = value;
		buffer.m_index += sizeof(int);
	}

	void writeChar(Buffer &buffer, char value)
	{
		assert(buffer.m_index + sizeof(char) <= buffer.m_size);

		*((char*)(buffer.m_data + buffer.m_index)) = value;
		buffer.m_index += sizeof(char);
	}

	int readInt(Buffer& buffer)
	{
		//assert(buffer.m_index + sizeof(int) <= buffer.m_size);

		int value = *((int*)(buffer.m_data + buffer.m_index));
		buffer.m_index += sizeof(int);

		return value;
	}

	char readChar(Buffer &buffer)
	{
		//assert(buffer.m_index + sizeof(char) <= buffer.m_size);

		char value = *((char*)(buffer.m_data + buffer.m_index));
		buffer.m_index += sizeof(char);

		return value;
	}

	void serialize(Buffer& buffer) {
		writeInt(buffer, m_packetType);
		writeInt(buffer, m_clientId);
	}

	void deserialize(Buffer& buffer) {
		m_packetType = readInt(buffer);
		m_clientId = readInt(buffer);
	}
};

struct SummonUnitPacket : Packet
{
	int unitId;
	int posX, posY;

	void serialize(Buffer& buffer) 
	{
		writeInt(buffer, m_packetType);
		writeInt(buffer, m_clientId);
		writeInt(buffer, unitId);
		writeInt(buffer, posX);
		writeInt(buffer, posY);
	}

	void deserialize(Buffer& buffer)
	{
		m_packetType = readInt(buffer);
		m_clientId = readInt(buffer);
		unitId = readInt(buffer);
		posX = readInt(buffer);
		posY = readInt(buffer);
	}
};

class AbilityPacket
{
	typedef std::vector<unit::Unit*> TargetUnits;
	typedef std::unordered_map<std::string, int> IntValues;
	typedef std::vector<kitten::K_GameObject*>  TargetTiles;
public:
	int packetType;
	int clientId;
	int sourceUnit;

	int m_abilityNameLength;
	//char m_abilityName[MAX_CHAR_BUFSIZE];
	std::string m_abilityName = "";

	void print();

	void serialize(Buffer& buffer);
	void deserialize(Buffer& buffer);

	void addTargetUnits(TargetUnits p_targets);
	void addIntValues(IntValues p_values);
	void addTargetTiles(TargetTiles p_targetTilesGO);

	TargetUnits getTargetUnits() { return m_targets; }
	IntValues getIntValues() { return m_intValue; }
	TargetTiles getTargetTiles() { return m_targetTilesGO; }
	int getSize();
	int getBytes() { return totalBytes; }

private:	
	int sumKeysLength = 0;
	int totalBytes = 0;

	int m_numTargetUnits;
	TargetUnits m_targets;

	int m_numIntValues;
	IntValues m_intValue;

	int m_numTargetTiles;
	TargetTiles m_targetTilesGO;

	void writeInt(Buffer &buffer, int value);
	void writeChar(Buffer &buffer, char value);
	int readInt(Buffer &buffer);
	char readChar(Buffer &buffer);
	
};

struct TestPacket : Packet {

	int num;
	char msg[MAX_CHAR_BUFSIZE];

	void serialize(char* data) {
		int *q = (int*)data;
		*q = this->m_packetType;   q++;
		*q = this->m_clientId;	q++;
		*q = this->num;			q++;

		char *p = (char*)q;
		int i = 0;
		for (int i = 0; i < MAX_CHAR_BUFSIZE; i++) {
			*p = this->msg[i];
			p++;
		}
	}

	void deserialize(char* data) {
		int *q = (int*)data;
		this->m_packetType = *q;		q++;
		this->m_clientId = *q;		q++;
		this->num = *q;				q++;

		char *p = (char*)q;
		int i = 0;
		for (int i = 0; i < MAX_CHAR_BUFSIZE; i++) {
			this->msg[i] = *p;
			p++;
		}
	}
};
