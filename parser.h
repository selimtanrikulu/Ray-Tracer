#ifndef __HW1__PARSER__
#define __HW1__PARSER__

#include <string>
#include <vector>

//extras
#include <cmath>
#include <iostream>


namespace parser
{
    //Notice that all the structures are as simple as possible
    //so that you are not enforced to adopt any style or design.
    class Vec3f
    {
        public : 
            float x, y, z;
            Vec3f(float x,float y,float z){this->x=x; this->y=y; this->z=z;}
            Vec3f(){x = 0; y = 0; z = 0;}

            //methods
            float Dot(Vec3f second){return x*second.x+y*second.y+z*second.z;}
            Vec3f Cross(Vec3f second){
                Vec3f result(y*second.z-z*second.y,z*second.x-x*second.z,x*second.y-y*second.x);
                return result;
            }
            Vec3f Normalized()
            {   
                float len = sqrt(x*x+y*y+z*z);
                Vec3f result(x/len,y/len,z/len);
                return result;
            }

            Vec3f ColorProduct(Vec3f second)
            {
                Vec3f result;
                result.x = x*second.x;
                result.y = y*second.y;
                result.z = z*second.z;
                return result;
            }

            float Length()
            {
                return sqrt(x*x+y*y+z*z);
            }


            //operator overloadings
            Vec3f operator+(const Vec3f& second)
            {
            Vec3f result;
            result.x = this->x + second.x;
            result.y = this->y + second.y;
            result.z = this->z + second.z;
            return result;
            }
            Vec3f operator-(const Vec3f& second)
            {
            Vec3f result;
            result.x = this->x - second.x;
            result.y = this->y - second.y;
            result.z = this->z - second.z;
            return result;
            }
            Vec3f operator*(const float& coef)
            {
            Vec3f result;
            result.x = this->x * coef;
            result.y = this->y * coef;
            result.z = this->z * coef;
            return result;
        }


        
    };

    class Vec3i
    {
        public : 
            int x, y, z;

            void Clamp()
            {
                if(x<0)x=0;if(y<0)y=0;if(z<0)z=0;
                if(x>255)x=255;if(y>255)y=255;if(z>255)z=255;
            }

            Vec3i()
            {
                x=0;
                y=0;
                z=0;
            }

            Vec3i operator+(const Vec3i& second)
            {
            Vec3i result;
            result.x = this->x + second.x;
            result.y = this->y + second.y;
            result.z = this->z + second.z;
            return result;
            }

    };

    struct Vec4f
    {
        float x, y, z, w;
    };

    class Camera
    {
        public : 
            Vec3f position;
            Vec3f gaze;
            Vec3f up;
            Vec4f near_plane;
            float near_distance;
            int image_width, image_height;
            std::string image_name;

            //constants yielded by camera information
            Vec3f m;
            Vec3f q;
            float pw;
            float ph;
            Vec3f u;
            Vec3f v;
            float l = near_plane.x;float r = near_plane.y;float b = near_plane.y;float t = near_plane.w;

            void CalculateConstants()
            {   
                l = near_plane.x;r = near_plane.y;b = near_plane.z;t = near_plane.w;
                u = (gaze.Cross(up)).Normalized();
                v = up.Normalized();
                m = position + gaze.Normalized()*near_distance;
                
                q = m + u*l + v*t;
                pw = (r-l)/image_width;
                ph = (t-b)/image_height;
            }
            
    };

    struct PointLight
    {
        Vec3f position;
        Vec3f intensity;
    };

    class Material
    {
        public : 
            bool is_mirror;
            Vec3f ambient;
            Vec3f diffuse;
            Vec3f specular;
            Vec3f mirror;
            float phong_exponent;

            
    };

    struct Face
    {
        int v0_id;
        int v1_id;
        int v2_id;
    };

    

    class Triangle
    {
        public : 
            int material_id;
            Face indices;

            //constants yielded by Triangle information
            Vec3f vertex1;
            Vec3f vertex2;
            Vec3f vertex3;
            Material mat;
            Vec3f normal;

            //Constructor
            Triangle(int material_id,Face indices)
            {
                this-> material_id = material_id;
                this-> indices = indices;
            }

            Triangle()
            {
                
            }

            void CalculateConstants(Vec3f vertex1, Vec3f vertex2, Vec3f vertex3,Material mat)
            {  
                this->vertex1=vertex1;this->vertex2=vertex2;this->vertex3=vertex3;this->mat=mat;
                normal = (vertex2-vertex1).Cross(vertex3-vertex1).Normalized();
            }

            


    };

    struct Mesh
    {

        int material_id;
        std::vector<Face> faces;

    };
    class Sphere
    {
        public : 
            int material_id;
            int center_vertex_id;
            float radius;

            //constants yielded by sphere information
            Material mat;
            Vec3f center;

            void CalculateConstants(Material mat,Vec3f center)
            {
                this->mat=mat;this->center=center;
            }


            


    };

    struct Ray
    {
        Vec3f start;
        Vec3f dir;
    };


    struct Scene
    {
        //Data
        Vec3i background_color;
        float shadow_ray_epsilon;
        int max_recursion_depth;
        std::vector<Camera> cameras;
        Vec3f ambient_light;
        std::vector<PointLight> point_lights;
        std::vector<Material> materials;
        std::vector<Vec3f> vertex_data;
        std::vector<Mesh> meshes;
        std::vector<Triangle> triangles;
        std::vector<Sphere> spheres;

        //Functions
        void loadFromXml(const std::string &filepath);
    };
}

#endif
