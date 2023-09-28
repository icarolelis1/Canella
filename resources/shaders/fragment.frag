#version 450

layout( set = 1,binding = 0) uniform samplerCube irradiance_map;
layout( set = 1,binding = 0) uniform samplerCube specular_map;
layout( set = 1,binding = 0) uniform sampler2D   brdflut;

layout( set = 4,binding = 0) uniform sampler2D albedo_sampler;
layout( set = 4,binding = 1) uniform sampler2D ao_sampler;
layout( set = 4,binding = 2) uniform sampler2D metalness_sampler;
layout( set = 4,binding = 3) uniform sampler2D normal_sampler;
layout( set = 4,binding = 4) uniform sampler2D roughness_sampler;

layout (set = 0 ,binding = 0) uniform UBO
{
    mat4 view_projection;
    vec4 eye;
    mat4 projection;
    mat4 view;
} ubo;

layout (location = 0) in VertexInput {
  vec3 world_pos;
  vec2 uv;
  vec3 normal;
} vertexInput;

layout(location = 0) out vec4 outFragColor;
layout(location = 1) out vec3 outAlbedos;
layout(location = 2) out vec2 outNormals;

const vec3 SUN = vec3(234,544,1000);
const vec3 SUN_COLOR = vec3(10);
const float PI = 3.14159265359;


vec3 get_normal_from_map(vec2 scale)
{
    vec3 tangent = texture(normal_sampler,  vertexInput.uv.xy * scale).xyz * 2.0 - 1.0;
    vec3 Q1  = dFdx(vertexInput.world_pos);
    vec3 Q2  = dFdy(vertexInput.world_pos);
    vec2 st1 = dFdx(vertexInput.uv.xy);
    vec2 st2 = dFdy(vertexInput.uv.xy);
    vec3 N   = normalize(vertexInput.normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);
    return normalize(TBN * tangent);
}
// ----------------------------------------------------------------------------
float distributioGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float geometry_schlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometry_schlickGGX(NdotV, roughness);
    float ggx1 = geometry_schlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnel_schlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
vec3 fresnel_schlick_roughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main()
{
    vec2 uv = vertexInput.uv.xy;
    vec3 world_pos =  vertexInput.world_pos;
    // material properties
    vec3 albedo = pow(texture(albedo_sampler, uv).rgb, vec3(2.2));
    float metallic = texture(metalness_sampler, uv).r;
    float roughness = texture(roughness_sampler, uv).r;
    float ao = texture(ao_sampler, uv).r;

    vec3 N = get_normal_from_map(vec2(1.0,1.0));
    vec3 V = normalize(ubo.eye.xyz - world_pos);
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 Lo = vec3(0.0);

    for(int i = 0; i < 1;i++)
    {
        vec3 L = normalize(SUN - world_pos);
        vec3 H = normalize(V + L);
        float distance = length(SUN_COLOR - world_pos);
        float attenuation = 1.0; /// (distance * distance);
        vec3 radiance = SUN_COLOR * attenuation;

        float NDF = distributioGGX(N, H, roughness);
        float G   = geometry_smith(N, V, L, roughness);
        vec3 F    = fresnel_schlick(max(dot(H, V), 0.0), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;
    }

    vec3 F = fresnel_schlick_roughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;
    vec3 irradiance = texture(irradiance_map, N).rgb;
    vec3 diffuse      = irradiance * albedo;

    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefiltered_color = textureLod(specular_map, R,  roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdflut, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefiltered_color * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    outFragColor = vec4(color,1.0);
}