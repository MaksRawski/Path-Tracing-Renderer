// overall resources:
// - https://www.youtube.com/watch?v=Qz0KTGYJtUk
// - https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing (FOV, reflection stuff)

#version 330 core

uniform int iFrame;
uniform vec2 iResolution;
uniform sampler2D iChannel0;

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#positionablecamera (12.2)
uniform vec3 camPos; // camera's position
uniform vec3 camLookat; // what's camera pointed at
uniform vec3 camUp; // camera's relative up direction
uniform float camFov; // horizontal field of view in radians

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

struct Triangle {
    vec3 a, b, c;
};

struct MeshInfo {
    // index of the first triangle in the StructuredBuffer triangle list
    uint firstTriangleIndex;
    uint numTriangles;
    // bounding box
    vec3 boundsMin;
    vec3 boundsMax;
    Material material;
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
const int MAX_BOUNCE_COUNT = 4;
const int SAMPLES_PER_PIXEL = 4;

const int NUM_OF_SPHERES = 6;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
        Sphere(vec3(25.0, 0.0, -20.0), 6.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0))), // white ball
        Sphere(vec3(20.0, -1.4, -10.0), 4.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0))), // red ball
        Sphere(vec3(18.0, -2.4, -4.0), 3.5, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 1.0, 0.0))), // green ball
        Sphere(vec3(16.0, -2.4, 4.0), 3.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 0.0, 1.0))), // blue ball
        Sphere(vec3(65.0, 21.0, 9.0), 16.0, Material(vec3(0.8, 0.8, 0.8), 15.0, vec3(0.8, 0.8, 0.0))), // sun
        Sphere(vec3(-5.0, -1005.0, 10.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.5, 0.5, 0.5))) // ground
    );

