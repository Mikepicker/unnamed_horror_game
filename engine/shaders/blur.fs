#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D texture_blur;

void main() 
{
  vec2 texel_size = 1.0 / vec2(textureSize(texture_blur, 0));
  float result = 0.0;
  for (int x = -2; x < 2; ++x) 
  {
    for (int y = -2; y < 2; ++y) 
    {
      vec2 offset = vec2(float(x), float(y)) * texel_size;
      result += texture(texture_blur, TexCoords + offset).r;
    }
  }
  FragColor = result / (4.0 * 4.0);
}  
