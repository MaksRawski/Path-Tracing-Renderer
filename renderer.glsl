// overall resources:
// - https://www.youtube.com/watch?v=Qz0KTGYJtUk
// - https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing (FOV, reflection stuff)

#version 330 core

uniform int iFrame;
uniform vec2 iResolution;
uniform sampler2D backBufferTexture;
uniform samplerBuffer trianglesBuffer;
uniform samplerBuffer meshesInfoBuffer;
uniform samplerBuffer materialsBuffer;
uniform int numOfMeshes;

const vec3 camOrigin = vec3(0, 1, 0);
const float radius = 2.5;
const float speed = 100;

#define INFTY 1.0e30

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#positionablecamera (12.2)
struct Camera {
    // Pozycja kamery w świecie.
    vec3 pos;
    // Punkt na który "patrzy" kamera.
    vec3 lookat;
    // Relatywny dla kamery kierunek "góra".
    vec3 up;
    // Field of view - pole widzenia, zapisane jako kąt w radianach.
    float fov;
};

struct Ray {
    // Punkt z którego promień wychodzi.
    vec3 origin;
    // Kierunek w jakim promień się porusza.
    vec3 dir;
    // Odwrotność kierunku w jakim promień się porusza,
    // obliczamy ją raz aby później móc ją łatwo wykorzystywać.
    vec3 inv_dir;
    // Wartość bardzo zbliżona do zera, będziemy uważać wszystkie liczby
    // co do modułu mniejsze od niej jako 0.
    float epsilon;
};

struct Material {
    // Jaki kolor emituje.
    vec3 emissionColor;
    // Siła z jaką emituje, z przedziału [0,1]
    float emissionStrength;
    // Kolor jaki odbija, pod białym światłem.
    vec3 albedo;
    // Decyduje o sposobie odbijania światła:
    // gdy 0, materiał jest matowy,
    // gdy 1, materiał jest idealnie płaski (odbija światło jak lustro).
    float specularComponent;
};

struct Sphere {
    vec3 pos;
    float r;
    Material mat;
};

struct HitInfo {
    bool didHit;
    float dst;
    vec3 hitPoint;
    vec3 normal;
    Material mat;
};

uint wang_hash(inout uint seed)
{
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}

float RandomFloat(inout uint state)
{
    return float(wang_hash(state)) / 4294967296.0;
}

const float C_PI = 3.141592653589793;
const float C_TWOPI = 6.283185307179586;
const int MAX_BOUNCE_COUNT = 8;
const int SAMPLES_PER_PIXEL = 2;

const int NUM_OF_SPHERES = 9;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
        Sphere(vec3(0.0, -1002.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 0.1)), // podłoga
        Sphere(vec3(0.0, 0.0, -1001.8), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0), 0.1)), // tylnia ściana
        Sphere(vec3(0.0, 0.0, 1002.8), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 1.0), 0.1)), // ściana z przodu (za kamerą)
        Sphere(vec3(-1003.5, 0.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 1.0, 0.0), 0.1)), // lewa ściana
        Sphere(vec3(1002.5, 0.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 0.0, 1.0), 0.1)), // prawa ściana
        Sphere(vec3(-2.2, -1.2, -0.5), 0.8, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 1.0)), // lewa kulka lustrzana
        Sphere(vec3(-0.5, -1.2, -0.5), 0.8, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 1.0)), // prawa kulka lustrzana
        Sphere(vec3(0.6, -1.4, 0.1), 0.4, Material(vec3(1.0, 1.0, 0.0), 0.4, vec3(1.0, 1.0, 1.0), 0.4)), // świecąca się kulka
        Sphere(vec3(-1.0, 101.971, 1.0), 100.0, Material(vec3(1.0, 1.0, 0.96), 1.0, vec3(1.0, 0.0, 0.0), 0.0)) // sufit (świecący)
    );


vec3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat(state) * 2.0f - 1.0f;
    float a = RandomFloat(state) * C_TWOPI;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}


HitInfo RaySphereIntersection(Ray ray, Sphere s) {
    HitInfo hitInfo;
    hitInfo.didHit = false;
    vec3 offsetRay = ray.origin - s.pos;
    float a = dot(ray.dir, ray.dir);
    float b = 2.0 * dot(ray.dir, offsetRay);
    float c = dot(offsetRay, offsetRay) - s.r * s.r;

    float d = b * b - 4.0 * a * c;
    if (d >= 0.0) {
        float d1 = (-b - sqrt(d)) / (2.0 * a);
        float d2 = (-b + sqrt(d)) / (2.0 * a);
        hitInfo.dst = min(d1, d2);

        if (hitInfo.dst > ray.epsilon) {
            hitInfo.didHit = true;
            hitInfo.hitPoint = ray.origin + hitInfo.dst * ray.dir;
            hitInfo.normal = normalize(hitInfo.hitPoint - s.pos);
            hitInfo.mat = s.mat;
        }
    }

    return hitInfo;
}

