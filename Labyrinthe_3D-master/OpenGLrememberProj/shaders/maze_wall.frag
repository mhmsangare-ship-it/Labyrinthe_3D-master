varying vec2 texCoord;
varying vec3 Normal;
varying vec3 Position;

uniform sampler2D tex;
uniform vec3 light_pos_v;
uniform vec3 Ia;
uniform vec3 Id;
uniform vec3 Is;

void main(void)
{
    vec4 texColor = texture2D(tex, texCoord);

    vec3 N = normalize(Normal);
    vec3 L = normalize(light_pos_v - Position);
    vec3 V = normalize(-Position);
    vec3 R = reflect(-L, N);

    vec3 ambient  = Ia * texColor.rgb * 0.25;
    vec3 diffuse  = Id * texColor.rgb * max(dot(N, L), 0.0) * 0.80;
    vec3 specular = Is * 0.10 * pow(max(dot(R, V), 0.0), 16.0);

    gl_FragColor = vec4(ambient + diffuse + specular, texColor.a);
}
