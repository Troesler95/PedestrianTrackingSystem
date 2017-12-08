#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <caffe/proto/caffe.pb.h>
//#include <caffe/util/db.hpp>
//#include <caffe/util/format.hpp>
//#include <caffe/util/io.hpp>
//#include <caffe/util/rng.hpp>

using namespace std;

void CreateLMDBDataset(string rootFolderPath, string listfile, string dbname) 
{
	ifstream listFile(listfile);
	vector < std::pair<string, vector<int>> > lines;
}