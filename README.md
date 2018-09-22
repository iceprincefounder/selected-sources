

![headtitle](https://user-images.githubusercontent.com/16664056/43779184-507f5328-9a8a-11e8-8d88-cea7c984d0aa.png)

------------------------
* [Procedurals](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals)
* [Application](https://github.com/iceprincefounder/selected-sources/tree/master/Application)
* [Katana-SuperTools](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-SuperTools)
* [Arnold-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Shaders)
* [OSL-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/OSL-Shaders)
* [Katana-Plugins](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-Plugins)
* [USD-Scripts](https://github.com/iceprincefounder/selected-sources/tree/master/USD-Scripts)
* [UE4-Plugins](https://github.com/iceprincefounder/selected-sources/tree/master/UE4-Plugins)

## Procedurals
----------------------------------------------
### Speedtree Kit

![sp_2](https://user-images.githubusercontent.com/16664056/44787787-ae406780-abca-11e8-9b1e-35f288dc3734.png)
1. **Overview and Purpose**

     The [SpeedtreeKit](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals/speedtreeKit) is a    toolset which contents speedtreeLib(based on Unreal Engine), [speedtreeProc](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals/speedtreeKit/speedtreeProc) and [speedtreeOp](https://github.com/iceprincefounder/selected-sources/tree/master/Procedurals/speedtreeKit/speedtreeOp). The speedtreeLib is a speedtree translation lib would convert speedtree original geometry data into general purpose vertex index vector so that the other applications could use. The speedtreeProc contents arnold procedural node " speedtree_procedural" and a shader "speedtree_shader", the procedural node would read speedtree original file which suffix is *.srt via translation lib\`s API and convert into renderer polymesh node, it might generate more than one polymesh node would be assigned with a "speedtree_shader" to make sure every polymesh node would get different shader and different look. The speedtreeOp contents a Katana Op node "Speedtree_In" which would read original speedtree file into Katana and available to render and also create a proxy alembic file in the viewer.
 
 2. **Pipeline Design**

    Use "Speedtree_In" node to read original speedtree files and create the looks with "speedtree_shader" then export assets into arnold ASS files and go back to maya and use xgen to create the plant in shot.Finally, render the shot assets in Katana with "xgen_procedural" node.
 
 3. **Why use speedtreeKit**

    The speedtreeKit toolset would make your assets of plant very small and don\`t need the animation sequences because every animation frame would be calculated by the renderer. That would improve both your IO speed and also render speed. 


## Application
----------------------------------------------
### EXRAutoCropper
 1. **Overview and Purpose**

    The [EXRAutoCropper](https://github.com/iceprincefounder/selected-sources/tree/master/Application/EXRAutoCropper) would read the OpenEXR files and auto crop it. As we know, most of the OpenEXR images from renderer have large sizes and tiled type, we could read the original image data and write out with optimized color information and scanline type which NUKE preferred with no quality lost and then crop the data window of new exr file. The EXRAutoCropper supporting muti-channel EXR file and muti-threads process.

 2. **Benefit**
     
     Lower size and faster IO speed in Nuke.


### OpenEXRResizer

![resizer](https://user-images.githubusercontent.com/16664056/44787800-b7c9cf80-abca-11e8-90ca-7c8a215cfdb7.png)
 1. **Overview and Purpose**

    The [OpenEXRResizer](https://github.com/iceprincefounder/selected-sources/tree/master/Application/OpenEXRResizer) is a tool that aim to resize EXR files to get lower size proxy images. Supporting muti-threads process. Supporting Cryptomatte files.

 2. **Pipeline Design**
    
    In composting pipeline, those are usually so many 2k or 4k OpenEXR files IO at same time in Nuke, we could convert those files into lower size as proxy and change it back on final render to extremely improve the efficiency of composting.


## Katana-SuperTools
----------------------------------------------
### ArnoldLightGafferPackage

![gtp](https://user-images.githubusercontent.com/16664056/43777604-df175ca2-9a85-11e8-88f1-8c36f4ea6667.png)

The [ArnoldLightGafferPackage](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-SuperTools/ArnoldLightGafferPackage) is a GafferThree Supertools that contents all presets of lights and light filters we could create them easily.


### MaterialXBake

![mx](https://user-images.githubusercontent.com/16664056/43777614-e41b9470-9a85-11e8-983f-4395cbc382c2.png)

 1. **Overview and Purpose**
    
    The [MaterialXBake](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-SuperTools/MaterialXBake) tools is a Katana Supertools node to bake Katana shading networks into materialx file. Since Arnold support MaterialX, we could make shading pipeline much more flexible.

 2. **Benefit**
    
    In the final lighting shot, we could override the whole shot materials and also override the specified asset look include in Arnold ASS archives.


## Arnold-Shaders
----------------------------------------------
### ParallaxMap Shader

![pm](https://user-images.githubusercontent.com/16664056/43777727-447f6396-9a86-11e8-88a7-1ed80f6b310c.png)

 1. **Overview and Purpose**

    The [ParallaxMap](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Shaders/parallax_map) shader is the implementation of [OpenGL Parallax Map](https://learnopengl.com/Advanced-Lighting/Parallax-Mapping) in Arnold, with this one, we can create very realistic looks of environment just with normal map and bump(depth) map, the look is very similar to the displacement map but much more efficient then displacement map.
 2. **Benefit**
 
     More efficient and easier to create and control.
 3. **How to use**

     Step 1: *Create "parallax_map" shader;* 
 
     Step 2. *Connect arnold "standard_surface"shader to parameter "Standard Surface";* 
 
     Step 3. *Connect depth map or bump map to parameter "Parallax Map";* 
 
     Step 4. *Control the height with parameter "Parallax Value";*


### TextureRepetition Shader Kit

![tr](https://user-images.githubusercontent.com/16664056/43777159-93359160-9a84-11e8-896b-e02b00132921.png)

The [TextureRepetition](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Shaders/texture_repetition) shader kit would repeat a texture into a complex one but with no stitch. With a simple and small texture, we could get a huge size but randomized map. 

Please feel free to check the source code on my Github.


### Triplanar Shader Kit

![tp](https://user-images.githubusercontent.com/16664056/43777168-981706a0-9a84-11e8-93fa-321c97c77000.png)

The [TriplanarPlus](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Shaders/triplanar_plus) shader would project three texture images into three axis(x,y,z), the object without UV information would still get nice shading and the Pref attribute would still get smooth normal. 

Please feel free to check the source code on my Github.


## OSL-Shaders
----------------------------------------------
 1. **Overview and Purpose**

    The [OSL-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/OSL-Shaders) is the general purpose math shader library. There are a lot of shaders we almost use in every asset, the purpose of the OSL-Shaders is to handle and transform the shading pipeline from different application and renderers easily.
 
 2. **By The Way**
    
    Those are a lot of osl shaders I don\`t list here which deal with different project assets translation. 


## Katana-Plugins
----------------------------------------------
### LightViewerModifier

![lvm](https://user-images.githubusercontent.com/16664056/43777594-d9eb4586-9a85-11e8-9537-0308c57742f0.png)

The [LightViewerModifier](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-Plugins/LightViewerModifier) is a simple plugin to customize the light shapes in Katana viewer for Arnold light.


## USD-Scripts

![usd](https://user-images.githubusercontent.com/16664056/43777650-fffd271c-9a85-11e8-9581-9f6558309301.png)
 1. **Overview and Purpose**

    A [USD-Script](https://github.com/iceprincefounder/selected-sources/tree/master/USD-Scripts) library which contents all functionality to translate a Alembic based pipeline into a USD based one, from Maya to Katana.

 2. **Why I wrote**
    
    The purpose of writing those scripts is to learn, test and play with USD project. Just for fun and curious.


## UE4-Plugins
----------------------------------------------
### Cache_Actor

The Cache_Actor plugin would read a custom animation cache file and generate the animation frame in Unreal Engine, very similar to Alembic cache but we store vertex data into a texture file.

### DownloadAssets

The DownloadAssets plugin would download project file from server which exported from Maya pipeline.

### ImportAssets

The ImportAssets plugin would import all Maya assets into Unreal Engine and assign the materials automatically.

### MiliBlueprintLibrary

Some useful Blueprint node in Unreal Engine.

### MiliToolBox

A tool box shelf.
