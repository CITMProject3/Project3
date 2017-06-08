[HOME](index.md)
    
**Name:** Carlos Randino    
<img src="http://i.imgur.com/Is4iFzU.jpg?1" height="200" width="200">
    
**Role:** Engine and Audio Programmer   
      
**Contact:**     
[Linkedin](https://www.linkedin.com/in/carlosrandino/)    
[GitHub](https://github.com/crandino)    
Mail: carlosmanbe@gmail.com    
    
**_My Job:_**  

At the very beginning I worked within the Engine department, adding and improving new funcionality on the early Engine states. Very soon, one of my main priorities was the whole Audio stuff (design, creation and implementation), building a coherent environment sound landscape over the game using [Wwise](https://www.audiokinetic.com/products/wwise/) as a sound engine. 

**_What I've done:_**

- _Understanding Wwise sound engine:_ Requiered knowledge acquiered for audio sources, hierarchy, events, listeners, attenuation, soundbanks, format conversions, buses and RTPC values to maximize the use of Wwise tool and to adapt all the offered functionality within our engine.

![Wwise_Hierarchy](http://i.imgur.com/DL5Rlj2.png)

- _Sound designer:_ Design and creation of mainly all sound effects shown in game using own recorded sounds, [FreeSound](https://www.freesound.org/) sound database and Audacity as an edition tool. 

![RTCP_on_Engine](http://i.imgur.com/BcuOBcp.png)

- _Audio programmer_: My job here was divided into different tasks:
    - Wwise implementation: Wwise library was implemented on our engine (Debug, Profile and Release versions), initiating and terminating all modules when needed. That was needed to work with the Wwise authoring tool and to communicate all audio callback information inside the engine with the external Wwise profiler.
    - Extract soundbank information: Wwise event information within soundbanks was extracted and classified to show it in engine on the correspondings components. 
    - Soundbank management: Soundbanks were loaded only when requiered, so memory wasn't wasted.
    - Create Audio Listener component: Up to 8 audio listeners can be added on the engine, and audio events can be filtered to only use the selected ones.
     - Create AudioSource component: This component contains the Wwise events. An infinite number of them can be added and tested with the Play/Stop buttons.        
    <img src="http://i.imgur.com/Zfjr48B.png" >
- _MONO scripting libray_: Somewhere during the project, a scripting module started to be a must. So, two different approaches were taken (C++/C#). I implemented the [C# Mono Scripting Library](http://www.mono-project.com/) version on our engine, useful to create C# scripts to control all gameplay and gameflow details.
- _Event Queue_: When loading our scenes, components with other gameobject references where not correctly loaded due to the sequencial loading process, trying to link the referenced ones when they wasn't loaded yet. So, an event queue was created to decouple loading and linking processes.
- _[Brofiler](http://brofiler.com/) implementation_: A very useful tool that was implemented to analyse and detect processes with high computational costs (bottlenecks) in order to optimize them and improve the framerate of our game.
- _More flexible use of Triggers:_ In order to add more flexibility on the trigger system within Bullet 3D, three types were added: on Trigger (called every frame that the trigger collides), on Enter (called only once on the first trigger collision) and on Exit (called only once when trigger does not collide anymore). 
- _Push car gameplay:_ Related with the driving gameplay once we decided to reject Bullet 3D as our physics car controller, one of the features that I've implemented was to create the Push system when the second player repeatedly pushed the corresponding button, highly increasing the car acceleration.
- _Activation of gameobjects_: Trying to imitate Unity behaviour, the activation or deactivation of any gameobject properly activates or deactivates the corresponding state of this gameobject but keeping their child gameobjects with their corresponding activation state.
- _Bugs, bugs, bugs..._: Some OpenGL Buffer problems when deleting them, scene management when loading, elimination of Assets folder used when engine runs in game mode, ...
