/**
 * @file MIPS_Processor.hpp
 * @author Mallika Prabhakar and Sayam Sethi
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
	struct LATCH_BETWEEN_REGISTER
	{
		vector<string> com = {};
		int REG_ONE = 0;
		int VALUE_ONE = 0;
		int REG_TWO = 0;
		int VALUE_TWO = 0;
	};
	int REGISTERS[32] = {0}, current_PC = 0, next_Program_Counter;													// REGISTERS
	unordered_map<string, function<int(MIPS_Architecture &, string, string, string)>> INSTRUCTIONS; // INSTRUCTIONS
	unordered_map<string, int> registerMap, address;												// Memory
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	vector<vector<string>> commands;
	vector<int> commandCount;
	LATCH_BETWEEN_REGISTER L2, L3, L4, L5;
	bool stall = false;
	int stall_UNTIL_CYCLE = 0;

	enum exit_code
	{
		SUCCESS = 0,
		INVALID_REGISTER,
		INVALID_LABEL,
		INVALID_ADDRESS,
		SYNTAX_ERROR,
		MEMORY_ERROR
	};

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
	int add(string r1, string r2, string r3)
	{
		return REGISTERS[registerMap[r2]] + REGISTERS[registerMap[r3]];
	}

	// perform subtraction operation
	int sub(string r1, string r2, string r3)
	{
		return REGISTERS[registerMap[r2]] - REGISTERS[registerMap[r3]];
	}

	// perform multiplication operation
	int mul(string r1, string r2, string r3)
	{
		return REGISTERS[registerMap[r2]] * REGISTERS[registerMap[r3]];
	}

	// perform the beq operation
	int beq(string r1, string r2, string label)
	{
		return REGISTERS[registerMap[r1]] == REGISTERS[registerMap[r2]];
	}

	// perform the bne operation
	int bne(string r1, string r2, string label)
	{
		return REGISTERS[registerMap[r1]] != REGISTERS[registerMap[r2]];
	}

	// implements slt operation
	int slt(string r1, string r2, string r3)
	{
		return REGISTERS[registerMap[r1]] < REGISTERS[registerMap[r2]];
	}

	// perform the jump operation
	int j(string label, string unused1 = "", string unused2 = "")
	{
		if (!LABEL_CHECK(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		return address[label];
	}

	// perform load word operation
	int lw(string r, string location, string unused1 = "")
	{
		if (!REGISTER_CHECK(r) || registerMap[r] == 0)
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);

		return address;
	}

	// perform store word operation
	int sw(string r, string location, string unused1 = "")
	{
		if (!REGISTER_CHECK(r))
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);
		return address;
	}

	int locateAddress(string location)
	{
		string input = location;
		int pos1 = input.find("(");										   // find the position of the opening parenthesis
		int pos2 = input.find(")");										   // find the position of the closing parenthesis
		string number = input.substr(0, pos1);							   // extract number1 as a string and convert it to an int
		string dollarSign = "$" + input.substr(pos1 + 2, pos2 - pos1 - 2); // extract number2 as a string and convert it to an int
		return (stoi(number) + REGISTERS[registerMap[dollarSign]]) / 4;
	}

	// perform add immediate operation
	int addi(string r1, string r2, string num)
	{
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
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   INSTRUCTIONS.find(str) == INSTRUCTIONS.end();
	}

	// checks if the register is a valid one
	 bool REGISTER_CHECK(string r)
	{
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the REGISTERS are valid or not
	bool REGISTERS_CHECK(vector<string> regs)
	{
		return all_of(regs.begin(), regs.end(), [&](string r)
					  { return REGISTER_CHECK(r); });
	}

	
	void handleExit(exit_code code, int cycleCount)
	{
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
	}

	// parse the command assuming correctly formatted MIPS instruction (or label)
	void parseCommand(string line)
	{
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
			string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command.clear();
		}
		else if (command[0].back() == ':')
		{
			string label = command[0].substr(0, command[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = commands.size();
			else
				address[label] = -1;
			command = vector<string>(command.begin() + 1, command.end());
		}
		else if (command[0].find(':') != string::npos)
		{
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
	}

	void constructCommands(ifstream &file)
	{
		string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}


	void executeCommandsPipelined()
	{
		if (commands.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int NUMBER_OF_CYCLES = 0;
		vector<int> LIST_OF_COMMANDS;
		vector<vector<string>> CURRENT_COMMANDS_IN_PIPELINE;
		EXECUTE_THE_PIPELINE(NUMBER_OF_CYCLES, LIST_OF_COMMANDS, CURRENT_COMMANDS_IN_PIPELINE);
	}

	void EXECUTE_THE_PIPELINE(int &NUMBER_OF_CYCLES, vector<int> &LIST_OF_COMMANDS, vector<vector<string>> &CURRENT_COMMANDS_IN_PIPELINE)
	{

		register_PRINT(NUMBER_OF_CYCLES);
		NUMBER_OF_CYCLES++;
		bool storedword = false;
		bool branch = false;
		int storedaddress = 0;
		int storedvalue = 0;

		// stage5               ---------------------------------

		if (L5.com.size() > 0)
		{

			if (L5.com[0] == "add" || L5.com[0] == "sub" || L5.com[0] == "mul" || L5.com[0] == "slt" || L5.com[0] == "addi")
			{
				REGISTERS[registerMap[L5.com[1]]] = L5.VALUE_ONE;
			}
			else if (L5.com[0] == "beq" || L5.com[0] == "bne" || L5.com[0] == "j")
			{
				// nothing to do.
			}
			else if (L5.com[0] == "sw")
			{
			}
			else if (L5.com[0] == "lw")
			{
				REGISTERS[L5.REG_ONE] = L5.VALUE_ONE; // loading done in DM stage.
			}
			else
			{
				cout << "something wrong occured in stage5!!";
			}
		}

		// marks completion of commands.
		if (CURRENT_COMMANDS_IN_PIPELINE.size() > 0 && L5.com == CURRENT_COMMANDS_IN_PIPELINE[0])
		{ // if we found that some command has been completed in this cycle. Then remove it.
			CURRENT_COMMANDS_IN_PIPELINE.erase(CURRENT_COMMANDS_IN_PIPELINE.begin());
			LIST_OF_COMMANDS.erase(LIST_OF_COMMANDS.begin());
		}

		// stage4   DM ---------------------------------------------------------------

		if (L4.com.size() > 0)
		{
			if (L4.com[0] == "lw")
			{
				L5.com = L4.com;
				L5.REG_ONE = L4.REG_ONE;
				L5.REG_TWO = L4.REG_TWO;
				L5.VALUE_ONE = L4.VALUE_ONE;
				L5.VALUE_TWO = L4.VALUE_TWO;
				string input = L5.com[2];
				int pos1 = input.find("(");										   // find the position of the opening parenthesis
				int pos2 = input.find(")");										   // find the position of the closing parenthesis
				string number = input.substr(0, pos1);							   // extract number1 as a string and convert it to an int
				string dollarSign = "$" + input.substr(pos1 + 2, pos2 - pos1 - 2); // extract number2 as a string and convert it to an int

				L5.VALUE_ONE = data[(stoi(number) + L4.VALUE_TWO) / 4]; // LATCH_BETWEEN_REGISTER loads value at lw.
			}
			else if (L4.com[0] == "sw")
			{
				L5.com = L4.com;
				L5.REG_ONE = L4.REG_ONE;
				L5.REG_TWO = L4.REG_TWO;
				L5.VALUE_ONE = L4.VALUE_ONE;
				L5.VALUE_TWO = L4.VALUE_TWO;
				string input = L5.com[2];
				int pos1 = input.find("(");										   // find the position of the opening parenthesis
				int pos2 = input.find(")");										   // find the position of the closing parenthesis
				string number = input.substr(0, pos1);							   // extract number1 as a string and convert it to an int
				string dollarSign = "$" + input.substr(pos1 + 2, pos2 - pos1 - 2); // extract number2 as a string and convert it to an int
				storedword = true;
				data[(stoi(number) + L5.VALUE_TWO) / 4] = L5.VALUE_ONE; // storage done
				storedaddress = (stoi(number) + L5.VALUE_TWO) / 4;
				storedvalue = L5.VALUE_ONE;
				cout << "1 " << storedaddress << " " << L5.VALUE_ONE << endl;
			}
			else
			{
				L5.com = L4.com;
				L5.REG_ONE = L4.REG_ONE;
				L5.REG_TWO = L4.REG_TWO;
				L5.VALUE_ONE = L4.VALUE_ONE;
				L5.VALUE_TWO = L4.VALUE_TWO;
			}
		}

		if (!storedword)
		{
			cout << "0" << endl;
		}
		// Stage 3 ALU handling
		if (L3.com.size() > 0)
		{
			if (L3.com[0] == "add")
			{
				L4.com = L3.com;
				L4.REG_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = L3.VALUE_ONE + L3.VALUE_TWO;
			}
			else if (L3.com[0] == "sub")
			{
				L4.com = L3.com;
				L4.REG_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = L3.VALUE_ONE - L3.VALUE_TWO;
			}
			else if (L3.com[0] == "mul")
			{
				L4.com = L3.com;
				L4.REG_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = L3.VALUE_ONE * L3.VALUE_TWO;
			}
			else if (L3.com[0] == "slt")
			{
				L4.com = L3.com;
				L4.REG_ONE = registerMap[L3.com[1]];
				L4.VALUE_ONE = 0;
				if (L3.VALUE_ONE < L3.VALUE_TWO)
					L4.VALUE_ONE = 1;
			}

			else if (L3.com[0] == "beq" || L3.com[0] == "bne" || L3.com[0] == "j")
			{					 // during bypassing
				L4.com = L3.com; // stores the next_Program_Counter value. if -1 then the next value is current_PC+1.
				if (L3.com[0] == "j")
				{
					// L4.VALUE_ONE=address[L3.com[1]];
					// current_PC=L4.VALUE_ONE;
					// stall=true;
					// stall_UNTIL_CYCLE=NUMBER_OF_CYCLES+1;
					// if(CURRENT_COMMANDS_IN_PIPELINE.size()>0 && L2.com==CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size()-1]){
					// 	LIST_OF_COMMANDS.pop_back();
					// 	CURRENT_COMMANDS_IN_PIPELINE.pop_back();
					// }
					// L3.com.clear();
					// L2.com.clear();
				}
				else if (L3.com[0] == "beq")
				{
					L4.VALUE_ONE = address[L3.com[3]];

					stall = true;
					stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					if (CURRENT_COMMANDS_IN_PIPELINE.size() > 0 && L2.com == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 1])
					{
						current_PC--;
						LIST_OF_COMMANDS.pop_back();
						CURRENT_COMMANDS_IN_PIPELINE.pop_back();
					}
					if (L3.VALUE_ONE == L3.VALUE_TWO)
					{
						current_PC = L4.VALUE_ONE;
					}

					L3.com.clear();
					L2.com.clear();
				}
				else if (L3.com[0] == "bne")
				{
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
					L3.com.clear();
					L2.com.clear();
				}
			}
			else if (L3.com[0] == "sw")
			{
				L4.com = L3.com;
				L4.VALUE_TWO = L3.VALUE_TWO; // data address value
				L4.VALUE_ONE = L3.VALUE_ONE; // register number
				L4.REG_ONE = L3.REG_ONE;
				L4.REG_TWO = L3.REG_TWO;
			}
			else if (L3.com[0] == "lw")
			{
				L4.com = L3.com;
				L4.VALUE_TWO = L3.VALUE_TWO; // data address value
				L4.VALUE_ONE = L3.VALUE_ONE; // register number
				L4.REG_ONE = L3.REG_ONE;
				L4.REG_TWO = L3.REG_TWO;
			}
			else if (L3.com[0] == "addi")
			{
				L4.com = L3.com;
				L4.REG_ONE = L3.REG_ONE;
				L4.VALUE_ONE = stoi(L3.com[3]) + L3.VALUE_ONE;
				L4.VALUE_TWO = L3.VALUE_TWO;
				L4.REG_TWO = L3.REG_TWO;
				// cout<<"Here !!! "<<L4.VALUE_ONE<<endl;
			}
			else
			{
				cout << L3.com[0] << endl;
				cout << "ALU handling something wrong came!!" << NUMBER_OF_CYCLES << endl;
			}
		}

		// Stage 2 ID Stage  ---------------------------------------------------------
		// implement stalls.

		if (stall && stall_UNTIL_CYCLE == NUMBER_OF_CYCLES)
		{ // done
			stall = false;
		}
		else if (!stall && !L2.com.empty())
		{
			if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2 && CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
			{
				if (L2.com[0] == "add" || L2.com[0] == "sub" || L2.com[0] == "slt" || L2.com[0] == "mul")
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.com[2] || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.com[3])
					{
						stall = true;
						stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					}
				}
				else if (L2.com[0] == "addi")
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.com[2])
					{
						stall = true;
						stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					}
				}
				else if (L2.com[0] == "beq" || L2.com[0] == "bne")
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.com[1] || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.com[2])
					{
						stall = true;
						stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					}
				}
				else if (L2.com[0] == "sw" || L2.com[0] == "lw")
				{
					size_t pos1 = L2.com[2].find("("); // find the position of the opening parenthesis
					string res = "";
					if (pos1 != string::npos)
					{												 // if opening parenthesis is found
						size_t pos2 = L2.com[2].find(")", pos1 + 1); // find the position of the closing parenthesis after the opening parenthesis
						if (pos2 != string::npos)
						{													   // if closing parenthesis is found
							res = L2.com[2].substr(pos1 + 1, pos2 - pos1 - 1); // extract the substring between the parentheses
						}
					}

					if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
					{
						stall = true;
						stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
					}

					if (!stall && L2.com[0] == "sw")
					{ // if sw after lw.
						if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							stall = true;
							stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
						}
					}
				}
			}
		}

		if (!stall && !L2.com.empty())
		{

			L3.com = L2.com; ////commadn gets copied anyways.

			if (L2.com[0] == "add" || L2.com[0] == "sub" || L2.com[0] == "slt" || L2.com[0] == "mul")
			{
				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 1)
				{ // only one command behind.
					L3.REG_ONE = registerMap[L2.com[2]];
					L3.REG_TWO = registerMap[L2.com[3]];
					L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
					L3.VALUE_TWO = REGISTERS[registerMap[L2.com[3]]];
					// cout<<"Here !!! "<<L3.VALUE_ONE<<" "<<L3.VALUE_TWO<<endl;
				}
				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{ // the command was lw.
						L3.REG_ONE = registerMap[L2.com[2]];
						L3.REG_TWO = registerMap[L2.com[3]];

						// if second last command had some effects.
						if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
						}

						if (L2.com[3] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[L2.com[3]]];
						}
					}

					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub")
					{
						L3.REG_ONE = registerMap[L2.com[2]];
						L3.REG_TWO = registerMap[L2.com[3]];

						if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
						}

						if (L2.com[3] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							L3.VALUE_TWO = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[L2.com[3]]];
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
					{
						L3.REG_ONE = registerMap[L2.com[2]];
						L3.REG_TWO = registerMap[L2.com[3]];
						L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
						L3.VALUE_TWO = REGISTERS[registerMap[L2.com[3]]];
					}

					// if third last had some effects.
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "lw")
						{ // the command was lw.
							L3.REG_ONE = registerMap[L2.com[2]];
							L3.REG_TWO = registerMap[L2.com[3]];

							// if second last command had some effects.
							if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}

							if (L2.com[3] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}

						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub")
						{
							L3.REG_ONE = registerMap[L2.com[2]];
							L3.REG_TWO = registerMap[L2.com[3]];

							if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}

							if (L2.com[3] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}
						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
						{
							// L3.REG_ONE=registerMap[L2.com[2]];
							// L3.REG_TWO=registerMap[L2.com[3]];
							// L3.VALUE_ONE=REGISTERS[registerMap[L2.com[2]]];
							// L3.VALUE_TWO=REGISTERS[registerMap[L2.com[3]]];
						}
					}
				}
			}
			else if (L2.com[0] == "beq" || L2.com[0] == "bne")
			{
				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 1)
				{ // only one command behind.

					L3.REG_ONE = registerMap[L2.com[1]];
					L3.REG_TWO = registerMap[L2.com[2]];
					L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
					L3.VALUE_TWO = REGISTERS[registerMap[L2.com[2]]];
				}

				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{ // the command was lw.
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[L2.com[2]];

						// if second last command had some effects.
						if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
						}

						if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[L2.com[2]]];
						}
					}

					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub")
					{
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[L2.com[2]];

						if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
						}

						if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							L3.VALUE_TWO = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[L2.com[2]]];
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
					{
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[L2.com[2]];
						L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
						L3.VALUE_TWO = REGISTERS[registerMap[L2.com[2]]];
					}

					// if third last had some effects.
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "lw")
						{ // the command was lw.
							L3.REG_ONE = registerMap[L2.com[1]];
							L3.REG_TWO = registerMap[L2.com[2]];

							// if second last command had some effects.
							if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}

							if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}

						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub")
						{
							L3.REG_ONE = registerMap[L2.com[1]];
							L3.REG_TWO = registerMap[L2.com[2]];

							if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}

							if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}
						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
						{
							// L3.REG_ONE=registerMap[L2.com[1]];
							// L3.REG_TWO=registerMap[L2.com[2]];
							// L3.VALUE_ONE=REGISTERS[registerMap[L2.com[1]]];
							// L3.VALUE_TWO=REGISTERS[registerMap[L2.com[2]]];
						}
					}
				}
			}
			else if (L2.com[0] == "j")
			{
				L3.com = L2.com;
				L3.VALUE_ONE = address[L2.com[1]];
				current_PC = L3.VALUE_ONE;
				stall = true;
				stall_UNTIL_CYCLE = NUMBER_OF_CYCLES + 1;
				if (CURRENT_COMMANDS_IN_PIPELINE.size() > 0 && L2.com == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 1])
				{
					LIST_OF_COMMANDS.pop_back();
					CURRENT_COMMANDS_IN_PIPELINE.pop_back();
				}
				L3.com.clear();
				L2.com.clear();
			}
			else if (L2.com[0] == "addi")
			{
				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 1)
				{ // only one command behind.

					L3.REG_ONE = registerMap[L2.com[2]];
					L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
					L3.VALUE_TWO = stoi(L2.com[3]);
				}

				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{ // the command was lw.
						L3.REG_ONE = registerMap[L2.com[2]];

						// if second last command had some effects.
						if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
						}
					}

					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub")
					{
						L3.REG_ONE = registerMap[L2.com[2]];

						if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
					{
						L3.REG_ONE = registerMap[L2.com[2]];
						L3.VALUE_ONE = REGISTERS[registerMap[L2.com[2]]];
						L3.VALUE_TWO = stoi(L2.com[3]);
					}

					// if third last had some effects.
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "lw")
						{ // the command was lw.
							L3.REG_ONE = registerMap[L2.com[2]];

							// if second last command had some effects.
							if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}

						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub")
						{
							L3.REG_ONE = registerMap[L2.com[2]];

							if (L2.com[2] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}
						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
						{
							// L3.REG_ONE=registerMap[L2.com[1]];
							// L3.REG_TWO=registerMap[L2.com[2]];
							// L3.VALUE_ONE=REGISTERS[registerMap[L2.com[1]]];
							// L3.VALUE_TWO=REGISTERS[registerMap[L2.com[2]]];
						}
					}
				}
			}
			else if (L2.com[0] == "lw")
			{
				size_t pos1 = L2.com[2].find("("); // find the position of the opening parenthesis
				string res = "";
				if (pos1 != string::npos)
				{												 // if opening parenthesis is found
					size_t pos2 = L2.com[2].find(")", pos1 + 1); // find the position of the closing parenthesis after the opening parenthesis
					if (pos2 != string::npos)
					{													   // if closing parenthesis is found
						res = L2.com[2].substr(pos1 + 1, pos2 - pos1 - 1); // extract the substring between the parentheses
					}
				}

				// res contains the required register to be checked.
				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 1)
				{ // only one command behind.
					L3.REG_ONE = registerMap[L2.com[1]];
					L3.REG_TWO = registerMap[res];
					L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
					L3.VALUE_ONE = REGISTERS[registerMap[res]];
				}

				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{ // the command was lw.
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[res];

						// if second last command had some effects.
						if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[res]];
						}
					}

					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub")
					{
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[res];

						if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_TWO = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[res]];
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
					{
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[res];
					}

					// if third last had some effects.
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "lw")
						{ // the command was lw.
							L3.REG_ONE = registerMap[L2.com[1]];
							L3.REG_TWO = registerMap[res];

							// if second last command had some effects.
							if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}

						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub")
						{
							L3.REG_ONE = registerMap[L2.com[1]];
							L3.REG_TWO = registerMap[res];

							if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}
						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
						{
						}
					}
				}
				// cout<<"Here!!!! lw "<<L3.REG_ONE<<" "<<L3.VALUE_ONE<<" "<<L3.REG_TWO<<" "<<L3.VALUE_TWO<<endl;
			}

			else if (L2.com[0] == "sw")
			{
				size_t pos1 = L2.com[2].find("("); // find the position of the opening parenthesis
				string res = "";
				if (pos1 != string::npos)
				{												 // if opening parenthesis is found
					size_t pos2 = L2.com[2].find(")", pos1 + 1); // find the position of the closing parenthesis after the opening parenthesis
					if (pos2 != string::npos)
					{													   // if closing parenthesis is found
						res = L2.com[2].substr(pos1 + 1, pos2 - pos1 - 1); // extract the substring between the parentheses
					}
				}

				if (CURRENT_COMMANDS_IN_PIPELINE.size() == 1)
				{ // only one command behind.

					L3.REG_ONE = registerMap[L2.com[1]];
					L3.REG_TWO = registerMap[res];
					L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
					L3.VALUE_TWO = REGISTERS[registerMap[res]];
				}

				else if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 2)
				{
					if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{ // the command was lw.
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[res];

						// if second last command had some effects.
						if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
						}

						if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[res]];
						}
					}

					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub")
					{
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[res];

						if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{					   // if there was a conflict with 2nd arg register.
							L3.VALUE_ONE = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
						}

						if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][1])
						{
							L3.VALUE_TWO = L4.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
						}
						else
						{
							L3.VALUE_TWO = REGISTERS[registerMap[res]];
						}
					}
					else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
					{
						L3.REG_ONE = registerMap[L2.com[1]];
						L3.REG_TWO = registerMap[res];
						L3.VALUE_ONE = REGISTERS[registerMap[L2.com[1]]];
						L3.VALUE_TWO = REGISTERS[registerMap[res]];
					}

					// if third last had some effects.
					if (CURRENT_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "lw")
						{ // the command was lw.
							L3.REG_ONE = registerMap[L2.com[1]];
							L3.REG_TWO = registerMap[res];

							// if second last command had some effects.
							if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}

							if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}

						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub")
						{
							L3.REG_ONE = registerMap[L2.com[1]];
							L3.REG_TWO = registerMap[res];

							if (L2.com[1] == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{					   // if there was a conflict with 2nd arg register.
								L3.VALUE_ONE = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}

							if (res == CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][1])
							{
								L3.VALUE_TWO = L5.VALUE_ONE; // take from LATCH_BETWEEN_REGISTER
							}
						}
						else if (CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "beq" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "bne" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "j" || CURRENT_COMMANDS_IN_PIPELINE[CURRENT_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
						{
							// L3.REG_ONE=registerMap[L2.com[1]];
							// L3.REG_TWO=registerMap[L2.com[2]];
							// L3.VALUE_ONE=REGISTERS[registerMap[L2.com[1]]];
							// L3.VALUE_TWO=REGISTERS[registerMap[L2.com[2]]];
						}
					}
				}
				// cout<<"Here!!!! sw "<<L3.REG_ONE<<" "<<L3.VALUE_ONE<<" "<<L3.REG_TWO<<" "<<L3.VALUE_TWO<<endl;
			}
			else
			{
				cout << "There is some problem in ID stage!!" << endl;
				return;
			}
		}
		// cout<<"there is stall "<<stall<<endl;

		// Stage 1 ----------------------------------------------------

		vector<string> command;
		if (current_PC < commands.size() && !stall)
		{ // push new command into pipeline
			// cout<<NUMBER_OF_CYCLES<<" "<<stall<<endl;
			command = commands[current_PC];
			if (INSTRUCTIONS.find(command[0]) == INSTRUCTIONS.end())
			{
				handleExit(SYNTAX_ERROR, NUMBER_OF_CYCLES);
				return;
			}

			LIST_OF_COMMANDS.push_back(current_PC);
			CURRENT_COMMANDS_IN_PIPELINE.push_back(command);
		}

		// register_PRINT(NUMBER_OF_CYCLES);

		if (CURRENT_COMMANDS_IN_PIPELINE.empty())
		{ // cycles are completed if no commmand left to execute.
			register_PRINT(NUMBER_OF_CYCLES);
			if (!storedword)
			{
				cout << "0" << endl;
			}
			else
			{
				cout << "1 " << storedaddress << " " << storedvalue << endl;
			}
			return;
		}
		// Stage 1 IF Stage -----------------------------------------------------
		if (current_PC < commands.size() && !stall)
		{
			L2.com = command;
			current_PC++;
		}

		// if(L5.com.size()>2 && L2.com.size()>2 && L3.com.size()>2){
		// 	cout<<"We are in cycle "<<NUMBER_OF_CYCLES<<" stall: "<<stall<<endl;
		// 	cout<<L5.com[0]<<" "<<L5.com[1]<<" "<<L5.com[2]<<endl;
		// 	cout<<L4.com[0]<<" "<<L4.com[1]<<" "<<L4.com[2]<<endl;
		// 	cout<<L3.com[0]<<" "<<L3.com[1]<<" "<<L3.com[2]<<endl;
		// }

		// if(NUMBER_OF_CYCLES>100){
		// 	return;
		// }

		// if(stall){
		// 	cout<<"stall"<<" in "<<NUMBER_OF_CYCLES <<" "<<L2.com[0]<<endl;
		// }
		// if(NUMBER_OF_CYCLES==4){
		// 	cout<<CURRENT_COMMANDS_IN_PIPELINE.size()<<endl;
		// }

		EXECUTE_THE_PIPELINE(NUMBER_OF_CYCLES, LIST_OF_COMMANDS, CURRENT_COMMANDS_IN_PIPELINE);
	}

	// print the register data in hexadecimal
	void register_PRINT(int clockCycle)
	{
		// cout << "Cycle number: " << clockCycle << '\n';
		for (int i = 0; i < 32; ++i)
			cout << REGISTERS[i] << ' ';
		cout << dec << '\n';
	}

	void clearLatches()
	{
		L2.com.clear();
		L3.com.clear();
		L4.com.clear();
		L5.com.clear();

		L2.REG_ONE = 0;
		L2.VALUE_ONE = 0;
		L2.REG_TWO = 0;
		L2.VALUE_TWO = 0;

		L3.REG_ONE = 0;
		L3.VALUE_ONE = 0;
		L3.REG_TWO = 0;
		L3.VALUE_TWO = 0;

		L4.REG_ONE = 0;
		L4.VALUE_ONE = 0;
		L4.REG_TWO = 0;
		L4.VALUE_TWO = 0;

		L5.REG_ONE = 0;
		L5.VALUE_ONE = 0;
		L5.REG_TWO = 0;
		L5.VALUE_TWO = 0;
	}
};

#endif