all:
	g++ -o DistanceVector -std=c++11 -Wall DistanceVector.cpp
	g++ -o SplitHorizon -std=c++11 -Wall SplitHorizon.cpp

clean:
	rm DistanceVector SplitHorizon