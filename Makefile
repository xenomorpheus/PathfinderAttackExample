
all: pathfinder_attack
	@./pathfinder_attack

pathfinder_attack: pathfinder_attack.cpp
	g++ --std=c++23 pathfinder_attack.cpp -o pathfinder_attack

clean:
	rm -f *.obj *test

clobber: clean
	rm -f pathfinder_attack
