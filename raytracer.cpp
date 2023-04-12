#include <iostream>
#include "parser.h"
#include "ppm.h"

//extras
using namespace parser;


Scene scene;

void MakeInitializations()
{
    for(int i=0;i<scene.cameras.size();i++)
    {
        scene.cameras[i].CalculateConstants();
    }
    for(int i=0;i<scene.spheres.size();i++)
    {
        scene.spheres[i].CalculateConstants(scene.materials[scene.spheres[i].material_id-1],scene.vertex_data[scene.spheres[i].center_vertex_id-1]);
    }

    
    

    for(Mesh mesh : scene.meshes)
    {
        for(Face face : mesh.faces)
        {
            
            scene.triangles.push_back(Triangle(mesh.material_id,face));
        }

    }

    for(int i=0;i<scene.triangles.size();i++)
    {
        scene.triangles[i].CalculateConstants(scene.vertex_data[scene.triangles[i].indices.v0_id-1],scene.vertex_data[scene.triangles[i].indices.v1_id-1]
                                                ,scene.vertex_data[scene.triangles[i].indices.v2_id-1],scene.materials[scene.triangles[i].material_id-1]);
    }
}

Ray CalculateRay(int i,int j,Camera cam)
{
    Ray result;
    result.start = cam.position;
    result.dir = (cam.q + cam.u*(cam.pw*(0.5+i)) + cam.v*(-cam.ph*(0.5+j)))+cam.position*-1;
    return result;
}

float IntersectSphere(Ray ray,Sphere sphere)
{
    float A,B,C; //constants for the quadratic equation
	
	float delta;
	
	Vec3f c;
	
	c = sphere.center;
	
	float t,t1,t2;
	
	C = (ray.start.x-c.x)*(ray.start.x-c.x)+(ray.start.y-c.y)*(ray.start.y-c.y)+(ray.start.z-c.z)*(ray.start.z-c.z)-sphere.radius*sphere.radius;

	B = 2*ray.dir.x*(ray.start.x-c.x)+2*ray.dir.y*(ray.start.y-c.y)+2*ray.dir.z*(ray.start.z-c.z);
	
	A = ray.dir.x*ray.dir.x+ray.dir.y*ray.dir.y+ray.dir.z*ray.dir.z;
	
	delta = B*B-4*A*C;
	
	if (delta<0) return -1;
	else if (delta==0)
	{
		t = -B / (2*A);
	}
	else
	{
		delta = sqrt(delta);
		A = 2*A;
		t1 = (-B + delta) / A;
		t2 = (-B - delta) / A;
				
		if (t1<t2) t=t1; else t=t2;
	}
	
	return t;
}

float Determinant(float a,float b,float c,float d,float e, float f,float g,float h,float i)
{
    return a*(e*i-h*f)+b*(g*f-d*i)+c*(d*h-e*g);
}


float IntersectTriangle(Ray ray,Triangle triangle, bool isShadowCheck)
{

    //BFC 
    if(!isShadowCheck)
    {
        if(triangle.normal.Dot(ray.dir)>0)return -1;
    }
    //END OF BFC
    Vec3f a = triangle.vertex1;
    Vec3f b = triangle.vertex2;
    Vec3f c = triangle.vertex3;
    Vec3f d = ray.dir;
    Vec3f o = ray.start;

    float detA = Determinant(a.x-b.x,a.y-b.y,a.z-b.z,a.x-c.x,a.y-c.y,a.z-c.z,d.x,d.y,d.z);
    float detBeta = Determinant(a.x-o.x,a.y-o.y,a.z-o.z,a.x-c.x,a.y-c.y,a.z-c.z,d.x,d.y,d.z);
    float detGama = Determinant(a.x-b.x,a.y-b.y,a.z-b.z,a.x-o.x,a.y-o.y,a.z-o.z,d.x,d.y,d.z);
    float detT = Determinant(a.x-b.x,a.y-b.y,a.z-b.z,a.x-c.x,a.y-c.y,a.z-c.z,a.x-o.x,a.y-o.y,a.z-o.z);

    float beta = detBeta/detA;
    float gama = detGama/detA;
    float t = detT/detA;

    if(t>=0 && beta+gama<=1 && beta >= 0 && gama >= 0)
    {
        return t;
    }
    else{
        return -1;
    }

}


float max(float a,float b)
{
    if(a>b)return a;
    return b;
}


bool isInShadow(Ray ray, float dist)
{
    float tSphere;
    for (Sphere sphere : scene.spheres)
	{
		tSphere = IntersectSphere(ray,sphere);
		if (tSphere > scene.shadow_ray_epsilon && tSphere < dist)
		{
            return true;
		}
	}

    

    float tTriangle;
    for(Triangle triangle : scene.triangles)
    {
        tTriangle = IntersectTriangle(ray,triangle,true);
        if(tTriangle > scene.shadow_ray_epsilon && tTriangle < dist)
        {
            return true;
        }
    }

    return false;
}


