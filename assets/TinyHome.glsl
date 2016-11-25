varying vec2 uv;
uniform float time;
uniform vec2 resolution;
uniform sampler2D texture;

float shine = 0.;
vec4 frequency;
vec2 g;
vec3 a;

float sdCylinder(vec3 p, vec2 h) {
    vec2 d = abs(vec2(length(p.zy), p.x)) - h;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

float Hash(vec2 co) {
    float a = 120.98;
    float b = 78.23;
    float c = 48.553;
    vec2 s = vec2(a, b);
    float dt = dot(co, s);
    float sn = mod(dt, 3.14);
    return fract(sin(sn) * c);
}

vec2 Hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx / vec3(3.07965, 7.1235, 4.998784)));
    p3 += dot(p3.zxy, p3.yxz + 19.19);
    return fract(vec2(p3.x * p3.y, p3.z * p3.x));
}

float Noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    //f = f*f*(3. - 2.*f);
    f = f * f * f * (6.0 * f * f - 15.0 * f + 10.0);
    return mix(mix(Hash(i), Hash(i + vec2(1., 0.)), f.x),
        mix(Hash(i + vec2(0., 1.)), Hash(i + vec2(1.)), f.x), f.y);
}

vec2 Noise3(vec2 p) {
    p = floor(p);
    vec2 f = fract(p);
    f = f * f * (3. - 2. * f);
    float n = p.x + p.y * 57.;
    return mix(mix(Hash22(p), Hash22(p + vec2(0., 1.)), f.x),
        mix(Hash22(p + vec2(1., 0.)), Hash22(p + vec2(1.)), f.x), f.y);
}

vec3 sunLight = normalize(vec3(0.5, 1.5, 0.1));
vec3 sunColour = vec3(1.0, .9, .83);
float specular = 0.0;
float ambient;

void DoLighting(inout vec3 mat, in vec3 pos, in vec3 normal, in vec3 eyeDir, in float dis) {
    float h = dot(sunLight, normal);
    float c = max(h, 0.0) + ambient;
    mat = mat * sunColour * c;
    // Specular...
    if (h > 0.0) {
        vec3 R = reflect(sunLight, normal);
        float specAmount = pow(max(dot(R, normalize(eyeDir)), 0.0), 100.0) * specular;
        mat = mix(mat, sunColour, specAmount);
    }
}

vec3 TerrainCol(vec3 ro, vec3 pos, vec3 normal, float dis) {
    vec3 mat;
    ambient = 0.1;
    specular;
    vec3 dir = pos - ro;
    vec3 matPos = pos * 3.;
    float d = dis * dis;
    float scale = 10.01;
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
    if (matPos.y > scale * 35. || normal.y >= .42) {
        float snow = Noise(matPos.xy * 0.0005) * clamp((matPos.y - 35.0 - Noise(matPos.xz * .1) * 28.0) * 0.035, 0.0, 1.0);
        mat = mix(mat, vec3(.7, .7, .8), snow);
        specular += snow;
        ambient += snow * .3;
    }
    if (matPos.y < scale * 2.) {
        if (normal.y > .4) {
            f = Noise(matPos.xz * 0.084) * 1.5;
            f = clamp((2.0 - f - matPos.y) * 1.35, 0., 0.67);
            float t = normal.y - .4;
            t = t * t;
            mat = mix(mat, vec3(.09 + t, .07 + t, .03 + t), f);
        }
    }
    if (matPos.y < scale * 0.) {
        mat *= .5;
    }

    DoLighting(mat, pos, normal, dir, dis * dis);
    if (matPos.y < scale * 1.) {
        //matPos.y = 0.;
        //(iGlobalTime*.03);
        vec3 watPos = matPos;
        watPos += -dir * (watPos.y / dir.y);

        float tx = cos(watPos.x * .052) * 4.5;
        float tz = sin(watPos.z * 0.072) * 4.5;
        vec2 co = Noise3(vec2(watPos.x * 4.7 + 1.3 * tz, watPos.z * 4.69 + time * 35. - tx));
        mat = mix(vec3(0.1, 0.1, 0.5), vec3(0.8, 0.8, 0.72) * 0.7, clamp(normal.y * 0.3, 0.2, 0.9));
        pos.y = 0.;
        pos.x += tx;
        pos.y += tz;
        //mat = mix(renderRefl((pos), reflect(dir, normal.xyz), fragCoord),mat,0.9);

    }
    float fog = exp(-.001 * dis * dis * sin(time));
    return mix(vec3(0.9, 0.9, 0.9), mat, fog);
}

