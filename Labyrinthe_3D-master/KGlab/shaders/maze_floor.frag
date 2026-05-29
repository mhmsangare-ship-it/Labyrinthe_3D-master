varying vec2 texCoord;
varying vec3 Normal;
varying vec3 Position;

uniform sampler2D tex;
uniform vec3 light_pos_v;
uniform vec3 Ia;
uniform vec3 Id;

void main(void)
{
    vec4 texColor = texture2D(tex, texCoord);

    vec3 N    = normalize(Normal);
    vec3 L    = normalize(light_pos_v - Position);
    float diff = max(dot(N, L), 0.0);

    vec3 color = Ia * texColor.rgb * 0.20
               + Id * texColor.rgb * diff * 0.80;

    gl_FragColor = vec4(color, 1.0);
}
