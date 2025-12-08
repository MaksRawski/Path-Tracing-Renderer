// overall resources:
// - https://www.youtube.com/watch?v=Qz0KTGYJtUk
// - https://github.com/ssloy/tinyraytracer/wiki/Part-1:-understandable-raytracing (FOV, reflection stuff)

#version 460

uniform int iFrame;
uniform vec2 iResolution;
uniform sampler2D backBufferTexture;
uniform float aspectRatioWdH;

uniform int MAX_BOUNCE_COUNT = 5;
uniform int SAMPLES_PER_PIXEL = 2;
uniform float DIVERGE_STRENGTH = 0.001;

const float EPSILON = 0.00001;
const float INFINITY = 1.0e30;
const float C_PI = 3.141592653589793;
const float C_TWOPI = 6.283185307179586;

out vec4 FragColor;

// #define CULLING

#define STACK_SIZE 40
#define MAX_ITERATIONS 200

// https://raytracing.github.io/books/RayTracingInOneWeekend.html#positionablecamera (12.2)
struct Camera {
    vec4 pos;
    vec4 dir;
    vec4 up;
    // horizontal field of view in radians
    float fov;
    float focal_length;
};

struct Ray {
    vec3 origin;
    vec3 dir;
    vec3 inv_dir; // 1 / dir
};

// NOTE: these are just `vec3`s but because they come from a buffer-backed
// blocks which have layouts that pad them to 16 bytes turning `vec3`s in to `vec4`s
struct Triangle {
    vec4 a, b, c;
    vec4 _na, _nb, _nc;
};

// NOTE: same as above
struct BVHnode {
    vec4 boundsMin, boundsMax;
    uint first, count;
    int _, _1; // padding so that this struct's size is a multiple of 16 bytes
};

struct Material {
    // what color it emits
    vec3 emissionColor;
    // how much light it emits, in range [0; 1]
    float emissionStrength;
    // what color it is under white light
    vec3 albedo;
    // how reflective a surface is, in range [0; 1]
    // when 0, diffuse
    // when 1, reflect
    float specularComponent;
};

struct Primitive {
    // index of the material in materialsBuffer
    uint mat;
};

layout(std430, binding = 1) readonly buffer trianglesBuffer {
    Triangle triangles[];
};
layout(std430, binding = 2) readonly buffer bvhNodesBuffer {
    BVHnode nodes[];
};
layout(std430, binding = 3) readonly buffer materialsBuffer {
    Material mats[];
};
layout(std430, binding = 4) readonly buffer primitivesBuffer {
    Primitive primitives[];
};
layout(std430, binding = 5) readonly buffer cameraBuffer {
    Camera camera;
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
    return float(pcg_hash(state)) / 4294967296.0;
}

