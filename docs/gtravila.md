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

Because we started from the basic Engine I created for another class, I've spent almost all the development working on the engine and improving it's functionality. At some points, I've worked in the graphics part creating the shaders that were needed for the game.

**What I've done:**

During the project I've worked in the following features:

- [Resource Manager](#resource)
- [Prefabs](#prefabs)
- [Shaders](#shaders)
- [Particle system](#particles)
- [Shadows](#shadows)

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

<p align="center">
  <img src="http://i.imgur.com/0PtlJMj.png" alt="cell_shading"/>
</p>

In the end, it was decided to go with a more realistic style, so I changed the shaders to Phong. I did several variations of the shader: the classic one, with a normal map, with the mesh animated and with the mesh animated and a normal map.

![phong](http://i.imgur.com/Z1UNqze.png) ![phong_normal](http://i.imgur.com/xzq2xLq.png) ![animation](http://i.imgur.com/kcVmgJ3.gif)

The vertex deformation of a GameObject with animation was done through shaders to take advantage of the GPU and reduce the CPU computational cost.
Also, I was asked to do a shader for the water similar to the one in The Legend of Zelda: The Wind Waker. Here is the result:

<p align="center">
  <img src="http://i.imgur.com/gBMc1A2.gif" alt="water"/>
</p>

<a name="particles"></a> **Particle System**

I was responsible of making the particle system from scratch as well as implementing all the particles in the game. At the begining, I implemented the system all updated through the GPU, the particles position were updated in the GPU so the CPU just had to manage the creation. This method was discarted because sorting each particle position depending on the camera distance in the GPU was too complex. After that, I changed the system to update the positions through the CPU and I added all the functionality that the artist need it.

Some of the most interesting features of the system are:

- Shape of emission (cube or sphere)
- Texture animation
- Color over lifetime 
- An editor to simulate the states of the particle (Play/Pause/Stop)

<p align="center">
  <img src="http://i.imgur.com/PhJjLGN.gif" alt="particles"/>
</p>

![burst](http://i.imgur.com/RPQHfHk.gif)![particle_anim](http://i.imgur.com/BHQHyM1.gif)![particle_game](http://i.imgur.com/MIhIUaz.gif)

<a name="shadows"></a> **Shadows**

This feature was implemented at the end and due to time limitations the shadows are in low resolution and perform very poorly (but do their job!). The shadows are mapped in a 4096x4096 texture and udpated at real time for the whole map (very optimal...). However, I had time to improved it a little bit implementing the Percentage Close Filtering method.

![shadow_big](http://i.imgur.com/BjmRIOE.png)![shadow_small](http://i.imgur.com/GgixscC.png)
