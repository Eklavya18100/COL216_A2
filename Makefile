all: sample
# sample:sample.cpp MIPS_Processor.hpp
# 	g++ sample.cpp MIPS_Processor.hpp -o sample

sample1:sample.cpp submitpart1.hpp
	/opt/homebrew/bin/g++-12 sample.cpp submitpart1.hpp -I /opt/homebrew/Cellar/boost/1.81.0_1/include -o sample1

sample2:sample.cpp submitpart2.hpp
	/opt/homebrew/bin/g++-12 sample.cpp submitpart2.hpp -I /opt/homebrew/Cellar/boost/1.81.0_1/include -o sample2
clean:
	rm sample
