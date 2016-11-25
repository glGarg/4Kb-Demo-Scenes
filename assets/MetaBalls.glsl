varying vec2 uv;
varying vec2 texCoord;
uniform samplerCube box;
uniform float time;
uniform vec2 resolution;

#define count 5

vec3 bPos[count];

float Hash1(float n) {
    return fract(sin(n) * 3375.545383);
}

float Hash2(float n) {
    return fract(sin(n) * 3391.54397);
}

float HashMix(float m, float n, float lerp) {
    return mix(Hash1(m), Hash2(n), lerp);
}

float Noise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f * f * (3. - 2. * f);
    float n = p.x + 157. * p.y + 113. * p.z;
    return mix(mix(mix(HashMix(n + 0., n, 0.3), HashMix(n + 1., n + 1., .3), f.x),
            mix(HashMix(n + 157., n + 157., .3), HashMix(n + 158., n + 158., .3), f.x), f.y),
        mix(mix(HashMix(n + 113., n + 113., .3), HashMix(n + 114., n + 114., .3), f.x),
            mix(HashMix(n + 270., n + 270., .3), HashMix(n + 271., n + 271., .3), f.x), f.y), f.z);
}

float smin(float a, float b) {
    float s = exp(-2.5 * a) + exp(-2.5 * b);
    return -log(s) / 2.5;
}

float Balls(vec3 p, float s) {
    float value = 100.;
    float m = 0.;
    float delta = 100.;
    for (int i = 0; i < count; i++) {
        vec3 np = p - bPos[i];
        float l = length(np) - 0.55; //-.2*length(texture2D(iChannel1, 200.*bPos[i].xz));
        delta = smin(l, delta);
    }
    return delta;
}

vec2 Min(vec2 a, vec2 b) {
    return (a.x < b.x) ? a : b;
}

vec2 Map(vec3 pos) {
    //pos.xz = mod(pos.xz, 15.)-7.5;
    vec2 res = Min(vec2(10.), vec2(Balls(pos, .8), 32.));
    return res;
}

vec2 Scene(vec3 ro, vec3 rd) {
    float t = 0., m = 0., s = 1.;
    float k = 0.;
    for (int i = 0; i < 290; i++) {
        vec3 pos = ro + rd * t;
        vec2 res = Map(pos);
        k = max(res.x, k);
        if (res.x < 0.0001 || t > 50.)
            break;
        t += res.x * 0.5;
        m = res.y;
    }
    if (t > 50.)
        m = -1.;
    return vec2(t, m);
}

vec3 CalcNormal(vec3 pos) {
    vec3 eps = vec3(0.01, 0., 0.);
    vec3 nor = (vec3(Map(pos + eps).x - Map(pos - eps).x,
        Map(pos + eps.yxy).x - Map(pos - eps.yxy).x,
        Map(pos + eps.yyx).x - Map(pos - eps.yyx).x));
    return normalize(nor);
}

vec3 RenderRefl(vec3 ro, vec3 rd) {
    vec3 col = vec3(0.), lPos = normalize(vec3(.3, 1., 0.5)), normal = vec3(0.), lCol = vec3(0.3, 0.3, 0.3);
    vec2 res = Scene(ro, rd);
    float t = res.x;
    vec3 pos = ro + rd * t;
    normal = CalcNormal(pos);
    if (res.y >= 32.) { //sphere
        col = mix(vec3(0.5, 0., 0.4), vec3(0.8, 0.1, 0.2), normal.y);
        col += texture(box, reflect(normalize(-pos), normal)).xyz;
    }
    return col;
}

vec3 Render(vec3 ro, vec3 rd) {
    vec3 col = vec3(0.), lPos = normalize(vec3(0., 1., 1.)), normal = vec3(0.), lCol = vec3(1.);
    vec2 res = Scene(ro, rd);
    float t = res.x;
    vec3 pos = ro + rd * t;
    normal = CalcNormal(pos);
    if (res.y >= 32.) { //sphere
        col += .7 * texture(box, reflect(normalize(-pos), normal)).xyz;
        col += pow(max(dot(lPos, reflect(-normalize(ro + rd * t), normal)), 0.), 40.);
        if (Scene(pos, normalize(lPos - normalize(pos))).y == -1.)
            col *= .5;
    } else {
        col = texture(box, -(ro + rd * 20.)).xyz;
    }
    return col;
}

void main() {
    gl_FragColor = vec4(0.);
    vec2 v = uv * 2.;
    v -= 1.;
    v.x *= resolution.y / resolution.x;

    float x = -0.0;
    float y = 2.;
    float z = 7.0;

    for (int i = 0; i < count; i++) {
        bPos[i] = vec3(sin(time * .5 + .967 + 42. * (float(i))),
            1. + cos(time * .5 * .423 + 152. * (float(i))),
            3. + cos(.76321 * time * .5 + (float(i))));
    }

    vec3 ro = vec3(x, y, z);
    vec3 lookAt = ro * .5; //vec3(0., 0., 0.);
    vec3 eye = normalize(lookAt - ro);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));

    //normalize the direction vector before multiplying to the matrix
    vec3 rd = mat3(right, up, eye) * normalize(vec3(uv.xy, 2.));

    vec3 col = Render(ro, rd);

    gl_FragColor = vec4(col, 1.);
}