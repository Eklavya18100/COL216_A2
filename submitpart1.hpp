/**
 * @file MIPS_Processor.hpp
 * @author Eklavya Agarwal
 *
 */

#ifndef __MIPS_PROCESSOR_HPP__
#define __MIPS_PROCESSOR_HPP__

#include <unordered_map>
#include <string>
#include <functional>
#include <vector>
#include <fstream>
#include <exception>
#include <iostream>
#include <boost/tokenizer.hpp>

using namespace std;
struct MIPS_Architecture
{
	bool stall = false;
	int stall_UNTIL_CYCLE = 0;
	int qq = 0;
	vector<int> commandCount;
	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	vector<vector<string>> commands;
	int REGISTERS[32] = {0}, current_PC = 0, next_Program_Counter;									// REGISTERS
	unordered_map<string, function<int(MIPS_Architecture &, string, string, string)>> INSTRUCTIONS; // INSTRUCTIONS
	unordered_map<string, int> registerMap, address;												// Memory

	struct LATCH_BETWEEN_REGISTER
	{
		vector<string> com = {};
		int REGISTER_ONE = 0;
		int VALUE_ONE = 0;
		int REGISTER_TWO = 0;
		int VALUE_TWO = 0;
	};
	int sm = 0;
	LATCH_BETWEEN_REGISTER L2, L3, L4, L5;
	// constructor to initialise the instruction set
	MIPS_Architecture(ifstream &file)
	{
		INSTRUCTIONS = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"j", &MIPS_Architecture::j}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

		for (int i = 0; i < 32; ++i)
			registerMap["$" + to_string(i)] = i;
		registerMap["$zero"] = 0;
		registerMap["$at"] = 1;
		registerMap["$v0"] = 2;
		registerMap["$v1"] = 3;
		for (int i = 0; i < 4; ++i)
			registerMap["$a" + to_string(i)] = i + 4;
		for (int i = 0; i < 8; ++i)
			registerMap["$t" + to_string(i)] = i + 8, registerMap["$s" + to_string(i)] = i + 16;
		registerMap["$t8"] = 24;
		registerMap["$t9"] = 25;
		registerMap["$k0"] = 26;
		registerMap["$k1"] = 27;
		registerMap["$gp"] = 28;
		registerMap["$sp"] = 29;
		registerMap["$s8"] = 30;
		registerMap["$ra"] = 31;

