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

struct Triangle {
    vec3 a, b, c;
    vec3 na, nb, nc;
};

Triangle getTriangle(int i) {
    vec3 a = texelFetch(trianglesBuffer, 6 * i).rgb;
    vec3 b = texelFetch(trianglesBuffer, 6 * i + 1).rgb;
    vec3 c = texelFetch(trianglesBuffer, 6 * i + 2).rgb;
    vec3 na = texelFetch(trianglesBuffer, 6 * i + 3).rgb;
    vec3 nb = texelFetch(trianglesBuffer, 6 * i + 4).rgb;
    vec3 nc = texelFetch(trianglesBuffer, 6 * i + 5).rgb;
    return Triangle(a, b, c, na, nb, nc);
}

struct MeshInfo {
    // index of the first triangle in trianglesBuffer
    int firstTriangleIndex;
    int numTriangles;
    // index of the material in materialsBuffer
    int materialIndex;
    // bounding box
    vec3 boundsMin;
    vec3 boundsMax;
};

MeshInfo getMesh(int i) {
    MeshInfo mi;
    vec3 t = texelFetch(meshesInfoBuffer, 9 * i).rgb;
    mi.firstTriangleIndex = int(t.r);
    mi.numTriangles = int(t.g);
    mi.materialIndex = int(t.b);
    mi.boundsMin = texelFetch(meshesInfoBuffer, 9 * i + 1).rgb;
    mi.boundsMax = texelFetch(meshesInfoBuffer, 9 * i + 2).rgb;
    return mi;
}

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

Material getMaterial(int i) {
    Material mat;
    vec4 emission = texelFetch(materialsBuffer, 2 * i).rgba;
    vec4 rest = texelFetch(materialsBuffer, 2 * i + 1).rgba;
    mat.emissionColor = emission.rgb;
    mat.emissionStrength = emission.a;
    mat.albedo = rest.rgb;
    mat.specularComponent = rest.a;
    return mat;
}

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
const int MAX_BOUNCE_COUNT = 4;
const int SAMPLES_PER_PIXEL = 2;

const int NUM_OF_SPHERES = 8;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
        Sphere(vec3(0.0, -1002.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 0.1)), // ground
        Sphere(vec3(0.0, 0.0, -1001.8), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 0.0), 0.1)), // back
        Sphere(vec3(0.0, 0.0, 1002.8), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 0.0, 1.0), 0.1)), // front
        Sphere(vec3(-1003.5, 0.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 1.0, 0.0), 0.1)), // left
        Sphere(vec3(1002.5, 0.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(0.0, 0.0, 1.0), 0.1)), // right
        Sphere(vec3(0.0, 1003.0, 0.0), 1000.0, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 0.1)), // ceiling
        Sphere(vec3(-1.6, -0.4, -0.5), 0.8, Material(vec3(0.0, 0.0, 0.0), 0.0, vec3(1.0, 1.0, 1.0), 1)), // mirror
        Sphere(vec3(-1.0, 101.971, 1.0), 100.0, Material(vec3(1.0, 1.0, 0.96), 1.2, vec3(1.0, 0.0, 0.0), 0.0)) // sun
    );

