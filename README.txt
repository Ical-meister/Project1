1221304904_Wisyal

Prerequisites:

Windows: Visual Studio 2019/2022 with C++ and OpenGL development libraries installed

------------------------------------------------------------------------------------------------------

Compilation Instructions:
Windows (Visual Studio)

Open Project1.sln in Visual Studio.

Ensure that GLEW, GLM and GLFW include/library directories are correctly set in project properties:

C/C++ → General → Additional Include Directories

Linker → General → Additional Library Directories

Build the project using Ctrl + Shift + B or Build → Build Solution.

The executable will be located in 1221304904_Assignment\Project1\Project1.exe

------------------------------------------------------------------------------------------------------

Running Instructions:

Ensure your assets folder (models, textures, shaders) is in the same directory as the executable. (this should already
be done unless OneDrive messed up something)

Launch the program:

Windows: Double-click Project1.exe or run via Visual Studio.

Controls:

W/A/S/D: Move the camera forward/left/back/right
Spacebar: Ascend
Shift: Descend
O(not zero, its the letter O): Cycle through camera-modes.
P/L: In planet close-up mode, use P or L to cycle up/down through the planets.

Mouse: Look around

Esc: Exit the program

Notes:
Ensure your GPU drivers support OpenGL 3.3 or higher.

If the program shows a black screen, check that the shader files are correctly located and paths are valid.

All transformations, animations, and lighting calculations are handled in the respective shaders and main loop.
