# auto_tie
A header-only kinda, sorta emulation of structured binding for C++14

#TLDR

`#include <auto_tie.hpp>` in your file

```cpp
// Set of student id, name, gpa,grade
std::set<std::tuple<int,std::string,char,double>> myset;

// AUTO_TIE copies/moves elements of the tuple/pair
auto r = AUTO_TIE(iterator, success) = myset.insert(std::make_tuple(2,"Raja",'B',3.1)); 
if (r.success) {
  // AUTO_TIE_REF has references to the tuple/pair elements
	auto s = AUTO_TIE_REF(id, name, grade, gpa) = *r.iterator;
	std::cout << "Successfully inserted " << s.id << " " << " " << s.name << " " << s.grade << " " << s.gpa << "\n";
}

```
