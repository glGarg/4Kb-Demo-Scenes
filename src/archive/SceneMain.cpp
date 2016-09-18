#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <math.h>
#include <gl/glew.h>
#include <glfw.h>
#include <GL/GL.H>
#include <GL/glu.h>
#include <GL/glut.h>
#include "haders.h"

#define ARRAY_COUNT( array ) (sizeof( array ) / (sizeof( array[0] ) * (sizeof( array ) != sizeof(void*) || sizeof( array[0] ) <= sizeof(void*))))
GLuint theProgram;
GLuint elapsedTimeUniform;
const float vertexData[] = {
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
};

const std::string strVertexShader(
	"in vec2 position;\n"
	"smooth out vec2 uv;\n"
	"void main()\n"
	"{\n"
	"	uv = position;\n"
	"	gl_Position = vec4(uv, 0., 1.);\n"
	"}\n"
);

const std::string strFragmentShader(
/*	"precision mediump float;\n"
	"in vec2 uv;\n"
	"uniform float time;\n"
	"float snoise(vec2 p){\n"
		"vec2 f =  fract(p);\n"
		"p = floor(p);\n"
		"float v = p.x+p.y*1000.0;\n"
		"vec4 r = vec4(v, v+1., v+1000., v+1001.);\n"
		"r = fract(100000.*sin(r*0.001));\n"
		"return 2.*(mix(mix(r.x, r.y, f.x),\n"
        "             mix(r.z, r.w, f.x), f.y)) - 1.;\n"
	"}\n"

"float terrain(vec2 cPos, int octaves){\n"
"	float h;\n"
 "   float w = 0.5;\n"
"    float m = 0.4; //dont know why its set to this val\n"
"    for(int i = 0; i < 11; i++){\n"
 "       if(i < octaves){\n"
 "       	h += w* snoise(cPos*m);\n"
 "       	w *= .5;\n"
 "       	m *= 2.;\n"
 "       }\n"
 "       else break;\n"
 "   }\n"
 "   return h;\n"
"}\n"

"vec2 map(vec3 p, int octaves){\n"
"    float dMin = p.x;\n"
"    float d;\n"
"    float mID = 1.;\n"
"    float h = terrain(p.xz, octaves);\n"
    
"    h += smoothstep(-0.3, 1.5, h);\n"
 "   h *= smoothstep(-1.5, -0.3, h);\n"
    
 "   d = p.y - h;\n"
 "   if(d < dMin){\n"
 "   	dMin = d;\n"
 "       mID = 0.;\n"\
 "  }\n"
  "  if(h < 0.){\n"
  "  	if(d < 26.){\n"
   "     	dMin = d;\n"
    "        mID = 1.;\n"
     "   }\n"
    "}\n"
    "return vec2(dMin, mID);\n"
"}\n"

"vec2 castRay(vec3 ro, vec3 rd, int octaves){\n"
"	const float d = 0.001;\n"
"    float t = 0.0;\n"
"	float h = d* 2.;\n"
"    float m = -1.; //material\n"
"    for(int i = 0 ; i < 150; i++){\n"
"        if(abs(h) > d || t < 26.){\n"
"        	t += h;\n"
 "           vec2 res = map(ro + rd*t, octaves);\n"
 "           h = res.x;\n"
 "           m = res.y;\n"
 "       }else break;\n"
"    }\n"
"    if(t > 26.) m = -1.;\n"
"    return vec2(t, m);\n"
"}\n"

"vec3 calcNormal(vec3 p, int octaves){\n"
"	vec3 eps = vec3(0.0001, 0., 0.);\n"
"    return normalize(vec3(map(p+eps, octaves).x - map(p-eps, octaves).x,\n"
"                          map(p+eps.yxy, octaves).x - map(p-eps.yxy, octaves).x,\n"
"                          map(p+eps.yyx, octaves).x - map(p-eps.yyx, octaves).x));\n"
"}\n"

"float shadows(vec3 ro, vec3 rd, float tMax, float k, int octaves){\n"
"	float res = 1.;\n"
 "   float t = 0.001;\n"
  "  for(int i = 0 ; i< 5; i++){\n"
  "      if(t < tMax){\n"
 "       	float h = map(ro + rd*t, octaves).x;\n"
 "       	res = min(res, k*h/t);\n"
 "       	t += h;\n"
 "       }else break;\n"
 "   }\n"
 "   return clamp(res, 0., 1.);\n"
"}\n"

"vec3 render(vec3 ro, vec3 rd){\n"
"	vec3 color = vec3(0.2, 0.2, 0.4);\n"
 "   const int LOD = 7;\n"
 "   vec2 res = castRay(ro, rd, LOD);\n"
 //"   vec3 lPos = normalize(mat3(cos(0.4*iGlobalTime), 0., sin(0.4*iGlobalTime), sin(0.4*iGlobalTime), 0.,  -cos(0.4*iGlobalTime), 0., 0., 0.)*vec3(.0, 1.1, 0.5));\n"
"vec3 lPos = vec3(.0, 1.1, 0.5);\n" 
 "  vec3 lCol = vec3(1., 1.0, 1.0);\n"
  "  int norLOD = int(max(2., 12. - 11.*res.x/26.));\n"
   " vec3 normal = calcNormal(ro + rd*res.x, norLOD);\n"
   " if(res.y < -.5){\n"
   " 	color = vec3(0.2, 0.3, 0.7);\n"
   " }\n"
    
  "  else if(res.y>-0.5 && res.y < 0.5){\n"
  " 	color = mix(vec3(0.2, 0.2, 0.2), vec3(0.23, 0.2, 0.13),normal.y);\n"
  "  	float n = 0.5*snoise((ro + rd*res.x).xy*vec2(2., 40.)) + 1.;\n"
    "	color = mix(n*vec3(0.5, 0.4, 0.4), color, normal.y);\n"
    "	color = mix(color, vec3(0., 0.05, -0.05), smoothstep(0.7, 0.9, normal.y));\n"
    "	float amb = clamp(0.2+0.2*normal.y, 0.0, 1.); //lower is darker\n"
   "     float dif = clamp(dot(lPos, normal), 0., 2.);\n"
     " 	if (dif > 0.05) dif *= shadows((ro+rd*res.x), lPos, 1.0, 1.0, LOD);\n"
		
    "    color += 0.2*amb * lCol;\n"
    "    color *= 1.7*dif * lCol;\n"
    "}\n"
    "else{\n"
    "	color = mix(vec3(0.2, 0.2, 0.4), vec3(0.2, 0.2, 0.7), snoise((ro + rd*res.x).xy));\n"
   " }\n"
   " float n = smoothstep(-1.2, -0.2, terrain((ro+rd*res.x).xz, 3));\n"
  "  float fog = exp(-0.01*res.x*res.x);\n"
 "   color = mix(vec3(0.9, 0.9, 0.9),color, n*fog);\n"
"	return color; \n"
"}\n"

"void main()\n"
"{\n"
	"vec2 p = 2.*uv.xy-1.;\n"
    //"pos -= 1.;\n"
    //"pos.x *= 400/500;\n"
    //"p.x*=4./5.;\n"
    //camera
    "float x = 5.+(0.2)*time;\n"
    "float y = 0.;\n"
    "float z = 0.3+(0.1*time);\n"
    
    "vec3 cPos = vec3(x, y, z); //our position\n"
    "cPos.y = terrain(cPos.xz, 1) + 1.5;\n"
    "const vec3 cUp = vec3(0., 1., 0.);\n"
    "vec3 cLook = .5*cPos;//lookAt\n"
    
    //camera matrix
    "vec3 ww = normalize(cLook - cPos);\n"
    "vec3 uu = normalize(cross(ww, cUp));\n"
    "vec3 vv = normalize(cross(uu, ww));\n"
    //want ray to travel according to the matrix
    "vec3 rd = normalize(p.x*uu + p.y*vv + 2.*ww);\n"
    "vec3 color = render(cPos, rd);\n"
    "gl_FragColor = vec4(color, 1.);\n"
"}\n"*/
"uniform float time;\n"
"varying vec2 uv;\n"
"uniform vec2 mouse;\n"
"mat3 m = mat3( 0.00,  0.80,  0.60,\n"
"              -0.80,  0.36, -0.48,\n"
"              -0.60, -0.48,  0.64 );\n"

"float Hash( float n ){\n"
"    return fract(sin(n)*4758.15);\n"
"}\n"

"float Noise(vec3 x){\n"
"	vec3 p = floor(x);\n"
"    vec3 f = fract(x);\n"
"    f = f*f*(3.-2.*f);\n"
"    float n = p.x + 157.*p.y + 113.*p.z;\n"    
"    return mix(mix(mix(Hash(n+0.), Hash(n+1.), f.x),\n"
"               mix(Hash(n+157.), Hash(n+158.), f.x), f.y),\n"
"           mix(mix(Hash(n+113.), Hash(n+114.), f.x),\n"
"               mix(Hash(n+270.), Hash(n+271.), f.x), f.y), f.z);\n"
"}\n"

"float sdSphere(vec3 p, float s){\n"
"    return length(p) - (2.8+2.5*(sin(1.*time)));\n"
"}\n"

"float Fbm( vec3 p ){\n"
"    float f = 0.5000*Noise( p ); p = m*p*2.02;\n"
"    f += 0.2500*Noise( p ); p = m*p*2.03;\n"
"    f += 0.1250*Noise( p ); p = m*p*2.01;\n"
"    f += 0.0625*Noise( p );\n"
"    p = m*p*2.02;\n"
"    f += 0.03125*abs(Noise( p ));\n"	
"    return f/.9375;\n"
"}\n"

"float Map(vec3 pos, out float fbm){\n"
"	float sph = sdSphere(pos, .8);\n"
"    fbm = 1.25*Fbm(0.6*pos+vec3(0.,1., 0.)*time);\n"//was .45
"    sph -= 2.*fbm;\n"
"    return sph;\n"
"}\n"

"vec3 GetNormal(vec3 pos){\n"
 "   float t;\n"
"	vec3 eps = vec3(.001, 0., 0.);\n"
"    vec3 nor = vec3(0., Map(pos, t), 0.);\n"
 "   vec3 v2 = nor - vec3(eps.x, Map(pos, t),0.);\n"
 "   vec3 v3 = nor - vec3(0., Map(pos, t), -eps.x);\n"
 "   return normalize(cross(v2, v3));\n"
"}\n"

"bool Scene(vec3 ro, vec3 rd, out float fbm, out float t){\n"
    "float m, s = 1.;\n"
    "for(int i = 0; i < 200; i++){\n"
      "  vec3 pos = ro+rd*t;\n"
      "  float h = Map(pos, fbm);\n"
      "  if(h < 0.005){\n"
      "   	return true;\n"
      "  }\n"
      "  if(t > 12.){\n"
     "   	return false;\n"
    "    }\n"
   "     t += h*0.5;\n"
  "  }\n"
 "   return false;\n"
"}\n"

"void main(void)\n"
"{\n"
	"vec2 p = uv* 2.;\n"
    "p -= 1.;\n"
    "p.x *= 1./1.;\n"
    
    "float x = 0.;\n"
    "float y = 0.;\n"
   " float z = 5.;\n"
    
    "vec3 ro = vec3(x, y, z);\n"
   " vec3 lookAt = vec3(0., 0., 0.);\n"
    "vec3 eye = normalize(lookAt - ro);\n"
   " vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));\n"
   " vec3 up = normalize(cross(right, eye));\n"
   " vec3 rd = normalize(vec3(.8*eye + p.x*right + p.y*up));\n" 
   " float t, fbm;\n"
   " vec3 col = vec3(0.);\n"
   " if(Scene(ro, rd, fbm, t)){\n"
   "     vec3 lCol = vec3(0.2, .2, 0.);//0.5, 0.5, 0.\n"
  "      vec3 lPos = normalize(vec3(1.));\n"
 "   	vec3 white = vec3(4., 4., 4.);\n"
"		vec3 yellow = vec3(1., 1., 0);\n"
"		vec3 red = vec3(1., 0, 0);\n"
"		vec3 grey = vec3(0.4, 0.4, 0.4);\n"
"		float t = fract(fbm*3.0);\n"
"		if (fbm < 0.3333) {\n"
"			col =  mix(white, yellow, t);\n"
"		} else if (fbm < 0.6666) {\n"
"			col = mix(yellow, red, t);\n"
"		} else {\n"
"			col = mix(red, grey, t);\n"
"		}\n"
 "    }\n"
    
"    gl_FragColor = vec4(0.8*col+0.1, 1.);\n"
"}\n"
/*"vec3 CSize = 1.*vec3(0.633, 0.832, .713);\n"
"float Map(vec3 p){\n"
 "   float scale = 1.;\n"
  "  float t;\n"
  "vec3 pos = p;\n"
  "   for(int i = 0; i < 7; i++){\n"
   " 	 p = 2.*clamp(p, -CSize, CSize) - p;\n"
   "     float k = max(0.812/dot(p, p), 1.);\n"
   "     p *= k;\n"
   "     scale *= k;\n"
   "  }\n"
   "	float rxy = (-p.z*length(p.xy)/(length(p)+.07-.02*cos(p.x*pos.x + p.y*pos.y + p.z*pos.z)));\n"
   "	return max(rxy-(dot(CSize, vec3(pos))), abs(rxy*pos.z)/length(pos))/scale;\n"
"}\n"

"bool Scene(vec3 ro, vec3 rd, out float t){\n"
"	bool hit = false;\n"
"	float h;\n"
 "   for(int i = 0; i < 200; i++){\n"
  "      if(t < 12.){\n"
   "     	h = Map(ro + rd*t);\n"
    "    	if(h < .0003){\n"
     "   		hit = true;\n"
      "  	    break;\n"
       " 	}\n"
        "	t += h*.7;\n"
        "}\n"
   " }\n"
"	return hit;\n"
"}\n"
"vec3 Color(vec3 p){\n"
"	float col = 0.;\n"
 "   float k, scale = 1.;\n"
  "  for(int i = 0; i < 7; i++){\n"
   " 	vec3 p1 = 2.*clamp(p, -CSize, CSize)-p;\n"
    "    col += abs(p.x - p1.x);\n"
     "   p = p1;\n"
      "  k = max((.812/dot(p, p)), 1.);\n"
      "  p *= k;\n"
     "   scale *= k;\n"
    "}\n"
	"   return sin(col * vec3(.6, -.9, 4.9))*.75+.4;\n"
"}\n"


"vec3 CalcNormal(vec3 pos, float distance){\n"
 "   float t = min(.3, .001+.0001*distance*distance);\n"
"	vec3 eps = vec3(.00001*t, 0., 0.);//just .1\n"
 "   vec3 nor = vec3(0., Map(pos), 0.);\n"
  "  vec3 v2 = nor - vec3(eps.x, Map(pos+eps),0.);\n"
   " vec3 v3 = nor - vec3(0., Map(pos-eps.yyx), -eps.x);\n"
   " return normalize(cross(v2, v3));\n"
"}\n"

"float Shadow(vec3 pos, vec3 ld, float d){\n"
 "   float h = 0., t = 0.;\n"
  "  float res = 1.;\n"
   " for(int i = 0; i < 14; i++){\n"
    "    if(t < d){\n"
     "   	h = Map(pos+ld*t);\n"
      "      res = min(4.*h/t, res);\n"
       " 	t += .4*h;\n"
        "}\n"
    "}\n"
    "return clamp(res, 0., 1.);\n"
"}\n"

"vec3 lightPos;\n"
"vec3 Render(vec3 pos, vec3 ro, vec3 rd, vec3 nor, float t){\n"
"	vec3 col = vec3(0.);\n"
 "   vec3 lightPos = pos+vec3(cos(time*2.0+2.8)*.3, 1., sin(time*2.0+2.4));// + vec3(cos(iGlobalTime*2.0+2.8)*.3, cos(iGlobalTime*2.0+1.5)*.1, 0.);//ro + sin(iGlobalTime*0.05)*vec3(pos.x, 20., pos.z);\n"
  "  vec3 lPos = lightPos - pos;\n"
   " float bri = 1.;//max(dot(normalize(lPos), nor), 0.);\n"
   " float diff = dot(normalize(normalize(lPos)-normalize(pos)), nor);\n"
   " float spec = max(dot(reflect(normalize(pos), nor), normalize(lPos)), 0.);\n"
   " float amb = max(abs(nor.x)*.02, 0.025);\n"
   " float shad = Shadow(pos, normalize(lPos), length(lPos));\n"
   " col = Color(pos);\n"
   " return col*shad + 0.5*col*diff +.5*pow(spec, 15.)*shad + 0.3*amb*col;\n"
"}\n"

"float Glow(vec3 lPos, vec3 rd, vec3 pos){\n"
"	float l;\n"
 "   if(lPos.y < pos.y){\n"
  "  	l = pow(dot(normalize(lPos), rd), 1000.);\n"
   " 	l *= 3.;\n"
    "}\n"
    "return l;\n"
"}\n"

"void main()\n"
"{\n"
 "   vec3 normal;\n"
  "  vec2 p = 2.*uv - 1.;\n"
   " p.x *= 1.;\n"
   " float x = -.5;\n"
   " float y = -1.+10.*sin(time*.05)-5.*cos( -sin(time*.1));//-19.*sin(0.01*time);//iMouse.x/iResolution.x;\n"
   " float z = 7.;\n"
    
   " vec3 ro = vec3(x, y, z);\n"
    "vec3 lookAt = vec3(1., 0., z);//vec3(0.);\n"
   " vec3 eye = normalize(lookAt - ro);\n"
    "vec3 right = -normalize(cross(eye, vec3(0., 1., 0.)));\n"
   " vec3 up = normalize(cross(right, eye));\n"
    "vec3 rd = normalize(2.5*eye + p.y*right + p.x*up);\n"
    "float t;\n"
    "Scene(ro, rd, t);\n"
    "normal = CalcNormal(ro + rd*t, t);\n"
    "vec3 pos = ro +rd*t;\n"
    "gl_FragColor = vec4(t < 12. ? Glow(vec3(cos(time*2.0+2.8)*.3, 1., sin(time*2.0+2.4)*.2), rd, pos) + Render(pos, ro, rd, normal, t) : vec3(0.), 1.0);\n"
"}\n"*/
/*"float Hash(float n){\n"
"	return fract(cos(n)*2384.424);\n"
"}\n"

"float Hash(vec2 p){\n"
"	vec2 x = fract(p / vec2(3.29, 5.23));\n"
 "   return fract(3235.3432*sin(x.x*x.y));\n"
"}\n"

"float Noise(vec2 x){\n"
"	vec2 p = floor(x);\n"
    "vec2 f = fract(x);    \n"
   " f = f*f*(3.-2.*f);\n"
  "  return mix(mix(Hash(p),Hash(p + vec2(1., 0.)),f.x), mix(Hash(p + vec2(0., 1.)),Hash(p + vec2(1.)),f.x),f.y);\n"
"}\n"

"float Water(vec2 p){\n"
	"float height = 0.;\n"
    "float amp = 8.;\n"
    "vec2 shift1 = 0.002*vec2(time*320., time*240.);\n"
    "vec2 shift2 = 0.002*vec2(time*380., -1.*time*260.);\n"
    
    "float wave = sin(.021*p.x+shift1.x);\n"
    "wave += sin(p.x*.0172+p.y*0.01+shift2.x*1.121)*3.;\n"
    "wave += sin(p.x*.00172 + p.y*.005 + shift2.x*.121)*3.;\n"
    "wave += sin(p.x*.02221+p.y*.01233+shift2.x*3.437)*4.;\n"
    "wave += sin(p.x*.03221 + p.y*.01311 + shift2.x*4.269)*2.0;\n"
    "wave *= 1.4; //no more than 1.4\n"
    "for(int i = 0; i < 14; i++){ //9 works well if high enough\n"
    "	wave += abs(sin(Noise(p*.01+shift1)-.5)*3.14)*amp;\n"
     "   amp *= -.5;\n"
    "    shift1 *= 1.841;\n"
   "     p *= 2.*mat2(.866, .5, -.5, .866);\n"
  "  }\n"
 "   return height+wave;\n"
"}\n"

"float WaterH(vec2 p){\n"
	"float height = 0.;\n"
    "float amp = 8.;\n"
    "vec2 shift1 = 0.002*vec2(time*320., time*240.);\n"
    "vec2 shift2 = 0.002*vec2(time*380., -1.*time*260.);\n"
    
    "float wave = sin(.021*p.x+shift1.x);\n"
    "wave += sin(p.x*.0172+p.y*0.01+shift2.x*1.121)*3.;\n"
    "wave += sin(p.x*.00172 + p.y*.005 + shift2.x*.121)*3.;\n"
    "wave += sin(p.x*.02221+p.y*.01233+shift2.x*3.437)*4.;\n"
    "wave += sin(p.x*.03221 + p.y*.01311 + shift2.x*4.269)*2.;\n"
    "wave *= 1.4; //no more than 1.4\n"
    "for(int i = 0; i < 9; i++){\n" //25 works well enough if high
    	"wave += abs(sin(Noise(p*.01+shift1)-.5)*3.14)*amp;\n"
        "amp *= -.5;\n"
        "shift1 *= 1.841;\n"
        "p *= 2.*mat2(.866, .5, -.5, .866);\n"
    "}\n"
 "   return height+wave;\n"
"}\n"

"float Map(vec3 pos){\n"
"	return pos.y - Water(pos.xz);\n"
"}\n"

"bool Scene(vec3 ro, vec3 rd, vec3 lPos, out float t){\n"
"	bool hit = false;\n"
"	float h;\n"
    "for(int i = 0; i < 150; i++){\n"
    	"h = Map(ro + rd*t);\n"
        "if(t > 1700.){\n"
        "	hit = false;\n"
        "    break;\n"
        "}\n"
        "if(h < .1){\n"
        "	hit = true;\n"
        "}\n"
  "      t += h*.6 + t*.003;\n"
 "   }\n"
"	return hit;\n"
"}\n"
"vec3 GetSky(vec3 ro, vec3 rd, float t, vec3 normal, vec2 coord){\n"
	"vec3 col = vec3(0.);\n"
	"float y = normalize(ro+rd*t).y;\n"
	"col.xyz -= clamp(dot(normalize(vec3(ro+rd*t)), normalize(vec3(0., 1000., -2000.))), 0., 1.);\n"
	"col = mix(vec3(0.1, 0., 0.8), col,clamp(y, 0., .8));\n"
	"col += mix(vec3(0.1, 0., 0.2), vec3(0.1, 0., 0.4), y);\n"
	"return 0.4*col + vec3(0.1, 0.1, 0.);\n"
"}"
"vec3 CalcNormal(vec3 pos, float distance){\n"
    "float t = min(.1, .005+.0001*distance*distance);\n"
	"vec3 eps = vec3(.0001*distance*distance, 0., 0.);\n"
    "vec3 nor = vec3(0., WaterH(pos.xz), 0.);\n"
    "vec3 v2 = nor - vec3(eps.x,WaterH(pos.xz+eps.xy),0.);\n"
  "  vec3 v3 = nor - vec3(0., WaterH(pos.xz - eps.yx), -eps.x);\n"
 "   return normalize(cross(v2, v3));\n"
"}\n"
"void main(void)\n"
"{\n"
	"vec2 m = mouse/2. - 250.;\n"
 "   vec3 normal;\n"
	"vec2 p = 2.*uv - 1.;\n"
	"float x = 4.*time;\n"
    "float y = 50.;\n"
   " float z = 4.*time;\n"
    
   " vec3 ro = vec3(x, y, z);\n"
   " ro.y = 15.;\n"
    "vec3 lookAt = .5*ro;\n"
   " vec3 eye = normalize(lookAt - ro);\n"
    "vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));\n"
    "vec3 up = normalize(cross(right, eye));\n"
    
    "vec3 rd = normalize(vec3(.8*eye + p.x*right + p.y*up));\n"
    "vec3 col, lPos = vec3(0., 3., 1.0);\n" //vec3(0.5, 3., 1.0)
    "float t;\n"
	"if(!Scene(ro, rd, lPos, t)||t > 300.){\n"
    	//create sky
     "   col = vec3(0.9, 0.9, 0.9);\n"
      "  col = mix(vec3(0.4, 0.4, 0.4), col, clamp(exp(-t*t/100.*0.002), 0., 0.8));\n"
	  "col = GetSky(ro, rd, t, normal, p);\n"
        "gl_FragColor = vec4(col, 1.0);\n"
        //"return;\n"
    "}\n"
    "else{\n"
     "   vec3 lCol = .5*vec3(.55, .55, .53);\n"
        "col = .7*vec3(0.05, 0.48, 0.54);\n"
    	"normal = CalcNormal(ro + rd*t, t);\n"
        "float effect = 0.;\n"
		"float fresnel = 1. - max(dot(normalize(-normalize(ro)+normalize(lPos)),normalize(lPos)), 0.);\n"
        "fresnel = pow(fresnel, 20.);\n"
        //fresnel += (1.-fresnel)*.65;
        "effect += 1.-fresnel;\n" // was 1. - fresnel
        "effect += .7*dot(normalize(normalize(ro+rd*t)+normalize(lPos)), normal);\n"
		//"effect += 3.*pow(max(dot(reflect(normalize(ro+rd*t),normal), normalize(lPos)), 0.), 10.);\n"
		//"effect += pow(max(dot(normalize(normalize(ro+rd*t)+normalize(lPos)), normal), 0.), 10.);\n"
		"col += effect*lCol+pow(max(dot(reflect(normalize(ro+rd*t),normal), normalize(lPos)), 0.), 10.);\n"
        "float atten =0.022;\n"
        "col = exp(-atten*t)*vec3(0.75,0.8,0.6) + (1. - exp(-atten*t))*col;\n" //col = exp(-atten*t)*vec3(0.75,0.8,0.6) + (1. - exp(-atten*t))*col;\n
        
		//float atten = 0.022;   //for sea water
        //col = exp(-atten*t)*vec3(0.8,0.9,0.6) + (1. - exp(-atten*t))*vec3(0.1, 0.19, 0.22);
        //col *= .8;
		
        //"col = mix(col,vec3(0.), fresnel);\n"
    " }\n"
    //col.yz -= .2;
    "col = mix(vec3(0.2, 0.2, 0.2), col, clamp(exp(-t*t/50.*0.002), 0., 0.8));\n"
   " gl_FragColor = vec4(col, 1.0);\n"
"}\n"*/


/*
"void main(void)\n"
"{\n"
    "vec3 normal;\n"
	"vec2 p = uv;\n"
	"p.x*=2.2;\n"
	"float x = 10.+time;\n"
    "float y = 0.;\n"
    "float z = 10.+time;\n"
    
    "vec3 ro = vec3(x, y, z);\n"
    "ro.y += 20.;\n"
    "vec3 lookAt = .5*ro;\n"
    "vec3 eye = normalize(lookAt - ro);\n"
    "vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));\n"
    "vec3 up = normalize(cross(right, eye));\n"    
    "vec3 rd = normalize(vec3(.8*eye + p.x*right + p.y*up));\n"
    "vec3 col, lPos = vec3(0.5, 3., 1.0);\n"
    "float t;\n"
    "if(!Scene(ro, rd, lPos, t)){\n"
    	//create sky
      "  col = vec3(0.9, 0.9, 0.9);\n"
     "   col = mix(vec3(0.4, 0.4, 0.4), col, clamp(exp(-t*t/100.*0.002), 0., 0.8));\n"
        //fragColor = vec4(col, 1.0);
        //return;
    "}\n"
    "else{\n"
        "vec3 lCol = .5*vec3(.55, .55, .73);\n"
        "col = .8*vec3(0.05, 0.48, 0.54);\n"
    	"normal = CalcNormal(ro + rd*t, t);\n"
        "float effect = 0.;\n"
        "float fresnel = 1. - max(dot(normalize(-ro+normalize(lPos)),normalize(lPos)), 0.);\n"
        "fresnel = pow(fresnel, 3.);\n"
        //fresnel += (1.-fresnel)*.65;
        "effect += 0.2 + 1.- fresnel;\n"
        "effect += .5*dot(normal, normalize(lPos));\n"
        "effect += pow(max(dot(reflect(eye,normal), normalize(lPos)), 0.), 40.);\n"
        "col = effect*lCol;\n"
        "float atten =0.022;\n"
        "col += exp(-atten*t)*vec3(0.8,0.9,0.6) + (1. - exp(-atten*t))*vec3(0.1,0.19,0.22);\n"
        //col = mix(col,vec3(0.), fresnel);
    "}\n"
    //col.yz -= .2;
 "   col = mix(vec3(0.2, 0.2, 0.2), col, clamp(exp(-t*t/50.*0.002), 0., 0.8));\n"
"    gl_FragColor = vec4(col, 1.0);\n"
"}"

/*"uniform vec2 mouse;\n"
"uniform float time;\n"
"varying vec2 uv;\n"
"float treeLine;\n"
"float treeCol;\n"

"float Hash(vec2 p){\n"
"	p = fract(p/vec2(3.07965, 7.4235));\n"
"    p += dot(p, p.yx+19.19);\n"
"    return fract(p.x*p.y);\n"
"}\n"

"float Noise(vec2 p){\n"
"	vec2 i = floor(p);\n"
"    vec2 f = fract(p);\n"
"    f = f*f*(3. - 2.*f);\n"
 "   return mix(mix(Hash(i), Hash(i + vec2(1., 0.)), f.x),mix(Hash(i + vec2(0., 1.)), Hash(i + vec2(1.)), f.x),f.y);\n"
"}\n"

"float Terrain(vec2 p){\n"
"	vec2 pos = 0.05*p;\n"
 "   float w = (Noise(pos*.25)*.75 + .15), h;\n"
 "   w= 66.*w*w;\n"
  "  for(int i = 0; i < 9; i++){\n"
  "  	h += Noise(pos)*(w);\n"
 "       w *= -.4;\n"
  "      pos *= 1.*mat2(1.2323, 1.999231, -1.999231, 1.22);\n"
   " }\n"
    
 "   return h + pow(abs(Noise(p*0.002)) , 5.)*255. - 5.;\n" //add variation to the value
"}\n"

"float Trees(vec2 pos){\n"
"	return Noise(pos*13.) *treeLine;\n"
"}\n"

"float Terrain2(vec2 p){\n"
"	vec2 pos = 0.05*p;\n"
 "   float w = (Noise(pos*.25)*.75 + .15), h;\n"
   " w= 66.*w*w;\n"
  "  for(int i = 0; i < 35; i++){\n"
    "	h += w*(Noise(pos));\n"
     "   w *= -.4;\n"
     "   pos *= mat2(1.2323, 1.999231, -1.999231, 1.22);\n"
    "}\n"
    
    
   " for(int i = 0; i < 15; i++){\n"
    "	h += (w*Noise(pos));\n"
     "   w *= -.4;\n"
     "   pos *= mat2(1.2323, 1.999231, -1.999231, 1.22);\n"
    "}\n"
"    return h;\n"
"}\n"

"float SphereTrace(float t){\n"
"	t = abs(t - 10.);\n"
 "   t = 0.01*t;\n"
  "  return clamp(t*t, 0., 80.);\n"
"}\n"
"vec3 GetSky(vec3 rd, vec3 light, vec3 color){\n"
 "   float sun = clamp(dot(rd, light), 0., 1.);\n"
  "  float v = pow(1. - max(rd.y, 0.), 2.);\n"
   " vec3 sky = mix(vec3(0.4, 0.4, 0.8), vec3(0.8, 0.2, 0.8), v);\n"
   " sky += color * v * v * .15;\n"
   " sky += vec3(1.) * pow(sun, 200.);\n"
    "return clamp(sky,0., 1.);\n"
"}\n"

"float map(vec3 pos){\n"
 "   float h = Terrain(pos.xz);\n"    
"	return pos.y - h;\n"
"}\n"

"vec3 GetNormal(vec3 pos, float distance){\n" //replace with terrain2 to give more previse height ratios
"	vec3 eps = vec3(.1, 0., 0.);\n"
  "  vec3 nor = vec3(0., Terrain2(pos.xz), 0.);\n"
 "   vec3 v2 = nor - vec3(eps.x, Terrain2((pos + eps).xz), 0.);\n"
   " vec3 v3 = nor - vec3(0., Terrain2((pos - eps.yyx).xz), -eps.x);\n"
"    return normalize(cross(v2, v3));\n"
"}\n"

"bool Scene(vec3 ro, vec3 rd, out float dis, vec2 fragCoord){\n"
"	bool hit = false;\n"
 "   float h, sphereR, s;\n"
  "  for(int i = 0; i < 300; i++){\n"
   "     if(dis > 4900.){\n"
    "    	hit = false;\n"
     "       break;\n"
      "  }\n"
        "h = map((ro+rd*dis));\n"
    "	if(h < 0.2){\n"
		"hit = true;\n"
		"break;}\n"
      "  dis += h*0.5+.03*dis;\n"
   " }\n"
    
 "   return hit;\n"
"}\n"
//plan to do later
"vec3 Render(vec3 ro, vec3 rd, float t, vec3 normal){\n"
"	vec3 col;\n"
"    vec3 p = 2.*(ro+rd*t);\n"
 "   float f = clamp(Noise(p.xz*0.05), 0., 1.);\n"
  "  vec3 m = mix(vec3(0.35+f*0.41 ,0.1+f*0.22 ,0.35+f*0.45 ), vec3(0.2+f*0.41,.1+f*.42 ,.35+f*.4), f*0.62);\n"
  "  col = m*vec3(0.7*m.x + 0.5, 0.6*m.y + 0.4, 0.6*m.z+0.55);\n"
   " return col;\n"
"}\n"

"void main(){\n"
 "   vec3 normal;\n"
"	vec2 p = 2.*uv;\n"
 "   p -= 1.;\n"
   "  float x = 10. +4.*(time);\n"
  "  float y = 10.;\n"
   " float z = 8. + 4.*time;\n"
    
   " vec3 ro = vec3(x, y, z);\n"
    "ro.y += Terrain(ro.xz);\n"
    
    "vec3 lookAt = 0.5*ro;\n"//vec3(ro.x, ro.y*0.5, 0.);\n"
    
    "vec3 eye = normalize(ro - lookAt);\n"
    "vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));\n"
    "vec3 up = normalize(cross(right, eye));\n"
	
  "  vec3 rd = normalize(eye*0.8 + right*p.x*0.8 + up*p.y);\n"
    
   " float distance;\n"
   " vec3 col;\n"
    "vec3 lCol = vec3(0.4);\n"
    "vec3 lPos = normalize(vec3(0.1, 1.2, 0.2));\n"
   " if( !Scene(ro, rd, distance, uv )|| distance > 350.){\n"
    "	col = GetSky(rd, lPos, lCol);\n"
     "   gl_FragColor = vec4(col, 1.);\n"
    "    return;\n"
    "}\n"
    "else{\n"
    "	vec3 pos = ro + rd*distance;\n"
    "    normal = GetNormal(pos, distance);\n"
     "   col = Render(ro, rd, distance, normal);\n"
     "  float effect = 0.2;\n"
      "  effect += max(dot(normal, lPos), 0.);\n"
     "   col += effect*lCol;\n"
    "}\n"
	"gl_FragColor = vec4(col, 1.);\n"    
"}\n"*/



/*"float treeLine;\n" 
"float treeCol;\n" 

"float Hash(vec2 p){\n" 
"	p = fract(p/vec2(3.07965,  7.4235));\n" 
 "   p += dot(p,  p.yx+19.19);\n" 
 "   return fract(p.x* p.y);\n" 
"}\n" 

"vec2 Hash22(vec2 p){\n" 
"	vec3 p3 = fract(vec3(p.xyx/vec3(3.07965,  7.1235,  4.998784)));\n" 
 "   p3 += dot(p3.zxy,  p3.yxz+19.19);\n" 
  "  return fract(vec2(p3.x*p3.y,  p3.z*p3.x));\n" 
"}\n" 

"vec2 Noise3(vec2 p){\n"
"	p = floor(p);\n"
 "   vec2 f = fract(p);\n"
  "  f = f*f*(3. - 2.*f);\n"
   " float n = p.x + p.y*57.;\n"
  "  return mix(mix(Hash22(p), Hash22(p + vec2(0., 1.)),f.x),\n"
  "             mix(Hash22(p + vec2(1., 0.)), Hash22(p + vec2(1.)),f.x), f.y);\n"
"}\n"

"float Noise(vec2 p){\n"
"	vec2 i = floor(p);\n"
"	vec2 f = fract(p);\n"
 "   //f = f*f*(3. - 2.*f);\n"
 "   f = f*f*f*(6.0*f*f - 15.0*f + 10.0);\n"
    "return mix(mix(Hash(i), Hash(i+ vec2(1., 0.)), f.x),\n"
                    "mix(Hash(i + vec2(0., 1.)), Hash(i + vec2(1.)), f.x), f.y);\n"
"}\n"
"float Noise2(vec2 p){\n"
"	vec2 i = floor(p);\n"
"	vec2 f = fract(p);\n"
 "   f = f*f*f*(6.0*f*f - 15.0*f + 10.0);\n"
  "  //f = f*f*(3. - 2.*f);\n"
   " return fract(mix(mix(Hash(i), Hash(i+ vec2(2., 0.)), f.x),\n"
    "                mix(Hash(i + vec2(0., 2.)), Hash(i + vec2(2.)), f.x), f.y));\n"
"}\n"
"float Terrain(vec2 p,int count){\n"
"	vec2 pos = p*0.02;\n"
 "   float w = (Noise(pos*.25)*0.75+.15);\n"
"	w = 86.0 * w * w;\n"
 "   float h = 0.;\n"
  "  for(int i = 0; i < 5; i++){\n"
   " 	h += pow((w*(Noise(pos))), Noise(pos)); //+Noise2(pos)/w\n"
    "	w *= -0.5;\n"
    "	pos *= mat2(1.2323, 1.999231, -1.999231, 1.22);\n"
    "}\n"
"	return h + pow(abs(Noise(p*0.02)) , 5.)*55. - 5.; //for variation\n"
"}\n"

"float Trees(vec2 p){\n"
"	return treeLine*Noise(p*13.);\n"
"}\n"

"float Terrain2(vec2 p,int count){\n"
"	vec2 pos = p*0.05;\n"
 "  float w = (Noise(pos*.25)*0.75+.15);\n"
"	w = 66.0 * w * w;\n"
 "   float h = 0.;\n"
  "  for(int i = 0; i < 5; i++){\n"
   " 	h += pow((w*(Noise(pos))), Noise(.4*pos))+w*0.2*Noise(1.2*pos);\n"
    "	w *= -0.4;\n"
    "	pos *= 2.*mat2(0.866, 0.5, -0.5, 0.866);\n"
    "}\n"
    "float ff = Noise(pos*.002);\n"
"	h += pow(abs(ff), 5.0)*55.-5.0;\n"
	
 "   treeCol = Trees(p);\n"
  "  h += treeCol;\n"
   " for(int i = 0; i < 7; i++){\n"
    "	h += w*Noise(pos);\n"
    "	w *= -0.5;\n"
    "	pos *= 2.*mat2(0.866, 0.5, -0.5, 0.866);\n"
    "}\n"
"	return h;\n"
"}\n"


"float Map(vec3 p){\n"
 "   float h = Terrain(p.xz, 5);\n"
    
  "  float ff = Noise(p.xz*3.0) + Noise(p.xz*3.3)*.5;\n"
   " treeLine = smoothstep(ff - 2., ff*2. + 4., h)*smoothstep(1.0+ff*3., .4*ff + 1.,h);\n"
    "treeCol = Trees(p.xz*10.);\n" 
    "h+= 10.*treeCol;\n"
    
"	return p.y - h;\n"
"}\n"

"float SphereRadius(float t){\n"
"	t = abs(t -10.);\n"
 "   t *= 0.01;\n"
  "  return clamp(t*t, 50./705., 80.);\n"
"}\n"

"bool Scene(vec3 ro, vec3 rd, out float t, vec2 fragCoord){\n"
"	bool hit = false;\n"
 "   float s = 0.;\n"
"	t = 1.2 + Hash(fragCoord);\n"
 "   float sphereR = SphereRadius(t);\n"
  "  for(int i = 0; i < 105; i++){\n"
   "     if(s >.8){\n"
    "        hit = true;\n"
     "       break;\n"
      "  }\n"
       " if(t > 3900.){\n"
         "   if(s != 0.){\n"
        "    	hit = true;\n"
          "  }\n"
        "	break;\n"
        "}\n"
        "vec3 p = ro+rd*t;\n"
        "float h = Map(p);\n"
        "sphereR = SphereRadius(t);\n"
        "if(h < sphereR){\n"
         "   s = (1.-s)*(sphereR-h)/(sphereR);\n"
          "  //if(s > 1./sphereR){\n"
           " 	hit = true;\n"
            "//}\n"
            "//hit = true;\n"
            "//break;\n"
            "//s+=s;\n"
            
        "}\n"
    "    t += 0.5*h + 0.003*t;\n"
    "}\n"
"    return hit;\n"
"}\n"
"vec3 sunLight  = normalize( vec3(  0.5, 0.5,  0.1 ) );\n"
"vec3 sunColour = vec3(1.0, .9, .83);\n"
"float specular = 0.0;\n"
"float ambient;\n"
"void DoLighting(inout vec3 mat, in vec3 pos, in vec3 normal, in vec3 eyeDir, in float dis)\n"
"{\n"
"	float h = dot(sunLight,normal);\n"
"	float c = max(h, 0.0)+ambient;\n"
"	mat = mat * sunColour * c ;\n"
"	// Specular...\n"
"	if (h > 0.0)\n"
"	{\n"
"		vec3 R = reflect(sunLight, normal);\n"
"		float specAmount = pow( max(dot(R, normalize(eyeDir)), 0.0), 100.0)*specular;\n"
"		mat = mix(mat, sunColour, specAmount);\n"
"	}\n"
"}\n"
"vec3 GetNormal(vec3 p, float distance){\n"
    "float t = min(.3, .0005+.00005 * distance*distance);\n"
	"vec3 eps = vec3(t, 0., 0.);\n"
    "vec3 nor = (vec3(0., Terrain2(p.xz, 10), 0.));\n"
    "vec3 v2 = nor - vec3(eps.x, Terrain2((eps+p).xz, 10), 0.);\n"
    "vec3 v3 = nor - vec3(0., Terrain2((p-eps.yyx).xz, 10), -eps.x);\n"
    "return normalize(cross(v2, v3));\n"
"}\n"

"vec3 renderRefl(vec3 ro, vec3 rd, vec2 fragCoord){\n"
"	vec3 col = vec3(0.2, 0.3, 0.6);\n"
 "   float t;\n"
  "  if(Scene(ro, rd, t, fragCoord)){\n"
    	"vec3 normal = GetNormal(ro+rd*t, t);\n"
        "vec3 mat;\n"
    "ambient = 0.1;\n"
    "specular;\n"
    "vec3 matPos = (ro+rd*t)*2.;\n"
    "float d =t*t;\n"
    
    "float f = clamp(Noise(matPos.xz*0.05),0., 1.);\n"
   "	vec3 m = mix(vec3(0.63*f+.2, .7*f+.1, .7*f+.1), vec3(f*.43+.1, f*.3+.2, f*.34+.1), f*0.65);\n" 
    "mat = m*vec3(f*m.x+.36, f*m.y+.3, f*m.z+.28);\n"
   
    "if(matPos.y < 25. || normal.y > .45){\n"
    "	m = vec3( Noise(matPos.xz*0.035)*.5+.15, Noise(matPos.xz*.03)*.6+.25,0.);\n"
     "   m *= (normal.y - 0.65)*.6;\n"
      "  mat = mix(mat, m, clamp((normal.y-.65)*1.3*(45.-matPos.y)*.1,0., 1.));\n"
    "}\n"
    "col = 0.2*mat;\n"
    "}\n"
    "return col;\n"
"}\n"

"vec3 TerrainColour(vec3 ro, vec3 pos, vec3 normal, float dis, vec2 fragCoord){\n"
"	vec3 mat;\n"
 "   ambient = 0.1;\n"
  "  specular;\n"
   " vec3 dir = pos - ro;\n"
    "vec3 matPos = pos*2.;\n"
    "float d = dis*dis;\n"
    
    "float f = clamp(Noise(matPos.xz*0.05),0., 1.);\n"
   "	vec3 m = mix(vec3(0.63*f+.2, .7*f+.1, .7*f+.1), vec3(f*.43+.1, f*.3+.2, f*.34+.1), f*0.65);\n" 
    "mat = m*vec3(f*m.x+.36, f*m.y+.3, f*m.z+.28);\n"
    "if(normal.y < .5){\n"
    "	float c = (.5-normal.y)*4.;\n"
     "   c = clamp(c*c, 0.1, 1.);\n"
      "  f = Noise(vec2(matPos.x*.09, matPos.z*.095) + matPos.yy*.15);\n"
       " f += Noise(vec2(matPos.x*2.233, matPos.z*2.23))*.5;\n"
       " mat = mix(mat, vec3(.4*f), c);\n"
        "specular += .1;\n"
    "}\n"
    "if(matPos.y < 25. || normal.y > .45){\n"
    "	m = vec3( Noise(matPos.xz*0.035)*.5+.15, Noise(matPos.xz*.03)*.6+.25,0.);\n"
     "   m *= (normal.y - 0.65)*.6;\n"
      "  mat = mix(mat, m, clamp((normal.y-.65)*1.3*(45.-matPos.y)*.1,0., 1.));\n"
    "}\n"
    "if(treeCol > 0.){\n"
    "	mat = vec3(.02+Noise(matPos.xz*5.)*.03, .05, .0);\n"
     "   normal = normalize(normal+vec3(Noise(matPos.xz*33.)-.5, 0., Noise(matPos.xz*3.)-.5));\n"
      "  specular = 0.;\n"
    "}\n"
    "if(matPos.y > 35. || normal.y >= .42){\n"
    "	float snow = Noise(matPos.xy*0.0005)*clamp((matPos.y - 35.0 - Noise(matPos.xz * .1)*28.0) * 0.035, 0.0, 1.0);\n"
"		mat = mix(mat, vec3(.7,.7,.8), snow);\n"
"		specular += snow;\n"
"		ambient+=snow *.3;\n"
 "   }\n"
  "  if(matPos.y < 2.){\n"
   "     if(normal.y > .4){\n"
    "    	f = Noise(matPos.xz*0.084)*1.5;\n"
     "       f = clamp((2.0 - f - matPos.y)*1.35,0., 0.67);\n"
      "      float t = normal.y - .4;\n"
       "     t = t*t;\n"
        "    mat = mix(mat, vec3(.09+t , .07+t, .03+t), f);\n"
        "}\n"
    "}\n"
    "if(matPos.y < 0.){\n"
    "	mat*=.5;\n"
    "}\n"
    "DoLighting(mat, pos, normal,dir, dis*dis);\n"
	
    "if(matPos.y < 0.){\n"
    "	//matPos.y = 0.;\n"
     "   float time = (time*.03);\n"
      "  vec3 watPos = matPos;\n"
        "watPos += -dir*(watPos.y/dir.y);\n"
        
        "float tx = cos(watPos.x*.052)*4.5;\n"
        "float tz = sin(watPos.z*0.072)*4.5;\n"
        "vec2 co = Noise3(vec2(watPos.x*4.7 + 1.3 * tz, watPos.z * 4.69 + time*35.-tx));\n"
        "mat*=0.5;\n"
		"mat += .5*mix(vec3(0.1, 0.1, 0.5), vec3(0.8, 0.8, 0.72)*0.7, clamp(dot(normal, sunLight)*.3, 0.2, 0.9));\n"
        "pos.y = 0.;\n"
        "pos.x += tx;\n"
        "pos.y += tz;\n"
        "//mat = mix(renderRefl((pos), reflect(dir, normal.xyz), fragCoord),mat,0.9);\n"
    	
    "}\n"
    "float fog = exp(-.00005 * dis*dis);\n"
"	return mix(vec3(0.9, 0.9, 0.9), mat, fog);\n"
"}\n"

"void main(void)\n"
"{\n"
 "   vec3 normal;\n"
"	vec2 p = 2.*uv-1.;\n"
//"	p.x*=1306./705.;\n"
   " float x = 10. + 10.*time + 10.*mouse.x;\n"
    "float y = 5.;//+iMouse.y;\n"
    "float z = 8. + 10.*time + 10.*mouse.y;\n"
    
    "vec3 ro = vec3(x, y, z);\n"
    "ro.y += Terrain(ro.xz, 5);\n"
   "vec3 lookAt = .5*ro;//vec3(0.)+4.*vec3(0., ro.y, 0.);//0.5*ro+vec3(0., ro.y, 0.);\n" 
    
    
    
    "vec3 eye = normalize(ro - lookAt);\n"
    "vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));\n"
    "vec3 up = normalize(cross(right, eye));\n"
	
    "vec3 rd = normalize(eye*0.8 + right*p.x*0.8 + up*p.y);\n"
    
    "float distance;\n"
    "vec3 col;\n"
    
    "if( !Scene(ro, rd, distance, uv )|| distance > 500.){\n"
    "	gl_FragColor = vec4(0.7, 0.7, 0.72, 0.);\n"
     "   float fog = exp(-.0005 * distance*distance);\n"
"		gl_FragColor = mix(vec4(0.9, 0.9, 0.9, 1.), gl_FragColor, fog);\n"
 "       return;\n"
  "  }\n"
   " else{\n"
    "	vec3 pos = ro + rd*distance;\n"
     "   normal = GetNormal(pos, distance);\n"
    	
      "  col = TerrainColour(ro, pos, normal, distance, uv);\n"
       // col += pow(dot(normal, normalize(sunLight)), 10.)/5.;
    "}\n"
    //col += pow(dot(normal, normalize(vec3(1., 1., 1.))), 2.)*0.5;
"	gl_FragColor = vec4(col, 1.);\n"    
"}\n"*/



/*
"uniform float time;\n"
"in vec2 uv;\n"
"float Hash( float n )\n"
"{\n"
"    return fract(sin(n)*3355.545383);\n"
"}\n"
"float Noise(vec3 x){\n"
	"vec3 p = floor(x);\n"
    "vec3 f = fract(x);\n"
    "f = f*f*(3.-2.*f);\n"
    "float n = p.x + 157.*p.y + 113.*p.z; \n"
    "return mix(mix(mix(Hash(n+0.), Hash(n+1.), f.x), mix(Hash(n+157.),Hash(n+158.), f.x), f.y),mix(mix(Hash(n+113.), Hash(n+114.), f.x),mix(Hash(n+270.), Hash(n+271.), f.x), f.y), f.z);\n"
"}\n"
"vec3 Noise( in vec2 x )\n"
"{\n"
"	x+=4.2;\n"
  "  vec2 p = floor(x);\n"
 "   vec2 f = fract(x);\n"

    //vec2 u = f*f*(3.0-2.0*f);
"	vec2 u = f*f*f*(6.0*f*f - 15.0*f + 10.0);\n"
 "   float n = p.x + p.y*507.0;\n"

"    float a = Hash(n+  0.0);\n"
   " float b = Hash(n+  1.0);\n"
  "  float c = Hash(n+ 507.0);\n"
 "   float d = Hash(n+ 508.0);\n"
"	return 1.*vec3(a+(b-a)*u.x+(c-a)*u.y+(a-b-c+d)*u.x*u.y,6.0*f*(f-1.0)*(vec2(b-a,c-a)+(a-b-c+d)*u.yx));\n"
"}\n"
"mat2 m = mat2(cos(22./21.), sin(22./21.), -sin(22./21.), cos(22/21.));\n"
"vec3 Fbm(in vec2 p){\n"
"	vec3 h;\n"
    "h = 5.0*Noise(p-vec2(0., 12.)); p *= 2.02*mat2(cos(22./21.), sin(22./21.), -1.*sin(22./21.), cos(22/21.));\n"
    "h+= 2.5*Noise(p); p *= 2.33*mat2(cos(22./21.), sin(22./21.), -1.*sin(22./21.), cos(22/21.));\n"
    "h+= 1.25*Noise(p); p*= 2.01*mat2(cos(22./21.), sin(22./21.), -1.*sin(22./21.), cos(22/21.));\n"
   " h+= 0.625*Noise(p); p*= 2.01*mat2(cos(22./21.), sin(22./21.), -1.*sin(22./21.), cos(22/21.));\n"
  "  h+= 0.3125*Noise(p); p *= 2.33*mat2(cos(22./21.), sin(22./21.), -1.*sin(22./21.), cos(22/21.));\n"
    //h+= 0.150625*Noise(p); p*= 2.01*m;
    //h+= 0.0703125*Noise(p); p*= 2.01*m;
 "   return h;\n"
"}\n"

"float Fbm(in vec3 p){\n"
"	float h;\n"
    "float t = 1.;\n"
    
    //p = 2.*floor(2.*p);
    "h = Noise(p+vec3(0.,0., 0.)); p *= 2.02*t;\n"
    "h+= 2.5*Noise(p); p *= 2.33*t;\n"
   " h+= 1.25*Noise(p); p*= 2.01*t;\n"
  "  h+= 0.625*Noise(p); p*= 2.01*t;\n"
//    h+= 0.3125*Noise(p); p *= 2.33*t;
    //h+= 0.150625*Noise(p); p*= 2.01*t;
    //h+= 0.0703125*Noise(p); p*= 2.01*t;
 "   return h;\n"
"}\n"
"float Terrain(vec3 p){\n"
    "float density;\n"
    "vec3 pos = p;\n"
    //float hard_floor = -4.;
    //pos += clamp((hard_floor-p.y)*3., -0., 1.)*40.;	
    
    "float freq = 4.;\n"
    "float amp = .25;\n"
    "for(int i = 0; i < 11; i++){\n"
        "density += Noise(pos*freq)*amp;\n"
        "amp *= 2.;\n"
        "freq *= .5;\n"
    "}\n"
    "float hardFloor = -13.;\n"
    "density += clamp((hardFloor+p.y)*4., 0., 1.)*40.;\n"  //try 1. instead of 4.
    "density = clamp(density, 0., 420.);\n"
    "return density;\n"
	//"float distanceFlat = 0.1*length(p.xz-vec2(-100.))/500.;\n"
 //"   return mix(density, p.y + 310., distanceFlat);\n"
"}\n"
"float map(vec3 p){\n"
 "	return p.y - Terrain(p);\n"
"}\n"
"vec3 GetNormal(vec3 p, float t){\n"
"	vec3 eps = vec3(13., 0., 0.);\n"
    "vec3 normal = vec3(0., map(p), 0.);\n"
    "vec3 v2 = normal - vec3(eps.x, map(p + eps), 0.);\n"
    "vec3 v3 = normal - vec3(0., map(p - eps.yyx), -eps.x);\n"
    "return normalize(cross(v2, v3));\n"
"}\n"
"vec3 GetNormalH(vec3 p){\n"
	"vec3 eps = vec3(0.1, 0., 0.);\n"  //5 gave big reflections
    "vec3 normal = vec3(0., map(p), 0.);\n"
    "vec3 v2 = normal - vec3(eps.x, map(p + eps), 0.);\n"
    "vec3 v3 = normal - vec3(0., map(p - eps.yyx), -eps.x);\n"
    "return normalize(cross(v2, v3));\n"
"}\n"
"bool Scene(in vec3 ro, in vec3 rd, out float t){\n"
	"bool hit = false;\n"
    "for(int i = 0; i < 300; i++)\n"
    "{\n"
    "	if(hit || t > 2600.) break;\n"
        "float h = map(ro + rd*t);\n"
        "if(h < 0.1){\n"
     "       hit = true;\n"
    "    }\n"
   "     t += h*0.5 + 0.003*t;\n"
  "  }\n"
    
 "   return hit;\n"
"}\n"


"void main(void)\n"
"{\n"
"	vec2 p = 2.* uv - 1.;\n"
   //"p -= 1.;\n"
    
    "p.x *= 1366./705.;\n"
    "p.y *= 705./1366.;\n"
    "float x = 1. +(6.*time);\n"
    "float y = 60.;\n"
    "float z = 1. + 6.*time;\n"
    
    "vec3 ro = vec3(x, y, z);\n"
    "ro.y = 250.+ Terrain(ro);\n"
    
    "vec3 lookAt = vec3(0.);\n"
    
    "vec3 eye = normalize(lookAt - ro);\n"
    "vec3 right = normalize(cross(eye, vec3(0., 1., 0.)));\n"
    "vec3 up = normalize(cross(right, eye));\n"
    "float effect;\n"
    "vec3 rd = normalize(0.8*eye + p.x*right + p.y*up);\n"
"	float t;\n"
    "vec3 normal, col, lCol = vec3(0.42, 0.4, 0.4);\n"
    "if(Scene(ro, rd, t)){\n"
   "     normal = GetNormal(ro+rd*t, t);\n"
   "		col = vec3(0.172, 0.135, .241);\n"
    "	vec3 lPos = normalize(Noise(ro+rd*t)*(ro+rd*t)+vec3(0., 500., 0.));\n"
    	
    "	float ambient = 0.3;\n"
   " 	col += ambient*lCol;\n"
   " 	col += .7*max(dot(normal, normalize(lPos)), 0.)*lCol;\n"
   "     col = mix(vec3(0.1, 0.4, 0.7),normal, 0.8);\n"
   "     col += 0.3*lCol*Noise(max(sin(0.0625*time), 0.2)*0.05*(ro+rd*t));\n"
   "     col -= 0.3*lCol*(Noise(max(cos(0.125*time), 0.2)*0.025*(ro+rd*t)));\n"
   "     col += 0.1*lCol*(Noise(max(cos(.25*time), 0.2)*0.0125*(ro+rd*t)));\n"
   "     col -= 0.225*lCol*Noise(max(sin(0.25*time), 0.2)*0.0625*(ro+rd*t));\n"
  "  	vec3 nor = (normalize(normal-.5+GetNormal(0.3*(ro+rd*t), 2.)));\n"
  "      float sky = clamp(.5+.5*nor.y, 0., 1.);\n"
   "     lCol += 0.7 * sky * lCol;\n"
  "      col += .7*max(dot(normal, normalize(lPos)), 0.)*lCol;\n"
  "      effect += 0.3*(max(cos(0.125*time), 0.2));\n"
  "      effect += 0.1*max(cos(.25*time), 0.2);\n"
   "     effect += 0.225*max(sin(0.25*time), 0.2);\n"
  "      effect +=.3*max(sin(0.0625*time), 0.2);\n"
  "      col += 0.5*effect*pow(dot(lPos, reflect(normalize(ro+rd*t), GetNormalH(ro+rd*t))), 150.);\n"
 "   }\n"
 "   else{\n"
 "   	col = mix(vec3(0.2, 0.2, 0.4), vec3(0.1, 0.1, .5), 1.-p.y);\n"
 "   }\n"
    
 "   col = mix(vec3(0.04, 0.04, 0.10), col, clamp(exp(-t/50.*0.05), 0., 0.7));\n"
 "   col += clamp(1.-effect, 0.,1.)*mix(vec3(0.),vec3(0.0, 0.2, 0.7),p.y);\n"
    
    //another fog
  "  col = mix(vec3(0.04, 0.2, 0.30), col, clamp(exp(length(ro+rd*t)/50.*0.01), 0., 0.8));\n"
 "   gl_FragColor = vec4(col, 1.);\n"
"}"*/
);

