


#define PI  3.1415926535897932
#define g 9.81
#include <complex>
#include <fstream>
#include <iostream>
#include <string>


int main() {



    //-----------------------
    //Variables
    double l,x,y,vx,vy,t,t0,tf,dt;
    double theta,theta0,dtheta_dt,omega;
    std::string buf;

    // Ask for user input
    std::cout << "# Enter l: \n";
    std::cin >> l;
    std::cout << "# theta0: \n";
    std::cin >> theta0; std::getline(std::cin, buf);
    std::cout << "# Enter t0,tf,dt: \n";
    std::cin >> t0 >> tf >> dt; std::getline(std::cin, buf);
    std::cout << "l= " << l << "theta0= " << theta0 << std::endl;
    std::cout << "t0= " << t0 << "tf= " << tf
        << "dt= " << dt << std::endl;

    //Initialize
    omega=std::sqrt(g/l);
    std::cout << "# Omega= " << omega << "T= " << 2.0*PI/omega <<std::endl;
    std::ofstream myfile("SimplePendulum.dat");
    myfile.precision(17);


    //Compute
    t = t0;
    while (t <= tf) {
        theta = theta0*cos(omega *(t-t0));
        dtheta_dt = -omega*theta0*sin(omega* (t-t0));
        x = l*sin(theta);
        y= -l*cos(theta);
        vx = l* dtheta_dt*cos(theta);
        vy = l*dtheta_dt*sin(theta);
        myfile << t << " " << x << " " << y << "" << vx << " "<< vy << " " << std::endl;
        t = t+ dt;

    }










    return 0;
}
