all: sample
# sample:sample.cpp MIPS_Processor.hpp
# 	g++ sample.cpp MIPS_Processor.hpp -o sample

sample:sample.cpp 5stage.hpp
	g++ sample.cpp 5stage.hpp -o sample

clean:
	rm sample