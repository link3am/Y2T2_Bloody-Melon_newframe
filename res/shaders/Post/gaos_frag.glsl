#version 410

uniform sampler2D colorTexture;

layout(location = 0) in vec2 inUV;
out vec4 fragColor;

uniform float u_threshold = 1.0;
void main()
{

     float blur = 1.0;
    float offset = 1.0 / textureSize(colorTexture, 0).x;
  



    vec4 color = texture2D( colorTexture , inUV );
   
    float brightness = dot( color.rgb , vec3( 0.2126 , 0.7152 , 0.0722 ) );


    if ( brightness > u_threshold)
        {

        vec4 sum = texture2D( colorTexture , vec2( inUV.x - 4.0*blur*offset , inUV.y - 4.0*blur*offset )) * 0.0162162162;
        sum += texture2D( colorTexture , vec2( inUV.x - 3.0*blur*offset , inUV.y - 3.0*blur*offset )) * 0.0540540541;
        sum += texture2D( colorTexture , vec2( inUV.x - 2.0*blur*offset , inUV.y - 2.0*blur*offset )) * 0.1216216216;
        sum += texture2D( colorTexture , vec2( inUV.x - 1.0*blur*offset , inUV.y - 1.0*blur*offset )) * 0.1945945946;
        sum += texture2D( colorTexture , vec2( inUV.x , inUV.y )) * 0.2270270270;
        sum += texture2D( colorTexture , vec2( inUV.x + 1.0*blur*offset , inUV.y + 1.0*blur*offset )) * 0.1945945946;
        sum += texture2D( colorTexture , vec2( inUV.x + 2.0*blur*offset , inUV.y + 2.0*blur*offset )) * 0.1216216216;
        sum += texture2D( colorTexture , vec2( inUV.x + 3.0*blur*offset , inUV.y + 3.0*blur*offset )) * 0.0540540541;
        sum += texture2D( colorTexture , vec2( inUV.x + 4.0*blur*offset , inUV.y + 4.0*blur*offset )) * 0.0162162162;
        fragColor = vec4( sum.rgb *1.05, 1.0 );


    }
    else
       fragColor = vec4(color.rgb,1.0);
       //fragColor = vec4( 0.0 , 0.0 , 0.0 , 1.0 );

  
}