## Standalone-bin

### EXRAutoCropper

The EXRAutoCropper tool would read the EXR file and auto crop it, then write the image data into a new one with optimized RGB information and Scanline type which NUKE preferred so that we would get a pretty pure EXR file for compositing, suporting muti-channel EXR file.

## OpenEXRResizer

The OpenEXRResizer is a C++ command executable file to resize EXR files, supporting muti-threads and cropped file, the resized files could be the proxy files in compositing.