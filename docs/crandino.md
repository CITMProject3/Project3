[HOME](index.md)
    
**Role:** Engine and Audio Programmer    

**Name:** Carlos Randino    
![Profile picture]()      
      
**Contact:**     
[Linkedin](https://www.linkedin.com/in/carlosrandino/)    
[GitHub](https://github.com/crandino)    
Mail: carlosmanbe@gmail.com    
    
**_My Job:_**  

At the very beginning I worked within the Engine department, adding and improving new funcionality on the first Engine states. Very soon, one of my main priorities was the whole Audio stuff (design and implementation), creating a coherent environment sound landscape trough the game using Wwise as a sound engine. 

**_What I've done:_**

- _Understanding Wwise sound engine:_ 
- _Sound designer:_
- _Audio programmer_: My job here was to manage
- _MONO implementation_: 
- _Event Queue_: for component loading
- _Brofiler implementation_: A very useful tool that was implemented to analyse and detect processes with high computational costs (bottlenecks) in order to optimize them and improve the framerate of our game.
- _More flexible use of Triggers:_ In order to add more flexibility on the trigger system within Bullet 3D, three types were added: on Trigger (called every frame that the trigger collides), on Enter (called only once on the first trigger collision) and on Exit (called only once when trigger does not collide anymore). 
- _Push car gameplay:_ Related with the driving gameplay once we decided to reject Bullet 3D as our physics car controller, one of the features that I've implemented was to create the Push system when the second player repeatedly pushed the corresponding button, highly increasing the car acceleration.
- _Activation of gameobjects_: Trying to imitate Unity behaviour, the activation or deactivation of any gameobject properly activates or deactivates the corresponding state to this gameobject but keeping all children gameobjects with their corresponding activation state.
- _Bugs, bugs, bugs..._: Buffer management for SkyBox, Physics problems
       
![ColliderImage](http://i.imgur.com/C9NEDJh.jpg)