const int NUM_OF_TRIANGLES = 1;
const float OFX = -2.5;
const float OFY = 2.5;
const float OFZ = 32.0;
const Triangle TRIANGLES[NUM_OF_TRIANGLES] = Triangle[NUM_OF_TRIANGLES](
        Triangle(vec3(OFX+20.0, 0.0, -20.0+OFZ), vec3(OFX+10, 0.0, -20.0+OFZ), vec3(OFX+17.5, 5.0, -18.0+OFZ))
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

// https://www.youtube.com/watch?v=fK1RPmF_zjQ
// https://cadxfem.org/inf/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf
// Point T(u,v) = (1-u-v)A + uB + vC, where (u, v) are barycentric coordinates
// and A, B, C are positions of vertices of the triangle,
// is inside a triangle if u >= 0, v >= 0 and u + v <= 1.
// u and v can be thought of as closeness to either vertex of a triangle.
// A ray has an equation R(t) = O + tD, where O is origin,
// D is direction and t is distance traveled in that direction.
// If we assume that a ray hits the triangle we can then equate both formulas:
// R(t) = T(u,v)
// O + tD = (1-u-v)A + uB + vC (only t, u and v are unknowns)
//
// Rearanging gives:
// A - uA -vA + uB + vC - tD = O
// - tD + u(B - A) + v(C - A) = O - A
// [-D, B-A, C-A]x[t, u, v] = O - A
// renaming: E1 := B - A, E2 := C - A, T := O - A, gives:
// [-D, E1, E2]x[t, u, v] = T
//
// We can apply cramer's rule, which in case of:
// [a1 b1 c1][x] = [d1]
// [a2 b2 c2][y] = [d2]
// [a3 b3 c3][z] = [d3],
// allows us to find that:
//     | d1 b1 c1 |   | a1 b1 c1 |
// x = | d2 b2 c2 | / | a2 b2 c2 |
//     | d3 b3 c3 |   | a3 b3 c3 |
//
//     | a1 d1 c1 |   | a1 b1 c1 |
// y = | a2 d2 c2 | / | a2 b2 c2 |
//     | a3 d3 c3 |   | a3 b3 c3 |
//
//     | a1 b1 d1 |   | a1 b1 c1 |
// z = | a2 b2 d2 | / | a2 b2 c2 |
//     | a3 b3 d3 |   | a3 b3 c3 |.
//
// Applying this to our situation, we get:
// t = | T E1 E2 | / | -D E1 E2 |
// u = | -D T E2 | / | -D E1 E2 |
// v = | -D E1 T | / | -D E1 E2 |.
// Keep in mind that those equations for t, u and v above are actually
// on 3x3 matrix as each element is actually a vec3.
// We can think of a value of a 3x3 determinant as a signed area of a parallelogram
// defined by 3 vectors. The area of a (vector described) parallelogram is a . (b x c),
// where b x c is a cross product of the 2 base vectors, which results in a vertical
// vector with the magnitude of the area of the base and dot product simply multiplies
// the height with the area of a base (which is a vector with the same direction as a)
// and as they have the same direction and therefore the angle between them is zero,
// just their magnitudes get multiplied and we get a scalar.
//
// So the first determinants for each of the unknowns can be found as:
// t = T . (E1 x E2)
// u = -D . (T x E2)
// v = -D . (E1 x T).
// As "the scalar triple product is unchanged under a circular shift" (https://en.wikipedia.org/wiki/Triple_product)
// we can change the equation for t to instead be:
// t = (T x E1) . E2
// We can also use the fact that "Swapping any two of the three operands negates the triple product", and get:
// v = D . (T x E1)
//
// For the determinant common among the three equations, we will have:
// det = -D . (E1 x E2)
// and that's it!
HitInfo RayTriangleIntersection(Ray ray, Triangle tri) {
    HitInfo hitInfo;
    hitInfo.didHit = false;
    vec3 D = ray.dir;
    vec3 e1 = tri.b - tri.a;
    vec3 e2 = tri.c - tri.a;
    vec3 e1e2 = cross(e1, e2);
    float det = dot(-D, e1e2);

    if (abs(det) < ray.epsilon){
       // the vectors -D, e1 and e2 are not linearly independent,
       // meaning ray is coming in parallel to the triangle
        return hitInfo;
    }

    float inv_det = 1.0/det;

    vec3 T = ray.origin - tri.a;
    vec3 Te2 = cross(T, e2);
    vec3 Te1 = cross(T, e1);

    float u = dot(-D, Te2) * inv_det;

    // the barycentric coordinate u is too far, for the point of intersection
    // to be inside the triangle
    if (u < 0 || u > 1) return hitInfo;

    float v = dot(D, Te1)  * inv_det;

    // the barycentric coordinate v is too far, for the point of intersection
    // to be inside the triangle or both u and v are too far from centre
    if (v < 0 || u + v > 1) return hitInfo;

    // finally, if we got here, it means that we did actually hit the triangle
    float t = dot(Te1, e2) * inv_det;
    hitInfo.didHit = true;
    hitInfo.hitPoint = ray.origin + ray.dir * t;
    // look at the right beginning of the description of this function
    float w = 1 - u - v;
    hitInfo.normal = normalize(tri.a * w + tri.b * u + tri.c * v);
    hitInfo.dst = t;

    return hitInfo;
}

// returns info about the nearest point which the ray hits
HitInfo CalculateRayCollision(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.dst = 1.0e30; // _infinity_

    // iterate through all triangles
    for (int i = 0; i < NUM_OF_TRIANGLES; ++i) {
        HitInfo hit = RayTriangleIntersection(ray, TRIANGLES[i]);
        if (hit.didHit && hit.dst < closestHit.dst) {
            closestHit = hit;
            closestHit.mat = Material(vec3(0.0, 1.0, 1.0), 1.0, vec3(1.0 / 64.0, 1.0 / 224.0, 1.0 / 208.0));
        }
    }

    // iterate through all the spheres
    for (int i = 0; i < NUM_OF_SPHERES; ++i) {
        HitInfo hit = RaySphereIntersection(ray, SPHERES[i]);
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

    float skyGradientT = pow(smoothstep(0.0, 0.4, ray.dir.y), 0.35);
    vec3 skyGradient = mix(SkyColorHorizon, SkyColorZenith, skyGradientT);
    float sun = pow(max(0.0, dot(ray.dir, -SunLightDirection)), SunFocus) * SunIntensivity;

    // combine ground sky and sun
    float groundToSkyT = smoothstep(-0.01, 0.0, ray.dir.y);
    float sunMask = groundToSkyT >= 1.0 ? 1.0 : 0.0;
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
            // bounce
            ray.origin = hitInfo.hitPoint;
            // light strength depends on the angle between the normal vector of the surface
            // and the light direction, the smaller the angle, the stronger the light
            // we can use a cosine weighted distribution to achieve that
            // we attach a random vector at the end of the normal vector and then normalize
            // the result to get any point at a hemisphere
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
    // Camera cam;
    // cam.pos = vec3(-5.0, 1.0, 0.0);
    // cam.lookat = vec3(0.0, 1.0, 0.0);
    // cam.up = vec3(0.0, 1.0, 0.0);
    // cam.fov = C_PI / 2.0; // 90 degrees
    vec3 cameraDirection = normalize(camLookat - camPos);
    vec3 viewportRight = cross(cameraDirection, camUp); // cross calculates the vector perpendicular to both its arguments
    vec3 viewportUp = cross(viewportRight, cameraDirection); // use the right-hand rule to see why it makes sense :)
    float cameraDistanceFromViewport = length(camLookat - camPos); // AKA focal length

    float viewportWidth = 2.0 * cameraDistanceFromViewport * tan(camFov / 2.0);
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
    ray.origin = camPos;
    ray.dir = normalize(rayTarget - ray.origin);
    ray.epsilon = 0.00001;

    // shooting rays
    // HitInfo hitInfo = CalculateRayCollision(ray);
    // vec4 c = hitInfo.didHit ? vec4(hitInfo.mat.albedo, 1.0) : vec4(0.0, 0.0, 0.0, 1.0);
    vec3 c = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        c += GetColorForRay(ray, rngState);
    }
    c /= float(SAMPLES_PER_PIXEL);

    vec3 lastFrameColor = texture(iChannel0, fragCoord / iResolution.xy).rgb;
    if (iFrame == 0) lastFrameColor = c;
    c = mix(c, lastFrameColor, 1.0 / float(iFrame + 1));
    float weight = 1.0 / (float(iFrame) + 1.0);
    c = lastFrameColor * (1.0 - weight) + c * weight;
    // c = lastFrameColor * vec3(1.0, 0.0, 0.0) + vec3(0.1, 0.0, 0.0);

    // fragColor = vec4(uv, 0, 1);
    fragColor = vec4(c, 1);
    // fragColor = vec4(lastFrameColor, 1.0);
}

// mainImage is what shadertoy requires
// but this is how shaders actually work
void main() {
    vec4 fragColor;
    mainImage(fragColor, gl_FragCoord.xy);
    gl_FragColor = fragColor;
}
