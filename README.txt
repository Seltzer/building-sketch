How to use our code:

1.) Open Graphics\Prototype\BuildingSketch.sln
2.) Build in release mode
3.) Run

Controls are given in the implementation report.



All source files are located at the top level. Descriptions:


* main.cpp, BuildingSketch.h / cpp and Common.h
This is the backbone of our app featuring sketch/building rendering
methods and our UI.

Authors: David, Nathan, Sutirtha

* SymmetryDetection.h / inl / cpp (all symmetry work)

Author: Nathan

* BuildingGeneration.h / cpp
Contains the three building generation algorithms

Authors: David, Sutirtha

* Shader.h / Shader.cpp

Author: David

* displacement.frag / displacement.vert

Author: David
Note: Based on code from gamedev.net

* Poly.h 

Author: David

* Concave.h / cpp

Author: David
Based on code from flipcode.com

* HeightmapProcessing.h

Author: David

* DisplacementMapping.h/cpp

Author: Sutirtha

* Clip.h / cpp

Author: David
Based on: Graphics Gems text


* Types.h, vec.h, vec.inl, vecops.inl, vecspec.inl

These provide a utility vector class. David wrote these prior to this project.