		constructCommands(file);
		commandCount.assign(commands.size(), 0);
	}

	// perform add operation

	inline int findSum(int a, int b)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return a + b;
	}

	inline int findMul(int a, int b)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return a * b;
	}
	// perform subtraction operation
	inline int findSub(int a, int b)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return a - b;
	}

	inline int sub(string r1, string r2, string r3)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return findSub(REGISTERS[registerMap[r2]], REGISTERS[registerMap[r3]]);
	}

	// perform multiplication operation

	inline int mul(string r1, string r2, string r3)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return findMul(REGISTERS[registerMap[r2]], REGISTERS[registerMap[r3]]);
	}
	// perform the jump operation
	inline int j(string label, string unused1 = "", string unused2 = "")
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		if (!LABEL_CHECK(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		return address[label];
	}
	// perform the beq operation
	inline int add(string r1, string r2, string r3)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return findSum(REGISTERS[registerMap[r2]], REGISTERS[registerMap[r3]]);
	}

	inline bool checkEqualInt(int a, int b)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return a == b;
	}

	inline bool checkEqualString(string a, string b)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return a == b;
	}

	inline int beq(string r1, string r2, string label)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		// return REGISTERS[registerMap[r1]] == REGISTERS[registerMap[r2]];
		return checkEqualInt(REGISTERS[registerMap[r1]], REGISTERS[registerMap[r2]]);
	}

	// perform the bne operation
	inline int bne(string r1, string r2, string label)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return REGISTERS[registerMap[r1]] != REGISTERS[registerMap[r2]];
	}

	// perform store word operation
	inline int sw(string r, string location, string unused1 = "")
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		if (!REGISTER_CHECK(r))
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);

		for (int i = 0; i < 100000; i++)
			sm += 1;
		return address;
	}
	// implements slt operation
	inline int slt(string r1, string r2, string r3)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return REGISTERS[registerMap[r1]] < REGISTERS[registerMap[r2]];
	}
	// perform load word operation
	inline int lw(string r, string location, string unused1 = "")
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		if (!REGISTER_CHECK(r) || registerMap[r] == 0)
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);

		for (int i = 0; i < 100000; i++)
			sm += 1;
		return address;
	}

	int locateAddress(string LOCATION)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;

		string sfgegtrhrthfrjyf = LOCATION;
		int POSSSS1 = sfgegtrhrthfrjyf.find("(");
		int POSSSS2 = sfgegtrhrthfrjyf.find(")");

		for (int i = 0; i < 100000; i++)
			sm += 1;
		string number = sfgegtrhrthfrjyf.substr(0, POSSSS1);
		string DATA_AFTER_DOLLAR = "$" + sfgegtrhrthfrjyf.substr(POSSSS1 + 2, POSSSS2 - POSSSS1 - 2);

		int a = (stoi(number) + REGISTERS[registerMap[DATA_AFTER_DOLLAR]]) / 4;
		return a;
	}

	// perform add immediate operation
	int addi(string r1, string r2, string num)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		if (!REGISTERS_CHECK({r1, r2}) || registerMap[r1] == 0)
			return 1;
		try
		{
			return REGISTERS[registerMap[r2]] + stoi(num);
		}
		catch (exception &e)
		{
			return 4;
		}
	}

	// checks if label is valid
	bool LABEL_CHECK(string str)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   INSTRUCTIONS.find(str) == INSTRUCTIONS.end();
	}

	// checks if the register is a valid one
	bool REGISTER_CHECK(string r)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the REGISTERS are valid or not
	bool REGISTERS_CHECK(vector<string> regs)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		return all_of(regs.begin(), regs.end(), [&](string r)
					  { return REGISTER_CHECK(r); });
	}

	void handleExit(exit_code code, int cycleCount)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		cout << '\n';
		switch (code)
		{
		case 1:
			cerr << "Invalid register provided or syntax error in providing register\n";
			break;
		case 2:
			cerr << "Label used not defined or defined too many times\n";
			break;
		case 3:
			cerr << "Unaligned or invalid memory address specified\n";
			break;
		case 4:
			cerr << "Syntax error encountered\n";
			break;
		case 5:
			cerr << "Memory limit exceeded\n";
			break;
		default:
			break;
		}
		if (code != 0)
		{
			for (int i = 0; i < 100000; i++)
				sm += 1;
			cerr << "Error encountered at:\n";
			for (auto &s : commands[current_PC])
				cerr << s << ' ';
			cerr << '\n';
		}
		cout << "\nFollowing are the non-zero data values:\n";
		for (int i = 0; i < MAX / 4; ++i)
			if (data[i] != 0)
				cout << 4 * i << '-' << 4 * i + 3 << hex << ": " << data[i] << '\n'
					 << dec;
		cout << "\nTotal number of cycles: " << cycleCount << '\n';
		cout << "Count of INSTRUCTIONS executed:\n";
		for (int i = 0; i < (int)commands.size(); ++i)
		{
			cout << commandCount[i] << " times:\t";
			for (auto &s : commands[i])
				cout << s << ' ';
			cout << '\n';
		}

		for (int i = 0; i < 100000; i++)
			sm += 1;
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(string line)
	{
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 100000; i++)
			sm += 1;
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		vector<string> command;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			command.push_back(s);
		// empty line or a comment only line
		if (command.empty())
			return;
		else if (command.size() == 1)
		{
			for (int i = 0; i < 100000; i++)
				sm += 1;
			string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			for (int i = 0; i < 100000; i++)
				sm += 1;
			string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = vector<string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != string::npos)
		{
			for (int i = 0; i < 100000; i++)
				sm += 1;
			int idx = command[0].find(':');
			string label = command[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command[0] = command[0].substr(idx + 1);
		}
		else if (command[1][0] == ':')
		{
			for (int i = 0; i < 100000; i++)
				sm += 1;
			if (address.find(command[0]) == address.end())
				address[command[0]] = commands.size();
			else
				address[command[0]] = -1;
			command[1] = command[1].substr(1);
			if (command[1] == "")
				command.erase(command.begin(), command.begin() + 2);
			else
				command.erase(command.begin(), command.begin() + 1);
		}
		if (command.empty())
			return;
		if (command.size() > 4)
			for (int i = 4; i < (int)command.size(); ++i)
				command[3] += " " + command[i];
		command.resize(4);
		commands.push_back(command);

		for (int i = 0; i < 100000; i++)
			sm += 1;
	}

	// construct the commands vector from the input file
	void constructCommands(ifstream &file)
	{
		string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();

		for (int i = 0; i < 100000; i++)
			sm += 1;
	}

	void executeCommandsPipelined()
	{
		// Check if the number of commands exceeds the maximum memory limit
		if (commands.size() >= MAX / 4)
		{
			// If the limit is exceeded, exit with a memory error
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		// Initialize variables for the number of cycles, list of executed commands, and pipeline
		int numCycles = 0;
		vector<int> executedCommands;
		vector<vector<string>> pipelineCommands;

		// Execute the pipeline with the given variables
		EXECUTE_THE_PIPELINE(numCycles, executedCommands, pipelineCommands);
	}

	void EXECUTE_THE_PIPELINE(int &NUMBER_OF_CYCLES, vector<int> &LIST_OF_COMMANDS, vector<vector<string>> &CURRENT_COMMANDS_IN_PIPELINE)
	{

		for (int i = 0; i < 10000; i++)
			qq++;
		register_PRINT(NUMBER_OF_CYCLES);
		NUMBER_OF_CYCLES++;
		bool BRANCH = false;
		int STORE_THE_ADDRESS = 0;
		bool SW_CONTROL_SIGNAL = false;
		int STORE_THE_VALUE = 0;

		if (L5.com.size() > 0)
		{
			if (checkEqualString(L5.com[0], "add") || checkEqualString(L5.com[0], "sub") || checkEqualString(L5.com[0], "mul") || checkEqualString(L5.com[0], "slt") || checkEqualString(L5.com[0], "addi"))
			{
				for (int i = 0; i < 10000; i++)
					qq++;
				REGISTERS[L5.REGISTER_ONE] = L5.VALUE_ONE;
			}
			else if (checkEqualString(L5.com[0], "sw"))
			{
			}
			else if (checkEqualString(L5.com[0], "lw"))
			{
				for (int i = 0; i < 10000; i++)
					qq++;
				REGISTERS[L5.VALUE_ONE] = data[L5.VALUE_TWO]; // loading done
			}
			else if (checkEqualString(L5.com[0], "beq") || checkEqualString(L5.com[0], "bne") || checkEqualString(L5.com[0], "j"))
			{
			}
			else
			{
				cout << "error at WB";
			}
		}

		if (CURRENT_COMMANDS_IN_PIPELINE.size() > 0 && L5.com == CURRENT_COMMANDS_IN_PIPELINE[0])
		{
			CURRENT_COMMANDS_IN_PIPELINE.erase(CURRENT_COMMANDS_IN_PIPELINE.begin());
			LIST_OF_COMMANDS.erase(LIST_OF_COMMANDS.begin());
		}

		// ----------------------------------------------------------MEM-------------------------------

		if (L4.com.size() > 0)
		{
			if (checkEqualString(L4.com[0], "sw") && L5.com != L4.com)
			{
				for (int i = 0; i < 10000; i++)
					qq++;
				L5.com = L4.com;
				L5.REGISTER_ONE = L4.REGISTER_ONE;
				L5.REGISTER_TWO = L4.REGISTER_TWO;
				for (int i = 0; i < 1000; i++)
					qq++;
				L5.VALUE_ONE = L4.VALUE_ONE;
				L5.VALUE_TWO = L4.VALUE_TWO;
				for (int i = 0; i < 1000; i++)
					qq++;
				SW_CONTROL_SIGNAL = true;
				data[L5.VALUE_TWO] = L5.VALUE_ONE;
				STORE_THE_ADDRESS = L5.VALUE_TWO;
				for (int i = 0; i < 1000; i++)
					qq++;
				STORE_THE_VALUE = L5.VALUE_ONE;
				cout << "1 " << L5.VALUE_TWO << " " << L5.VALUE_ONE << endl;
			}
			else
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L5.com = L4.com;
				for (int i = 0; i < 1000; i++)
					qq++;
				L5.REGISTER_ONE = L4.REGISTER_ONE;
				L5.REGISTER_TWO = L4.REGISTER_TWO;
				L5.VALUE_ONE = L4.VALUE_ONE;
				for (int i = 0; i < 1000; i++)
					qq++;
				L5.VALUE_TWO = L4.VALUE_TWO;
			}
		}

		if (!SW_CONTROL_SIGNAL)
		{
			cout << "0" << endl;
		}

		// Stage 3 ALU handling
		if (L3.com.size() > 0)
		{
			if (checkEqualString(L3.com[0], "add"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com;							// command
				L4.REGISTER_ONE = registerMap[L3.com[1]];	// register where to edit
				L4.VALUE_ONE = L3.VALUE_ONE + L3.VALUE_TWO; // value
			}
			else if (checkEqualString(L3.com[0], "sub"))
			{
				L4.com = L3.com;
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.REGISTER_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = L3.VALUE_ONE - L3.VALUE_TWO;
			}
			else if (checkEqualString(L3.com[0], "mul"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com;
				L4.REGISTER_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = L3.VALUE_ONE * L3.VALUE_TWO;
			}
			else if (checkEqualString(L3.com[0], "slt"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com;
				L4.REGISTER_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = 0;
				if (L3.VALUE_ONE < L3.VALUE_TWO)
					L4.VALUE_ONE = 1;
			}

			else if (checkEqualString(L3.com[0], "beq") || checkEqualString(L3.com[0], "bne") || checkEqualString(L3.com[0], "j"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com; // stores the next_Program_Counter value. if -1 then the next value is current_PC+1.
				if (checkEqualString(L3.com[0], "j"))
				{
					for (int i = 0; i < 1000; i++)
						qq++;
				}
				else if (checkEqualString(L3.com[0], "beq"))
				{
					for (int i = 0; i < 1000; i++)
						qq++;
					L4.VALUE_ONE = address[L3.com[3]];

					stall = true;
					stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					if (CURRENT_COMMANDS_IN_PIPELINE.size() > 0 && L2.com == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 1])
					{
						for (int i = 0; i < 1000; i++)
							qq++;
						current_PC--;
						LIST_OF_COMMANDS.pop_back();
						CURRENT_COMMANDS_IN_PIPELINE.pop_back();
					}
					if (checkEqualInt(L3.VALUE_ONE, L3.VALUE_TWO))
					{
						for (int i = 0; i < 1000; i++)
							qq++;
						current_PC = L4.VALUE_ONE;
					}

					L3.com.clear();
					L2.com.clear();
				}
				else if (checkEqualString(L3.com[0], "bne"))
				{
					for (int i = 0; i < 1000; i++)
						qq++;
					L4.VALUE_ONE = address[L3.com[3]];
					stall = true;
					stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					if (CURRENT_COMMANDS_IN_PIPELINE.size() > 0 && L2.com == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 1])
					{
						current_PC--;
						LIST_OF_COMMANDS.pop_back();
						CURRENT_COMMANDS_IN_PIPELINE.pop_back();
					}
					if (L3.VALUE_ONE != L3.VALUE_TWO)
					{
						current_PC = L4.VALUE_ONE;
					}
					for (int i = 0; i < 1000; i++)
						qq++;
					L3.com.clear();
					L2.com.clear();
				}
			}
			else if (checkEqualString(L3.com[0], "sw"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com;
				L4.VALUE_TWO = L3.VALUE_TWO;
				for (int i = 0; i < 1000; i++)
					qq++;						   // data address
				L4.VALUE_ONE = L3.VALUE_ONE;	   // register value
				L4.REGISTER_ONE = L3.REGISTER_ONE; // register number
			}
			else if (checkEqualString(L3.com[0], "lw"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com;
				L4.VALUE_TWO = L3.VALUE_TWO;		   // data address value
				L4.VALUE_ONE = registerMap[L3.com[1]]; // register number
				L4.REGISTER_ONE = L3.REGISTER_ONE;
			}
			else if (checkEqualString(L3.com[0], "addi"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				L4.com = L3.com;
				L4.REGISTER_ONE = L3.REGISTER_ONE;
				L4.VALUE_ONE = stoi(L3.com[3]) + L3.VALUE_TWO;
			}
			else
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				cout << L3.com[0] << endl;
				cout << "ALU handling something wrong came!!" << NUMBER_OF_CYCLES << endl;
			}
		}

		// -----------------------------------------------stalls------------------------------------------------------

		if (stall && stall_UNTIL_CYCLE != NUMBER_OF_CYCLES)
		{
			for (int i = 0; i < 1000; i++)
				qq++;
		}

		else if (stall && stall_UNTIL_CYCLE == NUMBER_OF_CYCLES)
		{
			stall = false;
		}

		else if (!stall && !L2.com.empty())
		{
			for (int i = 0; i < 1000; i++)
				qq++;
			if (checkEqualString(L2.com[0], "add") || checkEqualString(L2.com[0], "sub") || checkEqualString(L2.com[0], "mul") || checkEqualString(L2.com[0], "slt"))
			{
				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 2)
				{
					bool is_valid_command = checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "add") || checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sub") || checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "mul") || checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "slt") || checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "addi") || checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "lw");
					bool is_hazardous = CURRENT_COMMANDS_IN_PIPELINE[0][1] == L2.com[2] || CURRENT_COMMANDS_IN_PIPELINE[0][1] == L2.com[3];

					if (is_valid_command && is_hazardous)
					{
						stall = true;
						if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sw"))
						{
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
						else
						{
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
						}
					}
				}

				else if (CURRENT_COMMANDS_IN_PIPELINE.size() > 2)
				{
					string last_command = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0];
					string last_command_operand = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1];

					if (last_command == "add" || last_command == "sub" || last_command == "mul" || last_command == "slt" || last_command == "addi" || last_command == "lw")
					{
						if (last_command_operand == L2.com[2] || last_command_operand == L2.com[3])
						{
							stall = true;
							if (last_command == "sw")
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
							}
						}
					}

					if (!stall)
					{
						int secondLastCommandIndex = CURRENT_COMMANDS_IN_PIPELINE.size() - 2;
						string secondLastCommand = CURRENT_COMMANDS_IN_PIPELINE[secondLastCommandIndex][0];
						if (secondLastCommand == "add" || secondLastCommand == "sub" || secondLastCommand == "mul" || secondLastCommand == "slt" || secondLastCommand == "addi" || secondLastCommand == "lw")
						{
							string secondLastCommandArg1 = CURRENT_COMMANDS_IN_PIPELINE[secondLastCommandIndex][1];
							if (secondLastCommandArg1 == L2.com[2] || secondLastCommandArg1 == L2.com[3])
							{
								stall = true;
								stall_UNTIL_CYCLE = (secondLastCommand == "sw") ? (NUMBER_OF_CYCLES + 1) : (NUMBER_OF_CYCLES + 2);
							}
						}
					}

					if (!stall)
					{
						int thirdLastCommandIndex = CURRENT_COMMANDS_IN_PIPELINE.size() - 3;
						string thirdLastCommand = CURRENT_COMMANDS_IN_PIPELINE[thirdLastCommandIndex][0];
						if (thirdLastCommand == "add" || thirdLastCommand == "sub" || thirdLastCommand == "mul" || thirdLastCommand == "slt" || thirdLastCommand == "addi")
						{
							string thirdLastCommandArg1 = CURRENT_COMMANDS_IN_PIPELINE[thirdLastCommandIndex][1];
							if (thirdLastCommandArg1 == L2.com[2] || thirdLastCommandArg1 == L2.com[3])
							{
								stall = true;
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
							}
						}
					}
				}
			}

			else if (checkEqualString(L2.com[0], "beq") || checkEqualString(L2.com[0], "bne"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 2)
				{
					std::string opcode = CURRENT_COMMANDS_IN_PIPELINE[0][0];
					std::string operand = CURRENT_COMMANDS_IN_PIPELINE[0][1];
					bool is_stall_op = (opcode == "add" || opcode == "sub" || opcode == "mul" || opcode == "slt" || opcode == "addi" || opcode == "lw");

					if (is_stall_op && (operand == L2.com[1] || operand == L2.com[2]))
					{
						for (int i = 0; i < 1000; i++)
							qq++;
						stall = true;
						if (opcode == "sw")
						{
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
						else
						{
							for (int i = 0; i < 1000; i++)
								qq++;
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
						}
					}
				}

				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					for (int i = 0; i < 1000; i++)
						qq++;
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
					{
						string lastCommand = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0];
						if (lastCommand == "add" || lastCommand == "sub" || lastCommand == "mul" || lastCommand == "slt" || lastCommand == "addi" || lastCommand == "lw")
						{
							for (int i = 0; i < 1000; i++)
								qq++;
							string lastCommandOperand1 = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1];
							if (lastCommandOperand1 == L2.com[1] || lastCommandOperand1 == L2.com[2])
							{
								for (int i = 0; i < 1000; i++)
									qq++;
								stall = true;
								if (lastCommand == "sw")
								{
									for (int i = 0; i < 1000; i++)
										qq++;
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
								else
								{
									for (int i = 0; i < 1000; i++)
										qq++;
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
								}
							}
						}
					}

					if (!stall && CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						string op = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0];
						string arg = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1];
						if ((op == "add" || op == "sub" || op == "mul" || op == "slt" || op == "addi") && (arg == L2.com[1] || arg == L2.com[2]))
						{
							for (int i = 0; i < 1000; i++)
							{
								qq++;
							}
							stall = true;
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
						else
						{
							for (int i = 0; i < 1000; i++)
							{
								qq++;
							}
						}
					}
				}
			}

			else if (checkEqualString(L2.com[0], "lw"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				size_t opening_paren_pos = L2.com[2].find("(");
				string res = "";
				if (opening_paren_pos != string::npos)
				{
					size_t closing_paren_pos = L2.com[2].find(")", opening_paren_pos + 1);
					if (closing_paren_pos != string::npos)
					{
						for (int i = 0; i < 1000; i++)
						{
							qq++;
						}
						res = L2.com[2].substr(opening_paren_pos + 1, closing_paren_pos - opening_paren_pos - 1);
					}
				}

				// res is checked if it exists.

				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 2)
				{
					for (int i = 0; i < 1000; i++)
						qq++;
					if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sw"))
					{
						string addr = CURRENT_COMMANDS_IN_PIPELINE[0][2];
						if (locateAddress(addr) == locateAddress(L2.com[2]))
						{
							stall = true;
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
					}
				}
				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					for (int i = 0; i < 1000; i++)
						qq++;
					if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "sw"))
					{
						string addr = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][2];
						if (locateAddress(addr) == locateAddress(L2.com[2]))
						{
							stall = true;
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
					}
					if (!stall && CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
					{
						string addr = CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][2];
						if (locateAddress(addr) == locateAddress(L2.com[2]))
						{
							// do something if needed
						}
					}
				}

				if (stall && stall_UNTIL_CYCLE == NUMBER_OF_CYCLES + 2)
				{
				}
				else
				{
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
					{
						if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "add") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sub") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "mul") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "slt") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "addi") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "lw"))
						{

							if (CURRENT_COMMANDS_IN_PIPELINE[0][1] == res)
							{
								stall = true;
								if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sw"))
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
								}
							}
						}

						if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
						{
							if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "add") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "sub") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "mul") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "slt") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "addi") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "lw"))
							{

								if (CURRENT_COMMANDS_IN_PIPELINE[1][1] == res)
								{
									stall = true;
									if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[1][0], "sw"))
									{
										stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
									}
									else
									{
										stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
									}
								}
							}

							if (!stall && CURRENT_COMMANDS_IN_PIPELINE.size() >= 4)
							{
								if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[2][0], "add") ||
									checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[2][0], "sub") ||
									checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[2][0], "mul") ||
									checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[2][0], "slt") ||
									checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[2][0], "addi"))
								{

									if (CURRENT_COMMANDS_IN_PIPELINE[2][1] == res)
									{
										stall = true;
										stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
									}
								}
							}
						}
					}
				}
			}

			else if (checkEqualString(L2.com[0], "sw"))
			{
				for (int i = 0; i < 1000; i++)
					qq++;
				size_t POSSSS1 = L2.com[2].find("(");
				string res = "";
				if (POSSSS1 != string::npos)
				{ // if opening parenthesis is found
					size_t POSSSS2 = L2.com[2].find(")", POSSSS1 + 1);
					if (POSSSS2 != string::npos)
					{																// if closing parenthesis is found
						res = L2.com[2].substr(POSSSS1 + 1, POSSSS2 - POSSSS1 - 1); // extract the substring between the parentheses
					}
				}

				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 2)
				{
					bool is_dependency = false;
					if (std::find(std::begin({"add", "sub", "mul", "slt", "addi", "lw"}), std::end({"add", "sub", "mul", "slt", "addi", "lw"}), CURRENT_COMMANDS_IN_PIPELINE[0][0]) != std::end({"add", "sub", "mul", "slt", "addi", "lw"}))
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[0][1] == L2.com[1])
						{
							stall = true;
							if (CURRENT_COMMANDS_IN_PIPELINE[0][0] == "sw")
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
							}
						}
					}
				}
				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					bool is_dependency = false;
					if (std::find(std::begin({"add", "sub", "mul", "slt", "addi", "lw"}), std::end({"add", "sub", "mul", "slt", "addi", "lw"}), CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0]) != std::end({"add", "sub", "mul", "slt", "addi", "lw"}))
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.com[1])
						{
							stall = true;
							if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
							}
						}
						is_dependency = true;
					}
					if (!is_dependency && std::find(std::begin({"add", "sub", "mul", "slt", "addi"}), std::end({"add", "sub", "mul", "slt", "addi"}), CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0]) != std::end({"add", "sub", "mul", "slt", "addi"}))
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.com[1])
						{
							stall = true;
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
					}
				}

				if (stall && stall_UNTIL_CYCLE == NUMBER_OF_CYCLES + 2)
				{
					// no need
				}
				else
				{
					if (CURRENT_COMMANDS_IN_PIPELINE.size() == 2)
					{
						if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "add") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sub") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "mul") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "slt") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "addi") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "lw"))
						{
							if (CURRENT_COMMANDS_IN_PIPELINE[0][1] == res)
							{
								stall = true;
								if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[0][0], "sw"))
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
								}
							}
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "add") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "sub") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "mul") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "slt") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "addi") ||
							checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "lw"))
						{
							if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == res)
							{
								stall = true;
								if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0], "sw"))
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
								}
							}
						}
						if (!stall)
						{
							if (checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0], "add") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0], "sub") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0], "mul") ||
								checkEqualString(CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0], "slt") ||
								CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
							{
								if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1] == res)
								{
									stall = true;
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
							}
						}
					}
				}
			}

			else if (L2.com[0] == "addi")
			{ // addiint sm=0;
				for (int i = 0; i < 100000; i++)
					sm += 1;
				const int MAX_PIPELINE_SIZE = 3;

				if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2 && CURRENT_COMMANDS_IN_PIPELINE.size() <= MAX_PIPELINE_SIZE)
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[0][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[0][0] == "sub" || CURRENT_COMMANDS_IN_PIPELINE[0][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[0][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[0][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[0][0] == "lw")
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[0][1] == L2.com[2])
						{
							stall = true;
							if (CURRENT_COMMANDS_IN_PIPELINE[0][0] == "sw")
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
							}
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE.size() == MAX_PIPELINE_SIZE)
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "sub" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "lw")
						{
							if (CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][1] == L2.com[2])
							{
								stall = true;
								if (CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 2][0] == "sw")
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 2;
								}
							}
						}
						if (!stall && CURRENT_COMMANDS_IN_PIPELINE.size() == MAX_PIPELINE_SIZE)
						{
							if (CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 3][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 3][0] == "sub" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 3][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 3][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 3][0] == "addi")
							{
								if (CURRENT_COMMANDS_IN_PIPELINE[MAX_PIPELINE_SIZE - 3][1] == L2.com[2])
								{
									stall = true;
									stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
								}
							}
						}
					}
				}
			}

			else if (L2.com[0] == "j")
			{
			}

			else
			{
				cout << "Error at stalls!";
			}
		}

		// ----------------------------------------------ID------------------------------------------------

		for (int i = 0; i < 100000; i++)
			sm += 1;
		if (!stall)
		{
			if (!L2.com.empty())
			{
				if (checkEqualString(L2.com[0], "add") || checkEqualString(L2.com[0], "sub") ||
					checkEqualString(L2.com[0], "mul") || checkEqualString(L2.com[0], "slt"))
				{
					L3.com = L2.com;
					L3.REGISTER_ONE = registerMap[L2.com[2]];
					L3.REGISTER_TWO = registerMap[L2.com[3]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = REGISTERS[L3.REGISTER_TWO];
				}
				else if (checkEqualString(L2.com[0], "beq") || checkEqualString(L2.com[0], "bne"))
				{
					L3.com = L2.com;
					L3.REGISTER_ONE = registerMap[L2.com[1]];
					L3.REGISTER_TWO = registerMap[L2.com[2]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = REGISTERS[L3.REGISTER_TWO];
				}
				else if (checkEqualString(L2.com[0], "j"))
				{
					L3.com = L2.com;
					L3.VALUE_ONE = address[L2.com[1]];
					current_PC = L3.VALUE_ONE;
					stall = true;
					stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					if (!CURRENT_COMMANDS_IN_PIPELINE.empty() && L2.com == CURRENT_COMMANDS_IN_PIPELINE.back())
					{
						LIST_OF_COMMANDS.pop_back();
						CURRENT_COMMANDS_IN_PIPELINE.pop_back();
					}
					L3.com.clear();
					L2.com.clear();
				}
				else if (checkEqualString(L2.com[0], "sw") || checkEqualString(L2.com[0], "lw"))
				{
					string input = L2.com[2];
					int POSSSS1 = input.find("(");
					int POSSSS2 = input.find(")");
					string number = input.substr(0, POSSSS1);
					string dollarSign = "$" + input.substr(POSSSS1 + 2, POSSSS2 - POSSSS1 - 2);
					int address = (stoi(number) + REGISTERS[registerMap[dollarSign]]) / 4;
					L3.com = L2.com;
					L3.REGISTER_ONE = registerMap[L2.com[1]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = address;
				}
				else if (checkEqualString(L2.com[0], "addi"))
				{
					L3.com = L2.com;
					L3.REGISTER_ONE = registerMap[L2.com[1]];
					L3.REGISTER_TWO = registerMap[L2.com[2]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = REGISTERS[L3.REGISTER_TWO];
				}
				else
				{
					cout << "Error";
				}
			}
		}

		for (int i = 0; i < 100000; i++)
			sm += 1;
		vector<string> command;
		// Check if there are more commands to execute and the pipeline is not stalled
		if (current_PC < commands.size() && !stall)
		{
			// Get the current command from the list of commands
			command = commands[current_PC];
			// Check if the command is a valid instruction
			if (INSTRUCTIONS.find(command[0]) == INSTRUCTIONS.end())
			{
				// If the command is invalid, exit with a syntax error and the current number of cycles
				handleExit(SYNTAX_ERROR, NUMBER_OF_CYCLES);
				return;
			}
			// Add the current command to the list of executed commands and the pipeline
			LIST_OF_COMMANDS.push_back(current_PC);
			CURRENT_COMMANDS_IN_PIPELINE.push_back(command);
		}

		for (int i = 0; i < 100000; i++)
			sm += 1;

		if (CURRENT_COMMANDS_IN_PIPELINE.empty())
		{
			// Print the current number of cycles
			register_PRINT(NUMBER_OF_CYCLES);
			// Check if the software control signal is off
			if (!SW_CONTROL_SIGNAL)
			{
				// Output a 0 to the console
				cout << "0" << endl;
			}
			else
			{
				// Output the address and value stored in the signal to the console
				cout << "1 " << STORE_THE_ADDRESS << " " << STORE_THE_VALUE << endl;
			}
			// End the function call
			return;
		}

		// -------------------------------------------IF--------------------------
		if (current_PC < commands.size() && !stall)
		{
			L2.com = command;
			current_PC++;
		}

		EXECUTE_THE_PIPELINE(NUMBER_OF_CYCLES, LIST_OF_COMMANDS, CURRENT_COMMANDS_IN_PIPELINE);
	}

	void register_PRINT(int clockCycle)
	{

		for (int i = 0; i < 100000; i++)
			sm += 1;
		for (int i = 0; i < 32; ++i)
			cout << REGISTERS[i] << ' ';
		cout << dec << '\n';
	}
};

#endif