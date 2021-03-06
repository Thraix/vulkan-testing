# This Makefile was generated using MakeGen v1.3.0 made by Tim Håkansson
# and is licensed under MIT. Full source of the project can be found at
# https://github.com/Thraix/MakeGen
CC=@g++
CO=@g++ -o
MKDIR_P=mkdir -p
BIN=bin/
OBJPATH=$(BIN)intermediates
INCLUDES=-I./src/ 
OBJECTS=$(OBJPATH)/Device.o $(OBJPATH)/SwapChainHandler.o $(OBJPATH)/main.o $(OBJPATH)/Mat3.o $(OBJPATH)/Mat4.o $(OBJPATH)/Quaternion.o $(OBJPATH)/Vec2.o $(OBJPATH)/Vec3.o $(OBJPATH)/Vec4.o 
CFLAGS=$(INCLUDES) -std=c++17 -c -w -g3 -D_DEBUG 
LIBDIR=
LDFLAGS=
LIBS=$(LIBDIR) -lvulkan -lglfw -lfreeimage -lfreetype 
OUTPUT=$(BIN)vulkan.x86_64
.PHONY: all directories rebuild clean run
all: directories $(OUTPUT)
directories: $(BIN) $(OBJPATH)
$(BIN):
	$(info Creating output directories)
	@$(MKDIR_P) $(BIN)
$(OBJPATH):
	@$(MKDIR_P) $(OBJPATH)
run: all
	@./$(OUTPUT)
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
$(OBJPATH)/Device.o : src/Device.cpp src/Device.h src/SwapChainHandler.h src/ImageView.h  src/VulkanHandle.h   
	$(info -[11%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/SwapChainHandler.o : src/SwapChainHandler.cpp src/Device.h src/SwapChainHandler.h src/ImageView.h  src/VulkanHandle.h  
	$(info -[22%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/main.o : src/main.cpp src/Application.h src/Device.h src/ImageUtils.h src/ImageView.h  src/VulkanHandle.h  src/SwapChainHandler.h    src/math/Maths.h src/math/Mat3.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Mat4.h    src/math/MathFunc.h   src/math/Quaternion.h      
	$(info -[33%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Mat3.o : src/math/Mat3.cpp src/math/Mat3.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/MathFunc.h  
	$(info -[44%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Mat4.o : src/math/Mat4.cpp src/math/Mat4.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h    src/math/MathFunc.h  
	$(info -[55%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Quaternion.o : src/math/Quaternion.cpp src/math/MathFunc.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Quaternion.h 
	$(info -[66%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec2.o : src/math/Vec2.cpp src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h 
	$(info -[77%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec3.o : src/math/Vec3.cpp src/math/MathFunc.h src/math/Vec2.h src/math/Vec3.h src/math/Vec4.h   src/math/Maths.h src/math/Mat3.h   src/math/Mat4.h     src/math/Quaternion.h     
	$(info -[88%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
$(OBJPATH)/Vec4.o : src/math/Vec4.cpp src/math/Vec4.h
	$(info -[100%]- $<)
	$(CC) $(CFLAGS) -o $@ $<
