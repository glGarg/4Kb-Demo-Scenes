varying vec2 uv;
varying vec2 texCoord;
uniform samplerCube box;
uniform float time;
uniform vec2 resolution;

mat3 m;
float l;
const vec3 glowColor = vec3(1.0, 0.075, 0.01) * 5.0;
const vec3 innerColor = vec3(0.1, 0.5, 1.0) * 2.0;
const float clip = 290.;

float Map(vec3 pos) {
    float r, s = 1.2;
    float scale = 1.0;
    for (int i = 0; i < 20; i++) {
        pos *= s;
        pos = abs(pos);
        scale *= s;
        pos += vec3(-.8, -1.6, -.4);
        pos *= m;
        r = scale * length(pos);
    }
    l = (length(pos) - 1.5) / scale;
    return l;
}

vec3 CalcNormal(vec3 pos) {
    vec3 eps = vec3(.01, 0., 0.); //just .1
    vec3 nor = vec3(0., Map(pos), 0.);
    vec3 v2 = nor - vec3(eps.x, Map(pos + eps), 0.);
    vec3 v3 = nor - vec3(0., Map(pos - eps.yyx), -eps.x);
    return normalize(cross(v2, v3));
}

vec3 Trace(vec3 pos, vec3 ro, vec3 rd, float t, float closest) {
    vec3 normal = CalcNormal(pos), col = vec3(0.3);
    col += exp(-t * length(pos * rd)) * normal - 1.5 * sin(normal) * innerColor + 0.6 * cos(normal) * glowColor;
    if (t >= clip - 20.) {
        col += exp(-t / length(pos)) * glowColor;
    }
    vec3 lPos = -normalize(ro + vec3(1., -1., 0.)); //normalize(vec3(5., -15., 5.));
    col += dot(normalize(lPos - normalize(pos)), normal);
    float spec = pow(dot(-reflect(normalize(pos), normal), lPos), 50.);
    col += spec + max(sin(9. * time) * spec, 0.);
    float atten = 0.1;
    col += exp(-atten * t) * rd + (1. - exp(-atten * t)) * col;
    col = .7 * (3. * 1. / t * innerColor + .8 * (m * normal * .3 + col * .15)) + .3 * vec3(1., 0., 0.) * col * m * exp(length(pos) / t) * l;
    return 1.8 * col - .8; // + 0.3*tan(col);//vec3(1.2*col.x*col.z, .5*normal.y*col.z,.8*normal.y*col.y);
    //return .2*rd*pos*pow(normal*ro, -normalize(vec3(t))) + .8*(normal*.3 + closest + col*.15);
}

mat3 Quaternion(vec3 axis, float angle) {
    vec4 a = vec4(axis * sin(angle), cos(angle));
    return mat3(a.w * a.w + a.x * a.x - a.y * a.y - a.z * a.z, 2. * a.x * a.y + 2. * a.w * a.z, 2. * a.x * a.z - 2. * a.y * a.w,
        2. * a.x * a.y - 2. * a.w * a.z, a.w * a.w - a.x * a.x + a.y * a.y - a.z * a.z, 2. * a.y * a.z - 2. * a.w * a.x,
        2. * a.x * a.z + 2. * a.w * a.y, 2. * a.y * a.z + 2. * a.w * a.x, a.w * a.w - a.x * a.x - a.y * a.y + a.z * a.z);
}

vec3 Scene(vec3 ro, vec3 rd, out float t) {
    float closest = 1000.;
    for (int i = 0; i < 200; i++) {
        float d = Map(ro + rd * t);
        t += d * .33 + .0015 * t;
        closest = min(closest, d / t);
        if (d < 0.00004) {
            return Trace(ro + rd * t, ro, rd, t, closest);
        }
        if (t > clip) {
            t = clip;
            break;
        }
    }
    return textureCube(box, rd).xyz;
}
void main() {
    //mat3 m2 = Quaternion(normalize(vec3(0.1, 1.0, 0.01)), sin(.9*time));
    m = Quaternion(normalize((vec3(0.1 * sin(2. * time), 1.5 * cos(0.1 * time), 1.7 * tan(0.1 * time)))), .02 * mod(.5 * time, resolution.y));
    vec2 p = uv;
    p.x *= resolution.x / resolution.y;

    float x = 1. * (mod(abs(.2 * time), 10.)); //-1.*(sin(time));
    float y = -1.;
    float z = 1.2 * sqrt(100. - pow(x, 2.)); //-1.*(cos(time));
    vec3 ro = vec3(x, y, z);
    vec3 lookAt = vec3(0.);
    vec3 eye = normalize((lookAt - ro));
    vec3 right = normalize(cross(vec3(0., 1., 0.), eye));
    vec3 up = normalize(cross(right, eye));
    vec3 rd = normalize(1. * eye + p.x * right + p.y * up);
    float t;
    vec3 col = Scene(ro, rd, t);
    gl_FragColor = .3 + vec4(col, 1.);
}