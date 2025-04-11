clean: 
	$(RM) -r bin/*
	
_gfx_test: clean
	gcc src/gfx_test.c src/ascui.c  src/raytiles.c src/common.c -Wall -Werror -g -lc -lm -lraylib -I /usr/local/include -o bin/gfx_test
gfx_test: _gfx_test
	./bin/gfx_test

vg_main: _gfx_test
	valgrind --tool=memcheck --leak-check=full --track-origins=yes ./bin/gfx_test
	
_unit_tests: clean
	gcc src/unit_tests.c src/ascui.c src/raytiles.c src/common.c Unity/unity.c -Wall -Werror -g -lc -lm -lraylib -I /usr/local/include -o bin/unit_tests
unit_tests: _unit_tests
	./bin/unit_tests
