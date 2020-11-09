# direct2d_image_lut_effect
---
# 10/22/2020
---
Show how to load a LUT image and create a custom effect based on that.
---
## LUT (Lookup Table)
A color look-up table replaces the source video colors with different colors for dramatic effect. You got a color with RGB value from the source, and you need use the RGB value as paramater to find a specific piont at the look-up table, at use the color at the point to replace the source point color.
In Adobe Photoshop or Premiere Pro, you can create a Color Lookup Table in Adobe Photoshop with the extension .3DL or .CUBE. In some cases, the LUT file is just a .png file as the "Golden.png" in the source. The image file can be converted from .3DL or .CUBE(https://streamshark.io/obs-guide/converting-cube-3dl-lut-to-image).
## Sample Project files
### CustomEffect (.h, .cpp and .hlsl)
These files were used to create ID2D1Effect. In this situation, we need pass two textures to the pixel shader. The inputs must be difined for each of them while registing the effect with function Register(_In_ ID2D1Factory1* pFactory). All the other functions that require texture count will also need to be updated to 2.
In the pixel shader(CustomEffect.hlsl), we got the color from the source image, and use the value to find the point in the LUT image. 
Let's check the neutral LUT ("neutral.png")first.

![neutral](https://raw.githubusercontent.com/se6en/direct2d_image_lut_effect/master/readme_resources/neutral.png)  
Color will not change if we use this LUT image because this is the original color. I just use this to make the logic more clear.The LUT image was divided to 64 squares, and the blue value in each square are the same[0, 4, 8 ...255](notice that the difference between two numbers are not all the same). So now if we get the source color, we use the blue value to locate the square as fiest step. For example, if the blue value is 100, then we will find the NO.25 square (blue value : 97) and the No.26 square (blue value : 101). The index will be 24 and 25 if we number all square from 0 to 63. Now we will use the red value and green value to locate the point within the square. There are 64 small squares in each square, red value change in horizontal, green value change in vertical(square_detail.png). The color will be odd if the point was located at the edge of the square, so we must add the offset to avoid this situation.
![square_detal](https://raw.githubusercontent.com/se6en/direct2d_image_lut_effect/master/readme_resources/square_detail.png).
### DirectImageCtrl (.h and .cpp)
Create direct2d resources and draw the effect here.

# load_effect_dynamic
---
#11/9/2020
---
Show how to load LUT images as effect dynamically.
---
## Effect Resources
Used to save all the effect resource files. Application will load effect from this folder while initializing. Each ini file should be related to one png file.
### Golden.ini and Noir.ini
Record effect name, GUID and LUT image. Name was used to show the effect name if necessary. GUID was used to distinguish effect. The image file was LUT image.
