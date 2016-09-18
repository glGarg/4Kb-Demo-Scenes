float Hash(float n)
{
	return fract(cos(n)*2384.424);
}

float Hash(vec2 p)
{
	vec2 x = fract(p / vec2(3.29, 5.23));
    return fract(3235.3432*sin(x.x*x.y));
}

float Noise(vec2 x)
{
	vec2 p = floor(x);
    vec2 f = fract(x);    
    f = f*f*(3.-2.*f);
    return mix(mix(Hash(p),Hash(p + vec2(1., 0.)),f.x),
               mix(Hash(p + vec2(0., 1.)),Hash(p + vec2(1.)),f.x),f.y);
}

float Water(vec2 p)
{
	float height = 0.;
    float amp = 11.5;
    vec2 shift1 = 0.001*vec2(iGlobalTime*320., iGlobalTime*240.);
    vec2 shift2 = 0.001*vec2(iGlobalTime*380., -iGlobalTime*260.);
    
    float wave = sin(.021*p.x+shift1.x);
    wave += sin(p.x*.0172+p.y*0.01+shift2.x*1.121)*4.;
    wave += sin(p.x*.00172 + p.y*.005 + shift2.x*.121)*4.;
    wave += sin(p.x*.02221+p.y*.01233+shift2.x*3.437)*5.;
    wave += sin(p.x*.03221 + p.y*.01311 + shift2.x*4.269)*2.5;
    wave *= 1.4; //maximum 1.4
    for (int i = 0; i < 11; i++) {
    	wave += abs(sin(Noise(p*.01+shift1)-.5)*3.14)*amp;
        amp *= -.5;
        shift1 *= 1.841;
        p *= 2.*mat2(.866, .5, -.5, .866);
    }
    return height+wave;
}

float WaterH(vec2 p)
{
	float height = 0.;
    float amp = 11.5;
    vec2 shift1 = 0.001*vec2(iGlobalTime*320., iGlobalTime*240.);
    vec2 shift2 = 0.001*vec2(iGlobalTime*380., -iGlobalTime*260.);
    
    float wave = sin(.021*p.x+shift1.x);
    wave += sin(p.x*.0172+p.y*0.01+shift2.x*1.121)*4.;
    wave += sin(p.x*.00172 + p.y*.005 + shift2.x*.121)*4.;
    wave += sin(p.x*.02221+p.y*.01233+shift2.x*3.437)*5.;
    wave += sin(p.x*.03221 + p.y*.01311 + shift2.x*4.269)*2.5;
    wave *= 1.4;
    for (int i = 0; i < 13; i++) {
    	wave += abs(sin(Noise(p*.01+shift1)-.5)*3.14)*amp;
        amp *= -.5;
        shift1 *= 1.841;
        p *= 2.*mat2(.866, .5, -.5, .866);
    }
    return height+wave;
}

float Map(vec3 pos)
{
	return pos.y - Water(pos.xz);
}

bool Scene(vec3 ro, vec3 rd, vec3 lPos, out float t)
{
	bool hit = false;
	float h;
    for (int i = 0; i < 156; i++) {
    	h = Map(ro + rd*t);
        if (t > 4500.) {
        	hit = false;
            break;
        }
        if (h < .1) {
        	hit = true;
        }
        t += h*.5;
    }
	return hit;
}

vec3 CalcNormal(vec3 pos, float distance)
{
    float t = min(.3, .005+.0005*distance*distance);
	vec3 eps = vec3(.0001*distance*distance, 0., 0.);
    vec3 nor = vec3(0., WaterH(pos.xz), 0.);
    vec3 v2 = nor - vec3(eps.x,WaterH(pos.xz+eps.xy),0.);
    vec3 v3 = nor - vec3(0., WaterH(pos.xz - eps.yx), -eps.x);
    return normalize(cross(v2, v3));
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec3 normal;
	vec2 p = 2.*fragCoord.xy / iResolution.xy - 1.;
	p.x *= iResolution.x/iResolution.y;
    float x = 10.+iMouse.x;
    float y = 0.;
    float z = 10.+iMouse.x;
    
    vec3 ro = vec3(x, y, z);
    ro.y += 20.;
    vec3 lookAt = .5*ro;
    vec3 eye = normalize(lookAt - ro);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    
    vec3 rd = normalize(vec3(.8*eye + p.x*right + p.y*up));
    vec3 col, lPos = vec3(0.5, 3., 1.0);
    float t;
    if (!Scene(ro, rd, lPos, t)) {
        col = vec3(0.9, 0.9, 0.9);
        col = mix(vec3(0.4, 0.4, 0.4), col, clamp(exp(-t*t/100.*0.002), 0., 0.8));
    } else {
        vec3 lCol = .5*vec3(.55, .55, .73);
        col = .8*vec3(0.05, 0.48, 0.54);
    	normal = CalcNormal(ro + rd*t, t);
        float effect = 0.;
        float fresnel = 1. - max(dot(normalize(-ro+normalize(lPos)),normalize(lPos)), 0.);
        fresnel = pow(fresnel, 3.); //3.
        //fresnel += (1.-fresnel)*.65;
        
        effect += 0.2 + 1.- fresnel;
        //use fresnel as a reflection/refraction factor
        
        //effect += .5*dot(normal, normalize(lPos));
        effect += .4*dot(normalize(normalize(lPos)+normalize(ro+rd*t)), normal);
        col = effect*lCol + pow(max(dot(reflect(eye,normal), normalize(lPos)), 0.), 40.);
        float atten =0.022;
        col += exp(-atten*t)*vec3(0.8,0.9,0.6) + (1. - exp(-atten*t))*vec3(0.1,0.19,0.22);
    }
    col = mix(vec3(0.2, 0.2, 0.2), col, clamp(exp(-t*t/50.*0.002), 0., 0.8));
    fragColor = vec4(col, 1.0);
}