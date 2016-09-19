varying vec2 uv;
uniform float time;

//tree stuf
float treeLine = 0.;
float treeCol = 0.;

float Hash(vec2 co){
    float a = 12.98;
    float b = 78.23;
    float c = 438.553;
    vec2 s = vec2(a, b);
    float dt = dot(co, s);
    float sn = mod(dt, 3.14);
    return fract(sin(sn)*c);
}

/*float Hash(vec2 p){
    p = fract(p/vec2(3.07965, 7.4235));
    p += dot(p, p.yx+19.19);
    return fract(p.x*p.y);
}*/

float Noise(vec2 p){
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f*f*(3. - 2.*f);
    return mix(mix(Hash(i), Hash(i + vec2(1., 0.)), f.x),
               mix(Hash(i + vec2(0., 1.)), Hash(i + vec2(1.)), f.x),f.y);
}

float Terrain(vec2 p){
    vec2 pos = 0.05*p;
    float w = (Noise(pos*.25)*.75 + .15), h = 0.;
    w= 66.*w*w;
    for(int i = 0; i < 9; i++){
        h += Noise(pos)*(w);
        w *= -.4;
        pos *= 1.*mat2(1.2323, 1.999231, -1.999231, 1.22);
    }
    
    return h + pow(abs(Noise(p*0.002)) , 5.)*255. - 5.; //add variation to the value
}

//tree stuf
float Trees(vec2 pos){
    return Noise(pos*13.) *treeLine;
}

float Terrain2(vec2 p){
    vec2 pos = 0.05*p;
    float w = (Noise(pos*.25)*.75 + .15), h = 0.;
    w= 66.*w*w;
    for(int i = 0; i < 15; i++){
        h += w*(Noise(pos));
        w *= -.4;
        pos *= mat2(1.2323, 1.999231, -1.999231, 1.22);
    }
    
    //tree stuf
    /*float f = Noise(pos*0.02);
    h += pow(abs(f), 5.)*55. - 5.;
    treeCol = Trees(p);
    h += treeCol;*/
    //h+=pow(abs(Noise(p*0.01)) , 5.)*255. - 5.;
    
    for(int i = 0; i < 15; i++){
        h += (w*Noise(pos));
        w *= -.4;
        pos *= mat2(1.2323, 1.999231, -1.999231, 1.22);
    }
    return h; //add variation to the value
}

float SphereTrace(float t){
    t = abs(t - 10.);
    t = 0.01*t;
    return clamp(t*t, 50./400./*iResolution.y*/, 80.);
}

vec3 GetSky(vec3 rd, vec3 light, vec3 color){
    float sun = clamp(dot(rd, light), 0., 1.);
    float v = pow(1. - max(rd.y, 0.), 2.);
    vec3 sky = mix(vec3(0.4, 0.4, 0.8), vec3(0.8, 0.2, 0.8), v);
    //sun = pow(sun, 50.);
    sky += color * v * v * .15;
    sky += vec3(1.) * pow(sun, 700.);
    return clamp(sky,0., 1.);
}

float SphereIntersect(vec3 ro, vec3 rd, float t, vec3 center, float r){
    /*vec3 oc = center - ro;
    float tca = dot(oc, rd);
    float d = dot(oc, oc) - tca*tca;
    if(d < 0.)return -1.;
    float thc = r*r - d*d;
    return tca - thc;*/
    vec3 oc = ro - center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - r*r;
    float h = b*b - c;
    if(h < 0.)return -1.; //no intersect
    return -b - sqrt(h);
}

float map(vec3 pos){
    float h = Terrain(pos.xz);
    
    //tree stuf
    //float f = Noise(pos.xz*3.) * Noise(pos.xz*3.3) * .5;
    //treeLine = smoothstep(f*0.9 + 4., f*.5 + 2., h)*(smoothstep( f*0.4 - 1., f*0.4+.3, h));
    //treeCol = Trees(pos.xz);
    
    return pos.y /*- treeCol*4.*/ - h;
}

vec3 GetNormal(vec3 pos, float distance){ //replace with terrain2 to give more previse height ratios
    vec3 eps = vec3(min(0.3, 0.005+0.0005*distance*distance), 0., 0.);
    vec3 nor = vec3(0., Terrain2(pos.xz), 0.);
    vec3 v2 = nor - vec3(eps.x, Terrain2((pos + eps).xz), 0.);
    vec3 v3 = nor - vec3(0., Terrain2((pos - eps.yyx).xz), -eps.x);
    return normalize(cross(v2, v3));
}

bool Scene(vec3 ro, vec3 rd, out float dis, vec2 fragCoord){
    bool hit = false;
    float h = 0., sphereR = 0., s = 0.;
    for(int i = 0; i < 250; i++){
        if(dis > 1000.){
            hit = false;
            break;
        }
        if(hit){   
            break;
        }
        h = map((ro+rd*dis));
        if(h < 0.5){
            s += 1.;
            if(s > 5.)
                hit = true;
        }
        dis += h*0.5 + .001*dis;
    }
    
    return hit;
}

