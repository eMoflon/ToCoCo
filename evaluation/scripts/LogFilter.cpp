#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>

using namespace std;

typedef struct data {
	int moteId;
	string message;
}data_t;

/**
*This Program Analyzes a Cooja log and writes the results in a csv. Inputs are the log and the *number of simulated nodes
**/


map<int,float> getAvgExecTimePerMote(map<int,vector<data_t> > filteredLog, map<int,int>* totalTimes,map<int,int>* degree, map<int, int>* enddegree) {
	map<int, float> result;
	int count;
	int totalSum;
	string time;
	string degreeString;
	int lastdegree;
	for (pair<int,vector<data_t> > moteLog : filteredLog) {
		bool startdegree = false;
		totalSum = 0;
		count = 0;
		for (data_t entry : moteLog.second) {
			if (entry.message.find("TIME:") != string::npos) {
				istringstream iss2(entry.message);
				//Skip module and TIME:
				iss2 >> time;
				iss2 >> time;
				//Load time in string and convert
				iss2 >> time;
				totalSum += stoi(time);
				count+=1;
			}
			if (entry.message.find("DEGREE:") != string::npos) {
				istringstream iss2(entry.message);
				//Skip module and DEGREE:
				iss2 >> degreeString;
				iss2 >> degreeString;
				//Load time in string and convert
				iss2 >> degreeString;
				if (!startdegree) {
					startdegree = true;
					degree->insert(pair<int, int>(moteLog.first, stoi(degreeString)));
					lastdegree = stoi(degreeString);
				}
				else lastdegree = stoi(degreeString);
			}
		}
		enddegree->insert(pair<int, int>(moteLog.first, lastdegree));
		totalTimes->insert(pair<int,int>(moteLog.first, totalSum));
		result.insert(pair<int,float>(moteLog.first,totalSum / count));
	}
return result;
}
map<int, float> getErrorRateperMote(map<int, vector<data_t> > filteredLog, map<int,int>* vnumRuns, map<int,int>* vnumErrors){
	map<int, float> result;
	int numRuns;
	int numErrors;
	bool run = false;
	for (pair<int, vector<data_t> > moteLog : filteredLog) {
		numRuns = 0;
		numErrors = 0;
		for (data_t entry : moteLog.second) {
			if (entry.message.find("Run") != string::npos) {
				numRuns++;
				run=true;
				continue;
			}
			if (entry.message.find("ERROR") != string::npos){ 
				if (run) {
					numErrors++;
					run = false;
				}
			}
		}
		vnumRuns->insert(pair<int,int>(moteLog.first, numRuns));
		vnumErrors->insert(pair<int,int>(moteLog.first, numErrors));
		result.insert(pair<int,float>(moteLog.first, numErrors / numRuns));
	}
	return result;
}

map<int,int> getRunsErrorTimeline(map<int, vector<data_t> > filteredLog) {
	map <int, int> result;
	vector<pair<int, int> > vec;
	int numRuns;
	int numErrors;
	bool run = false;
	for (pair<int, vector<data_t> > moteLog : filteredLog) {
		numRuns = 1;
		numErrors = 0;
		for (data_t entry : moteLog.second) {
			if (entry.message.find("Run") != string::npos) {
				if(run)
					vec.push_back(pair<int, int>(numRuns, 0));
				numRuns++;
				run = true;
				continue;
			}
			if (entry.message.find("ERROR") != string::npos) {
				if (run) {
					numErrors++;
					vec.push_back(pair<int, int>(numRuns, 1));
					run = false;
				}
			}
		}
	}
	for (pair<int, int> moteRun : vec) {
		int run = moteRun.first;
		if (result.find(run)!=result.end())
			result[run]=result[run]+moteRun.second;
		else result.insert(pair<int,int>(run,moteRun.second));
	}
	return result;
}

map<int,vector<data_t> > processFlocklab(string inputFileName){
	map<int, vector<data_t>> result;
	ifstream log;
	string linestring;
	log.open(inputFileName,ios::in);
	int id;
	string message;
	if (!log.is_open()) {
		cerr << "Error: Unable to open Log\n" << inputFileName << endl;
		return result;
	}
	while (!log.eof()) {
		getline(log, linestring, '\n');
		//skip comments
		if (linestring.find_first_of('#') == 0)
			continue;
		//skip timestamp
		linestring=linestring.substr(linestring.find_first_of(',')+1);
		//skip observerID
		linestring = linestring.substr(linestring.find_first_of(',')+1);
		//get ID
		string idString=linestring.substr(0,linestring.find_first_of(','));			
		if(!idString.empty())			
			id = stoi(idString);
		else break;
		//Now skip ID and direction
		linestring = linestring.substr(linestring.find_first_of(',')+1);
        linestring = linestring.substr(linestring.find_first_of(',')+1);
		//Filter TC messages
		if (linestring.find("LOCAL") != string::npos||linestring.find("Triangle")!=string::npos) {
			data_t chunk;
			chunk.moteId = id;
			chunk.message = linestring;
			if (result.count(id) != 0) {
				result.at(id).push_back(chunk);
			}
			else {//If no vector to the corresponding id messages is found
				vector<data_t> temp;
				temp.push_back(chunk);
				result.insert(pair<int,vector<data_t> >(id, temp));
			}
		}
		else continue;


	}
	return result;
}

map<int, vector<data_t> > processContiki(string inputFileName) {
	map<int, vector<data_t>> result;
	ifstream log;
	string linestring;
	log.open(inputFileName, ios::in);
	string idString;
	int id;
	string message;
	if (!log.is_open()) {
		cerr << "Error: Unable to open Log\n" << inputFileName<< endl;
		return result;
	}
	while (!log.eof()) {
		getline(log, linestring, '\n');
		//skip comments
		if (linestring.find_first_of('#') == 0)
			continue;
		//skip to ID
		linestring = linestring.substr(linestring.find_first_of('\t') + 1);
		istringstream iss(linestring);
		iss >> idString;
		id = stoi(idString.substr(3));
		//skip to message
		linestring = linestring.substr(linestring.find_first_of('\t') + 1);
		//Filter TC messages
		if (linestring.find("LOCAL") != string::npos||linestring.find("Triangle")!=string::npos) {
			data_t chunk;
			chunk.moteId = id;
			chunk.message = linestring;
			if (result.count(id) != 0) {
				result.at(id).push_back(chunk);
			}
			else {//If no vector to the corresponding id messages is found
				vector<data_t> temp;
				temp.push_back(chunk);
				result.insert(pair<int,vector<data_t> >(id, temp));
			}
		}
		else continue;


	}
	return result;
}

int main(int argc, char* argv[]) {
    string out = argv[3];
	string type = argv[2];
	string path = argv[1];
	map<int, vector<data_t> > sortedLog;
	std::cout << "Serial File: " << path + "/serial.csv" << std::endl;
	if (type.compare("flocklab")==0)
		sortedLog = processFlocklab(path + "/serial.csv");
	else if (type.compare("cooja")==0)
		sortedLog = processContiki(path+"/serial.csv");
	else {
		cout << "Unknown input type. Either flocklab or cooja are supported" << endl;
		return 0;
	}
    ofstream fout(out);
    fout<<"ID,Message";
    for (pair<int,vector<data_t> > moteLog : sortedLog){
        for(data_t dataObject:moteLog.second){
            fout<<std::endl;
            fout<<dataObject.moteId<<","<<dataObject.message;
        }
	}
	fout<<std::endl;
	fout.close();
}
