varying vec2 uv;
uniform float time;

float WaveAmplitude() {
    return .04*exp(-.02*mod(time, 30.));
}

float WaterWaves(vec3 a) {
    return WaveAmplitude() * sin((2. * a.x * a.x + 2. * a.z * a.z) - 10.*mod(time, 30.));
}
float WaterSurface(vec3 pos){
    return length(max(abs(pos - vec3(0., WaterWaves(pos), 0.)) - vec3(3., 0., 3.), vec3(0.)));
}

float Pool(vec3 pos){
    return min(pos.x + 3., min(pos.z + 3. + .1, pos.y + 1.8 + .1));
}

float Ball(vec3 pos){
    return length(pos - sin(3.*time)*vec3(0., .9, 0.)) - .75;
}

float Map(vec3 pos){
    return min(Ball(pos), min(Pool(pos), WaterSurface(pos)));
}

bool isWaterSurface(vec3 pos){
    return Map(pos) == WaterSurface(pos);
}

bool isWater(vec3 pos){
    return (pos.y < WaterWaves(pos) || pos.y < 0.);
}
                                      
bool isPool(vec3 pos){
    return (Map(pos) == Pool(pos));                                  
}

bool isBall(vec3 pos){
    return (Map(pos) == Ball(pos));
}

vec3 GetNormal(vec3 pos){
    vec3 eps = vec3(0.001, 0., 0.);
    return normalize(vec3(Map(pos + eps.xyz) - Map(pos - eps.xyz),
                     Map(pos + eps.yxz) - Map(pos - eps.yxz),
                     Map(pos + eps.yzx) - Map(pos - eps.yzx)));
}
vec3 lPos = vec3(2., 1.5, 0.);
float Occlusion(vec3 pos, vec3 normal){
    float t = 0.;
    normal = normalize(normalize(lPos) - normalize(pos)); //comment to use the regular occlusion
    for(int i = 1; i < 10; i++){
        float d = Map(pos + normal*.05*float(i));
        t += max(d/float(i), 0.);
    }
    return clamp(t, 0., 1.);
}
float RefScene(vec3 ro, vec3 rd){
    vec3 col = vec3(0.), pos = vec3(0.);
    float t = 0.03;
    for(int i = 0; i < 100; i++){
        float d = Map(ro+rd*t);
        t += d;
        if(d < .001*t) break;  
    }
    return t;
}
vec3 Lighting(vec4 pos, vec3 ro, vec3 rd, vec3 normal){
    vec3 lDir = lPos - pos.xyz;
    vec3 l = vec3(max(dot(normal, normalize(lDir)), 0.));// = vec3(Occlusion(pos.xyz, GetNormal(pos.xyz)));
    float t = RefScene(pos.xyz, normalize(lDir));
    if (t < length(lDir)) {
        vec3 p = pos.xyz + normalize(lDir) * t;
        if(!(pos.w == 1. || pos.w == 2.) && Map(pos.xyz) != pos.y + 1.8 + .1)
            return vec3(-0.02);
    }
    if(pos.w == 0.){
        //ball
        l *= vec3(.5, .56, 1.);
        l += pow(max(dot(normalize(normalize(ro - pos.xyz) + normalize(lDir)), normal), 0.), 30.);
    }
    else if(pos.w == 1.){
        //water surface  
        l *= vec3(0.4, 0.9, 1);
        l += 120./25.*pow(max(dot(normalize(normalize(ro - pos.xyz) + normalize(lDir)), normal), 0.), 120.);
        //l *= vec3(0.4, 0.9, 1);
    }
    else if(pos.w == 2.){
        //water
        l *= vec3(0.4, 0.9, 1);
    }
    else{
        //pool tiles. do shadows
    }
    return l / dot(lDir, lDir);
}

