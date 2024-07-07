#version 330 core

uniform int iFrame;
uniform vec2 iResolution;
uniform sampler2D backBufferTexture;

const int MAX_BOUNCE_COUNT = 6;
const int SAMPLES_PER_PIXEL = 15;
const float DIVERGE_STRENGTH = 20.0;
const float EPSILON = 0.00001;
const float INFINITY = 1e30;
const float C_PI = 3.141592653589793;
const float C_TWOPI = 6.283185307179586;

struct Ray {
    vec3 origin;
    vec3 dir;
};

struct Material {
    vec3 emissionColor;
    float emissionStrength;
    vec3 albedo;
    float specularComponent;
};

struct HitInfo {
    bool didHit;
    vec3 hitPoint;
    vec3 normal;
    float dst;
    Material mat;
};

struct Sphere {
    vec3 pos;
    float r;
    Material mat;
};

struct Camera {
    vec3 pos;
    vec3 lookat;
    vec3 up;
    float fov;
};

const int NUM_OF_SPHERES = 9;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
        Sphere(vec3(0.0, -1002.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 0.1)), // floor
        Sphere(vec3(0.0, 0.0, -1002.2), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0), 0.1)), // back wall
        Sphere(vec3(0.0, 0.0, 1002.8), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 1.0), 0.1)), // front wall (behind camera)
        Sphere(vec3(-1003.5, 0.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 1.0, 0.0), 0.1)), // left wall
        Sphere(vec3(1002.5, 0.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 0.0, 1.0), 0.1)), // right wall
        Sphere(vec3(-2.2, -1.2, -0.5), 0.8, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 1.0)), // left mirror ball
        Sphere(vec3(-0.5, -1.2, -0.5), 0.8, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 1.0)), // right mirror ball
        Sphere(vec3(0.0, -1.49, 1.0), 0.5, Material(vec3(1.0, 1.0, 0.0), 0.4, vec3(1.0, 1.0, 1.0), 0.4)), // emitting ball
        Sphere(vec3(-1.0, 101.971, 1.0), 100.0, Material(vec3(1.0, 1.0, 0.96), 1.0, vec3(1.0, 0.0, 0.0), 0.0)) // ceiling emitter
    );

HitInfo RaySphereIntersection(Ray ray, Sphere s) {
    HitInfo hitInfo;
    hitInfo.didHit = false;
    vec3 offsetRay = ray.origin - s.pos;
    float a = dot(ray.dir, ray.dir);
    float b = 2.0 * dot(ray.dir, offsetRay);
    float c = dot(offsetRay, offsetRay) - s.r * s.r;

    float d = b * b - 4.0 * a * c;
    if (d >= 0.0) {
        hitInfo.dst = (-b - sqrt(d)) / (2.0 * a);

        if (hitInfo.dst > EPSILON) {
            hitInfo.didHit = true;
            hitInfo.hitPoint = ray.origin + hitInfo.dst * ray.dir;
            hitInfo.normal = normalize(hitInfo.hitPoint - s.pos);
            hitInfo.mat = s.mat;
        }
    }
    return hitInfo;
}

HitInfo CalculateRayCollsion(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.dst = INFINITY;

    for (int i = 0; i < NUM_OF_SPHERES; i++) {
        HitInfo hit = RaySphereIntersection(ray, SPHERES[i]);
        if (hit.didHit && hit.dst < closestHit.dst) {
            closestHit = hit;
            closestHit.mat = SPHERES[i].mat;
        }
    }
    return closestHit;
}

// RNG
uint pcg_hash(inout uint seed)
{
    seed = seed * 747796405u + 2891336453u;
    uint result = ((seed >> ((seed >> 28u) + 4u)) ^ seed) * 277803737u;
    result = (result >> 22) ^ result;
    return result;
}

float RandomFloat(inout uint state)
{
    return float(pcg_hash(state)) / 4294967296.0; // (2^32)
}

