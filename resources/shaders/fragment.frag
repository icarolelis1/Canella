/* Copyright (c) 2021, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450

layout( set = 3,binding = 0) uniform sampler2D albedo_sampler;


layout (location = 0) in VertexInput {
  vec4 color;
  vec2 uv;
} vertexInput;

layout(location = 0) out vec4 outFragColor;

void main()
{
	//outFragColor = vec4(vec3(linearize_depth(gl_FragCoord.z)/1000.),1.0f);
    vec2 uv = vertexInput.uv.xy;

    float gamma = 2.2;
    vec3 albedo = texture(albedo_sampler,uv).rgb;
    vec4 color = vec4(pow(albedo,vec3(1.0/gamma)),1.0);
	outFragColor = color;
}