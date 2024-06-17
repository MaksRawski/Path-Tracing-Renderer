// resources:
// - https://www.youtube.com/watch?v=Qz0KTGYJtUk
// - https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
// - https://www.realtimerendering.com/raytracing/Ray%20Tracing%20in%20a%20Weekend.pdf
// - https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing (FOV)
// - https://www.intel.com/content/www/us/en/content-details/763947/path-tracing-workshop-part-1-ray-tracing.html (camera setup idea)

#version 330 core

uniform int iFrame;
uniform vec2 iResolution;

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#positionablecamera (12.2)
struct Camera {
    // camera's position
    vec3 pos;
    // what's camera pointed at
    vec3 lookat;
    // camera's relative up direction, shouldn't be confused with the viewport's relative up direction
    vec3 up;
    // horizontal field of view in radians
    float fov;
};

struct Ray {
    vec3 origin;
    vec3 dir;
    // used to restrict the interval at which intersections are actually useful
    // basically if something is closer than epsilon to our ray or closer than
    // epsilon to light source then we consider it to not be a useful intersection?
    // and we might as well just ignore that ray then?
    float epsilon;
};

struct Material {
    // what color it emits
    vec3 emissionColor;
    // how much light it emits
    float emissionStrength;
    // what color it is under white light
    vec3 albedo;
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
const int MAX_BOUNCE_COUNT = 2;
const int SAMPLES_PER_PIXEL = 4;

const int NUM_OF_SPHERES = 5;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
        Sphere(vec3(25.0, 0.0, -20.0), 6.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0))), // white ball
        Sphere(vec3(20.0, -1.4, -10.0), 4.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0))), // red ball
        Sphere(vec3(18.0, -2.4, -4.0), 3.5, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 1.0, 0.0))), // green ball
        Sphere(vec3(16.0, -2.4, 4.0), 3.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 0.0, 1.0))), // blue ball
        // Sphere(vec3(-5.0, 21.0, 9.0), 16.0, Material(vec3(0.8, 0.8, 0.8), 50.0, vec3(0.8, 0.8, 0.0))), // sun
        Sphere(vec3(-5.0, -1005.0, 10.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.5, 0.5, 0.5))) // ground
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

// we hit the sphere if ||ray.origin + ray.dir * distance||² = r²
// o := ray.origin, d := ray.dir, s := distance
// ||o + d * s||² = r²
// sqrt((o_1 + s * d_1)² + (o_2 + s * d_2)² + (o_3 + s * d_3)²)² = r²
// (o_1 + s * d_1)² + (o_2 + s * d_2)² + (o_3 + s * d_3)² = r²
// (o_1² + 2*s*o_1*d_1 + s²d_1²) + (o_2² + 2*s*o_2*d_2 + s²d_2²) + (o_3² + 2*s*o_3*d_3 + s²d_3²) = r²
// s²(d_1² + d_2² + d_3²) + 2s(o_1*d_1 + o_2*d_2 + o_3*d_3) + (o_1²+o_2²+0_3²) = r²
// s² * dot(d, d) + 2s * dot(d, o), + dot(o,o) = r²
// quadratic equation for s
HitInfo RayHitsSphere(Ray ray, Sphere s) {
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
        hitInfo.dst = min(d1, d2); // TODO: d1 will always be smaller?

        if (hitInfo.dst > ray.epsilon) {
            hitInfo.didHit = true;
            hitInfo.hitPoint = ray.origin + hitInfo.dst * ray.dir;
            hitInfo.normal = normalize(hitInfo.hitPoint - s.pos);
            hitInfo.mat = s.mat;
        }
    }

    return hitInfo;
}

// returns info about the nearest point which the ray hits
HitInfo CalculateRayCollision(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.dst = 1.0 / 0.0; // infinity

    // iterate through all the spheres
    for (int i = 0; i < NUM_OF_SPHERES; ++i) {
        HitInfo hit = RayHitsSphere(ray, SPHERES[i]);
        if (hit.didHit && hit.dst < closestHit.dst) {
            closestHit = hit;
            closestHit.mat = SPHERES[i].mat;
        }
    }

    return closestHit;
}

// random direction in the same hemisphere the normal vector is in
// for diffuse reflection
vec3 RandomHemisphereDirection(vec3 normal, inout uint rngState) {
    // dot product is negative if the vectors are more than 90 degrees apart
    vec3 randomVector = RandomUnitVector(rngState);
    return sign(dot(normal, randomVector)) * randomVector;
}