GLuint vertexBufferObject;
GLuint vao;
GLuint positionAttrib;
float time, m;
void init(){
	std::vector<GLuint> shaderList;
	shaderList.push_back(createShader(GL_VERTEX_SHADER, strVertexShader));
	shaderList.push_back(createShader(GL_FRAGMENT_SHADER, strFragmentShader));
	theProgram = createProgram(shaderList);
	std::for_each(shaderList.begin(), shaderList.end(), glDeleteShader);
	
	glUseProgram(theProgram);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertexBufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
	positionAttrib = glGetAttribLocation(theProgram, "position");
	
	glEnableVertexAttribArray(positionAttrib);
	glVertexAttribPointer(positionAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
}
float mousePosition[2] = {0.f, 0.f};
void display(){
	glClearColor(0, 0, 0, 0);
	//glClear(GL_COLOR_BUFFER_BIT);
	GLuint t = glGetUniformLocation(theProgram, "time");
	GLuint click = glGetAttribLocation(theProgram, "mouse");
	glUniform1f(t, time);
	glUniform2f(m, mousePosition[0], mousePosition[1]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y){
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
		printf("Left Click at (%d, %d).\n", x, y);
		printf("Translating (%f, %f)\n", (float)x/2.f-250.f, (float)y/2.f-250.f);
		mousePosition[0] = x;
		mousePosition[1] = y;
	}
	glutPostRedisplay();
}

void timer(int x){
	time+=.1;
	glutTimerFunc(200, timer, x);
	glutPostRedisplay();	
}
void reshape (int w, int h){
	glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

int main(int argc, char **argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB|GLUT_DEPTH|GLUT_DOUBLE);
	glutInitWindowPosition(50, 50);
	glutInitWindowSize(500, 500);
	glutCreateWindow(argv[0]);

	if (glewInit() != GLEW_OK){
        printf("OpenGL not available\n");
        return 1;
    } else {
		printf("Program ran successfully.....\n");
	}
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutTimerFunc(200, timer, 0);
	glutMainLoop();
	return 0;
}