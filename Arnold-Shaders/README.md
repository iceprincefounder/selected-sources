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
