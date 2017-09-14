# topirateb

### C

The C code depends on `glfw3` (from repo) and `glad` (bundled).

Compile and run with:

```
gcc main.c -Iinclude -lglfw && ./a.out
```

### Odin

The Odin code depends on `odin-glfw` and `odin-gl`, put them in `Odin/shared`:

```
cd /path/to/Odin/shared
git clone https://github.com/vassvik/odin-glfw.git
git clone https://github.com/vassvik/odin-gl.git
```

and compile and run with 

```
/path/to/odin run main.odin
```
