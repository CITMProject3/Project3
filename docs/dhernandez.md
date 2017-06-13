[HOME](index.md)
    
**Role:** Programmer    

**Name:** David Hernandez    
![Profile picture](http://i.imgur.com/cq5nlcU.jpg)      
      

**Contact:**     
[Linkedin](https://www.linkedin.com/in/davidhlazaro/)    
[GitHub](https://github.com/Vulpem)    
Mail: davidher96@gmail.com    
    
    
**_My Job:_**    
    
I started as the "Level Programmer", creating and fixing the different tools the level designers and artists were going to use, especially those regarding the terrain and the prop placement; as well as creating any features that were needed, such as the "Turbo Pads" or the checkpoint system.

Later on, I had to stop working on the terrain to get into gameplay, where i had to remove the "Bullet Raycast PhysCar" that was causing a lot of trouble and created a custom Kart from scratch, keeping Bullet3D only on the raycast and the collision detection, not the kart behavior.

From there on, i pretty much stuck working on the Kart, as well as still improving some tools for the level designers and, above all, cleaning bugs and putting out fires.
    
Everything i've done here has been in C++, using the libraries of ImGui for the Editor, MathGeoLib to handle matrix and other math, OpenGL to draw and Bullet for the collision detection.     

       
**_What i've done:_**

- Created component collider:
   - Component that can be added to any GameObject in order to make them exist in the physics world.
   - Three different shapes: Cube, Sphere and Convex shape, adaptable to the mesh.
   - Can be set to be a trigger, that will react to any collision.  
![ColliderImage](http://i.imgur.com/C9NEDJh.jpg)
   
- Terrain creation and editor:
   - Generate graphical and physical terrains from a Heightmap.
   - The terrain is subdivided and ordered in regular chunks.
   - Edit that terrain directly from the engine with sculpt tools.
   - Paint multiple textures onto the terrain.
   - Easily place GameObjects onto the terrain, following it's normals, with the GameObject Placer Tool.
   <a href="http://www.youtube.com/watch?feature=player_embedded&v=Aydv5b5T-4Y
" target="_blank"><img src="http://i.imgur.com/vJ4HBWv.jpg" 
alt="Terrain image" border="10" /></a>    
[Video showcasing the terrain editor tools](https://youtu.be/Aydv5b5T-4Y)    
     
- Kart behaviour:
   - Kart behaviour, using Bullet Raycast and custom physics.
   - Manage player input (except items).
   - Drifting, wall and kart collisions, enviroment interaction.
   - Turbo functionality and manager.
![KartImage](http://i.imgur.com/jICXB3B.jpg)

- Level components:
   - Created checkpoint system and "Out of Bounds" markers.
   - Created "Turbo Pads".
   - Managing karts position in the race (which one's first and which one's second).

- Engine: 
   - Fixed Object Raycast and implemented terrain Raycast.
   - More bugfix than i can count.
   
   
   
- And, in general:    

![Code](https://image.spreadshirtmedia.com/image-server/v1/compositions/110357305/views/1,width=300,height=300,appearanceId=2,backgroundColor=E8E8E8,version=1452254899/a-programmer-life-men-s-t-shirt-by-american-apparel.jpg)
