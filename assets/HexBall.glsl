float shine;
vec4 frequency;

vec4 Map(vec3 p)
{
    vec4 col = 0.87*vec4(p, 1.);
    vec2 i = p.xz*5.;
    i = i/col.y+iGlobalTime;
    col.xy -= step(i -= col.xy = ceil(i += i.x *= .577), mod(col.x+col.y, 3.)+--i.yx);
    col = mod(col.xyyy,.8);
    col *= 1.5*frequency;
    shine += .09;
    return vec4(length(p) - 4.3 + dot(col, vec4(0.5)), col.xyz);
}

vec3 GetNormal(vec3 pos)
{
	vec3 eps = vec3(.1, 0., 0.);
    vec3 nor = (vec3(Map(pos + eps).x - Map(pos - eps).x,
                     Map(pos + eps.yxy).x - Map(pos - eps.yxy).x,
                     Map(pos + eps.yyx).x - Map(pos - eps.yyx).x));
	return normalize(nor);
}

vec3 Scene(vec3 ro, vec3 rd)
{
    float t = 0., d = 10.;
    for (int i = 0; i < 900; i++) {
    	if (t > 10. || d < 0.)
            return .2*shine*textureCube(iChannel1, ro+rd*100.).xyz;
        vec4 a = Map(ro + rd*t);
        d = a.x;
        if (d < 0.001) {
            for (int i = 0; i < 20; i++) {
    			vec4 a = Map(ro + rd*t);
        		d = a.x;
        		t += d*.03;
    		}
            shine += .09*20.;
            vec3 normal = GetNormal(ro+rd*t);
            a.xyz += .005*textureCube(iChannel1, rd).xyz;
            a.xyz += .04*textureCube(iChannel1, reflect((rd), normal)).xyz;
            return .8*shine*(a.yzx+a.yxz+a.zxy)/12.;
        }
        t += d*.05;
    }
    return vec3(0.);
}

void mainImage (out vec4 fragColor, in vec2 fragCoord) {
	vec2 uv = 2. * fragCoord.xy / iResolution.xy - 1.;
    float x = 0.;
    float y = 5.5;
    float z = .3;
    vec3 ro = vec3(x, y, z);
    vec3 lookAt = vec3(0., 0., 0.);
    vec3 eye = normalize(lookAt - ro);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    vec3 rd = mat3(right/.58, up, eye)*normalize(vec3(uv/2., 0.3));
    frequency = vec4(texture2D(iChannel0, vec2(0.01, 0.25)).x, 
                     texture2D(iChannel0, vec2(0.07, 0.25)).x,
                     texture2D(iChannel0, vec2(0.15, 0.25)).x,
                     texture2D(iChannel0, vec2(0.30, 0.25)).x);
    
    fragColor = vec4(Scene(ro, rd),1.0);
}                           