#version 450 core

layout (location = 2) uniform bool show_pathing_map_static;
layout (location = 3) uniform bool show_lighting;
//layout (location = 3) uniform bool show_pathing_map_dynamic;

layout (binding = 3) uniform usampler2D pathing_map_static;
//layout (binding = 3) uniform usampler2D pathing_map_dynamic;

layout (binding = 4) uniform sampler2DArray sample0;
layout (binding = 5) uniform sampler2DArray sample1;
layout (binding = 6) uniform sampler2DArray sample2;
layout (binding = 7) uniform sampler2DArray sample3;
layout (binding = 8) uniform sampler2DArray sample4;
layout (binding = 9) uniform sampler2DArray sample5;
layout (binding = 10) uniform sampler2DArray sample6;
layout (binding = 11) uniform sampler2DArray sample7;
layout (binding = 12) uniform sampler2DArray sample8;
layout (binding = 13) uniform sampler2DArray sample9;
layout (binding = 14) uniform sampler2DArray sample10;
layout (binding = 15) uniform sampler2DArray sample11;
layout (binding = 16) uniform sampler2DArray sample12;
layout (binding = 17) uniform sampler2DArray sample13;
layout (binding = 18) uniform sampler2DArray sample14;
layout (binding = 19) uniform sampler2DArray sample15;

layout (location = 0) in vec2 UV;
layout (location = 1) in flat uvec4 texture_indices;
layout (location = 2) in vec2 pathing_map_uv;

in vec3 normal;

out vec4 color;

vec4 get_fragment(uint id, vec3 uv) {
	vec2 dx = dFdx(uv.xy);
	vec2 dy = dFdy(uv.xy);

	switch(id) {
		case 0:
			return textureGrad(sample0, uv, dx, dy);
		case 1:
			return textureGrad(sample1, uv, dx, dy);
		case 2:
			return textureGrad(sample2, uv, dx, dy);
		case 3:
			return textureGrad(sample3, uv, dx, dy);
		case 4:
			return textureGrad(sample4, uv, dx, dy);
		case 5:
			return textureGrad(sample5, uv, dx, dy);
		case 6:
			return textureGrad(sample6, uv, dx, dy);
		case 7:
			return textureGrad(sample7, uv, dx, dy);
		case 8:
			return textureGrad(sample8, uv, dx, dy);
		case 9:
			return textureGrad(sample9, uv, dx, dy);
		case 10:
			return textureGrad(sample10, uv, dx, dy);
		case 11:
			return textureGrad(sample11, uv, dx, dy);
		case 12:
			return textureGrad(sample12, uv, dx, dy);
		case 13:
			return textureGrad(sample13, uv, dx, dy);
		case 14:
			return textureGrad(sample14, uv, dx, dy);
		case 15:
			return textureGrad(sample15, uv, dx, dy);
		case 16:
			return vec4(0, 0, 0, 0);
	}
}


void main() {
	color = get_fragment(texture_indices.a & 31, vec3(UV, texture_indices.a >> 5));
	color = color * color.a + get_fragment(texture_indices.b & 31, vec3(UV, texture_indices.b >> 5)) * (1 - color.a);
	color = color * color.a + get_fragment(texture_indices.g & 31, vec3(UV, texture_indices.g >> 5)) * (1 - color.a);
	color = color * color.a + get_fragment(texture_indices.r & 31, vec3(UV, texture_indices.r >> 5)) * (1 - color.a);

	if (show_lighting) {
		vec3 light_direction = vec3(-0.3, -0.3, 0.25);
		light_direction = normalize(light_direction);

		color *= clamp(dot(normal, light_direction) + 0.45, 0, 1);
	}

	uvec4 byte = texelFetch(pathing_map_static, ivec2(pathing_map_uv), 0);
	if (show_pathing_map_static) {
		color = color + vec4(min(byte.r & 2, 1), min(byte.r & 4, 1), min(byte.r & 8, 1), 255) * 0.25;
	}

	// byte = texelFetch(pathing_map_dynamic, ivec2(pathing_map_uv), 0);
	// if (show_pathing_map_dynamic) {
	// 	color = color + vec4(min(byte.r & 2, 1), min(byte.r & 4, 1), min(byte.r & 8, 1), 255) * 0.25;
	// }
}