HitInfo CalculateRayCollision(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.dst = INFTY;

    for (int i = 0; i < NUM_OF_SPHERES; ++i) {
        HitInfo hit = RaySphereIntersection(ray, SPHERES[i]);
        if (hit.didHit && hit.dst < closestHit.dst) {
            closestHit = hit;
            closestHit.mat = SPHERES[i].mat;
        }
    }

    return closestHit;
}

vec3 DiffuseDirection(vec3 normal, inout uint rngState) {
    return normalize(normal + RandomUnitVector(rngState));
}

vec3 ReflectDirection(vec3 dir, vec3 normal) {
    return dir - 2.0 * dot(dir, normal) * normal;
}

vec3 GetColorForRay(Ray ray, inout uint rngState) {
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 incomingLight = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i <= MAX_BOUNCE_COUNT; ++i) {
        HitInfo hitInfo = CalculateRayCollision(ray);
        if (hitInfo.didHit) {
            ray.origin = hitInfo.hitPoint;
            vec3 diffuseDir = DiffuseDirection(hitInfo.normal, rngState);
            vec3 reflectDir = ReflectDirection(ray.dir, hitInfo.normal);
            ray.dir = mix(diffuseDir, reflectDir, hitInfo.mat.specularComponent);

            vec3 emittedLight = hitInfo.mat.emissionColor * hitInfo.mat.emissionStrength;
            incomingLight += emittedLight * c;
            c *= hitInfo.mat.albedo;
        } else {
            break;
        }
    }
    return incomingLight;
}

// void mainImage(out vec4 gl_FragColor, in vec2 gl_FragCcoord) {
void main(){
    uint rngState = uint(uint(gl_FragCoord.x) * uint(1973) + uint(gl_FragCoord.y) * uint(9277) + uint(iFrame) * uint(26699)) | uint(1);

    // Znormalizowane koordynaty piksela obecnie rozpatrywanego przez shader.
    // Są w przedziale [0,1]x[0,1]
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    // Wygodniej jednak będzie używać takich z przedziału [-1,1]x[-1,1].
    // Środek ekranu będzie mieć wtedy punkt (0,0).
    uv = 2.0 * uv - 1.0;
    // Stosunek wymiarów "fizycznego" okna, utowrzonego przez użytkownika.
    float aspectRatio = iResolution.x / iResolution.y;

    // Inicjalizacja kamery.
    Camera cam;
    cam.pos = vec3(-1.2, 0.5, 4.5);
    cam.lookat = vec3(-1.2, -0.3, 0.0);
    cam.up = vec3(0.0, 1.0, 0.0);
    cam.fov = C_PI / 2.0; // 90 stopni

    // Kierunek w jaki patrzy kamera.
    vec3 cameraDirection = normalize(cam.lookat - cam.pos);
    // Wektor mówiący, gdzie jest "prawo" naszego obszaru widocznego.
    vec3 viewportRight = cross(cameraDirection, cam.up);
    // Wektor mówiący, gdzie jest "góra" naszego obszaru widocznego.
    vec3 viewportUp = cross(viewportRight, cameraDirection);
    // Odległość kamery w lini prostej od naszego Viewportu, który
    // możemy sobie wyobrazić jako siatka pomiędzy kamerą a sceną.
    float cameraDistanceFromViewport = length(cam.lookat - cam.pos);

    // Szerokość viewportu obliczamy poprzez zauważenie, że tangensem połowy kąta
    // fov będzie stosunek połowy szerokości ekranu do odległości kamery od viewportu.
    float viewportWidth = 2.0 * cameraDistanceFromViewport * tan(cam.fov / 2.0);
    // Wysokość możemy obliczyć za pomocą wcześniej ustalonego aspectRatio.
    float viewportHeight = viewportWidth / aspectRatio;

    // Punkt we viewport'cie w jaki powinien lecieć promień z danego piksela.
    vec3 rayTarget = cameraDirection * cameraDistanceFromViewport +
            viewportWidth * viewportRight * uv.x +
            viewportHeight * viewportUp * uv.y;

    // Inicjalizacja promienia dla obecnie rozpatrywanego piksela.
    Ray ray;
    ray.origin = cam.pos;
    ray.dir = normalize(rayTarget - ray.origin);
    ray.inv_dir = 1.0 / ray.dir;
    ray.epsilon = 0.00001;

    vec3 c = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        c += GetColorForRay(ray, rngState);
    }
    c /= float(SAMPLES_PER_PIXEL);

    // vec3 lastFrameColor = texture(iChannel0, (uv+1.0)/2.0).rgb;
    vec3 lastFrameColor = texture(backBufferTexture, gl_FragCoord.xy / iResolution.xy).rgb;
    if (iFrame == 0) lastFrameColor = c;
    float weight = 1.0 / (float(iFrame) + 1.0);
    c = lastFrameColor * (1.0 - weight) + c * weight;

    gl_FragColor = vec4(c, 1.0);
}
