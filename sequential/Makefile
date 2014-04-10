CFLAGS = -O0
OBJECTS = game_of_life_seq.o
test_seq : $(OBJECTS)
	gcc $(CFLAGS) -o $@ $^

game_of_life_seq.o : game_of_life_seq.c
	gcc $(CFLAGS) -c $<

clean:
	rm -rf test_seq $(OBJECTS)
