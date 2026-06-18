#version 410 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uScene;     // image HDR rendue hors ecran (FBO, cf. 1.d)
uniform float     uExposure;  // facteur d'exposition
uniform int       uPostMode;  // 0=aucun, 1=niveaux de gris, 2=negatif, 3=sepia

void main() {
    // Lecture de la couleur HDR (espace lineaire) et application de l'exposition
    vec3 hdr = texture(uScene, vUV).rgb * uExposure;

    // Tone mapping de Reinhard : ramene les valeurs HDR dans [0,1[
    //   color = C / (C + 1)
    vec3 color = hdr / (hdr + vec3(1.0));

    // Effets de post-traitement optionnels (3.a)
    if (uPostMode == 1) {
        // Niveaux de gris (luminance percue)
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        color = vec3(gray);
    } else if (uPostMode == 2) {
        // Negatif
        color = vec3(1.0) - color;
    } else if (uPostMode == 3) {
        // Sepia
        color = vec3(dot(color, vec3(0.393, 0.769, 0.189)),
                     dot(color, vec3(0.349, 0.686, 0.168)),
                     dot(color, vec3(0.272, 0.534, 0.131)));
    }

    // Correction gamma (lineaire -> sRGB), gamma standard 2.2
    color = pow(clamp(color, 0.0, 1.0), vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
