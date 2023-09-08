#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "RenderLib.h"



/* my renderer only have one camera ,maybe need more later , let it be */
//Camera camera(WIDTH, HEIGHT, Eigen::Vector3f(0.0f, 0.0f, 1.0f));


const unsigned int width = 800;
const unsigned int height = 600;




#ifdef CLOTH
int main(int argc, char **argv)
 {
    /* interesting things about glfw , it needs a brace to make it close normal */
    {
        Windows_Init(width,height);

        Camera camera(width, height, Eigen::Vector3f(0.0f, 1.0f, 3.0f));
        camera.lookAt(Eigen::Vector3f(0.f,1.f,3.f),Eigen::Vector3f(0,0,0),Eigen::Vector3f(0,1.f,1.f));
        Bind_Camera(&camera);

        /*Entity entity(Eigen::Vector3f(0.f,0.f,0.f),Eigen::Vector3f(0.f,0.f,0.f),Eigen::Vector3f(0.1f,0.1f,0.1f));
        entity.Load("res/models/nanosuit/nanosuit.obj");*/

        /*Plane plane(Eigen::Vector3f(0, -1.f, 0), Eigen::Vector3f(0, 1.f, 0), Eigen::Vector3f(1.f, 0, 0), 10, 10);
        plane.Load_Texture("res/textures/box.png");

        Plane plane2(Eigen::Vector3f(2.f, 0, 0), Eigen::Vector3f(-1.f, 0, 0), Eigen::Vector3f(0.f, 1, 0), 10, 10);
        plane2.Load_Texture("res/textures/box.png");

        Cube cube(Eigen::Vector3f(0, 0.1f,0), Eigen::Vector3f(0,0,0), Eigen::Vector3f(1,1,1),1.f);

        RigidBody bunny(Eigen::Vector3f(0,0,0),Eigen::Vector3f(0,0,0),Eigen::Vector3f(1.f,1.f,1.f));
        bunny.Load("res/models/bunny.obj");*/
        Cloth cloth(Eigen::Vector3f(0,0,0),Eigen::Vector3f(0,0,0),Eigen::Vector3f(1,1,1));
        cloth.Load_Texture("res/textures/cloth.jpg");


        DirectionalLight light(
            Eigen::Vector3f(0.0f, -3.0f, 1.0f), //m_Direction
            Eigen::Vector3f(0.2f, 0.2f, 0.2f),  //m_Ambient
            Eigen::Vector3f(0.8f, 0.8f, 0.8f),  //m_Diffuse
            Eigen::Vector3f(1.0f, 1.0f, 1.0f)   //m_Specular
        );
        


        Renderer renderer(&camera);
        renderer.Insert_Entity(&cloth);
        /*renderer.Insert_Entity(&bunny);
        renderer.Insert_Entity(&plane);
        renderer.Insert_Entity(&plane2);*/
        //renderer.Insert_Entity(&cube);

        renderer.Insert_Light(&light);
        renderer.SetBackGroundColor(Eigen::Vector4f(1, 1, 1, 1));

        Windows_Start(&renderer);


    }
            Windows_Terminate();

    return 0;
}
#endif