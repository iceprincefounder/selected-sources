## Arnold-Shaders

### parallax_map

The parallax_map shader is the implementation of OpenGL parallax map in Arnold, with this shader, we can create very realistic looks of environment with normal map and bump(depth) map, the look is very simillar to displacement map but much more efficient then displacement shader.

![parallax_mapping_rocks](https://user-images.githubusercontent.com/16664056/41274826-664d956c-6e51-11e8-8116-439cba3f8a5e.png)

### texture_repetition

The texture_repetition shader would repeat a texture into a complex one but with no stitch. With a simple and small texture, we could get a huge size but randomized map.

### triplanar_plus

The triplanar_plus shader would project three texture images into three axis(x,y,z), the object without UV information would still get nice shading and the Pref attribute would still get smooth normal.