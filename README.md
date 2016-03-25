# COMP371_whsd

We have selected the walk through of a procedurally modelled world as the topic of our OpenGL team project. The title of the application will be “Minecraft Reimagined”, which will be a recreation inspired by the popular game Minecraft. The open world from our application will be significantly smaller than the actual game as our resources are limited. Currently, our scope is to include a small village, skybox, tree, water, and different materials into the open world. The camera will be from the perspective of a virtual character controlled by the user.

Application Goal
The goal of the application is to recreate the Minecraft game model by implementing OpenGL technologies and applying Computer Graphics concepts. The application aims to allow the user to explore a world using the keyboard and the mouse as input devices. As the virtual world is inspired by Minecraft, the graphic will not be realistic. Instead, the world will be represented by using only cubes, which will have different sizes and textures. Currently, our scope is as follows:

Virtual Character: The virtual character will be our camera in the virtual world. Therefore, as the user manipulates the virtual character, he is also manipulating the view. Controls will be inspired by first person shooter video games. In first person mode, the hands of the character will be displayed within the view. Third person mode is also included. While in third person mode, the camera will be placed slightly behind and above the user, but pointing at where the character is facing. The movement control of the character and the interaction scheme for view manipulation will be the following:

Keyboard:
- W Move Forward
- S Move Backward
- A Strafe Left
- D Strafe Right
- C Toggle First Person / Third Person

Mouse:
- Cursor Adjust the viewing angle
- Left Click Interaction (additional feature)

Dirt, Grass, Stone, Wood and Various Different Materials: These items will be the foundation of the
ground. They will both be medium sized cubes with their corresponding texture. The user will be able to
stand on top of these blocks. An additional feature would be the ability for the user to interact with the
block, such as destroying or relocating a cube at a desired location.
- Water: The water will be a medium size cube with the corresponding water texture. This kind of block will not allow the user to stand on top of it, instead, the user will be able to swim within a water block.
- Trees: Trees will be created by a series of cubes of different sizes. The trunk will have a brown texture consisting in series of medium sized cubes placed a top of each other vertically. The leaves will be a series of small cubes that are widespread with green texture. The leaves will be located at the top of the tree trunk.
- Skybox: This will be the sky of the virtual world. An additional feature would be to change the skybox after a certain amount of time. This feature will not necessarily be a dynamic daytime cycle but a static daytime cycle. 
- Village: The village will consist of different building structure created by using different medium sized cube with different texture, such as wood and stone. The user is able to interact with certain parts of the structure, for example, opening and closing a door. 
- Light Source, Shadows: We will only be having a single light source in the virtual world, which is the sun, or the moon if there is a daytime cycle. Shadows will be present and generated accordingly.

Asset Creation Method

- Create the base square with a set dimension which consists of smaller cubes
- Translate each vertex on the square to create the 3D cube
- Create different objects as classes with some parameters to set the size using the cubes we have created previously
            
            createTreeObject(..., ..., ...)

            createGrassObject(..., ..., ...)

- Each cube will be able to hold a certain texture/model for grass/leaves, the cubes could be green for now for trees, the branches are brown, and we could use the leaves specified above
- Use the objects to recreate the terrain procedurally to hold all our objects 

            createMap(...., …., ….)

            createForest(..., ..., …,)

Expected Challenges
- Finding textures and models if we do not want all objects to only have a solid color 
- Preventing the user from passing through solid objects Creating the world procedurally
- Skybox implementation
- Performance optimization

Additional Comments
As an open world contains numerous objects, performance issues will be carefully looked at when drawing large amounts of items on the screen. We believe that creating an open world may prove difficult as we have no prior experience nor knowledge. A lot of research will be done during the following weeks. Our main priority will be to ensure that our core features are functional and that they are fault free. Additional features listed among others that aren’t listed will be implemented if time permits.

