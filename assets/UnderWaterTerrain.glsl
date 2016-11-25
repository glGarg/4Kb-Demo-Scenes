varying vec2 uv;
uniform float time;
uniform vec2 camera;

#define PI 3.14159
#define SAND 0

float Hash(float n) {
    return fract(sin(n) * 3355.545383);
}

float Noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3. - 2. * f);
    float n = p.x + 157. * p.y + 113. * p.z;
    return mix(mix(mix(Hash(n + 0.), Hash(n + 1.), f.x),
            mix(Hash(n + 157.), Hash(n + 158.), f.x), f.y),
        mix(mix(Hash(n + 113.), Hash(n + 114.), f.x),
            mix(Hash(n + 270.), Hash(n + 271.), f.x), f.y), f.z);
}

vec3 Noise( in vec2 x) {
    x += 4.2;
    vec2 p = floor(x);
    vec2 f = fract(x);

    //vec2 u = f*f*(3.0-2.0*f);
    vec2 u = f * f * f * (6.0 * f * f - 15.0 * f + 10.0);
    float n = p.x + p.y * 507.0;

    float a = Hash(n + 0.0);
    float b = Hash(n + 1.0);
    float c = Hash(n + 507.0);
    float d = Hash(n + 508.0);
    return 1. * vec3(a + (b - a) * u.x + (c - a) * u.y + (a - b - c + d) * u.x * u.y,
        6.0 * f * (f - 1.0) * (vec2(b - a, c - a) + (a - b - c + d) * u.yx));
}

mat2 m = mat2(cos(PI / 3.), sin(PI / 3.), -sin(PI / 3.), cos(PI / 3.));

vec3 Fbm( in vec2 p) {
    vec3 h = vec3(0.);
    h = 5.0 * Noise(p - vec2(0., 12.));
    p *= 2.02 * m;
    h += 2.5 * Noise(p);
    p *= 2.33 * m;
    h += 1.25 * Noise(p);
    p *= 2.01 * m;
    h += 0.625 * Noise(p);
    p *= 2.01 * m;
    h += 0.3125 * Noise(p);
    p *= 2.33 * m;
    //h+= 0.150625*Noise(p); p*= 2.01*m;
    //h+= 0.0703125*Noise(p); p*= 2.01*m;
    return h;
}

float Fbm( in vec3 p) {
    float h = 0.;
    float t = 1.;

    h = Noise(p + vec3(0., 0., 0.));
    p *= 2.02 * t;
    h += 2.5 * Noise(p);
    p *= 2.33 * t;
    h += 1.25 * Noise(p);
    p *= 2.01 * t;
    h += 0.625 * Noise(p);
    p *= 2.01 * t;
    return h;
}

float Terrain(vec3 p) {
    float density = 0.;
    vec3 pos = p;
    //float hard_floor = -4.;
    //pos += clamp((hard_floor-p.y)*3., -0., 1.)*40.;	

    float freq = 4.;
    float amp = .25;
    for (int i = 0; i < 11; i++) {
        density += Noise(pos * freq) * amp;
        amp *= 2.;
        freq *= .5;
    }
    float hardFloor = -13.;
    density += clamp((hardFloor + p.y) * 4., 0., 1.) * 40.; //try 1. instead of 4.
    density = clamp(density, 0., 420.);
    float distanceFlat = 0.1 * length(p.xz - vec2(-100.)) / 500.;
    return mix(density, p.y + 310., distanceFlat);
}

float map(vec3 p) {
    return p.y - Terrain(p);
}

vec3 GetNormal(vec3 p, float t) {
    vec3 eps = vec3(25., 0., 0.);
    vec3 normal = vec3(0., map(p), 0.);
    vec3 v2 = normal - vec3(eps.x, map(p + eps), 0.);
    vec3 v3 = normal - vec3(0., map(p - eps.yyx), -eps.x);
    return normalize(cross(v2, v3));
}

