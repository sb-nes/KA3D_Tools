# KA3D Tools
This app is a tool to convert KA3D/Fusion Engine assets back to their general format:

1. .ntx -> Img/Tex -> .png/.jpg/.tiff
2. .hgr -> Assets/Objects -> .fbx [under progress]

[NOTE: not all .hgr files (KA3D engine's gamedata, leveldata, prefabs, etc.) can be directly converted into existing files]

## Downloads
[Windows](https://github.com/sb-nes/KA3D_Tools/releases)

## Information
### NTX
KA3D stores images and textures as binary NTX files (written using buffer-writer), thus can be read using any buffer-reader.
It is made up of three parts: Header, Colour Palette, and Pixel Data.

[All the data in header is of size 'uint16_t']

Header: 
1. Version (Engine or Converter)
2. [Width, Height]
3. Format (KA3D Image Data Format | Not the actual image format) - How to efficiently store 'data'
4. Palette Size - for reading the colour palette
5. Flags(unused)
6. User Flags (unused)

Colour Palette: Next in buffer of size 'Palette Size' from the header; It is an array of all the colours that may be present in an image. Size limited to 256 unique colours.

Pixel Data: Remaining data in the buffer; Each Pixel contains the index for its colour to lookup in the colour palette.

### HGR
Hierarchical Graphics Rendering Pipeline
refer to documentation here [link](https://github.com/pent0/ka3d/blob/master/docs/KA3D%20hgr%20file%20format.doc)

It holds a vast collection of graphical objects and it's information in proprietary format, to be used with Kajala's(GamePixelgene) ka3d engine.



### changelog: a little error i made in v0.1

What it looked like: &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; What it should look like:

![Error Png](./Files/nokia_splash_de_error.png)
![Fixed Png](./Files/nokia_splash_de_fixed.png)

This intepolating artifact occurred simply because of the swap between height and width.