vec3 RandomUnitVector(inout uint state)
{
    float z = RandomFloat(state) * 2.0f - 1.0f;
    float a = RandomFloat(state) * C_TWOPI;
    float r = sqrt(1.0f - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

// https://stackoverflow.com/a/6178290
float RandomFloatNormalDistribution(inout uint state)
{
    float theta = C_TWOPI * RandomFloat(state);
    float rho = sqrt(-2 * log(RandomFloat(state)));
    return rho * cos(theta);
}

vec2 RandomPointInCircle(inout uint state)
{
    float angle = RandomFloat(state) * C_TWOPI;
    vec2 pointOnCircle = vec2(cos(angle), sin(angle));
    return pointOnCircle * sqrt(RandomFloat(state));
}
// using Möller-Trumbore intersection algorithm
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
// det = E1 . (D x E2)
// and that's it!
HitInfo RayTriangleIntersection(Ray ray, Triangle tri) {
    HitInfo hitInfo;
    hitInfo.didHit = false;
    vec3 D = ray.dir;
    vec3 e1 = (tri.b - tri.a).xyz;
    vec3 e2 = (tri.c - tri.a).xyz;
    vec3 De2 = cross(D, e2);
    float det = dot(e1, De2);

    #ifdef CULLING
    // if the determinant is negative, then the direction from which ray hits
    // the triangle is opposite of the triangle's normal
    bool condition = det < -EPSILON;
    #else
    // if determinant is zero then the ray is coming in parallel to the triangle
    bool condition = abs(det) < EPSILON;
    #endif

    if (condition) {
        // the vectors -D, e1 and e2 are not linearly independent,
        // meaning ray is coming in parallel to the triangle
        return hitInfo;
    }

    float inv_det = 1.0 / det;

    vec3 T = ray.origin - tri.a.xyz;
    vec3 Te1 = cross(T, e1);

    float u = dot(T, De2) * inv_det;

    // the barycentric coordinate u is too far, for the point of intersection
    // to be inside the triangle
    if ((u < 0 && abs(u) > EPSILON) || (u > 1 && abs(u - 1.0) > EPSILON)) return hitInfo;

    float v = dot(D, Te1) * inv_det;

    // the barycentric coordinate v is too far, for the point of intersection
    // to be inside the triangle or both u and v are too far from centre
    if ((v < 0 && abs(v) > EPSILON) || (u + v > 1 && abs(u + v - 1.0) > EPSILON)) return hitInfo;

    // finally, if we got here, it means that we did actually hit the triangle
    float t = dot(Te1, e2) * inv_det;
    if (t > EPSILON) {
        hitInfo.didHit = true;
        hitInfo.hitPoint = ray.origin + ray.dir * t;
        if (det > EPSILON)
            hitInfo.normal = normalize(cross(e1, e2));
        else
            hitInfo.normal = normalize(cross(e2, e1));
        hitInfo.dst = t;
    }

    return hitInfo;
}

const int NUM_OF_SPHERES = 5;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
        Sphere(vec3(4.0, -2, -5.0), 1.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0), 0.0)), // red
        Sphere(vec3(3.0, -1, -2.0), 1.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 1.0, 0.0), 0.0)), // green
        Sphere(vec3(2.0, -0, 0.0), 1.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 0.0, 1.0), 0.0)), // blue
        Sphere(vec3(0.4, -0.3, -1.0), 0.5, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 1.0)), // mirror
        Sphere(vec3(3.0, 3, 1.0), 1.0, Material(vec3(1.0, 1.0, 0.96), 1.0, vec3(1.0, 0.0, 0.0), 0.0)) // emitter
    );

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

    // this is 1 only if the vector is normalized, so it's safer to just calculate it
    float a = dot(ray.dir, ray.dir);
    float b = 2.0 * dot(ray.dir, offsetRay);
    float c = dot(offsetRay, offsetRay) - s.r * s.r;

    float d = b * b - 4.0 * a * c;
    if (d >= EPSILON) {
        float dst1 = (-b - sqrt(d)) / (2.0 * a);
        float dst2 = (-b + sqrt(d)) / (2.0 * a);
        if (dst1 < EPSILON && dst2 < EPSILON) return hitInfo;
        bool outside = dst1 >= EPSILON;
        // hitInfo.dst = dst1;
        hitInfo.dst = outside ? dst1 : dst2;

        if (hitInfo.dst > EPSILON) {
            hitInfo.didHit = true;
            hitInfo.hitPoint = ray.origin + hitInfo.dst * ray.dir;
            // hitInfo.normal = normalize(hitInfo.hitPoint - s.pos);
            hitInfo.normal = (outside ? 1.0 : -1.0) * normalize(hitInfo.hitPoint - s.pos);
            hitInfo.mat = s.mat;
        }
    }

    return hitInfo;
}

