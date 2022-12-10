all: say_hello

#----------------------------------------------------------

reg_cat: reg_cat.c
	$(CC) $(^) -o $(@)

rt_cat: rt_cat.c 
	$(CC) $(^) -o $(@)

#----------------------------------------------------------

.PHONY: clean say_hello

say_hello:
	@ echo "Please. specify target."

clean:
	- rm rt_cat
	- rm reg_cat
