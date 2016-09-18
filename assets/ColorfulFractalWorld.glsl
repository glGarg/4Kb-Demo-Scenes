float Hash(float n)
{
	return fract(sin(n)*3584.327);
}

float Hash(vec2 p)
{
	vec2 x = fract(p / vec2(2.99, 5.67));
    return fract(3795.3091*cos(x.x*x.y));
}

float Noise(vec2 x)
{
	vec2 p = floor(x);
    vec2 f = fract(x);    
    f = f*f*(3.-2.*f);
    return mix(mix(Hash(p),Hash(p + vec2(1., 0.)),f.x),
               mix(Hash(p + vec2(0., 1.)),Hash(p + vec2(1.)),f.x),f.y);
}

vec3 CSize = 1.*vec3(0.633, 0.832, .713); //vec3(0.613, 0.712, .81);

float Map(vec3 p)
{
    float scale = 1.;
    float t;
    vec3 pos = p;
    for (int i = 0; i < 8; i++) {
    	p = 2.*clamp(p, -CSize, CSize) - p;
        float k = max(0.812/dot(p, p), 1.);
        p *= k;
        scale *= k;
     }
    float rxy = (-p.z*length(p.xy)/(length(p)+.07-.02*cos(pos.x*p.x + pos.y*p.y + pos.z*p.z)));
	return max(rxy-dot(CSize, pos), abs(rxy*pos.z)/length(pos))/scale;
}

bool Scene(vec3 ro, vec3 rd, out float t)
{
	bool hit = false;
	float h;
    for (int i = 0; i < 150; i++) {
        if (t < 12.) {
        	h = Map(ro + rd*t);
        	if (h < .0003*t) {
        		hit = true;
        	    break;
        	}
        	t += h*.7;
        }
    }
	return hit;
}

vec3 Color(vec3 p)
{
	float col = 0.;
    float k, scale = 1.;
    for(int i = 0; i < 8; i++){
    	vec3 p1 = 2.*clamp(p, -CSize, CSize)-p;
        col += abs(p.x - p1.x);
        p = p1;
        k = max((.812/dot(p, p)), 1.);
        p *= k;
        scale *= k;
    }
    return sin(col * vec3(.6, -.9, 4.9))*.75+.4;//normalize(20.*s*Noise(100.*p.xz));//sin(col * vec3(.65, -.9, 2.4))*.75 + 0.4;//vec3(.6, -.9, 4.9)//sin(col * vec3(.4, -.9, .6))*.75 + .4;//sin(col * vec3(.6, -.9, 4.9))*.75 + 0.4
}


vec3 CalcNormal(vec3 pos, float distance)
{
    float t = min(.3, .001+.0001*distance*distance);
	vec3 eps = vec3(.00001*t, 0., 0.);
    vec3 nor = vec3(0., Map(pos), 0.);
    vec3 v2 = nor - vec3(eps.x, Map(pos+eps),0.);
    vec3 v3 = nor - vec3(0., Map(pos-eps.yyx), -eps.x);
    return normalize(cross(v2, v3));
}

float Shadow(vec3 pos, vec3 ld, float d)
{
    float h = 0., t = 0.;
    float res = 1.;
    for(int i = 0; i < 14; i++){
        if(t < d){
        	h = Map(pos+ld*t);
            res = min(4.*h/t, res);
        	t += .4*h;
        }
    }
    return clamp(res, 0.2, 1.);
}

vec3 lightPos;

vec3 Render(vec3 pos, vec3 ro, vec3 rd, vec3 nor, float t)
{
	vec3 col = vec3(0.);
    vec3 lightPos = pos+vec3(cos(iGlobalTime*2.0+2.8+iMouse.x*.1)*.3, 1.+cos(iGlobalTime*2.0+2.8+iMouse.x*.1)*.2, sin(iMouse.x*.1+iGlobalTime*2.0+2.4));// + vec3(cos(iGlobalTime*2.0+2.8)*.3, cos(iGlobalTime*2.0+1.5)*.1, 0.);//ro + sin(iGlobalTime*0.05)*vec3(pos.x, 20., pos.z);
    vec3 lPos = lightPos - pos;
    float bri = 1.;//max(dot(normalize(lPos), nor), 0.);
    float diff = dot(normalize(normalize(lPos)-normalize(pos)), nor);
    float spec = max(dot(reflect(normalize(pos), nor), normalize(lPos)), 0.);
    float amb = max(abs(nor.x)*.02, 0.025);
    float shad = Shadow(pos, normalize(lPos), length(lPos));
    col = Color(pos);
    return col*shad + 0.5*col*diff +.5*pow(spec, 15.)*shad + 0.3*amb*col;
}

float Glow(vec3 lPos, vec3 rd, vec3 pos)
{
	float l;
    if(lPos.y < pos.y){
    	l = pow(dot(normalize(lPos), rd), 1000.);
    	l *= 3.;
    }
    return l;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    vec3 normal;
	vec2 p = 2.*fragCoord.xy/iResolution.xy - 1.;
	p.x *= iResolution.x/iResolution.y;

    float x = -.5;
    float y = -1.+10.*sin(iGlobalTime*.05)-5.*cos(0.01*iMouse.x - sin(iGlobalTime*.1));//-19.*sin(0.01*iGlobalTime);//iMouse.x/iResolution.x;
    float z = 7.;
    float t;

    vec3 ro = vec3(x, y, z);
    vec3 lookAt = vec3(1., 0., z);
    vec3 eye = normalize(lookAt - ro);
    vec3 right = -normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    vec3 rd = normalize(2.5*eye + p.y*right + p.x*up);

    Scene(ro, rd, t);
    normal = CalcNormal(ro + rd*t, t);
    vec3 pos = ro +rd*t;
    fragColor = vec4(t < 12. ? Glow(vec3(cos(iGlobalTime*2.0+2.8+iMouse.x*.1)*.3, 1.+cos(iGlobalTime*2.0+2.8+iMouse.x*.1)*.2, sin(iMouse.x*.1+iGlobalTime*2.0+2.4)*.2), rd, pos) + Render(pos, ro, rd, normal, t) : vec3(0.), 1.0);
}