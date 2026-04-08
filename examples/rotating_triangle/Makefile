TARGET = vkg_triangle

all: $(TARGET)

$(TARGET): $(TARGET).cpp vert_rotating.spv frag.spv
	g++ -std=c++17 -Wno-deprecated-declarations $< out.cpp -o $@ -lglfw -ldl -g

vert_rotating.spv: shader_rotating.vert
	glslc $< -o $@

frag.spv: shader.frag
	glslc $< -o $@

clean:
	rm -f $(TARGET) *.spv

.PHONY: clean all
