const mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );

float Hash( float n )
{
    return fract(sin(n)*4758.15);
}

float Noise(vec3 x)
{
	vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.-2.*f);
    float n = p.x + 157.*p.y + 113.*p.z;    
    return mix(mix(mix(Hash(n+0.), Hash(n+1.), f.x),
               mix(Hash(n+157.), Hash(n+158.), f.x), f.y),
           mix(mix(Hash(n+113.), Hash(n+114.), f.x),
               mix(Hash(n+270.), Hash(n+271.), f.x), f.y), f.z);
}

float sdSphere(vec3 p, float s)
{
    return length(p) - (2.8+2.5*(sin(1.*iGlobalTime+0.004*iMouse.x)));
}

float Fbm(vec3 p)
{
    float f = 1.0000*Noise( p ); p = m*p*2.02;
    f += 0.500*Noise( p ); p = m*p*2.03;
    f += 0.250*Noise( p ); p = m*p*2.01;
    f += 0.125*Noise( p );
    p = m*p*2.02;
    f += 0.0625*abs(Noise( p ));
    return f/1.9375;
}

float Map(vec3 pos, out float fbm)
{
	float sph = sdSphere(pos, .8);
    fbm = 1.25*Fbm(0.6*pos+vec3(0.,1., 0.)*iGlobalTime);//was .45
    sph -= 2.*fbm;
    return sph;
}

vec3 GetNormal(vec3 pos)
{
    float t;
	vec3 eps = vec3(.001, 0., 0.);
    vec3 nor = vec3(0., Map(pos, t), 0.);
    vec3 v2 = nor - vec3(eps.x, Map(pos, t),0.);
    vec3 v3 = nor - vec3(0., Map(pos, t), -eps.x);
    return normalize(cross(v2, v3));
}

bool Scene(vec3 ro, vec3 rd, out float fbm, out float t)
{
    float m, s = 1.;
    for (int i = 0; i < 200; i++) {
        vec3 pos = ro+rd*t;
        float h = Map(pos, fbm);
        if (h < 0.001) {
         	return true;
        }
        if (t > 12.) {
        	return false;
        }
        t += h*0.5;
    }
    return false;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
	vec2 uv = fragCoord.xy / iResolution.xy * 2.;
    uv -= 1.;
    uv.x *= iResolution.x/iResolution.y;
    
    float x = 0.;
    float y = 0.;
    float z = 7.;
    
    vec3 ro = vec3(x, y, z);
    vec3 lookAt = vec3(0., 0., 1.);
    vec3 eye = normalize(lookAt - ro);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    vec3 rd = normalize(vec3(.8*eye + uv.x*right + uv.y*up)); 
    float t, fbm;
    vec3 col = vec3(0.);
    if (Scene(ro, rd, fbm, t)) {
        vec3 lCol = vec3(0.2, .2, 0.);//0.5, 0.5, 0.
        vec3 lPos = normalize(vec3(1.));
    	vec3 white = vec3(4., 4., 4.);
		vec3 yellow = vec3(1., 1., 0);
		vec3 red = vec3(1., 0, 0);
		vec3 grey = vec3(0.4, 0.4, 0.4);
		float t = fract(fbm*3.0);
		if (fbm < 0.3333) {
			col =  mix(white, yellow, t);
		} else if (fbm < 0.6666) {
			col = mix(yellow, red, t);
		} else {
			col = mix(red, grey, t);
		}
        //vec3 nor = GetNormal(ro+rd*t);
        //col += 0.7*dot(nor, lPos)*lCol;
    	//col += 0.3*lCol;
    }
    
    fragColor = vec4(0.8*col+0.1, 1.);
}