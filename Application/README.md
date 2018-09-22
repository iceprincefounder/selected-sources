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
