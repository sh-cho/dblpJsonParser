#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/regex.hpp>

#include <json/json.h>

using namespace std;

const char* DBLP_FILENAME = "dblp.json";
const char* DBLP_COAUTHOR_FILENAME = "tmp_dblp_coauthorship.json";
const string COLUMN_DELIMITER = "||";
const string AUTHOR_DELIMITER = "&&";

int main() {
	//init
	Json::Value root;
	Json::Reader reader;
	ifstream dblp_paper, dblp_coauthor;
	ofstream dblp_paper_out, dblp_coauthor_out;
	boost::regex paper_reg{"(conf|journals).*"};

	try {
		//1. dblp paper dataset
		dblp_paper.open(DBLP_FILENAME);
		dblp_paper_out.open((string(DBLP_FILENAME)+string(".out")).c_str());
		if (!dblp_paper || !dblp_paper_out) {
			throw exception("dblp paper file error");
		}
		printf("* complete open\n");

		if (!reader.parse(dblp_paper, root)) {
			throw exception("parse error");
		}
		printf("* complete parse\n");


		Json::Value row;
		Json::Value coauthors;

		std::string paper_key;
		std::vector<string> coauthor_list;
		int year;

		int count = 1;
		for (auto it=root.begin();
			it!=root.end();
			++it) {

			//전처리
			row.clear();
			coauthors.clear();
			coauthor_list.clear();

			//print
			if (count%1000000 == 0) {
				printf("* [%d]\n", count);
			}

			//row 단위로 read
			row = *it;
			paper_key = row[0].asString();

			//check whether it is paper
			if (boost::regex_match(paper_key, paper_reg)) {
				coauthors = row[1];
				for (auto coit=coauthors.begin(); coit!=coauthors.end(); ++coit) {
					coauthor_list.push_back(coit->asString());
				}
				year = ((row[2].isNull())?-1:row[2].asInt());

				//write
				dblp_paper_out << paper_key << COLUMN_DELIMITER;
				if (coauthor_list.size() > 0) {
					for (auto auit=coauthor_list.begin(); auit!=coauthor_list.end();) {
						dblp_paper_out << (*auit);
						++auit;
						if (auit != coauthor_list.end()) {
							dblp_paper_out << AUTHOR_DELIMITER;
						}
					}
				} else {
					//empty
					throw exception("paper without author");
				}
				dblp_paper_out << COLUMN_DELIMITER
					<< year
					<< endl;
			} else {
				//not paper
			}

			//후처리
			++count;
		}
		printf("* complete convert dblp paper\n");
	

		//2. dblp coauthorship dataset
		//dblp_coauthor.open(DBLP_COAUTHOR_FILENAME);

	}
	catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	}


	//release
	if (dblp_paper) dblp_paper.close();
	if (dblp_coauthor) dblp_coauthor.close();
	if (dblp_paper_out) dblp_paper_out.close();
	if (dblp_coauthor_out) dblp_coauthor_out.close();
	return 0;
}