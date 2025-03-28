all: agent1 agent2 arena

agent1:
	gcc -DTIMEOUT -o agent1 jh-agent.c

agent2:
	gcc -o agent2 agent1.c

arena:
	gcc -o arena arena.c

clean:
	rm agent1 agent2 arena

