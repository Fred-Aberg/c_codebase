base = common.c raytiles.c ascui.c  

clean:
	rm -f *.o main
build:
	clean
demo_ms:
	gcc main.c minesweeper.c common.c raytiles.c -g -lc -lm -lraylib -I /usr/local/include -o demo_ms.o
demo_ui:
	gcc ascui_test.c common.c raytiles.c ascui.c -g -lc -lm -lraylib -I /usr/local/include -o demo_ui.o

# mem: 
# 	valgrind --leak-check=full ./output
