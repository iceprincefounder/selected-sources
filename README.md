![title](https://user-images.githubusercontent.com/16664056/43779184-507f5328-9a8a-11e8-8d88-cea7c984d0aa.png)


------------------------
* [Arnold-Procedurals](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Procedurals)
* [Arnold-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Shaders)
* [Katana-Plugins](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-Plugins)
* [Katana-SuperTools](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-SuperTools)
* [OSL-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/OSL-Shaders)
* [Standalone-bin](https://github.com/iceprincefounder/selected-sources/tree/master/Standalone-bin)
* [UE4-Plugins](https://github.com/iceprincefounder/selected-sources/tree/master/UE4-Plugins)
* [USD-Scripts](https://github.com/iceprincefounder/selected-sources/tree/master/USD-Scripts)

------------------------
## Arnold-Procedurals

### speedtree procdural Kit

![sp_2](https://user-images.githubusercontent.com/16664056/44787787-ae406780-abca-11e8-9b1e-35f288dc3734.png)

The SpeedtreeKit plugin would read the speedtree file(.srt) and translate the geometry data into arnold polymesh node. This used in our natural enviroment pipeline. In Katana, using Speedtree_In Op node to read speedtree srt file and We could assign different shaders to one procedural node with speedtree_shader, the Speedtree_In node would create a alembic proxy in viewer so that you could see it in Katana and render it as a rendererProcedural in Arnold.


For more details: [Arnold-Procedurals](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Procedurals).




------------------------
## Arnold-Shaders

### parallax_map shader

![pm](https://user-images.githubusercontent.com/16664056/43777727-447f6396-9a86-11e8-88a7-1ed80f6b310c.png)

The parallax_map shader is the implementation of OpenGL parallax map in Arnold, with this shader, we can create very realistic looks of environment with normal map and bump(depth) map, the look is very simillar to displacement map but much more efficient then displacement shader.


### texture_repetition shader kit

![tr](https://user-images.githubusercontent.com/16664056/43777159-93359160-9a84-11e8-896b-e02b00132921.png)

The texture_repetition shader would repeat a texture into a complex one but with no stitch. With a simple and small texture, we could get a huge size but randomized map.


### triplanar shader kit

![tp](https://user-images.githubusercontent.com/16664056/43777168-981706a0-9a84-11e8-93fa-321c97c77000.png)

The triplanar_plus shader would project three texture images into three axis(x,y,z), the object without UV information would still get nice shading and the Pref attribute would still get smooth normal.


For more details: [Arnold-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/Arnold-Shaders).




------------------------
## Katana-Plugins

### LightViewerModifier

![lvm](https://user-images.githubusercontent.com/16664056/43777594-d9eb4586-9a85-11e8-9537-0308c57742f0.png)

The LightViewerModifier plugin would customize the light shapes in Katana viewer for Arnold light.

For more details: [Katana-Plugins](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-Plugins).




------------------------
## Katana-SuperTools

### ArnoldLightGafferPackage

![gtp](https://user-images.githubusercontent.com/16664056/43777604-df175ca2-9a85-11e8-88f1-8c36f4ea6667.png)

The ArnoldLightGafferPackage is a GafferThree Supertools that contents all presets of lights and light filters we could create them easily.


### MaterialXBake

![mx](https://user-images.githubusercontent.com/16664056/43777614-e41b9470-9a85-11e8-983f-4395cbc382c2.png)

The MaterialXBake tools is a Katana Supertools node, since Arnold support MaterialX but didn\`t provide any tools to export MaterialX file so I create Katana version MaterialXBake node to export shading networks from Katana, get ready to replace KLF(Katana look file) file in our pipeline in some special shots.


For more details: [Katana-SuperTools](https://github.com/iceprincefounder/selected-sources/tree/master/Katana-SuperTools).




------------------------
## OSL-Shaders

We find that there are some shaders we use every day and every where, for gennaral purpuse, I wrote it as an OSL shader so that we could handle it easily. 

For more details: [OSL-Shaders](https://github.com/iceprincefounder/selected-sources/tree/master/OSL-Shaders).




------------------------
## Standalone-bin

### EXRAutoCropper

The EXRAutoCropper tool would read the EXR file and auto crop it, then write the image data into a new one with optimized RGB information and Scanline type which NUKE preferred so that we would get a pretty pure EXR file for compositing, suporting muti-channel EXR file.

## OpenEXRResizer

![resizer](https://user-images.githubusercontent.com/16664056/44787800-b7c9cf80-abca-11e8-90ca-7c8a215cfdb7.png)

The OpenEXRResizer is a C++ command executable file to resize EXR files, supporting muti-threads and cropped file, the resized files could be the proxy files in compositing pipeline. The Cryptomatte files could be resized with this tools too.

For more details: [Standalone-bin](https://github.com/iceprincefounder/selected-sources/tree/master/Standalone-bin).




------------------------
## UE4-Plugins

### Cache_Actor

The Cache_Actor plugin would read a custom animation cache file and gennarate the animation frame in Unreal Engine, very similar to Alembic cache but we store vertex data into a texture file.

### DownloadAssets

The DownloadAssets plugin would download project file from server which exported from Maya pipeline.

### ImportAssets

The ImportAssets plugin would import all Maya assets into Unreal Engine and assign the materials automatically.

### MiliBlueprintLibrary

Some useful Blueprint node in Unreal Engine.

### MiliToolBox

A tool box shelf.

For more details: [UE4-Plugins](https://github.com/iceprincefounder/selected-sources/tree/master/UE4-Plugins).




------------------------
## USD-Scripts

![usd](https://user-images.githubusercontent.com/16664056/43777650-fffd271c-9a85-11e8-9581-9f6558309301.png)

A USD script library which contents all functionality to tanslate a Almebic based pipeline into a USD based one, from Maya to Katana.


For more details: [USD-Scripts](https://github.com/iceprincefounder/selected-sources/tree/master/USD-Scripts).
