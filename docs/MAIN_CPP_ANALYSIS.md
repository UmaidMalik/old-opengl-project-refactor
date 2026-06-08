# main.cpp Analysis

## Program Initialization
- GLFW initialization
- OpenGL context creation
- GLAD loading
- callback registration
- OpenGL state configuration

## Global State
- camera position and orientation
- timing values
- model transforms
- projection mode
- Rubik's cube state
- lighting and shadow state

## Input Handling
- keyboard callback
- mouse callback
- scroll callback
- per-frame key polling

## Rendering
- shader setup
- texture setup
- geometry buffer creation
- shadow pass
- main scene pass
- skybox rendering
- Rubik's cube rendering
- timer rendering

## Helper Functions
- list each major function and its purpose

## Resource Lifetime
- shaders
- textures
- VAOs
- VBOs
- framebuffers
- OpenGL program deletion

## Known Problems
- incorrect Rubik's cube behavior
- global mutable state
- repeated rendering code
- manual OpenGL cleanup
- weak asset-loading errors