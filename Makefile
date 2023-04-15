all: sample
# sample:sample.cpp MIPS_Processor.hpp
# 	g++ sample.cpp MIPS_Processor.hpp -o sample

sample:sample.cpp submitpart1.hpp
	g++ sample.cpp submitpart1.hpp -o sample

clean:
	rm sample