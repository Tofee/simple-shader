#version 100

struct buf
{
    mat4 matrix;
    float opacity;
};

uniform buf ubuf;

attribute vec4 vertexCoord;
varying vec4 color;
attribute vec4 vertexColor;
attribute float _qt_order;

void main()
{
    gl_Position = ubuf.matrix * vertexCoord;
    color = vertexColor * ubuf.opacity;
    gl_Position.z = _qt_order * gl_Position.w;
}