vec3 Scene(vec3 ro, vec3 rd){
    vec3 col = vec3(0.), pos = vec3(0.);
    float t = 0.;
    for(int i = 0; i < 60; i++){
        float d = Map(ro+rd*t);
        if(t >= 20.) break;
        if(d < .001*t){
            float m = 0.;
            pos = ro +rd*t;
            vec3 col = vec3(0.), normal = GetNormal(pos);//vec3(Occlusion(pos.xyz, GetNormal(pos.xyz)));
            if(isBall(pos)){
                m = 0.;
                col = Occlusion(pos, normal)*vec3(.5, .56, 1.);
            }
            else if(isWaterSurface(pos)){
                col = Occlusion(pos, normal)*vec3(.4, .7, 1.);
                vec3 ref = refract(rd, normal, .9);
                vec3 l = Lighting(vec4(pos, 1.), ro, rd, normal);
                float t = RefScene(pos.xyz, ref);
                vec4 refPos = vec4(pos.xyz + ref*t, 0.);
                vec3 refNormal = GetNormal(refPos.xyz);
                if(isBall(refPos.xyz)){
                    refPos.w = 0.;
                    col += Occlusion(refPos.xyz, refNormal)*.1*vec3(.5, .56, 1.);
                }
                else{
                    vec3 floo = vec3(0.);
                    refPos.w = 3.;
                    vec3 c = refPos.xyz;
                    m = 3.;
                    refPos.xyz = mod(refPos.xyz, vec3(.3));
                    if(refPos.x > .26 || refPos.y > .26 || refPos.z > .26)
                        floo = vec3(0.);
                    else floo = vec3(sin(refPos.x + refPos.y - refPos.z + 1.));//vec3(sin(floor((refPos.x + 1.) / .2)) * cos(floor((refPos.y + 1.) / .2)) * sin(floor((refPos.z + 1.) / .2)) + 3.);
                    refPos.xyz = c;
                    col += .1*Occlusion(refPos.xyz, refNormal)*floo;
                }
                col += Lighting(refPos, ro, rd, refNormal);
                col *=  vec3(0.4, 0.9, 1.);
                col += l;
            }
            else if(isWater(pos)){
                if(pos.y > 1. || pos.z > 3. || pos.x > 3.) return vec3(0.);
                m = 2.;
                col = Occlusion(pos, normal)*vec3(.4, .7, 1.);
                vec3 l = Lighting(vec4(pos, 1.), ro, rd, normal);
                {
                    vec3 floo = vec3(0.);
                    vec3 c = pos.xyz;
                    m = 3.;
                    pos.xyz = mod(pos.xyz, vec3(.3));
                    if(pos.x > .26 || pos.y > .26 || pos.z > .26)
                        floo = vec3(0.);
                    else floo = vec3(sin(pos.x + pos.y - pos.z + 1.));//vec3(sin(floor((pos.x + 1.) / .2)) * cos(floor((pos.y + 1.) / .2)) * sin(floor((pos.z + 1.) / .2)) + 3.);
                    pos.xyz = c;
                    col += .1*Occlusion(pos.xyz, normal)*floo;
                }
                col *=  vec3(0.4, 0.9, 1.);
                col += l;
            }
            else{
                if(pos.y > 0.9 || pos.z > 3. || pos.x > 3.) return vec3(0.);
                vec3 c = pos.xyz;
                m = 3.;
                pos.xyz = mod(pos.xyz, vec3(.3));
                if(pos.x > .26 || pos.y > .26 || pos.z > .26)
                    col = vec3(0.);
                else col = Occlusion(c, normal)*vec3(sin(pos.x + pos.y - pos.z + 1.));//sin(floor((pos.x+1.)/.3))*cos(floor((pos.y+1.)/.3))*sin(floor((pos.z+1.)/.3))+1.//vec3(sin(pos.x + pos.y + pos.z),
                pos.xyz = c;
            }
            col += Lighting(vec4(pos, m), ro, rd, normal);
            return col;
        }
        t += d;
    }
    return col;
}

void main(void)
{
    gl_FragColor = vec4(0., 0., 0., 1.);
    vec2 v = 2.* uv - 1.;
    v.x *= 400. / 400.;
    vec3 ro = vec3(3., 2., 6.);
    vec3 eye = normalize(vec3(0., -1., 0.) - ro);
    vec3 right = cross(eye, normalize(vec3(0., 1., 0.)));
    vec3 up = cross(right, eye);
    vec3 rd = normalize(mat3(right, up, eye)*vec3(uv, 2.));
    gl_FragColor = vec4(Scene(ro, rd),1.0);
}