// using slab method
// Imagine that we put two parallel planes per each axis, such that
// distance between those planes will be that of the bounding box.
// We look through each of those planes and calculate the distance to
// intersection with both planes on every axis individually.
// To calculate whether a ray hits a plane and if so what's the distance
// we have to consider each plane to be defined in terms of any point on it
// and a normal vector to the plane, so we get this as an equation for a plane:
// P . N + d = 0, where
// P is any point on that plane,
// N is the normal vector of the plane,
// d is the plane's offset from origin.
//
// Let's assume that P is the point of intersection of the ray with the plane.
// The ray equation is P = O + t * D, where
// P is the destination point,
// O is the ray's origin point,
// t is the distance travelled by the ray to P,
// D is the ray's direction.
// We can transform the ray equation to instead be:
// t = (P - O) / D,
// which tells us that if we know the point of intersection we can easily
// calculate the distance to that point.
// if there is no intersection we will end up with infinities which
// funnily enough get automagically handled
bool RayBVHnodeIntersection(Ray ray, BVHnode bb) {
    float tx1 = (bb.boundsMin.x - ray.origin.x) * ray.inv_dir.x, tx2 = (bb.boundsMax.x - ray.origin.x) * ray.inv_dir.x;
    float tmin = min(tx1, tx2), tmax = max(tx1, tx2);
    float ty1 = (bb.boundsMin.y - ray.origin.y) * ray.inv_dir.y, ty2 = (bb.boundsMax.y - ray.origin.y) * ray.inv_dir.y;
    tmin = max(tmin, min(ty1, ty2)), tmax = min(tmax, max(ty1, ty2));
    float tz1 = (bb.boundsMin.z - ray.origin.z) * ray.inv_dir.z, tz2 = (bb.boundsMax.z - ray.origin.z) * ray.inv_dir.z;
    tmin = max(tmin, min(tz1, tz2)), tmax = min(tmax, max(tz1, tz2));
    return tmax >= tmin && tmax > 0;
}

// TODO: this is just for debugging
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

// returns info about the nearest point which the ray hits
HitInfo CalculateRayCollision(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    // TODO: with finite camera projection this will be zfar
    closestHit.dst = INFINITY;

    uint stack[STACK_SIZE], stack_ptr = 0;
    stack[stack_ptr++] = 0;
    int max_iterations = MAX_ITERATIONS;
    while (stack_ptr > 0 && max_iterations-- > 0) {
        BVHnode node = nodes[stack[--stack_ptr]];

        if (!RayBVHnodeIntersection(ray, node))
        {
            if (stack_ptr == 0) return closestHit;
            else continue;
        }
        // if node is a leaf
        if (node.count > 0) {
            for (int i = 0; i < node.count; ++i) {
                uint t_index = node.first + i;
                Triangle t = triangles[t_index];
                HitInfo hit = RayTriangleIntersection(ray, t);
                if (hit.didHit && hit.dst < closestHit.dst) {
                    closestHit = hit;
                    closestHit.mat = mats[primitives[t_index].mat];
                    // closestHit.mat = Material(vec3(0.0, 0.0, 0.0), 0.0, hsv2rgb(vec3(float(stack_ptr) / float(bvhNodeCount), 0.5, 0.5)), 0.0);
                }
            }
        } else {
            // left will be checked first, so must push the right one first
            stack[stack_ptr++] = node.first + 1;
            stack[stack_ptr++] = node.first + 0;
        }
    }

    // iterate through all triangles
    // for (int i = 0; i < 10; ++i) {
    //     // BVHnode node = getNode(i);
    //     Triangle t = triangles[i];
    //     HitInfo hit = RayTriangleIntersection(ray, t);
    //     if (hit.didHit && hit.dst < closestHit.dst) {
    //         closestHit = hit;
    //         closestHit.mat = Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0), 0.0);
    //     }
    // }

    // iterate through all the spheres
    // for (int i = 0; i < NUM_OF_SPHERES; ++i) {
    //     HitInfo hit = RaySphereIntersection(ray, SPHERES[i]);
    //     if (hit.didHit && hit.dst < closestHit.dst) {
    //         closestHit = hit;
    //     }
    // }

    return closestHit;
}

