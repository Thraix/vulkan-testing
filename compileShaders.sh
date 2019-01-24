for f in res/shaders/*.vert; do glslangValidator -V $f -o "$f.spv"; done
for f in res/shaders/*.frag; do glslangValidator -V $f -o "$f.spv"; done
