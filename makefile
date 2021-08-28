
cc = g++
prom= main
obj = main.o

$(prom):$(obj) 
	$(cc) -o ./bin/$(prom) $(obj) --std=c++11 -pthread 
	rm -f ./*.o

main.o : main.cpp
	$(cc) -c main.cpp

clean: 
	rm -f ./*.o
	rm -f *.o
