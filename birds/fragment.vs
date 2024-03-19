#version 330 core
out vec4 color;

in vec3 normal;
in vec3 FragPos;
in vec2 TexCoord;
in vec2 pos;

// texture samplers
uniform sampler2D texture0;
uniform sampler2D texture1;
//uniform sampler2D texture2;
uniform float st;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec2 iResolution;
//uniform vec3 lightColor;
//uniform vec3 objectColor;

//holographic texture from
//<a href="https://www.freepik.com/free-vector/abstract-hologram-gradient-background_4350032.htm#query=holographic%20texture&position=1&from_view=keyword&track=ais&uuid=55dca26a-b949-4558-9223-783dcb5fcb72">Image by kjpargeter</a> on Freepik

#define MOD int(mod(st/1000.0/3.,4.))

#define hash(p) fract(sin(dot(p,vec3(127.1,311.7, 74.7)))*43758.5453123)

float noise (vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
	f = f*f*(3.0-2.0*f); // smoothstep

    float v= mix( mix( mix(hash(i+vec3(0.0,0.0,0.0)),hash(i+vec3(1.0,0.0,0.0)),f.x),
                       mix(hash(i+vec3(0.0,1.0,0.0)),hash(i+vec3(1.0,1.0,0.0)),f.x), f.y), 
                  mix( mix(hash(i+vec3(0.0,0.0,1.0)),hash(i+vec3(1.0,0.0,1.0)),f.x),
                       mix(hash(i+vec3(0.0,1.0,1.0)),hash(i+vec3(1.0,1.0,1.0)),f.x), f.y), f.z);
	//return   MOD==0.0 ? v
	//       : MOD==1.0 ? 2.0*v-1.0
    //       : MOD==2.0 ? abs(2.0*v-1.0)
    //                : 1.0-abs(2.0*v-1.0);
	if (MOD == 0.0) {
		return v;
	}
	if (MOD == 1.0) {
		return 2.0*v-1.0;
	}
	if (MOD == 2.0) {
		return abs(2.0*v-1.0);
	}
	return 1.0 - abs(2.0*v-1.0);
}

#define rot(a) mat2(cos(a),-sin(a),sin(a),cos(a))

float fbm (vec3 p) {
    float v = 0.2;
	float a = 0.5;
    mat2 R = rot(0.87 + (st/100.0)/1e4);

    for (int i = 0; i < 9; i++, p*=2.0,a/=2.0) {
        p.xy *= R;
		p.yz *= R;
        v += a * noise(p);
	}

    return v;
}

void main()
{

	//vec2 posi = vec2(pos.x + sin(st), pos.y + cos(st));
	vec3 fp = vec3(pos, 0.0);
	vec3 vp = vec3(0.0, 0.0, 0.0);
	//vec3 lightColor = vec3(0.9, 0.9, 1.0);
	vec2 scaled = 1.0*TexCoord;
	vec2 scaled2 = vec2(scaled.x, scaled.y+(0.005*sin((st/100.0)+scaled.x)));
	scaled2 = vec2(scaled.x+(0.005*cos((st/100.0)+scaled.y)), scaled.y);
	scaled2 = scaled;
	scaled = vec2(scaled.x, scaled.y);

		//rainbow
	vec2 U = (2.0*pos.xy) / 1.0;
	vec4 rainbow_color = .5+ .55* cos( 9.*fbm(vec3(U,(st/100.0)/3.))+ vec4(0,23,21,0));
	if ( length(U*4.-vec2(.1,.2*float(MOD+1)))<.1 ) rainbow_color--;
	rainbow_color = abs(rainbow_color);
	//vec3 lightColor = vec3(rainbow_color);
	//vec3 lightColor = vec3(texture(texture1, scaled2));
	vec3 lightColor = vec3(rainbow_color);
	vec3 lightColor2 = vec3(texture(texture0, scaled2));
	vec3 lightColor3 = vec3(texture(texture1, scaled2));
	
	// linearly interpolate between both textures
	//FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2);
	//FragColor = texture(texture0, TexCoord);
	
	color = texture(texture0, scaled);

	//lighting
	//ambient
	float ambientStrength = 0.0;
	vec3 ambient = ambientStrength * lightColor;

	//diffuse
	vec3 norm = normalize(normal);
	vec3 lightDir = normalize(lightPos - fp);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	//specular
	float specularStrength = 0.4;
	vec3 viewDir = normalize(vp - fp);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength*spec*lightColor;

	//vec3 result = (ambient + diffuse + specular) * lightColor;
	//vec3 result = (ambient + diffuse + specular) * vec3(rainbow_color);
	vec3 result = (ambient + diffuse + specular) * vec3(texture(texture1, scaled2));

	//COLOR 2 /////////////////////////////////////////

	//lighting
	//ambient
	float ambientStrength2 = 0.0;
	vec3 ambient2 = ambientStrength2 * lightColor2;

	//diffuse
	vec3 norm2 = normalize(normal);
	vec3 lightDir2 = normalize(lightPos - fp);
	float diff2 = max(dot(norm2, lightDir2), 0.0);
	vec3 diffuse2 = diff2 * lightColor2;

	//specular
	float specularStrength2 = 0.9;
	vec3 viewDir2 = normalize(vp - fp);
	vec3 reflectDir2 = reflect(-lightDir2, norm2);
	float spec2 = pow(max(dot(viewDir2, reflectDir2), 0.0), 32);
	vec3 specular2 = specularStrength2*spec2*lightColor2;
	vec3 result2 = (ambient2 + diffuse2 + specular2) * lightColor2;
	vec3 result3 = (ambient2 + diffuse2 + specular2) * lightColor3;

	//vec3 tex2col = vec3(texture(texture1, scaled2));

	//if (tex2col.x > 0.9 && tex2col.y > 0.9 && tex2col.z > 0.9) {
	//	color = texture(texture0, scaled2);
	//} else if (color.x != 0.0 && color.y != 0.0 && color.z != 0.0) {
	//	color = mix(vec4(result, 1.0), color, 1.0);
	//}
	if (color.w == 0.0) {

	} else if (color.x < 0.8 && color.y < 0.8 && color.z < 0.8) {
		color = mix(color, vec4(result, 1.0), 0.7);
		//color = mix(color, vec4(result, 1.0), smoothstep(0.0, 1.0, abs(spec)));
		//smoothstep(0.1,0.9,st.x)
	} else {
		//color = mix(color, vec4(result2, 1.0), 1.0);
		//color = mix(color, vec4(result, 1.0), (1.0+(sin(spec)/2.0))*0.7);
		color = mix(color, vec4(result3, 1.0), 0.6);
	}

	if (color.x > 0.8 && color.y > 0.8 && color.z > 0.8) {
		//color = vec4(1.0, 1.0, 1.0, 1.0);
	}

	//color = vec4(vec3(color) + result, 1.0);

	//if (color.x == 0.0 && color.y == 0.0 && color.z == 0.0) {
	//	color = vec4(0.0, 1.0, 0.0, 1.0);
	//}
}