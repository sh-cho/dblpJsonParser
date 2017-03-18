#include <cstdio>
#include <cinttypes>	//PRId64
#include <exception>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/regex.hpp>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/reader.h>

using namespace std;

const char* DBLP_FILENAME = "dblp.json";
const char* DBLP_COAUTHOR_FILENAME = "tmp_dblp_coauthorship.json";
const string COLUMN_DELIMITER = "||";
const string AUTHOR_DELIMITER = "&&";
boost::regex paper_reg{"(conf|journals).*"};


struct PaperRecord {
	string paper_key;
	vector<string> authors;
	unsigned int year;

	void write(ofstream& fout) {
		fout << paper_key << COLUMN_DELIMITER;
		for (auto it=authors.begin(); it!=authors.end(); ++it) {
			if (it != authors.begin()) {
				fout << AUTHOR_DELIMITER;
			}
			fout << *it;
		}
		fout << COLUMN_DELIMITER << year << endl;
	}
	void clear() {
		paper_key.clear();
		authors.clear();
		year = 1;
	}
	void add_author(string str) {
		authors.push_back(str);
	}
};

struct DblpPaperHandler {
	bool whole_array = false;
	bool is_record = false;
	bool is_authors = false;
	bool is_paper = false;
	uint64_t record_count = 0;

	PaperRecord paper;
	ofstream& ofs;

	DblpPaperHandler(ofstream& fout)
		: ofs(fout) {
	}

	//
	bool Null() {
		return true;
	}
	bool Bool(bool b) {
		//cout << "Bool(" << boolalpha << b << ")" << endl;
		return true;
	}
	bool Int(int i) {
		//cout << "Int(" << i << ")" << endl;
		return true;
	}
	bool Uint(unsigned u) {
		//cout << "Uint(" << u << ")" << endl;
		if (is_paper) {
			paper.year = u;
		}

		return true;
	}
	bool Int64(int64_t i) {
		//cout << "Int64(" << i << ")" << endl;
		return true;
	}
	bool Uint64(uint64_t u) {
		//cout << "Uint64(" << u << ")" << endl;
		return true;
	}
	bool Double(double d) {
		//cout << "Double(" << d << ")" << endl;
		return true;
	}
	bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) {
		//cout << "Number(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		if (is_record) {
			if (is_authors) {
				if (!is_paper)
					return true;

				paper.add_author(string(str));
			} else {
				if (boost::regex_match(str, paper_reg)) {
					is_paper = true;
					paper.paper_key = string(str);
				}
			}
		}
		return true;
	}
	bool StartObject() {
		//cout << "StartObject()" << endl;
		return true;
	}
	bool Key(const char* str, rapidjson::SizeType length, bool copy) {
		//cout << "Key(" << str << ", " << length << ", " << boolalpha << copy << ")" << endl;
		return true;
	}
	bool EndObject(rapidjson::SizeType memberCount) {
		//cout << "EndObject(" << memberCount << ")" << endl;
		return true;
	}
	bool StartArray() {
		if (!whole_array) {
			whole_array = true;
		} else if (!is_record) {
			is_record = true;
		} else if (!is_authors) {
			is_authors = true;
		}
		return true;
	}
	bool EndArray(rapidjson::SizeType elementCount) {
		if (is_record) {
			if (is_authors) {
				is_authors = false;
			} else {
				if (is_paper) {
					paper.write(ofs);
					paper.clear();
				}

				is_record = false;
				is_paper = false;
				++record_count;
				if (record_count % 100000 == 0) {
					printf("* [%" PRIu64 "] \n", record_count);
				}
			}
		} else {
			whole_array = false;
			printf("* total paper record: [%" PRIu64 "]\n", record_count);
		}
		return true;
	}
};


int main() {
	try {
		ifstream dblp_paper_in, dblp_coauthor_in;
		ofstream dblp_paper_out, dblp_coauthor_out;
		dblp_paper_in.open(DBLP_FILENAME);
		dblp_paper_out.open((string(DBLP_FILENAME)+string(".out")).c_str());
		if (!dblp_paper_in || !dblp_paper_out) {
			throw exception("dblp paper file");
		}
		
		DblpPaperHandler paper_handler(dblp_paper_out);

		rapidjson::IStreamWrapper dblp_paper_isw(dblp_paper_in);
		rapidjson::Reader reader;

		reader.Parse(dblp_paper_isw, paper_handler);

		//release
		if (dblp_paper_in) dblp_paper_in.close();
		if (dblp_paper_out) dblp_paper_out.close();
		if (dblp_coauthor_in) dblp_coauthor_in.close();
		if (dblp_coauthor_out) dblp_coauthor_out.close();
	}
	catch (const exception& e) {
		cerr << "Error: " << e.what() << endl;
		return -1;
	}

	return 0;
}