// copy pasted from sebastian's video
vec3 GetEnviromentLight(Ray ray) {
    // vec3 SunLightDirection = vec3(0.0, -5.0, 0.0);
    float SunFocus = 0.1;
    float SunIntensivity = 0.5;
    vec3 SkyColorHorizon = vec3(0.8, 0.8, 0.8);
    vec3 SkyColorZenith = vec3(0.3, 0.6, 0.8);
    vec3 GroundColor = vec3(0.7, 0.7, 0.7);
    vec3 SunLightDirection = vec3(0.0, -2.0, 0.0);

    float skyGradientT = pow(smoothstep(0, 0.4, ray.dir.y), 0.35);
    vec3 skyGradient = mix(SkyColorHorizon, SkyColorZenith, skyGradientT);
    float sun = pow(max(0, dot(ray.dir, -SunLightDirection)), SunFocus) * SunIntensivity;

    // combine ground sky and sun
    float groundToSkyT = smoothstep(-0.01, 0, ray.dir.y);
    float sunMask = groundToSkyT >= 1 ? 1.0 : 0.0;
    return mix(GroundColor, skyGradient, groundToSkyT) + sun * sunMask;
}

vec3 GetColorForRay(Ray ray, inout uint rngState) {
    // we start with pure white
    // as we (ig?) assume that the potential light source emits just that
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 incomingLight = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i <= MAX_BOUNCE_COUNT; ++i) {
        HitInfo hitInfo = CalculateRayCollision(ray);
        if (hitInfo.didHit) {
            ray.origin = hitInfo.hitPoint;
            // This is because of Lambert's cosine law?
            ray.dir = normalize(hitInfo.normal + RandomUnitVector(rngState));

            // ray.dir = RandomHemisphereDirection(hitInfo.normal, rngState);
            // calculate the potential light that the object is emitting
            vec3 emittedLight = hitInfo.mat.emissionColor * hitInfo.mat.emissionStrength;
            // tint the color of the incoming light by that color
            incomingLight += emittedLight * c;
            c *= hitInfo.mat.albedo;
        } else {
            // we could sample enviroment here
            incomingLight += GetEnviromentLight(ray) * c;
            break;
        }
    }
    return incomingLight;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    // init seed
    uint rngState = uint(uint(fragCoord.x) * uint(1973) + uint(fragCoord.y) * uint(9277) + uint(iFrame) * uint(26699)) | uint(1);

    // fragCoord stores the pixel coordinates [0.5, resolution-0.5]
    vec2 uv = fragCoord.xy / iResolution.xy; // normalized coordinates [0, 1]
    uv = 2.0 * uv - 1.0; // normalized coordinates [-1, 1]
    float aspectRatio = iResolution.x / iResolution.y;

    // camera
    Camera cam;
    cam.pos = vec3(-5.0, 1.0, 0.0);
    cam.lookat = vec3(0.0, 1.0, 0.0);
    cam.up = vec3(0.0, 1.0, 0.0);
    cam.fov = C_PI / 2.0; // 90 degrees
    vec3 cameraDirection = normalize(cam.lookat - cam.pos);
    vec3 viewportRight = cross(cameraDirection, cam.up); // cross calculates the vector perpendicular to both its arguments
    vec3 viewportUp = cross(viewportRight, cameraDirection); // use the right-hand rule to see why it makes sense :)
    float cameraDistanceFromViewport = length(cam.lookat - cam.pos); // AKA focal length

    float viewportWidth = 2.0 * cameraDistanceFromViewport * tan(cam.fov / 2.0);
    float viewportHeight = viewportWidth / aspectRatio;
    // float viewportHeight = 2 * cameraDistanceFromViewport * tan(cam.fov / 2);
    // float viewportWidth = viewportHeight * aspectRatio;

    vec3 rayTarget = cameraDirection * cameraDistanceFromViewport +
            viewportWidth * viewportRight * uv.x +
            viewportHeight * viewportUp * uv.y;

    // rays
    // imagine that the camera is just a point that shoots rays
    // but is behind the viewport and each ray has to travel through a different pixel
    Ray ray;
    ray.origin = cam.pos;
    ray.dir = normalize(rayTarget - ray.origin);
    ray.epsilon = 0.001;

    // shooting rays
    // HitInfo hitInfo = CalculateRayCollision(ray);
    // vec4 c = hitInfo.didHit ? vec4(hitInfo.mat.albedo, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
    vec3 c = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        c += GetColorForRay(ray, rngState);
    }
    c /= float(SAMPLES_PER_PIXEL);

    // vec3 lastFrameColor = texture(iChannel0, fragCoord / iResolution.xy).rgb;
    // if (iFrame == 0) lastFrameColor = c;
    // c = mix(lastFrameColor, c, 1.0 / float(iFrame + 1));

    // fragColor = vec4(uv, 0, 1);
    fragColor = vec4(c, 1);
    // fragColor = vec4(lastFrameColor, 1.0);
}

// mainImage is what shadertoy requires
// but this is how you should actually write it
void main() {
    vec4 fragColor;
    mainImage(fragColor, gl_FragCoord.xy);
    gl_FragColor = fragColor;
}
