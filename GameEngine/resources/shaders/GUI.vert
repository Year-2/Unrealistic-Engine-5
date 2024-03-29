#version 140

in vec3 vertexPos3D;
in vec2 vUV;

out vec2 uv;

uniform mat4 projection;

void main() {
	vec4 v = projection * vec4(vertexPos3D.xyz, 1.0);
	gl_Position = v;
	uv = vUV;
}