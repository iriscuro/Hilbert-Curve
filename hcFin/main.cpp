//HECHO 10/06/2023
#include <stdio.h>
#include <math.h>
#include <omp.h>
#include <chrono>
#include <iostream>
#include <iomanip>  // Para std::fixed y std::setprecision
#include <ctime>
#include <vector>
#include <sstream>

#define STEP_SZ 10
#define H_WINDOWS 800
#define W_WINDOWS 800
#define SPEED 1 // check https://docs.python.org/3/library/turtle.html#turtle.speed

using namespace std;
FILE* out;
std::vector<std::vector<std::string>> final_commands(4); // Vector de vectores de strings con las instrucciones finales
// Definir una lista de colores para cada cuadrante
vector<string> colors = {"blue", "red", "green", "purple"};

void print_python_head() {
    fprintf(out, "import turtle\n");
    fprintf(out, "turtle.setup(%d, %d)\n", H_WINDOWS, W_WINDOWS);
    //fprintf(out, "turtle.speed(%d)\n", SPEED);
    fprintf(out, "turtle.tracer(0)\n");
}

void print_python_end() {
    fprintf(out, "turtle.update()\n");
    fprintf(out, "screen = turtle.Screen()\n");// EXIT ON CLICK
    fprintf(out, "screen.exitonclick()\n");// EXIT ON CLICK
}


void hilbert_curve(int n, int angle) {
    if (n == 0) return;
    fprintf(out, "turtle.right(%d)\n", angle);
    hilbert_curve(n - 1, -angle);
    fprintf(out, "turtle.forward(%d)\n", STEP_SZ);
    fprintf(out, "turtle.left(%d)\n", angle);
    hilbert_curve(n - 1, angle);
    fprintf(out, "turtle.forward(%d)\n", STEP_SZ);
    hilbert_curve(n - 1, angle);
    fprintf(out, "turtle.left(%d)\n", angle);
    fprintf(out, "turtle.forward(%d)\n", STEP_SZ);
    hilbert_curve(n - 1, -angle);
    fprintf(out, "turtle.right(%d)\n", angle);
}



void hilbert_curve_p(int n, int angle, std::vector<std::string>& commands) {
    if (n == 0) return;
    commands.emplace_back("turtle.right(" + to_string(angle) + ")\n");
    hilbert_curve_p(n - 1, -angle, commands);

    commands.emplace_back("turtle.forward(" + to_string(STEP_SZ) + ")\n");
    commands.emplace_back("turtle.left(" + to_string(angle) + ")\n");

    hilbert_curve_p(n - 1, angle, commands);

    commands.emplace_back("turtle.forward(" + to_string(STEP_SZ) + ")\n");

    hilbert_curve_p(n - 1, angle, commands);
    commands.emplace_back("turtle.left(" + to_string(angle) + ")\n");
    commands.emplace_back("turtle.forward(" + to_string(STEP_SZ) + ")\n");

    hilbert_curve_p(n - 1, -angle, commands);
    commands.emplace_back("turtle.right(" + to_string(angle) + ")\n");
}
//Versión para cada thread usando coordenadas x e  y
void generate_commands_parallel(int order, int angle, int id_thread) {
    int xx = 0;
    int yy = 0;
    //Para x
    switch (id_thread) {
        case 0:
        case 1:
            xx = 0;
            break;
        case 2:
            xx = STEP_SZ * std::pow(2, order - 1);
            break;
        default:
            xx = STEP_SZ * std::pow(2, order - 1) * 2 - STEP_SZ;
            break;
    }
    //Para y
    if (id_thread != 0) {
        yy = -STEP_SZ * std::pow(2, order - 1);
    }
    if (id_thread == 3) {
        yy += STEP_SZ;
    }

    int thread_angle = (id_thread == 0 || id_thread == 3) ? -angle : angle;

    std::vector<std::string> commandsT;
    commandsT.emplace_back("turtle.setpos(" + std::to_string(xx) + "," + std::to_string(yy) + ")\n");
    commandsT.emplace_back("turtle.pendown()\n");
    commandsT.emplace_back("turtle.color(\"" + colors[id_thread] + "\")\n");

    std::string nn;
    switch (id_thread) {
        case 0:
            nn = "turtle.setheading(-90)\n";
            break;
        case 3:
            nn = "turtle.setheading(90)\n";
            break;
        default:
            nn = "turtle.setheading(0)\n";
            break;
    }
    commandsT.emplace_back(nn);

    hilbert_curve_p(order - 1, thread_angle, commandsT);

    // Almacenar las cadenas generadas por cada hilo en la matriz compartida
    final_commands[id_thread] = std::move(commandsT);
}

