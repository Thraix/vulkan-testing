# This Makefile was generated using MakeGen v1.1.1 made by Tim Håkansson
# and is licensed under MIT. Full source of the project can be found at
# https://github.com/Thraix/MakeGen
CC=@g++
CO=@g++ -o
MKDIR_P=mkdir -p
BIN=bin/
OBJPATH=$(BIN)intermediates
INCLUDES=-I./src/ 
OBJECTS=$(OBJPATH)/SwapChainHandler.o $(OBJPATH)/main.o $(OBJPATH)/Mat3.o $(OBJPATH)/Mat4.o $(OBJPATH)/Quaternion.o $(OBJPATH)/Vec2.o $(OBJPATH)/Vec3.o $(OBJPATH)/Vec4.o 
CFLAGS=$(INCLUDES) -std=c++17 -c -w -g3 -D_DEBUG 
LIBDIR=
LDFLAGS=
LIBS=$(LIBDIR) -lvulkan -lglfw -lfreeimage -lfreetype 
OUTPUT=$(BIN)vulkan.x86_64
.PHONY: directories all rebuild clean
all: directories $(OUTPUT)
directories: $(BIN) $(OBJPATH)
$(BIN):
	$(info Creating output directories)
	@$(MKDIR_P) $(BIN)
$(OBJPATH):
	@$(MKDIR_P) $(OBJPATH)
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
$(OBJPATH)/SwapChainHandler.o : src/SwapChainHandler.cpp src/SwapChainHandler.h src/ImageView.h src/VulkanHandle.h 
	$(info -[12%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/main.o : src/main.cpp src/Application.h src/ImageUtils.h src/ImageView.h src/VulkanHandle.h src/SwapChainHandler.h    src/math/Maths.h src/math/Mat3.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Mat4.h    src/math/MathFunc.h   src/math/Quaternion.h      
	$(info -[25%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Mat3.o : src/math/Mat3.cpp src/math/Mat3.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/MathFunc.h  
	$(info -[37%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Mat4.o : src/math/Mat4.cpp src/math/Mat4.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h    src/math/MathFunc.h  
	$(info -[50%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Quaternion.o : src/math/Quaternion.cpp src/math/MathFunc.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Quaternion.h 
	$(info -[62%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec2.o : src/math/Vec2.cpp src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h 
	$(info -[75%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec3.o : src/math/Vec3.cpp src/math/MathFunc.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Maths.h src/math/Mat3.h   src/math/Mat4.h     src/math/Quaternion.h     
	$(info -[87%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec4.o : src/math/Vec4.cpp src/math/Vec4.h
	$(info -[100%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
