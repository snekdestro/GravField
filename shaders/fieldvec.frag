uniform int s;
uniform float data[140];
uniform float masses[70];

void main(){
    vec2 force = vec2(0);
    for(int i = 0; i < 2 * s; i+= 2){
        vec2 xy = vec2(data[i], data[i + 1]);
        vec2 ab = xy- gl_FragCoord.xy;
        float dist = dot(ab,ab);
        if(dist == 0){
            continue;
        }
        force += masses[i/2]/ dist * normalize(ab) * 100;
    }
    float fl = length(force);
    gl_FragColor = vec4(1.0, 0.0,0.0, 1.0 - 1.0/ fl);
}