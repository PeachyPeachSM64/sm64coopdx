#version 120

varying vec2 vTexCoord;
varying vec3 vNormal;
varying vec3 vFragPos;

uniform sampler2D uTex0;
uniform int uLightingEnabled;
uniform int uNumLights;
uniform vec3 uAmbientLight;
uniform float uAmbientIntensity;
uniform vec3 uViewPos;

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

vec3 calculatePointLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
    
    if (distance > light.radius) {
        attenuation = 0.0;
    }
    
    vec3 diffuse = light.color * diff * light.intensity;
    vec3 specular = light.color * spec * light.intensity * 0.5;
    
    return (diffuse + specular) * attenuation;
}

vec3 calculateDirectionalLight(Light light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    vec3 diffuse = light.color * diff * light.intensity;
    vec3 specular = light.color * spec * light.intensity * 0.5;
    
    return diffuse + specular;
}

vec3 calculateSpotLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.attenuation.x + light.attenuation.y * distance + light.attenuation.z * (distance * distance));
    
    if (distance > light.radius) {
        attenuation = 0.0;
    }
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    
    vec3 diffuse = light.color * diff * light.intensity;
    vec3 specular = light.color * spec * light.intensity * 0.5;
    
    return (diffuse + specular) * attenuation * intensity;
}

void main() {
    vec4 texColor = texture2D(uTex0, vTexCoord);
    
    if (uLightingEnabled == 0) {
        gl_FragColor = texColor;
        return;
    }
    
    vec3 norm = normalize(vNormal);
    vec3 viewDir = normalize(uViewPos - vFragPos);
    
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
    
    vec3 result = texColor.rgb * lighting;
    gl_FragColor = vec4(result, texColor.a);
}
