# OpenGL Letter Scene

Explore five 3D letter-and-number models and a playable 3x3 picture cube. The scene runs with OpenGL 3.3 on desktop and WebGL 2 in a browser.

## Desktop

Use CMake 3.20 or newer, a C++17 compiler, and GLFW 3.3 or newer. The x64 Visual Studio generator can use the repository's bundled GLFW library.

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
cmake --build build --config Release --target run
```

## Browser

[Play the live WebAssembly build](https://umaidmalik.github.io/old-opengl-project-refactor/). GitHub Actions builds the browser target for pull requests and deploys it to GitHub Pages on pushes to `main`.

### Local build

Activate the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html) in your shell. With Ninja and Python available, run the following commands.

```sh
emcmake cmake -S . -B build-web -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-web
python -m http.server 8000 --directory build-web
```

Open [the scene on localhost](http://localhost:8000/opengl_project.html). The browser build generates HTML, JavaScript, WebAssembly, and a data file containing the scene assets. Keep the four files together when serving the build.

## Controls

| Input | Action |
| --- | --- |
| `1` to `5` | Select A7, O9, S0, M6, or N7 |
| `6` | Select the picture cube |
| `Q W E` / `A S D` | Turn the three X slices in opposite directions |
| `R T Y` / `F G H` | Turn the three Y slices in opposite directions |
| `U I O` / `J K L` | Turn the three Z slices in opposite directions |
| Arrow keys | Rotate the scene |
| Right mouse + move | Look around. Hold `W A S D`, Space, or `Z` to move, or scroll to zoom |
| `W A S D`, Space, `Z` | Move the selected letter |
| `U` / `J` | Scale the selected letter |
| `T` / `Y`, `G` / `H`, `B` / `N` | Rotate the selected letter around X, Y, or Z |
| Tab / Shift | Toggle stepped letter movement / speed up continuous movement |
| Left Alt | Randomize the selected letter's horizontal position |
| `R`, Shift+`R`, Ctrl+Shift+`R` | Reset the selected letter's position, size, or rotation |
| `Home`, `0` | Reset the camera and scene orientation, or reset the timer |
| `X`, `C` | Toggle letter and ground textures, or disable face culling while held |
| `E` / `V` | Perspective / orthographic view. `E` turns a slice when the cube is selected |
| `B`, `N`, `M` | Fill, wireframe, or point rendering on desktop |
