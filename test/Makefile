name=king_test

options = -g -Wall -Wextra -pedantic -std=c++14

include_path=-IMessageStore/
sources=$(wildcard MessageStore/*.cpp)

default: $(name)
	
$(name): $(sources) 
	rm -rf bin && mkdir bin && g++ $(options) $(include_path) -o bin/$@ $^

 