vec4 Map(vec3 p) {
    vec4 f;
    g /= resolution * vec2(12., 1.2);
    g.y += sin((g.x -= (f.a = .3) * .015) * 46.5 + f.a) * .12;
    f = texture2D(texture, .05 * p.xz, 4. * (sin(f.a) * .5 + .5));
    f = smoothstep(f + .5, f, f / f * .71);
    shine += .01;
    return vec4(length(p) - 5. + 1. - .5 * dot(f, vec4(.3)), f.xyz);
}

vec3 GetNormal(vec3 pos) {
    vec3 eps = vec3(.01, 0., 0.);
    vec3 nor = (vec3(Map(pos + eps).x - Map(pos - eps).x,
        Map(pos + eps.yxy).x - Map(pos - eps.yxy).x,
        Map(pos + eps.yyx).x - Map(pos - eps.yyx).x));
    return normalize(nor);
}

vec4 Map(vec3 p, vec3 ro, float t) {
    vec4 col = vec4(texture2D(texture, -.3 * p.xz + .5).xyz, 1.);
    //col *= 1.5*frequency;
    vec3 normal = GetNormal(p);
    vec4 f;
    g /= vec2(400., 400.) * vec2(12., 1.2);
    g.y += sin((g.x -= (f.a = .3) * .015) * 46.5 + f.a) * .12;
    f = texture2D(texture, .05 * p.xz, 4. * (sin(f.a) * .5 + .5));
    f = smoothstep(f + .5, f, f / f * .71);
    //shine += .01;
    col.xyz = TerrainCol(ro, 2. * p, normal, t);
    return vec4(length(p) - 4. + 1. - dot(f, vec4(.3)), .8 * col.xyz);
}

vec3 Scene(vec3 ro, vec3 rd) {
    float t = 0., d = 10.;
    for (int i = 0; i < 900; i++) {
        if (t > 10. || d < 0.) {
            return .2 + 100. * vec3(pow(texture2D(texture, a.xz).xyz, vec3(100.)));
        }
        vec4 a = Map(ro + rd * t);
        d = a.x;
        if (d < 0.001) {
            a = 5. * Map(ro + rd * t, ro, t);
            shine += .09 * 20.;
            vec3 normal = GetNormal(ro + rd * t);
            //a.yzw += .005*textureCube(iChannel1, rd).xyz;
            //a.yzw += .04*textureCube(iChannel1, reflect((rd), normal)).xyz;
            //return (2.2*a.wyz-.2+a.ywz)*.5*shine;
            return (a.yzw) * shine;
        }
        t += d * .2 + .003 * t;
    }
    return vec3(0.);
}

mat3 Quaternion(vec3 axis, float angle) {
    vec4 a = vec4(axis * sin(angle), cos(angle));
    return mat3(a.w * a.w + a.x * a.x - a.y * a.y - a.z * a.z, 2. * a.x * a.y + 2. * a.w * a.z, 2. * a.x * a.z - 2. * a.y * a.w,
        2. * a.x * a.y - 2. * a.w * a.z, a.w * a.w - a.x * a.x + a.y * a.y - a.z * a.z, 2. * a.y * a.z - 2. * a.w * a.x,
        2. * a.x * a.z + 2. * a.w * a.y, 2. * a.y * a.z + 2. * a.w * a.x, a.w * a.w - a.x * a.x - a.y * a.y + a.z * a.z);
}

void main() {
    vec2 v = 2. * uv - 1.;
    //v.y -= 1.*iMouse.y/iResolution.y-.5;
    //if (abs(v.x) > .8) {
    //	gl_FragColor = vec4(0., 0., 0., 1.);
    //   return;
    //}
    g = gl_FragCoord.xy;
    mat3 m = Quaternion(normalize(vec3(1., 3.5, 0.)), time * .1);
    float x = 0.2;
    float y = 5.5;
    float z = .3;
    vec3 ro = vec3(x, y, z);
    ro.xyz *= m;
    vec3 lookAt = vec3(0., 0., 0.);
    vec3 eye = normalize(lookAt - ro);
    vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));
    vec3 up = normalize(cross(right, eye));
    vec3 rd = mat3(right / .58, up, eye) * normalize(vec3(v / 2., 0.3));

    a = mat3(normalize(cross(normalize(vec3(-.2, -5.5, -.3)), vec3(0., 1., 0.))) / .58,
        normalize(cross(normalize(cross(normalize(vec3(-.2, -5.5, -.3)), vec3(0., 1., 0.))), normalize(vec3(-.2, -5.5, -.3)))),
        normalize(vec3(-.2, -5.5, -.3))) * m * normalize(vec3(v / 2., 0.3));

    gl_FragColor = vec4(Scene(ro, rd), 1.0);
}