#include <thread>
#include <mutex>
#include <iostream>
#include <map>
#include <stdlib.h>   
#include <random>
#include <string>
#include <condition_variable>
#include "LockGuardMy.h"

#define LOWER_LIMIT 0
#define UPPER_LIMIT 25
#define NUMBER_OF_NUMBERS 5
#define NOT_FOUND 0
#define OFFSET 1

std::condition_variable conVar;
std::condition_variable conVarProcess;
std::mutex *mutex;
std::mutex *mainMutex;
std::map<int, int> * generatedNumbersPlayer;
std::map<int, int> * drawnNumbers;
std::minstd_rand0 generator;
int score = 0;
bool resulted = false;
bool ready = false;

void printGeneratedNumbers(std::map<int, int> * generatedNumbers, std::string title){
	std::cout << title;
	std::map<int, int>::iterator it;

	for (it = generatedNumbers->begin(); it != generatedNumbers->end(); it++)
	{
		std::cout << it->first << "; ";
	}
}

int getRandomNumber(int min, int max) {
	return min + generator() % ((max + 1) - min);
}

void generateNumbers(std::map<int, int> * generatedNumbersMap) {
	for (int i = 0; i < NUMBER_OF_NUMBERS; i++) {
		int newNumber = getRandomNumber(LOWER_LIMIT, UPPER_LIMIT);

		if (generatedNumbersMap->count(newNumber)) {
			i--;
		}
		else {
			generatedNumbersMap->insert(std::pair<int, int>(newNumber, newNumber));
		}
	}
}

void getResults() {
	std::unique_lock<std::mutex> lock(*mainMutex);
	conVar.wait(lock, [] {return ready; });

	std::map<int, int>::iterator it;
	for (it = generatedNumbersPlayer->begin(); it != generatedNumbersPlayer->end(); it++)
	{
		std::cout << "Results Waiting\n";
		std::unique_lock<std::mutex> lock2(*mutex);
		conVarProcess.wait(lock2, [] {return !resulted; });
		std::cout << "Results Working\n";
		int currentNumber = it->first;

		if (drawnNumbers->count(currentNumber)) {
			score += 2;
		}

		resulted = true;
		lock2.unlock();
		conVarProcess.notify_one();
	}

	lock.unlock();
	conVar.notify_one();
}

void getResultsSecondChance() {

	std::map<int, int>::iterator it;
	for (it = generatedNumbersPlayer->begin(); it != generatedNumbersPlayer->end(); it++)
	{
		std::cout << "ResultsSec Waiting\n";
		std::unique_lock<std::mutex> lock(*mutex);
		conVarProcess.wait(lock, [] {return resulted; });
			
		std::cout << "ResultsSec Working\n";
			int currentNumber = it->first;
			if (drawnNumbers->count(currentNumber + OFFSET) || drawnNumbers->count(currentNumber - OFFSET)) {
				score++;
			}
		resulted = false;
		lock.unlock();
		conVarProcess.notify_one();
	}
}

void enterNumbersUser() {

	std::cout << "-----WELCOME in game LOTO 5 from 25-----\nWrite 5 numbers from range 0 - 25!\n";
	int currentnumber;
	for (int i = 0; i < NUMBER_OF_NUMBERS; i++) {
		std::cin >> currentnumber;

		if (currentnumber >= LOWER_LIMIT && currentnumber <= UPPER_LIMIT && generatedNumbersPlayer->count(currentnumber) == NOT_FOUND) {
			generatedNumbersPlayer->insert(std::pair<int, int>(currentnumber, currentnumber));
		}
		else {
			i--;
			std::cout << "Entred number is not from range 0 - 25 or is used yet! Choose another one!\n";
		}
	}
}

void printResults() {
	printGeneratedNumbers(generatedNumbersPlayer, "\nGenerated numbers for Player: ");
	printGeneratedNumbers(drawnNumbers, "\n\nDrawn numbers: ");
	std::cout << "\n\nYour Score: " << score << "\n\n\n";
}

int main() {
	mutex = new std::mutex;
	mainMutex = new std::mutex;
	generatedNumbersPlayer = new std::map<int, int>();
	drawnNumbers = new std::map<int, int>();

	generator.seed(time(NULL));
	std::thread enter(enterNumbersUser);
	std::thread drawnN(generateNumbers, drawnNumbers);

	drawnN.join();
	enter.join();

	std::thread results(getResults);
	std::thread resultsSecondChance(getResultsSecondChance);

	{
		ready = true;
	}

	conVar.notify_one();

	{
		std::unique_lock<std::mutex> lock(*mainMutex);
		conVar.wait(lock, [] {return ready; });
	}

	results.join();
	resultsSecondChance.join();

	printResults();

	delete generatedNumbersPlayer;
	delete drawnNumbers;

	system("pause");

	return 0;
}
