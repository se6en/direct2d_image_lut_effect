
Texture2D InputTexture : register(t0);
SamplerState InputSampler : register(s0);

Texture2D ClutTexture : register(t1);
SamplerState ClutSampler : register(s1);

// LUT image size was fixed as 512 * 512
static const float2 lutdimension = float2(512.f, 512.f);
// in the LUT image, it was divided as 64 square (8 rows and 8 colums)
static const float squarenumber = 8.f;

float4 lut(float4 src)
{
   float blueCube = src.b * (squarenumber * squarenumber - 1); // index from 0 to 63

   int row1 = (int)(floor(blueCube) / 8.f);
   int column1 = (int)(floor(blueCube) - row1 * 8);

   float2 lutPos1;
   lutPos1.y = row1 / 8.f + src.g * (1.f / 8.f - 1.f / 512.f) + (0.5f / 512.f); // must add an offset to make sure the point is in the middle of the square
                                                                                // it would be odd if the point is at the edge of the square
   lutPos1.x = column1 / 8.f + src.r * (1.f / 8.f - 1.f / 512.f) + (0.5f / 512.f);

   int row2 = (int)(ceil(blueCube / 8.f));
   int column2 = (int)(ceil(blueCube) - row2 * 8);

   float2 lutPos2;
   lutPos2.y = row2 / 8.f + src.g * (1.f / 8.f - 1.f / 512.f) + (0.5f / 512.f);
   lutPos2.x = column2 / 8.f + src.r * (1.f / 8.f - 1.f / 512.f) + (0.5f / 512.f);

   float4 lutcolor1 = ClutTexture.Sample(ClutSampler, lutPos1);
   float4 lutcolor2 = ClutTexture.Sample(ClutSampler, lutPos2);

   float4 lutcolor = lerp(lutcolor1, lutcolor2, frac(255.f * src.b));
   lutcolor.a = src.a;

   return lutcolor;
}

float4 main(
   float4 clipSpaceOutput  : SV_POSITION,
   float4 sceneSpaceOutput : SCENE_POSITION,
   float4 texelSpaceInput0 : TEXCOORD0
) : SV_Target
{
   float4 color = InputTexture.Sample(
      InputSampler,
      float2 (texelSpaceInput0.xy)
   );

   color.r /= color.a;
   color.g /= color.a;
   color.b /= color.a;

   float4 lutcolor = lut(color);
   
   lutcolor.r *= color.a;
   lutcolor.g *= color.a;
   lutcolor.b *= color.a;

   return lutcolor;
}