// const int NUM_OF_TRIANGLES = 1;
// const Triangle TRIANGLES[NUM_OF_TRIANGLES] = Triangle[NUM_OF_TRIANGLES](
//         Triangle(
//             vec3(20.0, -1.0, -5.0), vec3(17.5, 5.0, 10.0), vec3(10.0, 1.0, 10.0),
//             vec3(0, 1, 0), vec3(0, 1, 0), vec3(0, 1, 0)
//         )
//     );

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
    vec3 e1 = tri.b - tri.a;
    vec3 e2 = tri.c - tri.a;
    vec3 De2 = cross(D, e2);
    float det = dot(e1, De2);

    if (abs(det) < ray.epsilon) {
        // Jeśli wyznacznik jest bliski zeru (ray.epsilon przechowuje bardzo niewielką wartość)
        // to oznacza, że wektory D, e1 i e2 nie są liniowo niezależne. Co oznacza, że promień
        // leci równolegle do płaszczyzny, czyli nie trafia w trójkąt.
        return hitInfo;
    }

    float inv_det = 1.0 / det;

    vec3 T = ray.origin - tri.a;
    vec3 Te1 = cross(T, e1);

    float u = dot(T, De2) * inv_det;

    // Jeśli współrzędna u w układzie współrzędnych barycentrycznych jest mniejsza od 0
    // lub większa od 1 oznacza to, że punkt znajduje się za daleko od tego wierzchołka,
    // aby być w trójkącie.
    if (u < 0 || u > 1) return hitInfo;

    float v = dot(D, Te1) * inv_det;

    // Podobnie co w przypadku współrzędnej u, sprawdzamy dla v. Dodatkowo sprawdzamy
    // czy suma tych współrzędnych jest większa od 1, punkt znajduje się w środku trójkąta
    // tylko gdy suma wszystkich współrzędnych jest mniejsza od 1.
    if (v < 0 || u + v > 1) return hitInfo;

    float t = dot(Te1, e2) * inv_det;
    // Jeśli bylibyśmy oddaleni o tylko epsilon od trójkąta, to nie ma sensu
    // zaliczać tego jako kolizji.
    if (t > ray.epsilon) {
        hitInfo.didHit = true;
        hitInfo.hitPoint = ray.origin + ray.dir * t;

        float w = 1 - u - v;
        hitInfo.normal = normalize(w * tri.na + u * tri.nb + v * tri.nc);
        hitInfo.dst = t;
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
bool RayBoundingBoxIntersection(Ray ray, MeshInfo mi) {
    float tmin = -INFTY, tmax = INFTY;

    // check the intersection at x axis
    float tx1 = (mi.boundsMin.x - ray.origin.x) * ray.inv_dir.x;
    float tx2 = (mi.boundsMax.x - ray.origin.x) * ray.inv_dir.x;

    // we change the value of tmin if one of these is smaller than the current
    tmin = max(tmin, min(tx1, tx2));
    tmax = min(tmax, max(tx1, tx2));

    // check the intersection at y axis
    float ty1 = (mi.boundsMin.y - ray.origin.y) * ray.inv_dir.y;
    float ty2 = (mi.boundsMax.y - ray.origin.y) * ray.inv_dir.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    // check the intersection at z axis
    float tz1 = (mi.boundsMin.z - ray.origin.z) * ray.inv_dir.z;
    float tz2 = (mi.boundsMax.z - ray.origin.z) * ray.inv_dir.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    return tmax > max(tmin, 0.0);
}

// returns info about the nearest point which the ray hits
HitInfo CalculateRayCollision(Ray ray) {
    HitInfo closestHit;
    closestHit.didHit = false;
    closestHit.dst = INFTY;

    // iterate through all triangles
    for (int i = 0; i < numOfMeshes; ++i) {
        MeshInfo mi = getMesh(i);
        if (!RayBoundingBoxIntersection(ray, mi)) continue;

        for (int j = 0; j < mi.numTriangles; ++j) {
            HitInfo hit = RayTriangleIntersection(ray, getTriangle(mi.firstTriangleIndex + j));
            if (hit.didHit && hit.dst < closestHit.dst) {
                closestHit = hit;
                closestHit.mat = getMaterial(mi.materialIndex);
            }
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
// vec3 RandomHemisphereDirection(vec3 normal, inout uint rngState) {
//     // dot product is negative if the vectors are more than 90 degrees apart
//     vec3 randomVector = RandomUnitVector(rngState);
//     return sign(dot(normal, randomVector)) * randomVector;
// }

vec3 DiffuseDirection(vec3 normal, inout uint rngState) {
    return normalize(normal + RandomUnitVector(rngState));
}

vec3 ReflectDirection(vec3 dir, vec3 normal) {
    return dir - 2 * dot(dir, normal) * normal;
}

// copy pasted from sebastian's video
// TODO: write the enviroment function on your own!
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
            // ray.dir = RandomHemisphereDirection(hitInfo.normal, rngState);
            vec3 diffuseDir = DiffuseDirection(hitInfo.normal, rngState);
            vec3 reflectDir = ReflectDirection(ray.dir, hitInfo.normal);
            ray.dir = mix(diffuseDir, reflectDir, hitInfo.mat.specularComponent);
            // ray.dir = mix(ray.dir, nearestEmitter, 0.4);
            // return hitInfo.normal;
            // return ray.dir;

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

void main() {
    // init seed
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
    cam.pos = vec3(-2.2, 0.8, 4.5);
    cam.lookat = vec3(-2.2, 0.8, 0.0);
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

    // shooting rays
    vec3 c = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < SAMPLES_PER_PIXEL; ++i) {
        c += GetColorForRay(ray, rngState);
    }
    c /= float(SAMPLES_PER_PIXEL);

    vec3 lastFrameColor = texture(backBufferTexture, gl_FragCoord.xy / iResolution.xy).rgb;
    if (iFrame == 0) lastFrameColor = c;
    // c = mix(c, lastFrameColor, 1.0 / float(iFrame + 1));
    float weight = 1.0 / (float(iFrame) + 1.0);
    c = lastFrameColor * (1.0 - weight) + c * weight;

    gl_FragColor = vec4(c, 1.0);
}
