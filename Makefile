clean:
	$(RM) -r bin/*
	$(RM) -r *.out
	$(RM) -r *.output

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

_ph_test: clean
	gcc src/phrender_test.c  src/common.c -Wall -g -lc -lm -lraylib -I /usr/local/include -o bin/ph_test
ph_test: _ph_test
	./bin/ph_test 10

ph_test_gprof: clean
	gcc src/phrender_test.c  src/common.c -fno-inline -Wall -pg -O3 -lc -lm -lraylib -I /usr/local/include -o bin/ph_test
	./bin/ph_test 1e6
	gprof bin/ph_test gmon.out > ph_gprof.output
