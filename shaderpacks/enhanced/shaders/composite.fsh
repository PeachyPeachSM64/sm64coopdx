#version 120

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vFragPos;
varying vec3 vViewDir;

uniform sampler2D uTex0;
uniform int uLightingEnabled;
uniform int uNumLights;
uniform vec3 uAmbientLight;
uniform float uAmbientIntensity;

#define MAX_LIGHTS 32
#define LIGHT_TYPE_NONE 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_DIRECTIONAL 2
#define LIGHT_TYPE_SPOT 3

struct Light {
    int type;
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    float radius;
    vec3 attenuation;
    float innerCutoff;
    float outerCutoff;
};

uniform Light uLights[MAX_LIGHTS];

float fresnel(vec3 normal, vec3 viewDir, float power) {
    return pow(1.0 - max(dot(normal, viewDir), 0.0), power);
}

vec3 calculatePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
    
    if (distance > light.radius) {
        attenuation = 0.0;
    }
    
    float rim = fresnel(normal, viewDir, 3.0) * 0.3;
    
    vec3 diffuse = light.color * diff * light.intensity;
    vec3 specular = light.color * spec * light.intensity * 0.8;
    vec3 rimLight = light.color * rim * light.intensity * 0.5;
    
    return (diffuse + specular + rimLight) * attenuation;
}

vec3 calculateDirectionalLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    float rim = fresnel(normal, viewDir, 2.5) * 0.2;
    
    vec3 diffuse = light.color * diff * light.intensity;
    vec3 specular = light.color * spec * light.intensity * 0.6;
    vec3 rimLight = light.color * rim * light.intensity * 0.4;
    
    return diffuse + specular + rimLight;
}

vec3 calculateSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
    
    if (distance > light.radius) {
        attenuation = 0.0;
    }
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    intensity = smoothstep(0.0, 1.0, intensity);
    
    float rim = fresnel(normal, viewDir, 3.0) * 0.25;
    
    vec3 diffuse = light.color * diff * light.intensity;
    vec3 specular = light.color * spec * light.intensity * 0.7;
    vec3 rimLight = light.color * rim * light.intensity * 0.3;
    
    return (diffuse + specular + rimLight) * attenuation * intensity;
}

void main() {
    vec4 texColor = texture2D(uTex0, vTexCoord);
    
    if (uLightingEnabled == 0) {
        gl_FragColor = texColor;
        return;
    }
    
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(vViewDir);
    
    vec3 ambient = uAmbientLight * uAmbientIntensity;
    vec3 lighting = ambient;
    
    for (int i = 0; i < uNumLights && i < MAX_LIGHTS; i++) {
        if (uLights[i].type == LIGHT_TYPE_POINT) {
            lighting += calculatePointLight(uLights[i], norm, vFragPos, viewDir);
        } else if (uLights[i].type == LIGHT_TYPE_DIRECTIONAL) {
            lighting += calculateDirectionalLight(uLights[i], norm, viewDir);
        } else if (uLights[i].type == LIGHT_TYPE_SPOT) {
            lighting += calculateSpotLight(uLights[i], norm, vFragPos, viewDir);
        }
    }
    
    lighting = clamp(lighting, 0.0, 3.0);
    
    vec3 result = texColor.rgb * lighting;
    
    float luminance = dot(result, vec3(0.299, 0.587, 0.114));
    result = mix(result, vec3(luminance), -0.1);
    
    gl_FragColor = vec4(result, texColor.a);
}
