#version 130

void main()
{
    int gray = gl_VertexID ^ (gl_VertexID >> 1);
    gl_Position = vec4(
        (gray        & 1) * 2.0 - 1.0,
        ((gray >> 1) & 1) * 2.0 - 1.0,
        0.0,
        1.0);
}
