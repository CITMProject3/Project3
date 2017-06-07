[HOME](index.md)

[ABOUT US](about_us.md)

![Profile picture](http://i.imgur.com/DtNambh.png)     

**Name:** Guillem Travila.

**Role:** Programmer.

**Contact:**

[LinkedIn](www.linkedin.com/in/traguill)

[GitHub](https://github.com/traguill)

Mail: traguill1@gmail.com

**My Job:** 

I spent the whole project working on the engine. 

**What I've done:**

- [Resource Manager](#resource)
- [Prefabs](#prefabs)
- [Shaders](#shaders)
- Particle system
- Shadowmaps



<a name="resource"></a> **Resource Manager**
I had to add more functionality to the existing resource manager to be able to work in group. At the end, the resource manager was able to:
- Detect the new assets and import them
- Detect the modified assets and reimport them
- Import the assets dropped in the engine at runtime

The major difficulty was mantaining a version control of all the assets in different computers. 

<a name="prefabs"></a> **Prefabs**
This was another feature required to work fluently in a large group. The prefab system is able to add and remove new GameObjects in the prefab hierarchy as well as changing any value of any component of any GameObject of the prefab hierarchy and apply the changes at runtime.

<a name="shaders"></a> **Shaders**
At the begining, the team decided to go with a cell shading style. 
(insert cell shading photo)
In the end, it was decided to go with a more realistic style, so I changed the shaders to Phong. I did several variations of the shader: the classic one, with a normal map, with the mesh animated and with the mesh animated and a normal map.
(insert phong photo, phong with normal, animation)
The vertex deformation of a GameObject with animation was done through shaders to take advantage of the GPU and reduce the CPU computational cost.
Also, I was asked to do a shader for the water similar to the one in The Legend of Zelda: The Wind Waker. Here is the result:
(insert water gif)
