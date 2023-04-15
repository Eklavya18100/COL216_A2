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
	struct LATCH_BETWEEN_REGISTER
	{
		vector<string> COMAND = {};
		int REGISTER_ONE = 0;
		int VALUE_ONE = 0;
		int REGISTER_TWO = 0;
		int VALUE_TWO = 0;
	};
	int REGISTERS[32] = {0}, current_Program_Counter = 0, next_Program_Counter;						
	unordered_map<string, function<int(MIPS_Architecture &, string, string, string)>> instructions; 
	unordered_map<string, int> registerMap, address;												
	static const int MAX = (1 << 20);
	int data[MAX >> 2] = {0};
	vector<vector<string>> COMMANDS_TO_RUN;
	vector<int> commandCount;
	LATCH_BETWEEN_REGISTER L2, L3, L4, L5;
	bool STALL_CONTROL_SIGNAL = false;
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
		instructions = {{"add", &MIPS_Architecture::add}, {"sub", &MIPS_Architecture::sub}, {"mul", &MIPS_Architecture::mul}, {"beq", &MIPS_Architecture::beq}, {"bne", &MIPS_Architecture::bne}, {"slt", &MIPS_Architecture::slt}, {"jump", &MIPS_Architecture::jump}, {"lw", &MIPS_Architecture::lw}, {"sw", &MIPS_Architecture::sw}, {"addi", &MIPS_Architecture::addi}};

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
		commandCount.assign(COMMANDS_TO_RUN.size(), 0);
	}

	// perform add operation
	int findSum(int a,int b)
	{
		return a+b;
	}
	
	int add(string r1, string r2, string r3)
	{
		int sum=findSum(REGISTERS[registerMap[r2]], REGISTERS[registerMap[r3]]);
		return sum;
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
	int jump(string label, string unused1 = "", string unused2 = "")
	{
		if (!check_LABEL_FUNCTION(label))
			return 4;
		if (address.find(label) == address.end() || address[label] == -1)
			return 2;
		return address[label];
	}

	// perform load word operation
	int lw(string r, string location, string unused1 = "")
	{
		if (!check_REGISTER_FUNCTION(r) || registerMap[r] == 0)
			return 1;
		int address = locateAddress(location);
		if (address < 0)
			return abs(address);

		return address;
	}

	// perform store word operation
	int sw(string r, string location, string unused1 = "")
	{
		if (!check_REGISTER_FUNCTION(r))
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
		// cout<<"Here!!!!!!  "<<location<<"  "<<stoi(number)+REGISTERS[registerMap[dollarSign]]<<" "<<dollarSign<<endl;
		return (stoi(number) + REGISTERS[registerMap[dollarSign]]) / 4;
	}

	// perform add immediate operation
	int addi(string r1, string r2, string num)
	{
		if (!check_REGISTERS_FUNCTION({r1, r2}) || registerMap[r1] == 0)
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
	inline bool check_LABEL_FUNCTION(string str)
	{
		return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c)
														   { return (bool)isalnum(c); }) &&
			   instructions.find(str) == instructions.end();
	}

	// checks if the register is a valid one
	inline bool check_REGISTER_FUNCTION(string r)
	{
		return registerMap.find(r) != registerMap.end();
	}

	// checks if all of the REGISTERS are valid or not
	bool check_REGISTERS_FUNCTION(vector<string> regs)
	{
		return all_of(regs.begin(), regs.end(), [&](string r)
					  { return check_REGISTER_FUNCTION(r); });
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
			for (auto &s : COMMANDS_TO_RUN[current_Program_Counter])
				cerr << s << ' ';
			cerr << '\n';
		}
		cout << "\nFollowing are the non-zero data values:\n";
		for (int i = 0; i < MAX / 4; ++i)
			if (data[i] != 0)
				cout << 4 * i << '-' << 4 * i + 3 << hex << ": " << data[i] << '\n'
					 << dec;
		cout << "\nTotal number of cycles: " << cycleCount << '\n';
		cout << "Count of instructions executed:\n";
		for (int i = 0; i < (int)COMMANDS_TO_RUN.size(); ++i)
		{
			cout << commandCount[i] << " times:\t";
			for (auto &s : COMMANDS_TO_RUN[i])
				cout << s << ' ';
			cout << '\n';
		}
	}

	// parse the COmmand assuming correctly formatted MIPS instruction (or label)
	void parseCommand(string line)
	{
		// strip until before the comment begins
		line = line.substr(0, line.find('#'));
		vector<string> COmmand;
		boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
		for (auto &s : tokens)
			COmmand.push_back(s);
		// empty line or a comment only line
		if (COmmand.empty())
			return;
		else if (COmmand.size() == 1)
		{
			string label = COmmand[0].back() == ':' ? COmmand[0].substr(0, COmmand[0].size() - 1) : "?";
			if (address.find(label) == address.end())
				address[label] = COMMANDS_TO_RUN.size();
			else
				address[label] = -1;
			COmmand.clear();
		}
		else if (COmmand[0].back() == ':')
		{
			string label = COmmand[0].substr(0, COmmand[0].size() - 1);
			if (address.find(label) == address.end())
				address[label] = COMMANDS_TO_RUN.size();
			else
				address[label] = -1;
			COmmand = vector<string>(COmmand.begin() + 1, COmmand.end());
		}
		else if (COmmand[0].find(':') != string::npos)
		{
			int idx = COmmand[0].find(':');
			string label = COmmand[0].substr(0, idx);
			if (address.find(label) == address.end())
				address[label] = COMMANDS_TO_RUN.size();
			else
				address[label] = -1;
			COmmand[0] = COmmand[0].substr(idx + 1);
		}
		else if (COmmand[1][0] == ':')
		{
			if (address.find(COmmand[0]) == address.end())
				address[COmmand[0]] = COMMANDS_TO_RUN.size();
			else
				address[COmmand[0]] = -1;
			COmmand[1] = COmmand[1].substr(1);
			if (COmmand[1] == "")
				COmmand.erase(COmmand.begin(), COmmand.begin() + 2);
			else
				COmmand.erase(COmmand.begin(), COmmand.begin() + 1);
		}
		if (COmmand.empty())
			return;
		if (COmmand.size() > 4)
			for (int i = 4; i < (int)COmmand.size(); ++i)
				COmmand[3] += " " + COmmand[i];
		COmmand.resize(4);
		COMMANDS_TO_RUN.push_back(COmmand);
	}

	// construct the COMMANDS_TO_RUN vector from the input file
	void constructCommands(ifstream &file)
	{
		string line;
		while (getline(file, line))
			parseCommand(line);
		file.close();
	}

	void executeCommandsPipelined()
	{
		if (COMMANDS_TO_RUN.size() >= MAX / 4)
		{
			handleExit(MEMORY_ERROR, 0);
			return;
		}

		int clockCycles = 0;
		vector<int> COMMAND_FOR_CHECKING;
		vector<vector<string>> current_COMMANDS_IN_PIPELINE;
		EXECUTE_THE_PIPELINE(clockCycles, COMMAND_FOR_CHECKING, current_COMMANDS_IN_PIPELINE);
	}

	void EXECUTE_THE_PIPELINE(int &clockCycles, vector<int> &COMMAND_FOR_CHECKING, vector<vector<string>> &current_COMMANDS_IN_PIPELINE)
	{

		printRegisters(clockCycles);
		clockCycles++;
		bool store_the_word = false;
		bool branch = false;
		int store_the_address = 0;
		int store_the_value = 0;
		// ------------------------------------------------WB STAGE---------------------------------------------------------

		if (L5.COMAND.size() > 0)
		{

			if (L5.COMAND[0] == "add" || L5.COMAND[0] == "sub" || L5.COMAND[0] == "mul" || L5.COMAND[0] == "slt" || L5.COMAND[0] == "addi")
			{
				REGISTERS[L5.REGISTER_ONE] = L5.VALUE_ONE;
			}
			else if (L5.COMAND[0] == "beq" || L5.COMAND[0] == "bne" || L5.COMAND[0] == "jump")
			{
			}
			else if (L5.COMAND[0] == "sw")
			{
			}
			else if (L5.COMAND[0] == "lw")
			{
				REGISTERS[L5.VALUE_ONE] = data[L5.VALUE_TWO];
			}
			else
			{
				cout << "something wrong occured in stage5!!";
			}
		}

		if (current_COMMANDS_IN_PIPELINE.size() > 0 && L5.COMAND == current_COMMANDS_IN_PIPELINE[0])
		{
			current_COMMANDS_IN_PIPELINE.erase(current_COMMANDS_IN_PIPELINE.begin());
			COMMAND_FOR_CHECKING.erase(COMMAND_FOR_CHECKING.begin());
		}

		// -------------------------------------------------------ALU STAGE------------------------------------------------------

		if (L4.COMAND.size() > 0)
		{
			if (L4.COMAND[0] == "sw" && L5.COMAND != L4.COMAND)
			{
				L5.COMAND = L4.COMAND;
				L5.REGISTER_ONE = L4.REGISTER_ONE;
				L5.REGISTER_TWO = L4.REGISTER_TWO;
				L5.VALUE_ONE = L4.VALUE_ONE;
				L5.VALUE_TWO = L4.VALUE_TWO;
				store_the_word = true;
				data[L5.VALUE_TWO] = L5.VALUE_ONE; // storage done
				store_the_address = L5.VALUE_TWO;
				store_the_value = L5.VALUE_ONE;

				cout << "1 " << L5.VALUE_TWO << " " << L5.VALUE_ONE << endl;
			}
			else
			{
				L5.COMAND = L4.COMAND;
				L5.REGISTER_ONE = L4.REGISTER_ONE;
				L5.REGISTER_TWO = L4.REGISTER_TWO;
				L5.VALUE_ONE = L4.VALUE_ONE;
				L5.VALUE_TWO = L4.VALUE_TWO;
			}
		}
		if (!store_the_word)
		{
			cout << "0" << endl;
		}

		// -------------------------------------------------------------ALU STAGE---------------------------------------------------
		if (L3.COMAND.size() > 0)
		{
			if (L3.COMAND[0] == "add")
			{
				L4.COMAND = L3.COMAND;
				L4.REGISTER_ONE = registerMap[L3.COMAND[1]];
				L4.VALUE_ONE = L3.VALUE_ONE + L3.VALUE_TWO;
			}
			else if (L3.COMAND[0] == "sub")
			{
				L4.COMAND = L3.COMAND;
				L4.REGISTER_ONE = registerMap[L3.COMAND[1]];
				L4.VALUE_ONE = L3.VALUE_ONE - L3.VALUE_TWO;
			}
			else if (L3.COMAND[0] == "mul")
			{
				L4.COMAND = L3.COMAND;
				L4.REGISTER_ONE = registerMap[L3.COMAND[1]];
				L4.VALUE_ONE = L3.VALUE_ONE * L3.VALUE_TWO;
			}
			else if (L3.COMAND[0] == "slt")
			{
				L4.COMAND = L3.COMAND;
				L4.REGISTER_ONE = registerMap[L3.COMAND[1]];
				L4.VALUE_ONE = 0;
				if (L3.VALUE_ONE < L3.VALUE_TWO)
					L4.VALUE_ONE = 1;
			}

			else if (L3.COMAND[0] == "beq" || L3.COMAND[0] == "bne" || L3.COMAND[0] == "jump")
			{
				L4.COMAND = L3.COMAND;
				if (L3.COMAND[0] == "jump")
				{
				}
				else if (L3.COMAND[0] == "beq")
				{
					L4.VALUE_ONE = address[L3.COMAND[3]];

					STALL_CONTROL_SIGNAL = true;
					stall_UNTIL_CYCLE = clockCycles + 1;
					if (current_COMMANDS_IN_PIPELINE.size() > 0 && L2.COMAND == current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 1])
					{
						current_Program_Counter--;
						COMMAND_FOR_CHECKING.pop_back();
						current_COMMANDS_IN_PIPELINE.pop_back();
					}
					if (L3.VALUE_ONE == L3.VALUE_TWO)
					{
						current_Program_Counter = L4.VALUE_ONE;
					}

					L3.COMAND.clear();
					L2.COMAND.clear();
				}
				else if (L3.COMAND[0] == "bne")
				{
					L4.VALUE_ONE = address[L3.COMAND[3]];
					STALL_CONTROL_SIGNAL = true;
					stall_UNTIL_CYCLE = clockCycles + 1;
					if (current_COMMANDS_IN_PIPELINE.size() > 0 && L2.COMAND == current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 1])
					{
						current_Program_Counter--;
						COMMAND_FOR_CHECKING.pop_back();
						current_COMMANDS_IN_PIPELINE.pop_back();
					}
					if (L3.VALUE_ONE != L3.VALUE_TWO)
					{
						current_Program_Counter = L4.VALUE_ONE;
					}
					L3.COMAND.clear();
					L2.COMAND.clear();
				}
			}
			else if (L3.COMAND[0] == "sw")
			{
				L4.COMAND = L3.COMAND;
				L4.VALUE_TWO = L3.VALUE_TWO;	   // data address
				L4.VALUE_ONE = L3.VALUE_ONE;	   // register value
				L4.REGISTER_ONE = L3.REGISTER_ONE; // register number
			}
			else if (L3.COMAND[0] == "lw")
			{
				L4.COMAND = L3.COMAND;
				L4.VALUE_TWO = L3.VALUE_TWO;		   // data address value
				L4.VALUE_ONE = registerMap[L3.COMAND[1]]; // register number
				L4.REGISTER_ONE = L3.REGISTER_ONE;
			}
			else if (L3.COMAND[0] == "addi")
			{
				L4.COMAND = L3.COMAND;
				L4.REGISTER_ONE = L3.REGISTER_ONE;
				L4.VALUE_ONE = stoi(L3.COMAND[3]) + L3.VALUE_TWO;
			}
			else
			{
				cout << L3.COMAND[0] << endl;
				cout << "Some Error in ALU" << clockCycles << endl;
			}
		}

		// ---------------------------------------------------------------STALLS----------------------------------------------------

		if (STALL_CONTROL_SIGNAL && stall_UNTIL_CYCLE != clockCycles)
		{
		}

		else if (STALL_CONTROL_SIGNAL && stall_UNTIL_CYCLE == clockCycles)
		{
			STALL_CONTROL_SIGNAL = false;
		}

		else if (!STALL_CONTROL_SIGNAL && !L2.COMAND.empty())
		{

			if (L2.COMAND[0] == "add" || L2.COMAND[0] == "sub" || L2.COMAND[0] == "mul" || L2.COMAND[0] == "slt")
			{
				if (current_COMMANDS_IN_PIPELINE.size() == 2)
				{
					if (current_COMMANDS_IN_PIPELINE[0][0] == "add" || current_COMMANDS_IN_PIPELINE[0][0] == "sub" || current_COMMANDS_IN_PIPELINE[0][0] == "mul" || current_COMMANDS_IN_PIPELINE[0][0] == "slt" || current_COMMANDS_IN_PIPELINE[0][0] == "addi" || current_COMMANDS_IN_PIPELINE[0][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[0][1] == L2.COMAND[2] || current_COMMANDS_IN_PIPELINE[0][1] == L2.COMAND[3])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
				}
				else if (current_COMMANDS_IN_PIPELINE.size() > 2)
				{
					if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.COMAND[2] || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.COMAND[3])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
					if (!STALL_CONTROL_SIGNAL)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.COMAND[2] || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.COMAND[3])
							{
								STALL_CONTROL_SIGNAL = true;
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
						}
					}
				}
			}

			else if (L2.COMAND[0] == "beq" || L2.COMAND[0] == "bne")
			{
				if (current_COMMANDS_IN_PIPELINE.size() == 2)
				{
					if (current_COMMANDS_IN_PIPELINE[0][0] == "add" || current_COMMANDS_IN_PIPELINE[0][0] == "sub" || current_COMMANDS_IN_PIPELINE[0][0] == "mul" || current_COMMANDS_IN_PIPELINE[0][0] == "slt" || current_COMMANDS_IN_PIPELINE[0][0] == "addi" || current_COMMANDS_IN_PIPELINE[0][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[0][1] == L2.COMAND[1] || current_COMMANDS_IN_PIPELINE[0][1] == L2.COMAND[2])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
				}
				else if (current_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.COMAND[1] || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.COMAND[2])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
					if (!STALL_CONTROL_SIGNAL)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.COMAND[1] || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.COMAND[2])
							{
								STALL_CONTROL_SIGNAL = true;
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
						}
					}
				}
			}

			else if (L2.COMAND[0] == "lw")
			{									   // neither the address must have been changed earlier, nor the register used to check the address.
				size_t pos1 = L2.COMAND[2].find("("); // find the position of the opening parenthesis
				string res = "";
				if (pos1 != string::npos)
				{												 // if opening parenthesis is found
					size_t pos2 = L2.COMAND[2].find(")", pos1 + 1); // find the position of the closing parenthesis after the opening parenthesis
					if (pos2 != string::npos)
					{													   // if closing parenthesis is found
						res = L2.COMAND[2].substr(pos1 + 1, pos2 - pos1 - 1); // extract the substring between the parentheses
					}
				}

				// res is checked if it exists.

				if (current_COMMANDS_IN_PIPELINE.size() == 2)
				{
					if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
					{
						if (locateAddress(current_COMMANDS_IN_PIPELINE[0][2]) == locateAddress(L2.COMAND[2]))
						{
							STALL_CONTROL_SIGNAL = true;
							stall_UNTIL_CYCLE = clockCycles + 1;
						}
					}
				}
				else if (current_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
					{
						if (locateAddress(current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][2]) == locateAddress(L2.COMAND[2]))
						{
							STALL_CONTROL_SIGNAL = true;
							stall_UNTIL_CYCLE = clockCycles + 1;
						}
					}
					if (!STALL_CONTROL_SIGNAL)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sw")
						{
							if (locateAddress(current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][2]) == locateAddress(L2.COMAND[2]))
							{
								// STALL_CONTROL_SIGNAL=true;
								// stall_UNTIL_CYCLE=clockCycles+1;
							}
						}
					}
				}

				if (STALL_CONTROL_SIGNAL && stall_UNTIL_CYCLE == clockCycles + 2)
				{ /////????????????
				  // no need
				}
				else
				{
					if (current_COMMANDS_IN_PIPELINE.size() == 2)
					{
						if (current_COMMANDS_IN_PIPELINE[0][0] == "add" || current_COMMANDS_IN_PIPELINE[0][0] == "sub" || current_COMMANDS_IN_PIPELINE[0][0] == "mul" || current_COMMANDS_IN_PIPELINE[0][0] == "slt" || current_COMMANDS_IN_PIPELINE[0][0] == "addi" || current_COMMANDS_IN_PIPELINE[0][0] == "lw")
						{
							if (current_COMMANDS_IN_PIPELINE[0][1] == res)
							{
								STALL_CONTROL_SIGNAL = true;
								if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
								{
									stall_UNTIL_CYCLE = clockCycles + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = clockCycles + 2;
								}
							}
						}
					}
					else if (current_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == res)
							{
								STALL_CONTROL_SIGNAL = true;
								if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
								{
									stall_UNTIL_CYCLE = clockCycles + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = clockCycles + 2;
								}
							}
						}
						if (!STALL_CONTROL_SIGNAL)
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
							{
								if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == res)
								{
									STALL_CONTROL_SIGNAL = true;
									stall_UNTIL_CYCLE = clockCycles + 1;
								}
							}
						}
					}
				}
			}

			else if (L2.COMAND[0] == "sw")
			{ // cases are neither argument REGISTER_ONE should have been chenged earlier nor the res.

				size_t pos1 = L2.COMAND[2].find("("); // find the position of the opening parenthesis
				string res = "";
				if (pos1 != string::npos)
				{												 // if opening parenthesis is found
					size_t pos2 = L2.COMAND[2].find(")", pos1 + 1); // find the position of the closing parenthesis after the opening parenthesis
					if (pos2 != string::npos)
					{													   // if closing parenthesis is found
						res = L2.COMAND[2].substr(pos1 + 1, pos2 - pos1 - 1); // extract the substring between the parentheses
					}
				}

				if (current_COMMANDS_IN_PIPELINE.size() == 2)
				{

					if (current_COMMANDS_IN_PIPELINE[0][0] == "add" || current_COMMANDS_IN_PIPELINE[0][0] == "sub" || current_COMMANDS_IN_PIPELINE[0][0] == "mul" || current_COMMANDS_IN_PIPELINE[0][0] == "slt" || current_COMMANDS_IN_PIPELINE[0][0] == "addi" || current_COMMANDS_IN_PIPELINE[0][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[0][1] == L2.COMAND[1])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
				}
				else if (current_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{

						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.COMAND[1])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
					if (!STALL_CONTROL_SIGNAL)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.COMAND[1])
							{
								STALL_CONTROL_SIGNAL = true;
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
						}
					}
				}

				if (STALL_CONTROL_SIGNAL && stall_UNTIL_CYCLE == clockCycles + 2)
				{
					// no need
				}
				else
				{
					if (current_COMMANDS_IN_PIPELINE.size() == 2)
					{
						if (current_COMMANDS_IN_PIPELINE[0][0] == "add" || current_COMMANDS_IN_PIPELINE[0][0] == "sub" || current_COMMANDS_IN_PIPELINE[0][0] == "mul" || current_COMMANDS_IN_PIPELINE[0][0] == "slt" || current_COMMANDS_IN_PIPELINE[0][0] == "addi" || current_COMMANDS_IN_PIPELINE[0][0] == "lw")
						{
							if (current_COMMANDS_IN_PIPELINE[0][1] == res)
							{
								STALL_CONTROL_SIGNAL = true;
								if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
								{
									stall_UNTIL_CYCLE = clockCycles + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = clockCycles + 2;
								}
							}
						}
					}
					else if (current_COMMANDS_IN_PIPELINE.size() >= 3)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == res)
							{
								STALL_CONTROL_SIGNAL = true;
								if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
								{
									stall_UNTIL_CYCLE = clockCycles + 1;
								}
								else
								{
									stall_UNTIL_CYCLE = clockCycles + 2;
								}
							}
						}
						if (!STALL_CONTROL_SIGNAL)
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
							{
								if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == res)
								{
									STALL_CONTROL_SIGNAL = true;
									stall_UNTIL_CYCLE = clockCycles + 1;
								}
							}
						}
					}
				}
			}

			else if (L2.COMAND[0] == "addi")
			{ // addi
				if (current_COMMANDS_IN_PIPELINE.size() == 2)
				{
					if (current_COMMANDS_IN_PIPELINE[0][0] == "add" || current_COMMANDS_IN_PIPELINE[0][0] == "sub" || current_COMMANDS_IN_PIPELINE[0][0] == "mul" || current_COMMANDS_IN_PIPELINE[0][0] == "slt" || current_COMMANDS_IN_PIPELINE[0][0] == "addi" || current_COMMANDS_IN_PIPELINE[0][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[0][1] == L2.COMAND[2])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[0][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
				}
				else if (current_COMMANDS_IN_PIPELINE.size() >= 3)
				{
					if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "addi" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "lw")
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][1] == L2.COMAND[2])
						{
							STALL_CONTROL_SIGNAL = true;
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 2][0] == "sw")
							{
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
							else
							{
								stall_UNTIL_CYCLE = clockCycles + 2;
							}
						}
					}
					if (!STALL_CONTROL_SIGNAL)
					{
						if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "add" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "sub" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "mul" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "slt" || current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][0] == "addi")
						{
							if (current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 3][1] == L2.COMAND[2])
							{
								STALL_CONTROL_SIGNAL = true;
								stall_UNTIL_CYCLE = clockCycles + 1;
							}
						}
					}
				}
			}

			else if (L2.COMAND[0] == "jump")
			{ // addi
			  // no STALL_CONTROL_SIGNAL check required
			}

			else
			{
				cout << "something wrong happened with stalls.";
			}
		}

		// ------------------------------------------------------------ID STAGE----------------------------------------------------

		if (!STALL_CONTROL_SIGNAL)
		{
			if (L2.COMAND.size() > 0)
			{

				if (L2.COMAND[0] == "add" || L2.COMAND[0] == "sub" || L2.COMAND[0] == "mul" || L2.COMAND[0] == "slt")
				{
					L3.COMAND = L2.COMAND;
					L3.REGISTER_ONE = registerMap[L2.COMAND[2]];
					L3.REGISTER_TWO = registerMap[L2.COMAND[3]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = REGISTERS[L3.REGISTER_TWO];
				}

				else if (L2.COMAND[0] == "beq" || L2.COMAND[0] == "bne")
				{
					L3.COMAND = L2.COMAND;
					L3.REGISTER_ONE = registerMap[L2.COMAND[1]];
					L3.REGISTER_TWO = registerMap[L2.COMAND[2]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = REGISTERS[L3.REGISTER_TWO];
				}

				else if (L2.COMAND[0] == "jump")
				{
					L3.COMAND = L2.COMAND;
					L3.VALUE_ONE = address[L2.COMAND[1]];
					current_Program_Counter = L3.VALUE_ONE;
					STALL_CONTROL_SIGNAL = true;
					stall_UNTIL_CYCLE = clockCycles + 1;
					if (current_COMMANDS_IN_PIPELINE.size() > 0 && L2.COMAND == current_COMMANDS_IN_PIPELINE[current_COMMANDS_IN_PIPELINE.size() - 1])
					{
						COMMAND_FOR_CHECKING.pop_back();
						current_COMMANDS_IN_PIPELINE.pop_back();
					}
					L3.COMAND.clear();
					L2.COMAND.clear();
				}

				else if (L2.COMAND[0] == "sw" || L2.COMAND[0] == "lw")
				{
					string input = L2.COMAND[2];
					int pos1 = input.find("(");
					int pos2 = input.find(")");
					string number = input.substr(0, pos1);
					string dollarSign = "$" + input.substr(pos1 + 2, pos2 - pos1 - 2);
					int address = (stoi(number) + REGISTERS[registerMap[dollarSign]]) / 4;

					L3.COMAND = L2.COMAND;
					L3.REGISTER_ONE = registerMap[L2.COMAND[1]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = address;
				}

				else if (L2.COMAND[0] == "addi")
				{
					L3.COMAND = L2.COMAND;
					L3.REGISTER_ONE = registerMap[L2.COMAND[1]];
					L3.REGISTER_TWO = registerMap[L2.COMAND[2]];
					L3.VALUE_ONE = REGISTERS[L3.REGISTER_ONE];
					L3.VALUE_TWO = REGISTERS[L3.REGISTER_TWO];
				}

				else
				{
					cout << "Error!";
				}
			}
		}

		vector<string> COmmand;
		if (current_Program_Counter < COMMANDS_TO_RUN.size() && !STALL_CONTROL_SIGNAL)
		{
			COmmand = COMMANDS_TO_RUN[current_Program_Counter];
			if (instructions.find(COmmand[0]) == instructions.end())
			{
				handleExit(SYNTAX_ERROR, clockCycles);
				return;
			}

			COMMAND_FOR_CHECKING.push_back(current_Program_Counter);
			current_COMMANDS_IN_PIPELINE.push_back(COmmand);
		}

		if (current_COMMANDS_IN_PIPELINE.empty())
		{
			printRegisters(clockCycles);
			if (!store_the_word)
			{
				cout << "0" << endl;
			}
			else
			{
				cout << "1 " << store_the_address << " " << store_the_value << endl;
			}
			return;
		}

		// ----------------------------------------------IF STAGE----------------------------------------------------
		if (current_Program_Counter < COMMANDS_TO_RUN.size() && !STALL_CONTROL_SIGNAL)
		{
			L2.COMAND = COmmand;
			current_Program_Counter++;
		}

		EXECUTE_THE_PIPELINE(clockCycles, COMMAND_FOR_CHECKING, current_COMMANDS_IN_PIPELINE);
	}

	void printRegisters(int clockCycle)
	{

		for (int i = 0; i < 32; ++i)
			cout << REGISTERS[i] << ' ';
		cout << dec << '\n';
	}

};

#endif