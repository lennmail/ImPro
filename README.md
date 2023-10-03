# impro

 **WORK IN PROGRESS**

## Description
This repo contains a **C++ Library for Image Processing**. It is a project I started due to my interest in the field and the aspiration to become proficient in C++. The code has been influenced by a YT series uploaded by the user "Code Break". The library heavily relies on two C stb header files, **stb_image.h** and **stb_image_write.h**, which do most of the heavy lifting. They can be found [here](https://github.com/nothings/stb).

## Functionality
The library currently consists of an image class which defines the following member functions:

+ **read**: takes in a filename and constructs an image data struct consisting of image width, height, and # of channels. 
+ **write**: takes in a filename and writes data to the file. Relies on the **stbi_write_XXX** function defined in the stb header files, where XXX denotes the filetype. Returns a boolean, likewise to the **read** function.
+ **getImageType**: takes in a filename and returns the filetype. Returns **UNKNOWN** if the filetype is not one of **{JPG, PNG, BMP, TGA}**.
+ **gray_scale**: converts an image to greyscale as long as the image has 3 channels.
+ **grayscale_luminescence**: converts an image to greyscale as long as the image has 3 channels. Retains the natural luminescence of the image. The RGB values are based on a weighted linear combination of previous RGB values. Coefficients chosen according to [this](https://en.wikipedia.org/wiki/Grayscale) site.
+ **color_mask**: manipulates RGB values according to specified red, green, and blue input values.

... and so on
