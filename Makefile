CXXFLAGS = -g # -fsanitize=address
SOURCE = src
BUILD = build

MODULES = \
	abstract_syntax_tree \
	ast_to_ir \
	bytecode \
	bytecode_builder \
	compiler \
	intermediate_representation \
	main \
	virtual_machine \
	virtual_machine_value \

OBJECTS = $(MODULES:%=$(BUILD)/%.o)

bytecode: $(OBJECTS)
	$(CXX) $^ -o bytecode $(CXXFLAGS)

.PHONY: clean
clean:
	rm build -r
	rm bytecode

$(BUILD)/%.o: $(SOURCE)/%.cpp
	mkdir -p $$(dirname $@)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

include $(MODULES:%=$(BUILD)/%.d)

$(BUILD)/%.d: $(SOURCE)/%.cpp
	mkdir -p $$(dirname $@)
	echo "$(@:.d=.o): \\" > $@
	$(CXX) -MM -MG $< | sed 's/.*://' >> $@
