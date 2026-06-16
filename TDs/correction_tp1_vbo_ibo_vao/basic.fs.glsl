#version 120

varying vec3 v_Color;

// un FS doit toujours ecrire dans 
// gl_FragColor qui est un vec4

void main()
{
    gl_FragColor = vec4(v_Color, 1.0);
}