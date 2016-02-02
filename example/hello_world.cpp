#include "../include/auto_tie.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>




// returns a tuple of id, gpa pointer, and name
std::tuple<int,std::unique_ptr<double>,std::string> get_student() {
	return std::make_tuple(1, std::make_unique<double>(4.2), std::string{ "John" });
}



int main() {

	// Set of student id, name, gpa,grade
	std::set<std::tuple<int,std::string,char,double>> myset;


	// Current
	{
		bool success;
		std::set<std::tuple<int,std::string,char,double>>::iterator iterator;

		std::tie(iterator, success) = myset.insert(std::make_tuple(1,"John",'A',4.0));

		if (success) {
			int id;
			std::string name;
			char grade;
			double gpa;

			std::tie(id, name, grade, gpa) = *iterator;
			std::cout << "Successfully inserted " << id << " " << " " << name << " " << grade << " " << gpa << "\n";
		}
	}


	// auto_tie
	{
		auto r = AUTO_TIE(iterator, success) = myset.insert(std::make_tuple(2,"Raja",'B',3.1)); 

		if (r.success) {
			auto s = AUTO_TIE_REF(id, name, grade, gpa) = *r.iterator;
			std::cout << "Successfully inserted " << s.id << " " << " " << s.name << " " << s.grade << " " << s.gpa << "\n";
		}
	}


	auto s = AUTO_TIE(id, gpa, name) = get_student();

	std::cout << "name: " << s.name << " id: " << s.id << " gpa: " << *s.gpa << "\n";

	// Writing the above without macros
	// The AUTO_TIE(id,gpa,name) above essentially expands to the auto_tie function call below (without the id_gpa_name part)

	auto id_gpa_name = auto_tie([](auto id_, auto gpa_, auto name_) {
		struct auto_struct {
			decltype(id_) id;
			decltype(gpa_) gpa;
			decltype(name_) name;
		};
		return static_cast<auto_struct*>(nullptr);
	});

	auto s2 = id_gpa_name = get_student();
	std::cout << "name: " << s2.name << " id: " << s2.id << " gpa: " << *s2.gpa << "\n";


	// Should be error here
	// auto s2 = AUTO_TIE_REF(id, gpa, name) = get_student();
	
	std::map<std::string, int> m = { { "John",1 } };

	for (auto &p : m) {
		auto r = AUTO_TIE_REF(key, value) = p;
		std::cout << r.key << " " << r.value << "\n";
		r.value = r.value * 2;
	}
	for (auto &p : m) {
		auto r = AUTO_TIE_REF(key, value) = p;
		std::cout << r.key << " " << r.value << "\n";
		r.value = r.value * 2;
	}

}