vec3 GetNormalH(vec3 p) {
    vec3 eps = vec3(0.1, 0., 0.); //5 gave big reflections
    vec3 normal = vec3(0., map(p), 0.);
    vec3 v2 = normal - vec3(eps.x, map(p + eps), 0.);
    vec3 v3 = normal - vec3(0., map(p - eps.yyx), -eps.x);
    return normalize(cross(v2, v3));
}

bool Scene( in vec3 ro, in vec3 rd, out float t) {
    bool hit = false;
    for (int i = 0; i < 200; i++) {
        if (hit || t > 2600.) break;
        float h = map(ro + rd * t);
        if (h < 0.1) {
            hit = true;
        }
        t += h * 0.4 + 0.003 * t;
    }
    t = abs(t);
    return hit;
}

void main() {
    gl_FragColor = vec4(0., 0., 0., 1.);
    vec2 p = uv;
    p.x *= 400. / 400.;

    /*if (abs(p.y) > 0.8) {
    	gl_FragColor = vec4(0.);
        return;
    }*/

    float x = 1.; // +(6.*time);
    float y = 60.;
    float z = 6.; //*time;

    vec3 ro = camera + vec3(x, y, z);
    ro.y = 260. + Terrain(ro);

    vec3 lookAt = ro * .5;

    vec3 eye = normalize(lookAt - ro);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    float effect = 0.;
    vec3 rd = normalize(0.8 * eye + p.x * right + p.y * up);
    float t = 0.;
    vec3 normal = vec3(0.), col = vec3(0.), lCol = vec3(0.22, 0.2, 0.7);
    if (Scene(ro, rd, t)) {
        normal = GetNormal(ro + rd * t, t);
        col = vec3(0.172, 0.135, .241);
        vec3 lPos = normalize((ro + rd * t) + vec3(0., 900., 0.));

        float ambient = 0.3;
        col += ambient * lCol;
        float a = 0.;
        col += .7 * max(dot(normal, normalize(lPos)), 0.) * lCol;

#if SAND
        col = mix(vec3(0.8, 0.8, 0.1), vec3(0.6, 0.5, 0.1), 0.2);
#else
        col = mix(vec3(0.1, 0.4, 0.7), normal, 0.9); //colorful
#endif
        col += 0.3 * lCol * Noise(max(sin(0.0625 * time), 0.2) * 0.05 * (ro + rd * t));
        col -= 0.3 * lCol * (Noise(max(cos(0.125 * time), 0.2) * 0.025 * (ro + rd * t)));
        col += 0.1 * lCol * (Noise(max(cos(.25 * time), 0.2) * 0.0125 * (ro + rd * t)));
        col -= 0.225 * lCol * Noise(max(sin(0.258 * time), 0.2) * 0.0625 * (ro + rd * t));
        vec3 nor = (normalize(normal - .5 + GetNormal(0.3 * (ro + rd * t), 2.)));
        float sky = clamp(.5 + .5 * nor.y, 0., 1.);
        lCol += 0.7 * sky * lCol;
        col += .7 * max(dot(normal, normalize(lPos)), 0.) * lCol;
        effect += 0.3 * (max(cos(0.125 * time), 0.2));
        effect += 0.1 * max(cos(.25 * time), 0.2);
        effect += 0.225 * max(sin(0.25 * time), 0.2);
        effect += .3 * max(sin(0.0625 * time), 0.2);
        col += 0.5 * effect * pow(dot(lPos, reflect(normalize(ro + rd * t), GetNormalH(ro + rd * t))), 150.);
    } else {
        col = mix(vec3(0.2, 0.2, 0.4), vec3(0.1, 0.1, .5), 1. - p.y);
    }

    col = mix(vec3(0.04, 0.04, 0.10), col, clamp(exp(-t / 50. * 0.07), 0., 0.7));
    col += clamp(1. - effect, 0., 1.) * mix(vec3(0.), vec3(0.0, 0.2, 0.7), p.y);

    //another fog
    col = mix(vec3(0.04, 0.2, 0.30), col, clamp(exp(length(ro + rd * t) / 50. * 0.01), 0., 0.8));
    gl_FragColor = vec4(1.35 * col + .1, 1.);
}