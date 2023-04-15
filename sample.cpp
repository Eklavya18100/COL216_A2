// #include "MIPS_Processor.hpp"
#include "submitpart1.hpp"
using namespace std;

int main(int argc, char *argv[])
{	
	if (argc != 2)
	{
		cerr << "Required argument: file_name\n./MIPS_interpreter <file name>\n";
		return 0;
	}
	ifstream file(argv[1]);
	MIPS_Architecture *mips;
	if (file.is_open())
		mips = new MIPS_Architecture(file);
	else
	{
		cerr << "File could not be opened. Terminating...\n";
		return 0;
	}
	
	mips->executeCommandsPipelined();
	return 0;
}