vec3 RandomUnitVector(inout uint state)
{
    float y = RandomFloat(state) * 2.0f - 1.0f;
    float a = RandomFloat(state) * C_TWOPI;
    float r = sqrt(1.0f - y * y);
    float x = r * cos(a);
    float z = r * sin(a);
    return vec3(x, y, z);
}

vec2 RandomPointInCircle(inout uint state) {
    float angle = RandomFloat(state) * C_TWOPI;
    vec2 pointOnCircle = vec2(cos(angle), sin(angle));
    return pointOnCircle * sqrt(RandomFloat(state));
}

vec3 DiffuseDirection(vec3 normal, inout uint rngState) {
    return normalize(normal + RandomUnitVector(rngState));
}

vec3 ReflectDirection(vec3 dir, vec3 normal) {
    float cosPhi1 = dot(-normal, dir);
    if (cosPhi1 < 0) cosPhi1 = dot(normal, dir);
    return dir + 2 * cosPhi1 * normal;
}

vec3 GetColorForRay(Ray ray, uint rngState) {
    vec3 c = vec3(1, 1, 1);
    vec3 incomingLight = vec3(0, 0, 0);
    for (int i = 0; i < MAX_BOUNCE_COUNT; i++) {
        HitInfo hit = CalculateRayCollsion(ray);
        if (hit.didHit) {
            ray.origin = hit.hitPoint;
            vec3 diffuseDir = DiffuseDirection(hit.normal, rngState);
            vec3 reflectDir = ReflectDirection(ray.dir, hit.normal);
            ray.dir = reflectDir * hit.mat.specularComponent + diffuseDir * (1 - hit.mat.specularComponent);

            vec3 emittedLight = hit.mat.emissionColor * hit.mat.emissionStrength;
            incomingLight += emittedLight * c;
            c *= hit.mat.albedo;
        } else {
            break;
        }
    }
    return incomingLight;
}

void main() {
    uint pixelIndex = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(iResolution.x);
    uint rngState = uint(pixelIndex + uint(iFrame * 719393));

    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    uv = uv * 2.0 - 1.0;
    float aspectRatio = iResolution.x / iResolution.y;

    Camera cam;
    cam.pos = vec3(-2.0, -0.8, 3.0);
    cam.lookat = vec3(-2.0, -1.0, -1.0);
    cam.up = vec3(0, 1, 0);
    cam.fov = C_PI / 2.0;

    vec3 cameraDirection = normalize(cam.lookat - cam.pos);

    vec3 viewportRight = vec3(1, 0, 0);
    vec3 viewportUp = vec3(0, 1, 0);
    float cameraDistanceFromViewport = length(cam.lookat - cam.pos);
    float viewportWidth = 2.0 * cameraDistanceFromViewport * tan(cam.fov / 2.0);
    float viewportHeight = viewportWidth / aspectRatio;
    vec3 rayTarget = cameraDirection * cameraDistanceFromViewport +
            viewportWidth * viewportRight * uv.x +
            viewportHeight * viewportUp * uv.y;

    vec3 totalIncomingLight = vec3(0, 0, 0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; i++) {
        Ray ray;
        ray.origin = cam.pos;
        vec2 jitter = RandomPointInCircle(rngState) * DIVERGE_STRENGTH / iResolution.x;
        vec3 jitteredRayTarget = rayTarget + viewportRight * jitter.x + viewportUp * jitter.y;
        ray.dir = normalize(jitteredRayTarget - ray.origin);
        totalIncomingLight += GetColorForRay(ray, rngState);
    }
    totalIncomingLight /= float(SAMPLES_PER_PIXEL);

    vec3 lastFrameColor = texture(backBufferTexture, gl_FragCoord.xy / iResolution.xy).rgb;
    if (iFrame == 0) lastFrameColor = totalIncomingLight;
    float weight = 1.0 / (float(iFrame) + 1.0);
    totalIncomingLight = lastFrameColor * (1.0 - weight) + totalIncomingLight * weight;

    gl_FragColor = vec4(totalIncomingLight, 1.0);
}
