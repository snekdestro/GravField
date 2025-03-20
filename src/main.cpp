#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <cmath>

class Particle
{
private:
    float distance(float otherx,float  othery){
        float xs = (otherx - this->x) * (otherx - this->x) ;
        float ys = (othery - this->y) * (othery - this->y) ;
        return std::sqrt(xs + ys);
    }
public:
    float x;
    float y;
    float vx;
    float vy;
    float mass; //kg
    bool STATIC;
    bool remove;
    
    Particle(float xi, float yi, float kg, bool s){
        remove = false;
        mass = kg;
        x = xi;
        y = yi;
        vx = 0;
        vy = 0;
        STATIC = s;
    }
    Particle(sf::Vector2i pissr, float kg, bool s){
        remove = false;
        STATIC = s;
        mass = kg;
        x = (float)pissr.x;
        y = (float)1080 - pissr.y;
        vx = 0;
        vy = 0;
    }
    void calc(double delta, Particle* p){
        if(this->remove || p->remove){
            return;
        }
        float dist = distance(p->x, p->y);
        //need to handle collisions
        
        if(dist < (p->mass+mass) / 100){
            if((this->mass < p->mass && !this->STATIC)){
                this->remove = true;
                p->mass += this->mass;
                p->vx = 0;
                p->vy = 0;
            }else{
                this->mass += p->mass;
                p->remove = true;
                this->vx = 0;
                this->vy = 0;
            }
            return;
        }
        float force = mass * p->mass / (dist * dist) * 999;

        double theta = std::atan(std::abs((p->y - y) / (p->x - x)));
        double aX = -force * std::cos(theta) / mass;
        double aY = -force * std::sin(theta) / mass;
        //perform reflections
        if(x <  p->x){
            aX *= -1;
        }
        if(y < p->y){
            aY *= -1;
        }
        //wierd ass math that works like magic
        if(!STATIC){
        vx += aX * delta;
        vy += aY * delta;
        x += vx  * delta;
        y += vy * delta;
        }
    }



};
int main()
{
    sf::RectangleShape rect(sf::Vector2f(1920,1080));   
    sf::Shader fieldVecShader;
    //fieldVecShader.loadFromFile("shaders\\fieldvec.frag", sf::Shader::Fragment);
    std::string file  = "uniform int s;\n"
    "uniform float data[140];\n"
    "uniform float masses[70];\n"
    "void main(){\n"
    "vec2 force = vec2(0);\n"
    "for(int i = 0; i < 2 * s; i+= 2){\n"
        "vec2 xy = vec2(data[i], data[i + 1]);\n"
        "vec2 ab = xy- gl_FragCoord.xy;\n"
        "float dist = dot(ab,ab);\n"
        "if(dist == 0){\n"
            "continue;\n"
        "}\n"
        "force += masses[i/2]/ dist * normalize(ab) * 100;\n"
    "}\n"
    "float fl = length(force);\n"
    "gl_FragColor = vec4(1.0, 0.0,0.0, 1.0 - 1.0/ fl);\n"
    "}\n";
    fieldVecShader.loadFromMemory(file, sf::Shader::Fragment);
    int idx = 0;
    int size = 0;
    int tracker = 0;
    float currentMass = 100;
    bool building = false;
    auto window = sf::RenderWindow({1920u, 1080u}, "CMake SFML Project");
    window.setFramerateLimit(144);
    bool shift = false;
    std::vector<Particle*> particles; 
    sf::Clock clock;
    while (window.isOpen())
    {
        if(building){
            currentMass += 100 * clock.getElapsedTime().asSeconds();
        }
        for (auto event = sf::Event(); window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
            if(event.type == sf::Event::KeyPressed){
                shift = event.key.shift;
            }
            if(event.type == sf::Event::KeyReleased){
                shift = event.key.shift;
            }
            if(event.type == sf::Event::MouseButtonPressed){
                if(event.MouseLeft){
                    
                    building = true;
                    
                }
            }
            if(event.type == sf::Event::MouseButtonReleased){
                if(event.MouseLeft){
                    
                    building = false; 
                    if(tracker >= 140){
                        continue;
                    }
                    particles.push_back(new Particle(sf::Mouse::getPosition(window),currentMass, shift));
                    size++;
                    fieldVecShader.setUniform("s" , size);
                    currentMass = 100.0f;
                }
            }
            
        }
        window.clear();
        for(int i =0; i <particles.size(); i++){
            for(int j = 0; j < particles.size(); j++){
                if(j == i || particles[j]->remove || particles[i]->remove){
                    continue;
                }
                particles[j]->calc(clock.getElapsedTime().asSeconds(), particles[i]);
            }
        }
        
        for(int i = 0; i < particles.size(); i++){
          fieldVecShader.setUniform("data["  + std::to_string(idx++) + "]", particles[i]->x );
        
            fieldVecShader.setUniform("data[" + std::to_string(idx++) + "]", particles[i]->y );
            fieldVecShader.setUniform("masses[" + std::to_string(i)  +"]", particles[i]->mass);
        }
        idx = 0;
        
        clock.restart();
        
        auto  new_end = std::remove_if(particles.begin(), particles.end(), [](const Particle* p){return p->remove;});
        particles.erase(new_end, particles.end());
        fieldVecShader.setUniform("s" , (int)particles.size());
        window.draw(rect, &fieldVecShader);
        window.display();
    }
}