bool Scene2(vec3 ro, vec3 rd, out float dis, vec2 fragCoord, out float h){
    bool hit = false;
    float sphereR = 0., s = 0.;
    for(int i = 0; i < 100; i++){
        if(dis > 300.){
            return false;
        }
        h = map((ro+rd*dis));
        if(h < 0.1){
            return true;
        }
        dis += h*0.5+dis*.003;
    }
    
    return hit;
}

//plan to do later
vec3 Render(vec3 ro, vec3 rd, float t, vec3 normal){
    vec3 col = vec3(0.);
    vec3 p = 2.*(ro+rd*t);
    float f = clamp(Noise(p.xz*0.05), 0., 1.);
    vec3 m = mix(vec3(0.35+f*0.41 ,0.1+f*0.22 ,0.65+f*0.45 ), vec3(0.1+f*0.41,.1+f*.42 ,.15+f*.4), f*0.62);
    col = m*vec3(0.7*m.x + 0.5, 0.6*m.y + 0.4, 0.6*m.z+0.55);
    return col;
}



float sdSphere(vec3 r, float s){
    return length(r) - s;
}

void main()
{
    gl_FragColor = vec4(0.);
    vec3 normal = vec3(0.);
    vec2 p = 2.*uv;
    p -= 1.;
    p.x *= 400./400.;
    
    vec2 mouse = vec2(0.);//iMouse.xy/iResolution.xy;
    
    /*if(abs(p.y) > 0.85){
        gl_FragColor = vec4(0.);
        return;
    }*/

    float x = 10. + 10.*cos(0.002*time);
    float y = 2.+0.25*sin(5.*time);
    float z = 8. + 400.*sin(0.002*time) + 6.*mouse.x;
    
    vec3 ro = vec3(x, y, z);
    ro.y += Terrain(ro.xz);
    
    vec3 lookAt = 0.5*ro;//vec3(ro.x, ro.y*0.5, 0.);
    
    vec3 eye = normalize(ro - lookAt);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    
    vec3 rd = normalize(eye*0.8 + right*p.x*0.8 + up*p.y);
    
    float distance = 0.;
    vec3 col = vec3(0.);
    vec3 lCol = vec3(0.4);
    vec3 lPos = normalize(vec3(0.2, .2, 0.2));//*mat3(cos(0.05*iGlobalTime), sin(0.05*iGlobalTime), 0.,
                                                 // -sin(0.05*iGlobalTime), cos(0.05*iGlobalTime), 0.,
                                                  //0., 0., 0.));
    if( !Scene(ro, rd, distance, gl_FragCoord.xy )|| distance > 400.){
        col = GetSky(rd, lPos, lCol);
        //fragColor = vec4(0.0, 0.0, 0.12, 0.);
        //float t = SphereIntersect(ro, rd, 1., vec3(45000., 30000., -30000.), 14000.);
        //vec3 moonColor;
        //if(t > 0.){
        //  vec3 moon = ro + rd*t;
        //    vec3 nor = normalize(moon - vec3(45000., 35000., -15000.));
            
         //   moonColor = texture2D(iChannel1, 0.0001*(rd).xz, -100.).x*lCol*max(pow(dot(-nor, lPos), 2.), 0.);
        //}
        //else
        {
            //col += 8.*pow(texture2D(iChannel0, 4.*(rd).xz, -100.).x, 100.);
            //col*=0.7;
            col += pow(0.1*Noise((ro+rd*distance).xz), 1000.); //stars try
            gl_FragColor = vec4(col, 1.);
            return;
        }
        
        //col = mix(moonColor, col, 0.8);
        //gl_FragColor = vec4(col, 1.);
        
    }
    else{
        vec3 pos = ro + rd*distance;
        normal = GetNormal(pos, distance);
        col = Render(ro, rd, distance, normal);//TerrainColour(ro, pos, normal, distance, gl_FragCoord.xy);
        float effect = 0.;//= 0.3;
        effect += .7*max(dot(normal, lPos), 0.);
        //normal = reflect(eye, normal);
        //effect += 0.5*pow(dot(normal, lPos), 200.);
        //effect += 0.7*pow(max(dot(normal, normalize(normalize(lPos)-normalize(ro+rd*distance))), 0.), 100.); //specular
        float t = distance;
        col += effect*lCol;//+.1*pow(max(dot(normal, normalize((lPos)-normalize(ro+rd*t))), 0.), 600.);
        
        vec3 dir = normalize(lPos - (ro+rd*t));
        float h = 0.;
        if(Scene2((ro+rd*t), dir, t, gl_FragCoord.xy, h)){
            col *= 0.8;
            col *= clamp(exp(h/t), 0., 1.);
        }
    }
    gl_FragColor = vec4(0.);
    //col += pow(dot(normal, normalize(vec3(1., 1., 1.))), 2.)*0.5;
    gl_FragColor = vec4(col, 1.);    
}