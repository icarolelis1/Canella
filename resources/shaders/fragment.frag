/* Copyright (c) 2021, Sascha Willems
 *
 * SPDX-License-Identifier: MIT
 *
 */

#version 450
 
layout (location = 0) in VertexInput {
  vec4 color;
} vertexInput;

layout(location = 0) out vec4 outFragColor;
 

float linearize_depth(float depth)
{
    return (2.0 * 0.01 * 1000)/(0.01 + 1000 - (depth * 2.0 -1.0) * (1000 - 0.01));
}
void main()
{
	//outFragColor = vec4(vec3(linearize_depth(gl_FragCoord.z)/1000.),1.0f);
	outFragColor = vec4(vertexInput.color.xyz,0.5);
}