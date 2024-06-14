// resources:
// - https://www.youtube.com/watch?v=Qz0KTGYJtUk
// - https://blog.demofox.org/2020/05/25/casual-shadertoy-path-tracing-1-basic-camera-diffuse-emissive/
// - https://www.realtimerendering.com/raytracing/Ray%20Tracing%20in%20a%20Weekend.pdf
// - https://www.intel.com/content/www/us/en/content-details/763947/path-tracing-workshop-part-1-ray-tracing.html

struct Camera {
    vec3 pos;
    // right vector => vector pointing to where is right according to camera
    vec3 r;
    // up vector => vector pointing to where is up according to camera
    vec3 up;
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

struct Material{
	vec4 color;
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
    float b = 2 * dot(ray.dir, offsetRay);
    float c = dot(offsetRay, offsetRay) - s.r * s.r;

    float d = b * b - 4 * a * c;
    if (d >= 0) {
        float d1 = (-b - sqrt(d)) / (2 * a);
        float d2 = (-b + sqrt(d)) / (2 * a);
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

const int NUM_OF_SPHERES = 3;
const Sphere SPHERES[NUM_OF_SPHERES] = Sphere[NUM_OF_SPHERES](
	Sphere(vec3(-1,0,1), 1.2, Material(vec4(0,0,1,0))),
	Sphere(vec3(0,0,2), 1.3, Material(vec4(1,0,0,0))),
	Sphere(vec3(1,0,3), 1.5, Material(vec4(0,1,0,0)))
 );

// returns info about the nearest point which the ray hits
HitInfo CalculateRayCollision(Ray ray){
	HitInfo closestHit;
	closestHit.didHit = false;
	closestHit.dst = 1.0/0.0; // infinity

	// iterate through all the spheres
	for (int i = 0; i < NUM_OF_SPHERES; ++i){
		HitInfo hit = RayHitsSphere(ray, SPHERES[i]);
		if (hit.didHit && hit.dst < closestHit.dst){
			closestHit = hit;
			closestHit.mat = SPHERES[i].mat;
		}
	}
	return closestHit;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    // fragCoord stores the pixel coordinates [0.5, resolution-0.5]
    vec2 uv = fragCoord.xy / iResolution.xy; // normalized coordinates [0, 1]
    uv = 2 * uv - 1; // normalized coordinates [-1, 1]
    float aspectRatio = iResolution.x / iResolution.y;
    uv.x *= aspectRatio;

    // camera
    Camera cam;
    cam.pos = vec3(0, 0, -2);
    vec3 cameraForwardPlaneNormalVector = vec3(0, 0, 1);

    // rays
    // imagine that the camera is just a point that shoots rays
    // but is behind the screen and each ray has to travel through a different pixel
    Ray ray;
    ray.origin = cam.pos;
    ray.dir = vec3(uv, 0) + cameraForwardPlaneNormalVector;
    ray.epsilon = 0.001;

    // shooting rays
    HitInfo hitInfo = CalculateRayCollision(ray);
    vec4 c = hitInfo.didHit ? hitInfo.mat.color : vec4(0,0,0,0);
    fragColor = c;
}
