all: gen dummy center agent test1 test2

gen:
	gcc -o gen generate_map.c

dummy:
	gcc -o dummy agent_dummy.c

center:
	gcc -o center agent_center.c

agent:
	gcc -o agent agent_two.c

test1:
	gcc -DTEST -o test1 agent_two.c

test2:
	gcc -DTEST -o test2 agent_dummy.c

clean:
	rm -f gen dummy center agent test1 test2
