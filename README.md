# cornelis
cornelis is a small path tracer with a focus on rendering implicit surfaces. It is very early in conception.

## Goals
cornelis is not destined to be anything very useful. It is a learning opportunity and as such has some arguably strange trade-offs. For example I have yet to decide if I'll ever put in support for triangle geometry.

Current plan is for cornelis to only feature two shaders, some sort of "standard material" as it is commonly referred to in other renderers and a fog volume shader. 

### Milestone 1
 - [x] Raytraced spheres
 - [x] Render loop
 - [x] Image saving
 - [x] Thread local RNG System
 - [x] AA

### Milestone 2
 - [ ] Basic diffuse & glossy specular mix "standard material"
 - [ ] Monte Carlo path tracing

### Milestone 3
 - [ ] Sample point generators
 - [ ] Quasi Monte Carlo via Sobol sequences or similar
 - [ ] Gaussian Filtering via Filter Importance Sampling

### Milestone 4
 - [ ] Next event estimation (fancy name for direct light sampling)
 - [ ] Progressive mode (sample for x seconds)
 - [ ] OpenEXR output
 - [ ] Level Set surface rendering via NanoVDB
  
### Milestone 5
 - [ ] Fog volumes

### Milestone 6
 - [ ] Importance Sampling (for the material)
 - [ ] Spectral rendering (i.e not just sRGB)
 - [ ] Area lights

### Other wishes
 - [ ] MIS: Multiple Importance Sampling (material and lights)
 - [ ] Variance diagnostics
 - [ ] GPU support
 - [ ] Distorsion support in camera
 - [ ] Realistic lens camera model
 - [ ] Camera motion blur

## Scene description language
Current plan is to provide Python bindings, and then you set up and render from Python.

## Development
cornelis is developed in C++, targeting C++17. Build system is CMake. External dependencies are:
 - CMake
 - TBB
 - OpenEXR (maybe in the future)

There is unit testing, but some aspects of ray tracing is less than joyful to unit test. Thus, some features may not be tested at all, like the shaders. It is also unclear sometimes what a "correct" output is, making it necessary to unit test with the Human Eyeball.

## Design philosophy
*Note: this is just some nonsense I put here for my own sake, to remember me of my duties*

cornelis' aim is to be like a small library, with a command line tool attached to it. 

For the code in general
- Small, simple types with few methods.
- Mutability only for performance or extreme convenience
- Prefer free functions
- Bundle many parameters into a struct
- RAII is cool and your friend
- Strive for no manual allocations, and if possible ownership is always managed by a smart pointer

## Name & License
It is named after Cornelis Vreeswijk. 

License is MIT, except for "external" files that are grandfathered in.
