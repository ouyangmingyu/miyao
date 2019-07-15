src = $(wildcard ./src/*.c)			
obj = $(patsubst ./src/%.c, ./obj/%.o, $(src))

target = app
inc_path = ./inc

ALL:$(target)

$(target):$(obj)
	gcc $^ -o $@

$(obj):./obj/%.o : ./src/%.c
	gcc -c $< -o $@ -I $(inc_path)

clean:
	-rm -rf $(obj) $(target)

.PHONY: clean ALL
