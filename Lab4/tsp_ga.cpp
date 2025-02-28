#include "tsp_ga.h"
#include <cmath>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <chrono>

tsp_ga::tsp_ga(char* input) {
    p = pp_singleton::pp_instance(input);
    tsp_list = p ->return_interface() ->return_it();
    size = tsp_list.size();

    fill_distance();
}

//Pure virtual functions.

void tsp_ga::run_tsp() {
    ga_engine();
}

void tsp_ga::display() {

}

//GA functions

void tsp_ga::ga_engine() {
    //Repeat all the processes, rank the routes in current gen using rankRoutes(). 
    //Run as many generations as needed to find the best path.
    std::chrono::high_resolution_clock::time_point s = std::chrono::high_resolution_clock::now();
    ga_initialize();

    for(int i = 0; i < 250000; i++) {
        ga_selection();
        ga_crossbreed();
        ga_mutate();

        for(int j = 0; j < next_gen_list.size(); j++) {
            double fitness = euclidian_distance(next_gen_list[j].first);
            next_gen_list[j].second = fitness;
        }
        permutated_list.clear();
        permutated_list = next_gen_list;
        next_gen_list.clear();
        selected_list.clear();

        // for(int i = 0; i < permutated_list.size(); i++) {
        //     for(int j = 0; j < permutated_list[i].first.size(); j++) {
        //         std::cout << next_gen_list[i].first[j].get_nodeID() << " ";
        //     }
        //     std::cout << "\t" << next_gen_list[i].second;
        //     std::cout << std::endl;
        // }
    }
    std::chrono::high_resolution_clock::time_point e = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(e-s).count();
    std::cout << "execution time: " << duration << std::endl;

    std::cout << "solution list" << " ";

    for(int j = 0; j < solution_list[0].first.size(); j++) {
        std::cout << solution_list[0].first[j].get_nodeID() << " ";
    }   std::cout << std::endl;
}

void tsp_ga::ga_initialize() {
    //Create population -> all possible routes or optimize by cutting off this process.
    permutate();

    //Determine fitness, rank each individual(chromosome)
    //Output will be an ordered list with the routeID and each associated fitness score
    for(int i = 0; i < permutated_list.size(); i++) {
        double fitness = euclidian_distance(permutated_list[i].first);
        permutated_list[i].second = fitness;
    }

    //Sorting the population by fitness
    std::sort(permutated_list.begin(), permutated_list.end(), [this] (std::pair<std::vector<tsp_node>,double> a, std::pair<std::vector<tsp_node>,double> b) {return compare(a, b);});
}

void tsp_ga::ga_selection() {
    //Roulette wheel selection/ Elitist selection. Maybe use elitism for smaller sets, use roulette wheel for bigger inputset
    //USe output from initialize() to select routes

    int elite_size = permutated_list.size() * 0.3;
    for(int i = 0; i < elite_size; i++) {
        selected_list.push_back(permutated_list[i]);
    }
    //Add roulette wheel?
}

void tsp_ga::ga_crossbreed() {
    int start, final;
    double fitness = 0.0;

    //Ordered crossbreed
    solution_list.push_back(selected_list[0]);  //save the best performing chromosome in the solutions list
    for(int i = 0; i < selected_list.size()/2; i++) {
        do {
            start = rand() % selected_list.size();
            final = rand() % selected_list.size();
        } while (start == final);
        next_gen_list.push_back(breed(selected_list[start], selected_list[final]));
    }
    
    for(int i = 0; i < selected_list[0].first.size(); i++) {
        std::cout << selected_list[0].first[i].get_nodeID() << " ";
    }   std::cout << "< " << selected_list[0].second << " >" << std::endl;
}

void tsp_ga::ga_mutate() {
    for(int i = 0; i < next_gen_list.size(); i++) {
        mutate(next_gen_list[i]);
    }
}

//Utility functions

void tsp_ga::fill_distance() {
    //This function populates the weight distribution matrix with 3D euclidean coordinates
    distance = new double*[size];
    for(int i = 0; i < size; i++) {
        distance[i] = new double[size];
    }   //Declaring space for distance[][]

    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++) {
            distance[i][j] = euclidian_distance(tsp_list[i], tsp_list[j]);  //fill distance matrix with euclidian weights
        }
    }
}

double tsp_ga::euclidian_distance(tsp_node node1, tsp_node node2) {
    return sqrt(pow(node2.get_X() - node1.get_X(), 2)+ pow(node2.get_Y() - node1.get_Y(), 2) + pow(node2.get_Z() - node1.get_Z(), 2));
}

double tsp_ga::euclidian_distance(std::vector<tsp_node>& chromosome) {
    double total_distance = 0.0;
    for(int i = 0; i < size - 1; i++) {
        total_distance += chromosome[i].dist(chromosome[i + 1]);
    }
    total_distance += chromosome[0].dist(chromosome[size - 1]);
    return total_distance;
}

void tsp_ga::permutate() {
    int count = 0;
    do {
        permutated_list.push_back(make_pair(tsp_list, 0.0));
        count++;
    }   while(std::next_permutation(tsp_list.begin(), tsp_list.end()) && count < 500);
}

bool tsp_ga::compare(const std::pair<std::vector<tsp_node>,double>& e1,const std::pair<std::vector<tsp_node>,double>& e2) {
    return e1.second < e2.second;
}

std::pair<std::vector<tsp_node>, double> tsp_ga::breed(std::pair<std::vector<tsp_node>, double>& gene1, std::pair<std::vector<tsp_node>, double>& gene2) {
    std::pair<std::vector<tsp_node>,double> child;
    int size = gene1.first.size();

    int r_start = rand() % (size);
    int r_end = r_start + (size);

    std::vector<tsp_node>::iterator it = gene1.first.begin();

    for(int i = 0; i <= size / 3; i++) {
        child.first.push_back(gene1.first[i]);
    }
    for(int i = 0; i < gene2.first.size(); i++) {
        if(std::find(child.first.begin(), child.first.end(), gene2.first[i]) == child.first.end()) {
            child.first.push_back(gene2.first[i]);
        }
    }
    return child;
}

void tsp_ga::mutate(std::pair<std::vector<tsp_node>, double>& chrom) {
    int mutation_rate = 10;
    for(int i = 0; i < chrom.first.size(); i++) {
        int indic = random() % 100 + 1;
        int swapWith;
        do {
            swapWith = random() % chrom.first.size();
        }   while(swapWith == i);
        if(indic <= mutation_rate) {
            std::swap(chrom.first[i], chrom.first[swapWith]);
        }
    }
}