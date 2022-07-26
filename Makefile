all:: build

# gcc -v -E -x c++ -

Compiler := g++

Program_name := executable.exe
Program_name_opt := executable_opt.exe

#Header_files := $(wildcard *.hpp)
Source_files := $(wildcard *.cpp)
Object_files := $(patsubst %.cpp,%.o,$(Source_files))

Dependency := $(patsubst %.cpp,%.d,$(Source_files))


Supressing_flags := #-Wno-unused-value -Wno-error=unused-value -Wno-unused-parameter
Sanitizer_flags  := #-fsanitize=undefined,address,leak
Optimizing_flags_compile := -O3 -flto -fuse-linker-plugin -ffat-lto-objects
Optimizing_flags_link := -O3 -flto -fuse-linker-plugin
Warning_flags    := -pedantic -Wall -Wextra -g #-Werror
Flags := -std=c++20 $(Warning_flags) $(Sanitizer_flags) $(Supressing_flags)
Dependency_flags := -MMD -MP

$(Program_name): $(Object_files)
	$(Compiler) $(Flags) $(Optimizing_flags_link) $^ -o $@

$(Program_name_opt): $(Source_files) Makefile
	$(Compiler) $(Flags) $(Source_files) $(Optimizing_flags_compile) $(Optimizing_flags_link) -o $@

run:: $(Program_name)
	./$(Program_name)

build:: $(Program_name)

run_opt:: $(Program_name_opt)
	./$(Program_name_opt)

build_opt:: $(Program_name_opt)

-include $(Dependency)

%.o: %.cpp Makefile
	$(Compiler) $(Flags) $(Optimizing_flags_compile) $(Dependency_flags) -c $<

clean::
	rm -f *.s
	rm -f *.o
	rm -f *.d
	rm -f *.gch
	rm -f -r $(Program_name_opt).dSYM
	rm -f $(Program_name)
	rm -f $(Program_name_opt)

rerun:: clean run

rebuild:: clean build