vec3 DiffuseDirection(vec3 normal, inout uint rngState) {
    return normalize(normal + RandomUnitVector(rngState));
}

vec3 ReflectDirection(vec3 dir, vec3 normal) {
    return dir - 2 * dot(dir, normal) * normal;
}

vec3 GetColorForRay(Ray ray, inout uint rngState) {
    // we start with pure white
    // as we (ig?) assume that the potential light source emits just that
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 incomingLight = vec3(0.0, 0.0, 0.0);
    HitInfo hitInfo = CalculateRayCollision(ray);
    // return hitInfo.didHit ? hitInfo.mat.albedo : vec3(0.0, 0.0, 0.0);

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
            // ray.dir = RandomHemisphereDirection(hitInfo.normal, rngState);
            vec3 diffuseDir = DiffuseDirection(hitInfo.normal, rngState);
            vec3 reflectDir = ReflectDirection(ray.dir, hitInfo.normal);

            ray.dir = mix(diffuseDir, reflectDir, hitInfo.mat.specularComponent);
            ray.inv_dir = 1.0 / ray.dir;

            // calculate the potential light that the object is emitting
            vec3 emittedLight = hitInfo.mat.emissionColor * hitInfo.mat.emissionStrength;
            // tint the color of the incoming light by that color
            incomingLight += emittedLight * c;
            c *= hitInfo.mat.albedo;
        } else {
            // get color from environment
            // incomingLight += vec3(58, 58, 58) / 255 * c;
            break;
        }
    }
    return incomingLight;
}

void main() {
    uint pixelIndex = uint(gl_FragCoord.x) + uint(gl_FragCoord.y) * uint(iResolution.x);
    uint rngState = uint(pixelIndex + uint(iFrame * 719393));

    // gl_FragCoord stores the pixel coordinates [0.5, resolution-0.5]
    vec2 uv = gl_FragCoord.xy / iResolution.xy; // normalized coordinates [0, 1]
    uv = 2.0 * uv - 1.0; // normalized coordinates [-1, 1]
    float aspectRatio = iResolution.x / iResolution.y;
    // float aspectRatio = aspectRatioWdH;

    vec3 viewportRight = cross(camera.dir.xyz, camera.up.xyz);
    vec3 viewportUp = cross(viewportRight, camera.dir.xyz);
    float cameraDistanceFromViewport = camera.focal_length;

    float viewportWidth = 2.0 * cameraDistanceFromViewport * tan(camera.fov / 2.0);
    float viewportHeight = viewportWidth / aspectRatio;

    vec3 rayTarget = camera.dir.xyz * cameraDistanceFromViewport +
            viewportWidth * viewportRight * uv.x +
            viewportHeight * viewportUp * uv.y;

    vec3 totalIncomingLight = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        Ray ray;
        ray.origin = camera.pos.xyz;
        vec2 jitter = RandomPointInCircle(rngState) * DIVERGE_STRENGTH;
        vec3 jitteredRayTarget = rayTarget + viewportRight * jitter.x + viewportUp * jitter.y;

        ray.dir = normalize(jitteredRayTarget - ray.origin);
        ray.inv_dir = 1.0 / ray.dir;
        totalIncomingLight += GetColorForRay(ray, rngState);
    }
    totalIncomingLight /= float(SAMPLES_PER_PIXEL);

    vec3 lastFrameColor = texture(backBufferTexture, gl_FragCoord.xy / iResolution.xy).rgb;
    if (iFrame == 0) lastFrameColor = totalIncomingLight;
    float weight = 1.0 / (float(iFrame) + 1.0);
    totalIncomingLight = lastFrameColor * (1.0 - weight) + totalIncomingLight * weight;

    FragColor = vec4(totalIncomingLight, 1.0);
}
