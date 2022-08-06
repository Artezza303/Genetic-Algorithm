#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <random>
#include <time.h>
#include <numeric>
#include "omp.h"
using namespace std;

class Chromosome
{
public:

	string bitmap;

	Chromosome(int size)
	{
		bitmap = string(size, '0');

		for (int i = 0; i < size; i++)
		{
			if (((double)rand() / RAND_MAX) < 0.05)
			{
				bitmap.at(i) = '1';
			}
		}
	}
};

double Fitness(Chromosome chromo, vector<double> u, vector<double> w, double max_weight)
{
	double utility = 0.0;
	double weight = 0.0;

	for (int i = 0; i < chromo.bitmap.size(); i++)
	{
		if (chromo.bitmap.at(i) == '1')
		{
			utility += u.at(i);
			weight += w.at(i);

			if (weight > max_weight)
				return 1;
		}
	}
	return utility;
}


void Crossover(Chromosome chromo1, Chromosome chromo2, vector<Chromosome>& newGen)
{
	string one = chromo1.bitmap;
	string two = chromo2.bitmap;

	int random = rand() % (one.size() + 1);

	string c = one.substr(0, random) + two.substr(random, one.size());
	string d = two.substr(0, random) + one.substr(random, two.size());

	chromo1.bitmap = c;
	chromo2.bitmap = d;

	newGen.push_back(chromo1);
	newGen.push_back(chromo2);
}

class Population
{
public:
	vector<double> probabilityWheel;
	double averageFitness = 0.0;
	double bestFitness = 0.0;
	int bestChromo = 0;

	vector<Chromosome> population;
	int population_size;

	Population(int size, int pop) : population()
	{
		population_size = pop;
		for (int i = 0; i < pop; i++)
		{
			population.push_back(Chromosome(size));
		}
	}

	void Normalize_Population(vector<double> u, vector<double> w, double max_weight)
	{
		double value;

		vector<double> fitness;
		vector<double> squaredFitness;
		vector<double> probabiltyFitness;

		double sumOfSquares;

		for (int i = 0; i < population_size; i++)
		{
			value = Fitness(population.at(i), u, w, max_weight);
			fitness.push_back(value);

			value = fitness.at(i) * fitness.at(i);
			squaredFitness.push_back(value);
		}

		sumOfSquares = accumulate(squaredFitness.begin(), squaredFitness.end(), 0.0);

		for (int i = 0; i < squaredFitness.size(); i++)
		{
			value = squaredFitness.at(i) / sumOfSquares;
			probabiltyFitness.push_back(value);
		}

		probabilityWheel = probabiltyFitness;
		bestChromo = distance(fitness.begin(), max_element(fitness.begin(), fitness.end()));
		averageFitness = accumulate(fitness.begin(), fitness.end(), 0.0) / fitness.size();
		bestFitness = *max_element(fitness.begin(), fitness.end());
	}


	int SelectParent()
	{
		double rndNumber = ((double)rand() / (RAND_MAX));

		double offset = 0.0;
		int pick = 0;

		for (int i = 0; i < population_size; i++)
		{
			offset += probabilityWheel.at(i);
			if (rndNumber < offset)
			{
				pick = i;
				break;
			}
		}
		return pick;
	}

	void Selection()
	{
		vector<Chromosome> newGeneration;

		for (int i = 0; i < (population_size / 2); i++)
		{
			int parent1 = SelectParent();
			int parent2 = SelectParent();

			while (parent1 == parent2)
			{
				parent2 = SelectParent();
			}

			Crossover(population.at(parent1), population.at(parent2), newGeneration);
		}
		population = newGeneration;
	}

	void Mutation()
	{
		for (int i = 0; i < population_size; i++)
		{
			int r = rand() % 10000;

			if (r == 1)
			{
				int r2 = rand() % population.at(i).bitmap.size();

				if (population.at(i).bitmap.at(r2) == '1')
				{
					population.at(i).bitmap.at(r2) = '0';
				}
				else if (population.at(i).bitmap.at(r2) == '0')
				{
					population.at(i).bitmap.at(r2) = '1';
				}
			}
		}
	}


	void printBest(ofstream& file, vector<double> u, vector<double> w)
	{
		string bestBitmap = population.at(bestChromo).bitmap;

		double totalweight = 0;
		double totalUt = 0;

		for (int i = 0; i < bestBitmap.size(); i++)
		{
			if (bestBitmap.at(i) == '1')
			{
				totalweight += w.at(i);
				totalUt += u.at(i);
			}
		}

		cout << "Total Weight: " << totalweight << endl;
		cout << "Total Utility: " << totalUt << endl << endl;

		cout << "Items Selected (starting from 0):" << endl;


		file << "Total Weight: " << totalweight << endl;
		file << "Total Utility: " << totalUt << endl << endl;

		file << "Items Selected (starting from 0):" << endl;
		for (int i = 0; i < bestBitmap.size(); i++)
		{
			if (bestBitmap.at(i) == '1')
			{
				cout << i << endl;
				file << i << endl;
			}
		}
	}
};

int main()
{
	srand(time(NULL));

	vector<double> utility;
	vector<double> weight;

	const int limit = 500;
	const int population = 1000;

	ifstream ifs("Program2Input.txt");

	string line;

	while (getline(ifs, line))
	{
		istringstream iss(line);
		double a, b;
		if (!(iss >> a >> b)) { break; }

		utility.push_back(a);
		weight.push_back(b);
	}

	ifs.close();

	int x = utility.size();

	Population test(x, population);

	for (int i = 0; i < 1000; i++)
	{
		//double start = omp_get_wtime();
		test.Normalize_Population(utility, weight, limit);
		//double end = omp_get_wtime();
		//printf("Normalize took % f seconds\n", end - start);

		//start = omp_get_wtime();
		test.Selection();
		//end = omp_get_wtime();
		//printf("Selection took % f seconds\n", end - start);

		//start = omp_get_wtime();
		test.Mutation();
		//double end = omp_get_wtime();
		//printf("Mutation took % f seconds\n", end - start);

		//printf("Total took % f seconds\n", end - start);


		cout << "Gen: " << i << "    Avg: " << test.averageFitness << "      Best: " << test.bestFitness << endl;
	}

	test.Normalize_Population(utility, weight, limit);
	ofstream myfile;
	myfile.open("BestUtilitySolution.txt");
	test.printBest(myfile, utility, weight);
	myfile.close();
}








/*
		if (i % 10 == 0 && counter == 0)
		{
			initialFitness = test.averageFitness;
			counter += 1;
		}
		else if (i % 10 == 0 && counter == 1)
		{
			postFitness = test.averageFitness;
			counter = 0;
		}

		if ( abs(((postFitness - initialFitness) / ((initialFitness + postFitness) / 2))) < 0.005)
		{

			cout << "CALCULATION " << abs(((postFitness - initialFitness) / ((initialFitness + postFitness) / 2))) << endl;
			cout << "INITIAL " << initialFitness << endl;
			cout << "POST " << postFitness << endl;

			break;
		}
*/
