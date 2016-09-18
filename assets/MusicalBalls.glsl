vec3 HSVtoRGB(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

float Hash(vec3 co)
{
    float a = 12.98;
    float b = 78.23;
    float c = 438.553;
    float d = 31.309;
    vec3 s = vec3(a, b, c);
    float dt = dot(co, s);
    float sn = mod(dt, 3.14);
    return fract(sin(sn)*d);
}

float Map(vec3 pos)
{
	vec3 p = 2.*fract(pos) - 1.;
    return 1.1*length(p) - .9*texture2D(iChannel0, vec2(Hash(.2*floor(pos)), 0.2)).x;
}

float Scene(vec3 ro, vec3 rd)
{
    float t = 0.;
    for (int i = 0; i < 28; i++) {
    	float d = Map(ro + rd*t);
        t += .4*d; 
    }
    return t;
}

#define time iGlobalTime*.5+iMouse.x*.01

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = 2.*fragCoord.xy / iResolution.xy - 1.;
    uv.x *= iResolution.x / iResolution.y;
    vec3 rd = normalize(vec3(uv, 2.));
    vec3 ro = vec3(-3., 1., -1.);
    
    mat2 rot = mat2(cos(time), -sin(time),
                    sin(time),  cos(time));
    //Add rotations for origin and direction along different axes with respect to time
    ro.xz *= rot;
    rd.xy *= rot;
    rd.xz *= rot;
    
    float t = Scene(ro, rd);
    vec3 pos = (ro+rd*t);
    float angle = atan(pos.y, pos.z)/3.14/2.;
    vec3 c = HSVtoRGB(vec3(angle, 1., 1.));
    fragColor = vec4(c/(t*t*.2+1.),1.0);
}