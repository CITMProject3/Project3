[HOME](index.md)

**Role:** Programmer

**Name:** Ausiàs Dalmau Roig

**Contact:**

  [Linkedin](https://www.linkedin.com/in/ausias-dalmau-roig-005bb3a3/)
  
  [GitHub](https://github.com/auusi9)
  
  Mail: ausiasdalmauroig@gmail.com

**My Job:**

I started working in the engine fixing some bugs, improving some features and adding new ones. Since VS3, we needed to start developing a UI system. And that was my primary job. I developed all the UI system: rendering, components and behaviours. Once that was done, I started making the specific menus of the game using our scripting system.

Later when UI was more advanced than gameplay, I've been asked to help in that scrum to be able to reach our goals. So, I helped developing the Evil spirit behaviour, the hitodma  and improved the camera.

Finally, I returned to UI to finish the menus and add more cool functionality.

**What i’ve done:**

-  Texture changing on Inspector
    - With this feature you can change the texture of any mesh, in the inspector.

![texture changing](https://lh3.googleusercontent.com/rLVeXjP6KX7ygB9QFgHf4ciD1iP_42fq0Fde-1PSyIM4lJgjNjTV3NqUa4w4PpvdQTccJYKCd_nSem1bW4r5X-7XlFn-1ktQLrB0QssjqxD7Vzq6tcFDK-SRnTTV3AcvYIHRXpVhcLY)

-  UI System
    - Canvas component:
        - Canvas manages the UI of the scene.
        - Focus and organizes the rendering order.
    - Rect Component:
        - Handles the position of the UI element of the scene.
    - Image Component:
        - Texture.
        - Color.
        - Alpha/Blend.
        - Rendering function
    - Button Component:
        - Texture.
        - Color.
        - Alpha/Blend.
        - OnPress function that handles.
        - Rendering function
    - Simple Text Component
      - Rendering function
      - Text related functionality
    - Grid Component:
        - Grid organizes in a grid form several elements.

![UI EXAMPLE](https://lh3.google.com/u/0/d/0B8s5jSxw8e1td1JUdXhhc1hxSUk=w982-h909-iv1)

 -  Camera improvements:
    - Bullet physics made the car go crazy, and the camera was going crazy as well.
        - Fixed the camera to not go crazy, and to follow the car smoothly till he comes back to normal.
    - Smoothed a bit the camera when turning.
    
![CameraFixing]( https://lh4.googleusercontent.com/pmkUFnVXiy0MYSb94_biPmspuF9CWx144rrWgaKa5agIZj1tnCocrZFpIffBK3JTfJbB0tT3_K_Lo8REg-ciDkLMfqZkf0ZAz-M-3OyL8x2JJz0njwMPDGneXArpnC-6laEm)

 -  Evil spirit item:
    - Main features:
      - Reverse control.
      - Debuf to lower the max speed and the acceleration.
    - Second player defense:
      - Second player can remove the evil spirit earlier pressing LT and RT continuosly.
      
  - Hitodamas
    - Made that Hitodama increases the max speed.
    - Hitodama was removed from the game.
