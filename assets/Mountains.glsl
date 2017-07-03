uniform vec2 uv;
uniform float time;

float treeLine;
float treeCol;

float Hash(vec2 p) {
    p = fract(p / vec2(3.07965, 7.4235));
    p += dot(p, p.yx + 19.19);
    return fract(p.x * p.y);
}

vec2 Hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx / vec3(3.07965, 7.1235, 4.998784)));
    p3 += dot(p3.zxy, p3.yxz + 19.19);
    return fract(vec2(p3.x * p3.y, p3.z * p3.x));
}

vec2 Noise3(vec2 p) {
    p = floor(p);
    vec2 f = fract(p);
    f = f * f * (3. - 2. * f);
    float n = p.x + p.y * 57.;
    return mix(mix(Hash22(p), Hash22(p + vec2(0., 1.)), f.x),
        mix(Hash22(p + vec2(1., 0.)), Hash22(p + vec2(1.)), f.x), f.y);
}

float Noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * f * (6.0 * f * f - 15.0 * f + 10.0);
    return mix(mix(Hash(i), Hash(i + vec2(1., 0.)), f.x),
        mix(Hash(i + vec2(0., 1.)), Hash(i + vec2(1.)), f.x), f.y);
}
float Noise2(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * f * (6.0 * f * f - 15.0 * f + 10.0);
    return fract(mix(mix(Hash(i), Hash(i + vec2(2., 0.)), f.x),
        mix(Hash(i + vec2(0., 2.)), Hash(i + vec2(2.)), f.x), f.y));
}
float Terrain(vec2 p, int count) {
    vec2 pos = p * 0.02;
    float w = (Noise(pos * .25) * 0.75 + .15);
    w = 86.0 * w * w;
    float h = 0.;
    for (int i = 0; i < 5; i++) {
        h += pow((w * (Noise(pos))), Noise(pos));
        w *= -0.5;
        pos *= mat2(1.2323, 1.999231, -1.999231, 1.22);
    }
    return h + pow(abs(Noise(p * 0.02)), 5.) * 55. - 5.;
}

float Trees(vec2 p) {
    return treeLine * Noise(p * 13.);
}

float Terrain2(vec2 p, int count) {
    vec2 pos = p * 0.05;
    float w = (Noise(pos * .25) * 0.75 + .15);
    w = 66.0 * w * w;
    float h = 0.;
    for (int i = 0; i < 5; i++) {
        h += pow((w * (Noise(pos))), Noise(.4 * pos)) + w * 0.2 * Noise(1.2 * pos);
        w *= -0.4;
        pos *= 2. * mat2(0.866, 0.5, -0.5, 0.866);
    }
    float ff = Noise(pos * .002);
    h += pow(abs(ff), 5.0) * 55. - 5.0;

    treeCol = Trees(p);
    h += treeCol;
    for (int i = 0; i < 7; i++) {
        h += w * Noise(pos);
        w *= -0.5;
        pos *= 2. * mat2(0.866, 0.5, -0.5, 0.866);
    }
    return h;
}


float Map(vec3 p) {
    float h = Terrain(p.xz, 5);

    float ff = Noise(p.xz * 3.0) + Noise(p.xz * 3.3) * .5;
    treeLine = smoothstep(ff - 2., ff * 2. + 4., h) * smoothstep(1.0 + ff * 3., .4 * ff + 1., h);
    treeCol = Trees(p.xz * 10.);
    h += 10. * treeCol;

    return p.y - h;
}

float SphereRadius(float t) {
    t = abs(t - 10.);
    t *= 0.01;
    return clamp(t * t, 50. / 705., 80.);
}

bool Scene(vec3 ro, vec3 rd, out float t, vec2 fragCoord) {
    bool hit = false;
    float s = 0.;
    t = 1.2 + Hash(fragCoord);
    float sphereR = SphereRadius(t);
    for (int i = 0; i < 105; i++) {
        if (s > .8) {
            hit = true;
            break;
        }
        if (t > 3900.) {
            if (s != 0.) {
                hit = true;
            }
            break;
        }
        vec3 p = ro + rd * t;
        float h = Map(p);
        sphereR = SphereRadius(t);
        if (h < sphereR) {
            s = (1. - s) * (sphereR - h) / (sphereR);
            hit = true
        }
        t += 0.5 * h + 0.003 * t;
    }
    return hit;
}
vec3 sunLight = normalize(vec3(0.5, 0.5, 0.1));
vec3 sunColour = vec3(1.0, .9, .83);
float specular = 0.0;
float ambient;
void DoLighting(inout vec3 mat, in vec3 pos, in vec3 normal, in vec3 eyeDir, in float dis) {
    float h = dot(sunLight, normal);
    float c = max(h, 0.0) + ambient;
    mat = mat * sunColour * c;
    if (h > 0.0) {
        vec3 R = reflect(sunLight, normal);
        float specAmount = pow(max(dot(R, normalize(eyeDir)), 0.0), 100.0) * specular;
        mat = mix(mat, sunColour, specAmount);
    }
}
vec3 GetNormal(vec3 p, float distance) {
    float t = min(.3, .0005 + .00005 * distance * distance);
    vec3 eps = vec3(t, 0., 0.);
    vec3 nor = (vec3(0., Terrain2(p.xz, 10), 0.));
    vec3 v2 = nor - vec3(eps.x, Terrain2((eps + p).xz, 10), 0.);
    vec3 v3 = nor - vec3(0., Terrain2((p - eps.yyx).xz, 10), -eps.x);
    return normalize(cross(v2, v3));
}

