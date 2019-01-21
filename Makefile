# This Makefile was generated using MakeGen v1.0.8 made by Tim Håkansson
# and is licensed under MIT. Full source of the project can be found at
# https://github.com/Thraix/MakeGen
CC=@g++
CO=@g++ -o
BIN=bin/
OBJPATH=$(BIN)intermediates
INCLUDES=-I./src/ 
OBJECTS=$(OBJPATH)/main.o $(OBJPATH)/Mat3.o $(OBJPATH)/Mat4.o $(OBJPATH)/Quaternion.o $(OBJPATH)/Vec2.o $(OBJPATH)/Vec3.o $(OBJPATH)/Vec4.o 
CFLAGS=$(INCLUDES) -std=c++17 -c -w -g3 -D_DEBUG 
LIBDIR=
LDFLAGS=
LIBS=$(LIBDIR) -lvulkan -lglfw -lfreeimage -lfreetype 
OUTPUT=$(BIN)vulkan.x86_64
all: $(OUTPUT)
rebuild: clean all
clean:
	$(info Removing intermediates)
	rm -rf $(OBJPATH)/*.o
$(OUTPUT): $(OBJECTS)
	$(info Generating output file)
	$(CO) $(OUTPUT) $(OBJECTS) $(LDFLAGS) $(LIBS)
install: all
	$(info Installing Vulkan++ to /usr/bin/)
	@cp $(OUTPUT) /usr/bin/vulkan.x86_64
$(OBJPATH)/main.o : src/main.cpp src/Application.h src/math/Mat4.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h    
	$(info -[14%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Mat3.o : src/math/Mat3.cpp src/math/Mat3.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/MathFunc.h  
	$(info -[28%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Mat4.o : src/math/Mat4.cpp src/math/Mat4.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h    src/math/MathFunc.h  
	$(info -[42%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Quaternion.o : src/math/Quaternion.cpp src/math/MathFunc.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Quaternion.h 
	$(info -[57%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec2.o : src/math/Vec2.cpp src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h 
	$(info -[71%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec3.o : src/math/Vec3.cpp src/math/MathFunc.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Maths.h src/math/Mat3.h   src/math/Mat4.h     src/math/Quaternion.h     
	$(info -[85%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec4.o : src/math/Vec4.cpp src/math/Vec4.h
	$(info -[100%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