//Versión paralela analizando las direcciones para cada thread
//según el cuadrante que le toca dibujar
void generate_commands(int order, int angle, int id_thread) {

    int thread_angle = (id_thread == 0 || id_thread == 3) ? -angle : angle;

    std::vector<std::string> commandsT;
    commandsT.emplace_back("turtle.color(\"" + colors[id_thread] + "\")\n");

    std::string nn;
    switch (id_thread) {
        case 0:
            nn = "turtle.setheading(-90)\n";
            break;
        case 3:
            nn = "turtle.setheading(90)\n";
            break;
        default:
            nn = "turtle.setheading(0)\n";
            break;
    }
    commandsT.emplace_back(nn);

    hilbert_curve_p(order - 1, thread_angle, commandsT);

    if(id_thread == 2) commandsT.emplace_back("turtle.setheading(90)\n");
    if(id_thread != 3)
        commandsT.emplace_back("turtle.forward(" + to_string(STEP_SZ) + ")\n");

    // Almacenar las cadenas generadas por cada hilo en la matriz compartida
    final_commands[id_thread] = std::move(commandsT);
}

void test_performance(int order, int angle, std::vector<std::string>& output, int num_tests) {
    double total_time_seq = 0.0;
    double total_time_par = 0.0;

    for (int test = 0; test < num_tests; ++test) {
        // Secuencial
        out = fopen("hilbert_turtle_seq.py", "w+");
        print_python_head();
        std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
        hilbert_curve(order, angle);
        std::chrono::steady_clock::time_point end1 = std::chrono::steady_clock::now();
        print_python_end();
        fclose(out);
        double tiempo1 = std::chrono::duration_cast<std::chrono::duration<double>>(end1 - begin1).count();
        total_time_seq += tiempo1;

        // Paralelo
        out = fopen("hilbert_turtle_par.py", "w+");
        print_python_head();
        std::chrono::steady_clock::time_point begin2 = std::chrono::steady_clock::now();

        // Generar los comandos paralelamente
        #pragma omp parallel num_threads(4)
        {
            int id_thread = omp_get_thread_num();
            //generate_commands(order, angle, id_thread);
            generate_commands_parallel(order, angle, id_thread);
        }
        //Escribiendo los comandos finales
        for (int i = 0; i < 4; i++) {
            for (const auto& str : final_commands[i]) {
                fprintf(out, "%s", str.c_str());
            }
            fprintf(out, "turtle.stamp()\n");
        }
        std::chrono::steady_clock::time_point end2 = std::chrono::steady_clock::now();
        print_python_end();
        fclose(out);
        double tiempo2 = std::chrono::duration_cast<std::chrono::duration<double>>(end2 - begin2).count();
        total_time_par += tiempo2;
    }

    double average_time_seq = total_time_seq / num_tests;
    double average_time_par = total_time_par / num_tests;

    std::cout << "Average Time (Sequential): " << std::fixed << std::setprecision(6) << average_time_seq << " seconds\n";
    std::cout << "Average Time (Parallel): " << std::fixed << std::setprecision(6) << average_time_par << " seconds\n";
}


int main(int argc, char const* argv[]) {
    int order = 10;
    int angle = 90;
    int num_tests = 1; // Número de pruebas a realizar

    std::vector<std::string> output;//vector para almacenar los subcomandos
    test_performance(order, angle, output, num_tests);

    return 0;
}