vec3 renderRefl(vec3 ro, vec3 rd, vec2 fragCoord) {
    vec3 col = vec3(0.2, 0.3, 0.6);
    float t;
    if (Scene(ro, rd, t, fragCoord)) {
        vec3 normal = GetNormal(ro + rd * t, t);
        vec3 mat;
        ambient = 0.1;
        specular;
        vec3 matPos = (ro + rd * t) * 2.;
        float d = t * t;

        float f = clamp(Noise(matPos.xz * 0.05), 0., 1.);
        vec3 m = mix(vec3(0.63 * f + .2, .7 * f + .1, .7 * f + .1), vec3(f * .43 + .1, f * .3 + .2, f * .34 + .1), f * 0.65);
        mat = m * vec3(f * m.x + .36, f * m.y + .3, f * m.z + .28);

        if (matPos.y < 25. || normal.y > .45) {
            m = vec3(Noise(matPos.xz * 0.035) * .5 + .15, Noise(matPos.xz * .03) * .6 + .25, 0.);
            m *= (normal.y - 0.65) * .6;
            mat = mix(mat, m, clamp((normal.y - .65) * 1.3 * (45. - matPos.y) * .1, 0., 1.));
        }
        col = 0.2 * mat;
    }
    return col;
}

vec3 TerrainColour(vec3 ro, vec3 pos, vec3 normal, float dis, vec2 fragCoord) {
    vec3 mat;
    ambient = 0.1;
    specular;
    vec3 dir = pos - ro;
    vec3 matPos = pos * 2.;
    float d = dis * dis;

    float f = clamp(Noise(matPos.xz * 0.05), 0., 1.);
    vec3 m = mix(vec3(0.63 * f + .2, .7 * f + .1, .7 * f + .1), vec3(f * .43 + .1, f * .3 + .2, f * .34 + .1), f * 0.65);
    mat = m * vec3(f * m.x + .36, f * m.y + .3, f * m.z + .28);
    if (normal.y < .5) {
        float c = (.5 - normal.y) * 4.;
        c = clamp(c * c, 0.1, 1.);
        f = Noise(vec2(matPos.x * .09, matPos.z * .095) + matPos.yy * .15);
        f += Noise(vec2(matPos.x * 2.233, matPos.z * 2.23)) * .5;
        mat = mix(mat, vec3(.4 * f), c);
        specular += .1;
    }
    if (matPos.y < 25. || normal.y > .45) {
        m = vec3(Noise(matPos.xz * 0.035) * .5 + .15, Noise(matPos.xz * .03) * .6 + .25, 0.);
        m *= (normal.y - 0.65) * .6;
        mat = mix(mat, m, clamp((normal.y - .65) * 1.3 * (45. - matPos.y) * .1, 0., 1.));
    }
    if (treeCol > 0.) {
        mat = vec3(.02 + Noise(matPos.xz * 5.) * .03, .05, .0);
        normal = normalize(normal + vec3(Noise(matPos.xz * 33.) - .5, 0., Noise(matPos.xz * 3.) - .5));
        specular = 0.;
    }
    if (matPos.y > 35. || normal.y >= .42) {
        float snow = Noise(matPos.xy * 0.0005) * clamp((matPos.y - 35.0 - Noise(matPos.xz * .1) * 28.0) * 0.035, 0.0, 1.0);
        mat = mix(mat, vec3(.7, .7, .8), snow);
        specular += snow;
        ambient += snow * .3;
    }
    if (matPos.y < 2.) {
        if (normal.y > .4) {
            f = Noise(matPos.xz * 0.084) * 1.5;
            f = clamp((2.0 - f - matPos.y) * 1.35, 0., 0.67);
            float t = normal.y - .4;
            t = t * t;
            mat = mix(mat, vec3(.09 + t, .07 + t, .03 + t), f);
        }
    }
    if (matPos.y < 0.) {
        mat *= .5;
    }
    DoLighting(mat, pos, normal, dir, dis * dis);

    if (matPos.y < 0.) {
        float time = (time * .03);
        vec3 watPos = matPos;
        watPos += -dir * (watPos.y / dir.y);

        float tx = cos(watPos.x * .052) * 4.5;
        float tz = sin(watPos.z * 0.072) * 4.5;
        vec2 co = Noise3(vec2(watPos.x * 4.7 + 1.3 * tz, watPos.z * 4.69 + time * 35. - tx));
        mat *= 0.5;
        mat += .5 * mix(vec3(0.1, 0.1, 0.5), vec3(0.8, 0.8, 0.72) * 0.7, clamp(dot(normal, sunLight) * .3, 0.2, 0.9));
        pos.y = 0.;
        pos.x += tx;
        pos.y += tz;
    }
    float fog = exp(-.00005 * dis * dis);
    return mix(vec3(0.9, 0.9, 0.9), mat, fog);
}

void main(void) {
    vec3 normal;
    vec2 p = 2. * uv - 1.;
    float x = 10. + 10. * time;
    float y = 5.;
    float z = 8. + 10. * time;

    vec3 ro = vec3(x, y, z);
    ro.y += Terrain(ro.xz, 5);
    vec3 lookAt = .5 * ro;

    vec3 eye = normalize(ro - lookAt);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));

    vec3 rd = normalize(eye * 0.8 + right * p.x * 0.8 + up * p.y);

    float distance;
    vec3 col;

    if (!Scene(ro, rd, distance, uv) || distance > 500.) {
        gl_FragColor = vec4(0.7, 0.7, 0.72, 0.);
        float fog = exp(-.0005 * distance * distance);
        gl_FragColor = mix(vec4(0.9, 0.9, 0.9, 1.), gl_FragColor, fog);
        return;
    } else {
        vec3 pos = ro + rd * distance;
        normal = GetNormal(pos, distance);

        col = TerrainColour(ro, pos, normal, distance, uv);
    }
    gl_FragColor = vec4(col, 1.);
}