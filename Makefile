#OBJS specifies which files to compile as part of the project
OBJS = game/main.o game/game.o game/ui.o game/input.o game/bmp.o game/dungeon.o engine/glad.o engine/shader.o engine/random.o engine/renderer.o engine/importer.o engine/audio.o engine/dict.o engine/render_list.o engine/factory.o engine/debug.o engine/physics.o engine/skybox.o engine/animator.o engine/particle_generator.o engine/data/object.o engine/data/mesh.o engine/data/material.o engine/data/skeleton.o engine/data/frame.o engine/data/animation.o

#CC specifies which compiler we're using
CC = gcc -g -pg

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
#  COMPILER_FLAGS = -w
#
#  #LINKER_FLAGS specifies the libraries we're linking against

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = game/craft
LINKER_FLAGS = `pkg-config --static --libs openal freealut sdl2`

#This is the target that compiles our executable

$(OBJ_NAME): $(OBJS)
	$(CC) -o $@ $^ $(LINKER_FLAGS)

clean:
	rm -f $(OBJ_NAME) ./engine/*.o ./engine/data/*.o ./game/*.o
