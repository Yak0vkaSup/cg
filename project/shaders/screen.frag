#version 330 core
// OWNER : Personne C (Pipeline)
// Recopie la texture du FBO a l'ecran et applique la correction GAMMA finale
// (lineaire -> sRGB). C'est aussi l'endroit ideal pour le post-traitement (3.a).
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D uScreen;

void main() {
    vec3 color = texture(uScreen, vUV).rgb;

    // TODO (3.a) : post-traitement ici (negatif, grayscale, blur, vignette...).
    // Exemple grayscale : color = vec3(dot(color, vec3(0.299, 0.587, 0.114)));

    // Correction gamma : espace lineaire -> sRGB pour l'affichage.
    color = pow(color, vec3(1.0 / 2.2));
    FragColor = vec4(color, 1.0);
}
