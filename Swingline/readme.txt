1. Select a folder for 3rd parties
2. Create LIBS environment variable pointing to that folder
3. Download and unpack there: 
   libepoxy-shared-x64.zip from https://github.com/anholt/libepoxy/releases
4. Download and unpack there: 
   https://github.com/glfw/glfw/releases/download/3.2.1/glfw-3.2.1.bin.WIN64.zip
5. Build the project in VS2017

6. Run options:
-v      - Show Voronoi cells
-n NNN  - The number of points/segments
-r R.R  - Stipple radius (in arbitrary units)
-h H.H  - This controls stipple elongation, 0 for circular stipple
-o FILE - Basename of SVG and TXT files to output
-w      - Constructs weighted Voronoi diagram (more weigth to points in light cells)

Examples:

a) round stipples of varying radius
   Swingline -v -n 1000 -r 0.5 -h 0 -o testout Swingline\pic\grace-mini.png

b) round stipples of fixed radius
   Swingline -w -v -n 1000 -r 0.5 -h 0 -o testout Swingline\pic\grace-mini.png

c) horizontal stipples of varying size
   Swingline -v -n 1000 -r 0.5 -h 0.025 -o testout Swingline\pic\grace-mini.png

d) horizontal stipples of fixed size
   Swingline -w -v -n 1000 -r 0.5 -h 0.025 -o testout Swingline\pic\grace-mini.png
