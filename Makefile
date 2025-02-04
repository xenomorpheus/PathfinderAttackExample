
all: pathfinder_attack
	@./pathfinder_attack

logger.obj: logger.hpp logger.cpp
	g++ --std=c++23 -c logger.cpp -o logger.obj

pathfinder_attack.obj: logger.hpp pathfinder_attack.cpp
	g++ --std=c++23 -c pathfinder_attack.cpp -o pathfinder_attack.obj

pathfinder_attack: logger.obj pathfinder_attack.obj
	g++ --std=c++23 logger.obj pathfinder_attack.obj -o pathfinder_attack

clean:
	rm -f *.obj *test

clobber: clean
	rm -f pathfinder_attack
