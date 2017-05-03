------------------
-- EZWIX ENGINE --
------------------

Author: Guillem Travila

Repository: https://github.com/traguill/Ezwix-Engine

License: https://github.com/traguill/Ezwix-Engine/blob/master/LICENSE.txt

Demo video: https://www.youtube.com/watch?v=dHFYKukKWSc

HOW TO USE IT
----------------------------

Shaders
----------------------------
The last assignment for the engines class was to incorporate shaders in the engine. Here is how it works:

1- Drop the vertex program and/or fragment program to import it. There is a message in the console with the result of the compilation.
2- Go to Window/MaterialCreator
3- Write the path to save the material. Ex: /Assets/test.mat
(Must have .mat extension)
4- Choose the vertex and fragment program to use
5- Add all the uniforms that the shader is using. If it's a sampler2D select an imported texture or rendertexture.
6- Click create shader. The result message shows if it has been successful.
7- Select a gameobject with a mesh and material and change the material.

All objects uses the default shader if no material is specified. The default shader uses:
- Diffuse texture (if has)
- Normal map (if has)

Editor movement
------------------------------
- Move the camera with arrow keys
- Move the camera with the mouse wheel pressed
- Zoom in/out with the mouse wheel
- Left Alt + Right mouse orbit the camera (Needs improvement)

- Left Alt + Left Click to select an Object
- Left Ctrl + S to save a scene

Menu Options
------------------------------
File:
	- Create New Scene: removes the current scene and creates an empty one
Edit:
	- Camera: show the editor camera options. If the scene has at least one camera it's possible to switch the editor camera to one in the scene
	- Shaders: wip. The last input option shows a texture loaded in memory by it's id.
	- Lighting: ambient light configuration
	- Layers: create new layers or rename.
	- RenderTexture: configuration of a render texture

Windows:
	- Configuration: configuration of the editor windows
	- Console: Displays the console
	- Profiler: Displays the profiler.
	- Assets: it's open by default.
	- Resources: shows the amount of resources loaded in the GPU.
	- MaterialCreator: to create a new material.
Debug:
	- Show/Hide Octree: draws the boxes of the octree. 


Hierarchy
------------------------------
Right mouse click on the window to show the options:
- Create Empty GameObject
- Create Empty Child: inside the selected GameObject
- Remove selected GameObject

Inspector
------------------------------
- Click on the name to change it
- Right mouse click to add a new component

Note: all components need to have first a transform component attached.
Do NOT try to add a component to a GameObject which it's parent doesn't have a Transform component
Do NOT change the position of a static GameObject.

Play/Pause/Next
------------------------------
Press play to start the game simulation. Pause to Stop (and play again to continue). Press stop to end the simulation.

Note: Next button doesn't work yet.

Import New Files
------------------------------
Drag and drop files and/or folders to the application.

Accepted formats:

- Images/Textures: png and tga
- Meshes: fbx and obj

Assets Window
------------------------------
Left click on a folder/file to show the options.
It is possible to modify a texture or mesh and the editor automatically reimports the new files.