Vec3i ComputeColor(Ray ray,int recursionCount,bool isRecursionStart)
{
    Vec3i color;
    if(!isRecursionStart)
    {
        color.x = scene.background_color.x;
        color.y = scene.background_color.y;
        color.z = scene.background_color.z;
    }
    


    float minTSphere = 90000;
    float tSphere;
    Sphere minSphere;
    for (Sphere sphere : scene.spheres)
	{
		tSphere = IntersectSphere(ray,sphere);
		if (tSphere<minTSphere && tSphere > scene.shadow_ray_epsilon)
		{
			minTSphere = tSphere;
            minSphere = sphere;
		}
	}

    
    float minTTriangle = 90000;
    float tTriangle;
    Triangle minTriangle;
    for(Triangle triangle : scene.triangles)
    {
        tTriangle = IntersectTriangle(ray,triangle,false);
        if(tTriangle<minTTriangle && tTriangle > scene.shadow_ray_epsilon)
        {
            minTTriangle = tTriangle;
            minTriangle = triangle;
        }
    }
    
    if(minTTriangle > 89999 && minTSphere > 89999)
    {
        return color;
    }

    color.x = 0;
    color.y = 0;
    color.z = 0;
    Vec3i ambientComponent;
    Vec3i diffuseComponent;
    Vec3i specularComponent;
    Vec3i reflectComponent;
    Vec3f toRay = (ray.dir*-1).Normalized();
    Material mat;
    Vec3f intersectionPoint;
    Vec3f normal;

    //Intersection occurs with minSphere
    if(minTSphere<minTTriangle)
    {
        intersectionPoint = ray.start + (ray.dir*minTSphere);
        normal = (intersectionPoint-minSphere.center).Normalized();
        mat = minSphere.mat;
    }
    else
    {
        intersectionPoint = ray.start + (ray.dir*minTTriangle);
        normal = minTriangle.normal;
        mat = minTriangle.mat;
    }

        //AMBIENT CALCULATION
        Vec3f tempAmbient = mat.ambient.ColorProduct(scene.ambient_light);

        ambientComponent.x = tempAmbient.x;
        ambientComponent.y = tempAmbient.y;
        ambientComponent.z = tempAmbient.z;


        // DIFFUSE + SPECULAR CALCULATION
        for(PointLight pointLight : scene.point_lights)
        {
            //SHADOW CHECK
            Ray shadowRay;
            shadowRay.start = intersectionPoint;
            shadowRay.dir = pointLight.position-intersectionPoint;
            float dist = shadowRay.dir.Length();
            shadowRay.dir = shadowRay.dir.Normalized();
            if(isInShadow(shadowRay,dist))continue; //SHADOW OCCURS

            //DIFFUSE CALCULATION
            Vec3f toLight = (pointLight.position - intersectionPoint).Normalized();
            
            float lightDistance = (pointLight.position-intersectionPoint).Length();
            Vec3f radiance = pointLight.intensity*(1/(lightDistance*lightDistance));
            Vec3f adding = radiance*(max(0,toLight.Dot(normal)));

            Vec3f temp = mat.diffuse.ColorProduct(adding);

            diffuseComponent.x += temp.x;
            diffuseComponent.y += temp.y;
            diffuseComponent.z += temp.z;


            //SPECULAR CALCULATION
            
            Vec3f halfVector = (toRay+toLight).Normalized();
            Vec3f adding2 = radiance*(pow(max(0,halfVector.Dot(normal)),mat.phong_exponent));
            
            Vec3f temp2 = mat.specular.ColorProduct(adding2);
            
            

            specularComponent.x += temp2.x;
            specularComponent.y += temp2.y;
            specularComponent.z += temp2.z;
            

        }


        //REFLECT CALCULATION
        
        if(mat.is_mirror)
        {
            if(isRecursionStart)
            {
                if(recursionCount==0)
                {
                    //do nothing
                }
                else
                {
                    Ray reflectRay;
                    reflectRay.start = intersectionPoint;
                    reflectRay.dir = (toRay*-1) + ((normal*2)*(normal.Dot(toRay)));

                    Vec3i res = ComputeColor(reflectRay,recursionCount-1,true);
                    reflectComponent = res;

                    reflectComponent.x = reflectComponent.x*mat.mirror.x;
                    reflectComponent.y = reflectComponent.y*mat.mirror.y;
                    reflectComponent.z = reflectComponent.z*mat.mirror.z;
                }
                
            }
            else
            {
                Ray reflectRay;
                reflectRay.start = intersectionPoint;
                reflectRay.dir = (toRay*-1) + ((normal*2)*(normal.Dot(toRay)));
                Vec3i res = ComputeColor(reflectRay,scene.max_recursion_depth-1,true);
                reflectComponent = res;
                reflectComponent.x = reflectComponent.x*mat.mirror.x;
                reflectComponent.y = reflectComponent.y*mat.mirror.y;
                reflectComponent.z = reflectComponent.z*mat.mirror.z;
            }
      
        }

        color.x = ambientComponent.x+diffuseComponent.x+specularComponent.x+reflectComponent.x;
        color.y = ambientComponent.y+diffuseComponent.y+specularComponent.y+reflectComponent.y;
        color.z = ambientComponent.z+diffuseComponent.z+specularComponent.z+reflectComponent.z;

        //END OF DSA

    

  
    color.Clamp();
    return color;
}



int main(int argc, char* argv[])
{
   
    scene.loadFromXml(argv[1]);
    MakeInitializations();
    


    //Main Camera Loop      
    for(Camera cam : scene.cameras)
    {
        

        unsigned char* image = new unsigned char [cam.image_width * cam.image_height * 3];
        int a=0;
        for(int i=0;i<cam.image_width;i++)
        {
            for(int j=0;j<cam.image_height;j++)
            {
                Ray ray = CalculateRay(i,j,cam);

                Vec3i RGB = ComputeColor(ray,0,false);
                

                int startIndex = j*cam.image_width*3+i*3;
                image[startIndex++] = RGB.x;
			    image[startIndex++] = RGB.y;
			    image[startIndex++] = RGB.z;

            

            }
        }

        
        write_ppm(cam.image_name.c_str(), image, cam.image_width, cam.image_height);
    }
    
    


}
