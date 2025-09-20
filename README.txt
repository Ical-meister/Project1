1221304904_Wisyal

Prerequisites:

Windows: Visual Studio 2019/2022 with C++ and OpenGL development libraries installed

------------------------------------------------------------------------------------------------------

Compilation Instructions:
Windows (Visual Studio)

Open Project1.sln in Visual Studio.

Ensure that assimp, GLEW, GLM and GLFW include/library directories are correctly set in project properties:

C/C++ → General → Additional Include Directories

Linker → General → Additional Library Directories

Build the project using Ctrl + Shift + B or Build → Build Solution.

The executable will be located in the build folder.

------------------------------------------------------------------------------------------------------

Running Instructions:

Ensure your assets folder (models, textures, shaders) is in the same directory as the executable. (we'll do this at the end of the assignment)

For now, just run the program through Visual Studio otherwise the assets won't load.

Launch the program:

Windows: Double-click Project1.exe or run via Visual Studio.

Controls:

Esc: Exit the program

Notes:
Ensure your GPU drivers support OpenGL 3.3 or higher.

If the program shows a black screen, check that the shader files are correctly located and paths are valid.

All transformations, animations, and lighting calculations are handled in the respective shaders and main loop.
