# MyRevoke

## What is this?

Hey there! Welcome to MyRevoke super raw, experimental project I put together mainly for practice and to get my hands dirty with real software development. If you�re looking for something polished or production-ready, you�re definitely in the wrong place! This is more like a playground where I tried things, broke things, and (hopefully) learned a lot along the way.

## Why does this exist?

Honestly? i wanted to see what it�s like to build a little engine from scratch, mess around with some libraries, and figure out how all the pieces fit together. There are thousands of bugs, weird design choices, and unfinished features. That�s all part of the fun! The main goal here is learning, not perfection.

## How is it built?

### Architecture

- **Core Engine:** Everything starts with a custom C++ engine. I tried to keep things modular so i can swap stuff in and out as i learn more.
- **Scene Management:** I wrote my own way to handle scenes and entities in it. It�s basic, but it works (most of the time).
- **Scripting:** There�s a little scripting system built in, so you can (in theory) add some logic without touching the engine code (ofc it dosnt work properly).
- **Serialization:** Saving and loading scenes/entities is handled by my own serialization code. It mostly gets the job done.

### What did I use?

- **Language:** Good old C++
- **Libraries:**
  - [Box2D](https://github.com/erincatto/box2d) for physics 
  - [GLFW](https://github.com/glfw/glfw) for windowing 
  - [ImGui](https://github.com/ocornut/imgui) for the UI (because making my own would be a nightmare)
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp) for serialization (saving/loading stuff)
  - [GLAD](https://github.com/Dav1dde/glad) for OpenGL loading 

## What's actually in here?

If you want to poke around the code, here's the map. Each of these has its own README with more
detail (module purpose, key classes, and the bugs/typos I stumbled into while writing them):

- **[`MyRevoke/`](MyRevoke/README.md)** — the engine itself (static lib). Core app/window/layer
  plumbing, the OpenGL renderer, the EnTT-based scene/ECS layer, scripting, audio.
- **[`RevokeCraft/`](RevokeCraft/README.md)** — the editor app (the thing you actually run).
- **[`SandBox/`](SandBox/README.md)** — a bare-bones scratch app for poking at engine features
  directly, outside the editor.
- **[`MyRevoke-NativeScriptCore/`](MyRevoke-NativeScriptCore/README.md)** — the hot-reloadable DLL
  project native C++ gameplay scripts get compiled into.
- **[`MyRevoke-ScriptCore/`](MyRevoke-ScriptCore/README.md)** — the C# API surface for the (still
  early) Mono scripting path.

## How do I build this thing?

So you want to try it out? Here�s how you can use the project:

**Clone the repo** (with submodules!)
   ```bash
   git clone --recursive https://github.com/Revokeeee/MyRevoke.git
   ```

 **Use existing .exe file.** Go into RevokeCraft folder and run the .exe file and see how long can you go without crashes (my record is 2 minutes) 

Or if you intrested in code and debaging follow the next moves to build the solution:

1. **Generate the Visual Studio project files**  
Just run the `GenerateProject.bat` file in the root folder. This uses Premake to set everything up for you.
2. **Open the solution**  
Open the generated `.sln` file in Visual Studio 2026 (or a recent version that supports C++17 or newer).
3. **Build the solution** (just hit `Ctrl+Shift+B` or use the Build menu).
4. **Run it!** If everything goes well, you should see a window pop up.

> **Note:** You don�t need to mess with CMake or hunt down dependencies�Premake and the submodules handle all that for you. Just make sure you have a modern C++ compiler and Visual Studio installed.

## What can you actually do in MyRevoke?

Right now, MyRevoke is more of a sandbox than a finished product, but here�s what you can play with:

- **Create and manage scenes:** Add, remove, and move entities around in a scene.
- **Physics fun:** Drop some objects and watch them bounce and fall thanks to Box2D.
- **Basic scripting:** Attach simple scripts to entities to make them do stuff (or break stuff).
- **UI experiments:** Play around with the ImGui-based interface to see what�s possible.
- **Save and load scenes:** Save your creations and load them back up later.

### Screenshots  

Here are a few screenshots to give you an idea of what�s possible (or at least what it looks like when it�s working):  

![Screenshot of MyRevoke](media/screanshot.png)  

<div align="center">
  <img src="media/gif1.gif" alt="GIF of scene creation" width="600">
</div>

<div align="center">
  <img src="media/gif2.gif" alt="GIF of physics simulation" width="600">
</div>

## What�s the approach?

I�m not following any strict rules or best practices here. The idea is to experiment, try new things, and not be afraid to make mistakes. Sometimes I�ll refactor big chunks of code, sometimes I�ll leave a mess and move on.

## Can I use this?

You�re welcome to poke around and learn from my mistakes. Just don�t expect it to work perfectly (or at all, sometimes). And if you find a bug, i wont be fixing it anyways xD!

---
