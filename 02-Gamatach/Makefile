all: gamatch rand_agent greedy_agent agent_blue

gamatch:
	gcc -o gamatch gamatch.c

rand_agent:
	gcc -o rand_agent agents/rand_agent.c

greedy_agent:
	gcc -o greedy_agent agents/greedy_agent.c

agent_blue:
	gcc -o agent_blue agents/agent_blue.c

clean:
	rm gamatch *_agent agent_blue
